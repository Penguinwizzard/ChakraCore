//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

// REVIEW: ChakraCore Dependency
#include "..\..\..\private\bin\Chakra\jsrtprivate.h"

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
    JsrtExternalType(JsrtExternalType *type) : Js::DynamicType(type), typeDescription(type->typeDescription) {}
    JsrtExternalType(Js::ScriptContext* scriptContext, JsExternalTypeDescription * typeDescription);

    Js::PropertyId GetNameId() const { return ((Js::PropertyRecord *)typeDescription.className)->GetPropertyId(); }
    JsExternalTypeDescription * GetExternalTypeDescription() { return &typeDescription; }  

private:
    JsExternalTypeDescription typeDescription;
};
AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(JsrtExternalType, &Js::Type::DumpObjectFunction);

class JsrtExternalObject sealed : public Js::DynamicObject
{
private:
    DEFINE_VTABLE_CTOR(JsrtExternalObject, Js::DynamicObject);    
    virtual void MarshalToScriptContext(Js::ScriptContext * scriptContext) override
    {
        AssertMsg(false, "JsrtExternalObject should never get marshaled"); 
    }  
    
public:
    JsrtExternalObject(JsrtExternalType * type, void *data);

    static bool Is(Js::Var value);
    static JsrtExternalObject * FromVar(Js::Var value);

    JsrtExternalType * GetExternalType() const { return (JsrtExternalType *)this->GetType(); }

    void Finalize(bool isShutdown) override;
    void Dispose(bool isShutdown) override;    

    bool HasReadOnlyPropertiesInvisibleToTypeHandler() override { return true; }

    BOOL HasProperty(Js::PropertyId propertyId) override;
    BOOL GetProperty(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    BOOL GetProperty(Js::Var originalInstance, Js::JavascriptString* propertyNameString, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    BOOL GetPropertyReference(Js::Var originalInstance, Js::PropertyId propertyId, Js::Var* value, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    BOOL SetProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    BOOL SetProperty(Js::JavascriptString* propertyNameString, Js::Var value, Js::PropertyOperationFlags flags, Js::PropertyValueInfo* info) override;
    BOOL SetPropertyWithAttributes(Js::PropertyId propertyId, Js::Var value, Js::PropertyAttributes attributes, Js::PropertyValueInfo* info, Js::PropertyOperationFlags flags = Js::PropertyOperation_None, Js::SideEffects possibleSideEffects = Js::SideEffects_Any) override;
    BOOL InitProperty(Js::PropertyId propertyId, Js::Var value, Js::PropertyOperationFlags flags = Js::PropertyOperation_None, Js::PropertyValueInfo* info = NULL) override;
    BOOL DeleteProperty(Js::PropertyId propertyId, Js::PropertyOperationFlags flags) override;

    BOOL HasItem(uint32 index) override;
    BOOL GetItem(Js::Var originalInstance, uint32 index, Js::Var* value, Js::ScriptContext * requestContext) override;
    BOOL GetItemReference(Js::Var originalInstance, uint32 index, Js::Var* value, Js::ScriptContext * requestContext) override;
    Js::DescriptorFlags GetItemSetter(uint32 index, Js::Var* setterValue, Js::ScriptContext* requestContext) override;
    BOOL SetItem(uint32 index, Js::Var value, Js::PropertyOperationFlags flags) override;
    BOOL DeleteItem(uint32 index, Js::PropertyOperationFlags flags) override;

    BOOL GetEnumerator(BOOL enumNonEnumerable, Js::Var* enumerator, Js::ScriptContext * requestContext, bool preferSnapshotSemantics = true, bool enumSymbols = false) override;

    BOOL IsWritable(Js::PropertyId propertyId) override;
    BOOL IsConfigurable(Js::PropertyId propertyId) override;
    BOOL IsEnumerable(Js::PropertyId propertyId) override;

    // TODO: WARNING: slow perf as it calls GetConfigurable/Enumerable/Writable individually.
    BOOL SetAttributes(Js::PropertyId propertyId, Js::PropertyAttributes attributes) override;

    BOOL SetEnumerable(Js::PropertyId propertyId, BOOL value) override;
    BOOL SetWritable(Js::PropertyId propertyId, BOOL value) override;
    BOOL SetConfigurable(Js::PropertyId propertyId, BOOL value) override;

    BOOL GetAccessors(Js::PropertyId propertyId, Js::Var *getter, Js::Var *setter, Js::ScriptContext * requestContext) override;
    BOOL SetAccessors(Js::PropertyId propertyId, Js::Var getter, Js::Var setter, Js::PropertyOperationFlags flags) override;
    Js::DescriptorFlags GetSetter(Js::PropertyId propertyId, Js::Var *setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;
    Js::DescriptorFlags GetSetter(Js::JavascriptString* propertyNameString, Js::Var *setterValue, Js::PropertyValueInfo* info, Js::ScriptContext* requestContext) override;

    BOOL JsrtExternalObject::Equals(Js::Var other, BOOL* returnResult, Js::ScriptContext * requestContext) override;
    BOOL JsrtExternalObject::StrictEquals(Js::Var other, BOOL* returnResult, Js::ScriptContext * requestContext) override;

    Js::JavascriptString* GetClassName(Js::ScriptContext * requestContext) override;

    BOOL GetDiagValueString(Js::StringBuilder<ArenaAllocator>* stringBuilder, Js::ScriptContext* requestContext) override;
    BOOL GetDiagTypeString(Js::StringBuilder<ArenaAllocator>* stringBuilder, Js::ScriptContext* requestContext) override;

    Js::DynamicType* DuplicateType() override;

    void * GetSlotData() const;
    void SetSlotData(void * data);

    JsEnumeratePropertiesCallback GetEnumerateCallback() { return GetExternalType()->GetExternalTypeDescription()->enumerateCallback; }

#if DBG
    virtual BOOL DbgCanHaveInterceptors() const override { return true; }
#endif
private:
    void * slot;
};
AUTO_REGISTER_RECYCLER_OBJECT_DUMPER(JsrtExternalObject, &Js::RecyclableObject::DumpObjectFunction);
