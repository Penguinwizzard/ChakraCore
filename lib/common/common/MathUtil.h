//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

///---------------------------------------------------------------------------
///
/// class Math
///
///---------------------------------------------------------------------------

class Math
{
    static volatile UINT_PTR        RandSeed;

public:

    template <typename T>
    static      T                   Align(T size, T alignment)
    {
        return ((size + (alignment-1)) & ~(alignment-1));
    }

    // Explicit cast to integral(may truncate).  Avoids warning C4302 'type cast': truncation
    template <typename T>
    static      T                   PointerCastToIntegral(void * pointer)
    {
        return (T)(uintptr)pointer;
    }

    static      bool                FitsInDWord(size_t value);
    static      UINT_PTR            Rand();
    static      bool                IsPow2(int32 val) { return (val > 0 && ((val-1) & val) == 0); }
    static      uint32              NextPowerOf2(uint32 n);

    // Use for compile-time evaluation of powers of 2
    template<uint32 val> struct Is
    {
        static const bool Pow2 = ((val-1) & val) == 0;
    };

    // Defined in the header so that the Recycler static lib doesn't
    // need to pull in jscript.common.common.lib
    static uint32 Log2(uint32 value)
    {
        int i;

        for (i = 0; value >>= 1; i++);
        return i;
    }

    // Define a couple of overflow policies for the UInt32Math routines.

    // The default policy for overflow is to throw an OutOfMemory exception
    __declspec(noreturn) static void DefaultOverflowPolicy();

    // A functor (class with operator()) which records whether a the calculation
    // encountered an overflow condition.
    class RecordOverflowPolicy
    {
        bool fOverflow;
    public:
        RecordOverflowPolicy() : fOverflow(false)
        {
        }

        // Called when an overflow is detected
        void operator()()
        {
            fOverflow = true;
        }

        bool HasOverflowed() const
        {
            return fOverflow;
        }
    };

};
