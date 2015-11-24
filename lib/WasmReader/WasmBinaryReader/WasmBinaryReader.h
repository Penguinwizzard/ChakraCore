//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

namespace Wasm
{
    namespace Binary
    {
        struct DecoderContext
        {
            byte* source;
            size_t offset;
            size_t length;
        };

        namespace WasmTypes
        {
            // based on binary format encoding values
            enum LocalType {
                Stmt = 0,  // void
                I32 = 1,
                I64 = 2,
                F32 = 3,
                F64 = 4,
                Limit
            };

            // memory and global types in binary format
            enum MemType {
                I8 = 0,
                U8 = 1,
                I16 = 2,
                U16 = 3,
                I32 = 4,
                U32 = 5,
                I64 = 6,
                U64 = 7,
                F32 = 8,
                F64 = 9,
                Limit
            };

            // for functions and opcodes
            class Signature
            {
            private:
                LocalType *args;
                LocalType retType;
                uint32 argCount;
            public:
                Signature();
                Signature(ArenaAllocator *alloc, uint count, ...);
            };

            enum OpSignatureId
            {
#define WASM_SIGNATURE(id, ...) id,
#include "WasmBinaryOpcodes.h"
                Limit
            };
        }

        // binary opcodes
        enum OpCodes
        {
#define WASM_OPCODE(opname, opcode, token, sig) opname = opcode,
#include "WasmBinaryOpcodes.h"
            Limit
        };

        class WasmBinaryReader : public BaseWasmReader
        {
        public:
            WasmBinaryReader(PageAllocator * alloc, byte* source, size_t length);

            virtual WasmOp ReadFromScript() override;
            virtual WasmOp ReadFromModule() override;
            virtual WasmOp ReadExpr() override;

            static void __declspec(noreturn) ThrowSyntaxError();

        protected:
            DecoderContext   m_context;


        private:
            ArenaAllocator      m_alloc;
            uint m_funcNumber;
            WasmFunctionInfo *  m_funcInfo;

            byte *m_start, *m_end;
            
            WasmTypes::Signature opSignatureTable[WasmTypes::OpSignatureId::Limit];
            WasmTypes::OpSignatureId opSignature[256];

        };

    }

} // namespace Wasm
