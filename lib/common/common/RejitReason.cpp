// Copyright (C) Microsoft. All rights reserved. 

#include "CommonCommonPch.h"
#include "RejitReason.h"

const char *const RejitReasonNames[] =
{
    #define REJIT_REASON(n) "" STRINGIZE(n) "",
    #include "RejitReasons.h"
    #undef REJIT_REASON
};

const uint NumRejitReasons = _countof(RejitReasonNames);