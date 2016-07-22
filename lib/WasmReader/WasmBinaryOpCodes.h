//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation and contributors. All rights reserved.
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

//               OpCode Name,    Encoding,   Sexpr token,        Signature
WASM_CTRL_OPCODE(Nop,            0x00,       NOP,                Limit)
WASM_CTRL_OPCODE(Block,          0x01,       BLOCK,              Limit)
WASM_CTRL_OPCODE(Loop,           0x02,       LOOP,               Limit)
WASM_CTRL_OPCODE(If,             0x03,       IF,                 Limit)
WASM_CTRL_OPCODE(Else,           0x04,       ELSE,               Limit)

WASM_CTRL_OPCODE(Br,             0x06,       BR,                 Limit)
WASM_CTRL_OPCODE(BrIf,           0x07,       BR_IF,              Limit)
WASM_CTRL_OPCODE(BrTable,        0x08,       BR_TABLE,           Limit)
WASM_CTRL_OPCODE(Return,         0x09,       RETURN,             Limit)
WASM_CTRL_OPCODE(Unreachable,    0x0a,       NYI,                Limit)
WASM_CTRL_OPCODE(End,            0x0f,       END,                Limit)

WASM_SIMPLE_OPCODE(Select,       0x05,       SELECT,             Limit)

// Constants, locals, globals, and calls.
WASM_MISC_OPCODE(I32Const,       0x10,        CONST_I32,         Limit)
WASM_MISC_OPCODE(I64Const,       0x11,        NYI,               Limit)
WASM_MISC_OPCODE(F64Const,       0x12,        CONST_F64,         Limit)
WASM_MISC_OPCODE(F32Const,       0x13,        CONST_F32,         Limit)
WASM_MISC_OPCODE(GetLocal,       0x14,        GETLOCAL,          Limit)
WASM_MISC_OPCODE(SetLocal,       0x15,        SETLOCAL,          Limit)
WASM_MISC_OPCODE(Call,           0x16,        CALL,              Limit)
WASM_MISC_OPCODE(CallIndirect,   0x17,        CALL_INDIRECT,     Limit)
WASM_MISC_OPCODE(CallImport,     0x18,        CALL_IMPORT,       Limit)

// Load memory expressions.
// TODO: Map to node
WASM_MEM_OPCODE(I32LoadMem8S,    0x20,        LOAD8S_I32,       I_I)
WASM_MEM_OPCODE(I32LoadMem8U,    0x21,        LOAD8U_I32,       I_I)
WASM_MEM_OPCODE(I32LoadMem16S,   0x22,        LOAD16S_I32,      I_I)
WASM_MEM_OPCODE(I32LoadMem16U,   0x23,        LOAD16U_I32,      I_I)
WASM_MEM_OPCODE(I64LoadMem8S,    0x24,        NYI,              L_I)
WASM_MEM_OPCODE(I64LoadMem8U,    0x25,        NYI,              L_I)
WASM_MEM_OPCODE(I64LoadMem16S,   0x26,        NYI,              L_I)
WASM_MEM_OPCODE(I64LoadMem16U,   0x27,        NYI,              L_I)
WASM_MEM_OPCODE(I64LoadMem32S,   0x28,        NYI,              L_I)
WASM_MEM_OPCODE(I64LoadMem32U,   0x29,        NYI,              L_I)
WASM_MEM_OPCODE(I32LoadMem,      0x2a,        LOAD_I32,         I_I)
WASM_MEM_OPCODE(I64LoadMem,      0x2b,        NYI,              L_I)
WASM_MEM_OPCODE(F32LoadMem,      0x2c,        LOAD_F32,         F_I)
WASM_MEM_OPCODE(F64LoadMem,      0x2d,        LOAD_F64,         D_I)

