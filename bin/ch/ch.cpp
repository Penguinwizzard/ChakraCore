//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------
#include "stdafx.h"

unsigned int MessageBase::s_messageCount = 0;

LPCWSTR hostName = L"ch.exe";

extern "C"
HRESULT __stdcall OnChakraCoreLoadedEntry(TestHooks& testHooks)
{
    return ChakraRTInterface::OnChakraCoreLoaded(testHooks);
}

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

int HostExceptionFilter(int exceptionCode, _EXCEPTION_POINTERS *ep)
{
    ChakraRTInterface::NotifyUnhandledException(ep);

    bool crashOnException = false;
    ChakraRTInterface::GetCrashOnExceptionFlag(&crashOnException);

    if (exceptionCode == EXCEPTION_BREAKPOINT || (crashOnException && exceptionCode != 0xE06D7363))
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    fwprintf(stderr, L"FATAL ERROR: %ls failed due to exception code %x\n", hostName, exceptionCode);
    fflush(stderr);

    return EXCEPTION_EXECUTE_HANDLER;
}


void __stdcall PrintUsage()
{
    wprintf(L"\n\nUsage: ch.exe [flaglist] [filename]\n");
    ChakraRTInterface::PrintConfigFlagsUsageString();
}

HRESULT ExecuteTest(LPCWSTR fileName)
{
    HRESULT hr = S_OK;
    LPCWSTR fileContents = nullptr;
    JsRuntimeHandle runtime = JS_INVALID_RUNTIME_HANDLE;
    MessageQueue * messageQueue = nullptr;
    hr = Helpers::LoadScriptFromFile(fileName, fileContents);
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

HRESULT ExecuteTestWithMemoryCheck(BSTR fileName)
{
    HRESULT hr = E_FAIL;
#ifdef CHECK_MEMORY_LEAK
    // Always check memory leak, unless user specfied the flag already
    if (!ChakraRTInterface::IsEnabledCheckMemoryFlag())
    {
        ChakraRTInterface::SetCheckMemoryLeakFlag(true);
    }

    // Disable the output in case an unhandled exception happens
    // We will reenable it if there is no unhandled exceptions
    ChakraRTInterface::SetEnableCheckMemoryLeakOutput(false);
#endif

    __try
    {
        hr = ExecuteTest(fileName);
    }
    __except (HostExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
    {
        _flushall();

        // Exception happened, so we probably didn't clean up properly, 
        // Don't exit normally, just terminate
        TerminateProcess(::GetCurrentProcess(), GetExceptionCode());
    }

    _flushall();
#ifdef CHECK_MEMORY_LEAK
    // temporary work around ucrt limitation.
#if defined(_MSC_VER) && (_MSC_VER <= 1800)
    ChakraRTInterface::SetEnableCheckMemoryLeakOutput(true);
#endif
#endif
    return hr;
}

int _cdecl wmain(int argc, __in_ecount(argc) LPWSTR argv[])
{
    if (argc < 2)
    {
        PrintUsage();
        return EXIT_FAILURE;
    }

    Helpers::HandleArgsFlag(argc, argv);

    CComBSTR fileName;
    
    ChakraRTInterface::ArgInfo argInfo = { argc, argv, PrintUsage, &fileName.m_str };
    HINSTANCE chakraLibrary = ChakraRTInterface::LoadChakraDll(argInfo);

    if (chakraLibrary != nullptr)
    {
        ExecuteTestWithMemoryCheck(fileName.m_str);
        ChakraRTInterface::UnloadChakraDll(chakraLibrary);
    }

    return 0;
}

