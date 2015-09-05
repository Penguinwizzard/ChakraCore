//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#include "pinned.h"

#define UNREFERENCED_PARAMETER(x) (x)

void EnterPinnedScope(volatile void** var)
{
    UNREFERENCED_PARAMETER(var);    
    return;
}

void LeavePinnedScope()
{
    return;
}
