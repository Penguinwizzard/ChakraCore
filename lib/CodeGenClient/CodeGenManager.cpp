//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "CodeGenClientPch.h"

#include "CodeGen_c.c"

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

HRESULT
CodeGenManager::CreateServerProcess(
    __in LPCWSTR rpcConnectionUuidString,
    __out HANDLE * processHandle)
{
    HRESULT hr;
    PROCESS_INFORMATION processInfo = { 0 };
    STARTUPINFOW si = { 0 };
    WCHAR cmdLine[MAX_PATH];

    hr = StringCchCopyW(cmdLine, ARRAYSIZE(cmdLine), L"ChakraCodeGen.exe ");
    if (FAILED(hr))
    {
        return hr;
    }

    hr = StringCchCatW(cmdLine, ARRAYSIZE(cmdLine), rpcConnectionUuidString);
    if (FAILED(hr))
    {
        return hr;
    }

    if (!CreateProcessW(
        NULL,
        cmdLine,
        NULL,
        NULL,
        FALSE,
        CREATE_SUSPENDED,
        NULL,
        NULL,
        &si,
        &processInfo))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (ResumeThread(processInfo.hThread) == (DWORD)-1)
    {
        TerminateProcess(processInfo.hProcess, GetLastError());
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        return HRESULT_FROM_WIN32(GetLastError());
    }

    CloseHandle(processInfo.hThread);
    *processHandle = processInfo.hProcess;

    return NOERROR;
}
// This routine creates a binding with the server.
HRESULT
CodeGenManager::CreateBinding(
    __in HANDLE serverProcessHandle,
    __in UUID * connectionUuid,
    __out RPC_BINDING_HANDLE * bindingHandle)
{
    RPC_STATUS status;
    RPC_SECURITY_QOS_V4 securityQOS; // TODO: V5???
    DWORD attemptCount = 0;
    DWORD sleepInterval = 100; // in milliseconds
    RPC_BINDING_HANDLE localBindingHandle;
    RPC_BINDING_HANDLE_TEMPLATE_V1 bindingTemplate;
    RPC_BINDING_HANDLE_SECURITY_V1_W bindingSecurity;

    ZeroMemory(&securityQOS, sizeof(RPC_SECURITY_QOS_V4));
    securityQOS.Capabilities = RPC_C_QOS_CAPABILITIES_DEFAULT;
    securityQOS.IdentityTracking = RPC_C_QOS_IDENTITY_DYNAMIC;
    securityQOS.ImpersonationType = RPC_C_IMP_LEVEL_IDENTIFY;
    securityQOS.Version = 4;

    ZeroMemory(&bindingTemplate, sizeof(bindingTemplate));
    bindingTemplate.Version = 1;
    bindingTemplate.ProtocolSequence = RPC_PROTSEQ_LRPC;
    bindingTemplate.StringEndpoint = NULL;
    memcpy_s(&bindingTemplate.ObjectUuid, sizeof(UUID), connectionUuid, sizeof(UUID));
    bindingTemplate.Flags |= RPC_BHT_OBJECT_UUID_VALID;

    ZeroMemory(&bindingSecurity, sizeof(bindingSecurity));
    bindingSecurity.Version = 1;
    bindingSecurity.AuthnLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY;
    bindingSecurity.AuthnSvc = RPC_C_AUTHN_KERNEL;
    bindingSecurity.SecurityQos = (RPC_SECURITY_QOS*)&securityQOS;

    status = RpcBindingCreate(&bindingTemplate, &bindingSecurity, NULL, &localBindingHandle);
    if (status != RPC_S_OK)
    {
        return HRESULT_FROM_WIN32(status);
    }

    // We keep attempting to connect to the server with increasing wait intervals in between.
    // This will wait close to 5 minutes before it finally gives up.
    do
    {
        DWORD waitStatus;

        status = RpcBindingBind(NULL, localBindingHandle, CodeGen_v1_0_c_ifspec);
        if (status == RPC_S_OK)
        {
            break;
        }
        else if (status == EPT_S_NOT_REGISTERED)
        {
            // The Server side has not finished registering the RPC Server yet.
            // We should only breakout if we have reached the max attempt count.
            if (attemptCount > 600)
            {
                break;
            }
        }
        else
        {
            // Some unknown error occurred. We are not going to retry for arbitrary errors.
            break;
        }

        // When we come to this point, it means the server has not finished registration yet.
        // We should wait for a while and then reattempt to bind.
        waitStatus = WaitForSingleObject(serverProcessHandle, sleepInterval);
        if (waitStatus == WAIT_OBJECT_0)
        {
            DWORD exitCode = (DWORD)-1;

            // The server process died for some reason. No need to reattempt.
            // We use -1 as the exit code if GetExitCodeProcess fails.
            NT_VERIFY(GetExitCodeProcess(serverProcessHandle, &exitCode));
            status = RPC_S_SERVER_UNAVAILABLE;
            break;
        }
        else if (waitStatus == WAIT_TIMEOUT)
        {
            // Not an error. the server is still alive and we should reattempt.
        }
        else
        {
            // wait operation failed for an unknown reason.
            NT_ASSERT(false);
            status = HRESULT_FROM_WIN32(waitStatus);
            break;
        }

        attemptCount++;
        if (sleepInterval < 500)
        {
            sleepInterval += 100;
        }
    } while (status != RPC_S_OK); // redundant check, but compiler would not allow true here.

    if (status != RPC_S_OK)
    {
        RpcBindingFree(&localBindingHandle);
        return HRESULT_FROM_WIN32(status);
    }

    *bindingHandle = localBindingHandle;
    return S_OK;
}

