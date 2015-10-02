//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeLibraryPch.h"
#include "SIMDBool32x4Operation.h"

#ifdef SIMD_JS_ENABLED

namespace Js
{
    JavascriptSIMDBool32x4::JavascriptSIMDBool32x4(StaticType *type) : RecyclableObject(type)
    {
        Assert(type->GetTypeId() == TypeIds_SIMDBool32x4);
    }

    JavascriptSIMDBool32x4::JavascriptSIMDBool32x4(SIMDValue *val, StaticType *type) : RecyclableObject(type), value(*val)
    {
        Assert(type->GetTypeId() == TypeIds_SIMDBool32x4);
    }

    JavascriptSIMDBool32x4* JavascriptSIMDBool32x4::AllocUninitialized(ScriptContext* requestContext)
    {
        return (JavascriptSIMDBool32x4 *)AllocatorNew(Recycler, requestContext->GetRecycler(), JavascriptSIMDBool32x4, requestContext->GetLibrary()->GetSIMDBool32x4TypeStatic());
    }

    JavascriptSIMDBool32x4* JavascriptSIMDBool32x4::New(SIMDValue *val, ScriptContext* requestContext)
    {
        return (JavascriptSIMDBool32x4 *)AllocatorNew(Recycler, requestContext->GetRecycler(), JavascriptSIMDBool32x4, val, requestContext->GetLibrary()->GetSIMDBool32x4TypeStatic());
    }

    bool  JavascriptSIMDBool32x4::Is(Var instance)
    {
        return JavascriptOperators::GetTypeId(instance) == TypeIds_SIMDBool32x4;
    }

    JavascriptSIMDBool32x4* JavascriptSIMDBool32x4::FromVar(Var aValue)
    {
        Assert(aValue);
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptSIMDBool32x4'");

        return reinterpret_cast<JavascriptSIMDBool32x4 *>(aValue);
    }

    
    BOOL JavascriptSIMDBool32x4::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return GetPropertyBuiltIns(propertyId, value, requestContext);
    }

    BOOL JavascriptSIMDBool32x4::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        PropertyRecord const* propertyRecord;
        this->GetScriptContext()->FindPropertyRecord(propertyNameString, &propertyRecord);

        if (propertyRecord != nullptr && GetPropertyBuiltIns(propertyRecord->GetPropertyId(), value, requestContext))
        {
            return true;
        }
        return false;
    }

    BOOL JavascriptSIMDBool32x4::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return JavascriptSIMDBool32x4::GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    bool JavascriptSIMDBool32x4::GetPropertyBuiltIns(PropertyId propertyId, Var* value, ScriptContext* requestContext)
    {
        return false;
    }

    // Entry Points
    // None
    // End Entry Points

    Var JavascriptSIMDBool32x4::Copy(ScriptContext* requestContext)
    {
        return JavascriptSIMDBool32x4::New(&this->value, requestContext);
    }
}

#endif