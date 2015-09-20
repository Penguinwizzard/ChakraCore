//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "StdAfx.h"

MessageQueue* WScriptJsrt::s_messageQueue = nullptr;

bool WScriptJsrt::CreateArgumentsObject(JsValueRef *argsObject)
{
    LPWSTR *argv = HostConfigFlags::argsVal;
    JsValueRef retArr;

    Assert(argsObject);
    *argsObject = nullptr;

    IfJsrtErrorFail(ChakraRTInterface::JsCreateArray(HostConfigFlags::argsCount, &retArr), false);

    for (int i = 0; i < HostConfigFlags::argsCount; i++)
    {
        JsValueRef value;
        JsValueRef index;

        IfJsrtErrorFail(ChakraRTInterface::JsPointerToString(argv[i], wcslen(argv[i]), &value), false);
        IfJsrtErrorFail(ChakraRTInterface::JsDoubleToNumber(i, &index), false);
        IfJsrtErrorFail(ChakraRTInterface::JsSetIndexedProperty(retArr, index, value), false);
    }

    *argsObject = retArr;

    return true;
}

JsValueRef __stdcall WScriptJsrt::EchoCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    for (unsigned int i = 1; i < argumentCount; i++)
    {          
        JsValueRef strValue;
        JsErrorCode error = ChakraRTInterface::JsConvertValueToString(arguments[i], &strValue);
        if (error == JsNoError)
        {
            LPCWSTR str = nullptr;
            size_t length;
            error = ChakraRTInterface::JsStringToPointer(strValue, &str, &length);
            if (error == JsNoError)
            {
                if (i > 1)
                {
                    wprintf(L" ");
                }
                wprintf(L"%ls", str);
            }
        }

        if (error == JsErrorScriptException)
        {
            return nullptr;
        }
    }

    wprintf(L"\n");

    JsValueRef undefinedValue;
    if (ChakraRTInterface::JsGetUndefinedValue(&undefinedValue) == JsNoError)
    {
        return undefinedValue;
    }
    else
    {
        return nullptr;
    }
}

JsValueRef __stdcall WScriptJsrt::QuitCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    int exitCode = 0;

    if (argumentCount > 1)
    {
        double exitCodeDouble;
        IfJsrtErrorFail(ChakraRTInterface::JsNumberToDouble(arguments[1], &exitCodeDouble), JS_INVALID_REFERENCE);
        exitCode = (int)exitCodeDouble;
    }

    ExitProcess(exitCode);
}

JsValueRef __stdcall WScriptJsrt::LoadScriptFileCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = E_FAIL;
    JsValueRef returnValue = JS_INVALID_REFERENCE;
    JsErrorCode errorCode = JsNoError;

    if (argumentCount < 2 || argumentCount > 4)
    {
        fwprintf(stderr, L"Too many or too few arguments.\n");
    }
    else
    {
        const wchar_t *fileContent;
        const wchar_t *fileName;
        const wchar_t *scriptInjectType = L"self";
        size_t fileNameLength;
        size_t scriptInjectTypeLength;

        IfJsrtErrorSetGo(ChakraRTInterface::JsStringToPointer(arguments[1], &fileName, &fileNameLength));

        if (argumentCount > 2)
        {
            IfJsrtErrorSetGo(ChakraRTInterface::JsStringToPointer(arguments[2], &scriptInjectType, &scriptInjectTypeLength));
        }

        if (errorCode == JsNoError)
        {
            hr = Helpers::LoadScriptFromFile(fileName, fileContent);
            if (FAILED(hr))
            {
                fwprintf(stderr, L"Couldn't load file.\n");
            }
            else
            {
                returnValue = LoadScript(fileName, fileNameLength, fileContent, scriptInjectType);
            }
        }
    }

Error:
    return returnValue;
}

