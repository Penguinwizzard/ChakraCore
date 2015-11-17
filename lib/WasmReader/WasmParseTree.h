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
#define WASM_MEMTYPE(token, name) token,
#include "WasmKeywords.h"
            Void,
            Limit
        };
    }

    enum WasmOp
    {
        wnLIST,
        wnFUNC,
        wnMODULE,
        wnPARAM,
        wnGLOBAL,
        wnRESULT,
        wnLOCAL,
        wnTABLE,
        wnNOP,
        wnBLOCK,
        wnLOOP,
        wnLABEL,
        wnBREAK,
        wnSWITCH,
        wnRETURN,
        wnGETLOCAL,
        wnSETLOCAL,
        wnGetGlobal,
        wnSetGlobal,
        wnGetNearS,
        wnGetNearU,
        wnGetNearUnalignedS,
        wnGetNearUnalignedU,
        wnSetNearS,
        wnSetNearU,
        wnSetNearUnalignedS,
        wnSetNearUnalignedU,
        wnCONST,
        wnEXPORT,
        wnINVOKE,
        wnASSERT,
        wnMEMORY,
        wnDATA,
#define WASM_KEYWORD_BIN(token, name) wn##token,
#define WASM_KEYWORD_UNARY(token, name) wn##token,
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

    struct WasmInvokeNode
    {
        WasmNode * expr;
        union
        {
            LPCUTF8 name;
        };
    };

    struct WasmNode
    {
        WasmOp op;
        WasmTypes::WasmType type;
        union
        {
            WasmVarNode var;
            WasmConstLitNode cnst;
            WasmInvokeNode invk;
            WasmFuncNode func;
            WasmOptionalNode opt;
        };
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
