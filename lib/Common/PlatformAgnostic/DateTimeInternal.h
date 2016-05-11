//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef RUNTIME_PLATFORM_AGNOSTIC_DATETIME_INTERNAL
#define RUNTIME_PLATFORM_AGNOSTIC_DATETIME_INTERNAL

#include "Core/CommonTypedefs.h"

// todo: move this under PlatformAgnostic
//       Update to PlatformAgnostic::DateTime namespace
namespace PlatformAgnostic
{
namespace DateTime
{
    struct YMD;

#ifdef _WIN32

    #define DateTimeTicks_PerMinute 60000.0
    #define DateTimeTicks_PerDay (DateTimeTicks_PerMinute * 60 * 24)
    #define DateTimeTicks_PerLargestTZOffset (DateTimeTicks_PerMinute * 60 * 24 + 1)
    #define DateTimeTicks_PerNonLeapYear (DateTimeTicks_PerDay * 365)
    #define DateTimeTicks_PerSafeEndOfYear (DateTimeTicks_PerNonLeapYear - DateTimeTicks_PerLargestTZOffset)

    class TimeZoneInfo // DateTime.cpp
    {
    public:
        double daylightDate;
        double standardDate;
        double january1;
        double nextJanuary1;

        int32 daylightBias;
        int32 standardBias;
        int32 bias;
        uint32 lastUpdateTickCount;

        bool isDaylightTimeApplicable;
        bool isJanuary1Critical;

        TimeZoneInfo();
        bool IsValid(const double time);
        void Update(const double time);
    };

    struct DaylightTimeHelperData // DaylightHelper.cpp
    {
        TimeZoneInfo cache1, cache2;
        bool useFirstCache;

        static bool IsDaylightSavings(double time, bool isUtcTime, TimeZoneInfo *timeZoneInfo);
        static bool IsDaylightSavings(double utcTime, double localTime, int bias);
        static bool IsDaylightSavingsUnsafe(double time, TimeZoneInfo *timeZoneInfo);
        static inline bool IsCritical(double time, TimeZoneInfo *timeZoneInfo);
        TimeZoneInfo *GetTimeZoneInfo(const int year);
        TimeZoneInfo *GetTimeZoneInfoFromDouble(double time);

        static inline double UtcToLocalFast(double utcTime, TimeZoneInfo *timeZoneInfo, int &bias, int &offset, bool &isDaylightSavings);
               inline double UtcToLocalCritical(double utcTime, TimeZoneInfo *timeZoneInfo, int &bias, int &offset, bool &isDaylightSavings);
        static inline double LocalToUtcFast(double localTime, TimeZoneInfo *timeZoneInfo);
        static inline double LocalToUtcCritical(double localTime, TimeZoneInfo *timeZoneInfo);
    };

    #define CHAKRA_PA_DATETIME_DAYLIGHT_PLATFORM_SPECIFIC     \
    private:                                                  \
        DaylightTimeHelperData data;                          \
    public:                                                   \
        static bool ForceOldDateAPIFlag();                    \
        static void YmdToSystemTime(YMD *ymd, SYSTEMTIME *sys)\
        { ymd->ToSystemTime(sys); }

        // Moved DaylightTimeHelper to common.lib to share with hybrid debugging. However this function depends
        // on runtime. Runtime and hybrid debugging needs to provide implementation.

    #define CHAKRA_PA_DATETIME_UTILITY_PLATFORM_SPECIFIC  \
        WCHAR standardZoneName[32];                       \
        WCHAR daylightZoneName[32];                       \
        size_t standardZoneNameLength;                    \
        size_t daylightZoneNameLength;                    \
        uint32 lastTimeZoneUpdateTickCount;               \
                                                          \
        void UpdateTimeZoneInfo();                        \
    public:                                               \
        Utility(): lastTimeZoneUpdateTickCount(0) { }

    #define CHAKRA_PA_DATETIME_HIRESTIMER_PLATFORM_SPECIFIC \
        private:                                            \
            double dBaseTime;                               \
            double dLastTime;                               \
            double dAdjustFactor;                           \
            uint64 baseMsCount;                             \
            uint64 freq;                                    \
                                                            \
            bool fReset;                                    \
            bool fInit;                                     \
            bool fHiResAvailable;                           \
                                                            \
            double GetAdjustFactor();                       \
        public:                                             \
            HiResTimer(): fInit(false), dBaseTime(0),       \
            baseMsCount(0),  fHiResAvailable(true),         \
            dLastTime(0), dAdjustFactor(1), fReset(true) {} \
                                                            \
            void Reset() { fReset = true; }

#else // ! _WIN32

    #define CHAKRA_PA_DATETIME_DAYLIGHT_PLATFORM_SPECIFIC

    #define CHAKRA_PA_DATETIME_UTILITY_PLATFORM_SPECIFIC  \
        WCHAR standardZoneName[32];                       \
        size_t standardZoneNameLength;                    \
        uint32 lastTimeZoneUpdateTickCount;               \
                                                          \
        void UpdateTimeZoneInfo();                        \
    public:                                               \
        Utility(): lastTimeZoneUpdateTickCount(0) { }

    #define CHAKRA_PA_DATETIME_HIRESTIMER_PLATFORM_SPECIFIC \
    public:                                                 \
            void Reset() { }

#endif // ! _WIN32

} // namespace DateTime
} // namespace PlatformAgnostic

#endif // RUNTIME_PLATFORM_AGNOSTIC_DATETIME_INTERNAL
