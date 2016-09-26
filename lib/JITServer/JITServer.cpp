//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "JITServerPch.h"

__declspec(dllexport)
HRESULT JsInitializeJITServer(
    __in UUID* connectionUuid,
    __in_opt void* securityDescriptor,
    __in_opt void* alpcSecurityDescriptor)
{
    RPC_STATUS status;
    RPC_BINDING_VECTOR* bindingVector = NULL;
    UUID_VECTOR uuidVector;

    uuidVector.Count = 1;
    uuidVector.Uuid[0] = connectionUuid;

    status = RpcServerUseProtseqW(
        (RPC_WSTR)L"ncalrpc",
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        alpcSecurityDescriptor);
    if (status != RPC_S_OK)
    {
        return status;
    }

#ifndef NTBUILD
    status = RpcServerRegisterIf2(
        ServerIChakraJIT_v0_0_s_ifspec,
        NULL,
        NULL,
        RPC_IF_AUTOLISTEN,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        (ULONG)-1,
        NULL);
#else
    status = RpcServerRegisterIf3(
        ServerIChakraJIT_v0_0_s_ifspec,
        NULL,
        NULL,
        RPC_IF_AUTOLISTEN,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        (ULONG)-1,
        NULL,
        securityDescriptor);
#endif
    if (status != RPC_S_OK)
    {
        return status;
    }
    status = RpcServerInqBindings(&bindingVector);
    if (status != RPC_S_OK)
    {
        return status;
    }

    status = RpcEpRegister(
        ServerIChakraJIT_v0_0_s_ifspec,
        bindingVector,
        &uuidVector,
        NULL);

    RpcBindingVectorFree(&bindingVector);

    if (status != RPC_S_OK)
    {
        return status;
    }
    JITManager::GetJITManager()->SetIsJITServer();

    status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);

    return status;
}

HRESULT
ShutdownCommon()
{
    HRESULT status = RpcMgmtStopServerListening(NULL);
    if (status != RPC_S_OK)
    {
        return status;
    }

    status = RpcServerUnregisterIf(ServerIChakraJIT_v0_0_s_ifspec, NULL, FALSE);
    return status;
}

__declspec(dllexport)
HRESULT
JsShutdownJITServer()
{
    Assert(JITManager::GetJITManager()->IsOOPJITEnabled());

    if (JITManager::GetJITManager()->IsConnected())
    {
        // if client is hosting jit process directly, call to remotely shutdown
        return JITManager::GetJITManager()->Shutdown();
    }
    else
    {
        return ShutdownCommon();
    }
}

HRESULT
ServerShutdown(
    /* [in] */ handle_t binding)
{
    return ShutdownCommon();
}

HRESULT
ServerCleanupProcess(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t processHandle)
{
    CloseHandle((HANDLE)processHandle);
    return S_OK;
}

HRESULT
ServerInitializeThreadContext(
    /* [in] */ handle_t binding,
    /* [in] */ __RPC__in ThreadContextDataIDL * threadContextData,
    /* [out] */ __RPC__out intptr_t *threadContextRoot,
    /* [out] */ __RPC__out intptr_t *prereservedRegionAddr)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

    ServerThreadContext * contextInfo = HeapNew(ServerThreadContext, threadContextData);

    *threadContextRoot = (intptr_t)EncodePointer(contextInfo);
    *prereservedRegionAddr = (intptr_t)contextInfo->GetPreReservedVirtualAllocator()->EnsurePreReservedRegion();
    return S_OK;
}

HRESULT
ServerCleanupThreadContext(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t threadContextRoot)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

    ServerThreadContext * threadContextInfo = (ServerThreadContext*)DecodePointer((void*)threadContextRoot);
    if (threadContextInfo == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }

    while (threadContextInfo->IsJITActive()) { Sleep(1); }
    HeapDelete(threadContextInfo);

    return S_OK;
}

