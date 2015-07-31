//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

class Helpers
{
public :
    static LPWSTR* argsVal;
    static int argsCount;

    static HRESULT LoadScriptFromFile(LPCWSTR filename, LPCWSTR& contents, bool* isUtf8Out = nullptr, LPCWSTR* contentsRawOut = nullptr, UINT* lengthBytesOut = nullptr, bool printFileOpenError = true);

    // handling -args <flags> -endargs
    static void HandleArgsFlag(int& argc, _Inout_updates_to_(argc, argc) LPWSTR argv[]);

};
