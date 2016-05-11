// Copyright (C) Microsoft. All rights reserved.

#include "RuntimeTypePch.h"

namespace Js
{
    ObjectLiteralCreationSiteInfo::ObjectLiteralCreationSiteInfo(const PropertyIndex initialFieldCount)
        : initialFieldCount(initialFieldCount)
    {
    }

    template<class TAllocator>
    ObjectLiteralCreationSiteInfo *ObjectLiteralCreationSiteInfo::New(
        const PropertyIndex initialFieldCount,
        TAllocator *const allocator)
    {
        Assert(allocator);
        return AllocatorNew(TAllocator, allocator, ObjectLiteralCreationSiteInfo, initialFieldCount);
    }
    template ObjectLiteralCreationSiteInfo *ObjectLiteralCreationSiteInfo::New(const PropertyIndex initialFieldCount, ArenaAllocator *const allocator);
    template ObjectLiteralCreationSiteInfo *ObjectLiteralCreationSiteInfo::New(const PropertyIndex initialFieldCount, Recycler *const allocator);

    PropertyIndex ObjectLiteralCreationSiteInfo::GetInitialFieldCount() const
    {
        return initialFieldCount;
    }
}
