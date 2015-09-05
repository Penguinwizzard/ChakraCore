//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

#include "CommonMinMemory.h"

typedef _Return_type_success_(return >= 0) LONG NTSTATUS;
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

#include <wchar.h>

// === C Runtime Header Files ===
#include <stdarg.h>
#include <float.h>
#include <limits.h>
#if defined(_UCRT)
#include <cmath>
#else
#include <math.h>
#endif
#include <time.h>

#include <io.h>


extern "C" void * _AddressOfReturnAddress(void);

#include <malloc.h>

#define null 0

template <class T> struct DefaultComparer;
template <typename T> struct RecyclerPointerComparer;

class StackBackTrace;
class StackBackTraceNode;

// Exceptions
#include "Exceptions\Exceptionbase.h"
#include "Exceptions\OutOfMemoryException.h"

// Other Memory headers
#include "Memory\leakreport.h"
#include "Memory\AutoPtr.h"

// Other core headers
#include "Core\FinalizableObject.h"
#include "core\EtwTraceCore.h"
#include "core\ProfileInstrument.h"
#include "core\ProfileMemory.h"
#include "core\StackBackTrace.h"

#pragma warning(push)
#if defined(PROFILE_RECYCLER_ALLOC) || defined(HEAP_TRACK_ALLOC) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
#include <typeinfo.h>
#endif
#pragma warning(pop)


#include "Memory\Recycler.inl"
#include "Memory\MarkContext.inl"
#include "Memory\HeapBucket.inl"
#include "Memory\LargeHeapBucket.inl"
#include "Memory\HeapBlock.inl"
#include "Memory\HeapBlockMap.inl"