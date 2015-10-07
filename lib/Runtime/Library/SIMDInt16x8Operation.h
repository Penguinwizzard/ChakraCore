//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#ifdef SIMD_JS_ENABLED

namespace Js {

    struct SIMDInt16x8Operation
    {
        // following are operation wrappers for SIMDInt16x8 general implementation
        // input and output are typically SIMDValue
        static SIMDValue OpInt16x8(short x0, short x1, short x2, short x3, short x4, short x5, short x6, short x7);
        static SIMDValue OpInt16x8(const SIMDValue& v);
        
        static SIMDValue OpSplat(short x);
        static SIMDValue OpSplat(const SIMDValue& v);

        // conversion
        static SIMDValue OpFromInt32x4Bits(const SIMDValue& value); 
        static SIMDValue OpFromFloat32x4Bits(const SIMDValue& value);

        // Unary Ops
        static SIMDValue OpNeg(const SIMDValue& v);
        static SIMDValue OpNot(const SIMDValue& v);

        // Binary Ops 
        static SIMDValue OpAdd(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpSub(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpMul(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpAnd(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpOr (const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpXor(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpAddSaturate(const SIMDValue& aValue, const SIMDValue& bValue); 
        static SIMDValue OpSubSaturate(const SIMDValue& aValue, const SIMDValue& bValue); 
        static SIMDValue OpMin(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpMax(const SIMDValue& aValue, const SIMDValue& bValue);

        // CompareOps
        static SIMDValue OpLessThan(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpLessThanOrEqual(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpEqual(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpNotEqual(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpGreaterThan(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpGreaterThanOrEqual(const SIMDValue& aValue, const SIMDValue& bValue);

        // ShiftOps
        static SIMDValue OpShiftLeftByScalar(const SIMDValue& value, int count);
        static SIMDValue OpShiftRightLogicalByScalar(const SIMDValue& value, int count);
        static SIMDValue OpShiftRightArithmeticByScalar(const SIMDValue& value, int count);

        // load&store
        //static SIMDValue OpLoad(int* v, const int index); 
        //static SIMDValue OpStore(int* v, const int index); 

    };

} // namespace Js

#endif