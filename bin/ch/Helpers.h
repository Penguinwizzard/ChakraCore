//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

class Helpers
{
public :
    static HRESULT LoadScriptFromFile(LPCWSTR filename, LPCWSTR& contents, bool* isUtf8Out = nullptr, LPCWSTR* contentsRawOut = nullptr, UINT* lengthBytesOut = nullptr, bool printFileOpenError = true);
};
