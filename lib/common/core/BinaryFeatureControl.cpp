//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

//For making direct call in release binaries.
#if !defined(DELAYLOAD_SET_CFG_TARGET) && !defined(LANGUAGE_SERVICE)
extern "C"
WINBASEAPI
BOOL
WINAPI
GetProcessMitigationPolicy(
__in HANDLE hProcess,
__in PROCESS_MITIGATION_POLICY MitigationPolicy,
__out_bcount(nLength) PVOID lpBuffer,
__in SIZE_T nLength
);
#endif // ENABLE_DEBUG_CONFIG_OPTIONS

// this file is compiled without LANGUAGE_SERVICE defined and included in 
// jscript.common.commom.lib as the default

// jscript9ls.dll language service includes this file and recompile it with 
// LANGUAGE_SERVICE defined and put it on the linker command line, thus
// override the default.

bool
BinaryFeatureControl::DynamicProfile()
{
#ifdef LANGUAGE_SERVICE
    return false;
#else
    return true;
#endif
}

bool
BinaryFeatureControl::LanguageService()
{
#ifdef LANGUAGE_SERVICE
    return true;
#else
    return false;
#endif
}

bool
BinaryFeatureControl::NativeCodeGen()
{
#ifdef LANGUAGE_SERVICE
    return false;
#else
    return true;
#endif
}

bool
BinaryFeatureControl::PerfCounter()
{
#ifdef LANGUAGE_SERVICE
    return false;
#else
    return true;
#endif
}

bool
BinaryFeatureControl::RecyclerTest()
{
#ifdef RECYCLER_TEST
    return true;
#else
    return false;
#endif
}

bool
BinaryFeatureControl::JsGen()
{
#ifdef JSGEN
    return true;
#else
    return false;
#endif
}

BOOL 
BinaryFeatureControl::GetMitigationPolicyForProcess(__in HANDLE hProcess, __in PROCESS_MITIGATION_POLICY MitigationPolicy, __out_bcount(nLength) PVOID lpBuffer, __in SIZE_T nLength)
{
#if !defined(DELAYLOAD_SET_CFG_TARGET) && !defined(LANGUAGE_SERVICE)
    return GetProcessMitigationPolicy(hProcess, MitigationPolicy, lpBuffer, nLength);
#else
    return FALSE;
#endif
}
