//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "CommonDataStructuresPch.h"
#include "DataStructures\growingArray.h"

namespace JsUtil
{
    GrowingUint32HeapArray* GrowingUint32HeapArray::Create(int _length)
    {
        return HeapNew(GrowingUint32HeapArray, &HeapAllocator::Instance, _length);
    }

}
