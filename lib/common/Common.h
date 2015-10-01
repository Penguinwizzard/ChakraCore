//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#define _CRT_RAND_S         // Enable rand_s in the CRT

#include "CommonDefines.h"
#pragma warning(push)
#pragma warning(disable: 4995) /* 'function': name was marked as #pragma deprecated */

// === Windows Header Files ===
#define INC_OLE2                 /* for windows.h */
#define CONST_VTABLE             /* for objbase.h */
//#define WIN32_LEAN_AND_MEAN      /* for windows.h */
#ifdef WINDOWS
#include <windows.h>
#endif

#undef Yield /* winbase.h defines this but we want to use it for Js::OpCode::Yield; it is Win16 legacy, no harm undef'ing it */

#ifdef WINDOWS
template<class T> inline
    _Post_equal_to_(a < b ? a : b) _Post_satisfies_(return <= a && return <= b)
    const T& min(const T& a, const T& b) { return a < b ? a : b; }

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

template<class T> inline
    _Post_equal_to_(a > b ? a : b)
    const T& max(const T& a, const T& b) { return a > b ? a : b; }
#endif

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175)) || defined(_M_ARM32_OR_ARM64)
#ifdef __cplusplus
extern "C" {
#endif
        _Check_return_ unsigned short __cdecl _byteswap_ushort(_In_ unsigned short _Short);
        _Check_return_ unsigned long  __cdecl _byteswap_ulong (_In_ unsigned long _Long);
        _Check_return_ unsigned __int64 __cdecl _byteswap_uint64(_In_ unsigned __int64 _Int64);
#ifdef __cplusplus
}
#endif
#pragma intrinsic(_byteswap_ushort)
#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_byteswap_uint64)

#define RtlUshortByteSwap(_x)    _byteswap_ushort((USHORT)(_x))
#define RtlUlongByteSwap(_x)     _byteswap_ulong((_x))
#define RtlUlonglongByteSwap(_x) _byteswap_uint64((_x))
#else

#ifdef WINDOWS
#if (NTDDI_VERSION >= NTDDI_WIN2K)
NTSYSAPI
USHORT
FASTCALL
RtlUshortByteSwap(
    _In_ USHORT Source
    );
#endif

#if (NTDDI_VERSION >= NTDDI_WIN2K)
NTSYSAPI
ULONG
FASTCALL
RtlUlongByteSwap(
    _In_ ULONG Source
    );
#endif

#if (NTDDI_VERSION >= NTDDI_WIN2K)
NTSYSAPI
ULONGLONG
FASTCALL
RtlUlonglongByteSwap(
    _In_ ULONGLONG Source
    );
#endif

#endif
#endif

#include <wchar.h>
//#include "spymem.h"

// === C Runtime Header Files ===
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#if defined(_UCRT)
#include <cmath>
#else
#include <math.h>
#endif
#include <time.h>

#ifdef WINDOWS
#include <io.h>
#endif

#include <fcntl.h>
#ifdef WINDOWS
#include <share.h>
#endif

extern "C" void * _AddressOfReturnAddress(void);
#ifdef WINDOWS
#include <intrin.h>
#include <malloc.h>
#endif

#ifdef _M_AMD64
#include "amd64.h"
#endif

#ifdef _M_ARM
#include "arm.h"
#endif

#ifdef _M_ARM64
#include "arm64.h"
#endif

#ifndef GET_CURRENT_FRAME_ID
#if defined(_M_IX86)
#define GET_CURRENT_FRAME_ID(f) \
    __asm { mov f, ebp }
#elif defined(_M_X64)
#define GET_CURRENT_FRAME_ID(f) \
    (f = _ReturnAddress())
#elif defined(_M_ARM)
// ARM, like x86, uses the frame pointer rather than code address
#define GET_CURRENT_FRAME_ID(f) \
    (f = arm_GET_CURRENT_FRAME())
#elif defined(_M_ARM64)
#define GET_CURRENT_FRAME_ID(f) \
    (f = arm64_GET_CURRENT_FRAME())
#else
#define GET_CURRENT_FRAME_ID(f) \
    Js::Throw::NotImplemented()
#endif
#endif

#ifdef WINDOWS
#ifndef STACK_ALIGN
# if defined(_WIN64)
#  define STACK_ALIGN 16
# elif defined(_M_ARM)
#  define STACK_ALIGN 8
# elif defined(_M_IX86)
#  define STACK_ALIGN 4
# else
#  error_missing_target
# endif
#endif
#else
#ifndef STACK_ALIGN
#define STACK_ALIGN 16
#endif
#endif

