//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "ChakraCodeGenPch.h"

#include "CodeGen_s.c"

__bcount_opt(size) void * __RPC_USER midl_user_allocate(size_t size)
{
    return (HeapAlloc(GetProcessHeap(), 0, size));
}

void __RPC_USER midl_user_free(__inout void * ptr)
{
    if (ptr != NULL)
    {
        HeapFree(GetProcessHeap(), NULL, ptr);
    }
}

HRESULT InitializeRpcServer(
    __in UUID* connectionUuid)
{
    RPC_STATUS status;
    RPC_BINDING_VECTOR* bindingVector = NULL;
    UUID_VECTOR uuidVector;

    uuidVector.Count = 1;
    uuidVector.Uuid[0] = connectionUuid;

    status = RpcServerUseProtseqEpW(
        L"ncalrpc",
        RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
        NULL,
        NULL);
    if (status != RPC_S_OK)
    {
        return status;
    }

    status = RpcServerRegisterIf2(
        CodeGen_v1_0_s_ifspec,
        NULL,
        NULL,
        RPC_IF_AUTOLISTEN,
        RPC_C_LISTEN_MAX_CALLS_DEFAULT,
        (ULONG)-1,
        NULL);

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
        CodeGen_v1_0_s_ifspec,
        bindingVector,
        &uuidVector,
        NULL);

    RpcBindingVectorFree(&bindingVector);

    if (status != RPC_S_OK)
    {
        return status;
    }

    status = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, FALSE);

    return status;
}

// The client calls this API to shutdown the Byte Code Generator process.
void Shutdown(
    /* [in] */ handle_t binding)
{
    RPC_STATUS status;

    UNREFERENCED_PARAMETER(binding);

    status = RpcMgmtStopServerListening(NULL);
    if (status != RPC_S_OK)
    {
        TerminateProcess(GetCurrentProcess, status);
    }

    status = RpcServerUnregisterIf(CodeGen_v1_0_s_ifspec, NULL, FALSE);
    if (status != RPC_S_OK)
    {
        TerminateProcess(GetCurrentProcess, status);
    }
}

HRESULT HelloServer(
    /* [in] */ handle_t binding)
{
    UNREFERENCED_PARAMETER(binding);
    return S_OK;
}

int _cdecl wmain(int argc, __in_ecount(argc) WCHAR** argv)
{
    HRESULT hr;
    DWORD status;
    UUID connectionUuid;
    LPCWSTR connectionUuidString;

    if (argc != 2)
    {
        return ERROR_INVALID_PARAMETER;
    }

    connectionUuidString = argv[1];
    status = UuidFromStringW((RPC_WSTR)connectionUuidString, &connectionUuid);
    if (status != RPC_S_OK)
    {
        return status;
    }

    hr = InitializeRpcServer(&connectionUuid);
    if (FAILED(hr))
    {
        wprintf(L"InitializeRpcServer failed by 0x%x\n", hr);
        return hr;
    }

    return 0;

}
