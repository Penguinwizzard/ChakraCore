//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "stdafx.h"

extern "C" {
    ETW_INLINE
        VOID EtwCallback(
        ULONG controlCode,
        PVOID callbackContext)
    {
        EtwCallbackApi::OnSessionChange(controlCode, callbackContext);
    }
}

bool EtwTraceCore::s_registered = false;

//
// Registers the ETW provider - this is usually done on Jscript DLL load
// After registration, we will receive callbacks when ETW tracing is enabled/disabled.
//
void EtwTraceCore::Register()
{
    if (!s_registered)
    {
        s_registered = true;
        JSETW(EventRegisterMicrosoft_IE());
        JSETW(EventRegisterMicrosoft_JScript());
        JSETW(EventRegisterMicrosoft_JScript_Internal());

        // This will be used to distinguish the provider we are getting the callback for.
        PROVIDER_JSCRIPT9_Context.RegistrationHandle = Microsoft_JScriptHandle;
        BERP_IE_Context.RegistrationHandle = Microsoft_IEHandle;
    }
}

//
// Unregister to ensure we do not get callbacks.
//
void EtwTraceCore::UnRegister()
{
    if (s_registered)
    {
        s_registered = false;

        JSETW(EventUnregisterMicrosoft_IE());
        JSETW(EventUnregisterMicrosoft_JScript());
        JSETW(EventUnregisterMicrosoft_JScript_Internal());
    }
}