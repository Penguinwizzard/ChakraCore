//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// define this to include all opcodes
#ifndef WASM_OPCODE
#define WASM_OPCODE(opname, opcode, token, sig)
#endif

#ifndef WASM_SIGNATURE
#define WASM_SIGNATURE(id,...)
#endif

#ifndef WASM_CTRL_OPCODE
#define WASM_CTRL_OPCODE(opname, opcode, token, sig) WASM_OPCODE(opname, opcode, token, sig)
#endif

#ifndef WASM_MISC_OPCODE
#define WASM_MISC_OPCODE(opname, opcode, token, sig) WASM_OPCODE(opname, opcode, token, sig)
#endif

#ifndef WASM_MEM_OPCODE
#define WASM_MEM_OPCODE(opname, opcode, token, sig) WASM_OPCODE(opname, opcode, token, sig)
#endif

#ifndef WASM_SIMPLE_OPCODE
#define WASM_SIMPLE_OPCODE(opname, opcode, token, sig) WASM_OPCODE(opname, opcode, token, sig)
#endif


// built-in opcode signatures
//              id, retType, arg0, arg1, arg2
WASM_SIGNATURE(I_II,    3,   WasmTypes::bAstI32, WasmTypes::bAstI32, WasmTypes::bAstI32)
WASM_SIGNATURE(I_I,     2,   WasmTypes::bAstI32, WasmTypes::bAstI32)
WASM_SIGNATURE(I_V,     1,   WasmTypes::bAstI32) 
WASM_SIGNATURE(I_FF,    3,   WasmTypes::bAstI32, WasmTypes::bAstF32, WasmTypes::bAstF32)
WASM_SIGNATURE(I_F,     2,   WasmTypes::bAstI32, WasmTypes::bAstF32)
WASM_SIGNATURE(I_DD,    3,   WasmTypes::bAstI32, WasmTypes::bAstF64, WasmTypes::bAstF64)
WASM_SIGNATURE(I_D,     2,   WasmTypes::bAstI32, WasmTypes::bAstF64)
WASM_SIGNATURE(I_L,     2,   WasmTypes::bAstI32, WasmTypes::bAstI64)
WASM_SIGNATURE(L_LL,    3,   WasmTypes::bAstI64, WasmTypes::bAstI64, WasmTypes::bAstI64)
WASM_SIGNATURE(I_LL,    3,   WasmTypes::bAstI32, WasmTypes::bAstI64, WasmTypes::bAstI64)
WASM_SIGNATURE(L_L,     2,   WasmTypes::bAstI64, WasmTypes::bAstI64)
WASM_SIGNATURE(L_I,     2,   WasmTypes::bAstI64, WasmTypes::bAstI32)
WASM_SIGNATURE(L_F,     2,   WasmTypes::bAstI64, WasmTypes::bAstF32)
WASM_SIGNATURE(L_D,     2,   WasmTypes::bAstI64, WasmTypes::bAstF64)
WASM_SIGNATURE(F_FF,    3,   WasmTypes::bAstF32, WasmTypes::bAstF32, WasmTypes::bAstF32)
WASM_SIGNATURE(F_F,     2,   WasmTypes::bAstF32, WasmTypes::bAstF32)
WASM_SIGNATURE(F_D,     2,   WasmTypes::bAstF32, WasmTypes::bAstF64)
WASM_SIGNATURE(F_I,     2,   WasmTypes::bAstF32, WasmTypes::bAstI32)
WASM_SIGNATURE(F_L,     2,   WasmTypes::bAstF32, WasmTypes::bAstI64)
WASM_SIGNATURE(D_DD,    3,   WasmTypes::bAstF64, WasmTypes::bAstF64, WasmTypes::bAstF64)
WASM_SIGNATURE(D_D,     2,   WasmTypes::bAstF64, WasmTypes::bAstF64)
WASM_SIGNATURE(D_F,     2,   WasmTypes::bAstF64, WasmTypes::bAstF32)
WASM_SIGNATURE(D_I,     2,   WasmTypes::bAstF64, WasmTypes::bAstI32)
WASM_SIGNATURE(D_L,     2,   WasmTypes::bAstF64, WasmTypes::bAstI64)
WASM_SIGNATURE(D_ID,    3,   WasmTypes::bAstF64, WasmTypes::bAstI32, WasmTypes::bAstF64)
WASM_SIGNATURE(F_IF,    3,   WasmTypes::bAstF32, WasmTypes::bAstI32, WasmTypes::bAstF32)
WASM_SIGNATURE(L_IL,    3,   WasmTypes::bAstI64, WasmTypes::bAstI32, WasmTypes::bAstI64)