HRESULT
ServerUpdatePropertyRecordMap(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t threadContextInfoAddress,
    /* [in] */ __RPC__in UpdatedPropertysIDL * updatedProps)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

    ServerThreadContext * threadContextInfo = (ServerThreadContext*)DecodePointer((void*)threadContextInfoAddress);

    if (threadContextInfo == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }

    for (uint i = 0; i < updatedProps->reclaimedPropertyCount; ++i)
    {
        threadContextInfo->RemoveFromPropertyMap((Js::PropertyId)updatedProps->reclaimedPropertyIdArray[i]);
    }

    for (uint i = 0; i < updatedProps->newRecordCount; ++i)
    {
        threadContextInfo->AddToPropertyMap((Js::PropertyRecord *)updatedProps->newRecordArray[i]);
    }

    return S_OK;
}

HRESULT
ServerAddDOMFastPathHelper(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t scriptContextRoot,
    /* [in] */ intptr_t funcInfoAddr,
    /* [in] */ int helper)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

    ServerScriptContext * scriptContextInfo = (ServerScriptContext*)DecodePointer((void*)scriptContextRoot);

    if (scriptContextInfo == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }

    scriptContextInfo->AddToDOMFastPathHelperMap(funcInfoAddr, (IR::JnHelperMethod)helper);

    return S_OK;
}

HRESULT
ServerAddModuleRecordInfo(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t scriptContextInfoAddress,
    /* [in] */ unsigned int moduleId,
    /* [in] */ intptr_t localExportSlotsAddr)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

    ServerScriptContext * serverScriptContext = (ServerScriptContext*)DecodePointer((void*)scriptContextInfoAddress);
    if (serverScriptContext == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }
    serverScriptContext->AddModuleRecordInfo(moduleId, localExportSlotsAddr);
    HRESULT hr = E_FAIL;

    return hr;
}

HRESULT 
ServerSetWellKnownHostTypeId(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t threadContextRoot,
    /* [in] */ int typeId)
{
    ServerThreadContext * threadContextInfo = (ServerThreadContext*)DecodePointer((void*)threadContextRoot);

    if (threadContextInfo == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }
    threadContextInfo->SetWellKnownHostTypeId((Js::TypeId)typeId);

    return S_OK;
}

HRESULT
ServerInitializeScriptContext(
    /* [in] */ handle_t binding,
    /* [in] */ __RPC__in ScriptContextDataIDL * scriptContextData,
    /* [out] */ __RPC__out intptr_t * scriptContextInfoAddress)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

    ServerScriptContext * contextInfo = HeapNew(ServerScriptContext, scriptContextData);
    *scriptContextInfoAddress = (intptr_t)EncodePointer(contextInfo);
    return S_OK;
}

HRESULT
ServerCloseScriptContext(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t scriptContextRoot)
{
    ServerScriptContext * scriptContextInfo = (ServerScriptContext*)DecodePointer((void*)scriptContextRoot);

    if (scriptContextInfo == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }

#ifdef PROFILE_EXEC
    auto profiler = scriptContextInfo->GetCodeGenProfiler();
    if (profiler && profiler->IsInitialized())
    {
        profiler->ProfilePrint(Js::Configuration::Global.flags.Profile.GetFirstPhase());
    }
#endif

    scriptContextInfo->Close();
    return S_OK;
}

HRESULT
ServerCleanupScriptContext(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t scriptContextRoot)
{
    ServerScriptContext * scriptContextInfo = (ServerScriptContext*)DecodePointer((void*)scriptContextRoot);

    if (scriptContextInfo == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }

    while (scriptContextInfo->IsJITActive()) { Sleep(1); }
    HeapDelete(scriptContextInfo);
    return S_OK;
}

HRESULT
ServerNewInterpreterThunkBlock(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t scriptContextInfo,
    /* [in] */ intptr_t threadContextInfo,
    /* [in] */ boolean asmJsThunk,
    /* [out] */ __RPC__out InterpreterThunkInfoIDL * thunkInfo)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
    ServerThreadContext * threadContext = (ServerThreadContext*)DecodePointer((void*)threadContextInfo);
    ServerScriptContext * Scriptcontext = (ServerScriptContext *)DecodePointer((void*)scriptContextInfo);


    BYTE* buffer;
    BYTE* currentBuffer;
    DWORD bufferSize = InterpreterThunkEmitter::BlockSize;
    DWORD thunkCount = 0;

    intptr_t interpreterThunk = 0;
    EmitBufferManager<> * emitBufferManager = threadContext->GetEmitBufferManager(asmJsThunk);
    // the static interpreter thunk invoked by the dynamic emitted thunk
