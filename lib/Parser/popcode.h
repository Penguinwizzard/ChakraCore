//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

/***************************************************************************
Node operators (indicates semantics of the parse node)
***************************************************************************/
enum OpCode : byte
{
#define PTNODE(nop,sn,pc,nk,ok,json)  nop,
#include "ptlist.h"
    knopLim
};

