//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#include "StdAfx.h"

JsAPIHooks ChakraRTInterface::m_jsApiHooks = { 0 };

/*static*/
void ChakraRTInterface::Initialize(HINSTANCE library)
{
    if (library)
    {
        m_jsApiHooks.pfJsrtCreateRuntime = (JsAPIHooks::JsrtCreateRuntimePtr)GetProcAddress(library, "JsCreateRuntime");
        m_jsApiHooks.pfJsrtCreateContext = (JsAPIHooks::JsrtCreateContextPtr)GetProcAddress(library, "JsCreateContext");
        m_jsApiHooks.pfJsrtSetCurrentContext = (JsAPIHooks::JsrtSetCurrentContextPtr)GetProcAddress(library, "JsSetCurrentContext");
        m_jsApiHooks.pfJsrtGetCurrentContext = (JsAPIHooks::JsrtGetCurrentContextPtr)GetProcAddress(library, "JsGetCurrentContext");
        m_jsApiHooks.pfJsrtDisposeRuntime = (JsAPIHooks::JsrtDisposeRuntimePtr)GetProcAddress(library, "JsDisposeRuntime");
        m_jsApiHooks.pfJsrtCreateObject = (JsAPIHooks::JsrtCreateObjectPtr)GetProcAddress(library, "JsCreateObject");
        m_jsApiHooks.pfJsrtCreateExternalObject = (JsAPIHooks::JsrtCreateExternalObjectPtr)GetProcAddress(library, "JsCreateExternalObject");
        m_jsApiHooks.pfJsrtCreateFunction = (JsAPIHooks::JsrtCreateFunctionPtr)GetProcAddress(library, "JsCreateFunction");
        m_jsApiHooks.pfJsrtPointerToString = (JsAPIHooks::JsrtPointerToStringPtr)GetProcAddress(library, "JsPointerToString");
        m_jsApiHooks.pfJsrtSetProperty = (JsAPIHooks::JsrtSetPropertyPtr)GetProcAddress(library, "JsSetProperty");
        m_jsApiHooks.pfJsrtGetGlobalObject = (JsAPIHooks::JsrtGetGlobalObjectPtr)GetProcAddress(library, "JsGetGlobalObject");
        m_jsApiHooks.pfJsrtGetUndefinedValue = (JsAPIHooks::JsrtGetUndefinedValuePtr)GetProcAddress(library, "JsGetUndefinedValue");
        m_jsApiHooks.pfJsrtGetTrueValue = (JsAPIHooks::JsrtGetUndefinedValuePtr)GetProcAddress(library, "JsGetTrueValue");
        m_jsApiHooks.pfJsrtGetFalseValue = (JsAPIHooks::JsrtGetUndefinedValuePtr)GetProcAddress(library, "JsGetFalseValue");
        m_jsApiHooks.pfJsrtConvertValueToString = (JsAPIHooks::JsrtConvertValueToStringPtr)GetProcAddress(library, "JsConvertValueToString");
        m_jsApiHooks.pfJsrtConvertValueToNumber = (JsAPIHooks::JsrtConvertValueToNumberPtr)GetProcAddress(library, "JsConvertValueToNumber");
        m_jsApiHooks.pfJsrtConvertValueToBoolean = (JsAPIHooks::JsrtConvertValueToBooleanPtr)GetProcAddress(library, "JsConvertValueToBoolean");
        m_jsApiHooks.pfJsrtStringToPointer = (JsAPIHooks::JsrtStringToPointerPtr)GetProcAddress(library, "JsStringToPointer");
        m_jsApiHooks.pfJsrtBooleanToBool = (JsAPIHooks::JsrtBooleanToBoolPtr)GetProcAddress(library, "JsBooleanToBool");
        m_jsApiHooks.pfJsrtGetPropertyIdFromName = (JsAPIHooks::JsrtGetPropertyIdFromNamePtr)GetProcAddress(library, "JsGetPropertyIdFromName");
        m_jsApiHooks.pfJsrtGetProperty = (JsAPIHooks::JsrtGetPropertyPtr)GetProcAddress(library, "JsGetProperty");
        m_jsApiHooks.pfJsrtHasProperty = (JsAPIHooks::JsrtHasPropertyPtr)GetProcAddress(library, "JsHasProperty");
        m_jsApiHooks.pfJsrtRunScript = (JsAPIHooks::JsrtRunScriptPtr)GetProcAddress(library, "JsRunScript");
        m_jsApiHooks.pfJsrtCallFunction = (JsAPIHooks::JsrtCallFunctionPtr)GetProcAddress(library, "JsCallFunction");
        m_jsApiHooks.pfJsrtNumbertoDouble = (JsAPIHooks::JsrtNumberToDoublePtr)GetProcAddress(library, "JsNumberToDouble");
        m_jsApiHooks.pfJsrtNumbertoInt = (JsAPIHooks::JsrtNumberToIntPtr)GetProcAddress(library, "JsNumberToInt");
        m_jsApiHooks.pfJsrtDoubleToNumber = (JsAPIHooks::JsrtDoubleToNumberPtr)GetProcAddress(library, "JsDoubleToNumber");
        m_jsApiHooks.pfJsrtGetExternalData = (JsAPIHooks::JsrtGetExternalDataPtr)GetProcAddress(library, "JsGetExternalData");
        m_jsApiHooks.pfJsrtCreateArray = (JsAPIHooks::JsrtCreateArrayPtr)GetProcAddress(library, "JsCreateArray");
        m_jsApiHooks.pfJsrtSetException = (JsAPIHooks::JsrtSetExceptionPtr)GetProcAddress(library, "JsSetException");
        m_jsApiHooks.pfJsrtGetAndClearException = (JsAPIHooks::JsrtGetAndClearExceptiopnPtr)GetProcAddress(library, "JsGetAndClearException");
        m_jsApiHooks.pfJsrtCreateError = (JsAPIHooks::JsrtCreateErrorPtr)GetProcAddress(library, "JsCreateError");
        m_jsApiHooks.pfJsrtGetRuntime = (JsAPIHooks::JsrtGetRuntimePtr)GetProcAddress(library, "JsGetRuntime");
        m_jsApiHooks.pfJsrtRelease = (JsAPIHooks::JsrtReleasePtr)GetProcAddress(library, "JsRelease");
        m_jsApiHooks.pfJsrtAddRef = (JsAPIHooks::JsrtAddRefPtr)GetProcAddress(library, "JsAddRef");
        m_jsApiHooks.pfJsrtGetValueType = (JsAPIHooks::JsrtGetValueType)GetProcAddress(library, "JsGetValueType");
    }
}

