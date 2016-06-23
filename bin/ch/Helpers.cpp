//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "Codex/Utf8Codex.h"

///
/// Use the codex library to encode a UTF16 string to UTF8.
/// The caller is responsible for freeing the memory, which is allocated
/// using malloc.
/// The returned string is null terminated.
///
HRESULT Helpers::WideStringToNarrowDynamic(LPCWSTR sourceString, LPSTR* destStringPtr)
{
    size_t cchSourceString = wcslen(sourceString);
    
    if (cchSourceString >= MAXUINT32)
    {
        return E_OUTOFMEMORY;
    }

    size_t cbDestString = (cchSourceString + 1) * 3;

    // Check for overflow- cbDestString should be >= cchSourceString
    if (cbDestString < cchSourceString)
    {
        return E_OUTOFMEMORY;
    }

    utf8char_t* destString = (utf8char_t*)malloc(cbDestString);
    if (destString == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    size_t cbDecoded = utf8::EncodeIntoAndNullTerminate(destString, sourceString, (charcount_t) cchSourceString);
    Assert(cbDecoded <= cbDestString);
    static_assert(sizeof(utf8char_t) == sizeof(char), "Needs to be valid for cast");
    *destStringPtr = (char*)destString;
    return S_OK;
}

///
/// Use the codex library to encode a UTF8 string to UTF16.
/// The caller is responsible for freeing the memory, which is allocated
/// using malloc.
/// The returned string is null terminated.
///
HRESULT Helpers::NarrowStringToWideDynamic(LPCSTR sourceString, LPWSTR* destStringPtr)
{
    size_t cbSourceString = strlen(sourceString);
    charcount_t cchDestString = utf8::ByteIndexIntoCharacterIndex((LPCUTF8) sourceString, cbSourceString);
    size_t cbDestString = (cchDestString + 1) * sizeof(WCHAR);
    
    // Check for overflow- cbDestString should be >= cchSourceString
    if (cbDestString < cchDestString)
    {
        return E_OUTOFMEMORY;
    }

    WCHAR* destString = (WCHAR*)malloc(cbDestString);
    if (destString == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    utf8::DecodeIntoAndNullTerminate(destString, (LPCUTF8) sourceString, cchDestString);
    static_assert(sizeof(utf8char_t) == sizeof(char), "Needs to be valid for cast");
    *destStringPtr = destString;
    return S_OK;
}

HRESULT Helpers::LoadScriptFromFile(LPCSTR filename, LPCWSTR& contents, bool* isUtf8Out, LPCWSTR* contentsRawOut, UINT* lengthBytesOut, bool printFileOpenError)
{
    HRESULT hr = S_OK;
    LPCWSTR contentsRaw = nullptr;
    byte * pRawBytes = nullptr;
    UINT lengthBytes = 0;
    bool isUtf8 = false;
    contents = nullptr;
    FILE * file;

    //
    // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
    // etc.
    //
    if (fopen_s(&file, filename, "rb") != 0)
    {
        if (printFileOpenError)
        {
#ifdef _WIN32
            DWORD lastError = GetLastError();
            char16 wszBuff[512];
            fprintf(stderr, "Error in opening file '%s' ", filename);
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
#elif defined(_POSIX_VERSION)
            fprintf(stderr, "Error in opening file: ");
            perror(filename);
#endif            
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
    contentsRaw = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, lengthBytes + sizeof(WCHAR));
    if (nullptr == contentsRaw)
    {
        fwprintf(stderr, _u("out of memory"));
        IfFailGo(E_OUTOFMEMORY);
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
    pRawBytes = (byte*)contentsRaw;
    if ((0xEF == *pRawBytes && 0xBB == *(pRawBytes + 1) && 0xBF == *(pRawBytes + 2)))
    {
        isUtf8 = true;
    }
    else if (0xFFFE == *contentsRaw || (0x0000 == *contentsRaw && 0xFEFF == *(contentsRaw + 1)))
    {
        // unicode unsupported
        fwprintf(stderr, _u("unsupported file encoding"));
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
        utf8::DecodeOptions decodeOptions = utf8::doAllowInvalidWCHARs;

        UINT cUtf16Chars = utf8::ByteIndexIntoCharacterIndex(pRawBytes, lengthBytes, decodeOptions);
        contents = (LPCWSTR)HeapAlloc(GetProcessHeap(), 0, (cUtf16Chars + 1) * sizeof(WCHAR));
        if (nullptr == contents)
        {
            fwprintf(stderr, _u("out of memory"));
            IfFailGo(E_OUTOFMEMORY);
        }

        utf8::DecodeIntoAndNullTerminate((char16*) contents, pRawBytes, cUtf16Chars, decodeOptions);
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

LPCWSTR Helpers::JsErrorCodeToString(JsErrorCode jsErrorCode)
{
    bool hasException = false;
    ChakraRTInterface::JsHasException(&hasException);
    if (hasException)
    {
        WScriptJsrt::PrintException("", JsErrorScriptException);
    }

    switch (jsErrorCode)
    {
    case JsNoError:
        return _u("JsNoError");
        break;

    case JsErrorInvalidArgument:
        return _u("JsErrorInvalidArgument");
        break;

    case JsErrorNullArgument:
        return _u("JsErrorNullArgument");
        break;

    case JsErrorNoCurrentContext:
        return _u("JsErrorNoCurrentContext");
        break;

    case JsErrorInExceptionState:
        return _u("JsErrorInExceptionState");
        break;

    case JsErrorNotImplemented:
        return _u("JsErrorNotImplemented");
        break;

    case JsErrorWrongThread:
        return _u("JsErrorWrongThread");
        break;

    case JsErrorRuntimeInUse:
        return _u("JsErrorRuntimeInUse");
        break;

    case JsErrorBadSerializedScript:
        return _u("JsErrorBadSerializedScript");
        break;

    case JsErrorInDisabledState:
        return _u("JsErrorInDisabledState");
        break;

    case JsErrorCannotDisableExecution:
        return _u("JsErrorCannotDisableExecution");
        break;

    case JsErrorHeapEnumInProgress:
        return _u("JsErrorHeapEnumInProgress");
        break;

    case JsErrorOutOfMemory:
        return _u("JsErrorOutOfMemory");
        break;

    case JsErrorScriptException:
        return _u("JsErrorScriptException");
        break;

    case JsErrorScriptCompile:
        return _u("JsErrorScriptCompile");
        break;

    case JsErrorScriptTerminated:
        return _u("JsErrorScriptTerminated");
        break;

    case JsErrorFatal:
        return _u("JsErrorFatal");
        break;

    default:
        return _u("<unknown>");
        break;
    }
}

void Helpers::LogError(__in __nullterminated const char16 *msg, ...)
{
    va_list args;
    va_start(args, msg);
    wprintf(_u("ERROR: "));
    vfwprintf(stderr, msg, args);
    wprintf(_u("\n"));
    fflush(stdout);
    va_end(args);
}


void Helpers::TTReportLastIOErrorAsNeeded(BOOL ok, char* msg)
{
    if(!ok)
    {
#ifdef _WIN32
        DWORD lastError = GetLastError();
        LPTSTR pTemp = NULL;
        FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY, NULL, lastError, 0, (LPTSTR)&pTemp, 0, NULL);
        fwprintf(stderr, _u(": %s"), pTemp);
#endif
        fprintf(stderr, "msg is: %s", msg);

        AssertMsg(false, "IO Error!!!");
    }
}

void Helpers::CreateDirectoryIfNeeded(const char16* path)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
#else
    bool isPathDirName = (path[wcslen(path) - 1] == _u('\\'));

    size_t fplength = (wcslen(path) + 2);
    char16* fullpath = new char16[fplength];
    fullpath[0] = _u('\0');

    wcscat_s(fullpath, fplength, path);
    if(!isPathDirName)
    {
        wcscat_s(fullpath, fplength, _u("\\"));
    }

    DWORD dwAttrib = GetFileAttributes(fullpath);
    if((dwAttrib != INVALID_FILE_ATTRIBUTES) && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY))
    {
        delete[] fullpath;
        return;
    }

    BOOL success = CreateDirectory(fullpath, NULL);
    Helpers::TTReportLastIOErrorAsNeeded(success, "Failed Directory Create");

    delete[] fullpath;
#endif
}

void Helpers::DeleteDirectory(const char16* path)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
#else
    HANDLE hFile;
    WIN32_FIND_DATA FileInformation;

    bool isPathDirName = (path[wcslen(path) - 1] == _u('\\'));

    size_t splength = (wcslen(path) + 5);
    char16* strPattern = new char16[splength];
    strPattern[0] = _u('\0');

    wcscat_s(strPattern, splength, path);
    if(!isPathDirName)
    {
        wcscat_s(strPattern, splength, _u("\\"));
    }
    wcscat_s(strPattern, splength, _u("*.*"));

    hFile = ::FindFirstFile(strPattern, &FileInformation);
    if(hFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            if(FileInformation.cFileName[0] != '.')
            {
                size_t sfplength = (wcslen(path) + wcslen(FileInformation.cFileName) + 2);
                char16* strFilePath = new char16[sfplength];
                strFilePath[0] = _u('\0');

                wcscat_s(strFilePath, sfplength, path);
                if(!isPathDirName)
                {
                    wcscat_s(strFilePath, sfplength, _u("\\"));
                }
                wcscat_s(strFilePath, sfplength, FileInformation.cFileName);

                if(FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    DeleteDirectory(strFilePath);
                    ::RemoveDirectory(strFilePath);
                }
                else
                {
                    // Set file attributes
                    ::SetFileAttributes(strFilePath, FILE_ATTRIBUTE_NORMAL);
                    ::DeleteFile(strFilePath);
                }

                delete[] strFilePath;
            }
        } while(::FindNextFile(hFile, &FileInformation) == TRUE);

        // Close handle
        ::FindClose(hFile);
    }

    delete[] strPattern;
#endif
}

void Helpers::GetFileFromURI(const char16* uri, char16** res)
{
    int urilen = (int)wcslen(uri);
    int fpos = 0;
    for(int spos = urilen - 1; spos >= 0; --spos)
    {
        if(uri[spos] == _u('\\') || uri[spos] == _u('/'))
        {
            fpos = spos + 1;
            break;
        }
    }

    size_t rlength = (wcslen(uri + fpos) + 1);
    *res = new char16[rlength];
    (*res)[0] = _u('\0');

    wcscat_s(*res, rlength, uri + fpos);
}

void Helpers::GetDefaultTTDDirectory(char16** res, const char16* optExtraDir)
{
#ifndef _WIN32
    *res = nullptr;
    AssertMsg(false, "Not XPLAT yet.");
#else
    char16* path = new char16[MAX_PATH];
    path[0] = _u('\0');

    GetModuleFileName(NULL, path, MAX_PATH);

    char16* spos = wcsstr(path, _u("\\Build\\VcBuild\\"));
    AssertMsg(spos != nullptr, "Something got renamed or moved!!!");

    int ccount = (int)((((byte*)spos) - ((byte*)path)) / sizeof(char16));

    *res = (char16*)CoTaskMemAlloc(MAX_PATH * sizeof(char16));
    if(*res == nullptr)
    {
        //This is for testing only so just assert and return here is ok
        AssertMsg(false, "OOM");
        return;
    }

    (*res)[0] = _u('\0');

    for(int i = 0; i < ccount; ++i)
    {
        (*res)[i] = path[i];
    }
    (*res)[ccount] = _u('\0');

    wcscat_s(*res, MAX_PATH, _u("\\test\\_ttdlog\\"));

    if(wcslen(optExtraDir) == 0)
    {
        wcscat_s(*res, MAX_PATH, _u("_defaultLog"));
    }
    else
    {
        wcscat_s(*res, MAX_PATH, optExtraDir);
    }

    bool isPathDirName = ((*res)[wcslen(*res) - 1] == _u('\\'));
    if(!isPathDirName)
    {
        wcscat_s(*res, MAX_PATH, _u("\\"));
    }

    delete[] path;
#endif
}

void CALLBACK Helpers::GetTTDDirectory(const char16* uri, char16** fullTTDUri)
{
#ifndef _WIN32
    *fullTTDUri = nullptr;
    AssertMsg(false, "Not XPLAT yet.");
#else
    if(uri[0] != _u('!'))
    {
        bool isPathDirName = (uri[wcslen(uri) - 1] == _u('\\'));

        size_t rlength = (wcslen(uri) + wcslen(_u("\\")) + 1);
        *fullTTDUri = (wchar_t*)CoTaskMemAlloc(rlength * sizeof(char16));
        if(*fullTTDUri == nullptr)
        {
            //This is for testing only so just assert and return here is ok
            AssertMsg(false, "OOM");
            return;
        }

        (*fullTTDUri)[0] = _u('\0');

        wcscat_s(*fullTTDUri, rlength, uri);
        if(!isPathDirName)
        {
            wcscat_s(*fullTTDUri, rlength, _u("\\"));
        }
    }
    else
    {
        Helpers::GetDefaultTTDDirectory(fullTTDUri, uri + 1);
    }
#endif
}

void CALLBACK Helpers::TTInitializeForWriteLogStreamCallback(const char16* uri)
{
    //If the directory does not exist then we want to create it
    Helpers::CreateDirectoryIfNeeded(uri);

    //Clear the logging directory so it is ready for us to write into
    Helpers::DeleteDirectory(uri);
}

HANDLE Helpers::TTOpenStream_Helper(const char16* uri, bool read, bool write)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
    return 0;
#else
    AssertMsg((read | write) & (!read | !write), "Read/Write streams not supported yet -- defaulting to read only");

    HANDLE res = INVALID_HANDLE_VALUE;

    if(read)
    {
        res = CreateFile(uri, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    else
    {
        res = CreateFile(uri, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }

    Helpers::TTReportLastIOErrorAsNeeded(res != INVALID_HANDLE_VALUE, "Failed File Open");

    return res;
#endif
}

HANDLE CALLBACK Helpers::TTGetLogStreamCallback(const char16* uri, bool read, bool write)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
    return 0;
#else
    AssertMsg((read | write) & !(read & write), "Should be either read or write and at least one.");

    size_t rlength = (wcslen(uri) + wcslen(_u("ttdlog.log")) + 1);
    char16* logfile = new char16[rlength];
    logfile[0] = _u('\0');

    wcscat_s(logfile, rlength, uri);
    wcscat_s(logfile, rlength, _u("ttdlog.log"));

    HANDLE res = TTOpenStream_Helper(logfile, read, write);

    delete[] logfile;
    return res;
#endif
}

HANDLE CALLBACK Helpers::TTGetSnapshotStreamCallback(const char16* uri, const char16* snapId, bool read, bool write)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
    return 0;
#else
    AssertMsg((read | write) & !(read & write), "Should be either read or write and at least one.");

    size_t rlength = (wcslen(uri) + wcslen(_u("\\snap_")) + wcslen(snapId) + wcslen(_u(".snp")) + 1);
    char16* snapfile = new char16[rlength];
    snapfile[0] = _u('\0');

    wcscat_s(snapfile, rlength, uri);
    wcscat_s(snapfile, rlength, _u("\\snap_"));
    wcscat_s(snapfile, rlength, snapId);
    wcscat_s(snapfile, rlength, _u(".snp"));

    HANDLE res = TTOpenStream_Helper(snapfile, read, write);

    delete[] snapfile;
    return res;
#endif
}

HANDLE CALLBACK Helpers::TTGetSrcCodeStreamCallback(const char16* uri, const char16* bodyCtrId, const char16* srcFileName, bool read, bool write)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
    return 0;
#else
    AssertMsg((read | write) & !(read & write), "Should be either read or write and at least one.");

    char16* sFile = nullptr;
    Helpers::GetFileFromURI(srcFileName, &sFile);

    size_t rlength = (wcslen(uri) + wcslen(bodyCtrId) + wcslen(_u("_")) + wcslen(sFile) + 1);
    char16* srcPath = new char16[rlength];
    srcPath[0] = _u('\0');

    wcscat_s(srcPath, rlength, uri);
    wcscat_s(srcPath, rlength, bodyCtrId);
    wcscat_s(srcPath, rlength, _u("_"));
    wcscat_s(srcPath, rlength, sFile);

    HANDLE res = TTOpenStream_Helper(srcPath, read, write);

    delete[] sFile;
    delete[] srcPath;
    return res;
#endif
}

