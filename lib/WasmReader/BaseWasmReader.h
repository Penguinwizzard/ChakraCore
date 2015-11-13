#pragma once

namespace Wasm
{
    class BaseWasmReader
    {
    public:
        virtual WasmOp ReadFromScript() = 0;
        virtual WasmOp ReadFromModule() = 0;
        virtual WasmOp ReadExpr() = 0;

        WasmNode    m_currentNode;

    private:
        WasmFunctionInfo *  m_funcInfo;
    };
} // namespace Wasm
