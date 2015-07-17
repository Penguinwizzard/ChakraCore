//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------- 

#include "StdAfx.h"

namespace Js {
    int32 SIMDCheckTypedArrayIndex(ScriptContext* scriptContext, Var index)
    {
        int32 int32Value;
        Assert(index != NULL);

        int32Value = SIMDCheckInt32Number(scriptContext, index);
        return int32Value;
    }

    int32 SIMDCheckLaneIndex(ScriptContext* scriptContext, Var lane, const int32 range)
    {
        int32 int32Value;
        Assert(lane != NULL);

        int32Value = SIMDCheckInt32Number(scriptContext, lane);

        if (int32Value < 0 || int32Value >= range)
        {
            JavascriptError::ThrowRangeError(scriptContext, JSERR_SimdLaneRangeError);
        }
        
        return int32Value;
    }
    
    // Is Number with int32 value.
    int32 SIMDCheckInt32Number(ScriptContext* scriptContext, Var value)
    {
        int32 int32Value;
        
        if (JavascriptNumber::Is(value))
        {
            if (!JavascriptNumber::TryGetInt32Value(JavascriptNumber::GetValue(value), &int32Value))
            {
                JavascriptError::ThrowRangeError(scriptContext, JSERR_ArgumentOutOfRange);
            }
        }
        else if (TaggedInt::Is(value))
        {
            int32Value = TaggedInt::ToInt32(value);
        }
        else
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedNumber, L"index");
        }
        return int32Value;
    }

    template <int laneCount> SIMDValue SIMD128InnerShuffle(SIMDValue src1, SIMDValue src2, int32 lane0, int32 lane1, int32 lane2, int32 lane3)
    {
        SIMDValue result;
        CompileAssert(laneCount == 4 || laneCount == 2);
        if (laneCount == 4)
        {
            result.i32[SIMD_X] = lane0 < 4 ? src1.i32[lane0] : src2.i32[lane0 - 4];
            result.i32[SIMD_Y] = lane1 < 4 ? src1.i32[lane1] : src2.i32[lane1 - 4];
            result.i32[SIMD_Z] = lane2 < 4 ? src1.i32[lane2] : src2.i32[lane2 - 4];
            result.i32[SIMD_W] = lane3 < 4 ? src1.i32[lane3] : src2.i32[lane3 - 4];
        }
        else
        {
            result.f64[SIMD_X] = lane0 < 2 ? src1.f64[lane0] : src2.f64[lane0 - 2];
            result.f64[SIMD_Y] = lane1 < 2 ? src1.f64[lane1] : src2.f64[lane1 - 2];
        }
        return result;
    }

    template <class SIMDType, int laneCount> Var SIMD128SlowShuffle(Var src1, Var src2, Var lane0, Var lane1, Var lane2, Var lane3, int range, ScriptContext* scriptContext)
    {
        SIMDType *a = SIMDType::FromVar(src1);
        SIMDType *b = SIMDType::FromVar(src2);
        Assert(a);
        Assert(b);

        int32 lane0Value = 0;
        int32 lane1Value = 0;
        int32 lane2Value = 0; 
        int32 lane3Value = 0; 

        SIMDValue src1Value = a->GetValue();
        SIMDValue src2Value = b->GetValue();
        SIMDValue result;
        
        CompileAssert(laneCount == 4 || laneCount == 2);

        if (laneCount == 4)
        {
            lane0Value = SIMDCheckLaneIndex(scriptContext, lane0, range);
            lane1Value = SIMDCheckLaneIndex(scriptContext, lane1, range);
            lane2Value = SIMDCheckLaneIndex(scriptContext, lane2, range);
            lane3Value = SIMDCheckLaneIndex(scriptContext, lane3, range);

            Assert(lane0Value >= 0 && lane0Value < range);
            Assert(lane1Value >= 0 && lane1Value < range);
            Assert(lane2Value >= 0 && lane2Value < range);
            Assert(lane3Value >= 0 && lane3Value < range);

            result = SIMD128InnerShuffle(src1Value, src2Value, lane0Value, lane1Value, lane2Value, lane3Value);
        }
        else
        {
            lane0Value = SIMDCheckLaneIndex(scriptContext, lane0, range);
            lane1Value = SIMDCheckLaneIndex(scriptContext, lane1, range);

            Assert(lane0Value >= 0 && lane0Value < range);
            Assert(lane1Value >= 0 && lane1Value < range);

            result = SIMD128InnerShuffle<2>(src1Value, src2Value, lane0Value, lane1Value, lane2Value, lane3Value);
        }

        return SIMDType::New(&result, scriptContext);
    }

    template Var SIMD128SlowShuffle<JavascriptSIMDInt32x4, 4>   (Var src1, Var src2, Var lane0, Var lane1, Var lane2, Var lane3, int range, ScriptContext* scriptContext);
    template Var SIMD128SlowShuffle<JavascriptSIMDFloat32x4, 4> (Var src1, Var src2, Var lane0, Var lane1, Var lane2, Var lane3, int range, ScriptContext* scriptContext);
    template Var SIMD128SlowShuffle<JavascriptSIMDFloat64x2, 2> (Var src1, Var src2, Var lane0, Var lane1, Var lane2, Var lane3, int range, ScriptContext* scriptContext);



    bool SIMDIsSupportedTypedArray(Var value)
    {
        return JavascriptOperators::GetTypeId(value) >= TypeIds_Int8Array && JavascriptOperators::GetTypeId(value) <= TypeIds_Float64Array;
    }

    /*
    Checks if:
    1. Array is supported typed array
    2. Lane index is a Number/TaggedInt and int32 value
    3. Lane index is within array bounds
    */
    
    SIMDValue* SIMDCheckTypedArrayAccess(Var arg1, Var arg2, TypedArrayBase **tarray, int32 *index, uint32 dataWidth, ScriptContext *scriptContext)
    {
        if (!SIMDIsSupportedTypedArray(arg1))
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_SimdInvalidArgType, L"Simd typed array access");
        }
        *index = SIMDCheckInt32Number(scriptContext, arg2);

        // bound check
        *tarray = TypedArrayBase::FromVar(arg1);
        uint32 bpe = (*tarray)->GetBytesPerElement();
        uint32 offset = (*index) * bpe;
        if (index < 0 || (offset + dataWidth) > (*tarray)->GetByteLength())
        {
            JavascriptError::ThrowRangeError(scriptContext, JSERR_ArgumentOutOfRange, L"Simd typed array access");
        }
        return (SIMDValue*)((*tarray)->GetByteBuffer() + offset);
    }
    
    SIMDValue SIMDLdData(SIMDValue *data, uint8 dataWidth)
    {
        SIMDValue result = { 0, 0, 0, 0 };
        // bitwise copy. Always use integer fields to avoid wrong copy of NaNs.
        switch (dataWidth)
        {
        case 16:
            result.i32[SIMD_W] = data->i32[SIMD_W];
            // fall through
        case 12:
            result.i32[SIMD_Z] = data->i32[SIMD_Z];
            // fall through
        case 8:
            result.i32[SIMD_Y] = data->i32[SIMD_Y];
            // fall through
        case 4:
            result.i32[SIMD_X] = data->i32[SIMD_X];
            break;
        default:
            Assert(UNREACHED);
        }
        return result;
    }

    void SIMDStData(SIMDValue *data, SIMDValue simdValue, uint8 dataWidth)
    {
        // bitwise copy. Always use integer fields to avoid wrong copy of NaNs.
        switch (dataWidth)
        {
        case 16:
            data->i32[SIMD_W] = simdValue.i32[SIMD_W];
            // fall through
        case 12:
            data->i32[SIMD_Z] = simdValue.i32[SIMD_Z];
            // fall through
        case 8:
            data->i32[SIMD_Y] = simdValue.i32[SIMD_Y];
            // fall through
        case 4:
            data->i32[SIMD_X] = simdValue.i32[SIMD_X];
            break;
        default:
            Assert(UNREACHED);
        }
        
    }

    template <class SIMDType>
    Var SIMD128TypedArrayLoad(Var arg1, Var arg2, uint32 dataWidth, ScriptContext *scriptContext)
    {
        Assert(dataWidth >= 4 && dataWidth <= 16);

        TypedArrayBase *tarray = NULL;
        int32 index = -1;
        SIMDValue* data = NULL;
        
        data = SIMDCheckTypedArrayAccess(arg1, arg2, &tarray, &index, dataWidth, scriptContext);

        Assert(tarray != NULL);
        Assert(index >= 0);
        Assert(data != NULL);
        
        SIMDValue result = SIMDLdData(data, (uint8)dataWidth);

        return SIMDType::New(&result, scriptContext);

    }

    template Var SIMD128TypedArrayLoad<JavascriptSIMDFloat32x4>(Var arg1, Var arg2, uint32 dataWidth, ScriptContext *scriptContext);
    template Var SIMD128TypedArrayLoad<JavascriptSIMDInt32x4>(Var arg1, Var arg2, uint32 dataWidth, ScriptContext *scriptContext);
    template Var SIMD128TypedArrayLoad<JavascriptSIMDFloat64x2>(Var arg1, Var arg2, uint32 dataWidth, ScriptContext *scriptContext);

    template <class SIMDType>
    void SIMD128TypedArrayStore(Var arg1, Var arg2, Var simdVar, uint32 dataWidth, ScriptContext *scriptContext)
    {
        Assert(dataWidth >= 4 && dataWidth <= 16);

        TypedArrayBase *tarray = NULL;
        int32 index = -1;
        SIMDValue* data = NULL;

        data = SIMDCheckTypedArrayAccess(arg1, arg2, &tarray, &index, dataWidth, scriptContext);

        Assert(tarray != NULL);
        Assert(index >= 0);
        Assert(data != NULL);

        SIMDValue simdValue = SIMDType::FromVar(simdVar)->GetValue();
        SIMDStData(data, simdValue, (uint8)dataWidth);
    }

    template void SIMD128TypedArrayStore<JavascriptSIMDFloat32x4>(Var arg1, Var arg2, Var simdVar, uint32 dataWidth, ScriptContext *scriptContext);
    template void SIMD128TypedArrayStore<JavascriptSIMDInt32x4>(Var arg1, Var arg2, Var simdVar, uint32 dataWidth, ScriptContext *scriptContext);
    template void SIMD128TypedArrayStore<JavascriptSIMDFloat64x2>(Var arg1, Var arg2, Var simdVar, uint32 dataWidth, ScriptContext *scriptContext);

}
