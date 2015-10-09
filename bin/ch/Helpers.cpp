//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "StdAfx.h"

HRESULT Helpers::LoadScriptFromFile(LPCWSTR filename, LPCWSTR& contents, bool* isUtf8Out, LPCWSTR* contentsRawOut, UINT* lengthBytesOut, bool printFileOpenError)
{
    HRESULT hr = S_OK;
    LPCWSTR contentsRaw = nullptr;
    UINT lengthBytes = 0;
    bool isUtf8 = false;
    contents = nullptr;
    FILE * file;

    //
    // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
    // etc.
    //
    if (_wfopen_s(&file, filename, L"rb") != 0)
    {
        if (printFileOpenError)
        {
            DWORD lastError = GetLastError();
            wchar_t wszBuff[512];
            fwprintf(stderr, L"Error in opening file '%s' ", filename);
            wszBuff[0] = 0;
            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr,
                lastError,
                0,
                wszBuff,
                _countof(wszBuff),
                nullptr))
            {
                fwprintf(stderr, L": %s", wszBuff);
            }
            fwprintf(stderr, L"\n");
            IfFailGo(E_FAIL);
        }
        else
        {
            return E_FAIL;
        }
    }

    //
    // Determine the file length, in bytes.
    //
    fseek(file, 0, SEEK_END);
    lengthBytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    contentsRaw = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, lengthBytes + sizeof(WCHAR)); // Simulate Trident buffer, allocate by HeapAlloc
    if (nullptr == contentsRaw)
    {
        fwprintf(stderr, L"out of memory");
        IfFailGo(E_OUTOFMEMORY);
    }

    //
    // Read the entire content as a binary block.
    //
    fread((void*)contentsRaw, sizeof(char), lengthBytes, file);
    fclose(file);
    *(WCHAR*)((byte*)contentsRaw + lengthBytes) = L'\0'; // Null terminate it. Could be LPCWSTR.

    //
    // Read encoding, handling any conversion to Unicode.
    //
    // Warning: The UNICODE buffer for parsing is supposed to be provided by the host.
    // this is temporary code to read from Unicode and ANSI files.
    // It is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, SCSU, BOCU could be
    // wrongly classified as ANSI
    //
    byte * pRawBytes = (byte*)contentsRaw;
    if ((0xEF == *pRawBytes && 0xBB == *(pRawBytes + 1) && 0xBF == *(pRawBytes + 2)))
    {
        isUtf8 = true;
    }
    else if (0xFFFE == *contentsRaw || 0x0000 == *contentsRaw && 0xFEFF == *(contentsRaw + 1))
    {
        // unicode unsupported
        fwprintf(stderr, L"unsupported file encoding");
        IfFailGo(E_UNEXPECTED);
    }
    else if (0xFEFF == *contentsRaw)
    {
        // unicode LE
        contents = contentsRaw;
    }
    else
    {
        // Assume UTF8
        isUtf8 = true;
    }


    if (isUtf8)
    {
        UINT cAnsiChars = lengthBytes + 1;
        contents = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, cAnsiChars * sizeof(WCHAR)); // Simulate Trident buffer, allocate by HeapAlloc
        if (nullptr == contents)
        {
            fwprintf(stderr, L"out of memory");
            IfFailGo(E_OUTOFMEMORY);
        }

        // Covert to Unicode.
        if (0 == MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)contentsRaw, cAnsiChars,
            (LPWSTR)contents, cAnsiChars))
        {
            fwprintf(stderr, L"failed MultiByteToWideChar conversion");
            IfFailGo(HRESULT_FROM_WIN32(GetLastError()));
        }
    }

Error:
    if (SUCCEEDED(hr) && isUtf8Out)
    {
        Assert(contentsRawOut);
        Assert(lengthBytesOut);
        *isUtf8Out = isUtf8;
        *contentsRawOut = contentsRaw;
        *lengthBytesOut = lengthBytes;
    }
    else if (contentsRaw && (contentsRaw != contents)) // Otherwise contentsRaw is lost. Free it if it is different to contents.
    {
        HeapFree(GetProcessHeap(), 0, (void*)contentsRaw);
    }

    if (contents && FAILED(hr))
    {
        HeapFree(GetProcessHeap(), 0, (void*)contents);
        contents = nullptr;
    }

    return hr;
}


