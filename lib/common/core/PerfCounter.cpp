//---------------------------------------------------------------------------
// Copyright (C) 2011 by Microsoft Corporation.  All rights reserved.
//----------------------------------------------------------------------------

#include "CommonCorePch.h"
#include "PerfCounterImpl.cpp"

#ifdef PERF_COUNTERS
namespace PerfCounter
{
    Counter& Counter::operator+=(size_t value)
    { 
        Assert(count); 
        ::InterlockedExchangeAdd(count, (DWORD)value); 
        return *this; 
    }
    Counter& Counter::operator-=(size_t value)
    { 
        Assert(count); 
        ::InterlockedExchangeSubtract(count, (DWORD)value); 
        return *this; 
    }
    Counter& Counter::operator++()
    { 
        Assert(count); 
        ::InterlockedIncrement(count);
        return *this; 
    }
    Counter& Counter::operator--()
    { 
        Assert(count); 
        ::InterlockedDecrement(count); 
        return *this; 
    }
}
#endif