#ifdef ASMJS_PLAT
    if (asmJsThunk)
    {
        interpreterThunk = SHIFT_ADDR(threadContext, Js::InterpreterStackFrame::InterpreterAsmThunk);
    }
    else
#endif
    {
        interpreterThunk = SHIFT_ADDR(threadContext, Js::InterpreterStackFrame::InterpreterThunk);
    }

    EmitBufferAllocation * allocation = emitBufferManager->AllocateBuffer(bufferSize, &buffer);
    if (!emitBufferManager->ProtectBufferWithExecuteReadWriteForInterpreter(allocation))
    {
        Js::Throw::OutOfMemory();
    }

    currentBuffer = buffer;

#ifdef _M_X64
    PrologEncoder prologEncoder(threadContext->GetForegroundArenaAllocator());
    prologEncoder.EncodeSmallProlog(InterpreterThunkEmitter::PrologSize, InterpreterThunkEmitter::StackAllocSize);
    DWORD pdataSize = prologEncoder.SizeOfPData();
#elif defined(_M_ARM32_OR_ARM64)
    DWORD pdataSize = sizeof(RUNTIME_FUNCTION);
#else
    DWORD pdataSize = 0;
#endif
    DWORD bytesRemaining = bufferSize;
    DWORD bytesWritten = 0;
    DWORD epilogSize = sizeof(InterpreterThunkEmitter::Epilog);

    // Ensure there is space for PDATA at the end
    BYTE* pdataStart = currentBuffer + (bufferSize - Math::Align(pdataSize, EMIT_BUFFER_ALIGNMENT));
    BYTE* epilogStart = pdataStart - Math::Align(epilogSize, EMIT_BUFFER_ALIGNMENT);

    // Copy the thunk buffer and modify it.
    js_memcpy_s(currentBuffer, bytesRemaining, InterpreterThunk, HeaderSize);
    EncodeInterpreterThunk(currentBuffer, buffer, HeaderSize, epilogStart, epilogSize, interpreterThunk);
    currentBuffer += HeaderSize;
    bytesRemaining -= HeaderSize;

    // Copy call buffer
    DWORD callSize = sizeof(Call);
    while (currentBuffer < epilogStart - callSize)
    {
        js_memcpy_s(currentBuffer, bytesRemaining, Call, callSize);
#if _M_ARM
        int offset = (epilogStart - (currentBuffer + JmpOffset));
        Assert(offset >= 0);
        DWORD encodedOffset = EncoderMD::BranchOffset_T2_24(offset);
        DWORD encodedBranch = /*opcode=*/ 0x9000F000 | encodedOffset;
        Emit(currentBuffer, JmpOffset, encodedBranch);
#elif _M_ARM64
        int64 offset = (epilogStart - (currentBuffer + JmpOffset));
        Assert(offset >= 0);
        DWORD encodedOffset = EncoderMD::BranchOffset_26(offset);
        DWORD encodedBranch = /*opcode=*/ 0x14000000 | encodedOffset;
        Emit(currentBuffer, JmpOffset, encodedBranch);
#else
        // jump requires an offset from the end of the jump instruction.
        int offset = (int)(epilogStart - (currentBuffer + JmpOffset + sizeof(int)));
        Assert(offset >= 0);
        Emit(currentBuffer, JmpOffset, offset);
#endif
        currentBuffer += callSize;
        bytesRemaining -= callSize;
        thunkCount++;
    }

    // Fill any gap till start of epilog
    bytesWritten = FillDebugBreak(currentBuffer, (DWORD)(epilogStart - currentBuffer));
    bytesRemaining -= bytesWritten;
    currentBuffer += bytesWritten;

    // Copy epilog
    bytesWritten = CopyWithAlignment(currentBuffer, bytesRemaining, Epilog, epilogSize, EMIT_BUFFER_ALIGNMENT);
    currentBuffer += bytesWritten;
    bytesRemaining -= bytesWritten;

    // Generate and register PDATA
#if PDATA_ENABLED
    BYTE* epilogEnd = epilogStart + epilogSize;
    DWORD functionSize = (DWORD)(epilogEnd - buffer);
    Assert(pdataStart == currentBuffer);
