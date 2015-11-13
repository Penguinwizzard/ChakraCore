#include "xUnit++/xUnit++.h"

#define _JSRT_
#include "chakracommon.h"

#define VERIFY_FAIL(_exp) \
    printf("%s", _CRT_STRINGIZE(exp));

HRESULT GetCurrentModulePath(std::wstring &path)
{
    HMODULE hmod;

    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(GetCurrentModulePath), &hmod))
    {
        VERIFY_FAIL(L"GetCurrentModulePath: Failed loading test module handle");
    }

    WCHAR fullpath[_MAX_PATH];

    if (GetModuleFileNameW(hmod, fullpath, _MAX_PATH) == _MAX_PATH)
    {
        VERIFY_FAIL(L"GetCurrentModulePath: Failed getting module path");
    }

    WCHAR drive[_MAX_DRIVE];
    WCHAR dir[_MAX_DIR];
    WCHAR fname[_MAX_FNAME];
    WCHAR ext[_MAX_EXT];

    if (_wsplitpath_s<_MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT>(fullpath, drive, dir, fname, ext) ||
        _wmakepath_s<_MAX_PATH>(fullpath, drive, dir, NULL, NULL))
    {
        VERIFY_FAIL(L"GetCurrentModulePath: Failed computing module path");
    }

    path = fullpath;
    return S_OK;
}

LPCWSTR LoadScriptFileWithPath(LPCWSTR filename)
{
    FILE * file;
    LPCWSTR contents = NULL;
    std::wstring fullPath;
    if (GetCurrentModulePath(fullPath) != S_OK)
    {
        return nullptr;
    }

    fullPath += filename;

    // Open the file as a binary file to prevent CRT from handling encoding, line-break conversions,
    // etc.
    if (_wfopen_s(&file, fullPath.c_str(), L"rb"))
    {
        wprintf(L"Cannot open source file: %ls", fullPath.c_str());
        return nullptr;
    }

    // Determine the file length, in bytes.
    fseek(file, 0, SEEK_END);
    UINT lengthBytes = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *rawBytes = (char *)calloc(lengthBytes + 2, sizeof(char));
    if (NULL == rawBytes)
    {
        VERIFY_FAIL(L"Out of memory");
        return NULL;
    }

    // Read the entire content as a binary block.
    fread((void *)rawBytes, sizeof(char), lengthBytes, file);
    fclose(file);

    // TODO: This is not a complete read of the encoding. Some encodings like UTF7, UTF1, EBCDIC, 
    // SCSU, BOCU could be wrongly classified as ANSI

    // Read encoding, handling any conversion to Unicode.
    wchar_t *rawCharacters = (wchar_t *)rawBytes;
    bool isUtf8 = false;
    if ((0xEF == *rawBytes && 0xBB == *(rawBytes + 1) && 0xBF == *(rawBytes + 2)))
    {
        isUtf8 = true;
    }
    else if (0xFFFE == *rawCharacters || 0x0000 == *rawCharacters && 0xFEFF == *(rawCharacters + 1))
    {
        // unicode unsupported
        delete rawBytes;
        VERIFY_FAIL(L"Unsupported file encoding");
        return NULL;
    }
    else if (0xFEFF == *rawCharacters)
    {
        // unicode LE
        contents = rawCharacters;
    }
    else
    {
        // Assume UTF8
        isUtf8 = true;
    }

    if (isUtf8)
    {
        UINT ansiCharacters = lengthBytes + 1;
        contents = (wchar_t *)calloc(ansiCharacters, sizeof(wchar_t));
        if (NULL == contents)
        {
            delete rawBytes;
            VERIFY_FAIL(L"Out of memory");
            return NULL;
        }

        // Covert to Unicode.
        if (0 == MultiByteToWideChar(CP_UTF8, 0, rawBytes, ansiCharacters,
            (LPWSTR)contents, ansiCharacters))
        {
            delete rawBytes;
            delete contents;
            VERIFY_FAIL(L"Couldn't convert file to Unicode");
            return NULL;
        }
    }

    return contents;
}

