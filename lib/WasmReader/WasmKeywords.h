//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#ifndef WASM_KEYWORD
#define WASM_KEYWORD(token, name)
#endif

#ifndef WASM_KEYWORD_BIN
#define WASM_KEYWORD_BIN(token, name) WASM_KEYWORD(token, name)
#endif

#ifndef WASM_KEYWORD_COMPARE
#define WASM_KEYWORD_COMPARE(token, name, floatOp, doubleOp, intOp) WASM_KEYWORD_BIN(token, name)
#endif

#ifndef WASM_KEYWORD_BIN_MATH
#define WASM_KEYWORD_BIN_MATH(token, name, floatOp, doubleOp, intOp) WASM_KEYWORD_BIN(token, name)
#endif

#ifndef WASM_KEYWORD_UNARY
#define WASM_KEYWORD_UNARY(token, name) WASM_KEYWORD(token, name)
#endif

#ifndef WASM_MEMTYPE
#define WASM_MEMTYPE(token, name) WASM_KEYWORD(token, name)
#endif

#ifndef WASM_LOCALTYPE
#define WASM_LOCALTYPE(token, name) WASM_MEMTYPE(token, name)
#endif

// memory
WASM_KEYWORD(GET_NEAR_UNALIGNED_S,   getnearunaligneds)
WASM_KEYWORD(GET_FAR_UNALIGNED_S,    getfarunaligneds)
WASM_KEYWORD(GET_NEAR_S,             getnears)
WASM_KEYWORD(GET_FAR_S,              getfars)
WASM_KEYWORD(SET_NEAR_UNALIGNED_S,   setnearunaligneds)
WASM_KEYWORD(SET_FAR_UNALIGNED_S,    setfarunaligneds)
WASM_KEYWORD(SET_NEAR_S,             setnears)
WASM_KEYWORD(SET_FAR_S,              setfars)

WASM_KEYWORD(GET_NEAR_UNALIGNED_U,   getnearunalignedu)
WASM_KEYWORD(GET_FAR_UNALIGNED_U,    getfarunalignedu)
WASM_KEYWORD(GET_NEAR_U,             getnearu)
WASM_KEYWORD(GET_FAR_U,              getfaru)
WASM_KEYWORD(SET_NEAR_UNALIGNED_U,   setnearunalignedu)
WASM_KEYWORD(SET_FAR_UNALIGNED_U,    setfarunalignedu)
WASM_KEYWORD(SET_NEAR_U,             setnearu)
WASM_KEYWORD(SET_FAR_U,              setfaru)

WASM_KEYWORD(GETPARAM,               getparam)
WASM_KEYWORD(GETLOCAL,               getlocal)
WASM_KEYWORD(SETLOCAL,               setlocal)
WASM_KEYWORD(GETGLOBAL,              getglobal)
WASM_KEYWORD(SETGLOBAL,              setglobal)

// types
WASM_MEMTYPE(I8,     i8)
WASM_MEMTYPE(I16,    i16)

WASM_LOCALTYPE(I32,    i32)
WASM_LOCALTYPE(I64,    i64)
WASM_LOCALTYPE(F32,    f32)
WASM_LOCALTYPE(F64,    f64)

// control flow ops
WASM_KEYWORD(NOP,        nop)
WASM_KEYWORD(BLOCK,      block)
WASM_KEYWORD(IF,         if)
WASM_KEYWORD(LOOP,       loop)
WASM_KEYWORD(LABEL,      label)
WASM_KEYWORD(BREAK,      break)
WASM_KEYWORD(SWITCH,     switch)
WASM_KEYWORD(CALL,       call)
WASM_KEYWORD(DISPATCH,   dispatch)
WASM_KEYWORD(RETURN,     return)
WASM_KEYWORD(DESTRUCT,   destruct)
WASM_KEYWORD(CONST,      const)

// structures
WASM_KEYWORD(FUNC,       func)
WASM_KEYWORD(PARAM,      param)
WASM_KEYWORD(RESULT,     result)
WASM_KEYWORD(LOCAL,      local)
WASM_KEYWORD(MODULE,     module)
WASM_KEYWORD(GLOBAL,     global)
WASM_KEYWORD(EXPORT,     export)
WASM_KEYWORD(TABLE,      table)
WASM_KEYWORD(MEMORY,     memory)
WASM_KEYWORD(DATA,       data)

