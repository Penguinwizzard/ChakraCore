//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{    
    FunctionInfo BoundFunction::functionInfo(&BoundFunction::NewInstance, FunctionInfo::DoNotProfile);

    BoundFunction::BoundFunction(DynamicType * type)
        : JavascriptFunction(type, &functionInfo),
        targetFunction(null),
        boundThis(null),
        count(0),
        boundArgs(null)
    {
        // Constructor used during copy on write.
        DebugOnly(VerifyEntryPoint());
    }

    BoundFunction::BoundFunction(Arguments args, DynamicType * type)
        : JavascriptFunction(type, &functionInfo),
        count(0),
        boundArgs(null)
    {

        DebugOnly(VerifyEntryPoint());
        AssertMsg(args.Info.Count > 0, "wrong number of args in BoundFunction");

        ScriptContext *scriptContext = this->GetScriptContext();
        targetFunction = RecyclableObject::FromVar(args[0]);

        // Let proto be targetFunction.[[GetPrototypeOf]]().
        RecyclableObject* proto = JavascriptOperators::GetPrototype(targetFunction);
        if (proto != type->GetPrototype()) 
        {
            type->SetPrototype(proto);
        }
        // If targetFunction is proxy, need to make sure that traps are called in right order as per 19.2.3.2 in RC#4 dated April 3rd 2015.
        // Here although we won't use value of length, this is just to make sure that we call traps envoled for HasOwnProperty(Target, "length") and Get(Target, "length")
        if (JavascriptProxy::Is(targetFunction))
        {
            if (JavascriptOperators::HasOwnProperty(targetFunction, PropertyIds::length, scriptContext) == TRUE)
            {
                int len = 0;
                Var varLength;
                if (targetFunction->GetProperty(targetFunction, PropertyIds::length, &varLength, NULL, scriptContext))
                {
                    len = JavascriptConversion::ToInt32(varLength, scriptContext);
                }
            }
            EnsureObjectReady();
        }

        if (args.Info.Count > 1)
        {
            boundThis = args[1];

            // function object and "this" arg
            const uint countAccountedFor = 2;
            count = args.Info.Count - countAccountedFor;

            // Store the args excluding function obj and "this" arg
            if (args.Info.Count > 2)
            {
                boundArgs = RecyclerNewArray(scriptContext->GetRecycler(), Var, count);

                for (uint i=0; i<count; i++)
                {
                    boundArgs[i] = args[i+countAccountedFor];
                }
            }
        }
        else
        {
            // If no "this" is passed, "undefined" is used
            boundThis = scriptContext->GetLibrary()->GetUndefined();
        }
    }

    BoundFunction::BoundFunction(RecyclableObject* targetFunction, Var boundThis, Var* args, uint argsCount, DynamicType * type)
        : JavascriptFunction(type, &functionInfo),
        count(argsCount),
        boundArgs(null)
    {
        DebugOnly(VerifyEntryPoint());

        this->targetFunction = targetFunction;
        this->boundThis = boundThis;

        if (argsCount != 0)
        {
            this->boundArgs = RecyclerNewArray(this->GetScriptContext()->GetRecycler(), Var, argsCount);

            for (uint i = 0; i < argsCount; i++)
            {
                this->boundArgs[i] = args[i];
            }
        }
    }

    /*static*/ BoundFunction* BoundFunction::New(ScriptContext* scriptContext, ArgumentReader args)
    {
        Recycler* recycler = scriptContext->GetRecycler();

        BoundFunction* boundFunc = RecyclerNew(recycler, BoundFunction, args,
            scriptContext->GetLibrary()->GetBoundFunctionType());
        return boundFunc;
    }

    bool BoundFunction::CloneMethod(JavascriptFunction** pnewMethod, const Var newHome)
    {
        ScriptContext* scriptContext = this->GetScriptContext();
        Recycler* recycler = scriptContext->GetRecycler();

        BoundFunction* boundFunc = RecyclerNew(recycler, BoundFunction,
            this->GetTargetFunction(),
            this->boundThis,
            this->GetArgsForHeapEnum(),
            this->GetArgsCountForHeapEnum(),
            scriptContext->GetLibrary()->GetBoundFunctionType());

        *pnewMethod = boundFunc;
        return true;
    }

    Var BoundFunction::NewInstance(RecyclableObject* function, CallInfo callInfo, ...)
    {
        RUNTIME_ARGUMENTS(args, callInfo);
        ScriptContext* scriptContext = function->GetScriptContext();

        if (args.Info.Count == 0)
        {
            JavascriptError::ThrowTypeError(scriptContext, JSERR_NeedFunction /* TODO-ERROR: get arg name - args[0] */);
        }

        BoundFunction *boundFunction = (BoundFunction *) function;
        Var targetFunction = boundFunction->targetFunction;

        //
        // var o = new boundFunction()
        // a new object should be created using the actual function object
        //
        Var newVarInstance = null;
        if (callInfo.Flags & CallFlags_New)
        {
          if (JavascriptProxy::Is(targetFunction))
          {
            JavascriptProxy* proxy = JavascriptProxy::FromVar(targetFunction);
            Arguments proxyArgs(CallInfo(CallFlags_New, 1), &targetFunction);
            args.Values[0] = newVarInstance = proxy->ConstructorTrap(proxyArgs, scriptContext, 0);
          }
          else 
          {
            args.Values[0] = newVarInstance = JavascriptOperators::NewScObjectNoCtor(targetFunction, scriptContext);
          }
        }

        Js::Arguments actualArgs = args;

        if (boundFunction->count > 0)
        {
            // OACR thinks that this can change between here and the check in the for loop below
            const unsigned int argCount = args.Info.Count;

            if ((boundFunction->count + argCount) > CallInfo::kMaxCountArgs)
            {
                JavascriptError::ThrowRangeError(scriptContext, JSERR_ArgListTooLarge);
            }

            Var *newValues = RecyclerNewArray(scriptContext->GetRecycler(), Var, boundFunction->count + argCount);

            uint index = 0;

            //
            // For [[Construct]] use the newly created var instance
            // For [[Call]] use the "this" to which bind bound it.
            //
            if (callInfo.Flags & CallFlags_New)
            {
                newValues[index++] = args[0];
            }
            else
            {
                newValues[index++] = boundFunction->boundThis;
            }

            // Copy the bound args
            for (uint i=0; i<boundFunction->count; i++)
            {
                newValues[index++] = boundFunction->boundArgs[i];
            }

            // Copy the extra args
            for (uint i=1; i<argCount; i++)
            {
                newValues[index++] = args[i];
            }

            actualArgs = Arguments(args.Info, newValues);
            actualArgs.Info.Count = boundFunction->count + argCount;
        }
        else
        {
            if (!(callInfo.Flags & CallFlags_New))
            {
                actualArgs.Values[0] = boundFunction->boundThis;
            }
        }

        RecyclableObject* actualFunction = RecyclableObject::FromVar(targetFunction);
        Var aReturnValue = JavascriptFunction::CallFunction<true>(actualFunction, actualFunction->GetEntryPoint(), actualArgs);

        //
        // [[Construct]] and call returned a non-object
        // return the newly created var instance
        //
        if ((callInfo.Flags & CallFlags_New) && !JavascriptOperators::IsObject(aReturnValue))
        {
            aReturnValue = newVarInstance;
        }

        return aReturnValue;
    }   

    JavascriptFunction * BoundFunction::GetTargetFunction() const
    {
        if (targetFunction != nullptr)
        {
            RecyclableObject* _targetFunction = targetFunction;
            while (JavascriptProxy::Is(_targetFunction))
            {
                _targetFunction = JavascriptProxy::FromVar(_targetFunction)->GetTarget();
            }

            if (JavascriptFunction::Is(_targetFunction))
            {
                return JavascriptFunction::FromVar(_targetFunction);
            }

            // targetfunction should always be a javascriptfunction.
            Assert(FALSE);
        }
        return null;
    }

    JavascriptString* BoundFunction::GetDisplayNameImpl() const
    {
        JavascriptString* displayName = GetLibrary()->GetEmptyString();
        if (targetFunction != null)
        {
            Var value = JavascriptOperators::GetProperty(targetFunction, PropertyIds::name, targetFunction->GetScriptContext());
            if (JavascriptString::Is(value))
            {
                displayName = JavascriptString::FromVar(value);
            }
        }
        return LiteralString::Concat(LiteralString::NewCopySz(L"bound ", this->GetScriptContext()), displayName);
    }

    RecyclableObject* BoundFunction::GetBoundThis()
    {
        if (boundThis != null && RecyclableObject::Is(boundThis))
        {
            return RecyclableObject::FromVar(boundThis);
        }
        return NULL;
    }

    inline BOOL BoundFunction::IsConstructor() const
    {
        if (this->targetFunction != nullptr)
        {
            return JavascriptOperators::IsConstructor(this->GetTargetFunction());
        }

        return false;
    }

} // namespace Js
