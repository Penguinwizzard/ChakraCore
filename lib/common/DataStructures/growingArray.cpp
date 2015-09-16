//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "CommonDataStructuresPch.h"
#include "DataStructures\growingArray.h"

namespace JsUtil
{
    GrowingUint32HeapArray* GrowingUint32HeapArray::Create(int _length)
    {
        return HeapNew(GrowingUint32HeapArray, &HeapAllocator::Instance, _length);
    }

}
