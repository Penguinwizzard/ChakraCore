/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "RuntimeLibraryPch.h"

#ifdef ENABLE_DOM_FAST_PATH
namespace Js
{
    JavascriptTypedObjectSlotAccessorFunction::JavascriptTypedObjectSlotAccessorFunction(DynamicType* type, FunctionInfo* functionInfo, int allowedTypeId, PropertyId nameId) : 
        RuntimeFunction(type, functionInfo),
        allowedTypeId(allowedTypeId) 
    {
        DebugOnly(VerifyEntryPoint());
        SetFunctionNameId(Js::TaggedInt::ToVarUnchecked(nameId));
    }


    bool JavascriptTypedObjectSlotAccessorFunction::Is(Var instance)
    {
        if (VirtualTableInfo<Js::JavascriptTypedObjectSlotAccessorFunction>::HasVirtualTable(instance) ||
            VirtualTableInfo<Js::CrossSiteObject<Js::JavascriptTypedObjectSlotAccessorFunction>>::HasVirtualTable(instance) )
        {
            return true;
        }
        return false;
    }


    void JavascriptTypedObjectSlotAccessorFunction::ValidateThisInstance(Js::Var thisObj)
    {
        if (!InstanceOf(thisObj))
        {
            Js::JavascriptError::ThrowTypeError(GetType()->GetScriptContext(), JSERR_FunctionArgument_NeedObject, L"DOM object");
        }
    }

    bool JavascriptTypedObjectSlotAccessorFunction::InstanceOf(Var thisObj)
    {
        int allowedTypeId = GetAllowedTypeId();
        if (Js::JavascriptOperators::GetTypeId(thisObj) == allowedTypeId)
        {
            return true;
        }
        Type* type = RecyclableObject::FromVar(thisObj)->GetType();
        if (ExternalTypeWithInheritedTypeIds::Is(type))
        {
            return ((Js::ExternalTypeWithInheritedTypeIds*)type)->InstanceOf(allowedTypeId);
        }
        return false;
    }

    JavascriptTypedObjectSlotAccessorFunction* JavascriptTypedObjectSlotAccessorFunction::FromVar(Var instance)
    {
        Assert(Js::JavascriptTypedObjectSlotAccessorFunction::Is(instance));
        Assert((Js::JavascriptFunction::FromVar(instance)->GetFunctionInfo()->GetAttributes() & Js::FunctionBody::Attributes::NeedCrossSiteSecurityCheck) != 0);
        return static_cast<JavascriptTypedObjectSlotAccessorFunction*>(instance);
    }

    void JavascriptTypedObjectSlotAccessorFunction::ValidateThis(Js::JavascriptTypedObjectSlotAccessorFunction* func, Var thisObject)
    {
        func->ValidateThisInstance(thisObject);
    }
}
#endif