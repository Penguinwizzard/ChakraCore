//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
.intel_syntax noprefix
#include "unixasmmacros.inc"


#ifdef _ENABLE_DYNAMIC_THUNKS

//============================================================================================================
// InterpreterStackFrame::DelayDynamicInterpreterThunk
//============================================================================================================

// JavascriptMethod InterpreterStackFrame::EnsureDynamicInterpreterThunk(ScriptFunction * function)
// extrn _ZN2Js21InterpreterStackFrame29EnsureDynamicInterpreterThunkEPNS_14ScriptFunctionE

// Var InterpreterStackFrame::DelayDynamicInterpreterThunk(RecyclableObject* function, CallInfo callInfo, ...)
.balign 16
NESTED_ENTRY _ZN2Js21InterpreterStackFrame28DelayDynamicInterpreterThunkEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT, NoHandler
        // save volatile registers
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        lea  rbp, [rsp]

        sub rsp, 20h                            // allocate stack space for the callee params(min 4 slots is mandate)
        call _ZN2Js21InterpreterStackFrame29EnsureDynamicInterpreterThunkEPNS_14ScriptFunctionE

        add rsp, 20h                            // de-allocate stack space for the callee params(min 4 slots is mandate)

        //EPILOGUE starts here
        lea rsp, [rbp]
        pop rbp

        // restore volatile registers
        mov rcx, qword ptr [rsp + 8h]
        mov rdx, qword ptr [rsp + 10h]
        mov r8,  qword ptr [rsp + 18h]
        mov r9,  qword ptr [rsp + 20h]

        jmp rax
NESTED_END _ZN2Js21InterpreterStackFrame28DelayDynamicInterpreterThunkEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT


//============================================================================================================
// InterpreterStackFrame::AsmJsDelayDynamicInterpreterThunk
//============================================================================================================

// JavascriptMethod InterpreterStackFrame::EnsureDynamicInterpreterThunk(ScriptFunction * function)
// extrn _ZN2Js21InterpreterStackFrame29EnsureDynamicInterpreterThunkEPNS_14ScriptFunctionE

// Var InterpreterStackFrame::AsmJsDelayDynamicInterpreterThunk(RecyclableObject* function, CallInfo callInfo, ...)
.balign 16
NESTED_ENTRY _ZN2Js21InterpreterStackFrame33AsmJsDelayDynamicInterpreterThunkEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT, NoHandler
        // save volatile registers
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        lea  rbp, [rsp]

        sub rsp, 60h

        // spill potential floating point arguments to stack
        movaps xmmword ptr [rsp + 30h], xmm1
        movaps xmmword ptr [rsp + 40h], xmm2
        movaps xmmword ptr [rsp + 50h], xmm3
        call _ZN2Js21InterpreterStackFrame29EnsureDynamicInterpreterThunkEPNS_14ScriptFunctionE
        // restore potential floating point arguments from stack
        movaps xmm1, xmmword ptr [rsp + 30h]
        movaps xmm2, xmmword ptr [rsp + 40h]
        movaps xmm3, xmmword ptr [rsp + 50h]
        add rsp, 60h

        // EPILOGUE starts here
        lea rsp, [rbp]
        pop rbp

        // restore volatile registers
        mov rcx, qword ptr [rsp + 8h]
        mov rdx, qword ptr [rsp + 10h]
        mov r8,  qword ptr [rsp + 18h]
        mov r9,  qword ptr [rsp + 20h]

        jmp rax
NESTED_END _ZN2Js21InterpreterStackFrame33AsmJsDelayDynamicInterpreterThunkEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT


//============================================================================================================
// DynamicProfileInfo::EnsureDynamicProfileInfoThunk
//============================================================================================================
// JavascriptMethod DynamicProfileInfo::EnsureDynamicProfileInfo(ScriptFunction * function)
// extrn _ZN2Js18DynamicProfileInfo24EnsureDynamicProfileInfoEPNS_14ScriptFunctionE

// Var DynamicProfileInfo::EnsureDynamicProfileInfoThunk(RecyclableObject* function, CallInfo callInfo, ...)
.balign 16
NESTED_ENTRY _ZN2Js18DynamicProfileInfo29EnsureDynamicProfileInfoThunkEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT, NoHandler
        // save volatile registers
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        lea  rbp, [rsp]

        sub rsp, 20h
        call _ZN2Js18DynamicProfileInfo24EnsureDynamicProfileInfoEPNS_14ScriptFunctionE

        add rsp, 20h

        lea rsp, [rbp]
        pop rbp

        // restore volatile registers
        mov rcx, qword ptr [rsp + 8h]
        mov rdx, qword ptr [rsp + 10h]
        mov r8,  qword ptr [rsp + 18h]
        mov r9,  qword ptr [rsp + 20h]

        jmp rax
NESTED_END _ZN2Js18DynamicProfileInfo29EnsureDynamicProfileInfoThunkEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT

#endif // _ENABLE_DYNAMIC_THUNKS


#ifdef ENABLE_SCRIPT_PROFILING