#ifdef _M_X64
    Assert(bytesRemaining >= pdataSize);
    BYTE* pdata = prologEncoder.Finalize(buffer, functionSize, pdataStart);
    bytesWritten = CopyWithAlignment(pdataStart, bytesRemaining, pdata, pdataSize, EMIT_BUFFER_ALIGNMENT);
#elif defined(_M_ARM32_OR_ARM64)
    RUNTIME_FUNCTION pdata;
    GeneratePdata(buffer, functionSize, &pdata);
    bytesWritten = CopyWithAlignment(pdataStart, bytesRemaining, (const BYTE*)&pdata, pdataSize, EMIT_BUFFER_ALIGNMENT);
#endif
    void* pdataTable;
    PDataManager::RegisterPdata((PRUNTIME_FUNCTION)pdataStart, (ULONG_PTR)buffer, (ULONG_PTR)epilogEnd, &pdataTable);
#endif
    if (!emitBufferManager->CommitReadWriteBufferForInterpreter(allocation, buffer, bufferSize))
    {
        Js::Throw::OutOfMemory();
    }

    // Call to set VALID flag for CFG check
    ThreadContext::GetContextForCurrentThread()->SetValidCallTargetForCFG(buffer);

    // Update object state only at the end when everything has succeeded - and no exceptions can be thrown.
    ThunkBlock* block = this->thunkBlocks.PrependNode(allocator, buffer);
    UNREFERENCED_PARAMETER(block);
#if PDATA_ENABLED
    block->SetPdata(pdataTable);
#endif
    this->thunkCount = thunkCount;
    this->thunkBuffer = buffer;

    return S_OK;
}

HRESULT
ServerFreeAllocation(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t threadContextInfo,
    /* [in] */ intptr_t address)
{
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
    ServerThreadContext * context = (ServerThreadContext*)DecodePointer((void*)threadContextInfo);

    if (context == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }

    bool succeeded = context->GetCodeGenAllocators()->emitBufferManager.FreeAllocation((void*)address);
    return succeeded ? S_OK : E_FAIL;
}

HRESULT
ServerIsNativeAddr(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t threadContextInfo,
    /* [in] */ intptr_t address,
    /* [out] */ __RPC__out boolean * result)
{
    ServerThreadContext * context = (ServerThreadContext*)DecodePointer((void*)threadContextInfo);

    if (context == nullptr)
    {
        *result = false;
        return RPC_S_INVALID_ARG;
    }

    PreReservedVirtualAllocWrapper *preReservedVirtualAllocWrapper = context->GetPreReservedVirtualAllocator();
    if (preReservedVirtualAllocWrapper->IsInRange((void*)address))
    {
        *result = true;
    }
    else if (!context->IsAllJITCodeInPreReservedRegion())
    {
        CustomHeap::CodePageAllocators::AutoLock autoLock(context->GetCodePageAllocators());
        *result = context->GetCodePageAllocators()->IsInNonPreReservedPageAllocator((void*)address);
    }
    else
    {
        *result = false;
    }

    return S_OK;
}

HRESULT
ServerSetIsPRNGSeeded(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t scriptContextInfoAddress,
    /* [in] */ boolean value)
{
    ServerScriptContext * scriptContextInfo = (ServerScriptContext*)DecodePointer((void*)scriptContextInfoAddress);
    scriptContextInfo->SetIsPRNGSeeded(value != FALSE);

    return S_OK;
}

