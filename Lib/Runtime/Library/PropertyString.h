//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    struct PropertyCache
    {
        Type * type;
        union
        {
            struct
            {
                uint16 preventdataSlotIndexFalseRef;
                uint16 dataSlotIndex;
            };
            intptr_t ptrSlot1;
        };
        union
        {
            struct
            {
                uint8 preventFlagsFalseRef;
                ObjectSlotType::TSize slotTypeBits;
                bool isInlineSlot;
                bool isStoreFieldEnabled;
            };
            intptr_t ptrSlot2;
        };
        intptr_t blank;

        ObjectSlotType GetSlotType() const
        {
            return ObjectSlotType(slotTypeBits);
        }

        void SetSlotType(const ObjectSlotType slotType)
        {
            slotTypeBits = static_cast<ObjectSlotType::TSize>(slotType);
            Assert(GetSlotType() == slotType);
        }

        static size_t GetOffsetOfSlotTypeBits()
        {
            return offsetof(PropertyCache, slotTypeBits);
        }
    };

    CompileAssert(sizeof(PropertyCache) == sizeof(InlineCacheAllocator::CacheLayout));
    CompileAssert(offsetof(PropertyCache, blank) == offsetof(InlineCacheAllocator::CacheLayout, strongRef));

    class PropertyString sealed : public JavascriptString
    {
    protected:
        PropertyCache* propCache;
        bool registerScriptContext;
        const Js::PropertyRecord* m_propertyRecord;
        DEFINE_VTABLE_CTOR(PropertyString, JavascriptString);
        DECLARE_CONCRETE_STRING_CLASS;

        PropertyString(StaticType* type, const Js::PropertyRecord* propertyRecord, bool registerScriptContext);
    public:
        PropertyCache const * GetPropertyCache() const;
        Js::PropertyRecord const * GetPropertyRecord() const { return m_propertyRecord; }
        static PropertyString* New(StaticType* type, const Js::PropertyRecord* propertyRecord, Recycler *recycler);
        static PropertyString* New(StaticType* type, const Js::PropertyRecord* propertyRecord, ArenaAllocator *arena);
        void UpdateCache(Type * type, uint16 dataSlotIndex, const ObjectSlotType slotType, bool isInlineSlot, bool isStoreFieldEnabled);
        void ClearCache() { propCache->type = nullptr; }

        virtual void const * GetOriginalStringReference() override;
        virtual RecyclableObject * CloneToScriptContext(ScriptContext* requestContext) override;

        static uint32 GetOffsetOfPropertyCache() { return offsetof(PropertyString, propCache); }
    };
}
