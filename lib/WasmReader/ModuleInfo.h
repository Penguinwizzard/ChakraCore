//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

namespace Wasm
{
    class ModuleInfo
    {
    private:
        struct Memory
        {
            Memory() : minSize(0)
            {
            }
            uint64 minSize;
            uint64 maxSize;
            bool exported;
            static const uint64 PAGE_SIZE = 64 * 1024;
        } m_memory;
    public:
        ModuleInfo(ArenaAllocator * alloc);

        bool InitializeMemory(uint32 minSize, uint32 maxSize, bool exported);

        const Memory * GetMemory() const;

        void SetSignature(uint32 index, WasmSignature * signature);
        WasmSignature * GetSignature(uint32 index) const;
        void SetSignatureCount(uint32 count);
        uint32 GetSignatureCount() const;

        void AllocateIndirectFunctions(uint32 entries);
        void SetIndirectFunction(uint32 funcIndex, uint32 indirectIndex);
        uint32 GetIndirectFunctionIndex(uint32 indirTableIndex) const;
        uint32 GetIndirectFunctionCount() const;

        void SetFunctionCount(uint count);
        uint GetFunctionCount() const;

        void AllocateFunctions(uint32 count);
        bool SetFunSig(WasmFunctionInfo* funsig, uint32 index);
        WasmFunctionInfo * GetFunSig(uint index) const;

        void AllocateFunctionExports(uint32 entries);
        uint GetExportCount() const { return m_exportCount; }
        void SetFunctionExport(uint32 iExport, uint32 funcIndex, char16* exportName, uint32 nameLength);
        WasmExport* GetFunctionExport(uint32 iExport) const;

        void AllocateFunctionImports(uint32 entries);
        uint32 GetImportCount() const { return m_importCount; }
        void SetFunctionImport(uint32 i, uint32 sigId, char16* modName, uint32 modNameLen, char16* fnName, uint32 fnNameLen);
        WasmImport* GetFunctionImport(uint32 i) const;

        void AllocateDataSegs(uint32 count);
        bool AddDataSeg(WasmDataSegment* seg, uint32 index);
        WasmDataSegment * GetDataSeg(uint32 index) const;
        uint32 GetDataSegCount() const { return m_datasegCount; }

        void SetStartFunction(uint32 i);
        uint32 GetStartFunction() const;
    private:
        WasmSignature** m_signatures;
        uint32* m_indirectfuncs;
        WasmFunctionInfo** m_funsigs;
        WasmExport* m_exports;
        WasmImport* m_imports;
        WasmDataSegment** m_datasegs;

        uint m_signaturesCount;
        uint m_indirectFuncCount;
        uint m_funcCount;
        uint m_exportCount;
        uint32 m_importCount;
        uint32 m_datasegCount;

        uint32 m_startFunc;

        ArenaAllocator * m_alloc;
    };

    struct WasmModule
    {
        WasmModule() :
            functions(nullptr),
            memSize(0),
            indirFuncTableOffset(0),
            heapOffset(0),
            funcOffset(0),
            funcCount(0),
            importFuncOffset(0)
        {
        }
        WasmFunction** functions;
        ModuleInfo * info;
        uint heapOffset;
        uint funcOffset;
        uint funcCount;
        uint importFuncOffset;
        uint indirFuncTableOffset;
        uint memSize;
    };
} // namespace Wasm
