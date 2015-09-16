//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
namespace Js
{
    class JavascriptTypedObjectSlotAccessorFunction : public RuntimeFunction
    {
    private:      
        DEFINE_MARSHAL_OBJECT_TO_SCRIPT_CONTEXT(JavascriptTypedObjectSlotAccessorFunction); 
    protected:
        DEFINE_VTABLE_CTOR(JavascriptTypedObjectSlotAccessorFunction, RuntimeFunction);
    public:
        JavascriptTypedObjectSlotAccessorFunction(DynamicType* type, FunctionInfo* functionInfo, int allowedTypeId, PropertyId nameId);

        int GetAllowedTypeId() const {return allowedTypeId; }
        void ValidateThisInstance(Var thisObject);
        bool InstanceOf(Var thisObj);

        static JavascriptTypedObjectSlotAccessorFunction* FromVar(Var instance);
        static bool Is(Var instance);
        static void ValidateThis(Js::JavascriptTypedObjectSlotAccessorFunction* func, Var thisObject);

    private:
        int allowedTypeId;
    };
};