JsValueRef __stdcall WScriptJsrt::LoadScriptCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    HRESULT hr = E_FAIL;
    JsErrorCode errorCode = JsNoError;
    JsValueRef returnValue = JS_INVALID_REFERENCE;

    if (argumentCount < 2 || argumentCount > 4)
    {
        fwprintf(stderr, L"Too many or too few arguments.\n");
    }
    else
    {
        const wchar_t *fileContent;
        const wchar_t *fileName;
        const wchar_t *scriptInjectType = L"self";
        size_t fileContentLength;
        size_t fileNameLength;
        size_t scriptInjectTypeLength;

        IfJsrtErrorSetGo(ChakraRTInterface::JsStringToPointer(arguments[1], &fileContent, &fileContentLength));

        if (argumentCount > 2)
        {
            IfJsrtErrorSetGo(ChakraRTInterface::JsStringToPointer(arguments[2], &scriptInjectType, &scriptInjectTypeLength));
        }

        fileName = L"script.js";
        fileNameLength = wcslen(fileName);
        if (argumentCount > 3)
        {
            IfJsrtErrorSetGo(ChakraRTInterface::JsStringToPointer(arguments[3], &fileName, &fileNameLength));
        }
        returnValue = LoadScript(fileName, fileNameLength, fileContent, scriptInjectType);
    }

Error:
    return returnValue;
}

JsValueRef WScriptJsrt::LoadScript(LPCWSTR fileName, size_t fileNameLength, LPCWSTR fileContent, LPCWSTR scriptInjectType)
{
    HRESULT hr = E_FAIL;
    JsErrorCode errorCode = JsNoError;
    LPCWSTR errorMessage = L"Internal error.";
    size_t errorMessageLength = wcslen(errorMessage);
    JsValueRef returnValue = JS_INVALID_REFERENCE;
    JsErrorCode innerErrorCode = JsNoError;

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

    if (wcscmp(scriptInjectType, L"self") == 0)
    {
        errorCode = ChakraRTInterface::JsRunScript(fileContent, 0, fullPath, &returnValue);
        if (errorCode != JsNoError)
        {
            PrintException(fileName, errorCode);
        }
        else
        {
            errorCode = ChakraRTInterface::JsGetGlobalObject(&returnValue);
        }
    }
    else if (wcscmp(scriptInjectType, L"samethread") == 0)
    {
        JsValueRef context = JS_INVALID_REFERENCE;
        JsValueRef newContext = JS_INVALID_REFERENCE;

        // Create a new context and set it as the current context
        IfJsrtErrorSetGo(ChakraRTInterface::JsGetCurrentContext(&context));
        JsRuntimeHandle runtime = JS_INVALID_RUNTIME_HANDLE;
        IfJsrtErrorSetGo(ChakraRTInterface::JsGetRuntime(context, &runtime));

        IfJsrtErrorSetGo(ChakraRTInterface::JsCreateContext(runtime, &newContext));
        IfJsrtErrorSetGo(ChakraRTInterface::JsSetCurrentContext(newContext));

        // Initialize the host objects
        Initialize();

        errorCode = ChakraRTInterface::JsRunScript(fileContent, 0, fullPath, &returnValue);
        if (errorCode != JsNoError)
        {
            PrintException(fileName, errorCode);
        }
        else
        {
            errorCode = ChakraRTInterface::JsGetGlobalObject(&returnValue);
        }

        // Set the context back to the old one
        ChakraRTInterface::JsSetCurrentContext(context);
    }
    else
    {
        errorCode = JsErrorInvalidArgument;
        errorMessage = L"Unsupported argument type inject type.";
    }


Error:
    JsValueRef value = returnValue;
    if (errorCode != JsNoError)
    {
        if (innerErrorCode != JsNoError)
        {
            // Failed to retrieve the inner error message, so set a custom error string
            errorMessage = ConvertErrorCodeToMessage(errorCode);
        }

        JsValueRef error = JS_INVALID_REFERENCE;
        JsValueRef messageProperty = JS_INVALID_REFERENCE;
        errorMessageLength = wcslen(errorMessage);
        innerErrorCode = ChakraRTInterface::JsPointerToString(errorMessage, errorMessageLength, &messageProperty);
        if (innerErrorCode == JsNoError)
        {
            innerErrorCode = ChakraRTInterface::JsCreateError(messageProperty, &error);
            if (innerErrorCode == JsNoError)
            {
                innerErrorCode = ChakraRTInterface::JsSetException(error);
            }
        }

        ChakraRTInterface::JsDoubleToNumber(errorCode, &value);
    }

    _flushall();

    return value;
}

