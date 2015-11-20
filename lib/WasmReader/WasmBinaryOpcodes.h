//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#pragma once

#ifndef WASM_OPCODE
#define WASM_OPCODE(opname, opcode, token)
#endif

#ifndef WASM_OPCODE_BIN
#define WASM_OPCODE_BIN(opname, opcode, token) WASM_OPCODE(opname, opcode, token)
#endif

#ifndef WASM_OPCODE_COMPARE
#define WASM_OPCODE_COMPARE(opname, opcode, token) WASM_OPCODE_BIN(opname, opcode, token)
#endif

#ifndef WASM_OPCODE_BIN_MATH
#define WASM_OPCODE_BIN_MATH(opname, opcode, token) WASM_OPCODE_BIN(opname, opcode, token)
#endif

#ifndef WASM_OPCODE_UNARY
#define WASM_OPCODE_UNARY(opname, opcode, token) WASM_OPCODE(opname, opcode, token)
#endif

#ifndef WASM_MEMTYPE
#define WASM_MEMTYPE(opname, opcode, token) WASM_OPCODE(opname, opcode, token)
#endif

#ifndef WASM_LOCALTYPE
#define WASM_LOCALTYPE(opname, opcode, token) WASM_MEMTYPE(opname, opcode, token)
#endif

// names here should match keywrod names for SExpr
WASM_OPCODE(Nop,            0x00,       NOP                )
WASM_OPCODE(Block,          0x01,       BLOCK              )
WASM_OPCODE(Loop,           0x02,       LOOP               )
WASM_OPCODE(If,             0x03,       IF                 )
WASM_OPCODE(IfThen,         0x04,       IFTHEN             )
WASM_OPCODE(Select,         0x05,       SELECT             )
WASM_OPCODE(Br,             0x06,       BR                 )
WASM_OPCODE(BrIf,           0x07,       BRIF               )
WASM_OPCODE(TableSwitch,    0x08,       TABLESWITCH        )
WASM_OPCODE(Return,         0x14,       RETURN             )
WASM_OPCODE(Unreachable,    0x15,       UNREACHABLE        )


    // Constants, locals, globals, and calls.
#define FOREACH_MISC_OPCODE(V)
WASM_OPCODE(I8Const,        0x09,        CONST)
WASM_OPCODE(I32Const,       0x0a,        CONST)
WASM_OPCODE(I64Const,       0x0b,        CONST)
WASM_OPCODE(F64Const,       0x0c,        CONST)
WASM_OPCODE(F32Const,       0x0d,        CONST)
WASM_OPCODE(GetLocal,       0x0e,        GETLOCAL)
WASM_OPCODE(SetLocal,       0x0f,        SETLOCAL)
WASM_OPCODE(GetGlobal,      0x10,        GETGLOBAL)
WASM_OPCODE(SetGlobal,      0x11,        SETGLOBAL)
WASM_OPCODE(Call,           0x12,        CALL)
WASM_OPCODE(CallIndirect,   0x13,        DISPATCH)

    // Load memory expressions.
#define FOREACH_LOAD_MEM_OPCODE(V)
    V(I32LoadMem8S, 0x20, i_i)         
    V(I32LoadMem8U, 0x21, i_i)         
    V(I32LoadMem16S, 0x22, i_i)         
    V(I32LoadMem16U, 0x23, i_i)         
    V(I64LoadMem8S, 0x24, l_i)         
    V(I64LoadMem8U, 0x25, l_i)         
    V(I64LoadMem16S, 0x26, l_i)         
    V(I64LoadMem16U, 0x27, l_i)         
    V(I64LoadMem32S, 0x28, l_i)         
    V(I64LoadMem32U, 0x29, l_i)         
    V(I32LoadMem, 0x2a, i_i)         
    V(I64LoadMem, 0x2b, l_i)         
    V(F32LoadMem, 0x2c, f_i)         
    V(F64LoadMem, 0x2d, d_i)

    // Store memory expressions.
#define FOREACH_STORE_MEM_OPCODE(V)
    V(I32StoreMem8, 0x2e, i_ii)          
    V(I32StoreMem16, 0x2f, i_ii)          
    V(I64StoreMem8, 0x30, l_il)          
    V(I64StoreMem16, 0x31, l_il)          
    V(I64StoreMem32, 0x32, l_il)          
    V(I32StoreMem, 0x33, i_ii)          
    V(I64StoreMem, 0x34, l_il)          
    V(F32StoreMem, 0x35, f_if)          
    V(F64StoreMem, 0x36, d_id)

    // Load memory expressions.
#define FOREACH_MISC_MEM_OPCODE(V)
    V(MemorySize, 0x3b, i_v)             
    V(ResizeMemL, 0x39, i_i)             
    V(ResizeMemH, 0x3a, l_l)

    // Expressions with signatures.
