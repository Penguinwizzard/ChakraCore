//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#ifndef TEMP_DISABLE_ASMJS
namespace Js
{
    class AsmJsByteCodeWriter : public ByteCodeWriter
    {
    private:
        using ByteCodeWriter::MarkLabel;

    public:
        virtual void InitData        ( ArenaAllocator* alloc, int32 initCodeBufferSize );
        virtual void EmptyAsm        ( OpCodeAsmJs op );
        virtual void Conv            ( OpCodeAsmJs op, RegSlot R0, RegSlot R1 );
        virtual void AsmInt1Const1   ( OpCodeAsmJs op, RegSlot R0, int C1 );
        virtual void AsmFloat1Const1 ( OpCodeAsmJs op, RegSlot R0, float C1 );
        virtual void AsmDouble1Const1( OpCodeAsmJs op, RegSlot R0, double C1 );
        virtual void AsmReg1         ( OpCodeAsmJs op, RegSlot R0 );
        virtual void AsmReg2         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1 );
        virtual void AsmReg3         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2 );
        virtual void AsmReg4         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3 );
        virtual void AsmReg5         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4 );
        virtual void AsmReg6         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5 );
        virtual void AsmReg7         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6 );
        virtual void AsmReg9         (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8);
        virtual void AsmReg10        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8, RegSlot R9);
        virtual void AsmReg11        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8, RegSlot R9, RegSlot R10);
        virtual void AsmReg17        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8,
                                              RegSlot R9, RegSlot R10, RegSlot R11, RegSlot R12, RegSlot R13, RegSlot R14, RegSlot R15, RegSlot R16);
        virtual void AsmReg18        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8,
                                              RegSlot R9, RegSlot R10, RegSlot R11, RegSlot R12, RegSlot R13, RegSlot R14, RegSlot R15, RegSlot R16, RegSlot R17);
        virtual void AsmReg19        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8,
                                              RegSlot R9, RegSlot R10, RegSlot R11, RegSlot R12, RegSlot R13, RegSlot R14, RegSlot R15, RegSlot R16, RegSlot R17, RegSlot R18);
        virtual void AsmBr           ( ByteCodeLabel labelID, OpCodeAsmJs op = OpCodeAsmJs::AsmBr );
        virtual void AsmBrReg1       ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1 );
        virtual void AsmBrReg2       ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, RegSlot R2 );
        virtual void AsmBrReg1Const1 ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, int C1 );
        virtual void AsmStartCall    ( OpCodeAsmJs op, ArgSlot ArgCount, bool isPatching = false);
        virtual void AsmCall         ( OpCodeAsmJs op, RegSlot returnValueRegister, RegSlot functionRegister, ArgSlot givenArgCount, AsmJsRetType retType );
        virtual void AsmSlot         ( OpCodeAsmJs op, RegSlot value, RegSlot instance, int32 slotId );
        virtual void AsmTypedArr     ( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, ArrayBufferView::ViewType viewType );
        virtual void AsmSimdTypedArr ( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, uint8 dataWidth, ArrayBufferView::ViewType viewType );

        virtual void MarkAsmJsLabel  ( ByteCodeLabel labelID );
        virtual void AsmJsUnsigned1  ( OpCodeAsmJs op, uint C1 );
        virtual uint EnterLoop       ( ByteCodeLabel loopEntrance );
        virtual void ExitLoop        ( uint loopId );

    private:
        template <typename SizePolicy> bool TryWriteAsmReg1         ( OpCodeAsmJs op, RegSlot R0 );
        template <typename SizePolicy> bool TryWriteAsmReg2         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1 );
        template <typename SizePolicy> bool TryWriteAsmReg3         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2 );
        template <typename SizePolicy> bool TryWriteAsmReg4         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3 );
        template <typename SizePolicy> bool TryWriteAsmReg5         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4 );
        template <typename SizePolicy> bool TryWriteAsmReg6         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5 );
        template <typename SizePolicy> bool TryWriteAsmReg7         ( OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6 );
        template <typename SizePolicy> bool TryWriteAsmReg9         (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8);
        template <typename SizePolicy> bool TryWriteAsmReg10        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8, RegSlot R9);
        template <typename SizePolicy> bool TryWriteAsmReg11        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8, 
                                                                                     RegSlot R9, RegSlot R10);
        template <typename SizePolicy> bool TryWriteAsmReg17        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8,
                                                                                     RegSlot R9, RegSlot R10, RegSlot R11, RegSlot R12, RegSlot R13, RegSlot R14, RegSlot R15, RegSlot R16);
        template <typename SizePolicy> bool TryWriteAsmReg18        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8,
                                                                                     RegSlot R9, RegSlot R10, RegSlot R11, RegSlot R12, RegSlot R13, RegSlot R14, RegSlot R15, RegSlot R16, RegSlot R17);
        template <typename SizePolicy> bool TryWriteAsmReg19        (OpCodeAsmJs op, RegSlot R0, RegSlot R1, RegSlot R2, RegSlot R3, RegSlot R4, RegSlot R5, RegSlot R6, RegSlot R7, RegSlot R8,
                                                                                     RegSlot R9, RegSlot R10, RegSlot R11, RegSlot R12, RegSlot R13, RegSlot R14, RegSlot R15, RegSlot R16, RegSlot R17, RegSlot R18);

        template <typename SizePolicy> bool TryWriteInt1Const1      ( OpCodeAsmJs op, RegSlot R0, int C1 );
        template <typename SizePolicy> bool TryWriteFloat1Const1    ( OpCodeAsmJs op, RegSlot R0, float C1 );
        template <typename SizePolicy> bool TryWriteDouble1Const1   ( OpCodeAsmJs op, RegSlot R0, double C1 );
        template <typename SizePolicy> bool TryWriteAsmBrReg1       ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1 );
        template <typename SizePolicy> bool TryWriteAsmBrReg2       ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, RegSlot R2 );
        template <typename SizePolicy> bool TryWriteAsmBrReg1Const1 ( OpCodeAsmJs op, ByteCodeLabel labelID, RegSlot R1, int C1 );
        template <typename SizePolicy> bool TryWriteAsmCall         ( OpCodeAsmJs op, RegSlot returnValueRegister, RegSlot functionRegister, ArgSlot givenArgCount, AsmJsRetType retType );
        template <typename SizePolicy> bool TryWriteAsmSlot         ( OpCodeAsmJs op, RegSlot value, RegSlot instance, int32 slotId );
        template <typename SizePolicy> bool TryWriteAsmTypedArr     ( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, ArrayBufferView::ViewType viewType );
        template <typename SizePolicy> bool TryWriteAsmSimdTypedArr ( OpCodeAsmJs op, RegSlot value, uint32 slotIndex, uint8 dataWidth, ArrayBufferView::ViewType viewType );
        template <typename SizePolicy> bool TryWriteAsmJsUnsigned1  ( OpCodeAsmJs op, uint C1 );

        void AddJumpOffset( Js::OpCodeAsmJs op, ByteCodeLabel labelId, uint fieldByteOffset );
    };
}
#endif