JsValueRef WScriptJsrt::SetTimeoutCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    LPWSTR errorMessage = L"invalid call to WScript.SetTimeout";

    if (argumentCount != 3)
    {
        goto Error;
    }

    JsValueRef function = arguments[1];
    JsValueRef timerId;
    unsigned int time;
    double tmp;
    CallbackMessage *msg = nullptr;

    IfJsrtError(ChakraRTInterface::JsNumberToDouble(arguments[2], &tmp));

    time = static_cast<int>(tmp);
    msg = new CallbackMessage(time, function);
    s_messageQueue->Push(msg);

    IfJsrtError(ChakraRTInterface::JsDoubleToNumber(static_cast<double>(msg->GetId()), &timerId));
    return timerId;

Error:
    JsValueRef errorObject;
    JsValueRef errorMessageString;

    JsErrorCode errorCode = ChakraRTInterface::JsPointerToString(errorMessage, wcslen(errorMessage), &errorMessageString);

    if (errorCode != JsNoError)
    {
        errorCode = ChakraRTInterface::JsCreateError(errorMessageString, &errorObject);

        if (errorCode != JsNoError)
        {
            ChakraRTInterface::JsSetException(errorObject);
        }
    }

    return JS_INVALID_REFERENCE;
}

JsValueRef WScriptJsrt::ClearTimeoutCallback(JsValueRef callee, bool isConstructCall, JsValueRef *arguments, unsigned short argumentCount, void *callbackState)
{
    LPWSTR errorMessage = L"invalid call to WScript.ClearTimeout";

    if (argumentCount != 2)
    {
        goto Error;
    }

    unsigned int timerId;
    double tmp;
    JsValueRef undef;
    JsValueRef global;

    IfJsrtError(ChakraRTInterface::JsNumberToDouble(arguments[1], &tmp));

    timerId = static_cast<int>(tmp);
    s_messageQueue->RemoveById(timerId);

    IfJsrtError(ChakraRTInterface::JsGetGlobalObject(&global));
    IfJsrtError(ChakraRTInterface::JsGetUndefinedValue(&undef));

    return undef;

Error:
    JsValueRef errorObject;
    JsValueRef errorMessageString;

    JsErrorCode errorCode = ChakraRTInterface::JsPointerToString(errorMessage, wcslen(errorMessage), &errorMessageString);

    if (errorCode != JsNoError)
    {
        errorCode = ChakraRTInterface::JsCreateError(errorMessageString, &errorObject);

        if (errorCode != JsNoError)
        {
            ChakraRTInterface::JsSetException(errorObject);
        }
    }

    return JS_INVALID_REFERENCE;
}