#define FOREACH_SIMPLE_OPCODE(V)
    V(I32Add, 0x40, i_ii)              
    V(I32Sub, 0x41, i_ii)              
    V(I32Mul, 0x42, i_ii)              
    V(I32DivS, 0x43, i_ii)             
    V(I32DivU, 0x44, i_ii)             
    V(I32RemS, 0x45, i_ii)             
    V(I32RemU, 0x46, i_ii)             
    V(I32And, 0x47, i_ii)              
    V(I32Ior, 0x48, i_ii)              
    V(I32Xor, 0x49, i_ii)              
    V(I32Shl, 0x4a, i_ii)              
    V(I32ShrU, 0x4b, i_ii)             
    V(I32ShrS, 0x4c, i_ii)             
    V(I32Eq, 0x4d, i_ii)               
    V(I32Ne, 0x4e, i_ii)               
    V(I32LtS, 0x4f, i_ii)              
    V(I32LeS, 0x50, i_ii)              
    V(I32LtU, 0x51, i_ii)              
    V(I32LeU, 0x52, i_ii)              
    V(I32GtS, 0x53, i_ii)              
    V(I32GeS, 0x54, i_ii)              
    V(I32GtU, 0x55, i_ii)              
    V(I32GeU, 0x56, i_ii)              
    V(I32Clz, 0x57, i_i)               
    V(I32Ctz, 0x58, i_i)               
    V(I32Popcnt, 0x59, i_i)            
    V(BoolNot, 0x5a, i_i)              
    V(I64Add, 0x5b, l_ll)              
    V(I64Sub, 0x5c, l_ll)              
    V(I64Mul, 0x5d, l_ll)              
    V(I64DivS, 0x5e, l_ll)             
    V(I64DivU, 0x5f, l_ll)             
    V(I64RemS, 0x60, l_ll)             
    V(I64RemU, 0x61, l_ll)             
    V(I64And, 0x62, l_ll)              
    V(I64Ior, 0x63, l_ll)              
    V(I64Xor, 0x64, l_ll)              
    V(I64Shl, 0x65, l_ll)              
    V(I64ShrU, 0x66, l_ll)             
    V(I64ShrS, 0x67, l_ll)             
    V(I64Eq, 0x68, i_ll)               
    V(I64Ne, 0x69, i_ll)               
    V(I64LtS, 0x6a, i_ll)              
    V(I64LeS, 0x6b, i_ll)              
    V(I64LtU, 0x6c, i_ll)              
    V(I64LeU, 0x6d, i_ll)              
    V(I64GtS, 0x6e, i_ll)              
    V(I64GeS, 0x6f, i_ll)              
    V(I64GtU, 0x70, i_ll)              
    V(I64GeU, 0x71, i_ll)              
    V(I64Clz, 0x72, l_l)               
    V(I64Ctz, 0x73, l_l)               
    V(I64Popcnt, 0x74, l_l)            
    V(F32Add, 0x75, f_ff)              
    V(F32Sub, 0x76, f_ff)              
    V(F32Mul, 0x77, f_ff)              
    V(F32Div, 0x78, f_ff)              
    V(F32Min, 0x79, f_ff)              
    V(F32Max, 0x7a, f_ff)              
    V(F32Abs, 0x7b, f_f)               
    V(F32Neg, 0x7c, f_f)               
    V(F32CopySign, 0x7d, f_f)          
    V(F32Ceil, 0x7e, f_f)              
    V(F32Floor, 0x7f, f_f)             
    V(F32Trunc, 0x80, f_f)             
    V(F32NearestInt, 0x81, f_f)        
    V(F32Sqrt, 0x82, f_f)              
    V(F32Eq, 0x83, i_ff)               
    V(F32Ne, 0x84, i_ff)               
    V(F32Lt, 0x85, i_ff)               
    V(F32Le, 0x86, i_ff)               
    V(F32Gt, 0x87, i_ff)               
    V(F32Ge, 0x88, i_ff)               
    V(F64Add, 0x89, d_dd)              
    V(F64Sub, 0x8a, d_dd)              
    V(F64Mul, 0x8b, d_dd)              
    V(F64Div, 0x8c, d_dd)              
    V(F64Min, 0x8d, d_dd)              
    V(F64Max, 0x8e, d_dd)              
    V(F64Abs, 0x8f, d_d)               
    V(F64Neg, 0x90, d_d)               
    V(F64CopySign, 0x91, d_d)          
    V(F64Ceil, 0x92, d_d)              
    V(F64Floor, 0x93, d_d)             
    V(F64Trunc, 0x94, d_d)             
    V(F64NearestInt, 0x95, d_d)        
    V(F64Sqrt, 0x96, d_d)              
    V(F64Eq, 0x97, i_dd)               
    V(F64Ne, 0x98, i_dd)               
    V(F64Lt, 0x99, i_dd)               
    V(F64Le, 0x9a, i_dd)               
    V(F64Gt, 0x9b, i_dd)               
    V(F64Ge, 0x9c, i_dd)               
    V(I32SConvertF32, 0x9d, i_f)       
    V(I32SConvertF64, 0x9e, i_d)       
    V(I32UConvertF32, 0x9f, i_f)       
    V(I32UConvertF64, 0xa0, i_d)       
    V(I32ConvertI64, 0xa1, i_l)        
    V(I64SConvertF32, 0xa2, l_f)       
    V(I64SConvertF64, 0xa3, l_d)       
    V(I64UConvertF32, 0xa4, l_f)       
    V(I64UConvertF64, 0xa5, l_d)       
    V(I64SConvertI32, 0xa6, l_i)       
    V(I64UConvertI32, 0xa7, l_i)       
    V(F32SConvertI32, 0xa8, f_i)       
    V(F32UConvertI32, 0xa9, f_i)       
    V(F32SConvertI64, 0xaa, f_l)       
    V(F32UConvertI64, 0xab, f_l)       
    V(F32ConvertF64, 0xac, f_d)        
    V(F32ReinterpretI32, 0xad, f_i)    
    V(F64SConvertI32, 0xae, d_i)       
    V(F64UConvertI32, 0xaf, d_i)       
    V(F64SConvertI64, 0xb0, d_l)       
    V(F64UConvertI64, 0xb1, d_l)       
    V(F64ConvertF32, 0xb2, d_f)        
    V(F64ReinterpretI64, 0xb3, d_l)    
    V(I32ReinterpretF32, 0xb4, i_f)    
    V(I64ReinterpretF64, 0xb5, l_d)

namespace Wasm
{

}