//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

class CodeGenManager
{
public:
    HRESULT StartRpcServer();
    void StopRpcServer();
    HRESULT HelloServerCall();

private:
    HRESULT CreateServerProcess(
        __in LPCWSTR rpcConnectionUuidString,
        __out HANDLE* processHandle);

    HRESULT CodeGenManager::CreateBinding(
        __in HANDLE serverProcessHandle,
        __in UUID* connectionUuid,
        __out RPC_BINDING_HANDLE* bindingHandle);

    RPC_BINDING_HANDLE m_rpcBindingHandle;
    HANDLE m_rpcServerProcessHandle;
};