//============================================================================================================
// ScriptContext::ProfileModeDeferredParsingThunk
//============================================================================================================

// Js::JavascriptMethod ScriptContext::ProfileModeDeferredParse(ScriptFunction *function)
extrn ?ProfileModeDeferredParse@ScriptContext@Js@@SAP6APEAXPEAVRecyclableObject@2@UCallInfo@2@ZZPEAPEAVScriptFunction@2@@Z : PROC

// Var ScriptContext::ProfileModeDeferredParsingThunk(RecyclableObject* function, CallInfo callInfo, ...)
align 16
?ProfileModeDeferredParsingThunk@ScriptContext@Js@@SAPEAXPEAVRecyclableObject@2@UCallInfo@2@ZZ PROC FRAME
        // save volatile registers
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        .pushreg rbp
        lea  rbp, [rsp]
        .setframe rbp, 0
        .endprolog

        sub rsp, 20h
        lea rcx, [rsp + 30h]
        call ?ProfileModeDeferredParse@ScriptContext@Js@@SAP6APEAXPEAVRecyclableObject@2@UCallInfo@2@ZZPEAPEAVScriptFunction@2@@Z

ifdef _CONTROL_FLOW_GUARD
        mov rcx, rax                            // __guard_check_icall_fptr requires the call target in rcx.
        call [__guard_check_icall_fptr]         // verify that the call target is valid
        mov rax, rcx                            // restore call target
endif
        add rsp, 20h

        lea rsp, [rbp]
        pop rbp

        // restore volatile registers
        mov rcx, qword ptr [rsp + 8h]
        mov rdx, qword ptr [rsp + 10h]
        mov r8,  qword ptr [rsp + 18h]
        mov r9,  qword ptr [rsp + 20h]

        rex_jmp_reg rax
?ProfileModeDeferredParsingThunk@ScriptContext@Js@@SAPEAXPEAVRecyclableObject@2@UCallInfo@2@ZZ ENDP

//============================================================================================================


//============================================================================================================
// ScriptContext::ProfileModeDeferredDeserializeThunk
//============================================================================================================

// Js::JavascriptMethod ScriptContext::ProfileModeDeferredDeserialize(ScriptFunction *function)
extrn ?ProfileModeDeferredDeserialize@ScriptContext@Js@@SAP6APEAXPEAVRecyclableObject@2@UCallInfo@2@ZZPEAVScriptFunction@2@@Z : PROC

// Var ScriptContext::ProfileModeDeferredDeserializeThunk(RecyclableObject* function, CallInfo callInfo, ...)
align 16
?ProfileModeDeferredDeserializeThunk@ScriptContext@Js@@SAPEAXPEAVRecyclableObject@2@UCallInfo@2@ZZ PROC FRAME
        // save volatile registers
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        .pushreg rbp
        lea  rbp, [rsp]
        .setframe rbp, 0
        .endprolog

        sub rsp, 20h
        call ?ProfileModeDeferredDeserialize@ScriptContext@Js@@SAP6APEAXPEAVRecyclableObject@2@UCallInfo@2@ZZPEAVScriptFunction@2@@Z

        add rsp, 20h

        lea rsp, [rbp]
        pop rbp

        // restore volatile registers
        mov rcx, qword ptr [rsp + 8h]
        mov rdx, qword ptr [rsp + 10h]
        mov r8,  qword ptr [rsp + 18h]
        mov r9,  qword ptr [rsp + 20h]

        rex_jmp_reg rax
?ProfileModeDeferredDeserializeThunk@ScriptContext@Js@@SAPEAXPEAVRecyclableObject@2@UCallInfo@2@ZZ ENDP

#endif // ENABLE_SCRIPT_PROFILING


#ifdef _ENABLE_DYNAMIC_THUNKS

//============================================================================================================
// Js::AsmJsInterpreterThunk
//============================================================================================================

// extern _ZN2Js21InterpreterStackFrame29GetAsmJsInterpreterEntryPointEPNS_20AsmJsCallStackLayoutE

// AsmJsInterpreterThunk (AsmJsCallStackLayout *function, ...)
.balign 16
NESTED_ENTRY InterpreterAsmThunk_InterpreterStackFrame_Js_SAXPEAUAsmJsCallStackLayout_2__Z, _TEXT, NoHandler
        // spill arguments
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

        // get correct interpreter entrypoint

        call _ZN2Js21InterpreterStackFrame29GetAsmJsInterpreterEntryPointEPNS_20AsmJsCallStackLayoutE

        mov rcx, qword ptr [rsp + 70h] // restore rcx

        call rax // call appropriate template

        lea rsp, [rbp]
        pop rbp

        ret
NESTED_END InterpreterAsmThunk_InterpreterStackFrame_Js_SAXPEAUAsmJsCallStackLayout_2__Z, _TEXT

//============================================================================================================
// Js::AsmJsExternalEntryPoint
//============================================================================================================

