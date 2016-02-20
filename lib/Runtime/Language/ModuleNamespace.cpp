//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "RuntimeLanguagePch.h"
#include "ModuleNamespace.h"

namespace Js
{
    ModuleNamespace::ModuleNamespace(ModuleRecordBase* moduleRecord, DynamicType* type):
        moduleRecord(moduleRecord), DynamicObject(type)
    {

    }

    ModuleNamespace* ModuleNamespace::GetModuleNamespace(ModuleRecordBase* moduleRecord)
    {
        //ModuleNamespace* nsObject = moduleRecord->GetNamespace();
        //if (nsObject == nullptr)
        //{
        //    ScriptContext* scriptContext = moduleRecord->GetRealm()->GetScriptContext();
        //    RecyclableObject* nullValue = scriptContext->GetLibrary()->GetNull();
        //    Recycler* recycler = scriptContext->GetRecycler();


            //ulong propertyCount = exportNames->Count(); // return?
            //Var* slot = moduleRecord->GetExportSlots(); // what's the right name
            //uint count = 0;
            //// We don't want to have any inline slot as we are going to use the external slot.
            //SimpleDictionaryTypeHandlerNotExtensible* typeHandler = SimpleDictionaryTypeHandlerNotExtensible::New(recycler, propertyCount, 0, 0);
            //DynamicType* dynamicType = DynamicType::New(scriptContext, TypeIds_ModuleNamespace, nullValue, nullptr, typeHandler);
            //ModuleNamespace* nsObject = RecyclerNew(recycler, nsObject, moduleRecord, dynamicType);
            //exportNames->Map([&](PropertyId propertyId) {
            //    nsObject->DynamicObject::SetProperty(propertyId, nullValue, (PropertyOperationFlags)PropertyModuleNamespaceDefault, nullptr);
            //    count++;
            //});
            //Var* auxSlots = RecyclerNewArray(recycler, Var, count);

            //nsObject->SetAuxSlotsForModuleNS(auxSlots);

        //}
        //return nsObject;
        return nullptr;
    }

    // 
    ModuleNamespace* ModuleNamespace::New(ModuleRecordBase* moduleRecord)
    {
        //ScriptContext* scriptContext = moduleRecord->GetScriptContext();
        //RecyclableObject* nullValue = scriptContext->GetLibrary()->GetNull();
        //Recycler* recycler = scriptContext->GetRecycler();
        //ulong propertyCount = exportNames->Count(); // return?
        //Var* slot = moduleRecord->GetExportSlots(); // what's the right name
        //uint count = 0;
        //// We don't want to have any inline slot as we are going to use the external slot.
        //SimpleDictionaryTypeHandlerNotExtensible* typeHandler = SimpleDictionaryTypeHandlerNotExtensible::New(recycler, propertyCount, 0, 0); 
        //DynamicType* dynamicType = DynamicType::New(scriptContext, TypeIds_ModuleNamespace, nullValue, nullptr, typeHandler);
        //ModuleNamespace* nsObject = RecyclerNew(recycler, nsObject, moduleRecord, dynamicType);
        //exportNames->Map([&](PropertyId propertyId) {
        //    nsObject->DynamicObject::SetProperty(propertyId, nullValue, (PropertyOperationFlags)PropertyModuleNamespaceDefault, nullptr);
        //    count++;
        //});
        //Var* auxSlots = RecyclerNewArray(recycler, Var, count);

        //nsObject->SetAuxSlotsForModuleNS(auxSlots);
        return nullptr;
    }
}