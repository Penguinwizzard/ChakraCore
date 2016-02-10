// Copyright (C) Microsoft. All rights reserved.

#pragma once

namespace Memory
{
    class Recycler;
}

namespace Js
{
    class ObjectSlotType
    {
    public:
        typedef uint8 TSize;

    private:
        enum class SlotType : TSize
        {
            #define SLOT_TYPE(name) name,
            #define SLOT_TYPE_VALUE(name, value) name = value,
            #define SLOT_TYPE_FAKE(name) SLOT_TYPE(name)
            #include "ObjectSlotTypes.h"
            #undef SLOT_TYPE
            #undef SLOT_TYPE_VALUE
            #undef SLOT_TYPE_FAKE
        };

    public:
        static const size_t BitSize = 2;

    public:
        #define SLOT_TYPE(name) static ObjectSlotType Get##name();
        #define SLOT_TYPE_VALUE(name, value) SLOT_TYPE(name)
        #include "ObjectSlotTypes.h"
        #undef SLOT_TYPE
        #undef SLOT_TYPE_VALUE

    private:
        static const char *const SlotTypeNames[static_cast<TSize>(SlotType::Count)];

    private:
        SlotType slotType;

    public:
        ObjectSlotType();
    private:
        ObjectSlotType(const SlotType type);
    public:
        explicit ObjectSlotType(const size_t slotType);

    public:
        static ObjectSlotType *NewArrayOfVarSlotTypes(const size_t count, Recycler *const recycler);

    public:
        static bool RequiresWideSlotSupport();

    private:
        bool IsWide() const;
    public:
        bool IsVar() const;
        bool IsFloat() const;
        bool IsInt() const;

    public:
        template<class TValueType> static ObjectSlotType FromValueType();

    #ifndef IsJsDiag
    public:
        static ObjectSlotType FromVar(const bool doNativeFields, const Js::Var var);
        static ObjectSlotType FromVar(const bool doNativeFields, const Js::Var var, int32 *const intValueRef, double *const floatValueRef);
    #endif

    public:
        bool IsValueTypeEqualTo(const ObjectSlotType &other) const;
        bool IsValueTypeMoreConvervativeThan(const ObjectSlotType &other) const;
        ObjectSlotType ToNormalizedValueType() const;
        ObjectSlotType MergeValueType(const ObjectSlotType &other) const;
        ObjectSlotType Merge(const ObjectSlotType &other) const;

    public:
        PathTypePropertyIndex GetNextSlotIndexOrCount(const PropertyIndex slotIndexOrCount) const;

    public:
        bool operator ==(const ObjectSlotType &other) const;
        bool operator !=(const ObjectSlotType &other) const;
        operator TSize() const;

    public:
        const char *ToString() const;

        ENUM_CLASS_HELPER_FRIENDS(SlotType, TSize);
    };

    ENUM_CLASS_HELPERS(ObjectSlotType::SlotType, ObjectSlotType::TSize);
}
