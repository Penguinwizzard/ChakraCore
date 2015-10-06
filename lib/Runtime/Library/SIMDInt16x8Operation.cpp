//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------- 

#include "RuntimeLibraryPch.h"
#include "SIMDInt16x8Operation.h"

#if defined(_M_ARM32_OR_ARM64)
#ifdef SIMD_JS_ENABLED

namespace Js
{
    SIMDValue SIMDInt16x8Operation::OpInt16x8(int16 x0, int16 x1, int16 x2, int16 x3, int16 x4, int16 x5, int16 x6, int16 x7)
    {
        SIMDValue result;

        result.i16[0] = x0;
        result.i16[1] = x1;
        result.i16[2] = x2;
        result.i16[3] = x3;
        result.i16[4] = x4;
        result.i16[5] = x5;
        result.i16[6] = x6;
        result.i16[7] = x7;

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpInt16x8(const SIMDValue& v)
    {// overload function with input paramter as SIMDValue for completeness, may not need
        SIMDValue result = v;

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpSplat(short x)
    {
        SIMDValue result;

        result.i16[0] = result.i16[1] = result.i16[2] = result.i16[3] = x;
        result.i16[4] = result.i16[5] = result.i16[6] = result.i16[7] = x;

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpSplat(const SIMDValue& v)
    {
        SIMDValue result;

        result.i16[0] = result.i16[1] = result.i16[2] = result.i16[3] = v.i16[0];
        result.i16[4] = result.i16[5] = result.i16[6] = result.i16[7] = v.i16[0];

        return result;
    }

    // Conversions
    SIMDValue SIMDInt16x8Operation::OpFromInt32x4Bits(const SIMDValue& v)
    {
        SIMDValue result;

        result.i32[0] = v.i32[0];
        result.i32[1] = v.i32[1];
        result.i32[2] = v.i32[2];
        result.i32[3] = v.i32[3];

        return result;
    }


    SIMDValue SIMDInt16x8Operation::OpFromFloat32x4Bits(const SIMDValue& v)
    {
        SIMDValue result;

        result.f64[0] = v.f64[0];
        result.f64[1] = v.f64[1];

        return result;
    }

    // Unary Ops
    SIMDValue SIMDInt16x8Operation::OpNeg(const SIMDValue& value)
    {
        SIMDValue result;

        result.i16[0] = -1 * value.i16[0];
        result.i16[1] = -1 * value.i16[1];
        result.i16[2] = -1 * value.i16[2];
        result.i16[3] = -1 * value.i16[3];
        result.i16[4] = -1 * value.i16[4];
        result.i16[5] = -1 * value.i16[5];
        result.i16[6] = -1 * value.i16[6];
        result.i16[7] = -1 * value.i16[7];

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpNot(const SIMDValue& value)
    {
        SIMDValue result;

        result.i16[0] = ~(value.i16[0]);
        result.i16[1] = ~(value.i16[1]);
        result.i16[2] = ~(value.i16[2]);
        result.i16[3] = ~(value.i16[3]);
        result.i16[4] = ~(value.i16[4]);
        result.i16[5] = ~(value.i16[5]);
        result.i16[6] = ~(value.i16[6]);
        result.i16[7] = ~(value.i16[7]);

        return result;
    }

    // Binary Ops 
    SIMDValue SIMDInt16x8Operation::OpAdd(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i16[0] = aValue.i16[0] + bValue.i16[0];
        result.i16[1] = aValue.i16[1] + bValue.i16[1];
        result.i16[2] = aValue.i16[2] + bValue.i16[2];
        result.i16[3] = aValue.i16[3] + bValue.i16[3];
        result.i16[4] = aValue.i16[4] + bValue.i16[4];
        result.i16[5] = aValue.i16[5] + bValue.i16[5];
        result.i16[6] = aValue.i16[6] + bValue.i16[6];
        result.i16[7] = aValue.i16[7] + bValue.i16[7];

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpSub(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i16[0] = aValue.i16[0] - bValue.i16[0];
        result.i16[1] = aValue.i16[1] - bValue.i16[1];
        result.i16[2] = aValue.i16[2] - bValue.i16[2];
        result.i16[3] = aValue.i16[3] - bValue.i16[3];
        result.i16[4] = aValue.i16[4] - bValue.i16[4];
        result.i16[5] = aValue.i16[5] - bValue.i16[5];
        result.i16[6] = aValue.i16[6] - bValue.i16[6];
        result.i16[7] = aValue.i16[7] - bValue.i16[7];

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpMul(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i16[0] = aValue.i16[0] * bValue.i16[0];
        result.i16[1] = aValue.i16[1] * bValue.i16[1];
        result.i16[2] = aValue.i16[2] * bValue.i16[2];
        result.i16[3] = aValue.i16[3] * bValue.i16[3];
        result.i16[4] = aValue.i16[4] * bValue.i16[4];
        result.i16[5] = aValue.i16[5] * bValue.i16[5];
        result.i16[6] = aValue.i16[6] * bValue.i16[6];
        result.i16[7] = aValue.i16[7] * bValue.i16[7];

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpAnd(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i16[0] = aValue.i16[0] & bValue.i16[0];
        result.i16[1] = aValue.i16[1] & bValue.i16[1];
        result.i16[2] = aValue.i16[2] & bValue.i16[2];
        result.i16[3] = aValue.i16[3] & bValue.i16[3];
        result.i16[4] = aValue.i16[4] & bValue.i16[4];
        result.i16[5] = aValue.i16[5] & bValue.i16[5];
        result.i16[6] = aValue.i16[6] & bValue.i16[6];
        result.i16[7] = aValue.i16[7] & bValue.i16[7];

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpOr(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i16[0] = aValue.i16[0] | bValue.i16[0];
        result.i16[1] = aValue.i16[1] | bValue.i16[1];
        result.i16[2] = aValue.i16[2] | bValue.i16[2];
        result.i16[3] = aValue.i16[3] | bValue.i16[3];
        result.i16[4] = aValue.i16[4] | bValue.i16[4];
        result.i16[5] = aValue.i16[5] | bValue.i16[5];
        result.i16[6] = aValue.i16[6] | bValue.i16[6];
        result.i16[7] = aValue.i16[7] | bValue.i16[7];

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpXor(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        result.i16[0] = aValue.i16[0] ^ bValue.i16[0];
        result.i16[1] = aValue.i16[1] ^ bValue.i16[1];
        result.i16[2] = aValue.i16[2] ^ bValue.i16[2];
        result.i16[3] = aValue.i16[3] ^ bValue.i16[3];
        result.i16[4] = aValue.i16[4] ^ bValue.i16[4];
        result.i16[5] = aValue.i16[5] ^ bValue.i16[5];
        result.i16[6] = aValue.i16[6] ^ bValue.i16[6];
        result.i16[7] = aValue.i16[7] ^ bValue.i16[7];

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpAddSaturate(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;
        for (uint idx = 0; idx < 8; ++idx)
        {
            int16 val1 = aValue.i16[idx];
            int16 val2 = bValue.i16[idx];
            int16 sum = val1 + val2;

            result.i16[idx] = sum;
            if (val1 > 0 && val2 > 0 && sum < 0)
                result.i16[idx] = 0x7fff;
            else if (val1 < 0 && val2 < 0 && sum > 0)
                result.i16[idx] = result.i16[idx] & 0x8000;
        }
        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpSubSaturate(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;
        for (uint idx = 0; idx < 8; ++idx)
        {
            int16 val1 = aValue.i16[idx];
            int16 val2 = bValue.i16[idx];
            int16 diff = val1 + val2;

            result.i16[idx] = static_cast<int16>(diff);
            if (diff > 0x7fff)
                result.i16[idx] = 0x7fff;
            else if (diff < 0x8000)
                result.i16[idx] = result.i16[idx]  & 0x8000;
        }
        return result;
    }

	SIMDValue SIMDInt16x8Operation::OpMin(const SIMDValue& aValue, const SIMDValue& bValue)
	{
		SIMDValue result;
		for (int idx = 0; idx < 8; ++idx)
		{
			result.i16[idx] = (aValue.i16[idx] < bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
		}
		return result;
	}

	SIMDValue SIMDInt16x8Operation::OpMax(const SIMDValue& aValue, const SIMDValue& bValue)
	{
		SIMDValue result;

		for (int idx = 0; idx < 8; ++idx)
		{
			result.i16[idx] = (aValue.i16[idx] > bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
		}

		return result;
	}

    // compare ops
    SIMDValue SIMDInt16x8Operation::OpLessThan(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        for (uint idx = 0; idx < 8; ++idx)
        {
            result.i16[idx] = (aValue.i16[idx] < bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
        }

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpLessThanOrEqual(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        for (uint idx = 0; idx < 8; ++idx)
        {
            result.i16[idx] = (aValue.i16[idx] <= bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
        }

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpEqual(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        for (uint idx = 0; idx < 8; ++idx)
        {
            result.i16[idx] = (aValue.i16[idx] == bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
        }

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpNotEqual(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        for (uint idx = 0; idx < 8; ++idx)
        {
            result.i16[idx] = (aValue.i16[idx] != bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
        }

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpGreaterThan(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        for (uint idx = 0; idx < 8; ++idx)
        {
            result.i16[idx] = (aValue.i16[idx] > bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
        }

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpGreaterThanOrEqual(const SIMDValue& aValue, const SIMDValue& bValue)
    {
        SIMDValue result;

        for (uint idx = 0; idx < 8; ++idx)
        {
            result.i16[idx] = (aValue.i16[idx] >= bValue.i16[idx]) ? aValue.i16[idx] : bValue.i16[idx];
        }

        return result;
    }

    // Lane Access
    short SIMDInt16x8Operation::OpExtractLane(const SIMDValue& aValue, int index)
    {
        Assert(index >= 0 && index < 8);
        int16 result = 0;
        if (index >= 0 && index < 8)
        {
            result = aValue.i16[index];
        }
        else
        {
            Assert(FALSE);
        }
        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpReplaceLane(const SIMDValue& aValue, int index, short newValue)
    {
        SIMDValue result = aValue;

        Assert(index >= 0 && index < 8);

        if (index >= 0 && index < 8)
        {
            result.i16[index] = newValue;
            return result;
        }
        else
        {
            Assert(FALSE);

        }
        return result;

    }


    // ShiftOps
    SIMDValue SIMDInt16x8Operation::OpShiftLeftByScalar(const SIMDValue& value, int count)
    {
        SIMDValue result;

        if (count < 0 || count > 16)
        {  // if Bit Count is larger than 16, assume it to be 16 according to polyfill
            count = 16;
        }

        result.i16[0] = value.i16[0] << count;
        result.i16[1] = value.i16[1] << count;
        result.i16[2] = value.i16[2] << count;
        result.i16[3] = value.i16[3] << count;
        result.i16[4] = value.i16[4] << count;
        result.i16[5] = value.i16[5] << count;
        result.i16[6] = value.i16[6] << count;
        result.i16[7] = value.i16[7] << count;

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpShiftRightLogicalByScalar(const SIMDValue& value, int count)
    {
        SIMDValue result;

        if (count < 0 || count > 16)
        {  
            count = 16;
        }

        int nIntMin = INT_MIN; // INT_MIN = -2147483648 = 0x80000000
        int mask = ~((nIntMin >> count) << 1); // now first count bits are 0
        // right shift count bits and shift in with 0
        result.i16[0] = (value.i16[0] >> count) & mask;
        result.i16[1] = (value.i16[1] >> count) & mask;
        result.i16[2] = (value.i16[2] >> count) & mask;
        result.i16[3] = (value.i16[3] >> count) & mask;
        result.i16[4] = (value.i16[4] >> count) & mask;
        result.i16[5] = (value.i16[5] >> count) & mask;
        result.i16[6] = (value.i16[6] >> count) & mask;
        result.i16[7] = (value.i16[7] >> count) & mask;

        return result;
    }

    SIMDValue SIMDInt16x8Operation::OpShiftRightArithmeticByScalar(const SIMDValue& value, int count)
    {
        SIMDValue result;

        if (count < 0 || count > 16)
        {  // if Bit Count is larger than 16, assume it to be 16 according to polyfill
            count = 16;
        }

        result.i16[0] = value.i16[0] >> count;
        result.i16[1] = value.i16[1] >> count;
        result.i16[2] = value.i16[2] >> count;
        result.i16[3] = value.i16[3] >> count;
        result.i16[4] = value.i16[4] >> count;
        result.i16[5] = value.i16[5] >> count;
        result.i16[6] = value.i16[6] >> count;
        result.i16[7] = value.i16[7] >> count;

        return result;
    }

    // load&store
    //static SIMDValue OpLoad(int* tarray, const int index)
    //{
    //    SIMDValue result;

    //    /* todo: Need implement this 
    //    // private stuff.
    //    // Temporary buffers for swizzles and bitcasts.
    //    var _f32x4 = new Float32Array(4);
    //    var _f64x2 = new Float64Array(_f32x4.buffer);
    //    var _i32x4 = new Int32Array(_f32x4.buffer);
    //    var _i16x8 = new Int16Array(_f32x4.buffer);
    //    var _i8x16 = new Int8Array(_f32x4.buffer);

    //    // @param {Typed array} tarray An instance of a typed array.
    //    // @param {Number} index An instance of Number.
    //    // @return {Int16x8} New instance of Int16x8.
    //    SIMD.Int16x8.load = function(tarray, index) {
    //        if (!isTypedArray(tarray))
    //            throw new TypeError("The 1st argument must be a typed array.");
    //        if (!isInt32(index))
    //            throw new TypeError("The 2nd argument must be an Int32.");
    //        var bpe = tarray.BYTES_PER_ELEMENT;
    //        if (index < 0 || (index * bpe + 16) > tarray.byteLength)
    //            throw new RangeError("The value of index is invalid.");
    //        var i16temp = _i16x8;
    //        var array = bpe == 1 ? _i8x16 :
    //            bpe == 2 ? i16temp :
    //            bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
    //            _f64x2;
    //        var n = 16 / bpe;
    //        for (var i = 0; i < n; ++i)
    //            array[i] = tarray[index + i];
    //        return SIMD.Int16x8(i16temp[0], i16temp[1], i16temp[2], i16temp[3],
    //            i16temp[4], i16temp[5], i16temp[6], i16temp[7]);
    //    }
    //    */

    //    return result;
    //}

    //static SIMDValue OpStore(const SIMDValue& v, const int maskValue)
    //{
    //    SIMDValue result;

    //    /*
    //    // @param {Typed array} tarray An instance of a typed array.
    //    // @param {Number} index An instance of Number.
    //    // @return {Int16x8} New instance of Int16x8.
    //    SIMD.Int16x8.load = function(tarray, index) {
    //        if (!isTypedArray(tarray))
    //            throw new TypeError("The 1st argument must be a typed array.");
    //        if (!isInt32(index))
    //            throw new TypeError("The 2nd argument must be an Int32.");
    //        var bpe = tarray.BYTES_PER_ELEMENT;
    //        if (index < 0 || (index * bpe + 16) > tarray.byteLength)
    //            throw new RangeError("The value of index is invalid.");
    //        var i16temp = _i16x8;
    //        var array = bpe == 1 ? _i8x16 :
    //            bpe == 2 ? i16temp :
    //            bpe == 4 ? (tarray instanceof Float32Array ? _f32x4 : _i32x4) :
    //            _f64x2;
    //        var n = 16 / bpe;
    //        for (var i = 0; i < n; ++i)
    //            array[i] = tarray[index + i];
    //        return SIMD.Int16x8(i16temp[0], i16temp[1], i16temp[2], i16temp[3],
    //            i16temp[4], i16temp[5], i16temp[6], i16temp[7]);
    //    }
    //    */

    //    return result;
    //}
    
    // others
   
}

#endif
#endif