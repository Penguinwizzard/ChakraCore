//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "RuntimeLanguagePch.h"
#include "ModuleNamespace.h"
#include "ModuleNamespaceEnumerator.h"
#include "Types\PropertyIndexRanges.h"
#include "Types\SimpleDictionaryPropertyDescriptor.h"
#include "Types\SimpleDictionaryTypeHandler.h"

namespace Js
{
    ModuleNamespace::ModuleNamespace(ModuleRecordBase* moduleRecord, DynamicType* type) :
        moduleRecord(moduleRecord), DynamicObject(type), unambiguousNonLocalExports(nullptr)
    {

    }

    ModuleNamespace* ModuleNamespace::GetModuleNamespace(ModuleRecordBase* requestModule)
    {
        Assert(requestModule->IsSourceTextModuleRecord());
        SourceTextModuleRecord* moduleRecord = static_cast<SourceTextModuleRecord*>(requestModule);
        ModuleNamespace* nsObject = moduleRecord->GetNamespace();
        if (nsObject != nullptr)
        {
            return nsObject;
        }
        ScriptContext* scriptContext = moduleRecord->GetRealm()->GetScriptContext();
        RecyclableObject* nullValue = scriptContext->GetLibrary()->GetNull();
        Recycler* recycler = scriptContext->GetRecycler();

        // First, the storage for local exports are stored in the ModuleRecord object itself, and we can build up a simpleDictionaryTypeHanlder to
        // look them up locally.
        SimpleDictionaryTypeHandlerNotExtensible* typeHandler = SimpleDictionaryTypeHandlerNotExtensible::New(recycler, moduleRecord->GetLocalExportCount(), 0, 0);
        DynamicType* dynamicType = DynamicType::New(scriptContext, TypeIds_ModuleNamespace, nullValue, nullptr, typeHandler);
        nsObject = RecyclerNew(recycler, ModuleNamespace, moduleRecord, dynamicType);

        LocalExportIndexList* localExportIndexList = moduleRecord->GetLocalExportIndexList();
        if (localExportIndexList != nullptr)
        {
            for (uint i = 0; i < (uint)localExportIndexList->Count(); i++)
            {
#if DBG
                Assert(moduleRecord->GetLocalExportSlotIndexByLocalName(localExportIndexList->Item(i)) == i);
#endif
                nsObject->DynamicObject::SetPropertyWithAttributes(localExportIndexList->Item(i), nullValue, PropertyModuleNamespaceDefault, nullptr);
            }
        }
        // update the local slot to use the storage for local exports.
        nsObject->SetAuxSlotsForModuleNS(moduleRecord->GetLocalExportSlots());

        // For items that are not in the local export list, we need to resolve them to get it 
        ExportedNames* exportedName = moduleRecord->GetExportedNames(nullptr);
        ModuleNameRecord* moduleNameRecord = nullptr;
#if DBG
        uint unresolvableExportsCount = 0;
        uint localExportCount = 0;
#endif
        if (exportedName != nullptr)
        {
            exportedName->Map([&](PropertyId propertyId) {
                if (!moduleRecord->ResolveExport(propertyId, nullptr, nullptr, &moduleNameRecord))
                {
                    // ignore ambigious resolution.
#if DBG
                    unresolvableExportsCount++;
#endif
                    return;
                }
                // non-ambiguous resolution.
                if (moduleNameRecord == nullptr)
                {
                    JavascriptError::ThrowSyntaxError(scriptContext, JSERR_ResolveExportFailed, scriptContext->GetPropertyName(propertyId)->GetBuffer());
                }
                if (moduleNameRecord->module == requestModule)
                {
                    // skip local exports as they are covered in the localExportSlots.
#if DBG
                    localExportCount++;
#endif
                    return;
                }
                Assert(moduleNameRecord->module != moduleRecord);
                nsObject->AddUnambiguousNonLocalExport(propertyId, moduleNameRecord);
            });
        }
#if DBG
        uint totalExportCount = exportedName != nullptr ? exportedName->Count() : 0;
        uint unambiguousNonLocalCount = (nsObject->GetUnambiguousNonLocalExports() != nullptr) ? nsObject->GetUnambiguousNonLocalExports()->Count() : 0;
        Assert(totalExportCount == localExportCount + unambiguousNonLocalCount + unresolvableExportsCount);
#endif
        BOOL result = nsObject->PreventExtensions();
        moduleRecord->SetNamespace(nsObject);

        Assert(result);
        return nsObject;
    }

    void ModuleNamespace::AddUnambiguousNonLocalExport(PropertyId propertyId, ModuleNameRecord* nonLocalExportNameRecord)
    {
        Recycler* recycler = GetScriptContext()->GetRecycler();
        if (unambiguousNonLocalExports == nullptr)
        {
            unambiguousNonLocalExports = RecyclerNew(recycler, UnambiguousExportMap, recycler, 4);
        }
        // keep a local copy of the module/
        unambiguousNonLocalExports->AddNew(propertyId, *nonLocalExportNameRecord);
    }

    BOOL ModuleNamespace::HasProperty(PropertyId propertyId)
    {
        if (GetTypeHandler()->HasProperty(this, propertyId))
        {
            return TRUE;
        }
        if (unambiguousNonLocalExports != nullptr)
        {
            return unambiguousNonLocalExports->ContainsKey(propertyId);
        }
        return FALSE;
    }

    BOOL ModuleNamespace::HasOwnProperty(PropertyId propertyId)
    {
        return HasProperty(propertyId);
    }

    BOOL ModuleNamespace::GetProperty(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        if (DynamicObject::GetProperty(originalInstance, propertyId, value, info, requestContext))
        {
            return TRUE;
        }
        if (unambiguousNonLocalExports != nullptr)
        {
            ModuleNameRecord moduleNameRecord;
            // TODO: maybe we can cache the slot address & offset, instead of looking up everytime? We do need to look up the reference everytime.
            if (unambiguousNonLocalExports->TryGetValue(propertyId, &moduleNameRecord))
            {
                return moduleNameRecord.module->GetNamespace()->GetProperty(originalInstance, propertyId, value, info, requestContext);
            }
        }
        return FALSE;
    }

    BOOL ModuleNamespace::GetProperty(Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        Assert(false);
        return FALSE;
    }

    BOOL ModuleNamespace::GetInternalProperty(Var instance, PropertyId internalPropertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        // TODO: @@toStringTag, @@Iterator: Iterator should be added explicitly.
        return FALSE;
    }

    BOOL ModuleNamespace::GetPropertyReference(Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext)
    {
        return GetProperty(originalInstance, propertyId, value, info, requestContext);
    }

    BOOL ModuleNamespace::GetEnumerator(BOOL enumNonEnumerable, Var* enumerator, ScriptContext* scriptContext, bool preferSnapshotSemantics, bool enumSymbols)
    {
        ModuleNamespaceEnumerator* moduleEnumerator = ModuleNamespaceEnumerator::New(this, scriptContext, preferSnapshotSemantics, enumSymbols);
        if (moduleEnumerator == nullptr)
        {
            return FALSE;
        }
        *enumerator = moduleEnumerator;
        return TRUE;
    }
}
