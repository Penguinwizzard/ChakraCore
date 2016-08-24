//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "WasmReaderPch.h"

#if defined(ENABLE_WASM) && defined(ENABLE_NATIVE_CODEGEN)

namespace Wasm
{

void
WasmEncoder::EmptyAsm(Js::OpCodeAsmJs op)
{
    Assert(op == Js::OpCodeAsmJs::Ret);
}

void
WasmEncoder::AsmInt1Const1(Js::OpCodeAsmJs op, Js::RegSlot R0, int C1)
{
    // TODO:
}

void
WasmEncoder::AsmReg1(Js::OpCodeAsmJs op, Js::RegSlot R0)
{
    Assert(UNREACHED);
}

void
WasmEncoder::AsmReg2(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1)
{
    switch (op)
    {
#define DEF3_WMS_R2(x, op, func, y, t0, t1) WASM_ENCODE_REG2(op, func, t0, t1, R0, R1)

#include "Language/AsmJsEncoderHandler.inl"

    default:
        Assume(UNREACHED);
    }
}

void
WasmEncoder::AsmReg3(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2)
{
    switch (op)
    {
#define DEF3_WMS_R3(x, op, func, y, t0, t1, t2) WASM_ENCODE_REG3(op, func, t0, t1, t2, R0, R1, R2)
#include "Language/AsmJsEncoderHandler.inl"
    default:
        Assume(UNREACHED);
    }
}

void
WasmEncoder::AsmReg3(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2)
{
    switch (op)
    {
#define DEF3_WMS_R3(x, op, func, y, t0, t1, t2) WASM_ENCODE_REG3(op, func, t0, t1, t2, R0, R1, R2)
#include "Language/AsmJsEncoderHandler.inl"
    default:
        Assume(UNREACHED);
    }
}

template<>
JsUtil::GrowingArray<int, ArenaAllocator> const *
WasmEncoder::GetStackOffsetArray<int32>() const
{
    return &m_i32StackOffsets;
}

template<>
JsUtil::GrowingArray<int, ArenaAllocator> const *
WasmEncoder::GetStackOffsetArray<float>() const
{
    return &m_f32StackOffsets;
}

template<>
JsUtil::GrowingArray<int, ArenaAllocator> const *
WasmEncoder::GetStackOffsetArray<double>() const
{
    return &m_f64StackOffsets;
}

template<typename T>
int
WasmEncoder::CalculateOffset(Js::RegSlot location)
{
    JsUtil::GrowingArray<int, ArenaAllocator> const * arr = GetStackOffsetArray<T>();
    if (location > arr->Count())
    {
        m_stackOffsetCounter = Math::Align<int>(m_stackOffsetCounter, sizeof(T));
        arr->SetCount(location + 1);
        arr->GetBuffer()[location] = m_stackOffsetCounter;
        m_stackOffsetCounter += sizeof(T);
    }
    return arr->GetBuffer()[location];
}

} // namespace Wasm

#endif // defined(ENABLE_WASM) && defined(ENABLE_NATIVE_CODEGEN)