bool WScriptJsrt::Initialize()
{
    JsValueRef wscript;
    IfJsrtErrorFail(ChakraRTInterface::JsCreateObject(&wscript), false);

    JsValueRef echo;
    IfJsrtErrorFail(ChakraRTInterface::JsCreateFunction(EchoCallback, nullptr, &echo), false);
    JsPropertyIdRef echoName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"Echo", &echoName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(wscript, echoName, echo, true), false);

    JsValueRef argsObject;

    if (!CreateArgumentsObject(&argsObject))
    {
        return false;
    }
    JsPropertyIdRef argsName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"Arguments", &argsName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(wscript, argsName, argsObject, true), false);

    JsValueRef quit;
    IfJsrtErrorFail(ChakraRTInterface::JsCreateFunction(QuitCallback, nullptr, &quit), false);
    JsPropertyIdRef quitName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"Quit", &quitName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(wscript, quitName, quit, true), false);

    JsValueRef loadScriptFile;
    IfJsrtErrorFail(ChakraRTInterface::JsCreateFunction(LoadScriptFileCallback, nullptr, &loadScriptFile), false);
    JsPropertyIdRef loadScriptFileName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"LoadScriptFile", &loadScriptFileName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(wscript, loadScriptFileName, loadScriptFile, true), false);

    JsValueRef loadScript;
    IfJsrtErrorFail(ChakraRTInterface::JsCreateFunction(LoadScriptCallback, nullptr, &loadScript), false);
    JsPropertyIdRef loadScriptName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"LoadScript", &loadScriptName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(wscript, loadScriptName, loadScript, true), false);

    JsValueRef setTimeout;
    IfJsrtErrorFail(ChakraRTInterface::JsCreateFunction(SetTimeoutCallback, nullptr, &setTimeout), false);
    JsPropertyIdRef setTimeoutName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"SetTimeout", &setTimeoutName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(wscript, setTimeoutName, setTimeout, true), false);

    JsValueRef clearTimeout;
    IfJsrtErrorFail(ChakraRTInterface::JsCreateFunction(ClearTimeoutCallback, nullptr, &clearTimeout), false);
    JsPropertyIdRef clearTimeoutName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"ClearTimeout", &clearTimeoutName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(wscript, clearTimeoutName, clearTimeout, true), false);

    JsPropertyIdRef wscriptName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"WScript", &wscriptName), false);
    JsValueRef global;
    IfJsrtErrorFail(ChakraRTInterface::JsGetGlobalObject(&global), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(global, wscriptName, wscript, true), false);

    JsPropertyIdRef printName;
    IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"print", &printName), false);
    IfJsrtErrorFail(ChakraRTInterface::JsSetProperty(global, printName, echo, true), false);

    return true;
}

bool WScriptJsrt::PrintException(LPCWSTR fileName, JsErrorCode jsErrorCode)
{
    LPCWSTR errorTypeString = ConvertErrorCodeToMessage(jsErrorCode);
    JsValueRef exception;
    ChakraRTInterface::JsGetAndClearException(&exception);
    if (exception != nullptr)
    {
        if (jsErrorCode == JsErrorCode::JsErrorScriptCompile || jsErrorCode == JsErrorCode::JsErrorScriptException)
        {
            JsPropertyIdRef messagePropertyId = JS_INVALID_REFERENCE;
            JsValueRef messageProperty = JS_INVALID_REFERENCE;

            JsPropertyIdRef linePropertyId = JS_INVALID_REFERENCE;
            JsValueRef lineProperty = JS_INVALID_REFERENCE;

            JsPropertyIdRef columnPropertyId = JS_INVALID_REFERENCE;
            JsValueRef columnProperty = JS_INVALID_REFERENCE;

            LPCWSTR errorMessage = nullptr;
            size_t errorMessageLength = 0;

            IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"message", &messagePropertyId), false);
            IfJsrtErrorFail(ChakraRTInterface::JsGetProperty(exception, messagePropertyId, &messageProperty), false);
            IfJsrtErrorFail(ChakraRTInterface::JsStringToPointer(messageProperty, &errorMessage, &errorMessageLength), false);

            if (jsErrorCode == JsErrorCode::JsErrorScriptCompile)
            {
                int line;
                int column;
                IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"line", &linePropertyId), false);
                IfJsrtErrorFail(ChakraRTInterface::JsGetProperty(exception, linePropertyId, &lineProperty), false);
                IfJsrtErrorFail(ChakraRTInterface::JsNumberToInt(lineProperty, &line), false);

                IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"column", &columnPropertyId), false);
                IfJsrtErrorFail(ChakraRTInterface::JsGetProperty(exception, columnPropertyId, &columnProperty), false);
                IfJsrtErrorFail(ChakraRTInterface::JsNumberToInt(columnProperty, &column), false);

                WCHAR shortFileName[_MAX_PATH];
                WCHAR ext[_MAX_EXT];
                _wsplitpath_s(fileName, nullptr, 0, nullptr, 0, shortFileName, _countof(shortFileName), ext, _countof(ext));
                fwprintf(stderr, L"%ls: %ls\n\tat code (%ls%ls:%d:%d)\n", errorTypeString, errorMessage, shortFileName, ext, (int)line + 1, (int)column + 1);
            }
            else
            {
                JsPropertyIdRef stackPropertyId = JS_INVALID_REFERENCE;
                JsValueRef stackProperty = JS_INVALID_REFERENCE;
                LPCWSTR errorStack = nullptr;
                size_t errorStackLength = 0;

                IfJsrtErrorFail(ChakraRTInterface::JsGetPropertyIdFromName(L"stack", &stackPropertyId), false);
                IfJsrtErrorFail(ChakraRTInterface::JsGetProperty(exception, stackPropertyId, &stackProperty), false);
                IfJsrtErrorFail(ChakraRTInterface::JsStringToPointer(stackProperty, &errorStack, &errorStackLength), false);

                fwprintf(stderr, L"%ls\n", errorStack);
                // TODO : we need to extract the stack and print it.
            }
        }
        else
        {
            fwprintf(stderr, L"Error : %ls\n", errorTypeString);
        }
        return true;
    }
    else
    {
        fwprintf(stderr, L"Error : %ls\n", errorTypeString);
    }
    return false;
}

