//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

unsigned int MessageBase::s_messageCount = 0;

LPCWSTR hostName = L"ch.exe";

LPCWSTR chakratestDllName = L"chakracoretest.dll";
LPCWSTR chakraDllName = L"chakracore.dll";

JsRuntimeAttributes jsrtAttributes = JsRuntimeAttributeAllowScriptInterrupt;
LPCWSTR JsErrorCodeToString(JsErrorCode jsErrorCode)
{
    switch (jsErrorCode)
    {
    case JsNoError:
        return L"JsNoError";
        break;

    case JsErrorInvalidArgument:
        return L"JsErrorInvalidArgument";
        break;

    case JsErrorNullArgument:
        return L"JsErrorNullArgument";
        break;

    case JsErrorNoCurrentContext:
        return L"JsErrorNoCurrentContext";
        break;

    case JsErrorInExceptionState:
        return L"JsErrorInExceptionState";
        break;

    case JsErrorNotImplemented:
        return L"JsErrorNotImplemented";
        break;

    case JsErrorWrongThread:
        return L"JsErrorWrongThread";
        break;

    case JsErrorRuntimeInUse:
        return L"JsErrorRuntimeInUse";
        break;

    case JsErrorBadSerializedScript:
        return L"JsErrorBadSerializedScript";
        break;

    case JsErrorInDisabledState:
        return L"JsErrorInDisabledState";
        break;

    case JsErrorCannotDisableExecution:
        return L"JsErrorCannotDisableExecution";
        break;

    case JsErrorHeapEnumInProgress:
        return L"JsErrorHeapEnumInProgress";
        break;

    case JsErrorOutOfMemory:
        return L"JsErrorOutOfMemory";
        break;

    case JsErrorScriptException:
        return L"JsErrorScriptException";
        break;

    case JsErrorScriptCompile:
        return L"JsErrorScriptCompile";
        break;

    case JsErrorScriptTerminated:
        return L"JsErrorScriptTerminated";
        break;

    case JsErrorFatal:
        return L"JsErrorFatal";
        break;

    default:
        return L"<unknown>";
        break;
    }
}
#define IfJsErrorFailLog(expr) do { JsErrorCode jsErrorCode = expr; if ((jsErrorCode) != JsNoError) { fwprintf(stderr, L"ERROR: " TEXT(#expr) L" failed. JsErrorCode=0x%x (%s)\n", jsErrorCode, JsErrorCodeToString(jsErrorCode)); fflush(stderr); goto Error; } } while (0)

HINSTANCE LoadChakraDll(LPCWSTR dllName, LPCWSTR altDllName)
{
    HINSTANCE library = LoadLibraryEx(dllName, nullptr, 0);
    if (library == nullptr)
    {
        library = LoadLibraryEx(altDllName, nullptr, 0);
        if (library == nullptr)
        {
            int ret = GetLastError();
            fwprintf(stderr, L"FATAL ERROR: Unable to load %ls and %ls GetLastError=0x%x\n", dllName, altDllName, ret);
            return nullptr;
        }
    }

    ChakraRTInterface::Initialize(library);
    return library;
}

void UnloadChakraDll(HINSTANCE library)
{
    Assert(library != nullptr);
    FARPROC pDllCanUnloadNow = GetProcAddress(library, "DllCanUnloadNow");
    if (pDllCanUnloadNow != nullptr)
    {
        pDllCanUnloadNow();
    }
    FreeLibrary(library);
}

int HostExceptionFilter(int exceptionCode, _EXCEPTION_POINTERS *ep)
{
    if (exceptionCode == EXCEPTION_BREAKPOINT)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    fwprintf(stderr, L"FATAL ERROR: %ls failed due to exception code %x\n", hostName, exceptionCode);
    fflush(stderr);

    return EXCEPTION_EXECUTE_HANDLER;
}

HRESULT ExecuteTest(LPCWSTR fileName)
{
    HRESULT hr = S_OK;
    LPCWSTR fileContents = nullptr;
    JsRuntimeHandle runtime = JS_INVALID_RUNTIME_HANDLE;
    MessageQueue * messageQueue = nullptr;
    hr = LoadScriptFromFile(fileName, fileContents);
    IfFailGo(hr);

    IfJsErrorFailLog(ChakraRTInterface::JsCreateRuntime(jsrtAttributes, nullptr, &runtime));

    JsValueRef context = JS_INVALID_REFERENCE;
    IfJsErrorFailLog(ChakraRTInterface::JsCreateContext(runtime, &context));
    IfJsErrorFailLog(ChakraRTInterface::JsSetCurrentContext(context));

    if (!WScriptJsrt::Initialize())
    {
        IfFailGo(E_FAIL);
    }

    messageQueue = new MessageQueue();
    WScriptJsrt::AddMessageQueue(messageQueue);

    wchar_t fullPath[_MAX_PATH];

    if (_wfullpath(fullPath, fileName, _MAX_PATH) == nullptr)
    {
        IfFailGo(E_FAIL);
    }

    // canonicalize that path name to lower case for the profile storage
    size_t len = wcslen(fullPath);
    for (size_t i = 0; i < len; i++)
    {
        fullPath[i] = towlower(fullPath[i]);
    }
    JsErrorCode runScript = ChakraRTInterface::JsRunScript(fileContents, 0/*sourceContext*/, fullPath, nullptr/*result*/);
    if (runScript != JsNoError)
    {
        WScriptJsrt::PrintException(fileName, runScript);
    }
    else
    {
        // Repeatedly flush the message queue until it's empty.  It is necessary to loop on this
        // because setTimeout can add scripts to execute
        do
        {
            IfFailGo(messageQueue->ProcessAll());
        } while (!messageQueue->IsEmpty());
    }

Error:
    ChakraRTInterface::JsSetCurrentContext(nullptr);

    if (messageQueue != nullptr)
    {
        delete messageQueue;
    }

    if (runtime != JS_INVALID_RUNTIME_HANDLE)
    {
        ChakraRTInterface::JsDisposeRuntime(runtime);
    }

    _flushall();

    return hr;
}

int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR argv[])
{
    HINSTANCE chakraLibrary = LoadChakraDll(chakraDllName, chakratestDllName);

    if (chakraLibrary != nullptr)
    {
        __try
        {
            LPCWSTR fileName = L"test.js"; // TODO : to have real file here 
            ExecuteTest(fileName);
        }
        __except (HostExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
        {
            _flushall();

            // Exception happened, so we probably didn't clean up properly, 
            // Don't exit normally, just terminate
            TerminateProcess(::GetCurrentProcess(), GetExceptionCode());
        }

        UnloadChakraDll(chakraLibrary);
    }
    return 0;
}

