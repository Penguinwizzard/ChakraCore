//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
	class JavascriptSIMDBool16x8 sealed : public RecyclableObject
	{
	private:
		SIMDValue value;

		DEFINE_VTABLE_CTOR(JavascriptSIMDBool16x8, RecyclableObject);


	public:

		JavascriptSIMDBool16x8(StaticType *type);
		JavascriptSIMDBool16x8(SIMDValue *val, StaticType *type);

		static JavascriptSIMDBool16x8* AllocUninitialized(ScriptContext* requestContext);
		static JavascriptSIMDBool16x8* New(SIMDValue *val, ScriptContext* requestContext);
		static bool Is(Var instance);
		static JavascriptSIMDBool16x8* FromVar(Var aValue);

		__inline SIMDValue GetValue() { return value; }

		virtual BOOL GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
		virtual BOOL GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
		virtual BOOL GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;

		static size_t GetOffsetOfValue() { return offsetof(JavascriptSIMDBool16x8, value); }

		// Entry Points
		// None
		// End Entry Points

		Var  Copy(ScriptContext* requestContext);

	private:
		bool GetPropertyBuiltIns(PropertyId propertyId, Var* value, ScriptContext* requestContext);
	};
}
