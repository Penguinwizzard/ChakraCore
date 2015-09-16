//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
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

HRESULT RunScript(LPCWSTR fileName, LPCWSTR fileContents, BYTE *bcBuffer, wchar_t *fullPath)
{
    HRESULT hr = S_OK;
    MessageQueue * messageQueue = new MessageQueue();
    WScriptJsrt::AddMessageQueue(messageQueue);
    Assert(fileContents != nullptr || bcBuffer != nullptr);
    JsErrorCode runScript;
    if (bcBuffer != nullptr)
    {
        runScript = ChakraRTInterface::JsRunSerializedScript(fileContents, bcBuffer, JS_SOURCE_CONTEXT_NONE, fullPath, nullptr/*result*/);
    }
    else
    {
        runScript = ChakraRTInterface::JsRunScript(fileContents, 0/*sourceContext*/, fullPath, nullptr/*result*/);
    }

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
    if (messageQueue != nullptr)
    {
        delete messageQueue;
    }
    return hr;
}

HRESULT CreateAndRunSerializedScript(LPCWSTR fileName, LPCWSTR fileContents, wchar_t *fullPath)
{
    HRESULT hr = S_OK;
    JsRuntimeHandle runtime = JS_INVALID_RUNTIME_HANDLE;
    JsContextRef context = JS_INVALID_REFERENCE, current = JS_INVALID_REFERENCE;
    BYTE *bcBuffer = nullptr;
    DWORD bcBufferSize = 0;

    IfJsErrorFailLog(ChakraRTInterface::JsSerializeScript(fileContents, bcBuffer, &bcBufferSize));
    // Above call will return the size of the buffer only, once succeed we need to allocate memory of that much and call it again.
    if (bcBufferSize == 0)
    {
        AssertMsg(false, "bufferSize should not be zero");
        IfFailGo(E_FAIL);
    }
    bcBuffer = new BYTE[bcBufferSize];
    DWORD newBcBufferSize = bcBufferSize;
    IfJsErrorFailLog(ChakraRTInterface::JsSerializeScript(fileContents, bcBuffer, &newBcBufferSize));
    Assert(bcBufferSize == newBcBufferSize);

    // Bytecode buffer is created in one runtime and will be executed on different runtime.

    IfJsErrorFailLog(ChakraRTInterface::JsCreateRuntime(jsrtAttributes, nullptr, &runtime));

    IfJsErrorFailLog(ChakraRTInterface::JsCreateContext(runtime, &context));
    IfJsErrorFailLog(ChakraRTInterface::JsGetCurrentContext(&current));
    IfJsErrorFailLog(ChakraRTInterface::JsSetCurrentContext(context));

    // Initialized the WScript object on the new context
    if (!WScriptJsrt::Initialize())
    {
        IfFailGo(E_FAIL);
    }

    IfFailGo(RunScript(fileName, fileContents, bcBuffer, fullPath));

Error:
    if (bcBuffer != nullptr)
    {
        delete[] bcBuffer;
    }

    if (current != JS_INVALID_REFERENCE)
    {
        ChakraRTInterface::JsSetCurrentContext(current);
    }

    if (runtime != JS_INVALID_RUNTIME_HANDLE)
    {
        ChakraRTInterface::JsDisposeRuntime(runtime);
    }
    return hr;
}

HRESULT ExecuteTest(LPCWSTR fileName)
{
    HRESULT hr = S_OK;
    LPCWSTR fileContents = nullptr;
    JsRuntimeHandle runtime = JS_INVALID_RUNTIME_HANDLE;
    bool isUtf8 = false;
    LPCOLESTR contentsRaw = nullptr;
    UINT lengthBytes = 0;
    hr = Helpers::LoadScriptFromFile(fileName, fileContents, &isUtf8, &contentsRaw, &lengthBytes);
    contentsRaw; lengthBytes; // Unused for now.

    IfFailGo(hr);

    IfJsErrorFailLog(ChakraRTInterface::JsCreateRuntime(jsrtAttributes, nullptr, &runtime));

    JsContextRef context = JS_INVALID_REFERENCE;
    IfJsErrorFailLog(ChakraRTInterface::JsCreateContext(runtime, &context));
    IfJsErrorFailLog(ChakraRTInterface::JsSetCurrentContext(context));

    if (!WScriptJsrt::Initialize())
    {
        IfFailGo(E_FAIL);
    }

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

    if (HostConfigFlags::flags.GenerateLibraryByteCodeHeaderIsEnabled)
    {
        if (isUtf8)
        {
            if (HostConfigFlags::flags.GenerateLibraryByteCodeHeader != nullptr && *HostConfigFlags::flags.GenerateLibraryByteCodeHeader != L'\0')
            {
                Assert(false);
                // TODO : functionality will be added in next checkin.
            }
            else
            {
                fwprintf(stderr, L"FATAL ERROR: -GenerateLibraryByteCodeHeader must provide the file name eg. -GenerateLibraryByteCodeHeader:<bytecode file name>, exiting\n");
                IfFailGo(E_FAIL);
            }
        }
        else
        {
            fwprintf(stderr, L"FATAL ERROR: GenerateLibraryByteCodeHeader flag can only be used on UTF8 file, exiting\n");
            IfFailGo(E_FAIL);
        }
    }
    else if (HostConfigFlags::flags.SerializedIsEnabled)
    {
        if (isUtf8)
        {
            CreateAndRunSerializedScript(fileName, fileContents, fullPath);
        }
        else
        {
            fwprintf(stderr, L"FATAL ERROR: Serialized flag can only be used on UTF8 file, exiting\n");
            IfFailGo(E_FAIL);
        }
    }
    else
    {
        IfFailGo(RunScript(fileName, fileContents, nullptr, fullPath));
    }

Error:
    ChakraRTInterface::JsSetCurrentContext(nullptr);

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

    HostConfigFlags::HandleArgsFlag(argc, argv);

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

