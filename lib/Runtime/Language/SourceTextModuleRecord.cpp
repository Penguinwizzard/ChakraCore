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

    HRESULT SourceTextModuleRecord::ParseSource(LPCWSTR sourceText, unsigned long sourceLength, Var* exceptionVar)
    {
        // TODO: handle utf8/unicode?
        Assert(!wasParsed);
        Assert(parser == nullptr);
        HRESULT hr = NOERROR;
        ScriptContext* scriptContext = GetScriptContext();
        CompileScriptException se;
        try
        {
            AUTO_NESTED_HANDLED_EXCEPTION_TYPE((ExceptionType)(ExceptionType_OutOfMemory | ExceptionType_StackOverflow));
            TempArenaAllocatorObject* allocatorObject = EnsureTempAllocator();
            ArenaAllocator* allocator = allocatorObject->GetAllocator();
            this->parser = (Parser*)AllocatorNew(ArenaAllocator, allocator, Parser, scriptContext);
            Utf8SourceInfo* pSourceInfo = nullptr;
            parseTree = scriptContext->ParseScript(parser, sourceText, nullptr, &se, true, false, false, &pSourceInfo, L"module", false, true);
            if (parseTree == nullptr)
            {
                hr = E_FAIL;
            }
        }
        catch (Js::OutOfMemoryException)
        {
            hr = E_OUTOFMEMORY;
            se.ProcessError(nullptr, E_OUTOFMEMORY, nullptr);
        }
        catch (Js::StackOverflowException)
        {
            hr = VBSERR_OutOfStack;
            se.ProcessError(nullptr, VBSERR_OutOfStack, nullptr);
        }
        //if (FAILED(hr))
        //{
        //    ConvertPSEToVar(&se, exceptionVar);
        //}
        return hr;
    }

    TempArenaAllocatorObject* SourceTextModuleRecord::EnsureTempAllocator()
    {
        if (tempAllocatorObject == nullptr)
        {
            tempAllocatorObject = scriptContext->GetThreadContext()->GetTemporaryAllocator(L"Module");
        }
        return tempAllocatorObject;
    }

    bool SourceTextModuleRecord::ResolveExport(PropertyId exportName, ResolutionDictionary* resolveSet, ResolveSet* exportStarSet, ModuleNameRecord* exportRecord)
    {
        return false;
    }

    void SourceTextModuleRecord::ResolveExternalModuleDependencies()
    {
//        ScriptContext* scriptContext = GetScriptContext();
        if (requestModuleList != nullptr)
        {
            if (nullptr == childrenModuleSet)
            {
                TempArenaAllocatorObject* allocatorObject = EnsureTempAllocator();
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
        AssertMsg(false, "not implemented");
    }

    Var SourceTextModuleRecord::ModuleEvaluation()
    {
        AssertMsg(false, "not implemented");
        return nullptr;
    }
#if DBG
    void SourceTextModuleRecord::AddParent(SourceTextModuleRecord* parentRecord, LPCWSTR specifier, unsigned long specifierLength)
    {

    }
#endif
}