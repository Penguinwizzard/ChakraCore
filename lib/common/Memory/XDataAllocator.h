//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#ifdef _M_X64
#include "Memory\amd64\XDataAllocator.h"
#elif defined(_M_ARM)
#include "Memory\arm\XDataAllocator.h"
#elif defined(_M_ARM64)
#include "Memory\arm64\XDataAllocator.h"
#endif
