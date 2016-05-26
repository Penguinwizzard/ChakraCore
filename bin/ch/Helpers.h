//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

class Helpers
{
public :
    static HRESULT LoadScriptFromFile(LPCSTR filename, LPCSTR& contents, bool* isUtf8Out = nullptr, UINT* lengthBytesOut = nullptr);

    static HRESULT WideStringToNarrowDynamic(LPCWSTR sourceString, LPSTR* destStringPtr);
    static HRESULT NarrowStringToWideDynamic(LPCSTR sourceString, LPWSTR* destStringPtr);
    static LPCWSTR JsErrorCodeToString(JsErrorCode jsErrorCode);
    static void LogError(__in __nullterminated const char16 *msg, ...);
};
