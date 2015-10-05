//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js 
{
    class JavascriptSIMDBool32x4 sealed : public RecyclableObject
    {
    private:
        SIMDValue value;

        DEFINE_VTABLE_CTOR(JavascriptSIMDBool32x4, RecyclableObject);


    public:

        JavascriptSIMDBool32x4(StaticType *type);
        JavascriptSIMDBool32x4(SIMDValue *val, StaticType *type);

        static JavascriptSIMDBool32x4* AllocUninitialized(ScriptContext* requestContext);
        static JavascriptSIMDBool32x4* New(SIMDValue *val, ScriptContext* requestContext);
        static bool Is(Var instance);
        static JavascriptSIMDBool32x4* FromVar(Var aValue);

        __inline SIMDValue GetValue() { return value; }

        virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;

        static size_t GetOffsetOfValue() { return offsetof(JavascriptSIMDBool32x4, value); }

        // Entry Points
        // None
        // End Entry Points

        Var  Copy(ScriptContext* requestContext);

    private:
        bool GetPropertyBuiltIns(PropertyId propertyId, Var* value, ScriptContext* requestContext);
    };
}