//extrn ?GetStackSizeForAsmJsUnboxing@Js@@YAHPEAVScriptFunction@1@@Z: PROC
//extrn ?UnboxAsmJsArguments@Js@@YAPEAXPEAVScriptFunction@1@PEAPEAXPEADUCallInfo@1@@Z : PROC
// extrn ?BoxAsmJsReturnValue@Js@@YAPEAXPEAVScriptFunction@1@HNM@Z : PROC
//extrn ?BoxAsmJsReturnValue@Js@@YAPEAXPEAVScriptFunction@1@HNMT__m128@@@Z : PROC

//extrn ?GetArgsSizesArray@Js@@YAPEAIPEAVScriptFunction@1@@Z : PROC

// int Js::AsmJsExternalEntryPoint(RecyclableObject* entryObject, CallInfo callInfo, ...)// 
.balign 16
NESTED_ENTRY _ZN2Js23AsmJsExternalEntryPointEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT, NoHandler
        mov qword ptr [rsp + 8h],  rcx
        mov qword ptr [rsp + 10h], rdx
        mov qword ptr [rsp + 18h], r8
        mov qword ptr [rsp + 20h], r9

        push rbp
        lea  rbp, [rsp]

        sub rsp, 40h

        mov [rsp + 28h], rsi
        mov [rsp + 30h], rdi

        mov rsi, rcx // store entryObject in rsi
        mov rdi, rdx // store callInfo in rdi

        // allocate stack space for unboxed values
        // int GetStackSizeForAsmJsUnboxing(ScriptFunction* func)
        call _ZN2Js28GetStackSizeForAsmJsUnboxingEPNS_14ScriptFunctionE

        mov r9, rdi
        mov rdx, rsp // orig stack pointer is arg for the unboxing helper
        mov rdi, rdx // save orig stack pointer, so that we can add it back later
        add rdx, 68h // account for the changes we have already made to rsp

        sub rsp, rax // allocate additional stack space for args
        // UnboxAsmJsArguments(func, origArgsLoc, argDst, callInfo)
        mov rcx, rsi
        mov r8, rsp

        sub rsp, 20h // so stack space for unboxing function isn't same as where it is unboxing into. allocate args spill space for unboxing function.
        // unboxing function also does stack probe
        call _ZN2Js19UnboxAsmJsArgumentsEPNS_14ScriptFunctionEPPvPcNS_8CallInfoE
        // rax = target function address

        add rsp, 20h

        // move first 4 arguments into registers.
        // don't know types other than arg0 (which is ScriptFunction *), so put in both xmm and general purpose registers
        mov rcx, rsi

        // int GetArgsSizesArray(ScriptFunction* func)
        // get args sizes of target asmjs function
        // rcx has ScriptFunction*
        push rdi
        push rax
        push rcx
        sub rsp, 28h
        call _ZN2Js17GetArgsSizesArrayEPNS_14ScriptFunctionE
        mov rdi, rax
        add rsp, 28h
        pop rcx
        pop rax

        // Move 3 args to regs per convention. rcx already has first arg: ScriptFunction*
        push rsi
        // rsi->unboxed args
        lea rsi, [rsp + 18h] // rsp + size of(rdi + rsi + ScriptFunction*)

        // rdi is arg size
        cmp dword ptr [rdi], 10h
        je SIMDArg2
        mov rdx, [rsi]
        movq xmm1, qword ptr [rsi]
        add rsi, 8h
        jmp Arg3
    SIMDArg2:
        movups xmm1, xmmword ptr[rsi]
        add rsi, 10h
    Arg3:
        cmp dword ptr [rdi + 4h], 10h
        je SIMDArg3
        mov r8, [rsi]
        movq xmm2, qword ptr [rsi]
        add rsi, 8h
        jmp Arg4
    SIMDArg3:
        movups xmm2, xmmword ptr[rsi]
        add rsi, 10h
    Arg4:
        cmp dword ptr [rdi + 8h], 10h
        je SIMDArg4
        mov r9, [rsi]
        movq xmm3, qword ptr [rsi]
        jmp ArgsDone
   SIMDArg4:
        movups xmm3, xmmword ptr [rsi]

   ArgsDone:
        pop rsi
        pop rdi

        // call entry point
        call rax

        // Var BoxAsmJsReturnValue(ScriptFunction* func, int intRetVal, double doubleRetVal, float floatRetVal)
        mov rcx, rsi
        mov edx, eax
        movsd xmm2, xmm0
        movss xmm3, xmm0


        // store SIMD xmm value and pointer to it as argument to box function
        sub rsp, 40h
        movups [rsp + 30h], xmm0
        lea rsi, [rsp + 30h]
        mov qword ptr [rsp + 20h], rsi
        call _ZN2Js19BoxAsmJsReturnValueEPNS_14ScriptFunctionEidfDv4_f

        mov rsp, rdi // restore stack pointer
    Epilogue:
        mov rsi, [rsp + 28h]
        mov rdi, [rsp + 30h]

        lea  rsp, [rbp]
        pop rbp

        ret
NESTED_END _ZN2Js23AsmJsExternalEntryPointEPNS_16RecyclableObjectENS_8CallInfoEz, _TEXT

#endif // _ENABLE_DYNAMIC_THUNKS
