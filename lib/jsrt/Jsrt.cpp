//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "JsrtPch.h"
#include "jsrtInternal.h"
#include "JsrtExternalObject.h"
#include "JsrtExternalArrayBuffer.h"

#include "JsrtSourceHolder.h"
#include "Language\ByteCodeSerializer.h"
#include "common\ByteSwap.h"
#include "Library\dataview.h"
#include "Library\JavascriptSymbol.h"
#include "Library\ThreadContextTLSEntry.h"

// Parser Includes
#include "cmperr.h"     // For ERRnoMemory
#include "screrror.h"   // For CompileScriptException

JsErrorCode CheckContext(JsrtContext *currentContext, bool verifyRuntimeState, bool allowInObjectBeforeCollectCallback)
{
    if (currentContext == nullptr)
    {
        return JsErrorNoCurrentContext;
    }

    Js::ScriptContext *scriptContext = currentContext->GetScriptContext();
    Assert(scriptContext != nullptr);
    Recycler *recycler = scriptContext->GetRecycler();
    ThreadContext *threadContext = scriptContext->GetThreadContext();

    // We don't need parameter check if it's checked in previous wrapper.
    if (verifyRuntimeState)
    {
        if (recycler && recycler->IsHeapEnumInProgress())
        {
            return JsErrorHeapEnumInProgress;
        }
        else if (!allowInObjectBeforeCollectCallback && recycler && recycler->IsInObjectBeforeCollectCallback())
        {
            return JsErrorInObjectBeforeCollectCallback;
        }
        else if (threadContext->IsExecutionDisabled())
        {
            return JsErrorInDisabledState;
        }
        else if (scriptContext->IsInProfileCallback())
        {
            return JsErrorInProfileCallback;
        }
        else if (threadContext->IsInThreadServiceCallback())
        {
            return JsErrorInThreadServiceCallback;
        }

        // Make sure we don't have an outstanding exception.
        if (scriptContext->GetThreadContext()->GetRecordedException() != nullptr)
        {
            return JsErrorInExceptionState;
        }
    }

    return JsNoError;
}

template <class Fn, class T>
T CallbackWrapper(Fn fn, T default)
{
    T result = default;
    try
    {
        AUTO_NESTED_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));

        result = fn();
    }
    catch (Js::OutOfMemoryException)
    {
    }
    catch (Js::StackOverflowException)
    {
    }
    catch (Js::ExceptionBase)
    {
        AssertMsg(false, "Unexpected engine exception.");
    }
    catch (...)
    {
        AssertMsg(false, "Unexpected non-engine exception.");
    }

    return result;
}

template <class Fn>
bool CallbackWrapper(Fn fn)
{
    return CallbackWrapper(fn, false);
}