HRESULT
CodeGenManager::StartRpcServer()
{
    HRESULT hr;
    RPC_STATUS status;
    UUID connectionUuid;
    HANDLE localServerProcessHandle = NULL;
    WCHAR* connectionUuidString = NULL;
    RPC_BINDING_HANDLE localBindingHandle;

    status = UuidCreate(&connectionUuid);
    if (status != RPC_S_OK && status != RPC_S_UUID_LOCAL_ONLY)
    {
        return HRESULT_FROM_WIN32(status);
    }

    status = UuidToStringW(&connectionUuid, &connectionUuidString);
    if (status != S_OK)
    {
        return HRESULT_FROM_WIN32(status);
    }

    hr = CreateServerProcess(connectionUuidString, &localServerProcessHandle);
    RpcStringFree(&connectionUuidString);
    if (FAILED(hr))
    {
        // log an event

        return hr;
    }

    hr = CreateBinding(localServerProcessHandle, &connectionUuid, &localBindingHandle);
    if (FAILED(hr))
    {
        TerminateProcess(localServerProcessHandle, (UINT)hr);
        CloseHandle(localServerProcessHandle);

        // log an event

        return hr;
    }

    m_rpcBindingHandle = localBindingHandle;
    m_rpcServerProcessHandle = localServerProcessHandle;

    // log an event

    return S_OK;
}

void
CodeGenManager::StopRpcServer()
{
    HRESULT hr = S_OK;
    DWORD waitResult;

    if (m_rpcBindingHandle == NULL)
    {
        NT_ASSERT(m_rpcBindingHandle == NULL);
        NT_ASSERT(m_rpcServerProcessHandle == NULL);
        return;
    }

    RpcTryExcept
    {
        Shutdown(m_rpcBindingHandle);
    }
        RpcExcept(1)
    {
        hr = HRESULT_FROM_WIN32(RpcExceptionCode());
    }
    RpcEndExcept;

    RpcBindingFree(&m_rpcBindingHandle);
    m_rpcBindingHandle = NULL;

    waitResult = WaitForSingleObject(m_rpcServerProcessHandle, 15000);
    if (waitResult != WAIT_OBJECT_0)
    {
        if (waitResult == WAIT_FAILED)
        {
            waitResult = (DWORD)GetLastError();
        }

        // log dirty shutdown event

        TerminateProcess(m_rpcServerProcessHandle, (UINT)-1);
    }

    // log an event

    CloseHandle(m_rpcServerProcessHandle);
    m_rpcServerProcessHandle = NULL;

}

HRESULT
CodeGenManager::HelloServerCall()
{
    HRESULT hr = E_FAIL;

    RpcTryExcept
    {
        hr = HelloServer(
        m_rpcBindingHandle);
    }
        RpcExcept(1)
    {
        hr = HRESULT_FROM_WIN32(RpcExceptionCode());
    }
    RpcEndExcept;

    return hr;

}
