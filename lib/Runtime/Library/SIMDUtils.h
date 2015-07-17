//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------- 
#pragma once

#ifdef SIMD_JS_ENABLED

#define SIMD_JS_FLAG Js::Configuration::Global.flags.Simdjs

// The representations below assume little-endian.
#define SIMD_X 0
#define SIMD_Y 1
#define SIMD_Z 2
#define SIMD_W 3

struct _SIMDValue
{
    union{
        int     i32[4];
        float   f32[4];
        double  f64[2];
    };

    void SetValue(_SIMDValue value)
    {
        i32[SIMD_X] = value.i32[SIMD_X];
        i32[SIMD_Y] = value.i32[SIMD_Y];
        i32[SIMD_Z] = value.i32[SIMD_Z];
        i32[SIMD_W] = value.i32[SIMD_W];
    }
    void Zero()
    {
        f64[SIMD_X] = f64[SIMD_Y] = 0;
    }
    bool operator==(const _SIMDValue& r)
    {
        // don't compare f64/f32 because NaN bit patterns will not be considered equal.
        return (this->i32[SIMD_X] == r.i32[SIMD_X] &&
            this->i32[SIMD_Y] == r.i32[SIMD_Y] &&
            this->i32[SIMD_Z] == r.i32[SIMD_Z] &&
            this->i32[SIMD_W] == r.i32[SIMD_W]);
    }
    bool IsZero()
    {
        return (i32[SIMD_X] == 0 && i32[SIMD_Y] == 0 && i32[SIMD_Z] == 0 && i32[SIMD_W] == 0);
    }

};
typedef _SIMDValue SIMDValue;

// For dictionary use
template <>
struct DefaultComparer<_SIMDValue>
{
    __forceinline static bool Equals(_SIMDValue x, _SIMDValue y)
    {
        return x == y;
    }

    __forceinline static hash_t GetHashCode(_SIMDValue d)
    {
        return (hash_t)(d.i32[SIMD_X] ^ d.i32[SIMD_Y] ^ d.i32[SIMD_Z] ^ d.i32[SIMD_W]);
    }
};

#if _M_IX86 || _M_AMD64
struct _x86_SIMDValue
{
    union{
        _SIMDValue simdValue;
        __m128  m128_value;
        __m128d m128d_value;
        __m128i m128i_value;
    };

    static _x86_SIMDValue ToX86SIMDValue(const SIMDValue& val)
    {
        _x86_SIMDValue result;
        result.simdValue.i32[SIMD_X] = val.i32[SIMD_X];
        result.simdValue.i32[SIMD_Y] = val.i32[SIMD_Y];
        result.simdValue.i32[SIMD_Z] = val.i32[SIMD_Z];
        result.simdValue.i32[SIMD_W] = val.i32[SIMD_W];
        return result;
    }

    static SIMDValue ToSIMDValue(const _x86_SIMDValue& val)
    {
        SIMDValue result;
        result.i32[SIMD_X] = val.simdValue.i32[SIMD_X];
        result.i32[SIMD_Y] = val.simdValue.i32[SIMD_Y];
        result.i32[SIMD_Z] = val.simdValue.i32[SIMD_Z];
        result.i32[SIMD_W] = val.simdValue.i32[SIMD_W];
        return result;
    }
};

// These global values are 16-byte aligned.
const _x86_SIMDValue X86_ABS_MASK_F4 = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const _x86_SIMDValue X86_ABS_MASK_I4 = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
const _x86_SIMDValue X86_ABS_MASK_D2 = { 0xffffffff, 0x7fffffff, 0xffffffff, 0x7fffffff };

const _x86_SIMDValue X86_NEG_MASK_F4 = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
const _x86_SIMDValue X86_NEG_MASK_D2 = { 0x00000000, 0x80000000, 0x00000000, 0x80000000 };

const _x86_SIMDValue X86_ALL_ONES_F4 = { 0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000 }; // {1.0, 1.0, 1.0, 1.0}
const _x86_SIMDValue X86_ALL_ONES_I4 = { 0x00000001, 0x00000001, 0x00000001, 0x00000001 }; // {1, 1, 1, 1}
const _x86_SIMDValue X86_ALL_ONES_D2 = { 0x00000000, 0x3ff00000, 0x00000000, 0x3ff00000 }; // {1.0, 1.0}

const _x86_SIMDValue X86_ALL_NEG_ONES = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff };

const _x86_SIMDValue X86_ALL_ZEROS    = { 0x00000000, 0x00000000, 0x00000000, 0x00000000 };
const _x86_SIMDValue X86_LANE_W_ZEROS = { 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 };

typedef _x86_SIMDValue X86SIMDValue;
CompileAssert(sizeof(X86SIMDValue) == 16);
#endif

typedef SIMDValue     AsmJsSIMDValue; // alias for asmjs
CompileAssert(sizeof(SIMDValue) == 16);


namespace Js {
    int32 SIMDCheckTypedArrayIndex(ScriptContext* scriptContext, Var index);
    int32 SIMDCheckLaneIndex(ScriptContext* scriptContext, Var lane, bool &fastShuffle, const int range = 4);
    
    template <int laneCount = 4>
    SIMDValue SIMD128InnerShuffle(SIMDValue src1, SIMDValue src2, int32 lane0, int32 lane1, int32 lane2, int32 lane3);

    template <class SIMDType, int laneCount = 4> 
    Var SIMD128SlowShuffle(Var src1, Var src2, Var lane0, Var lane1, Var lane2, Var lane3, int range, ScriptContext* scriptContext);
    

    int32 SIMDCheckInt32Number(ScriptContext* scriptContext, Var value);
    bool        SIMDIsSupportedTypedArray(Var value);
    SIMDValue*  SIMDCheckTypedArrayAccess(Var arg1, Var arg2, TypedArrayBase **tarray, int32 *index, uint8 dataWidth, ScriptContext *scriptContext);
    AsmJsSIMDValue SIMDLdData(AsmJsSIMDValue *data, uint8 dataWidth);
    void SIMDStData(AsmJsSIMDValue *data, AsmJsSIMDValue simdValue, uint8 dataWidth);
    template <class SIMDType>
    Var   SIMD128TypedArrayLoad(Var arg1, Var arg2, uint32 dataWidth, ScriptContext *scriptContext);
    template <class SIMDType> 
    void  SIMD128TypedArrayStore(Var arg1, Var arg2, Var simdVar, uint32 dataWidth, ScriptContext *scriptContext);
}

#endif // SIMD_JS_ENABLED
