//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "Banned.h"
#include "Common.h"

//========================
// Parser includes
//========================
#include "ParserCommon.h"
#include "ParseFlags.h"
#include "rterror.h"

// Parser forward decl
class FuncInfo;
class Scope;
class Symbol;
struct Ident;
typedef Ident *IdentPtr;

enum SymbolType : byte;

// Regex forward decl
namespace UnifiedRegex
{
    struct RegexPattern;    
    template <typename T> class StandardChars;      // Used by ThreadContext.h
    struct TrigramAlphabet;
    struct RegexStacks;
#if ENABLE_REGEX_CONFIG_OPTIONS
    class DebugWriter;
    struct RegexStats;
    class RegexStatsDatabase;
#endif
};

//========================

#include "RuntimeCommon.h"

#include <intsafe.h>

#if !defined(UNREFERENCED_PARAMETER)
#define UNREFERENCED_PARAMETER(x) (x)
#endif

//#define BODLOG

class SRCINFO;
class Lowerer;
class LowererMD;
class LowererMDArch;
class ByteCodeGenerator;
interface IActiveScriptDataCache;

namespace Js
{
    //
    // Forward declarations
    //    
    class CharClassifier;
    typedef int32 MessageId;
    /* enum */ struct PropertyIds;
    class DebugDocument;
    struct Utf8SourceInfo;
    struct CallInfo;
    struct InlineeCallInfo;
    struct InlineCache;
    struct PolymorphicInlineCache;
    struct Arguments;
    class StringDictionaryWrapper;
    struct ByteCodeDumper;
    struct ByteCodeReader;
    struct ByteCodeWriter;
    class JavascriptConversion;
    class JavascriptDate;
    class JavascriptVariantDate;
    class DateImplementation;
    class BufferString;
    class BufferStringBuilder;
    class ConcatString;    
    class JavascriptBoolean;
    class JavascriptBooleanObject;
    class JavascriptSymbol;
    class JavascriptSymbolObject;
    class JavascriptProxy;
    class JavascriptReflect;
    class JavascriptEnumeratorIterator;
    class JavascriptArrayIterator;
    enum class JavascriptArrayIteratorKind;
    class JavascriptMapIterator;
    enum class JavascriptMapIteratorKind;
    class JavascriptSetIterator;
    enum class JavascriptSetIteratorKind;
    class JavascriptStringIterator;
    class JavascriptPromise;
    class JavascriptPromiseCapability;
    class JavascriptPromiseReaction;
    class JavascriptPromiseAsyncSpawnExecutorFunction;
    class JavascriptPromiseAsyncSpawnStepArgumentExecutorFunction;
    class JavascriptPromiseCapabilitiesExecutorFunction;
    class JavascriptPromiseResolveOrRejectFunction;
    class JavascriptPromiseReactionTaskFunction;
    class JavascriptPromiseResolveThenableTaskFunction;
    class JavascriptPromiseAllResolveElementFunction;
    struct JavascriptPromiseAllResolveElementFunctionRemainingElementsWrapper;
    class JavascriptGenerator;
    class LiteralString;
    class ArenaLiteralString;
    class JavascriptStringObject;
    struct PropertyDescriptor;
    class Type;
    class DynamicType;
    class ScriptFunctionType;
    class DynamicTypeHandler;
    class DeferredTypeHandlerBase;
    template <bool IsPrototype> class NullTypeHandler;
    template<size_t size> class SimpleTypeHandler;
    class PathTypeHandler;
    class IndexPropertyDescriptor;
    class DynamicObject;
    class ArrayObject;
    class WithScopeObject;
    class SpreadArgument;
    class JavascriptString;
    class StringCopyInfo;
    class StringCopyInfoStack;
    class ObjectPrototypeObject;
    class PropertyString;
    class ArgumentsObject;
    class HeapArgumentsObject;
    class ActivationObject;
    class JavascriptNumber;
    class JavascriptNumberObject;

    class ES5ArgumentsObjectEnumerator;
    class ScriptContextProfiler;

#ifdef SIMD_JS_ENABLED
    // SIMD
    class SIMDFloat32x4Lib;
    class JavascriptSIMDFloat32x4;
    class SIMDFloat64x2Lib;
    class JavascriptSIMDFloat64x2;
    class SIMDInt32x4Lib;
    class JavascriptSIMDInt32x4;
    class SIMDInt8x16Lib;
    class JavascriptSIMDInt8x16;    
#endif

