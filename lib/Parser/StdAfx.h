//----------------------------------------------------------------------------
//
// File: StdAfx.h
//
// Copyright (C) Microsoft. All rights reserved. 
//
//----------------------------------------------------------------------------

#pragma once

#include <intsafe.h>

// Parser Includes
#include "Parser.h"
#include "keywords.h"
#include "globals.h"

#include "RegexCommon.h"
#include "DebugWriter.h"
#include "RegexStats.h"
#include "BitCounts.h"
#include "StandardChars.h"
#include "OctoquadIdentifier.h"
#include "RegexCompileTime.h"
#include "RegexParser.h"
#include "RegexPattern.h"

// Runtime includes
#include "..\Runtime\runtime.h"
#include "..\Runtime\ByteCode\Symbol.h"
#include "..\Runtime\ByteCode\Scope.h"
#include "..\Runtime\ByteCode\FuncInfo.h"
#include "..\Runtime\ByteCode\ScopeInfo.h"