STDAPI_(JsErrorCode) JsCreateRuntime(JsRuntimeAttributes attributes, JsThreadServiceCallback threadService, JsRuntimeHandle *runtimeHandle)
{
    return GlobalAPIWrapper([&] () -> JsErrorCode {
        PARAM_NOT_NULL(runtimeHandle);
        *runtimeHandle = nullptr;

        const JsRuntimeAttributes JsRuntimeAttributesAll =
            (JsRuntimeAttributes)(
            JsRuntimeAttributeDisableBackgroundWork |
            JsRuntimeAttributeAllowScriptInterrupt |
            JsRuntimeAttributeEnableIdleProcessing |
            JsRuntimeAttributeDisableEval |
            JsRuntimeAttributeDisableNativeCodeGeneration |
            JsRuntimeAttributeEnableExperimentalFeatures |
            JsRuntimeAttributeDispatchSetExceptionsToDebugger
            );

        Assert((attributes & ~JsRuntimeAttributesAll) == 0);
        if ((attributes & ~JsRuntimeAttributesAll) != 0)
        {
            return JsErrorInvalidArgument;
        }

        AllocationPolicyManager * policyManager = HeapNew(AllocationPolicyManager, (attributes & JsRuntimeAttributeDisableBackgroundWork) == 0);
        ThreadContext * threadContext = HeapNew(ThreadContext, policyManager, threadService);

        if (((attributes & JsRuntimeAttributeDisableBackgroundWork) == 0)
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            || Js::Configuration::Global.flags.ConcurrentRuntime
#endif
            )
        {
            threadContext->OptimizeForManyInstances(false);
            threadContext->EnableBgJit(true);
        }
        else
        {
            threadContext->OptimizeForManyInstances(true);
            threadContext->EnableBgJit(false);
        }

        if (!threadContext->IsRentalThreadingEnabledInJSRT()
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            || Js::Configuration::Global.flags.DisableRentalThreading
#endif
            )
        {
            threadContext->SetIsThreadBound();
        }

        if (attributes & JsRuntimeAttributeAllowScriptInterrupt)
        {
            threadContext->SetThreadContextFlag(ThreadContextFlagCanDisableExecution);
        }

        if (attributes & JsRuntimeAttributeDisableEval)
        {
            threadContext->SetThreadContextFlag(ThreadContextFlagEvalDisabled);
        }

        if (attributes & JsRuntimeAttributeDisableNativeCodeGeneration)
        {
            threadContext->SetThreadContextFlag(ThreadContextFlagNoJIT);
        }

        if (attributes & JsRuntimeAttributeEnableExperimentalFeatures)
        {
            threadContext->SetThreadContextFlag(ThreadContextFlagExperimentalFeaturesEnabled);
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (Js::Configuration::Global.flags.PrimeRecycler)
        {
            threadContext->EnsureRecycler()->Prime();
        }
#endif

        bool enableIdle = (attributes & JsRuntimeAttributeEnableIdleProcessing) == JsRuntimeAttributeEnableIdleProcessing;
        bool dispatchExceptions = (attributes & JsRuntimeAttributeDispatchSetExceptionsToDebugger) == JsRuntimeAttributeDispatchSetExceptionsToDebugger;

        JsrtRuntime * runtime = HeapNew(JsrtRuntime, threadContext, enableIdle, dispatchExceptions);
        threadContext->SetCurrentThreadId(ThreadContext::NoThread);
        *runtimeHandle = runtime->ToHandle();

        return JsNoError;
    });
}

template <CollectionFlags flags>
JsErrorCode JsCollectGarbageCommon(JsRuntimeHandle runtimeHandle)
{
    return GlobalAPIWrapper([&]() -> JsErrorCode {
        VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);

        ThreadContext * threadContext = JsrtRuntime::FromHandle(runtimeHandle)->GetThreadContext();

        if (threadContext->GetRecycler() && threadContext->GetRecycler()->IsHeapEnumInProgress())
        {
            return JsErrorHeapEnumInProgress;
        }
        else if (threadContext->IsInThreadServiceCallback())
        {
            return JsErrorInThreadServiceCallback;
        }

        ThreadContextScope scope(threadContext);

        if (!scope.IsValid())
        {
            return JsErrorWrongThread;
        }

        Recycler* recycler = threadContext->EnsureRecycler();
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        if (flags & CollectOverride_SkipStack)
        {
            Recycler::AutoEnterExternalStackSkippingGCMode autoGC(recycler);
            recycler->CollectNow<flags>();
        }
        else
#endif
        {
            recycler->CollectNow<flags>();
        }

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCollectGarbage(JsRuntimeHandle runtimeHandle)
{
    return JsCollectGarbageCommon<CollectNowExhaustive>(runtimeHandle);
}

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
STDAPI_(JsErrorCode) JsPrivateCollectGarbageSkipStack(JsRuntimeHandle runtimeHandle)
{
    return JsCollectGarbageCommon<CollectNowExhaustiveSkipStack>(runtimeHandle);
}
#endif

STDAPI_(JsErrorCode) JsDisposeRuntime(JsRuntimeHandle runtimeHandle)
{
    return GlobalAPIWrapper([&] () -> JsErrorCode {
        VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);

        JsrtRuntime * runtime = JsrtRuntime::FromHandle(runtimeHandle);
        ThreadContext * threadContext = runtime->GetThreadContext();
        ThreadContextScope scope(threadContext);

        // We should not dispose if the runtime is being used.
        if (!scope.IsValid() ||
            scope.WasInUse() ||
            (threadContext->GetRecycler() && threadContext->GetRecycler()->IsHeapEnumInProgress()))
        {
            return JsErrorRuntimeInUse;
        }
        else if (threadContext->IsInThreadServiceCallback())
        {
            return JsErrorInThreadServiceCallback;
        }

        // Invoke and clear the callbacks while the contexts and runtime are still available
        Recycler* recycler = threadContext->GetRecycler();
        if (recycler != nullptr)
        {
            recycler->ClearObjectBeforeCollectCallbacks();
        }

        // Close any open Contexts.
        // We need to do this before recycler shutdown, because ScriptEngine->Close won't work then.
        runtime->CloseContexts();

#if defined(CHECK_MEMORY_LEAK) || defined(LEAK_REPORT)
        bool doFinalGC = false;

#if defined(LEAK_REPORT)
        if (Js::Configuration::Global.flags.IsEnabled(Js::LeakReportFlag))
        {
            doFinalGC = true;
        }
#endif

#if defined(CHECK_MEMORY_LEAK)
        if (Js::Configuration::Global.flags.CheckMemoryLeak)
        {
            doFinalGC = true;
        }
#endif

        if (doFinalGC)
        {
            Recycler *recycler = threadContext->GetRecycler();
            if (recycler)
            {
                recycler->EnsureNotCollecting();
                recycler->CollectNow<CollectNowFinalGC>();
                Assert(!recycler->CollectionInProgress());
            }
        }
#endif

        runtime->SetBeforeCollectCallback(nullptr, nullptr);
        threadContext->CloseForJSRT();
        HeapDelete(threadContext);

        HeapDelete(runtime);

        scope.Invalidate();

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsAddRef(JsRef ref, unsigned int *count)
{
    VALIDATE_JSREF(ref);
    if (count != nullptr)
    {
        *count = 0;
    }

    if (Js::TaggedNumber::Is(ref))
    {
        // The count is always one because these are never collected
        if (count)
        {
            *count = 1;
        }
        return JsNoError;
    }

    if (JsrtContext::Is(ref))
    {
        return GlobalAPIWrapper([&] () -> JsErrorCode
        {
            Recycler * recycler = static_cast<JsrtContext *>(ref)->GetRuntime()->GetThreadContext()->GetRecycler();
            recycler->RootAddRef(ref, count);
            return JsNoError;
        });
    }
    else
    {
        return ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode
        {
            Recycler * recycler = scriptContext->GetRecycler();

            // Note, some references may live in arena-allocated memory, so we need to do this check
            if (!recycler->IsValidObject(ref))
            {
                return JsNoError;
            }

            recycler->RootAddRef(ref, count);
            return JsNoError;
        },
        /*allowInObjectBeforeCollectCallback*/true);
    }
}

STDAPI_(JsErrorCode) JsRelease(JsRef ref, unsigned int *count)
{
    VALIDATE_JSREF(ref);
    if (count != nullptr)
    {
        *count = 0;
    }

    if (Js::TaggedNumber::Is(ref))
    {
        // The count is always one because these are never collected
        if (count)
        {
            *count = 1;
        }
        return JsNoError;
    }

    if (JsrtContext::Is(ref))
    {
        return GlobalAPIWrapper([&] () -> JsErrorCode
        {
            Recycler * recycler = static_cast<JsrtContext *>(ref)->GetRuntime()->GetThreadContext()->GetRecycler();
            recycler->RootRelease(ref, count);
            return JsNoError;
        });
    }
    else
    {
        return ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode
        {
            Recycler * recycler = scriptContext->GetRecycler();

            // Note, some references may live in arena-allocated memory, so we need to do this check
            if (!recycler->IsValidObject(ref))
            {
                return JsNoError;
            }
            recycler->RootRelease(ref, count);
            return JsNoError;
        },
        /*allowInObjectBeforeCollectCallback*/true);
    }
}

STDAPI_(JsErrorCode) JsSetObjectBeforeCollectCallback(JsRef ref, void *callbackState, JsObjectBeforeCollectCallback objectBeforeCollectCallback)
{
    VALIDATE_JSREF(ref);

    if (Js::TaggedNumber::Is(ref))
    {
        return JsErrorInvalidArgument;
    }

    if (JsrtContext::Is(ref))
    {
        return GlobalAPIWrapper([&]() -> JsErrorCode
        {
            Recycler * recycler = static_cast<JsrtContext *>(ref)->GetRuntime()->GetThreadContext()->GetRecycler();
            recycler->SetObjectBeforeCollectCallback(ref, reinterpret_cast<Recycler::ObjectBeforeCollectCallback>(objectBeforeCollectCallback), callbackState);
            return JsNoError;
        });
    }
    else
    {
        return ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode
        {
            Recycler * recycler = scriptContext->GetRecycler();
            if (!recycler->IsValidObject(ref))
            {
                return JsErrorInvalidArgument;
            }

            recycler->SetObjectBeforeCollectCallback(ref, reinterpret_cast<Recycler::ObjectBeforeCollectCallback>(objectBeforeCollectCallback), callbackState);
            return JsNoError;
        },
        /*allowInObjectBeforeCollectCallback*/true);
    }
}

STDAPI_(JsErrorCode) JsCreateContext(JsRuntimeHandle runtimeHandle, JsContextRef *newContext)
{
    return GlobalAPIWrapper([&]() -> JsErrorCode {
        PARAM_NOT_NULL(newContext);
        VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);

        JsrtRuntime * runtime = JsrtRuntime::FromHandle(runtimeHandle);
        ThreadContext * threadContext = runtime->GetThreadContext();

        if (threadContext->GetRecycler() && threadContext->GetRecycler()->IsHeapEnumInProgress())
        {
            return JsErrorHeapEnumInProgress;
        }
        else if (threadContext->IsInThreadServiceCallback())
        {
            return JsErrorInThreadServiceCallback;
        }

        ThreadContextScope scope(threadContext);

        if (!scope.IsValid())
        {
            return JsErrorWrongThread;
        }

        JsrtContext * context = JsrtContext::New(runtime);

        *newContext = (JsContextRef)context;
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetCurrentContext(JsContextRef *currentContext)
{
    PARAM_NOT_NULL(currentContext);

    BEGIN_JSRT_NO_EXCEPTION
    {
      *currentContext = (JsContextRef)JsrtContext::GetCurrent();
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsSetCurrentContext(JsContextRef newContext)
{
    return GlobalAPIWrapper([&] () -> JsErrorCode {
        JsrtContext *currentContext = JsrtContext::GetCurrent();

        if (currentContext && currentContext->GetScriptContext()->GetRecycler()->IsHeapEnumInProgress())
        {
            return JsErrorHeapEnumInProgress;
        }
        else if (currentContext && currentContext->GetRuntime()->GetThreadContext()->IsInThreadServiceCallback())
        {
            return JsErrorInThreadServiceCallback;
        }

        if (!JsrtContext::TrySetCurrent((JsrtContext *)newContext))
        {
            return JsErrorWrongThread;
        }

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetContextOfObject(JsValueRef object, JsContextRef *context)
{
    VALIDATE_JSREF(object);
    PARAM_NOT_NULL(context);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if(!Js::RecyclableObject::Is(object))
        {
            RETURN_NO_EXCEPTION(JsErrorArgumentNotObject);
        }
        Js::RecyclableObject* obj = Js::RecyclableObject::FromVar(object);
        *context = (JsContextRef)obj->GetScriptContext()->GetLibrary()->GetPinnedJsrtContextObject();
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsGetContextData(JsContextRef context, void **data)
{
    VALIDATE_JSREF(context);
    PARAM_NOT_NULL(data);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (!JsrtContext::Is(context))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        *data = static_cast<JsrtContext *>(context)->GetExternalData();
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsSetContextData(_In_ JsContextRef context, _In_ void *data)
{
    VALIDATE_JSREF(context);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (!JsrtContext::Is(context))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        static_cast<JsrtContext *>(context)->SetExternalData(data);
    }
    END_JSRT_NO_EXCEPTION
}

void HandleScriptCompileError(Js::ScriptContext * scriptContext, CompileScriptException * se)
{
    HRESULT hr = se->ei.scode;
    if (hr == E_OUTOFMEMORY || hr == VBSERR_OutOfMemory || hr == VBSERR_OutOfStack || hr == ERRnoMemory)
    {
        Js::Throw::OutOfMemory();
    }

    Js::JavascriptError * error = Js::JavascriptError::MapParseError(scriptContext, hr);
    const Js::PropertyRecord *record;

    Js::Var value = Js::JavascriptString::NewCopySz(se->ei.bstrDescription, scriptContext);
    scriptContext->GetOrAddPropertyRecord(L"message", wcslen(L"message"), &record);
    Js::JavascriptOperators::OP_SetProperty(error, record->GetPropertyId(), value, scriptContext);

    scriptContext->GetOrAddPropertyRecord(L"line", wcslen(L"line"), &record);
    if (se->hasLineNumberInfo)
    {
        value = Js::JavascriptNumber::New(se->line, scriptContext);
        Js::JavascriptOperators::OP_SetProperty(error, record->GetPropertyId(), value, scriptContext);
    }

    scriptContext->GetOrAddPropertyRecord(L"column", wcslen(L"column"), &record);
    if (se->hasLineNumberInfo)
    {
        value = Js::JavascriptNumber::New(se->ichMin - se->ichMinLine, scriptContext);
        Js::JavascriptOperators::OP_SetProperty(error, record->GetPropertyId(), value, scriptContext);
    }

    scriptContext->GetOrAddPropertyRecord(L"length", wcslen(L"length"), &record);
    if (se->hasLineNumberInfo)
    {
        value = Js::JavascriptNumber::New(se->ichLim - se->ichMin, scriptContext);
        Js::JavascriptOperators::OP_SetProperty(error, record->GetPropertyId(), value, scriptContext);
    }

    scriptContext->GetOrAddPropertyRecord(L"source", wcslen(L"source"), &record);
    if (se->bstrLine != nullptr)
    {
        value = Js::JavascriptString::NewCopySz(se->bstrLine, scriptContext);
        Js::JavascriptOperators::OP_SetProperty(error, record->GetPropertyId(), value, scriptContext);
    }

    Js::JavascriptExceptionObject * exceptionObject = RecyclerNew(scriptContext->GetRecycler(),
        Js::JavascriptExceptionObject, error, scriptContext, nullptr);

    scriptContext->GetThreadContext()->SetRecordedException(exceptionObject);
}

STDAPI_(JsErrorCode) JsGetUndefinedValue(JsValueRef *undefinedValue)
{
    return ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(undefinedValue);

        *undefinedValue = scriptContext->GetLibrary()->GetUndefined();

        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

STDAPI_(JsErrorCode) JsGetNullValue(JsValueRef *nullValue)
{
    return ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(nullValue);

        *nullValue = scriptContext->GetLibrary()->GetNull();

        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

STDAPI_(JsErrorCode) JsGetTrueValue(JsValueRef *trueValue)
{
    return ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(trueValue);

        *trueValue = scriptContext->GetLibrary()->GetTrue();

        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

STDAPI_(JsErrorCode) JsGetFalseValue(JsValueRef *falseValue)
{
    return ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(falseValue);

        *falseValue = scriptContext->GetLibrary()->GetFalse();

        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

STDAPI_(JsErrorCode) JsBoolToBoolean(bool value, JsValueRef *booleanValue)
{
    return ContextAPINoScriptWrapper([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(booleanValue);

        *booleanValue = value ? scriptContext->GetLibrary()->GetTrue() :
            scriptContext->GetLibrary()->GetFalse();
        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

STDAPI_(JsErrorCode) JsBooleanToBool(JsValueRef value, bool *boolValue)
{
    VALIDATE_JSREF(value);
    PARAM_NOT_NULL(boolValue);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (!Js::JavascriptBoolean::Is(value))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        *boolValue = Js::JavascriptBoolean::FromVar(value)->GetValue() ? true : false;
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsConvertValueToBoolean(JsValueRef value, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(value, scriptContext);
        PARAM_NOT_NULL(result);

        if (Js::JavascriptConversion::ToBool((Js::Var)value, scriptContext))
        {
            *result = scriptContext->GetLibrary()->GetTrue();
            return JsNoError;
        }
        else
        {
            *result = scriptContext->GetLibrary()->GetFalse();
            return JsNoError;
        }
    });
}

STDAPI_(JsErrorCode) JsGetValueType(JsValueRef value, JsValueType *type)
{
    VALIDATE_JSREF(value);
    PARAM_NOT_NULL(type);

    BEGIN_JSRT_NO_EXCEPTION
    {
        Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(value);
        switch (typeId)
        {
        case Js::TypeIds_Undefined:
            *type = JsUndefined;
            break;
        case Js::TypeIds_Null:
            *type = JsNull;
            break;
        case Js::TypeIds_Boolean:
            *type = JsBoolean;
            break;
        case Js::TypeIds_Integer:
        case Js::TypeIds_Number:
        case Js::TypeIds_Int64Number:
        case Js::TypeIds_UInt64Number:
            *type = JsNumber;
            break;
        case Js::TypeIds_String:
            *type = JsString;
            break;
        case Js::TypeIds_Function:
            *type = JsFunction;
            break;
        case Js::TypeIds_Error:
            *type = JsError;
            break;
        case Js::TypeIds_Array:
        case Js::TypeIds_NativeIntArray:
        case Js::TypeIds_CopyOnAccessNativeIntArray:
        case Js::TypeIds_NativeFloatArray:
        case Js::TypeIds_ES5Array:
            *type = JsArray;
            break;
        case Js::TypeIds_Symbol:
            *type = JsSymbol;
            break;
        case Js::TypeIds_ArrayBuffer:
            *type = JsArrayBuffer;
            break;
        case Js::TypeIds_DataView:
            *type = JsDataView;
            break;
        default:
            if (Js::TypedArrayBase::Is(typeId))
            {
                *type = JsTypedArray;
            }
            else
            {
                *type = JsObject;
            }
            break;
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsDoubleToNumber(double dbl, JsValueRef *asValue)
{
    PARAM_NOT_NULL(asValue);
    if (Js::JavascriptNumber::TryToVarFastWithCheck(dbl, asValue)) {
      return JsNoError;
    }

    return ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        *asValue = Js::JavascriptNumber::ToVarNoCheck(dbl, scriptContext);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsIntToNumber(int intValue, JsValueRef *asValue)
{
    PARAM_NOT_NULL(asValue);
    if (Js::JavascriptNumber::TryToVarFast(intValue, asValue))
    {
        return JsNoError;
    }

    return ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        *asValue = Js::JavascriptNumber::ToVar(intValue, scriptContext);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsNumberToDouble(JsValueRef value, double *asDouble)
{
    VALIDATE_JSREF(value);
    PARAM_NOT_NULL(asDouble);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (Js::TaggedInt::Is(value))
        {
            *asDouble = Js::TaggedInt::ToDouble(value);
        }
        else if (Js::JavascriptNumber::Is_NoTaggedIntCheck(value))
        {
            *asDouble = Js::JavascriptNumber::GetValue(value);
        }
        else
        {
            *asDouble = 0;
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsNumberToInt(JsValueRef value, int *asInt)
{
    VALIDATE_JSREF(value);
    PARAM_NOT_NULL(asInt);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (Js::TaggedInt::Is(value))
        {
            *asInt = Js::TaggedInt::ToInt32(value);
        }
        else if (Js::JavascriptNumber::Is_NoTaggedIntCheck(value))
        {
            *asInt = Js::JavascriptConversion::ToInt32(Js::JavascriptNumber::GetValue(value));
        }
        else
        {
            *asInt = 0;
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsConvertValueToNumber(JsValueRef value, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(value, scriptContext);
        PARAM_NOT_NULL(result);

        *result = (JsValueRef)Js::JavascriptOperators::ToNumber((Js::Var)value, scriptContext);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetStringLength(JsValueRef value, int *length)
{
    VALIDATE_JSREF(value);
    PARAM_NOT_NULL(length);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (!Js::JavascriptString::Is(value))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        *length = Js::JavascriptString::FromVar(value)->GetLengthAsSignedInt();
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsPointerToString(const wchar_t *stringValue, size_t stringLength, JsValueRef *string)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(stringValue);
        PARAM_NOT_NULL(string);

        *string = Js::JavascriptString::NewCopyBuffer(stringValue, stringLength, scriptContext);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsStringToPointer(JsValueRef stringValue, const wchar_t **stringPtr, size_t *stringLength)
{
    VALIDATE_JSREF(stringValue);
    PARAM_NOT_NULL(stringPtr);
    *stringPtr = nullptr;
    PARAM_NOT_NULL(stringLength);
    *stringLength = 0;

    if (!Js::JavascriptString::Is(stringValue))
    {
        return JsErrorInvalidArgument;
    }

    return GlobalAPIWrapper([&]() -> JsErrorCode {
        Js::JavascriptString *jsString = Js::JavascriptString::FromVar(stringValue);

        *stringPtr = jsString->GetSz();
        *stringLength = jsString->GetLength();
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsConvertValueToString(JsValueRef value, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(value, scriptContext);
        PARAM_NOT_NULL(result);
        *result = nullptr;

        *result = (JsValueRef) Js::JavascriptConversion::ToString((Js::Var)value, scriptContext);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetGlobalObject(JsValueRef *globalObject)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(globalObject);

        *globalObject = (JsValueRef)scriptContext->GetGlobalObject();
        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

STDAPI_(JsErrorCode) JsCreateObject(JsValueRef *object)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(object);

        *object = scriptContext->GetLibrary()->CreateObject();
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateExternalObject(void *data, JsFinalizeCallback finalizeCallback, JsValueRef *object)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(object);

        *object = RecyclerNewFinalized(scriptContext->GetRecycler(), JsrtExternalObject, RecyclerNew(scriptContext->GetRecycler(), JsrtExternalType, scriptContext, finalizeCallback), data);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsConvertValueToObject(JsValueRef value, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(value, scriptContext);
        PARAM_NOT_NULL(result);

        *result = (JsValueRef)Js::JavascriptOperators::ToObject((Js::Var)value, scriptContext);
        Assert(*result == nullptr || !Js::CrossSite::NeedMarshalVar(*result, scriptContext));

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetPrototype(JsValueRef object, JsValueRef *prototypeObject)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        PARAM_NOT_NULL(prototypeObject);

        *prototypeObject = (JsValueRef)Js::JavascriptOperators::OP_GetPrototype(object, scriptContext);
        Assert(*prototypeObject == nullptr || !Js::CrossSite::NeedMarshalVar(*prototypeObject, scriptContext));

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsSetPrototype(JsValueRef object, JsValueRef prototypeObject)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_OBJECT_OR_NULL(prototypeObject, scriptContext);

        // We're not allowed to set this.
        if (object == scriptContext->GetLibrary()->GetObjectPrototype())
        {
            return JsErrorInvalidArgument;
        }

        Js::JavascriptObject::ChangePrototype(Js::RecyclableObject::FromVar(object), Js::RecyclableObject::FromVar(prototypeObject), true, scriptContext);

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsInstanceOf(JsValueRef object, JsValueRef constructor, bool *result) {
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(object, scriptContext);
        VALIDATE_INCOMING_REFERENCE(constructor, scriptContext);
        PARAM_NOT_NULL(result);

        *result = Js::RecyclableObject::FromVar(constructor)->HasInstance(object, scriptContext) ? true : false;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetExtensionAllowed(JsValueRef object, bool *value)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        PARAM_NOT_NULL(value);
        *value = nullptr;

        *value = Js::RecyclableObject::FromVar(object)->IsExtensible() != 0;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsPreventExtension(JsValueRef object)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);

        Js::RecyclableObject::FromVar(object)->PreventExtensions();

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetProperty(JsValueRef object, JsPropertyIdRef propertyId, JsValueRef *value)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        PARAM_NOT_NULL(value);
        *value = nullptr;

        *value = Js::JavascriptOperators::OP_GetProperty((Js::Var)object, ((Js::PropertyRecord *)propertyId)->GetPropertyId(), scriptContext);
        Assert(*value == nullptr || !Js::CrossSite::NeedMarshalVar(*value, scriptContext));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetOwnPropertyDescriptor(JsValueRef object, JsPropertyIdRef propertyId, JsValueRef *propertyDescriptor)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        PARAM_NOT_NULL(propertyDescriptor);
        *propertyDescriptor = nullptr;

        Js::PropertyDescriptor propertyDescriptorValue;
        if (Js::JavascriptOperators::GetOwnPropertyDescriptor(Js::RecyclableObject::FromVar(object), ((Js::PropertyRecord *)propertyId)->GetPropertyId(), scriptContext, &propertyDescriptorValue))
        {
            *propertyDescriptor = Js::JavascriptOperators::FromPropertyDescriptor(propertyDescriptorValue, scriptContext);
        }
        else
        {
            *propertyDescriptor = scriptContext->GetLibrary()->GetUndefined();
        }
        Assert(*propertyDescriptor == nullptr || !Js::CrossSite::NeedMarshalVar(*propertyDescriptor, scriptContext));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetOwnPropertyNames(JsValueRef object, JsValueRef *propertyNames)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        PARAM_NOT_NULL(propertyNames);
        *propertyNames = nullptr;

        *propertyNames = Js::JavascriptOperators::GetOwnPropertyNames(object, scriptContext);
        Assert(*propertyNames == nullptr || !Js::CrossSite::NeedMarshalVar(*propertyNames, scriptContext));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetOwnPropertySymbols(JsValueRef object, JsValueRef *propertySymbols)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        PARAM_NOT_NULL(propertySymbols);

        *propertySymbols = Js::JavascriptOperators::GetOwnPropertySymbols(object, scriptContext);
        Assert(*propertySymbols == nullptr || !Js::CrossSite::NeedMarshalVar(*propertySymbols, scriptContext));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsSetProperty(JsValueRef object, JsPropertyIdRef propertyId, JsValueRef value, bool useStrictRules)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        VALIDATE_INCOMING_REFERENCE(value, scriptContext);

        Js::JavascriptOperators::OP_SetProperty(object, ((Js::PropertyRecord *)propertyId)->GetPropertyId(), value, scriptContext,
            nullptr, useStrictRules ? Js::PropertyOperation_StrictMode : Js::PropertyOperation_None);

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsHasProperty(JsValueRef object, JsPropertyIdRef propertyId, bool *hasProperty)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        PARAM_NOT_NULL(hasProperty);
        *hasProperty = nullptr;

        *hasProperty = Js::JavascriptOperators::OP_HasProperty(object, ((Js::PropertyRecord *)propertyId)->GetPropertyId(), scriptContext) != 0;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsDeleteProperty(JsValueRef object, JsPropertyIdRef propertyId, bool useStrictRules, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        PARAM_NOT_NULL(result);
        *result = nullptr;

        *result = Js::JavascriptOperators::OP_DeleteProperty((Js::Var)object, ((Js::PropertyRecord *)propertyId)->GetPropertyId(),
            scriptContext, useStrictRules ? Js::PropertyOperation_StrictMode : Js::PropertyOperation_None);
        Assert(*result == nullptr || !Js::CrossSite::NeedMarshalVar(*result, scriptContext));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsDefineProperty(JsValueRef object, JsPropertyIdRef propertyId, JsValueRef propertyDescriptor, bool *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        VALIDATE_INCOMING_OBJECT(propertyDescriptor, scriptContext);
        PARAM_NOT_NULL(result);
        *result = nullptr;

        Js::PropertyDescriptor propertyDescriptorValue;
        if (!Js::JavascriptOperators::ToPropertyDescriptor(propertyDescriptor, &propertyDescriptorValue, scriptContext))
        {
            return JsErrorInvalidArgument;
        }

        *result = Js::JavascriptOperators::DefineOwnPropertyDescriptor(
            Js::RecyclableObject::FromVar(object), ((Js::PropertyRecord *)propertyId)->GetPropertyId(), propertyDescriptorValue,
            true, scriptContext) != 0;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateArray(unsigned int length, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(result);
        *result = nullptr;

        *result = scriptContext->GetLibrary()->CreateArray(length);

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateArrayBuffer(unsigned int byteLength, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(result);

        Js::JavascriptLibrary* library = scriptContext->GetLibrary();
        *result = library->CreateArrayBuffer(byteLength);

        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*result));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateExternalArrayBuffer(void *data, unsigned int byteLength, JsFinalizeCallback finalizeCallback, void *callbackState, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(result);

        if (data == nullptr && byteLength > 0)
        {
            return JsErrorInvalidArgument;
        }

        Js::JavascriptLibrary* library = scriptContext->GetLibrary();
        *result = Js::JsrtExternalArrayBuffer::New(
            reinterpret_cast<BYTE*>(data),
            byteLength,
            finalizeCallback,
            callbackState,
            library->GetArrayBufferType());

        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*result));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateTypedArray(JsTypedArrayType arrayType, JsValueRef baseArray, unsigned int byteOffset, unsigned int elementLength, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(baseArray, scriptContext);
        PARAM_NOT_NULL(result);

        Js::JavascriptLibrary* library = scriptContext->GetLibrary();

        const bool fromArrayBuffer = (baseArray != JS_INVALID_REFERENCE && Js::ArrayBuffer::Is(baseArray));

        if (byteOffset != 0 && !fromArrayBuffer)
        {
            return JsErrorInvalidArgument;
        }

        if (elementLength != 0 && !(baseArray == JS_INVALID_REFERENCE || fromArrayBuffer))
        {
            return JsErrorInvalidArgument;
        }

        Js::JavascriptFunction* constructorFunc = nullptr;
        Js::Var values[4] =
        {
            library->GetUndefined(),
            baseArray != nullptr ? baseArray : Js::JavascriptNumber::ToVar(elementLength, scriptContext)
        };
        if (fromArrayBuffer)
        {
            values[2] = Js::JavascriptNumber::ToVar(byteOffset, scriptContext);
            values[3] = Js::JavascriptNumber::ToVar(elementLength, scriptContext);
        }

        Js::CallInfo info(Js::CallFlags_Value, fromArrayBuffer ? 4 : 2);
        Js::Arguments args(info, values);

        switch (arrayType)
        {
        case JsArrayTypeInt8:
            constructorFunc = library->GetInt8ArrayConstructor();
            break;
        case JsArrayTypeUint8:
            constructorFunc = library->GetUint8ArrayConstructor();
            break;
        case JsArrayTypeUint8Clamped:
            constructorFunc = library->GetUint8ClampedArrayConstructor();
            break;
        case JsArrayTypeInt16:
            constructorFunc = library->GetInt16ArrayConstructor();
            break;
        case JsArrayTypeUint16:
            constructorFunc = library->GetUint16ArrayConstructor();
            break;
        case JsArrayTypeInt32:
            constructorFunc = library->GetInt32ArrayConstructor();
            break;
        case JsArrayTypeUint32:
            constructorFunc = library->GetUint32ArrayConstructor();
            break;
        case JsArrayTypeFloat32:
            constructorFunc = library->GetFloat32ArrayConstructor();
            break;
        case JsArrayTypeFloat64:
            constructorFunc = library->GetFloat64ArrayConstructor();
            break;
        default:
            return JsErrorInvalidArgument;
        }

        *result = Js::JavascriptFunction::CallFunction<true>(constructorFunc, constructorFunc->GetEntryPoint(), args);

        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*result));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateDataView(JsValueRef arrayBuffer, unsigned int byteOffset, unsigned int byteLength, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(arrayBuffer, scriptContext);
        PARAM_NOT_NULL(result);

        if (!Js::ArrayBuffer::Is(arrayBuffer))
        {
            return JsErrorInvalidArgument;
        }

        Js::JavascriptLibrary* library = scriptContext->GetLibrary();
        *result = library->CreateDataView(Js::ArrayBuffer::FromVar(arrayBuffer), byteOffset, byteLength);

        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_OBJECT(*result));
        return JsNoError;
    });
}


C_ASSERT(JsArrayTypeUint8         - Js::TypeIds_Uint8Array        == JsArrayTypeInt8 - Js::TypeIds_Int8Array);
C_ASSERT(JsArrayTypeUint8Clamped  - Js::TypeIds_Uint8ClampedArray == JsArrayTypeInt8 - Js::TypeIds_Int8Array);
C_ASSERT(JsArrayTypeInt16         - Js::TypeIds_Int16Array        == JsArrayTypeInt8 - Js::TypeIds_Int8Array);
C_ASSERT(JsArrayTypeUint16        - Js::TypeIds_Uint16Array       == JsArrayTypeInt8 - Js::TypeIds_Int8Array);
C_ASSERT(JsArrayTypeInt32         - Js::TypeIds_Int32Array        == JsArrayTypeInt8 - Js::TypeIds_Int8Array);
C_ASSERT(JsArrayTypeUint32        - Js::TypeIds_Uint32Array       == JsArrayTypeInt8 - Js::TypeIds_Int8Array);
C_ASSERT(JsArrayTypeFloat32       - Js::TypeIds_Float32Array      == JsArrayTypeInt8 - Js::TypeIds_Int8Array);
C_ASSERT(JsArrayTypeFloat64       - Js::TypeIds_Float64Array      == JsArrayTypeInt8 - Js::TypeIds_Int8Array);

inline JsTypedArrayType GetTypedArrayType(Js::TypeId typeId)
{
    Assert(Js::TypedArrayBase::Is(typeId));
    return static_cast<JsTypedArrayType>(typeId + (JsArrayTypeInt8 - Js::TypeIds_Int8Array));
}

STDAPI_(JsErrorCode) JsGetTypedArrayInfo(JsValueRef typedArray, JsTypedArrayType *arrayType, JsValueRef *arrayBuffer, unsigned int *byteOffset, unsigned int *byteLength)
{
    VALIDATE_JSREF(typedArray);

    BEGIN_JSRT_NO_EXCEPTION
    {
        const Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(typedArray);

        if (!Js::TypedArrayBase::Is(typeId))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        if (arrayType != nullptr) {
            *arrayType = GetTypedArrayType(typeId);
        }

        Js::TypedArrayBase* typedArrayBase = Js::TypedArrayBase::FromVar(typedArray);
        if (arrayBuffer != nullptr) {
            *arrayBuffer = typedArrayBase->GetArrayBuffer();
        }

        if (byteOffset != nullptr) {
            *byteOffset = typedArrayBase->GetByteOffset();
        }

        if (byteLength != nullptr) {
            *byteLength = typedArrayBase->GetByteLength();
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsGetArrayBufferStorage(JsValueRef instance, BYTE **buffer, unsigned int *bufferLength)
{
    VALIDATE_JSREF(instance);
    PARAM_NOT_NULL(buffer);
    PARAM_NOT_NULL(bufferLength);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (!Js::ArrayBuffer::Is(instance))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        Js::ArrayBuffer* arrayBuffer = Js::ArrayBuffer::FromVar(instance);
        *buffer = arrayBuffer->GetBuffer();
        *bufferLength = arrayBuffer->GetByteLength();
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsGetTypedArrayStorage(JsValueRef instance, BYTE **buffer, unsigned int *bufferLength, JsTypedArrayType *typedArrayType, int *elementSize)
{
    VALIDATE_JSREF(instance);
    PARAM_NOT_NULL(buffer);
    PARAM_NOT_NULL(bufferLength);

    BEGIN_JSRT_NO_EXCEPTION
    {
        const Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(instance);
        if (!Js::TypedArrayBase::Is(typeId))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        Js::TypedArrayBase* typedArrayBase = Js::TypedArrayBase::FromVar(instance);
        *buffer = typedArrayBase->GetByteBuffer();
        *bufferLength = typedArrayBase->GetByteLength();

        if (typedArrayType)
        {
            *typedArrayType = GetTypedArrayType(typeId);
        }

        if (elementSize)
        {
            switch (typeId)
            {
                case Js::TypeIds_Int8Array:
                    *elementSize = sizeof(int8);
                    break;
                case Js::TypeIds_Uint8Array:
                    *elementSize = sizeof(uint8);
                    break;
                case Js::TypeIds_Uint8ClampedArray:
                    *elementSize = sizeof(uint8);
                    break;
                case Js::TypeIds_Int16Array:
                    *elementSize = sizeof(int16);
                    break;
                case Js::TypeIds_Uint16Array:
                    *elementSize = sizeof(uint16);
                    break;
                case Js::TypeIds_Int32Array:
                    *elementSize = sizeof(int32);
                    break;
                case Js::TypeIds_Uint32Array:
                    *elementSize = sizeof(uint32);
                    break;
                case Js::TypeIds_Float32Array:
                    *elementSize = sizeof(float);
                    break;
                case Js::TypeIds_Float64Array:
                    *elementSize = sizeof(double);
                    break;
                default:
                    AssertMsg(FALSE, "invalid typed array type");
                    *elementSize = 1;
                    RETURN_NO_EXCEPTION(JsErrorFatal);
            }
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsGetDataViewStorage(JsValueRef instance, BYTE **buffer, unsigned int *bufferLength)
{
    VALIDATE_JSREF(instance);
    PARAM_NOT_NULL(buffer);
    PARAM_NOT_NULL(bufferLength);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (!Js::DataView::Is(instance))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        Js::DataView* dataView = Js::DataView::FromVar(instance);
        *buffer = dataView->GetArrayBuffer()->GetBuffer() + dataView->GetByteOffset();
        *bufferLength = dataView->GetLength();
    }
    END_JSRT_NO_EXCEPTION
}


STDAPI_(JsErrorCode) JsCreateSymbol(JsValueRef description, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(result);
        *result = nullptr;

        Js::JavascriptString* descriptionString;

        if (description != JS_INVALID_REFERENCE)
        {
            VALIDATE_INCOMING_REFERENCE(description, scriptContext);
            descriptionString = Js::JavascriptConversion::ToString(description, scriptContext);
        }
        else
        {
            descriptionString = scriptContext->GetLibrary()->GetEmptyString();
        }

        *result = scriptContext->GetLibrary()->CreateSymbol(descriptionString);

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsHasIndexedProperty(JsValueRef object, JsValueRef index, bool *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_REFERENCE(index, scriptContext);
        PARAM_NOT_NULL(result);
        *result = false;

        *result = Js::JavascriptOperators::OP_HasItem((Js::Var)object, (Js::Var)index, scriptContext) != 0;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetIndexedProperty(JsValueRef object, JsValueRef index, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_REFERENCE(index, scriptContext);
        PARAM_NOT_NULL(result);
        *result = nullptr;

        *result = (JsValueRef)Js::JavascriptOperators::OP_GetElementI((Js::Var)object, (Js::Var)index, scriptContext);

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsSetIndexedProperty(JsValueRef object, JsValueRef index, JsValueRef value)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_REFERENCE(index, scriptContext);
        VALIDATE_INCOMING_REFERENCE(value, scriptContext);

        Js::JavascriptOperators::OP_SetElementI((Js::Var)object, (Js::Var)index, (Js::Var)value, scriptContext);

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsDeleteIndexedProperty(JsValueRef object, JsValueRef index)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);
        VALIDATE_INCOMING_REFERENCE(index, scriptContext);

        Js::JavascriptOperators::OP_DeleteElementI((Js::Var)object, (Js::Var)index, scriptContext);

        return JsNoError;
    });
}

template <class T, bool clamped = false> struct TypedArrayTypeTraits { static const JsTypedArrayType cTypedArrayType; };
template<> struct TypedArrayTypeTraits<int8> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeInt8; };
template<> struct TypedArrayTypeTraits<uint8, false> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeUint8; };
template<> struct TypedArrayTypeTraits<uint8, true> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeUint8Clamped; };
template<> struct TypedArrayTypeTraits<int16> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeInt16; };
template<> struct TypedArrayTypeTraits<uint16> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeUint16; };
template<> struct TypedArrayTypeTraits<int32> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeInt32; };
template<> struct TypedArrayTypeTraits<uint32> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeUint32; };
template<> struct TypedArrayTypeTraits<float> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeFloat32; };
template<> struct TypedArrayTypeTraits<double> { static const JsTypedArrayType cTypedArrayType = JsTypedArrayType::JsArrayTypeFloat64; };

template <class T, bool clamped = false>
Js::ArrayObject* CreateTypedArray(Js::ScriptContext *scriptContext, void* data, unsigned int length)
{
    Js::JavascriptLibrary* library = scriptContext->GetLibrary();
    Js::ArrayBuffer* arrayBuffer = RecyclerNew(
        scriptContext->GetRecycler(),
        Js::ExternalArrayBuffer,
        reinterpret_cast<BYTE*>(data),
        length * sizeof(T),
        library->GetArrayBufferType());

    return static_cast<Js::ArrayObject*>(Js::TypedArray<T, clamped>::Create(arrayBuffer, 0, length, library));
}

template <class T, bool clamped = false>
void GetObjectArrayData(Js::ArrayObject* objectArray, void** data, JsTypedArrayType* arrayType, uint* length)
{
    Js::TypedArray<T, clamped>* typedArray = Js::TypedArray<T, clamped>::FromVar(objectArray);
    *data = typedArray->GetArrayBuffer()->GetBuffer();
    *arrayType = TypedArrayTypeTraits<T, clamped>::cTypedArrayType;
    *length = typedArray->GetLength();
}

STDAPI_(JsErrorCode) JsSetIndexedPropertiesToExternalData(
    _In_ JsValueRef object,
    _In_ void* data,
    _In_ JsTypedArrayType arrayType,
    _In_ unsigned int elementLength)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_OBJECT(object, scriptContext);

        // Don't support doing this on array or array-like object
        Js::TypeId typeId = Js::JavascriptOperators::GetTypeId(object);
        if (!Js::DynamicType::Is(typeId)
            || Js::DynamicObject::IsAnyArrayTypeId(typeId)
            || (typeId >= Js::TypeIds_TypedArrayMin && typeId <= Js::TypeIds_TypedArrayMax)
            || typeId == Js::TypeIds_ArrayBuffer
            || typeId == Js::TypeIds_DataView
            || Js::RecyclableObject::FromVar(object)->IsExternal()
            )
        {
            return JsErrorInvalidArgument;
        }

        if (data == nullptr && elementLength > 0)
        {
            return JsErrorInvalidArgument;
        }

        Js::ArrayObject* newTypedArray = nullptr;
        switch (arrayType)
        {
        case JsArrayTypeInt8:
            newTypedArray = CreateTypedArray<int8>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeUint8:
            newTypedArray = CreateTypedArray<uint8>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeUint8Clamped:
            newTypedArray = CreateTypedArray<uint8, true>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeInt16:
            newTypedArray = CreateTypedArray<int16>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeUint16:
            newTypedArray = CreateTypedArray<uint16>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeInt32:
            newTypedArray = CreateTypedArray<int32>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeUint32:
            newTypedArray = CreateTypedArray<uint32>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeFloat32:
            newTypedArray = CreateTypedArray<float>(scriptContext, data, elementLength);
            break;
        case JsArrayTypeFloat64:
            newTypedArray = CreateTypedArray<double>(scriptContext, data, elementLength);
            break;
        default:
            return JsErrorInvalidArgument;
        }

        Js::DynamicObject* dynamicObject = Js::DynamicObject::FromVar(object);
        dynamicObject->SetObjectArray(newTypedArray);

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsHasIndexedPropertiesExternalData(JsValueRef object, bool *value)
{
    VALIDATE_JSREF(object);
    PARAM_NOT_NULL(value);

    BEGIN_JSRT_NO_EXCEPTION
    {
        *value = false;

        if (Js::DynamicType::Is(Js::JavascriptOperators::GetTypeId(object)))
        {
            Js::DynamicObject* dynamicObject = Js::DynamicObject::FromVar(object);
            Js::ArrayObject* objectArray = dynamicObject->GetObjectArray();
            *value = (objectArray && !Js::DynamicObject::IsAnyArray(objectArray));
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsGetIndexedPropertiesExternalData(
    _In_ JsValueRef object,
    _Out_ void** buffer,
    _Out_ JsTypedArrayType* arrayType,
    _Out_ unsigned int* elementLength)
{
    VALIDATE_JSREF(object);
    PARAM_NOT_NULL(buffer);
    PARAM_NOT_NULL(arrayType);
    PARAM_NOT_NULL(elementLength);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (!Js::DynamicType::Is(Js::JavascriptOperators::GetTypeId(object)))
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        *buffer = nullptr;
        *arrayType = JsTypedArrayType();
        *elementLength = 0;

        Js::DynamicObject* dynamicObject = Js::DynamicObject::FromVar(object);
        Js::ArrayObject* objectArray = dynamicObject->GetObjectArray();
        if (!objectArray)
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }

        switch (Js::JavascriptOperators::GetTypeId(objectArray))
        {
        case Js::TypeIds_Int8Array:
            GetObjectArrayData<int8>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Uint8Array:
            GetObjectArrayData<uint8>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Uint8ClampedArray:
            GetObjectArrayData<uint8, true>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Int16Array:
            GetObjectArrayData<int16>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Uint16Array:
            GetObjectArrayData<uint16>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Int32Array:
            GetObjectArrayData<int32>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Uint32Array:
            GetObjectArrayData<uint32>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Float32Array:
            GetObjectArrayData<float>(objectArray, buffer, arrayType, elementLength);
            break;
        case Js::TypeIds_Float64Array:
            GetObjectArrayData<double>(objectArray, buffer, arrayType, elementLength);
            break;
        default:
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsEquals(JsValueRef object1, JsValueRef object2, bool *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(object1, scriptContext);
        VALIDATE_INCOMING_REFERENCE(object2, scriptContext);
        PARAM_NOT_NULL(result);

        *result = Js::JavascriptOperators::Equal((Js::Var)object1, (Js::Var)object2, scriptContext) != 0;
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsStrictEquals(JsValueRef object1, JsValueRef object2, bool *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(object1, scriptContext);
        VALIDATE_INCOMING_REFERENCE(object2, scriptContext);
        PARAM_NOT_NULL(result);

        *result = Js::JavascriptOperators::StrictEqual((Js::Var)object1, (Js::Var)object2, scriptContext) != 0;
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsHasExternalData(JsValueRef object, bool *value)
{
    VALIDATE_JSREF(object);
    PARAM_NOT_NULL(value);

    BEGIN_JSRT_NO_EXCEPTION
    {
        *value = JsrtExternalObject::Is(object);
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsGetExternalData(JsValueRef object, void **data)
{
    VALIDATE_JSREF(object);
    PARAM_NOT_NULL(data);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (JsrtExternalObject::Is(object))
        {
            *data = JsrtExternalObject::FromVar(object)->GetSlotData();
        }
        else
        {
            *data = nullptr;
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsSetExternalData(JsValueRef object, void *data)
{
    VALIDATE_JSREF(object);

    BEGIN_JSRT_NO_EXCEPTION
    {
        if (JsrtExternalObject::Is(object))
        {
            JsrtExternalObject::FromVar(object)->SetSlotData(data);
        }
        else
        {
            RETURN_NO_EXCEPTION(JsErrorInvalidArgument);
        }
    }
    END_JSRT_NO_EXCEPTION
}

STDAPI_(JsErrorCode) JsCallFunction(JsValueRef function, JsValueRef *args, ushort cargs, JsValueRef *result)
{
    if (result != nullptr)
    {
        *result = nullptr;
    }

    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_FUNCTION(function, scriptContext);

        if (cargs == 0 || args == nullptr) {
          return JsErrorInvalidArgument;
        }

        for (int index = 0; index < cargs; index++)
        {
            VALIDATE_INCOMING_REFERENCE(args[index], scriptContext);
        }

        Js::JavascriptFunction *jsFunction = Js::JavascriptFunction::FromVar(function);
        Js::JavascriptMethod entryPoint = jsFunction->GetEntryPoint();
        Js::CallInfo callInfo(cargs);
        Js::Arguments jsArgs(callInfo, reinterpret_cast<Js::Var *>(args));

        Js::Var varResult = Js::JavascriptFunction::CallFunction<true>(jsFunction, entryPoint, jsArgs);
        if (result != nullptr)
        {
            *result = varResult;
            Assert(*result == nullptr || !Js::CrossSite::NeedMarshalVar(*result, scriptContext));
        }

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsConstructObject(JsValueRef function, JsValueRef *args, ushort cargs, JsValueRef *result)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_FUNCTION(function, scriptContext);
        PARAM_NOT_NULL(result);
        *result = nullptr;

        if (cargs == 0 || args == nullptr)
        {
            return JsErrorInvalidArgument;
        }

        for (int index = 0; index < cargs; index++)
        {
            VALIDATE_INCOMING_REFERENCE(args[index], scriptContext);
        }

        Js::JavascriptFunction *jsFunction = Js::JavascriptFunction::FromVar(function);
        Js::CallInfo callInfo(Js::CallFlags::CallFlags_New, cargs);
        Js::Arguments jsArgs(callInfo, reinterpret_cast<Js::Var *>(args));

        *result = Js::JavascriptFunction::CallAsConstructor(jsFunction, /* overridingNewTarget = */nullptr, jsArgs, scriptContext);
        Assert(*result == nullptr || !Js::CrossSite::NeedMarshalVar(*result, scriptContext));
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateFunction(JsNativeFunction nativeFunction, void *callbackState, JsValueRef *function)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(nativeFunction);
        PARAM_NOT_NULL(function);
        *function = nullptr;

        Js::JavascriptExternalFunction *externalFunction = scriptContext->GetLibrary()->CreateStdCallExternalFunction((Js::StdCallJavascriptMethod)nativeFunction, 0, callbackState);
        *function = (JsValueRef)externalFunction;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateNamedFunction(JsValueRef name, JsNativeFunction nativeFunction, void *callbackState, JsValueRef *function)
{
    return ContextAPIWrapper<true>([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(name, scriptContext);
        PARAM_NOT_NULL(nativeFunction);
        PARAM_NOT_NULL(function);
        *function = nullptr;

        if (name != JS_INVALID_REFERENCE)
        {
            name = Js::JavascriptConversion::ToString(name, scriptContext);
        }
        else
        {
            name = scriptContext->GetLibrary()->GetEmptyString();
        }

        Js::JavascriptExternalFunction *externalFunction = scriptContext->GetLibrary()->CreateStdCallExternalFunction((Js::StdCallJavascriptMethod)nativeFunction, Js::JavascriptString::FromVar(name), callbackState);

        *function = (JsValueRef)externalFunction;
        return JsNoError;
    });
}

void SetErrorMessage(Js::ScriptContext *scriptContext, JsValueRef newError, JsValueRef message)
{
    const Js::PropertyRecord *record;
    scriptContext->GetOrAddPropertyRecord(L"message", wcslen(L"message"), &record);
    Js::JavascriptOperators::OP_SetProperty(newError, record->GetPropertyId(), message, scriptContext);
}

STDAPI_(JsErrorCode) JsCreateError(JsValueRef message, JsValueRef *error)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(message, scriptContext);
        PARAM_NOT_NULL(error);
        *error = nullptr;

        JsValueRef newError = scriptContext->GetLibrary()->CreateError();
        SetErrorMessage(scriptContext, newError, message);
        *error = newError;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateRangeError(JsValueRef message, JsValueRef *error)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(message, scriptContext);
        PARAM_NOT_NULL(error);
        *error = nullptr;

        JsValueRef newError;

        newError = scriptContext->GetLibrary()->CreateRangeError();
        SetErrorMessage(scriptContext, newError, message);
        *error = newError;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateReferenceError(JsValueRef message, JsValueRef *error)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(message, scriptContext);
        PARAM_NOT_NULL(error);
        *error = nullptr;

        JsValueRef newError;

        newError = scriptContext->GetLibrary()->CreateReferenceError();
        SetErrorMessage(scriptContext, newError, message);
        *error = newError;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateSyntaxError(JsValueRef message, JsValueRef *error)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(message, scriptContext);
        PARAM_NOT_NULL(error);
        *error = nullptr;

        JsValueRef newError;

        newError = scriptContext->GetLibrary()->CreateSyntaxError();
        SetErrorMessage(scriptContext, newError, message);
        *error = newError;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateTypeError(JsValueRef message, JsValueRef *error)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(message, scriptContext);
        PARAM_NOT_NULL(error);
        *error = nullptr;

        JsValueRef newError;

        newError = scriptContext->GetLibrary()->CreateTypeError();
        SetErrorMessage(scriptContext, newError, message);
        *error = newError;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsCreateURIError(JsValueRef message, JsValueRef *error)
{
    return ContextAPIWrapper<true>([&] (Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(message, scriptContext);
        PARAM_NOT_NULL(error);
        *error = nullptr;

        JsValueRef newError;

        newError = scriptContext->GetLibrary()->CreateURIError();
        SetErrorMessage(scriptContext, newError, message);
        *error = newError;

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsHasException(bool *hasException)
{
    PARAM_NOT_NULL(hasException);
    *hasException = false;

    JsrtContext *currentContext = JsrtContext::GetCurrent();

    if (currentContext == nullptr)
    {
        return JsErrorNoCurrentContext;
    }

    Js::ScriptContext *scriptContext = currentContext->GetScriptContext();
    Assert(scriptContext != nullptr);

    if (scriptContext->GetRecycler() && scriptContext->GetRecycler()->IsHeapEnumInProgress())
    {
        return JsErrorHeapEnumInProgress;
    }
    else if (scriptContext->GetThreadContext()->IsInThreadServiceCallback())
    {
        return JsErrorInThreadServiceCallback;
    }

    if (scriptContext->GetThreadContext()->IsExecutionDisabled())
    {
        return JsErrorInDisabledState;
    }

    *hasException = scriptContext->HasRecordedException();

    return JsNoError;
}

STDAPI_(JsErrorCode) JsGetAndClearException(JsValueRef *exception)
{
    PARAM_NOT_NULL(exception);
    *exception = nullptr;

    JsrtContext *currentContext = JsrtContext::GetCurrent();

    if (currentContext == nullptr)
    {
        return JsErrorNoCurrentContext;
    }

    Js::ScriptContext *scriptContext = currentContext->GetScriptContext();
    Assert(scriptContext != nullptr);

    if (scriptContext->GetRecycler() && scriptContext->GetRecycler()->IsHeapEnumInProgress())
    {
        return JsErrorHeapEnumInProgress;
    }
    else if (scriptContext->GetThreadContext()->IsInThreadServiceCallback())
    {
        return JsErrorInThreadServiceCallback;
    }

    if (scriptContext->GetThreadContext()->IsExecutionDisabled())
    {
        return JsErrorInDisabledState;
    }

    HRESULT hr = S_OK;
    Js::JavascriptExceptionObject *recordedException = nullptr;

    BEGIN_TRANSLATE_OOM_TO_HRESULT
      recordedException = scriptContext->GetAndClearRecordedException();
    END_TRANSLATE_OOM_TO_HRESULT(hr)

    if (hr == E_OUTOFMEMORY) 
    {
      recordedException = scriptContext->GetThreadContext()->GetRecordedException();
    }
    if (recordedException == nullptr)
    {
        return JsErrorInvalidArgument;
    }

    *exception = recordedException->GetThrownObject(nullptr);
    if (*exception == nullptr)
    {
        return JsErrorInvalidArgument;
    }
    return JsNoError;
}

STDAPI_(JsErrorCode) JsSetException(JsValueRef exception)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext* scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(exception, scriptContext);

        Js::JavascriptExceptionObject *exceptionObject;

        exceptionObject = RecyclerNew(scriptContext->GetRecycler(), Js::JavascriptExceptionObject, exception, scriptContext, nullptr);

        JsrtContext * context = JsrtContext::GetCurrent();
        JsrtRuntime * runtime = context->GetRuntime();

        scriptContext->RecordException(exceptionObject, runtime->DispatchExceptions());

        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetRuntimeMemoryUsage(JsRuntimeHandle runtimeHandle, size_t * memoryUsage)
{
    VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);
    PARAM_NOT_NULL(memoryUsage);
    *memoryUsage = 0;

    ThreadContext * threadContext = JsrtRuntime::FromHandle(runtimeHandle)->GetThreadContext();
    AllocationPolicyManager * allocPolicyManager = threadContext->GetAllocationPolicyManager();

    *memoryUsage = allocPolicyManager->GetUsage();

    return JsNoError;
}

STDAPI_(JsErrorCode) JsSetRuntimeMemoryLimit(JsRuntimeHandle runtimeHandle, size_t memoryLimit)
{
    VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);

    ThreadContext * threadContext = JsrtRuntime::FromHandle(runtimeHandle)->GetThreadContext();
    AllocationPolicyManager * allocPolicyManager = threadContext->GetAllocationPolicyManager();

    allocPolicyManager->SetLimit(memoryLimit);

    return JsNoError;
}

STDAPI_(JsErrorCode) JsGetRuntimeMemoryLimit(JsRuntimeHandle runtimeHandle, size_t * memoryLimit)
{
    VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);
    PARAM_NOT_NULL(memoryLimit);
    *memoryLimit = 0;

    ThreadContext * threadContext = JsrtRuntime::FromHandle(runtimeHandle)->GetThreadContext();
    AllocationPolicyManager * allocPolicyManager = threadContext->GetAllocationPolicyManager();

    *memoryLimit = allocPolicyManager->GetLimit();

    return JsNoError;
}

C_ASSERT(JsMemoryAllocate == AllocationPolicyManager::MemoryAllocateEvent::MemoryAllocate);
C_ASSERT(JsMemoryFree == AllocationPolicyManager::MemoryAllocateEvent::MemoryFree);
C_ASSERT(JsMemoryFailure == AllocationPolicyManager::MemoryAllocateEvent::MemoryFailure);
C_ASSERT(JsMemoryFailure == AllocationPolicyManager::MemoryAllocateEvent::MemoryMax);

STDAPI_(JsErrorCode) JsSetRuntimeMemoryAllocationCallback(JsRuntimeHandle runtime, void *callbackState, JsMemoryAllocationCallback allocationCallback)
{
    VALIDATE_INCOMING_RUNTIME_HANDLE(runtime);

    ThreadContext* threadContext = JsrtRuntime::FromHandle(runtime)->GetThreadContext();
    AllocationPolicyManager * allocPolicyManager = threadContext->GetAllocationPolicyManager();

    allocPolicyManager->SetMemoryAllocationCallback(callbackState, (AllocationPolicyManager::PageAllocatorMemoryAllocationCallback)allocationCallback);

    return JsNoError;
}

STDAPI_(JsErrorCode) JsSetRuntimeBeforeCollectCallback(JsRuntimeHandle runtime, void *callbackState, JsBeforeCollectCallback beforeCollectCallback)
{
    return GlobalAPIWrapper([&]() -> JsErrorCode {
        VALIDATE_INCOMING_RUNTIME_HANDLE(runtime);

        JsrtRuntime::FromHandle(runtime)->SetBeforeCollectCallback(beforeCollectCallback, callbackState);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsDisableRuntimeExecution(JsRuntimeHandle runtimeHandle)
{
    VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);

    ThreadContext * threadContext = JsrtRuntime::FromHandle(runtimeHandle)->GetThreadContext();
    if (!threadContext->TestThreadContextFlag(ThreadContextFlagCanDisableExecution))
    {
        return JsErrorCannotDisableExecution;
    }

    if (threadContext->GetRecycler() && threadContext->GetRecycler()->IsHeapEnumInProgress())
    {
        return JsErrorHeapEnumInProgress;
    }
    else if (threadContext->IsInThreadServiceCallback())
    {
        return JsErrorInThreadServiceCallback;
    }

    threadContext->DisableExecution();
    return JsNoError;
}

STDAPI_(JsErrorCode) JsEnableRuntimeExecution(JsRuntimeHandle runtimeHandle)
{
    return GlobalAPIWrapper([&] () -> JsErrorCode {
        VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);

        ThreadContext * threadContext = JsrtRuntime::FromHandle(runtimeHandle)->GetThreadContext();
        if (!threadContext->TestThreadContextFlag(ThreadContextFlagCanDisableExecution))
        {
            return JsNoError;
        }

        if (threadContext->GetRecycler() && threadContext->GetRecycler()->IsHeapEnumInProgress())
        {
            return JsErrorHeapEnumInProgress;
        }
        else if (threadContext->IsInThreadServiceCallback())
        {
            return JsErrorInThreadServiceCallback;
        }

        ThreadContextScope scope(threadContext);

        if (!scope.IsValid())
        {
            return JsErrorWrongThread;
        }

        threadContext->EnableExecution();
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsIsRuntimeExecutionDisabled(JsRuntimeHandle runtimeHandle, bool *isDisabled)
{
    VALIDATE_INCOMING_RUNTIME_HANDLE(runtimeHandle);
    PARAM_NOT_NULL(isDisabled);
    *isDisabled = false;

    ThreadContext* threadContext = JsrtRuntime::FromHandle(runtimeHandle)->GetThreadContext();
    *isDisabled = threadContext->IsExecutionDisabled();
    return JsNoError;
}

STDAPI_(JsErrorCode) JsGetPropertyIdFromName(const wchar_t *name, JsPropertyIdRef *propertyId)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(name);
        PARAM_NOT_NULL(propertyId);
        *propertyId = nullptr;

        scriptContext->GetOrAddPropertyRecord(name, wcslen(name), (Js::PropertyRecord const **)propertyId);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetPropertyIdFromSymbol(JsValueRef symbol, JsPropertyIdRef *propertyId)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_REFERENCE(symbol, scriptContext);
        PARAM_NOT_NULL(propertyId);
        *propertyId = nullptr;

        if (!Js::JavascriptSymbol::Is(symbol))
        {
            return JsErrorPropertyNotSymbol;
        }

        *propertyId = (JsPropertyIdRef)Js::JavascriptSymbol::FromVar(symbol)->GetValue();
        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

STDAPI_(JsErrorCode) JsGetSymbolFromPropertyId(JsPropertyIdRef propertyId, JsValueRef *symbol)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        PARAM_NOT_NULL(symbol);
        *symbol = nullptr;

        Js::PropertyRecord const * propertyRecord = (Js::PropertyRecord const *)propertyId;
        if (!propertyRecord->IsSymbol())
        {
            return JsErrorPropertyNotSymbol;
        }

        *symbol = scriptContext->GetLibrary()->CreateSymbol(propertyRecord);
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetPropertyNameFromId(JsPropertyIdRef propertyId, const wchar_t **name)
{
    return GlobalAPIWrapper([&]() -> JsErrorCode {
        VALIDATE_INCOMING_PROPERTYID(propertyId);
        PARAM_NOT_NULL(name);
        *name = nullptr;

        Js::PropertyRecord const * propertyRecord = (Js::PropertyRecord const *)propertyId;

        if (propertyRecord->IsSymbol())
        {
            return JsErrorPropertyNotString;
        }

        *name = propertyRecord->GetBuffer();
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsGetPropertyIdType(JsPropertyIdRef propertyId, JsPropertyIdType* propertyIdType)
{
    return GlobalAPIWrapper([&]() -> JsErrorCode {
        VALIDATE_INCOMING_PROPERTYID(propertyId);

        Js::PropertyRecord const * propertyRecord = (Js::PropertyRecord const *)propertyId;

        if (propertyRecord->IsSymbol())
        {
            *propertyIdType = JsPropertyIdTypeSymbol;
        }
        else
        {
            *propertyIdType = JsPropertyIdTypeString;
        }
        return JsNoError;
    });
}


STDAPI_(JsErrorCode) JsGetRuntime(JsContextRef context, JsRuntimeHandle *runtime)
{
    VALIDATE_JSREF(context);
    PARAM_NOT_NULL(runtime);

    *runtime = nullptr;

    if (!JsrtContext::Is(context))
    {
        return JsErrorInvalidArgument;
    }

    *runtime = static_cast<JsrtContext *>(context)->GetRuntime();
    return JsNoError;
}

STDAPI_(JsErrorCode) JsIdle(unsigned int *nextIdleTick)
{
    PARAM_NOT_NULL(nextIdleTick);

    return ContextAPINoScriptWrapper(
        [&] (Js::ScriptContext * scriptContext) -> JsErrorCode {

            *nextIdleTick = 0;

            if (scriptContext->GetThreadContext()->GetRecycler() && scriptContext->GetThreadContext()->GetRecycler()->IsHeapEnumInProgress())
            {
                return JsErrorHeapEnumInProgress;
            }
            else if (scriptContext->GetThreadContext()->IsInThreadServiceCallback())
            {
                return JsErrorInThreadServiceCallback;
            }

            JsrtContext * context = JsrtContext::GetCurrent();
            JsrtRuntime * runtime = context->GetRuntime();

            if (!runtime->UseIdle())
            {
                return JsErrorIdleNotEnabled;
            }

            unsigned int ticks = runtime->Idle();

            *nextIdleTick = ticks;

            return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsSetPromiseContinuationCallback(JsPromiseContinuationCallback promiseContinuationCallback, void *callbackState)
{
    return ContextAPINoScriptWrapper([&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(promiseContinuationCallback);

        scriptContext->GetLibrary()->SetNativeHostPromiseContinuationFunction((Js::JavascriptLibrary::PromiseContinuationCallback)promiseContinuationCallback, callbackState);
        return JsNoError;
    },
    /*allowInObjectBeforeCollectCallback*/true);
}

JsErrorCode RunScriptCore(const wchar_t *script, JsSourceContext sourceContext, const wchar_t *sourceUrl, bool parseOnly, JsValueRef *result)
{
    Js::JavascriptFunction *scriptFunction;
    CompileScriptException se;

    JsErrorCode errorCode = ContextAPINoScriptWrapper(
        [&](Js::ScriptContext * scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(script);
        PARAM_NOT_NULL(sourceUrl);


        SourceContextInfo * sourceContextInfo = scriptContext->GetSourceContextInfo(sourceContext, nullptr);

        if (sourceContextInfo == nullptr)
        {
            sourceContextInfo = scriptContext->CreateSourceContextInfo(sourceContext, sourceUrl, wcslen(sourceUrl), nullptr);
        }

        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ 0,
            /* ulColumnHost        */ 0,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ wcslen(script),
            /* ulCharOffset        */ 0,
            /* mod                 */ kmodGlobal,
            /* grfsi               */ 0
        };

        Js::Utf8SourceInfo* utf8SourceInfo;
        scriptFunction = scriptContext->LoadScript(script, &si, &se, result != nullptr, false, &utf8SourceInfo, Js::Constants::GlobalCode);

        JsrtContext * context = JsrtContext::GetCurrent();
        context->OnScriptLoad(scriptFunction, utf8SourceInfo);

        return JsNoError;
    });

    if (errorCode != JsNoError)
    {
        return errorCode;
    }

    return ContextAPIWrapper<false>([&](Js::ScriptContext* scriptContext) -> JsErrorCode {
        if (scriptFunction == nullptr)
        {
            HandleScriptCompileError(scriptContext, &se);
            return JsErrorScriptCompile;
        }

        if (parseOnly)
        {
            PARAM_NOT_NULL(result);
            *result = scriptFunction;
        }
        else
        {
            Js::Arguments args(0, nullptr);
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            Js::Var varThis;
            if (PHASE_FORCE1(Js::EvalCompilePhase))
            {
                varThis = Js::JavascriptOperators::OP_GetThis(scriptContext->GetLibrary()->GetUndefined(), kmodGlobal, scriptContext);
                args.Info.Flags = (Js::CallFlags)Js::CallFlags::CallFlags_Eval;
                args.Info.Count = 1;
                args.Values = &varThis;
            }
#endif
            Js::Var varResult = Js::JavascriptFunction::CallFunction<true>(scriptFunction, scriptFunction->GetEntryPoint(), args);
            if (result != nullptr)
            {
                *result = varResult;
            }
        }
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsParseScript(const wchar_t * script, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunScriptCore(script, sourceContext, sourceUrl, true, result);
}

STDAPI_(JsErrorCode) JsRunScript(const wchar_t * script, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunScriptCore(script, sourceContext, sourceUrl, false, result);
}

JsErrorCode JsSerializeScriptCore(const wchar_t *script, BYTE *functionTable, int functionTableSize, unsigned char *buffer, unsigned long *bufferSize)
{
    Js::JavascriptFunction *function;
    CompileScriptException se;

    JsErrorCode errorCode = ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        PARAM_NOT_NULL(script);
        PARAM_NOT_NULL(bufferSize);

        if (*bufferSize > 0)
        {
            PARAM_NOT_NULL(buffer);
            ZeroMemory(buffer, *bufferSize);
        }

        if (scriptContext->IsInDebugMode())
        {
            return JsErrorCannotSerializeDebugScript;
        }

        SourceContextInfo * sourceContextInfo = scriptContext->GetSourceContextInfo(JS_SOURCE_CONTEXT_NONE, nullptr);
        Assert(sourceContextInfo != nullptr);

        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ 0,
            /* ulColumnHost        */ 0,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ wcslen(script),
            /* ulCharOffset        */ 0,
            /* mod                 */ kmodGlobal,
            /* grfsi               */ 0
        };

        Js::Utf8SourceInfo* sourceInfo;
        function = scriptContext->LoadScript(script, &si, &se, true, true, &sourceInfo, Js::Constants::GlobalCode);
        return JsNoError;
    });

    if (errorCode != JsNoError)
    {
        return errorCode;
    }

    return ContextAPIWrapper<false>([&](Js::ScriptContext* scriptContext) -> JsErrorCode {
        if (function == nullptr)
        {
            HandleScriptCompileError(scriptContext, &se);
            return JsErrorScriptCompile;
        }
        // Could we have a deserialized function in this case?
        // If we are going to seralize it, a check isn't to expensive
        if (CONFIG_FLAG(ForceSerialized) && function->GetFunctionProxy() != nullptr) {
            function->GetFunctionProxy()->EnsureDeserialized();
        }
        Js::FunctionBody *functionBody = function->GetFunctionBody();
        const Js::Utf8SourceInfo *sourceInfo = functionBody->GetUtf8SourceInfo();
        DWORD dwSourceCodeLength = sourceInfo->GetCbLength(L"JsSerializeScript");
        LPCUTF8 utf8Code = sourceInfo->GetSource(L"JsSerializeScript");

        BEGIN_TEMP_ALLOCATOR(tempAllocator, scriptContext, L"ByteCodeSerializer");
        HRESULT hr = Js::ByteCodeSerializer::SerializeToBuffer(scriptContext, tempAllocator, dwSourceCodeLength, utf8Code, 0, nullptr, functionBody, functionBody->GetHostSrcInfo(), false, &buffer, bufferSize);        
        END_TEMP_ALLOCATOR(tempAllocator, scriptContext);

        if (SUCCEEDED(hr))
        {
            return JsNoError;
        }
        else
        {
            return JsErrorScriptCompile;
        }
    });

}

STDAPI_(JsErrorCode) JsSerializeScript(const wchar_t *script, unsigned char *buffer, unsigned long *bufferSize)
{
    return JsSerializeScriptCore(script, nullptr, 0, buffer, bufferSize);
}

JsErrorCode RunSerializedScriptCore(const wchar_t *script, _In_ JsSerializedScriptLoadSourceCallback scriptLoadCallback, _In_ JsSerializedScriptUnloadCallback scriptUnloadCallback, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, bool parseOnly, JsValueRef *result)
{
    Js::JavascriptFunction *function;
    JsErrorCode errorCode = ContextAPINoScriptWrapper([&](Js::ScriptContext *scriptContext) -> JsErrorCode {
        if (result != nullptr)
        {
            *result = nullptr;
        }

        PARAM_NOT_NULL(buffer);
        PARAM_NOT_NULL(sourceUrl);

        Js::ISourceHolder *sourceHolder = nullptr;
        LPUTF8 utf8Source = nullptr;
        size_t utf8Length = 0;
        size_t length = 0;

        if (script != nullptr)
        {
            Assert(scriptLoadCallback == nullptr);
            Assert(scriptUnloadCallback == nullptr);
            Js::JsrtSourceHolder::ScriptToUtf8(scriptContext, script, &utf8Source, &utf8Length, &length);          
        }
        else
        {
            PARAM_NOT_NULL(scriptLoadCallback);
            PARAM_NOT_NULL(scriptUnloadCallback);            
            sourceHolder = RecyclerNewFinalized(scriptContext->GetRecycler(), Js::JsrtSourceHolder, scriptLoadCallback, scriptUnloadCallback, sourceContext);            
        }

        SourceContextInfo *sourceContextInfo;
        SRCINFO *hsi;
        Js::FunctionBody *functionBody = nullptr;

        HRESULT hr;

        sourceContextInfo = scriptContext->GetSourceContextInfo(sourceContext, nullptr);

        if (sourceContextInfo == nullptr)
        {
            sourceContextInfo = scriptContext->CreateSourceContextInfo(sourceContext, sourceUrl, wcslen(sourceUrl), nullptr);
        }
                
        SRCINFO si = {
            /* sourceContextInfo   */ sourceContextInfo,
            /* dlnHost             */ 0,
            /* ulColumnHost        */ 0,
            /* lnMinHost           */ 0,
            /* ichMinHost          */ 0,
            /* ichLimHost          */ length, // REVIEW: Currently 0 for Callback case.  Determine what is best to set this value.
            /* ulCharOffset        */ 0,
            /* mod                 */ kmodGlobal,
            /* grfsi               */ 0
        };

        ulong flags = 0;

        if (CONFIG_FLAG(CreateFunctionProxy) && !scriptContext->IsProfiling())
        {
            flags = fscrAllowFunctionProxy;
        }

        hsi = scriptContext->AddHostSrcInfo(&si);

        if (utf8Source != nullptr)
        {
            hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, utf8Source, hsi, buffer, nullptr, &functionBody);
        }
        else
        {
            hr = Js::ByteCodeSerializer::DeserializeFromBuffer(scriptContext, flags, sourceHolder, hsi, buffer, nullptr, &functionBody);
        }

        if (FAILED(hr))
        {
            return JsErrorBadSerializedScript;
        }

        function = scriptContext->GetLibrary()->CreateScriptFunction(functionBody);

        JsrtContext * context = JsrtContext::GetCurrent();
        context->OnScriptLoad(function, functionBody->GetUtf8SourceInfo());

        return JsNoError;
    });

    if (errorCode != JsNoError)
    {
        return errorCode;
    }

    return ContextAPIWrapper<false>([&](Js::ScriptContext* scriptContext) -> JsErrorCode {
        if (parseOnly)
        {
            PARAM_NOT_NULL(result);
            *result = function;
        }
        else
        {
            Js::Var varResult = Js::JavascriptFunction::CallFunction<true>(function, function->GetEntryPoint(), Js::Arguments(0, nullptr));
            if (result != nullptr)
            {
                *result = varResult;
            }
        }
        return JsNoError;
    });
}

STDAPI_(JsErrorCode) JsParseSerializedScript(const wchar_t * script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunSerializedScriptCore(script, nullptr, nullptr, buffer, sourceContext, sourceUrl, true, result);
}

STDAPI_(JsErrorCode) JsRunSerializedScript(const wchar_t * script, unsigned char *buffer, JsSourceContext sourceContext, const wchar_t *sourceUrl, JsValueRef * result)
{
    return RunSerializedScriptCore(script, nullptr, nullptr, buffer, sourceContext, sourceUrl, false, result);
}

STDAPI_(JsErrorCode) JsParseSerializedScriptWithCallback(_In_ JsSerializedScriptLoadSourceCallback scriptLoadCallback, _In_ JsSerializedScriptUnloadCallback scriptUnloadCallback, _In_ unsigned char *buffer, _In_ JsSourceContext sourceContext, _In_z_ const wchar_t *sourceUrl, _Out_ JsValueRef * result)
{
    return RunSerializedScriptCore(nullptr, scriptLoadCallback, scriptUnloadCallback, buffer, sourceContext, sourceUrl, true, result);
}

STDAPI_(JsErrorCode) JsRunSerializedScriptWithCallback(_In_ JsSerializedScriptLoadSourceCallback scriptLoadCallback, _In_ JsSerializedScriptUnloadCallback scriptUnloadCallback, _In_ unsigned char *buffer, _In_ JsSourceContext sourceContext, _In_z_ const wchar_t *sourceUrl, _Out_opt_ JsValueRef * result)
{
    return RunSerializedScriptCore(nullptr, scriptLoadCallback, scriptUnloadCallback, buffer, sourceContext, sourceUrl, false, result);
}
