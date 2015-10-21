//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

// This call binary level provide control to turn on or off a feature.
// A default implementation that enable all feature is included in jscript.common.common.lib

// To override, include an object that includes the definition of all the function in this file
// on the linker command line.  The linker always process symbols from object on the command line
// file first, thus the override will be chosen instead of the default one.

class BinaryFeatureControl
{
public:
    static bool RecyclerTest();
    static bool JsGen();
    static BOOL GetMitigationPolicyForProcess(__in HANDLE hProcess, __in PROCESS_MITIGATION_POLICY MitigationPolicy, __out_bcount(nLength) PVOID lpBuffer, __in SIZE_T nLength);
};
