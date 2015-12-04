//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "CodeGen.h"

__bcount_opt(size) void * __RPC_USER midl_user_allocate(size_t size)
{
    return (HeapAlloc(GetProcessHeap(), 0, size));
}

void __RPC_USER midl_user_free(__inout void * ptr)
{
    if (ptr != NULL)
    {
        HeapFree(GetProcessHeap(), NULL, ptr);
    }
}