    class RecyclableObject;
    class JavascriptRegExp;
    class JavascriptRegularExpressionResult;
    template<typename T> class SparseArraySegment;
    enum class DynamicObjectFlags : uint16;
    class JavascriptArray;
    class JavascriptNativeIntArray;
    class JavascriptCopyOnAccessNativeIntArray;
    class JavascriptNativeFloatArray;
    class ES5Array;
    class JavascriptFunction;
    class ScriptFunction;
    class StackScriptFunction;
    class GeneratorVirtualScriptFunction;
    class JavascriptRegExpConstructor;
    class JavascriptRegExpEnumerator;
    class BoundFunction;
    class JavascriptMap;
    class JavascriptSet;
    class JavascriptWeakMap;
    class JavascriptWeakSet;
    class DynamicObject;
    class HostObjectBase;
    class RootObjectBase;
    class ModuleRoot;
    class GlobalObject;    
    class Math;
    class JavascriptOperators;
    class JavascriptLibrary;    
    class JavascriptEncodeURI;
    class JavascriptEncodeURIComponent;
    class JavascriptDecodeURI;
    class JavascriptDecodeURIComponent;
    class DataView;
    struct ConstructorCache;
    enum class OpCode : ushort;
    enum class OpCodeAsmJs : ushort;    
    /* enum */ struct OpLayoutType;
    /* enum */ struct OpLayoutTypeAsmJs;    
    class ExceptionBase;    
    class OutOfMemoryException;
    class ScriptDebug;
    class ScriptContext;
    struct NativeModule;
    template <class T> class RcRef;    
    class TaggedInt;
    class TaggedNumber;
    struct InterpreterStackFrame;
    struct ScriptEntryExitRecord;
    class JavascriptStackWalker;
    struct AsmJsCallStackLayout;
    class JavascriptCallStackLayout;
    class Throw;
    struct Tick;
    struct TickDelta;
    class ByteBlock;
    class FunctionInfo;
    class FunctionBody;
    class ParseableFunctionInfo;
    struct StatementLocation;
    class EntryPointInfo;
    struct LoopHeader;
    class InternalString;
    /* enum */ struct JavascriptHint;
    /* enum */ struct BuiltinFunction;
    class EnterScriptObject;
    class PropertyRecord;
    struct IsInstInlineCache;
    class EntryPointInfo;
    class PolymorphicInlineCacheInfo;
    class PropertyGuard;

    // asm.js
    namespace ArrayBufferView
    {
        enum ViewType;
    }
    struct EmitExpressionInfo;
    struct AsmJsModuleMemory;
    namespace AsmJsLookupSource
    {
        enum Source;
    }
    struct AsmJsByteCodeWriter;
    class AsmJsArrayView;
    class AsmJsType;
    class AsmJsRetType;
    class AsmJsVarType;
    class AsmJsSymbol;
    class AsmJsVarBase;
    class AsmJsVar;
    class AsmJsConstantImport;
    class AsmJsArgument;
    class AsmJsFunc;
    class AsmJsFunctionDeclaration;
    class AsmJsFunctionInfo;
    class AsmJsModuleInfo;
    class AsmJsGlobals;
    class AsmJsImportFunction;
    class AsmJsTypedArrayFunction;
    class AsmJsMathFunction;
    class AsmJsMathConst;
#ifdef ASMJS_PLAT
    class AsmJsCodeGenerator;
    class AsmJsEncoder;
#endif
    struct MathBuiltin;
    struct ExclusiveContext;
    class AsmJsModuleCompiler;
    class AsmJSCompiler;
    class AsmJSByteCodeGenerator;
    enum AsmJSMathBuiltinFunction;
    //////////////////////////////////////////////////////////////////////////
    typedef JsUtil::WeakReferenceDictionary<PropertyId, PropertyString, PowerOf2SizePolicy> PropertyStringCacheMap;

    extern const FrameDisplay NullFrameDisplay;
    extern const FrameDisplay StrictNullFrameDisplay;    
}

#include "DataStructures\EvalMapString.h"

bool IsMathLibraryId(Js::PropertyId propertyId);
#include "ByteCode\PropertyIdArray.h"
#include "ByteCode\AuxArray.h"
#include "ByteCode\VarArrayVarCount.h"

// module id
const Js::ModuleID kmodGlobal = 0;

class SourceContextInfo;
class AsyncDebug;


