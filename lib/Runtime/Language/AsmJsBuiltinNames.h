//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
// Default all macros to nothing
#ifndef ASMJS_MATH_FUNC_NAMES
#define ASMJS_MATH_FUNC_NAMES(name, propertyName)
#endif

#ifndef ASMJS_MATH_CONST_NAMES
#define ASMJS_MATH_CONST_NAMES(name, propertyName)
#endif

#ifndef ASMJS_ARRAY_NAMES
#define ASMJS_ARRAY_NAMES(name, propertyName)
#endif

#ifndef ASMJS_SIMD_NAMES
#define ASMJS_SIMD_NAMES(name, propertyName)
#endif

ASMJS_MATH_FUNC_NAMES(sin,      sin)
ASMJS_MATH_FUNC_NAMES(cos,      cos)
ASMJS_MATH_FUNC_NAMES(tan,      tan)
ASMJS_MATH_FUNC_NAMES(asin,     asin)
ASMJS_MATH_FUNC_NAMES(acos,     acos)
ASMJS_MATH_FUNC_NAMES(atan,     atan)
ASMJS_MATH_FUNC_NAMES(ceil,     ceil)
ASMJS_MATH_FUNC_NAMES(floor,    floor)
ASMJS_MATH_FUNC_NAMES(exp,      exp)
ASMJS_MATH_FUNC_NAMES(log,      log)
ASMJS_MATH_FUNC_NAMES(pow,      pow)
ASMJS_MATH_FUNC_NAMES(sqrt,     sqrt)
ASMJS_MATH_FUNC_NAMES(abs,      abs)
ASMJS_MATH_FUNC_NAMES(atan2,    atan2)
ASMJS_MATH_FUNC_NAMES(imul,     imul)
ASMJS_MATH_FUNC_NAMES(fround,   fround)
ASMJS_MATH_FUNC_NAMES(min,      min)
ASMJS_MATH_FUNC_NAMES(max,      max)
ASMJS_MATH_FUNC_NAMES(clz32,    clz32)

ASMJS_MATH_CONST_NAMES(e,           E)
ASMJS_MATH_CONST_NAMES(ln10,        LN10)
ASMJS_MATH_CONST_NAMES(ln2,         LN2)
ASMJS_MATH_CONST_NAMES(log2e,       LOG2E)
ASMJS_MATH_CONST_NAMES(log10e,      LOG10E)
ASMJS_MATH_CONST_NAMES(pi,          PI)
ASMJS_MATH_CONST_NAMES(sqrt1_2,     SQRT1_2)
ASMJS_MATH_CONST_NAMES(sqrt2,       SQRT2)
ASMJS_MATH_CONST_NAMES(infinity,    Infinity)
ASMJS_MATH_CONST_NAMES(nan,         NaN)

ASMJS_ARRAY_NAMES(Uint8Array,   Uint8Array)
ASMJS_ARRAY_NAMES(Int8Array,    Int8Array)
ASMJS_ARRAY_NAMES(Uint16Array,  Uint16Array)
ASMJS_ARRAY_NAMES(Int16Array,   Int16Array)
ASMJS_ARRAY_NAMES(Uint32Array,  Uint32Array)
ASMJS_ARRAY_NAMES(Int32Array,   Int32Array)
ASMJS_ARRAY_NAMES(Float32Array, Float32Array)
ASMJS_ARRAY_NAMES(Float64Array, Float64Array)
ASMJS_ARRAY_NAMES(byteLength,   byteLength)