// unary ops
WASM_KEYWORD_UNARY(NOT,    not)
WASM_KEYWORD_UNARY(CLZ,    clz)
WASM_KEYWORD_UNARY(CTZ,    ctz)
WASM_KEYWORD_UNARY(NEG,    neg)
WASM_KEYWORD_UNARY(ABS,    abs)
WASM_KEYWORD_UNARY(CEIL,   ceil)
WASM_KEYWORD_UNARY(FLOOR,  floor)
WASM_KEYWORD_UNARY(TRUNC,  trunc)
WASM_KEYWORD_UNARY(ROUND,  round)

// binary ops
WASM_KEYWORD_BIN_MATH(ADD,        add,       Add_Flt, Add_Db, Add_Int)
WASM_KEYWORD_BIN_MATH(SUB, sub, Sub_Flt, Sub_Db, Sub_Int)
WASM_KEYWORD_BIN_MATH(MUL, mul, Mul_Flt, Mul_Db, Mul_Int)
WASM_KEYWORD_BIN_MATH(DIVS, divs, Nop, Nop, Div_Int)
WASM_KEYWORD_BIN_MATH(DIVU, divu, Nop, Nop, Div_Int)
WASM_KEYWORD_BIN_MATH(MODS, mods, Nop, Nop, Rem_Int)
WASM_KEYWORD_BIN_MATH(MODU, modu, Nop, Nop, Rem_Int)
WASM_KEYWORD_BIN_MATH(AND, and, Nop, Nop, And_Int)
WASM_KEYWORD_BIN_MATH(OR, or, Nop, Nop, Or_Int)
WASM_KEYWORD_BIN_MATH(XOR, xor, Nop, Nop, Xor_Int)
WASM_KEYWORD_BIN_MATH(SHL, shl, Nop, Nop, Shl_Int)
WASM_KEYWORD_BIN_MATH(SHR, shr, Nop, Nop, Shr_Int)
WASM_KEYWORD_BIN_MATH(SAR, sar, Nop, Nop, Nop)
WASM_KEYWORD_BIN_MATH(DIV, div, Nop, Nop, Nop)
WASM_KEYWORD_BIN_MATH(MOD, mod, Nop, Nop, Nop)
WASM_KEYWORD_BIN_MATH(COPYSIGN, copysign, Nop, Nop, Nop)

// compare ops
WASM_KEYWORD_COMPARE(EQ,     eq, CmEq_Flt, CmEq_Db, CmEq_Int)
WASM_KEYWORD_COMPARE(NEQ,    neq, CmNe_Db, CmNe_Db, CmNe_Int)
WASM_KEYWORD_COMPARE(LTS,    lts, Nop, Nop, CmLt_Int)
WASM_KEYWORD_COMPARE(LTU,    ltu, Nop, Nop, CmLt_UnInt)
WASM_KEYWORD_COMPARE(LES,    les, Nop, Nop, CmLe_Int)
WASM_KEYWORD_COMPARE(LEU,    leu, Nop, Nop, CmLe_UnInt)
WASM_KEYWORD_COMPARE(GTS,    gts, Nop, Nop, CmGt_Int)
WASM_KEYWORD_COMPARE(GTU,    gtu, Nop, Nop, CmGt_UnInt)
WASM_KEYWORD_COMPARE(GES,    ges, Nop, Nop, CmGe_Int)
WASM_KEYWORD_COMPARE(GEU,    geu, Nop, Nop, CmGe_UnInt)
WASM_KEYWORD_COMPARE(LT,     lt, CmLt_Flt, CmLt_Db, Nop)
WASM_KEYWORD_COMPARE(LE,     le, CmLe_Flt, CmLe_Db, Nop)
WASM_KEYWORD_COMPARE(GT,     gt, CmGt_Flt, CmGt_Db, Nop)
WASM_KEYWORD_COMPARE(GE,     ge, CmGe_Flt, CmGe_Db, Nop)

// conversions
WASM_KEYWORD(CONVERTS,   converts)
WASM_KEYWORD(CONVERTU,   convertu)
WASM_KEYWORD(CONVERT,    convert)
WASM_KEYWORD(CAST,       cast)

// process actions
WASM_KEYWORD(INVOKE,     invoke)
WASM_KEYWORD(ASSERTEQ,   asserteq)

#undef WASM_KEYWORD_COMPARE
#undef WASM_KEYWORD_UNARY
#undef WASM_KEYWORD_BIN_MATH
#undef WASM_KEYWORD_BIN
#undef WASM_LOCALTYPE
#undef WASM_MEMTYPE
#undef WASM_KEYWORD
