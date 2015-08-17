//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    inline JavascriptGeneratorFunction* JavascriptLibrary::CreateGeneratorFunction(JavascriptMethod entryPoint, GeneratorVirtualScriptFunction* scriptFunction)
        JavascriptPromiseCapabilitiesExecutorFunction* function = EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseCapabilitiesExecutorFunction, type, functionInfo, capability));
    inline JavascriptPromiseResolveThenableTaskFunction* JavascriptLibrary::CreatePromiseResolveThenableTaskFunction(JavascriptMethod entryPoint, JavascriptPromise* promise, RecyclableObject* thenable, RecyclableObject* thenFunction)
        JavascriptPromiseAllResolveElementFunction* function = EnsureReadyIfHybridDebugging(RecyclerNewEnumClass(this->GetRecycler(), EnumFunctionClass, JavascriptPromiseAllResolveElementFunction, type, functionInfo, index, values, capabilities, remainingElements));
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
