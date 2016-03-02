// Copyright (C) Microsoft. All rights reserved.

#pragma once

namespace Js
{
    class ObjectSlotIterator
    {
    private:
        DynamicTypeHandler *typeHandler;
        const TypePath *typePath;
        BigPropertyIndex slotCount;
        BigPropertyIndex propertyCount;
        BigPropertyIndex slotIndex;

    #if DBG
    private:
        ObjectSlotType slotType;
    #endif

    public:
        ObjectSlotIterator(Type *const type);
        ObjectSlotIterator(DynamicType *const type, const BigPropertyIndex slotIndex = 0);
        ObjectSlotIterator(RecyclableObject *const object);
        ObjectSlotIterator(DynamicObject *const object, const BigPropertyIndex slotIndex = 0);
    private:
        void Initialize(Type *const type, RecyclableObject *const object);
        void Initialize(DynamicType *const type, DynamicObject *const object);

    public:
        DynamicTypeHandler *GetTypeHandler() const;
        BigPropertyIndex GetSlotCount() const;
        BigPropertyIndex GetPropertyCount() const;

    public:
        bool IsValid() const;
        void MoveNext();

    public:
        BigPropertyIndex CurrentSlotIndex() const;
        void SetCurrentSlotIndex(const BigPropertyIndex slotIndex);
        void SetCurrentSlotIndexFromPropertyIndex(const BigPropertyIndex propertyIndex);
        ObjectSlotType CurrentSlotType() const;
        PropertyId CurrentPropertyId(ScriptContext *const scriptContext) const;
    };
}
