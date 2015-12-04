#pragma once

namespace Wasm
{

    class BaseWasmReader
    {
    public:
        virtual WasmOp ReadFromScript() = 0;
        virtual WasmOp ReadFromModule() = 0;
        virtual WasmOp ReadExpr() = 0;
        virtual WasmOp ReadFromBlock() = 0;
        virtual WasmOp ReadFromCall() = 0;
        virtual bool IsBinaryReader() = 0;
        WasmNode    m_currentNode;

    private:
        WasmFunctionInfo *  m_funcInfo;
    };
} // namespace Wasm
