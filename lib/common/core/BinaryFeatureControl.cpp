//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

//For making direct call in release binaries.
#if !defined(DELAYLOAD_SET_CFG_TARGET)

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
#if !defined(DELAYLOAD_SET_CFG_TARGET)
    return GetProcessMitigationPolicy(hProcess, MitigationPolicy, lpBuffer, nLength);
#else
    return FALSE;
#endif
}
