//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeLibraryPch.h"

namespace Js
{
    FunctionInfo JavascriptGeneratorFunction::functionInfo(&JavascriptGeneratorFunction::EntryGeneratorFunctionImplementation, (FunctionInfo::Attributes)(FunctionInfo::DoNotProfile | FunctionInfo::ErrorOnNew));

    JavascriptGeneratorFunction::JavascriptGeneratorFunction(DynamicType* type)
        : ScriptFunctionBase(type, &functionInfo),
        scriptFunction(nullptr)
    {
        // Constructor used during copy on write.
        DebugOnly(VerifyEntryPoint());
    }

    JavascriptGeneratorFunction::JavascriptGeneratorFunction(DynamicType* type, GeneratorVirtualScriptFunction* scriptFunction)
        : ScriptFunctionBase(type, &functionInfo),
        scriptFunction(scriptFunction)
    {
        DebugOnly(VerifyEntryPoint());
    }

    bool JavascriptGeneratorFunction::Is(Var var)
    {
        if (JavascriptFunction::Is(var))
        {
            JavascriptFunction* obj = JavascriptFunction::FromVar(var);

            return VirtualTableInfo<JavascriptGeneratorFunction>::HasVirtualTable(obj)
                || VirtualTableInfo<CrossSiteObject<JavascriptGeneratorFunction>>::HasVirtualTable(obj);
        }

        return false;
    }

    JavascriptGeneratorFunction* JavascriptGeneratorFunction::FromVar(Var var)
    {
        Assert(JavascriptGeneratorFunction::Is(var));

        return static_cast<JavascriptGeneratorFunction*>(var);
    }

    JavascriptGeneratorFunction* JavascriptGeneratorFunction::OP_NewScGenFunc(FrameDisplay *environment, FunctionProxy** proxyRef)
    {
        FunctionProxy* functionProxy = *proxyRef;
        ScriptContext* scriptContext = functionProxy->GetScriptContext();

        bool hasSuperReference = functionProxy->HasSuperReference();
        bool isDefaultConstructor = functionProxy->IsDefaultConstructor();

        AssertMsg(!isDefaultConstructor, "How is generator function is a default constructor?");

        GeneratorVirtualScriptFunction* scriptFunction = scriptContext->GetLibrary()->CreateGeneratorVirtualScriptFunction(functionProxy);
        scriptFunction->SetEnvironment(environment);
        scriptFunction->SetHasSuperReference(hasSuperReference);

        JS_ETW(EventWriteJSCRIPT_RECYCLER_ALLOCATE_FUNCTION(scriptFunction, EtwTrace::GetFunctionId(functionProxy)));

        JavascriptGeneratorFunction* genFunc = scriptContext->GetLibrary()->CreateGeneratorFunction(functionInfo.GetOriginalEntryPoint(), scriptFunction);
        scriptFunction->SetRealGeneratorFunction(genFunc);

        return genFunc;
    }

    Var JavascriptGeneratorFunction::EntryGeneratorFunctionImplementation(RecyclableObject* function, CallInfo callInfo, ...)
    {
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);
        ARGUMENTS(stackArgs, callInfo);

        ScriptContext* scriptContext = function->GetScriptContext();
        JavascriptGeneratorFunction* generatorFunction = JavascriptGeneratorFunction::FromVar(function);

        // InterpreterStackFrame takes a pointer to the args, so copy them to the recycler heap
        // and use that buffer for this InterpreterStackFrame.
        Var* argsHeapCopy = RecyclerNewArray(scriptContext->GetRecycler(), Var, stackArgs.Info.Count);
        js_memcpy_s(argsHeapCopy, sizeof(Var) * stackArgs.Info.Count, stackArgs.Values, sizeof(Var) * stackArgs.Info.Count);
        Arguments heapArgs(callInfo, argsHeapCopy);

        DynamicObject* prototype = scriptContext->GetLibrary()->CreateGeneratorConstructorPrototypeObject();
        JavascriptGenerator* generator = scriptContext->GetLibrary()->CreateGenerator(heapArgs, generatorFunction->scriptFunction, prototype);
        // Set the prototype from constructor
        JavascriptOperators::OrdinaryCreateFromConstructor(function, generator, prototype, scriptContext);

        Assert(!(callInfo.Flags & CallFlags_New));

        return generator;
    }

    Var JavascriptGeneratorFunction::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        // Get called when creating a new generator function through the constructor (e.g. gf.__proto__.constructor) and sets EntryGeneratorFunctionImplementation as the entrypoint
        PROBE_STACK(function->GetScriptContext(), Js::Constants::MinStackDefault);

        ARGUMENTS(args, callInfo);

        return JavascriptFunction::NewInstanceHelper(function->GetScriptContext(), function, callInfo, args, /* isGenerator: */ true);
    }

    JavascriptString* JavascriptGeneratorFunction::GetDisplayNameImpl() const
    {
        return scriptFunction->GetDisplayNameImpl();
    }

    Var JavascriptGeneratorFunction::GetHomeObj() const
    {
        return scriptFunction->GetHomeObj();
    }

    void JavascriptGeneratorFunction::SetHomeObj(Var homeObj)
    {
        scriptFunction->SetHomeObj(homeObj);
    }

    void JavascriptGeneratorFunction::SetComputedNameVar(Var computedNameVar) 
    { 
        scriptFunction->SetComputedNameVar(computedNameVar);
    }

    Var JavascriptGeneratorFunction::GetComputedNameVar() const
    { 
        return scriptFunction->GetComputedNameVar(); 
    }

    bool JavascriptGeneratorFunction::IsAnonymousFunction() const
    {
        return scriptFunction->IsAnonymousFunction();
    }
    
    Var JavascriptGeneratorFunction::GetSourceString() const
    {
        return scriptFunction->GetSourceString();
    }

    Var JavascriptGeneratorFunction::EnsureSourceString()
    {
        return scriptFunction->EnsureSourceString();
    }
}
