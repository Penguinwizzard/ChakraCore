//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "Runtime.h"
#include <time.h>
#include <sys/time.h>
#include "ChakraPlatform.h"

namespace PlatformAgnostic
{
namespace DateTime
{

    #define updatePeriod 1000

    const WCHAR *Utility::GetStandardName(size_t *nameLength) {
        UpdateTimeZoneInfo();
        *nameLength = standardZoneNameLength;
        return standardZoneName;
    }

    const WCHAR *Utility::GetDaylightName(size_t *nameLength) {
        // We have an abbreviated standard or daylight name
        // based on the date and time zone we are in.
        return GetStandardName(nameLength);
    }

    void Utility::UpdateTimeZoneInfo()
    {
        uint32 tickCount = GetTickCount();
        if (tickCount - lastTimeZoneUpdateTickCount > updatePeriod)
        {
            time_t gtime = time(NULL);
            struct tm ltime;

            tzset();
            localtime_r(&gtime, &ltime);

            standardZoneNameLength = strlen(ltime.tm_zone);

        #if defined(WCHAR_IS_CHAR16_T)
            for(int i = 0; i < standardZoneNameLength; i++) {
                standardZoneName[i] = (char16_t)ltime.tm_zone[i];
            }
        #elif defined(WCHAR_IS_WCHAR_T)
            mbstowcs( (wchar_t*) standardZoneName, ltime.tm_zone,
                      sizeof(standardZoneName) / sizeof(WCHAR));
        #else
        #error "WCHAR should be either wchar_t or char16_t"
        #endif

            standardZoneName[standardZoneNameLength] = (WCHAR)0;
            lastTimeZoneUpdateTickCount = tickCount;
        }
    }

    // DateTimeHelper ******
    #define TM_TO_SYSTIME(SYS, TM)           \
            SYS->wYear = TM->tm_year + 1900; \
            SYS->wMonth = TM->tm_mon + 1;    \
            SYS->wDayOfWeek = TM->tm_wday;   \
            SYS->wDay = TM->tm_mday;         \
            SYS->wHour = TM->tm_hour;        \
            SYS->wMinute = TM->tm_min;       \
            SYS->wSecond = TM->tm_sec

    #define SYSTIME_TO_TM(SYS, TM)           \
            TM->tm_year = SYS->wYear - 1900; \
            TM->tm_mon = SYS->wMonth - 1;    \
            TM->tm_wday = SYS->wDayOfWeek;   \
            TM->tm_mday = SYS->wDay;         \
            TM->tm_hour = SYS->wHour;        \
            TM->tm_min = SYS->wMinute;       \
            TM->tm_sec = SYS->wSecond

    const bool SysLocalToUtc(SYSTEMTIME *local, SYSTEMTIME *utc)
    {
        struct tm local_tm;
        SYSTIME_TO_TM(local, (&local_tm));

        // tm doesn't have milliseconds
        // uint32_t msec = utc->wMilliseconds;
        // simply keep it as is

        tzset();
        time_t utime = timegm(&local_tm);
        struct tm local_bck;
        memcpy(&local_bck, &local_tm, sizeof(struct tm));
        mktime(&local_bck);
        utime -= local_bck.tm_gmtoff; // reverse UTC

        struct tm utc_tm = *gmtime (&utime);

        TM_TO_SYSTIME(utc, (&utc_tm));
        // put milliseconds back
        utc->wMilliseconds = local->wMilliseconds;
        return true;
    }

    const bool SysUtcToLocal(SYSTEMTIME *utc, SYSTEMTIME *local, int64_t &bias, int &offset, bool &isDaylightSavings)
    {
        struct tm utc_tm;
        SYSTIME_TO_TM(utc, (&utc_tm));

        // tm doesn't have milliseconds
        // uint32_t msec = utc->wMilliseconds;
        // simply keep it as is
        tzset();
        time_t ltime = mktime(&utc_tm);
        time_t org_ltime = ltime;

        offset = utc_tm.tm_gmtoff / 60;
        ltime += utc_tm.tm_gmtoff; // reverse UTC

        struct tm local_tm = *localtime(&ltime);

        TM_TO_SYSTIME(local, (&local_tm));

        // put milliseconds back
        local->wMilliseconds = utc->wMilliseconds;

        isDaylightSavings = local_tm.tm_isdst;

        struct tm gtime = *gmtime(&org_ltime);
        bias = (int64_t)(mktime(&gtime) - org_ltime) / 60;

        return true;
    }

    // DaylightTimeHelper ******
    double DaylightTimeHelper::UtcToLocal(double utcTime, int &bias, int &offset, bool &isDaylightSavings)
    {
        SYSTEMTIME utcSystem, localSystem;
        YMD ymd;

        Js::DateUtilities::GetYmdFromTv(utcTime, &ymd);
        ymd.ToSystemTime(&utcSystem);

        int64_t bias64;
        SysUtcToLocal(&utcSystem, &localSystem, bias64, offset, isDaylightSavings);

        if (bias64 < INT_MAX)
        {
            bias = bias64;
        }
        else
        {
            bias = 0; // silently fail
        }

        return Js::DateUtilities::TimeFromSt(&localSystem);
    }

    double DaylightTimeHelper::LocalToUtc(double localTime)
    {
        SYSTEMTIME utcSystem, localSystem;
        YMD ymd;

        Js::DateUtilities::GetYmdFromTv(localTime, &ymd);
        ymd.ToSystemTime(&utcSystem);

        SysLocalToUtc(&utcSystem, &localSystem);

        return Js::DateUtilities::TimeFromSt(&localSystem);
    }

} // namespace DateTime
} // namespace PlatformAgnostic
