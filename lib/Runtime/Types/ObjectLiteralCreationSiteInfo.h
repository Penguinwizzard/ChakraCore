// Copyright (C) Microsoft. All rights reserved.

#pragma once

namespace Js
{
    class ObjectLiteralCreationSiteInfo sealed : public ObjectCreationSiteInfo
    {
    private:
        typedef ObjectCreationSiteInfo Base;

    private:
        const PropertyIndex initialFieldCount;

    private:
        ObjectLiteralCreationSiteInfo(const PropertyIndex initialFieldCount);

    public:
        template<class TAllocator> static ObjectLiteralCreationSiteInfo *New(const PropertyIndex initialFieldCount, TAllocator *const allocator);

    public:
        PropertyIndex GetInitialFieldCount() const;
    };
}
