//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "JsrtPch.h"
#include "JsrtExternalObject.h"

JsrtExternalType::JsrtExternalType(Js::ScriptContext* scriptContext, JsFinalizeCallback finalizeCallback) 
    : Js::DynamicType(
        scriptContext, 
        scriptContext->CreateTypeId(), 
        scriptContext->GetLibrary()->GetObjectPrototype(),
        nullptr, 
        Js::SimplePathTypeHandler::New(scriptContext, scriptContext->GetRootPath(), 0, 0, 0, true, true), 
        true, 
        true)
        , jsFinalizeCallback(finalizeCallback)
{
    // We don't know anything for certain about the type of properties an external object might have
    this->GetTypeHandler()->ClearHasOnlyWritableDataProperties();
    if (GetTypeHandler()->GetFlags() & Js::DynamicTypeHandler::IsPrototypeFlag)
    {
        scriptContext->GetLibrary()->NoPrototypeChainsAreEnsuredToHaveOnlyWritableDataProperties();
    }
    this->flags |= TypeFlagMask_CanHaveInterceptors;
}

JsrtExternalObject::JsrtExternalObject(JsrtExternalType * type, void *data) :
    slot(data), 
    Js::DynamicObject(type)
{
}

bool JsrtExternalObject::Is(Js::Var value)
{
    if (Js::TaggedNumber::Is(value))
    {
        return false;
    }

    return (VirtualTableInfo<JsrtExternalObject>::HasVirtualTable(value)) ||
        (VirtualTableInfo<Js::CrossSiteObject<JsrtExternalObject>>::HasVirtualTable(value));
}

JsrtExternalObject * JsrtExternalObject::FromVar(Js::Var value)
{
    Assert(Is(value));
    return static_cast<JsrtExternalObject *>(value);
}

void JsrtExternalObject::Finalize(bool isShutdown)
{
    JsFinalizeCallback finalizeCallback = this->GetExternalType()->GetJsFinalizeCallback();
    if (nullptr != finalizeCallback)
    {
        finalizeCallback(this->slot);
    }
}

void JsrtExternalObject::Dispose(bool isShutdown)
{    
}

void * JsrtExternalObject::GetSlotData() const
{
    return this->slot;
}

void JsrtExternalObject::SetSlotData(void * data)
{
    this->slot = data;
}

Js::DynamicType* JsrtExternalObject::DuplicateType()
{
    return RecyclerNew(this->GetScriptContext()->GetRecycler(), JsrtExternalType,
        this->GetExternalType());
}
