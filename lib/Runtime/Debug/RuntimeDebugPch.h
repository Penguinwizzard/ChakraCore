//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#if defined(_UCRT)
#include <cmath>
#else
#include <math.h>
#endif

#include "Runtime.h"
#ifdef ENABLE_MUTATION_BREAKPOINT
// REVIEW: ChakraCore Dependency
#include "activdbg_private.h"
#endif
#include "Debug\DiagObjectModel.h"
#include "Debug\DiagStackFrame.h"

#ifdef ENABLE_MUTATION_BREAKPOINT
#include "Debug\MutationBreakpoint.h"
#endif