//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeLanguagePch.h"

namespace Js
{
    SourceTextModuleRecord::SourceTextModuleRecord(ScriptContext* scriptContext) :
        ModuleRecordBase(scriptContext->GetLibrary()),
        scriptContext(scriptContext),
        parseTree(nullptr),
        requestModuleList(nullptr),
        importRecordList(nullptr),
        localExportRecordList(nullptr),
        indirectExportRecordList(nullptr),
        starExportRecordList(nullptr),
        childrenModuleSet(nullptr),
        parentModuleSet(nullptr),
        errorObject(nullptr),
        wasEvaluated(false),
        wasParsed(false),
        wasDeclarationInitialized(false)
    {

    }

    bool SourceTextModuleRecord::ResolveExport(PropertyId exportName, ResolutionDictionary* resolveSet, ResolveSet* exportStarSet, ModuleNameRecord* exportRecord)
    {
        return false;
    }

    void SourceTextModuleRecord::ResolveExternalModuleDependencies()
    {
        ScriptContext* scriptContext = GetScriptContext();
        if (requestModuleList != nullptr)
        {
            if (nullptr == childrenModuleSet)
            {
                TempArenaAllocatorObject * allocatorObject = scriptContext->GetThreadContext()->GetTemporaryAllocator(L"Module");
                ArenaAllocator* allocator = allocatorObject->GetAllocator();
                childrenModuleSet = (ChildModuleRecordSet*)AllocatorNew(ArenaAllocator, allocator, ChildModuleRecordSet, allocator);
            }
            requestModuleList->Map([](LiteralString* specifier) {
                //Assert(!childrenmoduleSet->Contains(specifier));
                //if (!scriptContext->GetHostContext()->)
            });

        }
    }

    void SourceTextModuleRecord::ModuleDeclarationInstantiation()
    {

    }
}