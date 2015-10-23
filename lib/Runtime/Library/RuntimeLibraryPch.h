//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#ifndef IsJsDiag
#include "Parser.h"
#include "RegexCommon.h"
#include "Runtime.h"

#include "Library\EtwTrace.h"

#include "Library\JavascriptNumberObject.h"
#include "Library\LiteralStringObject.h"
#include "Library\JavascriptBooleanObject.h"

#include "Library\ObjectPrototypeObject.h"

#include "common\ByteSwap.h"
#include "Library\DataView.h"

#include "Library\JSONString.h"
#include "Library\ProfileString.h"
#include "Library\SingleCharString.h"
#include "Library\SubString.h"

#include "Library\BoundFunction.h"

#include "Library\RegexHelper.h"
#include "Library\JavascriptRegularExpression.h"
#include "Library\JavascriptRegExpConstructor.h"
#include "Library\JavascriptRegularExpressionResult.h"

#include "Library\JavascriptVariantDate.h"
#include "Library\JavascriptPromise.h"
#include "Library\JavascriptSymbol.h"
#include "Library\JavascriptSymbolObject.h"
#include "Library\JavascriptProxy.h"
#include "Library\JavascriptReflect.h"
#include "Library\JavascriptGenerator.h"

#include "Library\SameValueComparer.h"
#include "Library\MapOrSetDataList.h"
#include "Library\JavascriptMap.h"
#include "Library\JavascriptSet.h"
#include "Library\JavascriptWeakMap.h"
#include "Library\JavascriptWeakSet.h"

#include "Library\ArgumentsObjectEnumerator.h"
#include "Library\JavascriptArrayEnumeratorBase.h"
#include "Library\JavascriptArrayEnumerator.h"
#include "Library\JavascriptArraySnapshotEnumerator.h"
#include "Library\JavascriptArrayNonIndexSnapshotEnumerator.h"
#include "Library\JavascriptArrayIndexEnumerator.h"
#include "Library\ES5ArrayEnumerator.h"
#include "Library\ES5ArrayNonIndexEnumerator.h"
#include "Library\ES5ArrayIndexEnumerator.h"
#include "Library\TypedArrayEnumerator.h"
#include "Library\JavascriptStringEnumerator.h"
#include "Library\JavascriptRegExpEnumerator.h"
#include "Library\IteratorObjectEnumerator.h"


#include "Library\JavascriptIterator.h"
#include "Library\JavascriptArrayIterator.h"
#include "Library\JavascriptMapIterator.h"
#include "Library\JavascriptSetIterator.h"
#include "Library\JavascriptStringIterator.h"
#include "Library\JavascriptEnumeratorIterator.h"

#include "Library\UriHelper.h"
#include "Library\HostObjectBase.h"

#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"

// SIMD_JS
// SIMD types
#include "Library\JavascriptSIMDFloat32x4.h"
#include "Library\JavascriptSIMDFloat64x2.h"
#include "Library\JavascriptSIMDInt32x4.h"
#include "Library\JavascriptSIMDInt16x8.h"
#include "Library\JavascriptSIMDInt8x16.h"
#include "Library\JavascriptSIMDUInt32x4.h"
#include "Library\JavascriptSIMDUint16x8.h"
#include "Library\JavascriptSIMDUint8x16.h"
#include "Library\JavascriptSIMDBool32x4.h"
#include "Library\JavascriptSIMDBool16x8.h"
#include "Library\JavascriptSIMDBool8x16.h"
// SIMD operations
#include "Library\SIMDFloat32x4Operation.h"
#include "Library\SIMDFloat64x2Operation.h"
#include "Library\SIMDInt32x4Operation.h"
#include "Library\SIMDInt16x8Operation.h"
#include "Library\SIMDInt8x16Operation.h"
#include "Library\SIMDUInt32x4Operation.h"
#include "Library\SIMDUint16x8Operation.h"
#include "Library\SIMDUint8x16Operation.h"
#include "Library\SIMDBool32x4Operation.h"
#include "Library\SIMDBool16x8Operation.h"
#include "Library\SIMDBool8x16Operation.h"
// SIMD libs
#include "Library\SIMDFloat32x4Lib.h"
#include "Library\SIMDFloat64x2Lib.h"
#include "Library\SIMDInt32x4Lib.h"
#include "Library\SIMDInt16x8Lib.h"
#include "Library\SIMDInt8x16Lib.h"
#include "Library\SIMDUInt32x4Lib.h"
#include "Library\SIMDUint16x8Lib.h"
#include "Library\SIMDUint8x16Lib.h"
#include "Library\SIMDBool32x4Lib.h"
#include "Library\SIMDBool16x8Lib.h"
#include "Library\SIMDBool8x16Lib.h"


#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
#include "Library\ScriptMemoryDumper.h"
#endif

#include "Language\JavascriptStackWalker.h"

#ifdef ENABLE_BASIC_TELEMETRY
#include "..\..\..\private\lib\Telemetry\ScriptContextTelemetry.h"
#endif
#endif // !IsJsDiag
