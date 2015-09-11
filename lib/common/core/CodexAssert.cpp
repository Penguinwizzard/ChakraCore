// Copyright (C) Microsoft. All rights reserved. 

#include "CommonCorePch.h"

// Method is expected to be implemented to link with codex.lib
// We have seperate implementations for Chakra and IE
void CodexAssert(bool condition)
{
    Assert(condition);
}