// Store memory expressions.
WASM_MEM_OPCODE(I32StoreMem8,    0x2e,        STORE8_I32,       I_II)
WASM_MEM_OPCODE(I32StoreMem16,   0x2f,        STORE16_I32,      I_II)
WASM_MEM_OPCODE(I64StoreMem8,    0x30,        NYI,              L_IL)
WASM_MEM_OPCODE(I64StoreMem16,   0x31,        NYI,              L_IL)
WASM_MEM_OPCODE(I64StoreMem32,   0x32,        NYI,              L_IL)
WASM_MEM_OPCODE(I32StoreMem,     0x33,        STORE_I32,        I_II)
WASM_MEM_OPCODE(I64StoreMem,     0x34,        NYI,              L_IL)
WASM_MEM_OPCODE(F32StoreMem,     0x35,        STORE_F32,        F_IF)
WASM_MEM_OPCODE(F64StoreMem,     0x36,        STORE_F64,        D_ID)

// Load memory
WASM_MEM_OPCODE(MemorySize,      0x3b,        NYI,                  I_V)
WASM_MEM_OPCODE(GrowMemory,      0x39,        NYI,                  I_I)

// Expressions
WASM_SIMPLE_OPCODE(I32Add,              0x40, ADD_I32,          I_II)
WASM_SIMPLE_OPCODE(I32Sub,              0x41, SUB_I32,          I_II)
WASM_SIMPLE_OPCODE(I32Mul,              0x42, MUL_I32,          I_II)
WASM_SIMPLE_OPCODE(I32DivS,             0x43, DIVS_I32,         I_II)
WASM_SIMPLE_OPCODE(I32DivU,             0x44, DIVU_I32,         I_II)
WASM_SIMPLE_OPCODE(I32RemS,             0x45, MODS_I32,         I_II)
WASM_SIMPLE_OPCODE(I32RemU,             0x46, MODU_I32,         I_II)
WASM_SIMPLE_OPCODE(I32And,              0x47, AND_I32,          I_II)
WASM_SIMPLE_OPCODE(I32Ior,              0x48, OR_I32,           I_II)
WASM_SIMPLE_OPCODE(I32Xor,              0x49, XOR_I32,          I_II)
WASM_SIMPLE_OPCODE(I32Shl,              0x4a, SHL_I32,          I_II)
WASM_SIMPLE_OPCODE(I32ShrU,             0x4b, SHRU_I32,         I_II)
WASM_SIMPLE_OPCODE(I32ShrS,             0x4c, SHRS_I32,         I_II)
WASM_SIMPLE_OPCODE(I32Eq,               0x4d, EQ_I32,           I_II)
WASM_SIMPLE_OPCODE(I32Ne,               0x4e, NEQ_I32,          I_II)
WASM_SIMPLE_OPCODE(I32LtS,              0x4f, LTS_I32,          I_II)
WASM_SIMPLE_OPCODE(I32LeS,              0x50, LES_I32,          I_II)
WASM_SIMPLE_OPCODE(I32LtU,              0x51, LTU_I32,          I_II)
WASM_SIMPLE_OPCODE(I32LeU,              0x52, LEU_I32,          I_II)
WASM_SIMPLE_OPCODE(I32GtS,              0x53, GTS_I32,          I_II)
WASM_SIMPLE_OPCODE(I32GeS,              0x54, GES_I32,          I_II)
WASM_SIMPLE_OPCODE(I32GtU,              0x55, GTU_I32,          I_II)
WASM_SIMPLE_OPCODE(I32GeU,              0x56, GEU_I32,          I_II)
WASM_SIMPLE_OPCODE(I32Clz,              0x57, CLZ_I32,          I_I)
WASM_SIMPLE_OPCODE(I32Ctz,              0x58, CTZ_I32,          I_I)
WASM_SIMPLE_OPCODE(I32Popcnt,           0x59, NYI,              I_I)
WASM_SIMPLE_OPCODE(I32Eqz,              0x5a, EQZ_I32,          I_I)
WASM_SIMPLE_OPCODE(I64Add,              0x5b, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64Sub,              0x5c, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64Mul,              0x5d, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64DivS,             0x5e, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64DivU,             0x5f, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64RemS,             0x60, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64RemU,             0x61, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64And,              0x62, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64Ior,              0x63, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64Xor,              0x64, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64Shl,              0x65, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64ShrU,             0x66, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64ShrS,             0x67, NYI,              L_LL)
WASM_SIMPLE_OPCODE(I64Eq,               0x68, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64Ne,               0x69, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64LtS,              0x6a, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64LeS,              0x6b, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64LtU,              0x6c, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64LeU,              0x6d, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64GtS,              0x6e, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64GeS,              0x6f, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64GtU,              0x70, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64GeU,              0x71, NYI,              I_LL)
WASM_SIMPLE_OPCODE(I64Clz,              0x72, NYI,              L_L)
WASM_SIMPLE_OPCODE(I64Ctz,              0x73, NYI,              L_L)
WASM_SIMPLE_OPCODE(I64Popcnt,           0x74, NYI,              L_L)
WASM_SIMPLE_OPCODE(F32Add,              0x75, ADD_F32,          F_FF)
WASM_SIMPLE_OPCODE(F32Sub,              0x76, SUB_F32,          F_FF)
WASM_SIMPLE_OPCODE(F32Mul,              0x77, MUL_F32,          F_FF)
WASM_SIMPLE_OPCODE(F32DIv,              0x78, DIV_F32,          F_FF)
WASM_SIMPLE_OPCODE(F32Min,              0x79, NYI,              F_FF)
WASM_SIMPLE_OPCODE(F32Max,              0x7a, NYI,              F_FF)
WASM_SIMPLE_OPCODE(F32Abs,              0x7b, ABS_F32,          F_F)
WASM_SIMPLE_OPCODE(F32Neg,              0x7c, NEG_F32,          F_F)
WASM_SIMPLE_OPCODE(F32CopySign,         0x7d, COPYSIGN_F32,     F_FF)
WASM_SIMPLE_OPCODE(F32Ceil,             0x7e, CEIL_F32,         F_F)
WASM_SIMPLE_OPCODE(F32Floor,            0x7f, FLOOR_F32,        F_F)
WASM_SIMPLE_OPCODE(F32Trunc,            0x80, TRUNC_F32,        F_F)
WASM_SIMPLE_OPCODE(F32NearestInt,       0x81, NEAREST_F32,      F_F)
WASM_SIMPLE_OPCODE(F32Sqrt,             0x82, SQRT_F32,         F_F)
WASM_SIMPLE_OPCODE(F32Eq,               0x83, EQ_F32,           I_FF)
WASM_SIMPLE_OPCODE(F32Ne,               0x84, NEQ_F32,          I_FF)
WASM_SIMPLE_OPCODE(F32Lt,               0x85, LT_F32,           I_FF)
WASM_SIMPLE_OPCODE(F32Le,               0x86, LE_F32,           I_FF)
WASM_SIMPLE_OPCODE(F32Gt,               0x87, GT_F32,           I_FF)
WASM_SIMPLE_OPCODE(F32Ge,               0x88, GE_F32,           I_FF)
WASM_SIMPLE_OPCODE(F64Add,              0x89, ADD_F64,          D_DD)
WASM_SIMPLE_OPCODE(F64Sub,              0x8a, SUB_F64,          D_DD)
WASM_SIMPLE_OPCODE(F64Mul,              0x8b, MUL_F64,          D_DD)
WASM_SIMPLE_OPCODE(F64Div,              0x8c, DIV_F64,          D_DD)
WASM_SIMPLE_OPCODE(F64Min,              0x8d, MIN_F64,          D_DD)
WASM_SIMPLE_OPCODE(F64Max,              0x8e, MAX_F64,          D_DD)
WASM_SIMPLE_OPCODE(F64Abs,              0x8f, ABS_F64,          D_D)
WASM_SIMPLE_OPCODE(F64Neg,              0x90, NEG_F64,          D_D)
WASM_SIMPLE_OPCODE(F64CopySign,         0x91, NYI,              D_DD) //COPYSIGN_F64
WASM_SIMPLE_OPCODE(F64Ceil,             0x92, CEIL_F64,         D_D)
WASM_SIMPLE_OPCODE(F64Floor,            0x93, FLOOR_F64,        D_D)
WASM_SIMPLE_OPCODE(F64Trunc,            0x94, TRUNC_F64,        D_D)
WASM_SIMPLE_OPCODE(F64NearestInt,       0x95, NEAREST_F64,      D_D)
WASM_SIMPLE_OPCODE(F64Sqrt,             0x96, SQRT_F64,         D_D)
WASM_SIMPLE_OPCODE(F64Eq,               0x97, EQ_F64,           I_DD)
WASM_SIMPLE_OPCODE(F64Ne,               0x98, NEQ_F64,          I_DD)
WASM_SIMPLE_OPCODE(F64Lt,               0x99, LT_F64,           I_DD)
WASM_SIMPLE_OPCODE(F64Le,               0x9a, LE_F64,           I_DD)
WASM_SIMPLE_OPCODE(F64Gt,               0x9b, GT_F64,           I_DD)
WASM_SIMPLE_OPCODE(F64Ge,               0x9c, GE_F64,           I_DD)
WASM_SIMPLE_OPCODE(I32SConvertF32,      0x9d, TRUNC_S_F32_I32,  I_F)
WASM_SIMPLE_OPCODE(I32SConvertF64,      0x9e, TRUNC_S_F64_I32,  I_D)
WASM_SIMPLE_OPCODE(I32UConvertF32,      0x9f, NYI,              I_F) //TRUNC_U_F32_I32
WASM_SIMPLE_OPCODE(I32UConvertF64,      0xa0, NYI,              I_D) //TRUNC_U_F64_I32
WASM_SIMPLE_OPCODE(I32ConvertI64,       0xa1, NYI,              I_L)
WASM_SIMPLE_OPCODE(I64SConvertF32,      0xa2, NYI,              L_F)
WASM_SIMPLE_OPCODE(I64SConvertF64,      0xa3, NYI,              L_D)
WASM_SIMPLE_OPCODE(I64UConvertF32,      0xa4, NYI,              L_F)
WASM_SIMPLE_OPCODE(I64UConvertF64,      0xa5, NYI,              L_D)
WASM_SIMPLE_OPCODE(I64SConvertI32,      0xa6, NYI,              L_I)
WASM_SIMPLE_OPCODE(I64UConvertI32,      0xa7, NYI,              L_I)
WASM_SIMPLE_OPCODE(F32SConvertI32,      0xa8, CONVERT_S_I32_F32,F_I)
WASM_SIMPLE_OPCODE(F32UConvertI32,      0xa9, NYI,              F_I) //CONVERT_U_I32_F32
WASM_SIMPLE_OPCODE(F32SConvertI64,      0xaa, NYI,              F_L)
WASM_SIMPLE_OPCODE(F32UConvertI64,      0xab, NYI,              F_L)
WASM_SIMPLE_OPCODE(F32ConvertF64,       0xac, DEMOTE_F64_F32,   F_D)
WASM_SIMPLE_OPCODE(F32ReinterpretI32,   0xad, NYI,              F_I) //REINTERPRET_I32_F32
WASM_SIMPLE_OPCODE(F64SConvertI32,      0xae, CONVERT_S_I32_F64,D_I)
WASM_SIMPLE_OPCODE(F64UConvertI32,      0xaf, CONVERT_U_I32_F64,D_I)
WASM_SIMPLE_OPCODE(F64SConvertI64,      0xb0, NYI,              D_L)
WASM_SIMPLE_OPCODE(F64UConvertI64,      0xb1, NYI,              D_L)
WASM_SIMPLE_OPCODE(F64ConvertF32,       0xb2, PROMOTE_F32_F64,  D_F)
WASM_SIMPLE_OPCODE(F64ReinterpretI64,   0xb3, NYI,              D_L)
WASM_SIMPLE_OPCODE(I32ReinterpretF32,   0xb4, REINTERPRET_F32_I32, I_F)
WASM_SIMPLE_OPCODE(I64ReinterpretF64,   0xb5, NYI,              L_D)
WASM_SIMPLE_OPCODE(I32Ror,              0xb6, ROR_I32,          I_II)
WASM_SIMPLE_OPCODE(I32Rol,              0xb7, ROL_I32,          I_II)


#undef WASM_SIMPLE_OPCODE
#undef WASM_MEM_OPCODE
#undef WASM_MISC_OPCODE
#undef WASM_CTRL_OPCODE
#undef WASM_OPCODE
#undef WASM_SIGNATURE
