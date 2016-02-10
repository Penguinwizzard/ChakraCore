// Copyright (C) Microsoft. All rights reserved.

// JScriptDiag does not link with Runtime.lib and does not include .cpp files, so this file will be included as a header
#pragma once

#include "RuntimeTypePch.h"

#ifdef IsJsDiag
    #define JsDiag_Inline inline
#else
    #define JsDiag_Inline
#endif

namespace Js
{
    #define SLOT_TYPE(name) \
        JsDiag_Inline ObjectSlotType ObjectSlotType::Get##name() \
        { \
            return SlotType::name; \
        }
    #define SLOT_TYPE_VALUE(name, value) SLOT_TYPE(name)
    #include "ObjectSlotTypes.h"
    #undef SLOT_TYPE
    #undef SLOT_TYPE_VALUE

    JsDiag_Inline ObjectSlotType::ObjectSlotType() : slotType(SlotType::Var)
    {
        CompileAssert(static_cast<TSize>(-1) >= static_cast<TSize>(0));
        CompileAssert(sizeof(TSize) == sizeof(ObjectSlotType));
        CompileAssert(static_cast<TSize>(SlotType::Var) == 0);
        DebugOnly(CompileAssert(static_cast<TSize>(SlotType::Count) <= (1u << BitSize)));
    }

    JsDiag_Inline ObjectSlotType::ObjectSlotType(const SlotType slotType) : slotType(slotType)
    {
        Assert(slotType < SlotType::Count);
    }

    JsDiag_Inline ObjectSlotType::ObjectSlotType(const size_t slotType) : slotType(static_cast<SlotType>(slotType))
    {
        Assert(slotType < static_cast<TSize>(SlotType::Count));
    }

    #ifndef IsJsDiag

    ObjectSlotType *ObjectSlotType::NewArrayOfVarSlotTypes(const size_t count, Recycler *const recycler)
    {
        CompileAssert(static_cast<TSize>(SlotType::Var) == 0);
        return reinterpret_cast<ObjectSlotType *>(RecyclerNewArrayLeafZ(recycler, uint8, count * sizeof(ObjectSlotType)));
    }

    #endif

    JsDiag_Inline bool ObjectSlotType::RequiresWideSlotSupport()
    {
    #if OBJECT_WIDE_SLOTS
        return true;
    #else
        return false;
    #endif
    }

    JsDiag_Inline bool ObjectSlotType::IsWide() const
    {
        return RequiresWideSlotSupport() && slotType != SlotType::Var;
    }

    JsDiag_Inline bool ObjectSlotType::IsVar() const
    {
        return slotType < SlotType::Float;
    }

    JsDiag_Inline bool ObjectSlotType::IsFloat() const
    {
        return slotType == SlotType::Float;
    }

    JsDiag_Inline bool ObjectSlotType::IsInt() const
    {
        return slotType > SlotType::Float;
    }

    #ifndef IsJsDiag

    template<>
    ObjectSlotType ObjectSlotType::FromValueType<int32>()
    {
        return GetInt();
    }

    template<>
    ObjectSlotType ObjectSlotType::FromValueType<double>()
    {
        return GetFloat();
    }

    template<>
    ObjectSlotType ObjectSlotType::FromValueType<Var>()
    {
        return GetVar();
    }

    ObjectSlotType ObjectSlotType::FromVar(const bool doNativeFields, const Js::Var var)
    {
        int32 intValue;
        double floatValue;
        return FromVar(doNativeFields, var, &intValue, &floatValue);
    }

    ObjectSlotType ObjectSlotType::FromVar(
        const bool doNativeFields,
        const Js::Var var,
        int32 *const intValueRef,
        double *const floatValueRef)
    {
        Assert(var);
        Assert(intValueRef);
        Assert(floatValueRef);

        if(doNativeFields)
        {
            if(TaggedInt::Is(var))
            {
                *intValueRef = TaggedInt::ToInt32(var);
                *floatValueRef = 0;
                return SlotType::Int;
            }

            if(JavascriptNumber::Is_NoTaggedIntCheck(var))
            {
                if(JavascriptNumber::TryGetInt32Value(JavascriptNumber::GetValue(var), intValueRef))
                {
                    *floatValueRef = 0;
                    return SlotType::Int;
                }

                *intValueRef = 0;
                *floatValueRef = JavascriptNumber::GetValue(var);
                return SlotType::Float;
            }
        }

        *intValueRef = 0;
        *floatValueRef = 0;
        return SlotType::Var;
    }

    bool ObjectSlotType::IsValueTypeEqualTo(const ObjectSlotType &other) const
    {
        return *this == other || RequiresWideSlotSupport() && IsVar() && other.IsVar();
    }

    bool ObjectSlotType::IsValueTypeMoreConvervativeThan(const ObjectSlotType &other) const
    {
        return slotType < other.slotType && !(RequiresWideSlotSupport() && other.IsVar());
    }

    ObjectSlotType ObjectSlotType::ToNormalizedValueType() const
    {
        // Intended for cases that don't care about the slot width, normalize wide var to narrow var
        return RequiresWideSlotSupport() && slotType == SlotType::ConvertedVar ? SlotType::Var : slotType;
    }

    ObjectSlotType ObjectSlotType::MergeValueType(const ObjectSlotType &other) const
    {
        return ObjectSlotType(min(slotType, other.slotType)).ToNormalizedValueType();
    }

    ObjectSlotType ObjectSlotType::Merge(const ObjectSlotType &other) const
    {
        if(RequiresWideSlotSupport() && (slotType == SlotType::ConvertedVar || other.slotType == SlotType::ConvertedVar))
            return SlotType::ConvertedVar;
        return MergeValueType(other);
    }

    #endif

    JsDiag_Inline PathTypePropertyIndex ObjectSlotType::GetNextSlotIndexOrCount(const PropertyIndex slotIndexOrCount) const
    {
        Assert(slotIndexOrCount <= TypePath::MaxSlotCapacity);

        PathTypePropertyIndex nextSlotIndexOrCount = static_cast<PathTypePropertyIndex>(slotIndexOrCount);
        Assert(static_cast<PropertyIndex>(nextSlotIndexOrCount) == slotIndexOrCount);
        Assert(static_cast<PathTypePropertyIndex>(nextSlotIndexOrCount + 1) > nextSlotIndexOrCount);
        ++nextSlotIndexOrCount;

        if(IsWide())
        {
            Assert(static_cast<PathTypePropertyIndex>(nextSlotIndexOrCount + 1) > nextSlotIndexOrCount);
            ++nextSlotIndexOrCount;
        }
        return nextSlotIndexOrCount;
    }

    #ifndef IsJsDiag

    bool ObjectSlotType::operator ==(const ObjectSlotType &other) const
    {
        return slotType == other.slotType;
    }

    bool ObjectSlotType::operator !=(const ObjectSlotType &other) const
    {
        return !(*this == other);
    }

    ObjectSlotType::operator TSize() const
    {
        return static_cast<TSize>(slotType);
    }

    const char *const ObjectSlotType::SlotTypeNames[] =
    {
        #define SLOT_TYPE(name) "" STRINGIZE(name) "",
        #define SLOT_TYPE_VALUE(name, value)
        #include "ObjectSlotTypes.h"
        #undef SLOT_TYPE
        #undef SLOT_TYPE_VALUE
    };

    const char *ObjectSlotType::ToString() const
    {
        return SlotTypeNames[static_cast<TSize>(slotType)];
    }

    #endif
}
