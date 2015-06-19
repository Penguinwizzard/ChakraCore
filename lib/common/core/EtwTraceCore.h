//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// C-style callback
extern "C" {
    void EtwCallback(
        ULONG controlCode,
        PVOID callbackContext);
}

class EtwCallbackApi
{
public:
    static void OnSessionChange(ULONG controlCode, PVOID callbackContext);
};

// The macro 'MCGEN_PRIVATE_ENABLE_CALLBACK_V2' below should be defined before microsoft-scripting-jscript9events.h
// is included. This is a compliler assert to ensure this happens. If this does not happen rundown events will not work.
#ifdef JSCRIPT_METHOD_METHODLOAD_OPCODE
CompileAssert(false)
#endif

// Declaring this macro enables the registration of the callback on DLL attach. See InetCore\inc\IERESP_mshtml.h for more details.
#define MCGEN_PRIVATE_ENABLE_CALLBACK_V2(SourceId, ControlCode, Level, MatchAnyKeyword, MatchAllKeyword, FilterData, CallbackContext) \
       EtwCallback(ControlCode, CallbackContext)

#include <microsoft-scripting-chakraevents.h>
#include <ieresp_mshtml.h>
#include <microsoft-scripting-jscript9.internalevents.h>

//
// Encapsulates base routines to initialize ETW tracing in the module
// 
class EtwTraceCore
{
public:
    static void Register();
    static void UnRegister();

    static bool s_registered;
};
