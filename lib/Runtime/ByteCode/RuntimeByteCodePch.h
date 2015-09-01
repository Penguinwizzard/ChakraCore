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
#include "ByteCode\Symbol.h"
#include "ByteCode\Scope.h"
#include "ByteCode\FuncInfo.h"
#include "ByteCode\ScopeInfo.h"
#include "ByteCode\StatementReader.h"

#include "ByteCode\AsmJsByteCodeWriter.h"
#include "ByteCode\AsmJSByteCodeDumper.h"

#include "ByteCode\OpCodeUtilAsmJs.h"

#include "Language\AsmJsTypes.h"





