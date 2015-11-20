//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once
#ifdef ENABLE_WASM
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
                bAstStmt = 0,  // void
                bAstI32 = 1,
                bAstI64 = 2,
                bAstF32 = 3,
                bAstF64 = 4,
                bAstLimit
            };

            // memory and global types in binary format
            enum MemType {
                bMemI8 = 0,
                bMemU8 = 1,
                bMemI16 = 2,
                bMemU16 = 3,
                bMemI32 = 4,
                bMemU32 = 5,
                bMemI64 = 6,
                bMemU64 = 7,
                bMemF32 = 8,
                bMemF64 = 9,
                bMemLimit
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
#define WASM_SIGNATURE(id, ...) bSig##id,
#include "WasmBinaryOpcodes.h"
                bSigLimit
            };
        } // namespace WasmTypes

        // binary opcodes
        enum OpCodes
        {
#define WASM_OPCODE(opname, opcode, token, sig) wb##opname = opcode,
#include "WasmBinaryOpcodes.h"
            wbLimit
        };

        class WasmBinaryReader : public BaseWasmReader
        {
        public:
            WasmBinaryReader(PageAllocator * alloc, byte* source, size_t length);

            virtual bool IsBinaryReader() override;
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
            
            WasmTypes::Signature opSignatureTable[WasmTypes::OpSignatureId::bSigLimit]; // table of opcode signatures
            WasmTypes::OpSignatureId opSignature[256];                                  // opcode -> opcode signature ID

        };

    }

} // namespace Wasm
#endif // ENABLE_WASM