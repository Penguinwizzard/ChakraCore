//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

namespace Js {

    struct SIMDUint8x16Operation
    {
        // following are operation wrappers for SIMDUint8x16 general implementation
        // input and output are typically SIMDValue
        static SIMDValue OpUint8x16(uint8 x0, uint8 x1, uint8 x2, uint8 x3, uint8 x4, uint8 x5, 
            uint8 x6, uint8 x7, uint8 x8, uint8 x9, uint8 x10, uint8 x11, uint8 x12, uint8 x13, uint8 x14, uint8 x15);

        //// Unary Ops
        static SIMDValue OpMul(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpMin(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpMax(const SIMDValue& aValue, const SIMDValue& bValue);

        static SIMDValue OpLessThan(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpLessThanOrEqual(const SIMDValue& aValue, const SIMDValue& bValue);

        static SIMDValue OpShiftRightByScalar(const SIMDValue& value, int count);

        static SIMDValue OpAddSaturate(const SIMDValue& aValue, const SIMDValue& bValue);
        static SIMDValue OpSubSaturate(const SIMDValue& aValue, const SIMDValue& bValue);
    };

} // namespace Js
