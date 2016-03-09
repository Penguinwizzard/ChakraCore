//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

namespace Wasm
{

    namespace WasmTypes
    {
        enum WasmType
        {
            Void,
#define WASM_LOCALTYPE(token, name) token,
#include "WasmKeywords.h"
            Limit
        };
    }

    enum WasmOp
    {
#define WASM_KEYWORD(token, name) wn##token,
#include "WasmKeywords.h"
        wnLIMIT
    };

    struct WasmFuncNode
    {
        WasmFunctionInfo * info;

    };

    struct WasmConstLitNode
    {
        union
        {
            float f32;
            double f64;
            int32 i32;
            int64 i64;
        };
    };

    struct WasmVarNode
    {
        uint num;
        union
        {
            LPCUTF8 exportName;
        };
    };

    struct WasmOptionalNode
    {
        bool exists;
    };

    struct WasmBlockNode
    {
        uint32 count;
    };

    struct WasmMemOpNode
    {
        uint32 offset;
        uint8 alignment;
    };

    struct WasmBrNode
    {
        uint32 depth;
        bool hasSubExpr;
    };

    struct WasmTableSwitchNode
    {
        uint32 numCases;
        uint32 numEntries;
        UINT16* jumpTable;
    };

    struct WasmNode
    {
        WasmOp op;
        union
        {
            WasmVarNode var;
            WasmConstLitNode cnst;
            WasmFuncNode func;
            WasmOptionalNode opt;
            WasmBlockNode block;
            WasmBrNode br;
            WasmTableSwitchNode tableswitch;
            WasmMemOpNode mem;
        };
    };

    struct WasmExport
    {
        uint32 funcIndex;
        uint32 nameLength;
        wchar_t* name;
    };
}

#define FOREACH_WASMNODE_IN_LIST(node, head) \
    Wasm::WasmNode * node; \
    Wasm::WasmNode * wasmNodeList; \
    if (head->op == wnLIST) \
    { \
        wasmNodeList = head; \
        node = head->bin.lhs; \
    } \
    else \
    { \
        wasmNodeList = nullptr; \
        node = head; \
    } \
    while (node != nullptr) \
    {

#define NEXT_WASMNODE_IN_LIST(node) \
        if (wasmNodeList != nullptr && wasmNodeList->bin.rhs != nullptr) \
        { \
            wasmNodeList = wasmNodeList->bin.rhs; \
            node = wasmNodeList->bin.lhs; \
        } \
        else \
        { \
            node = nullptr; \
        } \
    }
