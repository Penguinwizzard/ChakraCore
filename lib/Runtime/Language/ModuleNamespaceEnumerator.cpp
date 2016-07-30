//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "RuntimeLanguagePch.h"
#include "Types\PropertyIndexRanges.h"
#include "Types\SimpleDictionaryPropertyDescriptor.h"
#include "Types\SimpleDictionaryTypeHandler.h"
#include "ModuleNamespace.h"
#include "ModuleNamespaceEnumerator.h"

namespace Js
{
    ModuleNamespaceEnumerator::ModuleNamespaceEnumerator(ModuleNamespace* _nsObject, ScriptContext* scriptContext, bool _preferSnapshotSemantics, bool _enumSymbols) :
        JavascriptEnumerator(scriptContext), nsObject(_nsObject), preferSnapshotSemantics(_preferSnapshotSemantics), enumSymbols(_enumSymbols),
        currentMapIndex(0), nonLocalMap(nullptr)
    {
    }

    ModuleNamespaceEnumerator* ModuleNamespaceEnumerator::New(ModuleNamespace* nsObject, ScriptContext* scriptContext, bool preferSnapshotSemantics, bool enumSymbols)
    {
        ModuleNamespaceEnumerator* enumerator = RecyclerNew(scriptContext->GetRecycler(), ModuleNamespaceEnumerator, nsObject, scriptContext, preferSnapshotSemantics, enumSymbols);
        if (enumerator->Init())
        {
            return enumerator;
        }
        return nullptr;
    }

    BOOL ModuleNamespaceEnumerator::Init()
    {
        if (nsObject->GetTypeHandler()->GetPropertyCount() != 0)
        {
            if (!nsObject->DynamicObject::GetEnumerator(TRUE, (Var*)&this->objectEnumerator, this->GetScriptContext(), preferSnapshotSemantics, enumSymbols))
            {
                this->objectEnumerator = nullptr;
                return FALSE;
            }
        }
        else
        {
            objectEnumerator = nullptr;
        }
        nonLocalMap = nsObject->GetUnambiguousNonLocalExports();
        return TRUE;
    }

    void ModuleNamespaceEnumerator::Reset()
    {
        if (this->objectEnumerator != nullptr)
        {
            this->objectEnumerator->Reset();
        }
        this->currentMapIndex = 0;
    }

    Var ModuleNamespaceEnumerator::GetCurrentIndex()
    {
        Var undefined = GetLibrary()->GetUndefined();
        Var result = undefined;
        if (this->objectEnumerator != nullptr)
        {
            result = this->objectEnumerator->GetCurrentIndex();
            if (result == undefined)
            {
                // we are done with the object part; 
                this->objectEnumerator = nullptr;
            }
            else
            {
                return result;
            }
        }
        if (this->nonLocalMap != nullptr && currentMapIndex < nonLocalMap->Count())
        {
            result = this->GetScriptContext()->GetPropertyString(this->nonLocalMap->GetKeyAt(currentMapIndex));
        }
        return result;
    }

    Var ModuleNamespaceEnumerator::GetCurrentValue()
    {
        return GetScriptContext()->GetLibrary()->GetUndefined();
    }

    BOOL ModuleNamespaceEnumerator::MoveNext(PropertyAttributes* attributes)
    {
        BOOL result = FALSE;
        if (attributes != nullptr)
        {
            // all the attribute should have the same setting here in namespace object.
            *attributes = PropertyModuleNamespaceDefault;
        }
        if (this->objectEnumerator != nullptr)
        {
            result = this->objectEnumerator->MoveNext(attributes);
            if (!result)
            {
                // we are done with the object part; 
                this->objectEnumerator = nullptr;
            }
            else
            {
                return TRUE;
            }
        }
        if (this->nonLocalMap != nullptr && (currentMapIndex + 1 < nonLocalMap->Count()))
        {
            currentMapIndex++;
            return TRUE;
        }
        return FALSE;
    }

    bool ModuleNamespaceEnumerator::GetCurrentPropertyId(PropertyId *propertyId)
    {
        Assert(false);
        return FALSE;
    }
}
