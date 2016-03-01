// Copyright (C) Microsoft. All rights reserved.

#pragma once

namespace Js
{
    class ObjectCreationSiteInfo abstract
    {
    private:
        DynamicType *type;
        DynamicType *lastUsedType;
        bool canTypeChange;

    protected:
        ObjectCreationSiteInfo();

    public:
        template<class TAllocator> static ObjectCreationSiteInfo *New(TAllocator *const allocator);

    public:
        DynamicType *GetType() const;
        DynamicType *const *GetTypeRef() const;
        void SetType(DynamicType *const type, const bool canTypeChange);
        void SetLastUsedType(DynamicType *const type);
        void ClearType();
    protected:
        virtual void Unregister();
    public:
        PathTypeTransitionInfo *GetLastUsedRootTransitionInfo() const;
        bool CanTypeChange() const;
    };
}