ASMJS_SIMD_NAMES(Int32x4,                                   Int32x4)
ASMJS_SIMD_NAMES(int32x4_check,                             check)
ASMJS_SIMD_NAMES(int32x4_splat,                             splat)
ASMJS_SIMD_NAMES(int32x4_fromFloat64x2,                     fromFloat64x2)
ASMJS_SIMD_NAMES(int32x4_fromFloat64x2Bits,                 fromFloat64x2Bits)
ASMJS_SIMD_NAMES(int32x4_fromFloat32x4,                     fromFloat32x4)
ASMJS_SIMD_NAMES(int32x4_fromFloat32x4Bits,                 fromFloat32x4Bits)
ASMJS_SIMD_NAMES(int32x4_neg,                               neg)
ASMJS_SIMD_NAMES(int32x4_add,                               add)
ASMJS_SIMD_NAMES(int32x4_sub,                               sub)
ASMJS_SIMD_NAMES(int32x4_mul,                               mul)
ASMJS_SIMD_NAMES(int32x4_extractLane,                       extractLane)
ASMJS_SIMD_NAMES(int32x4_replaceLane,                       replaceLane)
ASMJS_SIMD_NAMES(int32x4_swizzle,                           swizzle)
ASMJS_SIMD_NAMES(int32x4_shuffle,                           shuffle)
ASMJS_SIMD_NAMES(int32x4_lessThan,                          lessThan)
ASMJS_SIMD_NAMES(int32x4_equal,                             equal)
ASMJS_SIMD_NAMES(int32x4_greaterThan,                       greaterThan)
ASMJS_SIMD_NAMES(int32x4_select,                            select)
ASMJS_SIMD_NAMES(int32x4_and,                               and)
ASMJS_SIMD_NAMES(int32x4_or,                                or)
ASMJS_SIMD_NAMES(int32x4_xor,                               xor)
ASMJS_SIMD_NAMES(int32x4_not,                               not)
// ToDo: Enable after fix in lib
//ASMJS_SIMD_NAMES(int32x4_shiftLeftByScalar,                 shiftLeftByScalar)
//ASMJS_SIMD_NAMES(int32x4_shiftRightLogicalByScalar,         shiftRightLogicalByScalar)
//ASMJS_SIMD_NAMES(int32x4_shiftRightArithmeticByScalar,      shiftRightArithmeticByScalar)
ASMJS_SIMD_NAMES(int32x4_load,                              load)
ASMJS_SIMD_NAMES(int32x4_load1,                             load1)
ASMJS_SIMD_NAMES(int32x4_load2,                             load2)
ASMJS_SIMD_NAMES(int32x4_load3,                             load3)
ASMJS_SIMD_NAMES(int32x4_store,                             store)
ASMJS_SIMD_NAMES(int32x4_store1,                            store1)
ASMJS_SIMD_NAMES(int32x4_store2,                            store2)
ASMJS_SIMD_NAMES(int32x4_store3,                            store3)

ASMJS_SIMD_NAMES(Float32x4,                                 Float32x4)
ASMJS_SIMD_NAMES(float32x4_check,                           check)
ASMJS_SIMD_NAMES(float32x4_splat,                           splat)
ASMJS_SIMD_NAMES(float32x4_fromFloat64x2,                   fromFloat64x2)
ASMJS_SIMD_NAMES(float32x4_fromFloat64x2Bits,               fromFloat64x2Bits)
ASMJS_SIMD_NAMES(float32x4_fromInt32x4,                     fromInt32x4)
ASMJS_SIMD_NAMES(float32x4_fromInt32x4Bits,                 fromInt32x4Bits)
ASMJS_SIMD_NAMES(float32x4_abs,                             abs)
ASMJS_SIMD_NAMES(float32x4_neg,                             neg)
ASMJS_SIMD_NAMES(float32x4_add,                             add)
ASMJS_SIMD_NAMES(float32x4_sub,                             sub)
ASMJS_SIMD_NAMES(float32x4_mul,                             mul)
ASMJS_SIMD_NAMES(float32x4_div,                             div)
ASMJS_SIMD_NAMES(float32x4_clamp,                           clamp)
ASMJS_SIMD_NAMES(float32x4_min,                             min)
ASMJS_SIMD_NAMES(float32x4_max,                             max)
ASMJS_SIMD_NAMES(float32x4_reciprocal,                      reciprocalApproximation)
ASMJS_SIMD_NAMES(float32x4_reciprocalSqrt,                  reciprocalSqrtApproximation)
ASMJS_SIMD_NAMES(float32x4_sqrt,                            sqrt)
ASMJS_SIMD_NAMES(float32x4_swizzle,                         swizzle)
ASMJS_SIMD_NAMES(float32x4_shuffle,                         shuffle)
ASMJS_SIMD_NAMES(float32x4_extractLane,                     extractLane)
ASMJS_SIMD_NAMES(float32x4_replaceLane,                     replaceLane)
ASMJS_SIMD_NAMES(float32x4_lessThan,                        lessThan)
ASMJS_SIMD_NAMES(float32x4_lessThanOrEqual,                 lessThanOrEqual)
ASMJS_SIMD_NAMES(float32x4_equal,                           equal)
ASMJS_SIMD_NAMES(float32x4_notEqual,                        notEqual)
ASMJS_SIMD_NAMES(float32x4_greaterThan,                     greaterThan)
ASMJS_SIMD_NAMES(float32x4_greaterThanOrEqual,              greaterThanOrEqual)
ASMJS_SIMD_NAMES(float32x4_select,                          select)
ASMJS_SIMD_NAMES(float32x4_and,                             and)
ASMJS_SIMD_NAMES(float32x4_or,                              or)
ASMJS_SIMD_NAMES(float32x4_xor,                             xor)
ASMJS_SIMD_NAMES(float32x4_not,                             not)
ASMJS_SIMD_NAMES(float32x4_load,                            load)
ASMJS_SIMD_NAMES(float32x4_load1,                           load1)
ASMJS_SIMD_NAMES(float32x4_load2,                           load2)
ASMJS_SIMD_NAMES(float32x4_load3,                           load3)
ASMJS_SIMD_NAMES(float32x4_store,                           store)
ASMJS_SIMD_NAMES(float32x4_store1,                          store1)
ASMJS_SIMD_NAMES(float32x4_store2,                          store2)
ASMJS_SIMD_NAMES(float32x4_store3,                          store3)

