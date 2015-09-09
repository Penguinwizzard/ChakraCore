//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "Parser.h"

#include "Runtime.h"
#include "ByteCode\AsmJsByteCodeWriter.h"

#include "Language\ByteCodeSerializer.h"
#include "Language\AsmJsTypes.h"
#include "Language\AsmJsUtils.h"
#include "Language\AsmJsLink.h"
#include "Language\AsmJsByteCodeGenerator.h"
#include "Language\AsmJsModule.h"
#include "Language\AsmJs.h"
#ifdef ASMJS_PLAT
#include "Language\AsmJSJitTemplate.h"
#include "Language\AsmJSEncoder.h"
#include "Language\AsmJSCodeGenerator.h"
#endif
#include "Language\FunctionCodeGenJitTimeData.h"


#include "Language\ProfilingHelpers.h"
#include "Language\CacheOperators.h"

#include "Language\ScriptContextProfiler.h"
#include "Language\JavascriptMathOperators.h"

#ifdef DYNAMIC_PROFILE_STORAGE
#include "Language\DynamicProfileStorage.h"
#endif


#include "ByteCode\AsmJSByteCodeDumper.h"

#include "Library\EtwTrace.h"


#ifdef ENABLE_MUTATION_BREAKPOINT
// REVIEW: ChakraCore Dependency
#include "activdbg_private.h"
#include "Debug\MutationBreakpoint.h"
#endif

#include "Types\TypePropertyCache.h"
#include "Library\JavascriptVariantDate.h"
#include "Library\JavascriptProxy.h"
#include "Library\JavascriptSymbol.h"
#include "Library\JavascriptSymbolObject.h"
#include "Library\JavascriptGenerator.h"
#include "Library\HostObjectBase.h"
#ifdef SIMD_JS_ENABLED
// SIMD libs
#include "Library\SIMDFloat32x4Lib.h"
#include "Library\SIMDFloat64x2Lib.h"
#include "Library\SIMDInt32x4Lib.h"
#include "Library\SIMDInt8x16Lib.h"
// SIMD operations
#include "Library\SIMDFloat32x4Operation.h"
#include "Library\SIMDInt32x4Operation.h"
#include "Library\SIMDInt8x16Operation.h"
#include "Library\SIMDFloat64x2Operation.h"
#endif

#include "Debug\DebuggingFlags.h"
#include "Debug\DiagProbe.h"
#include "Debug\DebugManager.h"
#include "Debug\ProbeContainer.h"
#include "Debug\DebugContext.h"

// .inl files
#include "Language\CacheOperators.inl"
#include "Language\JavascriptMathOperators.inl"
