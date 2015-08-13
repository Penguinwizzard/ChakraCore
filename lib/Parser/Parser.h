//---------------------------------------------------------------------------
//
// File: common.h
//
// Copyright (C) Microsoft. All rights reserved. 
//
//----------------------------------------------------------------------------

#pragma once

#define REGEX_TRIGRAMS 1 
// #define USE_POPCNT_INSTRUCTION 1
// #define USE_BITCOUNTS 1

#include "Common.h"

// FORWARD
namespace Js
{
    class ScriptContext;
    class JavascriptString;
    class JavascriptRegularExpressionResult;
}

namespace UnifiedRegex {
    struct RegexPattern;
    struct Program;
}

#include "ParserCommon.h"
#include "alloc.h"
#include "cmperr.h"
#include "errstr.h"
#include "globals.h"
#include "idiom.h"
#include "keywords.h"
#include "ptree.h"
#include "tokens.h"
#include "hash.h"
#include "CharClassifier.h"
#include "scan.h"
#include "screrror.h"
#include "rterror.h"
#include "parse.h"

#include "RegexFlags.h"

#include "Chars.h"
#include "DebugWriter.h"
#include "RegexStats.h"
#include "CaseInsensitive.h"
#include "BitCounts.h"
#include "CharSet.h"
#include "CharMap.h"
#include "CharTrie.h"
#include "StandardChars.h"
#include "OctoquadIdentifier.h"
#include "TextbookBoyerMoore.h"
#include "RegexRunTime.h"
#include "RegexCompileTime.h"
#include "RegexParser.h"
#include "RegexPattern.h"


#include "pnodewalk.h"
#include "pnodevisit.h"
#include "pnodechange.h"

#include "BackgroundParser.h"
