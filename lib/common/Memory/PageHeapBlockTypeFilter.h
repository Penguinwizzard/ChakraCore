//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

#ifdef RECYCLER_PAGE_HEAP

namespace Memory
{
    enum class PageHeapBlockTypeFilter
    {
        PageHeapBlockTypeFilterNormal = 0x01,
        PageHeapBlockTypeFilterLeaf = 0x02,
        PageHeapBlockTypeFilterFinalizable = 0x04,
        PageHeapBlockTypeFilterNormalBarrier = 0x08,
        PageHeapBlockTypeFilterFinalizableBarrier = 0x10,
        PageHeapBlockTypeFilterLarge = 0x20,
        PageHeapBlockTypeFilterMax = PageHeapBlockTypeFilterLarge,
        PageHeapBlockTypeFilterAll = 0xff
    };
};

#endif
