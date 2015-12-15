//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

class CodeGenManager
{
public:
    HRESULT ConnectRpcServer(__in DWORD processId, __in UUID connectionUuid);
    void DisconnectRpcServer();
    HRESULT HelloServerCall();

private:

    HRESULT CodeGenManager::CreateBinding(
        __in HANDLE serverProcessHandle,
        __in UUID* connectionUuid,
        __out RPC_BINDING_HANDLE* bindingHandle);

    RPC_BINDING_HANDLE m_rpcBindingHandle;
    HANDLE m_rpcServerProcessHandle;
};
