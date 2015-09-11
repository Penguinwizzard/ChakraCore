//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#include "CommonMin.h"

// === C Runtime Header Files ===
#if defined(_UCRT)
#include <cmath>
#else
#include <math.h>
#endif

// === Codex Header Files ===
#include "codex\Utf8Codex.h"

// === Common Header Files ===
#include "Common\NumberUtilitiesBase.h"
#include "Common\NumberUtilities.h"

#pragma warning(push)
#if defined(PROFILE_RECYCLER_ALLOC) || defined(HEAP_TRACK_ALLOC) || defined(ENABLE_DEBUG_CONFIG_OPTIONS)
#include <typeinfo.h>
#endif
#pragma warning(pop)