//Masking bits according to AutoSystemInfo::PageSize
#define PAGE_START_ADDR(address) ((size_t)(address) & ~(size_t)(AutoSystemInfo::PageSize - 1))
#define IS_16BYTE_ALIGNED(address) (((size_t)(address) & 0xF) == 0)
#define OFFSET_ADDR_WITHIN_PAGE(address) ((size_t)(address) & (AutoSystemInfo::PageSize - 1))

// Core APIs
#ifdef WINDOWS
#include "Core\api.h"
#else
#define __bcount(a) 
#define __ecount(a)
#define __in_bcount(a)
#define __in_ecount(a)
#define __inout
#define __in
#define __out
#define INT_PTR int64
#define INFINITE 10000
#define HANDLE int32
#define DWORD int32
#define BOOL bool
#define PVOID void*
#define LPWSTR wchar_t*
#include "core/api.h"
#endif

#pragma warning(pop)

#if _WIN32 || _WIN64
#if _M_IX86
#define I386_ASM 1
#endif //_M_IX86
#endif // _WIN32 || _WIN64

#ifndef PDATA_ENABLED
#if defined(_M_ARM32_OR_ARM64) || defined(_M_X64)
#define PDATA_ENABLED 1
#else
#define PDATA_ENABLED 0
#endif
#endif

/***************************************************************************
Types
***************************************************************************/
#undef GetObject
#undef GetClassName

typedef wchar_t wchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned long ulong;

typedef signed char sbyte;
#ifdef WINDOWS
typedef __int8 int8;
typedef __int16 int16;
typedef __int32 int32;
typedef __int64 int64;
#else
typedef __int8_t int8;
typedef __int16_t int16;
typedef __int32_t int32;
typedef __int64_t int64;
#endif

typedef unsigned char byte;

#ifdef WINDOWS
typedef unsigned __int8 uint8;
typedef unsigned __int16 uint16;
typedef unsigned __int32 uint32;
typedef unsigned __int64 uint64;
#else
typedef  __uint8_t uint8;
typedef  __uint16_t uint16;
typedef  __uint32_t uint32;
typedef  __uint64_t uint64;
#endif

#if defined (_WIN64)
typedef __int64 intptr;
typedef unsigned __int64 uintptr;
#else
#ifdef WINDOWS
typedef __int32 intptr;
typedef unsigned __int32 uintptr;
#else 
typedef int64 intptr;
typedef uint64 uintptr;
typedef uint64 UINT_PTR;
#endif
// do we need 32-bit unix define?
#endif

// charcount_t represents a count of characters in a JavascriptString
// It is unsigned and the maximum value is (INT_MAX-1)
typedef uint32 charcount_t;

//A Unicode code point
typedef uint32 codepoint_t;
const codepoint_t INVALID_CODEPOINT = (codepoint_t)-1;

// Synonym for above, 2^31-1 is used as the limit to protect against addition overflow
typedef uint32 CharCount;
const CharCount MaxCharCount = INT_MAX-1;
// As above, but 2^32-1 is used to signal a 'flag' condition (eg undefined)
typedef uint32 CharCountOrFlag;
const CharCountOrFlag CharCountFlag = (CharCountOrFlag)-1;


#define null 0

#define QUOTE(s) #s
#define STRINGIZE(s) QUOTE(s)
#define STRINGIZEW(s) TEXT(QUOTE(s))
#define IsTrueOrFalse(value)     ((value) ? L"True" : L"False")

template <class T> struct DefaultComparer;
template <typename T> struct RecyclerPointerComparer;

class StackBackTrace;
class StackBackTraceNode;

namespace regex
{    
    template <class T, bool partitionFromMiddle = false> class QuickSort;
}

namespace JsUtil
{        
    class NoResizeLock;
    template <class TKey, class TValue, class TAllocator, class SizePolicy, template <typename> class Comparer, template <typename, typename> class Entry, typename Lock> class BaseDictionary;
    template <class TKey, class TValue, class TAllocator, class SizePolicy, template <typename> class Comparer, template <typename, typename> class Entry, class LockPolicy, class SyncObject> class SynchronizedDictionary;
    template <class TKey, class TValue> class SimpleHashedEntry;
    template <class TKey, class TValue> class DictionaryEntry;
    template <class TKey, class TValue> class SimpleDictionaryEntry;
    template <class TKey, class TValue> class ImplicitKeyValueEntry;    
    template <class TKey, class TValue> class WeakRefValueDictionaryEntry;

    template <class TKey, class TValue> struct KeyValuePair;
    template <class T, class TAllocator, bool isLeaf, template <class TListType, bool clearOldEntries> class TRemovePolicy, template <typename Value> class TComparer> class List;
    template <class T, class TComparer> class QuickSort;
    template <typename T> class CharacterBuffer;    
}