HRESULT
ServerRemoteCodeGen(
    /* [in] */ handle_t binding,
    /* [in] */ intptr_t threadContextInfoAddress,
    /* [in] */ intptr_t scriptContextInfoAddress,
    /* [in] */ __RPC__in CodeGenWorkItemIDL *workItemData,
    /* [out] */ __RPC__out JITOutputIDL *jitData)
{
    UNREFERENCED_PARAMETER(binding);
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(static_cast<ExceptionType>(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

    LARGE_INTEGER start_time = { 0 };
    if (PHASE_TRACE1(Js::BackEndPhase))
    {
        QueryPerformanceCounter(&start_time);
    }
    memset(jitData, 0, sizeof(JITOutputIDL));

    ServerThreadContext * threadContextInfo = (ServerThreadContext*)DecodePointer((void*)threadContextInfoAddress);
    ServerScriptContext * scriptContextInfo = (ServerScriptContext*)DecodePointer((void*)scriptContextInfoAddress);

    if (threadContextInfo == nullptr || scriptContextInfo == nullptr)
    {
        return RPC_S_INVALID_ARG;
    }

    NoRecoverMemoryJitArenaAllocator jitArena(L"JITArena", threadContextInfo->GetPageAllocator(), Js::Throw::OutOfMemory);

    scriptContextInfo->BeginJIT(); // TODO: OOP JIT, improve how we do this
    threadContextInfo->BeginJIT();

    JITTimeWorkItem * jitWorkItem = Anew(&jitArena, JITTimeWorkItem, workItemData);

    if (PHASE_VERBOSE_TRACE_RAW(Js::BackEndPhase, jitWorkItem->GetJITTimeInfo()->GetSourceContextId(), jitWorkItem->GetJITTimeInfo()->GetLocalFunctionId()))
    {
        LARGE_INTEGER freq;
        LARGE_INTEGER end_time;
        QueryPerformanceCounter(&end_time);
        QueryPerformanceFrequency(&freq);

        Output::Print(
            L"BackendMarshalIn - function: %s time:%8.6f mSec\r\n",
            jitWorkItem->GetJITFunctionBody()->GetDisplayName(),
            (((double)((end_time.QuadPart - workItemData->startTime)* (double)1000.0 / (double)freq.QuadPart))) / (1));
        Output::Flush();
    }

    auto profiler = scriptContextInfo->GetCodeGenProfiler();
#ifdef PROFILE_EXEC
    if (profiler && !profiler->IsInitialized())
    {
        profiler->Initialize(threadContextInfo->GetPageAllocator(), nullptr);
    }
#endif

    jitData->numberPageSegments = (XProcNumberPageSegment*)midl_user_allocate(sizeof(XProcNumberPageSegment));
    if (!jitData->numberPageSegments)
    {
        scriptContextInfo->EndJIT();
        threadContextInfo->EndJIT();

        return E_OUTOFMEMORY;
    }
    __analysis_assume(jitData->numberPageSegments);

    memcpy_s(jitData->numberPageSegments, sizeof(XProcNumberPageSegment), jitWorkItem->GetWorkItemData()->xProcNumberPageSegment, sizeof(XProcNumberPageSegment));

    HRESULT hr = S_OK;
    try
    {
        Func::Codegen(
            &jitArena,
            jitWorkItem,
            threadContextInfo,
            scriptContextInfo,
            jitData,
            nullptr,
            nullptr,
            jitWorkItem->GetPolymorphicInlineCacheInfo(),
            threadContextInfo->GetCodeGenAllocators(),
            nullptr,
            profiler,
            true);
    }
    catch (Js::OutOfMemoryException)
    {
        hr = E_OUTOFMEMORY;
    }
    catch (Js::StackOverflowException)
    {
        hr = VBSERR_OutOfStack;
    }
    catch (Js::OperationAbortedException)
    {
        hr = E_ABORT;
    }
    scriptContextInfo->EndJIT();
    threadContextInfo->EndJIT();

#ifdef PROFILE_EXEC
    if (profiler && profiler->IsInitialized())
    {
        profiler->ProfilePrint(Js::Configuration::Global.flags.Profile.GetFirstPhase());
    }
#endif

    if (PHASE_VERBOSE_TRACE_RAW(Js::BackEndPhase, jitWorkItem->GetJITTimeInfo()->GetSourceContextId(), jitWorkItem->GetJITTimeInfo()->GetLocalFunctionId()))
    {
        LARGE_INTEGER freq;
        LARGE_INTEGER end_time;
        QueryPerformanceCounter(&end_time);
        QueryPerformanceFrequency(&freq);

        Output::Print(
            L"EndBackEndInner - function: %s time:%8.6f mSec\r\n",
            jitWorkItem->GetJITFunctionBody()->GetDisplayName(),
            (((double)((end_time.QuadPart - start_time.QuadPart)* (double)1000.0 / (double)freq.QuadPart))) / (1));
        Output::Flush();

    }
    LARGE_INTEGER out_time = { 0 };
    if (PHASE_TRACE1(Js::BackEndPhase))
    {
        QueryPerformanceCounter(&out_time);
        jitData->startTime = out_time.QuadPart;
    }
    return hr;
}