#include "activdbg100.h"
#ifndef NTDDI_WIN10
// These are only defined SDK Win10+
// TODO: Refactor to avoid needing these?
typedef
enum tagDEBUG_EVENT_INFO_TYPE
{
    DEIT_GENERAL = 0,
    DEIT_ASMJS_IN_DEBUGGING = (DEIT_GENERAL + 1),
    DEIT_ASMJS_SUCCEEDED = (DEIT_ASMJS_IN_DEBUGGING + 1),
    DEIT_ASMJS_FAILED = (DEIT_ASMJS_SUCCEEDED + 1)
} DEBUG_EVENT_INFO_TYPE;

#define SDO_ENABLE_LIBRARY_STACK_FRAME ((SCRIPT_DEBUGGER_OPTIONS)0x8)
#endif

#include "Language\SourceHolder.h"
#include "Language\Utf8SourceInfo.h"
#include "Language\PropertyRecord.h"
#include "Library\DelayLoadLibrary.h"
#include "Language\CallInfo.h"
#include "Language\ExecutionMode.h"
#include "BackEndAPI.h"
#include "DetachedStateBase.h"

#include "Library\Constants.h"
#include "ByteCode\OpLayoutsCommon.h"
#include "ByteCode\OpLayouts.h"
#include "ByteCode\OpLayoutsAsmJs.h"
#include "ByteCode\OpCodeUtil.h"
#include "Language\Arguments.h"

#include "Types\TypeId.h"
#include "Types\RecyclableObject.h"
#include "Library\ExpirableObject.h"
#include "Types\Type.h"
#include "Types\StaticType.h"
#include "Types\CrossSite.h"
#include "Types\CrossSiteObject.h"
#include "Types\CrossSiteEnumerator.h"
#include "Types\JavascriptEnumerator.h"
#include "Types\DynamicObject.h"
#include "Types\ArrayObject.h"
#include "Types\DynamicObjectEnumerator.h"
#include "Types\DynamicObjectSnapshotEnumerator.h"
#include "Types\DynamicObjectSnapshotEnumeratorWPCache.h"
#include "Types\TypePath.h"
#include "Types\SimplePropertyDescriptor.h"
#include "Types\SimpleDictionaryPropertyDescriptor.h"
#include "Types\DictionaryPropertyDescriptor.h"
#include "Types\TypeHandler.h"
#include "Types\NullTypeHandler.h"
#include "Types\DeferredTypeHandler.h"
#include "Types\SimpleTypeHandler.h"
#include "Types\PathTypeHandler.h"
#include "Types\SimpleDictionaryTypeHandler.h"
#include "Types\DictionaryTypeHandler.h"
#include "Types\DynamicType.h"
#ifdef NTBUILD
#include "Types\ExternalObject.h"
#endif
#ifdef ENABLE_DOM_FAST_PATH
#include "Types\DOMFastPath.h"
#endif
#include "Types\SpreadArgument.h"
#include "Language\StackTraceArguments.h"
#include "Types\PropertyDescriptor.h"
#include "Types\ActivationObjectType.h"
#include "Types\TempArenaAllocatorObject.h"
#include "Language\ValueType.h"
#include "Language\DynamicProfileInfo.h"
#include "Language\ReadOnlyDynamicProfileInfo.h"
#include "Language\SourceDynamicProfileManager.h"
#include "Debug\SourceContextInfo.h"
#include "Language\InlineCache.h"
#include "Language\InlineCachePointerArray.h"
#include "Types\FunctionInfo.h"
#include "Types\FunctionBody.h"
#include "Language\CodeGenRecyclableData.h"
#include "Types\JavascriptExceptionContext.h"
#include "Types\JavascriptExceptionObject.h"
#include "Types\PerfHint.h"

#include "ByteCode\ByteBlock.h"

#include "Library\JavascriptBuiltInFunctions.h"
#include "Library\JavascriptString.h"
#include "Library\StringCopyInfo.h"

#include "Library\ForInObjectEnumerator.h"
#include "Library\RootObjectBase.h"
#include "Library\ModuleRoot.h"
#include "Library\ArgumentsObject.h"
#include "Library\LiteralString.h"
#include "Library\BufferStringBuilder.h"
#include "Library\ConcatString.h"
#include "Library\CompoundString.h"
#include "Library\PropertyString.h"
#include "Library\LiteralStringObject.h"
#include "Library\JavascriptNumber.h"
#include "Library\JavascriptNumberObject.h"

