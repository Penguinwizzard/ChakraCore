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
#include "Language\CacheOperators.h"
#include "Language\InlineCachePointerArray.h"
#include "Types\WithScopeObject.h"
#include "Types\SimpleDictionaryUnorderedTypeHandler.h"
#include "Types\CrossSiteEnumerator.h"

#include "Types\TypePropertyCache.h"
#include "Types\MissingPropertyTypeHandler.h"
// .inl files
#include "Language\CacheOperators.inl"
