// Copyright (C) Microsoft. All rights reserved.

#include "RuntimeTypePch.h"

namespace Js
{
    ObjectCreationSiteInfo::ObjectCreationSiteInfo() : type(nullptr), lastUsedType(nullptr), canTypeChange(false)
    {
    }

    template<class TAllocator>
    ObjectCreationSiteInfo *ObjectCreationSiteInfo::New(TAllocator *const allocator)
    {
        Assert(allocator);
        return AllocatorNew(TAllocator, allocator, ObjectCreationSiteInfo);
    }

    DynamicType *ObjectCreationSiteInfo::GetType() const
    {
        return type;
    }

    DynamicType *const *ObjectCreationSiteInfo::GetTypeRef() const
    {
        return &type;
    }

    void ObjectCreationSiteInfo::SetType(DynamicType *const type, const bool canTypeChange)
    {
        Assert(!GetType());
        Assert(type);

        this->type = type;
        lastUsedType = type;
        this->canTypeChange = canTypeChange;
    }

    void ObjectCreationSiteInfo::SetLastUsedType(DynamicType *const type)
    {
        Assert(!GetType());
        Assert(type);

        lastUsedType = type;
    }

    void ObjectCreationSiteInfo::ClearType()
    {
        Assert(CanTypeChange());
        Assert(type);

//        if (type->GetTypeHandler()->IsPathTypeHandler())
        {
            Unregister();
        }
#if 0
        else
        {
            *(volatile char*)this;
        }
#endif
        type = nullptr;
    }

    void ObjectCreationSiteInfo::Unregister()
    {
        PathTypeTransitionInfo::UnregisterObjectCreationSiteInfo(this);
    }

    PathTypeTransitionInfo *ObjectCreationSiteInfo::GetLastUsedRootTransitionInfo() const
    {
        if(!lastUsedType)
            return nullptr;

        DynamicTypeHandler *const lastUsedTypeHandler = lastUsedType->GetTypeHandler();
        if(!lastUsedTypeHandler->IsPathTypeHandler())
            return nullptr;

        PathTypeTransitionInfo *const lastUsedRootTransitionInfo =
            PathTypeHandler::FromTypeHandler(lastUsedTypeHandler)->GetRootTransitionInfo();
    #if DBG
        PathTypeHandler *const lastUsedRootTypeHandler =
            PathTypeHandler::FromTypeHandler(lastUsedRootTransitionInfo->GetType()->GetTypeHandler());
        Assert(lastUsedRootTypeHandler->GetSlotCount() == 0);
        Assert(lastUsedRootTypeHandler->GetPropertyCount() == 0);
    #endif
        return lastUsedRootTransitionInfo;
    }

    bool ObjectCreationSiteInfo::CanTypeChange() const
    {
        Assert(GetType());
        return canTypeChange;
    }
}
