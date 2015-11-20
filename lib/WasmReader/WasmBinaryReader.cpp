//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "WasmReaderPch.h"

#ifdef ENABLE_WASM

namespace Wasm
{

    WasmBinaryReader::WasmBinaryReader(PageAllocator * alloc, byte* source, size_t length) :
        m_alloc(L"WasmBinaryDecoder", alloc, Js::Throw::OutOfMemory),
    {
        m_context.offset = 0;
        m_context.source = source;
        m_context.length = length;
        m_start = source;
        m_end = source + length;
    }

    WasmOp
    WasmBinaryReader::ReadFromScript()
    {
            // One module per file
            return wnMODULE;
    }

    WasmOp
    WasmBinaryReader::ReadFromModule()
    {

    }

    WasmOp
    WasmBinaryReader::ReadExpr()
    {
    }

   
} // namespace Wasm

#endif // ENABLE_WASM
