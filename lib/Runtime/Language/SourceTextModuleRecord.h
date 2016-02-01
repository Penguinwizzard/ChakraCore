//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    class ModuleImportRecord
    {
    public:
        ModuleImportRecord();
        LiteralString* GetImportName() const { return importName; }
        LiteralString* GetLocalName() const { return localName; }
        LiteralString* GetModuleRequest() const { return moduleRequest; }

    private:
        LiteralString* moduleRequest;
        LiteralString* importName;
        LiteralString* localName;
    };

    class ModuleExportRecord
    {
    public:
        ModuleExportRecord() :
            moduleRequest(nullptr), importName(nullptr),
            exportName(nullptr), localName(nullptr) {};
        ModuleExportRecord(LiteralString* moduleRequest, LiteralString* importName, LiteralString* exportName, LiteralString* localName) :
            moduleRequest(moduleRequest), importName(importName), exportName(exportName), localName(localName) {}
        LiteralString* GetModuleRequest() const { return moduleRequest; }
        LiteralString* GetImportName() const { return importName; }
        LiteralString* GetExportName() const { return exportName; }
        LiteralString* GetLocalname() const { return localName; }
    private:
        LiteralString* localName, *exportName, *importName, *moduleRequest;
    };

    class SourceTextModuleRecord;
    typedef SList<LiteralString*> ModuleList;
    typedef SList<ModuleImportRecord*> ModuleImportRecordList;
    typedef SList<ModuleExportRecord*> ModuleExportRecordList;
    typedef  JsUtil::BaseDictionary<LiteralString*, SourceTextModuleRecord*, ArenaAllocator, PowerOf2SizePolicy> ChildModuleRecordSet;
    typedef  JsUtil::BaseDictionary<SourceTextModuleRecord*, SourceTextModuleRecord*, ArenaAllocator, PowerOf2SizePolicy> ParentModuleRecordSet;

    class SourceTextModuleRecord : public ModuleRecordBase
    {
    public:
        SourceTextModuleRecord(ScriptContext* scriptContext);
        ModuleList* GetRequestModuleList() const { return requestModuleList; }
        ModuleImportRecordList* GetImportRecordList() const { return importRecordList; }
        ModuleExportRecordList* GetLocalExportRecordList() const { return localExportRecordList; }
        ModuleExportRecordList* GetIndirectExportRecordList() const { return indirectExportRecordList; }
        ModuleExportRecordList* GetStarExportRecordList() const { return starExportRecordList; }
        ExportedNames* GetExportedNames(ResolveSet* exportStarSet) override { Assert(false); return nullptr; }
        // return false when "ambiguous". otherwise exportRecord.
        bool ResolveExport(PropertyId exportName, ResolutionDictionary* resolveSet, ResolveSet* exportStarSet, ModuleNameRecord* exportRecord) override;
        void ModuleDeclarationInstantiation() override;
        Var ModuleEvaluation() override;

        void Finalize(bool isShutdown) override { return; }
        void Dispose(bool isShutdown) override { return; }
        void Mark(Recycler * recycler) override { return; }

        void ResolveExternalModuleDependencies();

        void* GetHostDefined() const { return hostDefined; }
        void SetHostDefined(void* hostObj) { hostDefined = hostObj; }

        void SetImportRecordList(ModuleImportRecordList* importList) { importRecordList = importList; }
        void SetLocalExportRecordList(ModuleExportRecordList* localExports) { localExportRecordList = localExports; }
        void SetIndirectExportRecordList(ModuleExportRecordList* indirectExports) { indirectExportRecordList = indirectExports; }
        void SetStarExportRecordList(ModuleExportRecordList* starExports) { starExportRecordList = starExports; }
        void SetRequestModuleList(ModuleList* requestModules) { requestModuleList = requestModules; }

        ScriptContext* GetScriptContext() const { return scriptContext; }
        HRESULT ParseSource(LPCWSTR sourceText, unsigned long sourceLength, Var* exceptionVar);

        static SourceTextModuleRecord* FromHost(void* hostModuleRecord)
        {
            return static_cast<SourceTextModuleRecord*>(hostModuleRecord);
        }
#if DBG
        void AddParent(SourceTextModuleRecord* parentRecord, LPCWSTR specifier, unsigned long specifierLength);
#endif

    private:
        // This is the parsed tree resulted from compilation. 
        bool wasEvaluated;
        bool wasParsed;
        bool wasDeclarationInitialized;
        ParseNodePtr parseTree;
        SRCINFO scrInfo; // we need to keep a copy of the sourceInfo in preparation of bytecodegen later.
        Parser* parser;  // we'll need to keep the parser around till we are done with bytecode gen.
        ParseableFunctionInfo* functionInfo;
        ScriptContext* scriptContext;
        ModuleList* requestModuleList;
        ModuleImportRecordList* importRecordList;
        ModuleExportRecordList* localExportRecordList;
        ModuleExportRecordList* indirectExportRecordList;
        ModuleExportRecordList* starExportRecordList;
        void* hostDefined;
        ChildModuleRecordSet* childrenModuleSet;
        ParentModuleRecordSet* parentModuleSet;
        Var errorObject;

        TempArenaAllocatorObject* tempAllocatorObject;

        TempArenaAllocatorObject* EnsureTempAllocator();
    };
}