//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Helpers.h"
#include "Codex/Utf8Codex.h"

///
/// Helper function to load a script file into memory
/// 
/// filename:
///     the path to the js file on disk
/// contents: 
///    Output pointer that will store a utf-16 buffer. 
///    It must be initialized to null, and if it's not null after execution of the function
///    The memory allocated must be freed using a call to free
/// isUtf8Out:
///    Output flag indicating whether the decoded buffer pointed to by contents is UTF8 or not. 
///    You can initialize this flag to indicate whether you want a utf8 file to be decoded into utf16 or not
///    If initialized to true, then, if the decoded file is indeed utf8, we'll skip the decode into utf16 and just return the buffer
///    If the file is utf16, we'll set this to false and return the utf16 buffer in contentsRaw instead.
/// contentsRawOut:
///    The contents of the file on disk. The is returned only if this is different that the value of contents.
///    If this is returned as non-null, this memory should be freed by a call to free
/// lengthBytesOut:
///    Size of the file in bytes
/// printFileOpenError:
///    Flag indicating whether an error should be printed to stderr if opening the file failed.
///
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
    if (_wfopen_s(&file, filename, _u("rb")) != 0)
    {
        if (printFileOpenError)
        {
            DWORD lastError = GetLastError();
            char16 wszBuff[512];
            fwprintf(stderr, _u("Error in opening file '%s' "), filename);
            wszBuff[0] = 0;
            if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                nullptr,
                lastError,
                0,
                wszBuff,
                _countof(wszBuff),
                nullptr))
            {
                fwprintf(stderr, _u(": %s"), wszBuff);
            }
            fwprintf(stderr, _u("\n"));
            Fail(E_FAIL);
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
    contentsRaw = (LPCWSTR)malloc(lengthBytes + sizeof(WCHAR));
    if (nullptr == contentsRaw)
    {
        fwprintf(stderr, _u("out of memory"));
        Fail(E_OUTOFMEMORY);
    }

    //
    // Read the entire content as a binary block.
    //
    fread((void*)contentsRaw, sizeof(char), lengthBytes, file);
    fclose(file);
    *(WCHAR*)((byte*)contentsRaw + lengthBytes) = _u('\0'); // Null terminate it. Could be LPCWSTR.

    //
    // Read encoding, handling any conversion to Unicode.
    //
    // Warning: The UNICODE buffer for parsing is supposed to be provided by the host.
    // This is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, SCSU, BOCU could be
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
        fwprintf(stderr, _u("unsupported file encoding"));
        Fail(E_UNEXPECTED);
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
        if (isUtf8Out && !*isUtf8Out)
        {
            utf8::DecodeOptions decodeOptions = utf8::doAllowInvalidWCHARs;

            UINT cUtf16Chars = utf8::ByteIndexIntoCharacterIndex(pRawBytes, lengthBytes, decodeOptions);
            contents = (LPCWSTR)malloc((cUtf16Chars + 1) * sizeof(WCHAR));
            if (nullptr == contents)
            {
                fwprintf(stderr, _u("out of memory"));
                Fail(E_OUTOFMEMORY);
            }

            utf8::DecodeIntoAndNullTerminate((char16*)contents, pRawBytes, cUtf16Chars, decodeOptions);
        }
    }

Error:
    if (SUCCEEDED(hr) && isUtf8Out)
    {
#pragma warning(push)
#pragma warning(disable:4127) // Conditional expression is constant
        Assert(contentsRawOut != nullptr);
        Assert(lengthBytesOut != nullptr);
#pragma warning(pop)

        *isUtf8Out = isUtf8;
        *contentsRawOut = contentsRaw;
        *lengthBytesOut = lengthBytes;
    }
    else if (contentsRaw && (contentsRaw != contents)) // Otherwise contentsRaw is lost. Free it if it is different to contents.
    {
        free((void*)contentsRaw);
    }

    if (contents && FAILED(hr))
    {
        free((void*)contents);
        contents = nullptr;
    }

    return hr;
}
