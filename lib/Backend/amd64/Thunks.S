//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

.intel_syntax noprefix
#include "unixasmmacros.inc"


//============================================================================================================
// Fake __chkstk
//============================================================================================================
.balign 16
LEAF_ENTRY __chkstk, _TEXT
        ret
LEAF_END __chkstk, _TEXT

//============================================================================================================
// NativeCodeGenerator::CheckCodeGenThunk
//============================================================================================================

//.extern _ZN19NativeCodeGenerator12CheckCodeGenEPN2Js14ScriptFunctionE
.balign 16
NESTED_ENTRY _ZN19NativeCodeGenerator17CheckCodeGenThunkEPN2Js16RecyclableObjectENS0_8CallInfoEz, _TEXT, NoHandler
        // save volatile registers
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        lea  rbp, [rsp]

        sub rsp, 20h                            // allocate stack space for the callee params(min 4 slots is mandate)
        call _ZN19NativeCodeGenerator12CheckCodeGenEPN2Js14ScriptFunctionE
        add rsp, 20h                            // de-allocate stack space for the callee params(min 4 slots is mandate)

        // EPILOGUE starts here

        lea rsp, [rbp]
        pop rbp

        // restore volatile registers
        mov rcx, qword ptr [rsp + 8h]
        mov rdx, qword ptr [rsp + 10h]
        mov r8,  qword ptr [rsp + 18h]
        mov r9,  qword ptr [rsp + 20h]

        jmp rax
NESTED_END _ZN19NativeCodeGenerator17CheckCodeGenThunkEPN2Js16RecyclableObjectENS0_8CallInfoEz, _TEXT

//============================================================================================================
// NativeCodeGenerator::CheckAsmJsCodeGenThunk
//============================================================================================================

//.extern _ZN19NativeCodeGenerator17CheckAsmJsCodeGenEPN2Js14ScriptFunctionE
.balign 16
NESTED_ENTRY _ZN19NativeCodeGenerator22CheckAsmJsCodeGenThunkEPN2Js16RecyclableObjectENS0_8CallInfoEz, _TEXT, NoHandler
        // save volatile registers
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        lea  rbp, [rsp]

        sub rsp, 60h

        // spill potential floating point arguments to stack
        movups xmmword ptr [rsp + 30h], xmm1
        movups xmmword ptr [rsp + 40h], xmm2
        movups xmmword ptr [rsp + 50h], xmm3

        call _ZN19NativeCodeGenerator17CheckAsmJsCodeGenEPN2Js14ScriptFunctionE

        // EPILOGUE starts here
        movups xmm1, xmmword ptr [rsp + 30h]
        movups xmm2, xmmword ptr [rsp + 40h]
        movups xmm3, xmmword ptr [rsp + 50h]

        lea rsp, [rbp]
        pop rbp

        // restore volatile registers
        mov rcx, qword ptr [rsp + 8h]
        mov rdx, qword ptr [rsp + 10h]
        mov r8,  qword ptr [rsp + 18h]
        mov r9,  qword ptr [rsp + 20h]

        jmp rax
NESTED_END _ZN19NativeCodeGenerator22CheckAsmJsCodeGenThunkEPN2Js16RecyclableObjectENS0_8CallInfoEz, _TEXT