WASM_CTRL_OPCODE(Nop,            0x00,       NOP,                Limit)
WASM_CTRL_OPCODE(Block,          0x01,       BLOCK,              Limit)
WASM_CTRL_OPCODE(Loop,           0x02,       LOOP,               Limit)
WASM_CTRL_OPCODE(If,             0x03,       IF,                 Limit)
WASM_CTRL_OPCODE(IfThen,         0x04,       IFTHEN,             Limit)
WASM_CTRL_OPCODE(Select,         0x05,       SELECT,             Limit)
WASM_CTRL_OPCODE(Br,             0x06,       BR,                 Limit)
WASM_CTRL_OPCODE(BrIf,           0x07,       BRIF,               Limit)
WASM_CTRL_OPCODE(TableSwitch,    0x08,       TABLESWITCH,        Limit)
WASM_CTRL_OPCODE(Return,         0x14,       RETURN,             Limit)
WASM_CTRL_OPCODE(Unreachable,    0x15,       UNREACHABLE,        Limit)


// Constants, locals, globals, and calls.
WASM_MISC_OPCODE(I8Const,        0x09,        CONST,             Limit)
WASM_MISC_OPCODE(I32Const,       0x0a,        CONST,             Limit)
WASM_MISC_OPCODE(I64Const,       0x0b,        CONST,             Limit)
WASM_MISC_OPCODE(F64Const,       0x0c,        CONST,             Limit)
WASM_MISC_OPCODE(F32Const,       0x0d,        CONST,             Limit)
WASM_MISC_OPCODE(GetLocal,       0x0e,        GETLOCAL,          Limit)
WASM_MISC_OPCODE(SetLocal,       0x0f,        SETLOCAL,          Limit)
WASM_MISC_OPCODE(GetGlobal,      0x10,        GETGLOBAL,         Limit)
WASM_MISC_OPCODE(SetGlobal,      0x11,        SETGLOBAL,         Limit)
WASM_MISC_OPCODE(Call,           0x12,        CALL,              Limit)
WASM_MISC_OPCODE(CallIndirect,   0x13,        DISPATCH,          Limit)

// Load memory expressions.
// TODO: Map to node ops
WASM_MEM_OPCODE(I32LoadMem8S,    0x20,        LIMIT,                 I_I)
WASM_MEM_OPCODE(I32LoadMem8U,    0x21,        LIMIT,                 I_I)
WASM_MEM_OPCODE(I32LoadMem16S,   0x22,        LIMIT,                 I_I)
WASM_MEM_OPCODE(I32LoadMem16U,   0x23,        LIMIT,                 I_I)
WASM_MEM_OPCODE(I64LoadMem8S,    0x24,        LIMIT,                 L_I)
WASM_MEM_OPCODE(I64LoadMem8U,    0x25,        LIMIT,                 L_I)
WASM_MEM_OPCODE(I64LoadMem16S,   0x26,        LIMIT,                 L_I)
WASM_MEM_OPCODE(I64LoadMem16U,   0x27,        LIMIT,                 L_I)
WASM_MEM_OPCODE(I64LoadMem32S,   0x28,        LIMIT,                 L_I)
WASM_MEM_OPCODE(I64LoadMem32U,   0x29,        LIMIT,                 L_I)
WASM_MEM_OPCODE(I32LoadMem,      0x2a,        LIMIT,                 I_I)
WASM_MEM_OPCODE(I64LoadMem,      0x2b,        LIMIT,                 L_I)
WASM_MEM_OPCODE(F32LoadMem,      0x2c,        LIMIT,                 F_I)
WASM_MEM_OPCODE(F64LoadMem,      0x2d,        LIMIT,                 D_I)
                                              
// Store memory expressIons.                  
WASM_MEM_OPCODE(I32StoreMem8,    0x2e,        LIMIT,                I_II)
WASM_MEM_OPCODE(I32StoreMem16,   0x2f,        LIMIT,                I_II)
WASM_MEM_OPCODE(I64StoreMem8,    0x30,        LIMIT,                L_IL)
WASM_MEM_OPCODE(I64StoreMem16,   0x31,        LIMIT,                L_IL)
WASM_MEM_OPCODE(I64StoreMem32,   0x32,        LIMIT,                L_IL)
WASM_MEM_OPCODE(I32StoreMem,     0x33,        LIMIT,                I_II)
WASM_MEM_OPCODE(I64StoreMem,     0x34,        LIMIT,                L_IL)
WASM_MEM_OPCODE(F32StoreMem,     0x35,        LIMIT,                F_IF)
WASM_MEM_OPCODE(F64StoreMem,     0x36,        LIMIT,                D_ID)
                                              