ASMJS_SIMD_NAMES(Float64x2,                                 Float64x2)
ASMJS_SIMD_NAMES(float64x2_check,                           check)
ASMJS_SIMD_NAMES(float64x2_splat,                           splat)
ASMJS_SIMD_NAMES(float64x2_fromFloat32x4,                   fromFloat32x4)
ASMJS_SIMD_NAMES(float64x2_fromFloat32x4Bits,               fromFloat32x4Bits)
ASMJS_SIMD_NAMES(float64x2_fromInt32x4,                     fromInt32x4)
ASMJS_SIMD_NAMES(float64x2_fromInt32x4Bits,                 fromInt32x4Bits)
ASMJS_SIMD_NAMES(float64x2_abs,                             abs)
ASMJS_SIMD_NAMES(float64x2_neg,                             neg)
ASMJS_SIMD_NAMES(float64x2_add,                             add)
ASMJS_SIMD_NAMES(float64x2_sub,                             sub)
ASMJS_SIMD_NAMES(float64x2_mul,                             mul)
ASMJS_SIMD_NAMES(float64x2_div,                             div)
ASMJS_SIMD_NAMES(float64x2_clamp,                           clamp)
ASMJS_SIMD_NAMES(float64x2_min,                             min)
ASMJS_SIMD_NAMES(float64x2_max,                             max)
ASMJS_SIMD_NAMES(float64x2_reciprocal,                      reciprocalApproximation)
ASMJS_SIMD_NAMES(float64x2_reciprocalSqrt,                  reciprocalSqrtApproximation)
ASMJS_SIMD_NAMES(float64x2_sqrt,                            sqrt)
ASMJS_SIMD_NAMES(float64x2_swizzle,                         swizzle)
ASMJS_SIMD_NAMES(float64x2_shuffle,                         shuffle)
ASMJS_SIMD_NAMES(float64x2_lessThan,                        lessThan)
ASMJS_SIMD_NAMES(float64x2_lessThanOrEqual,                 lessThanOrEqual)
ASMJS_SIMD_NAMES(float64x2_equal,                           equal)
ASMJS_SIMD_NAMES(float64x2_notEqual,                        notEqual)
ASMJS_SIMD_NAMES(float64x2_greaterThan,                     greaterThan)
ASMJS_SIMD_NAMES(float64x2_greaterThanOrEqual,              greaterThanOrEqual)
ASMJS_SIMD_NAMES(float64x2_select,                          select)
ASMJS_SIMD_NAMES(float64x2_load,                            load)
ASMJS_SIMD_NAMES(float64x2_load1,                           load1)
ASMJS_SIMD_NAMES(float64x2_store,                           store)
ASMJS_SIMD_NAMES(float64x2_store1,                          store1)

// help the caller to undefine all the macros
#undef ASMJS_MATH_FUNC_NAMES
#undef ASMJS_MATH_CONST_NAMES
#undef ASMJS_ARRAY_NAMES
#undef ASMJS_SIMD_NAMES
