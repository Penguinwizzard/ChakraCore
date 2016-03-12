//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#define WIN32_LEAN_AND_MEAN 1

#include <windows.h>
#include <stdarg.h>
#include <stdio.h>
#include <io.h>

#ifdef Assert
#undef Assert
#endif

#ifdef AssertMsg
#undef AssertMsg
#endif

#if defined(DBG)

#define AssertMsg(exp, comment)   \
do { \
if (!(exp)) \
{ \
    fprintf(stderr, "ASSERTION (%s, line %d) %s %s\n", __FILE__, __LINE__, _CRT_STRINGIZE(exp), comment); \
    fflush(stderr); \
    DebugBreak(); \
} \
} while (0)
#else
#define AssertMsg(exp, comment) ((void)0)
#endif //defined(DBG)

#define Assert(exp)             AssertMsg(exp, #exp)

#define Fail(err) hr = err; goto Error;
#define IfFailGo(expr) if (FAILED(hr = expr)) { goto Error; }