// Load memory                                
WASM_MEM_OPCODE(MemorySize,      0x3b,        LIMIT,                I_V)
WASM_MEM_OPCODE(ResizeMemL,      0x39,        LIMIT,                I_I)
WASM_MEM_OPCODE(ResizeMemH,      0x3a,        LIMIT,                L_L)

// Expressions
WASM_SIMPLE_OPCODE(I32Add,              0x40, ADD,              I_II)
WASM_SIMPLE_OPCODE(I32Sub,              0x41, Sub,              I_II)
WASM_SIMPLE_OPCODE(I32Mul,              0x42, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32DivS,             0x43, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32DivU,             0x44, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32RemS,             0x45, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32RemU,             0x46, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32And,              0x47, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32Ior,              0x48, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32Xor,              0x49, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32Shl,              0x4a, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32ShrU,             0x4b, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32ShrS,             0x4c, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32Eq,               0x4d, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32Ne,               0x4e, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32LtS,              0x4f, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32LeS,              0x50, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32LtU,              0x51, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32LeU,              0x52, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32GtS,              0x53, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32GeS,              0x54, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32GtU,              0x55, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32GeU,              0x56, LIMIT,            I_II)
WASM_SIMPLE_OPCODE(I32Clz,              0x57, LIMIT,            I_I)
WASM_SIMPLE_OPCODE(I32Ctz,              0x58, LIMIT,            I_I)
WASM_SIMPLE_OPCODE(I32Popcnt,           0x59, LIMIT,            I_I)
WASM_SIMPLE_OPCODE(BoolNot,             0x5a, LIMIT,            I_I)
WASM_SIMPLE_OPCODE(I64Add,              0x5b, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64Sub,              0x5c, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64Mul,              0x5d, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64DivS,             0x5e, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64DivU,             0x5f, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64RemS,             0x60, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64RemU,             0x61, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64And,              0x62, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64Ior,              0x63, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64Xor,              0x64, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64Shl,              0x65, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64ShrU,             0x66, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64ShrS,             0x67, LIMIT,            L_LL)
WASM_SIMPLE_OPCODE(I64Eq,               0x68, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64Ne,               0x69, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64LtS,              0x6a, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64LeS,              0x6b, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64LtU,              0x6c, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64LeU,              0x6d, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64GtS,              0x6e, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64GeS,              0x6f, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64GtU,              0x70, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64GeU,              0x71, LIMIT,            I_LL)
WASM_SIMPLE_OPCODE(I64Clz,              0x72, LIMIT,            L_L)
WASM_SIMPLE_OPCODE(I64Ctz,              0x73, LIMIT,            L_L)
WASM_SIMPLE_OPCODE(I64Popcnt,           0x74, LIMIT,            L_L)
WASM_SIMPLE_OPCODE(F32Add,              0x75, LIMIT,            F_FF)
WASM_SIMPLE_OPCODE(F32Sub,              0x76, LIMIT,            F_FF)
WASM_SIMPLE_OPCODE(F32Mul,              0x77, LIMIT,            F_FF)
WASM_SIMPLE_OPCODE(F32DIv,              0x78, LIMIT,            F_FF)
WASM_SIMPLE_OPCODE(F32Min,              0x79, LIMIT,            F_FF)
WASM_SIMPLE_OPCODE(F32Max,              0x7a, LIMIT,            F_FF)
WASM_SIMPLE_OPCODE(F32Abs,              0x7b, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32Neg,              0x7c, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32CopySign,         0x7d, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32Ceil,             0x7e, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32Floor,            0x7f, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32Trunc,            0x80, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32NearestInt,       0x81, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32Sqrt,             0x82, LIMIT,            F_F)
WASM_SIMPLE_OPCODE(F32Eq,               0x83, LIMIT,            I_FF)
WASM_SIMPLE_OPCODE(F32Ne,               0x84, LIMIT,            I_FF)
WASM_SIMPLE_OPCODE(F32Lt,               0x85, LIMIT,            I_FF)
WASM_SIMPLE_OPCODE(F32Le,               0x86, LIMIT,            I_FF)
WASM_SIMPLE_OPCODE(F32Gt,               0x87, LIMIT,            I_FF)
WASM_SIMPLE_OPCODE(F32Ge,               0x88, LIMIT,            I_FF)
WASM_SIMPLE_OPCODE(F64Add,              0x89, LIMIT,            D_DD)
WASM_SIMPLE_OPCODE(F64Sub,              0x8a, LIMIT,            D_DD)
WASM_SIMPLE_OPCODE(F64Mul,              0x8b, LIMIT,            D_DD)
WASM_SIMPLE_OPCODE(F64Div,              0x8c, LIMIT,            D_DD)
WASM_SIMPLE_OPCODE(F64Min,              0x8d, LIMIT,            D_DD)
WASM_SIMPLE_OPCODE(F64Max,              0x8e, LIMIT,            D_DD)
WASM_SIMPLE_OPCODE(F64Abs,              0x8f, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64Neg,              0x90, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64CopySign,         0x91, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64Ceil,             0x92, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64Floor,            0x93, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64Trunc,            0x94, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64NearestInt,       0x95, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64Sqrt,             0x96, LIMIT,            D_D)
WASM_SIMPLE_OPCODE(F64Eq,               0x97, LIMIT,            I_DD)
WASM_SIMPLE_OPCODE(F64Ne,               0x98, LIMIT,            I_DD)
WASM_SIMPLE_OPCODE(F64Lt,               0x99, LIMIT,            I_DD)
WASM_SIMPLE_OPCODE(F64Le,               0x9a, LIMIT,            I_DD)
WASM_SIMPLE_OPCODE(F64Gt,               0x9b, LIMIT,            I_DD)
WASM_SIMPLE_OPCODE(F64Ge,               0x9c, LIMIT,            I_DD)
WASM_SIMPLE_OPCODE(I32SConvertF32,      0x9d, LIMIT,            I_F)
WASM_SIMPLE_OPCODE(I32SConvertF64,      0x9e, LIMIT,            I_D)
WASM_SIMPLE_OPCODE(I32UConvertF32,      0x9f, LIMIT,            I_F)
WASM_SIMPLE_OPCODE(I32UConvertF64,      0xa0, LIMIT,            I_D)
WASM_SIMPLE_OPCODE(I32ConvertI64,       0xa1, LIMIT,            I_L)
WASM_SIMPLE_OPCODE(I64SConvertF32,      0xa2, LIMIT,            L_F)
WASM_SIMPLE_OPCODE(I64SConvertF64,      0xa3, LIMIT,            L_D)
WASM_SIMPLE_OPCODE(I64UConvertF32,      0xa4, LIMIT,            L_F)
WASM_SIMPLE_OPCODE(I64UConvertF64,      0xa5, LIMIT,            L_D)
WASM_SIMPLE_OPCODE(I64SConvertI32,      0xa6, LIMIT,            L_I)
WASM_SIMPLE_OPCODE(I64UConvertI32,      0xa7, LIMIT,            L_I)
WASM_SIMPLE_OPCODE(F32SConvertI32,      0xa8, LIMIT,            F_I)
WASM_SIMPLE_OPCODE(F32UConvertI32,      0xa9, LIMIT,            F_I)
WASM_SIMPLE_OPCODE(F32SConvertI64,      0xaa, LIMIT,            F_L)
WASM_SIMPLE_OPCODE(F32UConvertI64,      0xab, LIMIT,            F_L)
WASM_SIMPLE_OPCODE(F32ConvertF64,       0xac, LIMIT,            F_D)
WASM_SIMPLE_OPCODE(F32ReinterpretI32,   0xad, LIMIT,            F_I)
WASM_SIMPLE_OPCODE(F64SConvertI32,      0xae, LIMIT,            D_I)
WASM_SIMPLE_OPCODE(F64UConvertI32,      0xaf, LIMIT,            D_I)
WASM_SIMPLE_OPCODE(F64SConvertI64,      0xb0, LIMIT,            D_L)
WASM_SIMPLE_OPCODE(F64UConvertI64,      0xb1, LIMIT,            D_L)
WASM_SIMPLE_OPCODE(F64ConvertF32,       0xb2, LIMIT,            D_F)
WASM_SIMPLE_OPCODE(F64ReinterpretI64,   0xb3, LIMIT,            D_L)
WASM_SIMPLE_OPCODE(I32ReinterpretF32,   0xb4, LIMIT,            I_F)
WASM_SIMPLE_OPCODE(I64ReinterpretF64,   0xb5, LIMIT,            L_D)

#undef WASM_SIMPLE_OPCODE
#undef WASM_MEM_OPCODE
#undef WASM_MISC_OPCODE
#undef WASM_CTRL_OPCODE
#undef WASM_OPCODE
#undef WASM_SIGNATURE

