//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

namespace Wasm
{
    struct SExprParseContext
    {
        LPCUTF8 source;
        size_t offset;
        size_t length;
    };

    class SExprParser : public BaseWasmReader
    {
    public:
        SExprParser(PageAllocator * alloc, LPCUTF8 source, size_t length);

        virtual WasmOp ReadFromScript() override;
        virtual WasmOp ReadFromModule() override;
        virtual WasmOp ReadExpr() override;

        static void __declspec(noreturn) ThrowSyntaxError();

    protected:
        SExprParseContext   m_context;
        SExprToken          m_token;

    private:
        WasmOp ParseFunctionHeader();
        WasmOp ParseExport();
        void ParseParam();
        void ParseResult();
        void ParseLocal();
        WasmOp ParseReturnExpr();
        WasmOp ParseConstLitExpr();
        void ParseBasicExpr(WasmOp opcode);
        WasmNode * ParseInvoke();
        WasmNode * ParseAssertEq();

        void ParseVarNode(WasmOp opcode);

        bool IsEndOfExpr(SExprTokenType tok) const;
        WasmTypes::WasmType GetWasmType(SExprTokenType tok) const;

        ArenaAllocator      m_alloc;
        SExprScanner *      m_scanner;

        uint m_funcNumber;
        typedef JsUtil::BaseDictionary<LPCUTF8, uint, ArenaAllocator> NameToIndexMap;
        NameToIndexMap * m_nameToFuncMap;

        NameToIndexMap * m_nameToLocalMap;

        WasmFunctionInfo *  m_funcInfo;
        uint m_nestedRParens;

        bool m_inExpr;
    };

} // namespace Wasm