void ValidateOOMException()
{
    JsValueRef exception;
    JsErrorCode errorCode = JsGetAndClearException(&exception);
    if (errorCode == JsNoError)
    {
        JsPropertyIdRef property;
        JsValueRef value;
        LPCWSTR str;
        size_t length;

        xUnitpp::Assert.True(JsGetPropertyIdFromName(L"message", &property) == JsNoError);
        xUnitpp::Assert.True(JsGetProperty(exception, property, &value) == JsNoError);
        xUnitpp::Assert.True(JsStringToPointer(value, &str, &length) == JsNoError);
        xUnitpp::Assert.True(wcscmp(str, L"Out of memory") == 0);
        //VERIFY_ARE_EQUAL(String(L"Out of memory"), str);
    }
    else
    {
        // If we don't have enough memory to create error object, then GetAndClearException might 
        // fail and return ErrorInvalidArgument. Check if we don't get any other error code.
        xUnitpp::Assert.True(errorCode == JsErrorInvalidArgument);
    }
}

static const size_t MemoryLimit = 10 * 1024 * 1024;

void MemoryTestBasic(LPCWSTR fileName)
{
    LPCWSTR script = LoadScriptFileWithPath(fileName);
    xUnitpp::Assert.True(script != nullptr);

    // Create the runtime
    JsRuntimeHandle runtime;
    xUnitpp::Assert.True(JsCreateRuntime(JsRuntimeAttributeNone, NULL, &runtime) == JsNoError);

    // Set memory limit
    xUnitpp::Assert.True(JsSetRuntimeMemoryLimit(runtime, MemoryLimit) == JsNoError);

    size_t memoryLimit;
    size_t memoryUsage;

    xUnitpp::Assert.True(JsGetRuntimeMemoryLimit(runtime, &memoryLimit) == JsNoError);
    xUnitpp::Assert.True(memoryLimit == MemoryLimit);
    xUnitpp::Assert.True(JsGetRuntimeMemoryUsage(runtime, &memoryUsage) == JsNoError);
    xUnitpp::Assert.True(memoryUsage < MemoryLimit);

    // Create and initialize the script context
    JsContextRef context;
    xUnitpp::Assert.True(JsCreateContext(runtime, &context) == JsNoError);
    xUnitpp::Assert.True(JsSetCurrentContext(context) == JsNoError);

    // Invoke the script
    xUnitpp::Assert.True(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsErrorScriptException);
    ValidateOOMException();


    xUnitpp::Assert.True(JsGetRuntimeMemoryLimit(runtime, &memoryLimit) == JsNoError);
    xUnitpp::Assert.True(memoryLimit == MemoryLimit);
    xUnitpp::Assert.True(JsGetRuntimeMemoryUsage(runtime, &memoryUsage) == JsNoError);
    xUnitpp::Assert.True(memoryUsage <= MemoryLimit);
    //// first case (UnboundedMemory) have unlimited growth; we can have test property flag later if needed.
    //if (i != 0)
    //{
    //    xUnitpp::Assert.True(JsSetRuntimeMemoryLimit(runtime, 0xffffffff) == JsNoError);
    //    xUnitpp::Assert.True(JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"", nullptr) == JsNoError);
    //}

    // Destroy the runtime
    xUnitpp::Assert.True(JsSetCurrentContext(JS_INVALID_REFERENCE) == JsNoError);
    xUnitpp::Assert.True(JsCollectGarbage(runtime) == JsNoError);
    xUnitpp::Assert.True(JsDisposeRuntime(runtime) == JsNoError);
}

FACT("MemoryTest_Basic - UnboundedMemory.js")
{
     MemoryTestBasic(L"UnboundedMemory.js");
}

FACT("MemoryTest_Basic - arrayTest.js")
{
    MemoryTestBasic(L"arrayTest.js");
}

FACT("MemoryTest_Basic - arraybuffer.js")
{
    MemoryTestBasic(L"arraybuffer.js");
}

