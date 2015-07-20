//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#pragma once

HRESULT LoadScriptFromFile(LPCWSTR filename, LPCWSTR& contents, bool* isUtf8Out = nullptr, LPCWSTR* contentsRawOut = nullptr, UINT* lengthBytesOut = nullptr, bool printFileOpenError = true);

