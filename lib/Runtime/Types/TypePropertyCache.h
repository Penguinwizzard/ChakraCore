//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

// Must be a power of 2
#define TypePropertyCache_NumElements 16

namespace Js
{
    struct PropertyCacheOperationInfo;

    class TypePropertyCacheElement
    {
    private:
        DynamicObject *prototypeObjectWithProperty;
        PropertyId id;
        PropertyIndex index;
        ObjectSlotType slotType;
        union
        {
            struct
            {
                bool isInlineSlot : 1;
                bool isSetPropertyAllowed : 1;
                bool isMissing : 1;
            };
            uint8 bits;
        };

    public:
        TypePropertyCacheElement();

    public:
        PropertyId Id() const;
        PropertyIndex Index() const;
        ObjectSlotType SlotType() const;
        bool IsInlineSlot() const;
        bool IsSetPropertyAllowed() const;
        bool IsMissing() const;
        DynamicObject *PrototypeObjectWithProperty() const;

        void Cache(const PropertyId id, const PropertyIndex index, const ObjectSlotType slotType, const bool isInlineSlot, const bool isSetPropertyAllowed);
        void Cache(
            const PropertyId id,
            const PropertyIndex index,
            const ObjectSlotType slotType,
            const bool isInlineSlot,
            const bool isSetPropertyAllowed,
            const bool isMissing,
            DynamicObject *const prototypeObjectWithProperty,
            Type *const myParentType);
        void Clear();
    };

    class TypePropertyCache
    {
    private:
        TypePropertyCacheElement elements[TypePropertyCache_NumElements];

    private:
        static size_t ElementIndex(const PropertyId id);
        bool TryGetIndexForLoad(const bool checkMissing, const PropertyId id, PropertyIndex *const indexRef, ObjectSlotType *const slotTypeRef, bool *const isInlineSlotRef, bool *const isMissingRef, DynamicObject * *const prototypeObjectWithPropertyRef) const;
        bool TryGetIndexForStore(const PropertyId id, PropertyIndex *const indexRef, ObjectSlotType *const slotTypeRef, bool *const isInlineSlotRef) const;

    public:
        bool TryGetProperty(const bool checkMissing, RecyclableObject *const propertyObject, const PropertyId propertyId, Var *const propertyValue, ScriptContext *const requestContext, PropertyCacheOperationInfo *const operationInfo, PropertyValueInfo *const propertyValueInfo);
        bool TrySetProperty(RecyclableObject *const object, const PropertyId propertyId, Var propertyValue, ScriptContext *const requestContext, PropertyCacheOperationInfo *const operationInfo, PropertyValueInfo *const propertyValueInfo);

    public:
        void Cache(const PropertyId id, const PropertyIndex index, const ObjectSlotType slotType, const bool isInlineSlot, const bool isSetPropertyAllowed);
        void Cache(const PropertyId id, const PropertyIndex index, const ObjectSlotType slotType, const bool isInlineSlot, const bool isSetPropertyAllowed, const bool isMissing, DynamicObject *const prototypeObjectWithProperty, Type *const myParentType);
        void ClearIfPropertyIsOnAPrototype(const PropertyId id);
        void Clear(const PropertyId id);
    };
}
