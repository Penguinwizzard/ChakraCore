//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "RuntimePlatformAgnosticPch.h"
#include "Runtime.h"
#include <time.h>
#include "ChakraPlatform.h"

namespace PlatformAgnostic
{
namespace DateTime
{

    #define updatePeriod 1000

    static const double criticalMin = Js::DateUtilities::TvFromDate(1601, 0, 1, 0); // minimal year for which windows has time zone information
    static const double criticalMax = Js::DateUtilities::TvFromDate(USHRT_MAX-1, 0, 0, 0);

    typedef BOOL(*DateConversionFunction)(
        _In_opt_ CONST PVOID lpTimeZoneInformation,
        _In_ CONST SYSTEMTIME * lpLocalTime,
        _Out_ LPSYSTEMTIME lpUniversalTime
        );

    static DateConversionFunction sysLocalToUtc = NULL;
    static DateConversionFunction sysUtcToLocal = NULL;
    static HINSTANCE g_timezonedll = NULL;

    static HINSTANCE TryLoadLibrary()
    {
        if (g_timezonedll == NULL)
        {
            HMODULE hLocal = LoadLibraryExW(_u("api-ms-win-core-timezone-l1-1-0.dll"), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (hLocal != NULL)
            {
                if (InterlockedCompareExchangePointer((PVOID*) &g_timezonedll, hLocal, NULL) != NULL)
                {
                    FreeLibrary(hLocal);
                }
            }
        }

        if (g_timezonedll == NULL)
        {
            HMODULE hLocal = LoadLibraryExW(_u("kernel32.dll"), nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (hLocal != NULL)
            {
                if (InterlockedCompareExchangePointer((PVOID*) &g_timezonedll, hLocal, NULL) != NULL)
                {
                    FreeLibrary(hLocal);
                }
            }
        }
        return g_timezonedll;
    }

    static BOOL SysLocalToUtc(SYSTEMTIME *local, SYSTEMTIME *utc)
    {
        if (sysLocalToUtc == NULL)
        {
            HINSTANCE library = TryLoadLibrary();
            if (library != NULL && !DaylightTimeHelper::ForceOldDateAPIFlag())
            {
                sysLocalToUtc = (DateConversionFunction)GetProcAddress(library, "TzSpecificLocalTimeToSystemTimeEx");
            }
            if (sysLocalToUtc == NULL)
            {
                sysLocalToUtc = (DateConversionFunction)TzSpecificLocalTimeToSystemTime;
            }
        }
        return sysLocalToUtc(NULL, local, utc);
    }

    static BOOL SysUtcToLocal(SYSTEMTIME *utc, SYSTEMTIME *local)
    {
        if (sysUtcToLocal == NULL)
        {
            HINSTANCE library = TryLoadLibrary();
            if (library != NULL)
            {
                sysUtcToLocal = (DateConversionFunction)GetProcAddress(library, "SystemTimeToTzSpecificLocalTimeEx");
            }
            if (sysUtcToLocal == NULL)
            {
                sysUtcToLocal = (DateConversionFunction)SystemTimeToTzSpecificLocalTime;
            }
        }
        return sysUtcToLocal(NULL, utc, local);
    }

    TimeZoneInfo* DaylightTimeHelperData::GetTimeZoneInfoFromDouble(double time)
    {
        int32_t year, yearType;
        Js::DateUtilities::GetYearFromTv(time, year, yearType);

        return GetTimeZoneInfo(year);
    }

    TimeZoneInfo* DaylightTimeHelperData::GetTimeZoneInfo(const int year)
    {
        if (cache1.IsValid(year)) return &cache1;
        if (cache2.IsValid(year)) return &cache2;

        if (useFirstCache)
        {
            cache1.Update(year);
            useFirstCache = false;
            return &cache1;
        }
        else
        {
            cache2.Update(year);
            useFirstCache = true;
            return &cache2;
        }
    }

    // DaylightTimeHelperData ******

    bool DaylightTimeHelperData::IsDaylightSavings(double time, bool utcTime, TimeZoneInfo *timeZoneInfo)
    {
        if (!timeZoneInfo->isDaylightTimeApplicable)
        {
            return false;
        }

        double localDaylight = timeZoneInfo->daylightDate;
        double localStandard = timeZoneInfo->standardDate;

        if (utcTime)
        {
            double biasInTicks = timeZoneInfo->bias * DateTimeTicks_PerMinute;
            localDaylight += biasInTicks + (timeZoneInfo->standardBias * DateTimeTicks_PerMinute);
            localStandard += biasInTicks + (timeZoneInfo->daylightBias * DateTimeTicks_PerMinute);
        }
        else
        {
            localDaylight -= (timeZoneInfo->daylightBias * DateTimeTicks_PerMinute);
            localStandard -= (timeZoneInfo->standardBias * DateTimeTicks_PerMinute);
        }

        return (localDaylight < localStandard)
                ? localDaylight <= time && time < localStandard
                : time < localStandard || localDaylight <= time;
    }

    // in slow path we use system API to perform conversion, but we still need to know whether current time is
    // standard or daylight savings in order to create a string representation of a date.
    // So just compare whether difference between local and utc time equal to bias.
    bool DaylightTimeHelperData::IsDaylightSavings(double utcTime, double localTime, int bias)
    {
        return ((int)(utcTime - localTime)) / ((int)(DateTimeTicks_PerMinute)) != bias;
    }


    // This function does not properly handle boundary cases.
    // But while we use IsCritical we don't care about it.
    bool DaylightTimeHelperData::IsDaylightSavingsUnsafe(double time, TimeZoneInfo *timeZoneInfo)
    {
        return timeZoneInfo->isDaylightTimeApplicable && ((timeZoneInfo->daylightDate < timeZoneInfo->standardDate)
            ? timeZoneInfo->daylightDate <= time && time < timeZoneInfo->standardDate
            : time < timeZoneInfo->standardDate || timeZoneInfo->daylightDate <= time);
    }

    double DaylightTimeHelperData::UtcToLocalFast(double utcTime, TimeZoneInfo *timeZoneInfo, int &bias, int &offset, bool &isDaylightSavings)
    {
        double localTime;
        localTime = utcTime - DateTimeTicks_PerMinute * timeZoneInfo->bias;
        isDaylightSavings = IsDaylightSavingsUnsafe(utcTime, timeZoneInfo);
        if (isDaylightSavings)
        {
            localTime -= DateTimeTicks_PerMinute * timeZoneInfo->daylightBias;
        } else {
            localTime -= DateTimeTicks_PerMinute * timeZoneInfo->standardBias;
        }

        bias = timeZoneInfo->bias;
        offset = ((int)(localTime - utcTime)) / ((int)(DateTimeTicks_PerMinute));

        return localTime;
    }

    double DaylightTimeHelperData::UtcToLocalCritical(double utcTime, TimeZoneInfo *timeZoneInfo, int &bias, int &offset, bool &isDaylightSavings)
    {
        double localTime;
        SYSTEMTIME utcSystem, localSystem;
        YMD ymd;

        Js::DateUtilities::GetYmdFromTv(utcTime, &ymd);
        ymd.ToSystemTime(&utcSystem);

        if (!SysUtcToLocal(&utcSystem, &localSystem))
        {
            // SysUtcToLocal can fail if the date is beyond extreme internal boundaries (e.g. > ~30000 years).
            // Fall back to our fast (but less accurate) version if the call fails.
            return UtcToLocalFast(utcTime, timeZoneInfo, bias, offset, isDaylightSavings);
        }

        localTime = Js::DateUtilities::TimeFromSt(&localSystem);
        if (localSystem.wYear != utcSystem.wYear)
        {
            timeZoneInfo = GetTimeZoneInfo(localSystem.wYear);
        }

        bias = timeZoneInfo->bias;
        isDaylightSavings = IsDaylightSavings(utcTime, localTime, timeZoneInfo->bias + timeZoneInfo->standardBias);
        offset = ((int)(localTime - utcTime)) / ((int)(DateTimeTicks_PerMinute));

        return localTime;
    }

    double DaylightTimeHelper::UtcToLocal(double utcTime, int &bias, int &offset, bool &isDaylightSavings)
    {
        TimeZoneInfo *timeZoneInfo = data.GetTimeZoneInfoFromDouble(utcTime);

        if (data.IsCritical(utcTime, timeZoneInfo))
        {
            return data.UtcToLocalCritical(utcTime, timeZoneInfo, bias, offset, isDaylightSavings);
        }
        else
        {
            return data.UtcToLocalFast(utcTime, timeZoneInfo, bias, offset, isDaylightSavings);
        }
    }


    double DaylightTimeHelperData::LocalToUtcFast(double localTime, TimeZoneInfo *timeZoneInfo)
    {
        double utcTime = localTime + DateTimeTicks_PerMinute * timeZoneInfo->bias;
        bool isDaylightSavings = IsDaylightSavingsUnsafe(localTime, timeZoneInfo);

        if (isDaylightSavings)
        {
            utcTime += DateTimeTicks_PerMinute * timeZoneInfo->daylightBias;
        } else {
            utcTime += DateTimeTicks_PerMinute * timeZoneInfo->standardBias;
        }

        return utcTime;
    }

    double DaylightTimeHelperData::LocalToUtcCritical(double localTime, TimeZoneInfo *timeZoneInfo)
    {
        SYSTEMTIME localSystem, utcSystem;
        YMD ymd;

        Js::DateUtilities::GetYmdFromTv(localTime, &ymd);
        ymd.ToSystemTime(&localSystem);

        if (!SysLocalToUtc(&localSystem, &utcSystem))
        {
            // Fall back to our fast (but less accurate) version if the call fails.
            return LocalToUtcFast(localTime, timeZoneInfo);
        }

        return Js::DateUtilities::TimeFromSt(&utcSystem);
    }

    double DaylightTimeHelper::LocalToUtc(double localTime)
    {
        TimeZoneInfo *timeZoneInfo = data.GetTimeZoneInfoFromDouble(localTime);

        if (data.IsCritical(localTime, timeZoneInfo))
        {
            return data.LocalToUtcCritical(localTime, timeZoneInfo);
        }
        else
        {
            return data.LocalToUtcFast(localTime, timeZoneInfo);
        }
    }

    // we consider January 1st, December 31st and days when daylight savings time starts and ands to be critical,
    // because there might be ambiguous cases in local->utc->local conversions,
    // so in order to be consistent with Windows we rely on it to perform conversions. But it is slow.
    bool DaylightTimeHelperData::IsCritical(double time, TimeZoneInfo *timeZoneInfo)
    {
        return time > criticalMin && time < criticalMax &&
            (fabs(time - timeZoneInfo->daylightDate) < DateTimeTicks_PerLargestTZOffset ||
            fabs(time - timeZoneInfo->standardDate) < DateTimeTicks_PerLargestTZOffset ||
            time > timeZoneInfo->january1 + DateTimeTicks_PerSafeEndOfYear ||
            (timeZoneInfo->isJanuary1Critical && time - timeZoneInfo->january1 < DateTimeTicks_PerLargestTZOffset));
    }

    // Moved DaylightTimeHelper to common.lib to share with hybrid debugging, but this function depends on runtime.
    bool DaylightTimeHelper::ForceOldDateAPIFlag()
    {
        // The following flag was added to support Date unit test on win7.
        return CONFIG_ISENABLED(Js::ForceOldDateAPIFlag);
    }

} // namespace DateTime
} // namespace PlatformAgnostic
