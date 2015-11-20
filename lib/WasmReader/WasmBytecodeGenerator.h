//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Wasm
{
    struct EmitInfo
    {
        EmitInfo(Js::RegSlot location_, const WasmTypes::WasmType& type_) :
            location(location_), type(type_)
        {
        }
        EmitInfo(const WasmTypes::WasmType& type_) :
            location(Js::Constants::NoRegister), type(type_)
        {
        }
        EmitInfo() :
            location(Js::Constants::NoRegister), type(WasmTypes::Void)
        {
        }

        Js::RegSlot location;
        WasmTypes::WasmType type;
    };

    struct WasmLocal
    {
        WasmLocal() :
            location(Js::Constants::NoRegister), type(WasmTypes::Limit)
        {
        }
        WasmLocal(Js::RegSlot loc, WasmTypes::WasmType tp) :
            location(loc), type(tp)
        {
        }
        Js::RegSlot location;
        WasmTypes::WasmType type;
    };

    class WasmCompilationException
    {
    public:
        WasmCompilationException(const wchar_t* _msg, ...);
    };


    struct WasmFunction
    {
        WasmFunction() :
            body(nullptr)
        {
        }
        WasmFunction(Js::FunctionBody * body) :
            body(body)
        {
        }
        Js::FunctionBody * body;
    };

    typedef JsUtil::GrowingArray<WasmFunction*, ArenaAllocator> WasmFunctionArray;
    typedef JsUtil::BaseDictionary<LPCUTF8, uint, ArenaAllocator> WasmExportDictionary;

    struct WasmModule
    {
        WasmModule() : functions(nullptr), exports(nullptr)
        {
        }
        // TODO (michhol): use normal array, and get info from parser
        WasmFunctionArray * functions;
        WasmExportDictionary * exports;
    };

    struct WasmScript
    {
        WasmScript() : module(nullptr), globalBody(nullptr)
        {
        }

        WasmModule * module;
        Js::FunctionBody * globalBody;
    };

    class WasmBytecodeGenerator
    {
    public:
        static const Js::RegSlot ModuleSlotRegister = 0;
        static const Js::RegSlot ReturnRegister = 0;

        static const Js::RegSlot FunctionRegister = 0;
        static const Js::RegSlot CallReturnRegister = 0;
        static const Js::RegSlot ModuleEnvRegister = 1;
        static const Js::RegSlot ArrayBufferRegister = 2;
        static const Js::RegSlot ArraySizeRegister = 3;
        static const Js::RegSlot ScriptContextBufferRegister = 4;
        static const Js::RegSlot ReservedRegisterCount = 5;

        WasmBytecodeGenerator(Js::ScriptContext * scriptContext, Js::Utf8SourceInfo * sourceInfo, BaseWasmReader * reader);
        WasmScript * GenerateWasmScript();
        WasmModule * GenerateModule();
        void GenerateInvoke();
        WasmFunction * GenerateFunction();

    private:
        EmitInfo EmitExpr(WasmOp op);
        EmitInfo EmitConst();
        EmitInfo EmitBlock();
        EmitInfo EmitIfExpr();
        EmitInfo EmitGetLocal();
        EmitInfo EmitSetLocal();
        EmitInfo EmitReturnExpr();
        EmitInfo EmitBinExpr(WasmOp op);
        EmitInfo EmitCompareExpr(WasmOp op);

        void EnregisterLocals();
        void AddExport();

        void ReadParams(WasmNode * paramExpr);
        void ReadResult(WasmNode * paramExpr);
        void ReadLocals(WasmNode * localExpr);

        template <typename T> Js::RegSlot GetConstReg(T constVal);

        Js::OpCodeAsmJs GetOpCodeForBinNode() const;
        Js::OpCodeAsmJs GetOpCodeForCompareNode() const;
        Js::AsmJsRetType GetAsmJsReturnType() const;
        static Js::AsmJsVarType GetAsmJsVarType(WasmTypes::WasmType wasmType);
        WasmRegisterSpace * GetRegisterSpace(WasmTypes::WasmType type);

        ArenaAllocator m_alloc;

        WasmLocal * m_locals;

        WasmFunctionInfo * m_funcInfo;
        WasmFunction * m_func;
        WasmModule * m_module;

        uint m_nestedIfLevel;

        WasmScript * m_wasmScript;

        BaseWasmReader * m_reader;

        Js::AsmJsByteCodeWriter m_writer;
        Js::ScriptContext * m_scriptContext;
        Js::Utf8SourceInfo * m_sourceInfo;

        WasmRegisterSpace m_i32RegSlots;
        WasmRegisterSpace m_f32RegSlots;
        WasmRegisterSpace m_f64RegSlots;
    };
}
