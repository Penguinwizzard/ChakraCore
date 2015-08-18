//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline JavascriptPromiseAsyncSpawnExecutorFunction* JavascriptLibrary::CreatePromiseAsyncSpawnExecutorFunction(JavascriptMethod entryPoint, JavascriptGenerator* generatorFunction, Var target)
    {
        FunctionInfo* functionInfo = RecyclerNew(this->GetRecycler(), FunctionInfo, entryPoint);
        DynamicType* type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);
        JavascriptPromiseAsyncSpawnExecutorFunction* function = EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseAsyncSpawnExecutorFunction, type, functionInfo, generatorFunction, target));

        return function;
    }

    inline JavascriptPromiseAsyncSpawnStepArgumentExecutorFunction* JavascriptLibrary::CreatePromiseAsyncSpawnStepArgumentExecutorFunction(JavascriptMethod entryPoint, JavascriptGenerator* generator, Var argument, JavascriptFunction* resolve, JavascriptFunction* reject, bool isReject)
    {
        FunctionInfo* functionInfo = RecyclerNew(this->GetRecycler(), FunctionInfo, entryPoint);
        DynamicType* type = CreateDeferredPrototypeFunctionType(this->inDispatchProfileMode ? ProfileEntryThunk : entryPoint);
        JavascriptPromiseAsyncSpawnStepArgumentExecutorFunction* function = EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseAsyncSpawnStepArgumentExecutorFunction, type, functionInfo, generator, argument, resolve, reject, isReject));

        return function;
    }

    // Create a string literal from a C++ string (const wchar_t array with compile-time determined size)
    // Note: The template arg is the string length in characters, including the NUL terminator.
    template< size_t N > JavascriptString* JavascriptLibrary::CreateStringFromCppLiteral(const wchar_t(&value)[N]) const
    {
        CompileAssert(N>2); // Other values are handled by the specializations below
        return LiteralString::New(GetStringTypeStatic(), value, N - 1 /*don't include terminating NUL*/, this->GetRecycler());
    }

    // Specialization for the empty string
    template<> JavascriptString* JavascriptLibrary::CreateStringFromCppLiteral(const wchar_t(&value)[1]) const
    {
        return GetEmptyString();
    }

    // Specialization for single-char strings
    template<> JavascriptString* JavascriptLibrary::CreateStringFromCppLiteral(const wchar_t(&value)[2]) const
    {
        return charStringCache.GetStringForChar(value[0]);
    }

    template <size_t N>
    JavascriptFunction * JavascriptLibrary::AddFunctionToLibraryObjectWithPropertyName(DynamicObject* object, const wchar_t(&propertyName)[N], FunctionInfo * functionInfo, int length)
    {
        // The PID need to be tracked because it is assigned to the runtime function's nameId
        return AddFunctionToLibraryObject(object, scriptContext->GetOrAddPropertyIdTracked(propertyName), functionInfo, length);
    }

    template <>
    inline void JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray(const Var instance)
    {
        if (instance && JavascriptCopyOnAccessNativeIntArray::Is(instance))
        {
            JavascriptCopyOnAccessNativeIntArray::FromVar(instance)->ConvertCopyOnAccessSegment();
        }
    }

    template <typename T>
    void JavascriptLibrary::CheckAndConvertCopyOnAccessNativeIntArray(const T instance)
    {
        // dummy template function
        // Js::SCAEngine::Clone() to convert JavascriptCopyOnAccessNativeIntArray only if src is a 'Var'
    }

}
