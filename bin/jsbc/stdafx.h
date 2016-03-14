//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#include "TargetVer.h"

#ifdef _WIN32
#include <windows.h>
#include <winbase.h>
#include <oleauto.h>
#else
#include <CommonPal.h>
#endif

#ifdef _MSC_VER
#pragma warning(disable:4985)
#include <intrin.h>
#endif

#ifndef USING_PAL_STDLIB
#include <wtypes.h>
#include <stdio.h>
#include <cstdlib>
#endif

#define USED_IN_STATIC_LIB

#define DISABLE_JIT 1
#include "Runtime.h"
#include "jsbc.h"

