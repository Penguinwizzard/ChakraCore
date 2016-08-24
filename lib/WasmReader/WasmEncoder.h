//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

namespace Wasm
{
class WasmEncoder : Js::AsmJsByteCodeWriter
{

public:
    virtual void InitData(ArenaAllocator* alloc, long initCodeBufferSize) override;
    virtual void EmptyAsm(Js::OpCodeAsmJs op) override;
    virtual void AsmInt1Const1(Js::OpCodeAsmJs op, Js::RegSlot R0, int C1) override;
    virtual void AsmFloat1Const1(Js::OpCodeAsmJs op, Js::RegSlot R0, float C1) override;
    virtual void AsmDouble1Const1(Js::OpCodeAsmJs op, Js::RegSlot R0, double C1) override;
    virtual void AsmReg1(Js::OpCodeAsmJs op, Js::RegSlot R0) override;
    virtual void AsmReg2(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1) override;
    virtual void AsmReg3(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2) override;
    virtual void AsmReg4(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3) override;
    virtual void AsmReg5(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4) override;
    virtual void AsmReg6(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5) override;
    virtual void AsmReg7(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5, Js::RegSlot R6) override;
    virtual void AsmReg9(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5, Js::RegSlot R6, Js::RegSlot R7, Js::RegSlot R8) override;
    virtual void AsmReg10(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5, Js::RegSlot R6, Js::RegSlot R7, Js::RegSlot R8, Js::RegSlot R9) override;
    virtual void AsmReg11(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5, Js::RegSlot R6, Js::RegSlot R7, Js::RegSlot R8, Js::RegSlot R9, Js::RegSlot R10) override;
    virtual void AsmReg17(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5, Js::RegSlot R6, Js::RegSlot R7, Js::RegSlot R8,
        Js::RegSlot R9, Js::RegSlot R10, Js::RegSlot R11, Js::RegSlot R12, Js::RegSlot R13, Js::RegSlot R14, Js::RegSlot R15, Js::RegSlot R16) override;
    virtual void AsmReg18(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5, Js::RegSlot R6, Js::RegSlot R7, Js::RegSlot R8,
        Js::RegSlot R9, Js::RegSlot R10, Js::RegSlot R11, Js::RegSlot R12, Js::RegSlot R13, Js::RegSlot R14, Js::RegSlot R15, Js::RegSlot R16, Js::RegSlot R17) override;
    virtual void AsmReg19(Js::OpCodeAsmJs op, Js::RegSlot R0, Js::RegSlot R1, Js::RegSlot R2, Js::RegSlot R3, Js::RegSlot R4, Js::RegSlot R5, Js::RegSlot R6, Js::RegSlot R7, Js::RegSlot R8,
        Js::RegSlot R9, Js::RegSlot R10, Js::RegSlot R11, Js::RegSlot R12, Js::RegSlot R13, Js::RegSlot R14, Js::RegSlot R15, Js::RegSlot R16, Js::RegSlot R17, Js::RegSlot R18) override;
    virtual void AsmBr(Js::ByteCodeLabel labelID, Js::OpCodeAsmJs op = Js::OpCodeAsmJs::AsmBr) override;
    virtual void AsmBrReg1(Js::OpCodeAsmJs op, Js::ByteCodeLabel labelID, Js::RegSlot R1) override;
    virtual void AsmBrReg2(Js::OpCodeAsmJs op, Js::ByteCodeLabel labelID, Js::RegSlot R1, Js::RegSlot R2) override;
    virtual void AsmStartCall(Js::OpCodeAsmJs op, Js::ArgSlot ArgCount, bool isPatching = false) override;
    virtual void AsmCall(Js::OpCodeAsmJs op, Js::RegSlot returnValueRegister, Js::RegSlot functionRegister, Js::ArgSlot givenArgCount, Js::AsmJsRetType retType) override;
    virtual void AsmSlot(Js::OpCodeAsmJs op, Js::RegSlot value, Js::RegSlot instance, int32 slotId) override;
    virtual void AsmTypedArr(Js::OpCodeAsmJs op, Js::RegSlot value, uint32 slotIndex, Js::ArrayBufferView::ViewType viewType) override;
    virtual void AsmSimdTypedArr(Js::OpCodeAsmJs op, Js::RegSlot value, uint32 slotIndex, uint8 dataWidth, Js::ArrayBufferView::ViewType viewType) override;

    virtual void MarkAsmJsLabel(Js::ByteCodeLabel labelID) override;
    virtual void AsmJsUnsigned1(Js::OpCodeAsmJs op, uint C1) override;
    virtual uint EnterLoop(Js::ByteCodeLabel loopEntrance) override;
    virtual void ExitLoop(uint loopId) override;

private:
    template<typename T> int CalculateOffset(Js::RegSlot location);
    template<typename T> JsUtil::GrowingArray<int, ArenaAllocator> const * GetStackOffsetArray() const;

    JsUtil::GrowingArray<int, ArenaAllocator> m_i32StackOffsets;
    JsUtil::GrowingArray<int, ArenaAllocator> m_f32StackOffsets;
    JsUtil::GrowingArray<int, ArenaAllocator> m_f64StackOffsets;

    int m_stackOffsetCounter;
};
} // namespace Wasm

#define WASM_ENCODE_REG2(name, func, t0, t1, r0, r1) \
    case Js::OpCodeAsmJs::name: \
    { \
        Js::AsmJsJitTemplate::func::ApplyTemplate(m_encoder, m_pc, CalculateOffset<t0>(r0), CalculateOffset<t1>(r1)); \
        break; \
    }

#define WASM_ENCODE_REG3(name, func, t0, t1, t2, r0, r1, r2) \
    case Js::OpCodeAsmJs::name: \
    { \
        Js::AsmJsJitTemplate::func::ApplyTemplate(m_encoder, m_pc, CalculateOffset<t0>(r0), CalculateOffset<t1>(r1), CalculateOffset<t2>(r2)); \
        break; \
    }