#ifdef SIMD_JS_ENABLED
// SIMD types
#include "Library\JavascriptSIMDFloat32x4.h"
#include "Library\JavascriptSIMDFloat64x2.h"
#include "Library\JavascriptSIMDInt32x4.h"
#include "Library\JavascriptSIMDInt8x16.h"
// SIMD operations
#include "Library\SIMDFloat32x4Operation.h"
#include "Library\SIMDFloat64x2Operation.h"
#include "Library\SIMDInt32x4Operation.h"
#include "Library\SIMDInt8x16Operation.h"
#endif

#include "Library\JavascriptTypedNumber.h"
#include "Library\SparseArraySegment.h"
#include "Library\JavascriptError.h"
#include "Library\JavascriptErrorDebug.h"
#include "Library\JavascriptArray.h"

#include "Library\ES5ArrayTypeHandler.h"
#include "Library\ES5Array.h"
#include "Library\ArrayBuffer.h"
#include "Library\TypedArray.h"
#include "Library\JavascriptBoolean.h"
#include "Library\JavascriptBooleanObject.h"
#include "Library\JavascriptFunction.h"
#include "Library\ScriptFunctionType.h"
#include "Library\ScriptFunction.h"
#include "Library\StackScriptFunction.h"
#include "Library\RuntimeFunction.h"
#include "Library\JavascriptGeneratorFunction.h"
#include "Library\JavascriptTypedObjectSlotAccessorFunction.h"
#include "Library\JavascriptExternalFunction.h"

#include "Library\JavascriptObject.h"
#include "Library\BuiltInFlags.h"
#include "Library\CharStringCache.h"
#include "Library\JavascriptLibraryBase.h"
#include "Library\JavascriptLibrary.h"

#include "Library\GlobalObject.h"
#include "Language\JavascriptExceptionOperators.h"
#include "Language\JavascriptOperators.h"
#include "Library\TaggedInt.h"
#include "Library\HiResTimer.h"
#include "Library\DateImplementation.h"
#include "Library\JavascriptDate.h"

#include "Library\MathLibrary.h"
#include "Library\JSON.h"

#include "Library\ThrowErrorObject.h"
#include "Library\WindowsGlobalizationAdapter.h"
#include "Library\WindowsFoundationAdapter.h"
#include "Library\EngineInterfaceObject.h"
#include "Library\Debug.h"

#ifdef ENABLE_DOM_FAST_PATH
#include "Library\DOMFastPathInfo.h"
#endif

#ifdef TEST_LOG
#include "Library\HostLogger.h"
#endif

#ifdef _M_X64
#include "Language\amd64\stackframe.h"
#endif

#include "Library\Entropy.h"
#include "Language\PropertyRecord.h"
#ifdef ENABLE_BASIC_TELEMETRY
// REVIEW: ChakraCore Dependency
#include "..\..\..\private\lib\Telemetry\Telemetry.h"
#include "..\..\..\private\lib\Telemetry\ScriptContextTelemetry.h"
#include "..\..\..\private\lib\Telemetry\DirectCall.h"
#endif
#include "Library\ThreadContext.h"

#include "Library\StackProber.h"
#include "Library\ThreadContextTLSEntry.h"

#include "Language\EvalMapRecord.h"
#include "Language\RegexPatternMruMap.h"
#include "Language\JavascriptConversion.h"

#include "Language\ScriptContextOptimizationOverrideInfo.h"
#include "Language\scriptContextbase.h"
#include "Language\ScriptContext.h"

#include "Language\JavascriptFunctionArgIndex.h"
#include "Language\JavascriptStackWalker.h"
#include "ByteCode\ByteCodeDumper.h"
#include "ByteCode\ByteCodeReader.h"
#include "ByteCode\ByteCodeWriter.h"
#include "ByteCode\ByteCodeGenerator.h"

#include "Language\InterpreterStackFrame.h"
#include "Language\LeaveScriptObject.h"

//
// .inl files
//

#include "commoninl.h"
#include "Library\JavascriptString.inl"
#include "Library\ConcatString.inl"
#include "Language\JavascriptConversion.inl"
#include "Types\RecyclableObject.inl"
#include "Types\DynamicObject.inl"
#include "Library\JavascriptBoolean.inl"
#include "Library\JavascriptArray.inl"
#include "Library\SparseArraySegment.inl"
#include "Library\JavascriptNumber.inl"
#include "Library\JavascriptLibrary.inl"
#include "Language\InlineCache.inl"
#include "Language\InlineCachePointerArray.inl"
#include "Language\JavascriptOperators.inl"
#include "Library\TaggedInt.inl"
