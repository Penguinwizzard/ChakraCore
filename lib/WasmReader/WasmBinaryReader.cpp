//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "WasmReaderPch.h"

#ifdef ENABLE_WASM

namespace Wasm
{

    namespace Binary
    {
        namespace WasmTypes
        {

                Signature::Signature() : args(nullptr), retType(LocalType::Limit), argCount(-1){}
                Signature::Signature(ArenaAllocator *alloc, uint count, ...)
                {
                    va_list arguments;
                    va_start(arguments, count);
                
                    assert(count > 0);
                    argCount = count - 1;
                    retType = va_arg(arguments, LocalType);
                    args = AnewArray(alloc, LocalType, this.argCount);
                    for (uint i = 0; i < argCount; i++)
                    {
                        args[i] = va_arg(arguments, LocalType);
                    }
                    va_end(arguments);
                }
        } // namespace WasmTypes
    
        WasmBinaryReader::WasmBinaryReader(PageAllocator * alloc, byte* source, size_t length) :
            m_alloc(L"WasmBinaryDecoder", alloc, Js::Throw::OutOfMemory)
        {
            m_context.offset = 0;
            m_context.source = source;
            m_context.length = length;
            m_start = source;
            m_end = source + length;

            // initialize Op Signature Table
            {
#define WASM_SIGNATURE(id, count, ...) \
    AssertMsg(count >= 0 && count <= 3, "Invalid count for op signature"); \
    AssertMsg(id >= 0 && id < WasmTypes::OpSignatureId::Limit, "Invalid signature ID for op"); \
    opSignatureTable[id] = WasmTypes::Signature(&m_alloc, count, __VA_ARGS__);

#include "WasmBinaryOpcodes.h";
           }

           // initialize opcode to op signature table
            {
#define WASM_OPCODE(opname, opcode, token, sig) \
    opSignature[OpCodes::opname] = sig;
            }

       }

       WasmOp
       WasmBinaryReader::ReadFromScript()
       {
               // One module per binary file
               return wnMODULE;
       }

       WasmOp
       WasmBinaryReader::ReadFromModule()
       {
           // Read all module sections.


       }

       WasmOp
       WasmBinaryReader::ReadExpr()
       {
       }

    } // namespace Binary
} // namespace Wasm

#endif // ENABLE_WASM
