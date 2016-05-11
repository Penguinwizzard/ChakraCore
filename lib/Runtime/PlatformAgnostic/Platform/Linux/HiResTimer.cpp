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

    double HiResTimer::GetSystemTime()
    {
// todo-osx: mach_absolute_time
#ifndef __APPLE__
        struct timespec fast_time;
        if (clock_gettime(CLOCK_REALTIME, &fast_time) == 0)
        {
            return (fast_time.tv_sec * 1000) + (int32_t)(fast_time.tv_nsec/1e6);
        }
#endif

        time_t tt;
    #if HAVE_GMTIME_R
        struct tm ut;
    #endif  /* HAVE_GMTIME_R */
        struct tm *utPtr;
        struct timeval timeval;
        int timeofday_retval;

        tt = time(NULL);

        timeofday_retval = gettimeofday(&timeval,NULL);

    #if HAVE_GMTIME_R
        utPtr = &ut;
        if (gmtime_r(&tt, utPtr) == NULL)
    #else   /* HAVE_GMTIME_R */
        if ((utPtr = gmtime(&tt)) == NULL)
    #endif  /* HAVE_GMTIME_R */
        {
            Assert(0 && "gmtime() failed");
        }

        size_t milliseconds = 0;

        if(-1 != timeofday_retval)
        {
            int old_seconds;
            int new_seconds;
        
            milliseconds = timeval.tv_usec/1000;
        
            old_seconds = utPtr->tm_sec;
            new_seconds = timeval.tv_sec%60;
       
            /* just in case we reached the next second in the interval between 
               time() and gettimeofday() */
            if( old_seconds!=new_seconds )
            {
                milliseconds = 999;
            }  
        }                      

        milliseconds = (utPtr->tm_hour * 3600000.0)
                        + (utPtr->tm_min * 60000.0)
                        + (utPtr->tm_sec * 1000.0)
                        + milliseconds;

        return Js::DateUtilities::TvFromDate(1900 + utPtr->tm_year, utPtr->tm_mon, 
                                            utPtr->tm_mday-1, milliseconds);
    }

    double HiResTimer::Now()
    {
        return GetSystemTime();
    }

} // namespace DateTime
} // namespace PlatformAgnostic