namespace Js
{
    class JavascriptExceptionObject;
    class FunctionBody;
    class ParseableFunctionInfo;
    class InternalString;
    class SourceDynamicProfileManager;
    class DefaultListLockPolicy;
    class ConfigFlagsTable;
    class Configuration;
    typedef uint32 LocalFunctionId;
    typedef uint32 SourceId;
    typedef int32 PropertyId;
    typedef unsigned long ModuleID;
    typedef uint16 ProfileId;
    typedef uint32 InlineCacheIndex;
}

// module id
const Js::ModuleID kmodGlobal = 0;
extern int TotalNumberOfBuiltInProperties;

#ifdef ENABLE_JS_ETW
#define PAIR(a,b) a ## b

#define GCETW(e, args)                          \
    if (IsMemProtectMode())                     \
    {                                           \
        PAIR(EventWriteMEMPROTECT_ ## e, args); \
    }                                           \
    else                                        \
    {                                           \
        PAIR(EventWriteJSCRIPT_ ## e, args);    \
    }

#define JS_ETW(s) s
#define IS_JS_ETW(s) s
#else
#define GCETW(e, ...)
#define JS_ETW(s)
#define IS_JS_ETW(s) (false)
#endif

/*
 * Telemetry timestamp macros
 *
 * To record a particular timestamp, use RECORD_TIMESTAMP. This overwrite the previous timestamp.
 * To have auto-start/end events logged, use the AUTO_TIMESTAMP macro.
 */

#ifdef WINDOWS
#define RECORD_TIMESTAMP(Field) ::GetSystemTimeAsFileTime(&telemetryBlock->Field);
#define INC_TIMESTAMP_FIELD(Field) telemetryBlock->Field++;

class AutoTimestamp
{
public:
    AutoTimestamp(FILETIME * startTimestamp, FILETIME * endTimestamp) : endTimestamp(endTimestamp)
    {
        ::GetSystemTimeAsFileTime(startTimestamp);
    }
    ~AutoTimestamp()
    {
        ::GetSystemTimeAsFileTime(endTimestamp);
    }
private:
    FILETIME * endTimestamp;
};
#define AUTO_TIMESTAMP(Field) AutoTimestamp timestamp_##Field(&telemetryBlock->Field##StartTime, &telemetryBlock->Field##EndTime);
#endif

// Header files
#ifdef WINDOWS
#include "core\BinaryFeatureControl.h"
#endif
#include "TemplateParameter.h"
#ifdef WINDOWS
#include "CriticalSection.h"
#include "Common\Event.h"
#include "Common\vtinfo.h"
#else
#include "common/Event.h"
#include "common/vtinfo.h"
#endif

#include "EnumHelp.h"
#include "EnumClassHelp.h"
#include "TargetVer.h"
#include "Warnings.h"
#include "Assertions.h"
#ifdef WINDOWS
#include "core\SysInfo.h"
#include "Common\Tick.h"
#else
#include "common/Tick.h"
#endif

#ifdef WINDOWS
#include "Common\MathUtil.h"
#include "Common\Int16Math.h"
#include "Common\Int32Math.h"
#include "Common\UInt16Math.h"
#include "Common\UInt32Math.h"
#else
#include "common/MathUtil.h"
#include "common/Int16Math.h"
#include "common/Int32Math.h"
#include "common/UInt16Math.h"
#include "common/UInt32Math.h"
#endif
#include "Core\AllocSizeMath.h"
#include "Common\DaylightTimeHelper.h"
#include "Common\DateUtilities.h"
#include "Common\NumberUtilitiesBase.h"
#include "Common\NumberUtilities.h"
#include "core\FaultInjection.h"
#include <codex\Utf8Codex.h>
#include "Common\unicode.h"

#include "Exceptions\Throw.h"

#include "core\PerfCounter.h"
#include "core\PerfCounterSet.h"
#include "core\DelayLoadLibrary.h"

#include "Common\RejitReason.h"
#include "Common\ThreadService.h"

// Exceptions
#include "Exceptions\Exceptionbase.h"
#include "Exceptions\InternalErrorException.h"
#include "Exceptions\OutOfMemoryException.h"
#include "Exceptions\OperationAbortedException.h"
#include "Exceptions\RejitException.h"
#include "Exceptions\ScriptAbortException.h"
#include "Exceptions\StackOverflowException.h"
#include "Exceptions\NotImplementedException.h"
#include "Exceptions\EvalDisabledException.h"
#include "Exceptions\ExceptionCheck.h"
#include "Exceptions\reporterror.h"

#include "DataStructures\Comparer.h"
#include "DataStructures\SizePolicy.h"

// Memory Management
namespace Memory{}
using namespace Memory;
#include "Memory\Allocator.h"
#ifdef INTERNAL_MEM_PROTECT_HEAP_ALLOC
// REVIEW: ChakraCore Dependency
#include "..\..\..\private\lib\MemProtectHeap\MemProtectHeap.h"
#endif
#include "Memory\HeapAllocator.h"
#include "Memory\AutoPtr.h"
#include "Memory\AutoAllocatorObjectPtr.h"
#include "core\Output.h"
#include "Memory\leakreport.h"

#include "DataStructures\BitVector.h"
#include "DataStructures\SList.h"
#include "DataStructures\DList.h"
#include "DataStructures\DoublyLinkedListElement.h"
#include "DataStructures\DoublyLinkedList.h"

#include "Memory\VirtualAllocWrapper.h"
#include "Memory\ForcedMemoryConstraints.h"
#include "Memory\MemoryTracking.h"
#include "Memory\AllocationPolicyManager.h"
#include "Memory\PageAllocator.h"
#include "Memory\IdleDecommitPageAllocator.h"
#include "Memory\RecyclerPageAllocator.h"
#include "Memory\ArenaAllocator.h"
#include "Memory\FreeObject.h"
#ifdef _M_X64
#include "Memory\amd64\XDataAllocator.h"
#elif defined(_M_ARM)
#include "Memory\arm\XDataAllocator.h"
#elif defined(_M_ARM64)
#include "Memory\arm64\XDataAllocator.h"
#endif
#include "Memory\CustomHeap.h"
#include "Memory\PagePool.h"
#include "Memory\GCTelemetry.h"
#include "Memory\LanguageTelemetry.h"


#include "DataStructures\PageStack.h"
#include "DataStructures\ContinuousPageStack.h"
#include "Core\FinalizableObject.h"
#include "Memory\RecyclerWriteBarrierManager.h"
#include "Memory\HeapConstants.h"
#include "Memory\HeapBlock.h"
#include "Memory\SmallHeapBlockAllocator.h"
#include "Memory\SmallNormalHeapBlock.h"
#include "Memory\SmallLeafHeapBlock.h"
#include "Memory\SmallFinalizableHeapBlock.h"
#include "Memory\LargeHeapBlock.h"
#include "Memory\HeapBucket.h"
#include "Memory\SmallLeafHeapBucket.h"
#include "Memory\SmallNormalHeapBucket.h"
#include "Memory\SmallFinalizableHeapBucket.h"
#include "Memory\LargeHeapBucket.h"
#include "Memory\HeapInfo.h"

#include "Memory\HeapBlockMap.h"
#include "Memory\RecyclerObjectDumper.h"
#include "Memory\RecyclerWeakReference.h"
#include "Memory\RecyclerSweep.h"
#include "Memory\RecyclerHeuristic.h"
#include "Memory\RecyclerRootPtr.h"
#include "Memory\MarkContext.h"
#include "Memory\Recycler.h"
#include "Memory\RecyclerFastAllocator.h"
#include "Memory\RecyclerPointers.h"
#include "util\pinned.h"

// Data Structures 2

#include "DataStructures\StringBuilder.h"
#include "DataStructures\KeyValuePair.h"
#include "DataStructures\BaseDictionary.h"
#include "DataStructures\DictionaryEntry.h"
#include "DataStructures\Dictionary.h"
#include "DataStructures\List.h"
#include "DataStructures\Stack.h"
#include "DataStructures\Queue.h"
#include "DataStructures\CharacterBuffer.h"
#include "DataStructures\InternalString.h"
#include "DataStructures\Interval.h"
#include "DataStructures\InternalStringNoCaseComparer.h"
#include "DataStructures\SparseArray.h"
#include "DataStructures\growingArray.h"
#include "DataStructures\EvalMapString.h"
#include "DataStructures\RegexKey.h"
#include "DataStructures\LineOffsetCache.h"

#include "core\ConfigFlagsTable.h"
#include "core\ICustomConfigFlags.h"
#include "core\CmdParser.h"
#include "core\ProfileInstrument.h"
#include "core\ProfileMemory.h"
#include "core\EtwTraceCore.h"
#include "core\StackBackTrace.h"
#include "Common\Jobs.h"


#include "DataStructures\Cache.h" // Depends on config flags
#include "DataStructures\MruDictionary.h" // Depends on DoublyLinkedListElement

#include "Common\vtregistry.h"
#include "Memory\MemoryLogger.h"

#include "common\SmartFPUControl.h"

// This class is only used by AutoExp.dat
class AutoExpDummyClass
{
};

#pragma warning(push)
#if defined(PROFILE_RECYCLER_ALLOC) || defined(HEAP_TRACK_ALLOC) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
#include <typeinfo.h>
#endif
#pragma warning(pop)