bool CALLBACK Helpers::TTReadBytesFromStreamCallback(HANDLE handle, BYTE* buff, DWORD size, DWORD* readCount)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
    return FALSE;
#else
    AssertMsg(handle != INVALID_HANDLE_VALUE, "Bad file handle.");

    *readCount = 0;
    BOOL ok = ReadFile(handle, buff, size, readCount, NULL);

    Helpers::TTReportLastIOErrorAsNeeded(ok, "Failed Read!!!");

    return ok ? true : false;
#endif
}

bool CALLBACK Helpers::TTWriteBytesToStreamCallback(HANDLE handle, BYTE* buff, DWORD size, DWORD* writtenCount)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
    return FALSE;
#else
    AssertMsg(handle != INVALID_HANDLE_VALUE, "Bad file handle.");

    BOOL ok = WriteFile(handle, buff, size, writtenCount, NULL);

    Helpers::TTReportLastIOErrorAsNeeded(ok && (*writtenCount == size), "Write Failed!!!");

    return ok ? true : false;
#endif
}

void CALLBACK Helpers::TTFlushAndCloseStreamCallback(HANDLE handle, bool read, bool write)
{
#ifndef _WIN32
    AssertMsg(false, "Not XPLAT yet.");
#else
    AssertMsg((read | write) & !(read & write), "Should be either read or write and at least one.");

    if(handle != INVALID_HANDLE_VALUE)
    {
        if(write)
        {
            FlushFileBuffers(handle);
        }

        CloseHandle(handle);
    }
#endif
}

