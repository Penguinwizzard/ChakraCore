//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include "chakrart.h"

#define BEGIN_INTERCEPTOR(scriptContext) \
    BEGIN_LEAVE_SCRIPT(scriptContext) \
    try {

#define END_INTERCEPTOR(scriptContext) \
    } \
    catch (...) \
    { \
        Assert(false); \
    } \
    END_LEAVE_SCRIPT(scriptContext) \
    \
    if (scriptContext->HasRecordedException()) \
    { \
        scriptContext->RethrowRecordedException(NULL); \
    }


class JsrtExternalType sealed : public Js::DynamicType
{
public:
    JsrtExternalType(JsrtExternalType *type) : Js::DynamicType(type), jsFinalizeCallback(type->jsFinalizeCallback) {}
    JsrtExternalType(Js::ScriptContext* scriptContext, JsFinalizeCallback finalizeCallback);

    //Js::PropertyId GetNameId() const { return ((Js::PropertyRecord *)typeDescription.className)->GetPropertyId(); }
    JsFinalizeCallback GetJsFinalizeCallback() const { return this->jsFinalizeCallback; }

private:
    JsFinalizeCallback jsFinalizeCallback;
};
AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(JsrtExternalType, &Js::Type::DumpObjectFunction);

class JsrtExternalObject : public Js::DynamicObject
{
protected:
    DEFINE_VTABLE_CTOR(JsrtExternalObject, Js::DynamicObject);    
    DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JsrtExternalObject);
    
public:
    JsrtExternalObject(JsrtExternalType * type, void *data);

    static bool Is(Js::Var value);
    static JsrtExternalObject * FromVar(Js::Var value);

    JsrtExternalType * GetExternalType() const { return (JsrtExternalType *)this->GetType(); }

    void Finalize(bool isShutdown) override;
    void Dispose(bool isShutdown) override;    

    bool HasReadOnlyPropertiesInvisibleToTypeHandler() override { return true; }

    Js::DynamicType* DuplicateType() override;

    void * GetSlotData() const;
    void SetSlotData(void * data);

#if DBG
    virtual BOOL DbgCanHaveInterceptors() const override { return true; }
#endif
private:
    void * slot;
};
AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(JsrtExternalObject, &Js::RecyclableObject::DumpObjectFunction);