void WScriptJsrt::AddMessageQueue(MessageQueue *messageQueue)
{
    Assert(s_messageQueue == nullptr);

    s_messageQueue = messageQueue;
}

WScriptJsrt::CallbackMessage::CallbackMessage(unsigned int time, JsValueRef function) : MessageBase(time), m_function(function)
{
    ChakraRTInterface::JsAddRef(m_function, nullptr);
}

WScriptJsrt::CallbackMessage::~CallbackMessage()
{
    ChakraRTInterface::JsRelease(m_function, nullptr);
    m_function = JS_INVALID_REFERENCE;
}

HRESULT WScriptJsrt::CallbackMessage::Call()
{
    HRESULT hr = S_OK;

    JsValueRef global;
    JsValueRef result;
    JsValueRef stringValue;
    JsValueType type;

    IfJsrtErrorHR(ChakraRTInterface::JsGetGlobalObject(&global));
    IfJsrtErrorHR(ChakraRTInterface::JsGetValueType(m_function, &type));

    if (type == JsString)
    {
        LPCWSTR script = nullptr;
        size_t length = 0;

        IfJsrtErrorHR(ChakraRTInterface::JsConvertValueToString(m_function, &stringValue));
        IfJsrtErrorHR(ChakraRTInterface::JsStringToPointer(stringValue, &script, &length));

        // Run the code
        IfJsrtErrorHR(ChakraRTInterface::JsRunScript(script, JS_SOURCE_CONTEXT_NONE, L"" /*sourceUrl*/, nullptr /*no result needed*/));
    }
    else
    {
        IfJsrtErrorHR(ChakraRTInterface::JsCallFunction(m_function, &global, 1, &result));
    }

Error:
    if(FAILED(hr))
    {
        JsValueRef exception;
        JsValueRef strExcep;
        LPCWSTR msg;
        size_t length;
        IfJsrtErrorFail(ChakraRTInterface::JsGetAndClearException(&exception), E_FAIL);
        IfJsrtErrorFail(ChakraRTInterface::JsConvertValueToString(exception, &strExcep), E_FAIL);
        IfJsrtErrorFail(ChakraRTInterface::JsStringToPointer(strExcep, &msg, &length), E_FAIL);

        wprintf(L"Script Error: %ls\n", msg);        
    }
    return hr;
}

