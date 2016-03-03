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
            return SlotType::name|SlotType::DynamicTypeDefaultAttributes; \
        } 
    #define SLOT_TYPE_VALUE(name, value) SLOT_TYPE(name)
    #define ATTRIBUTE_VALUE(name, value)
    #include "ObjectSlotTypes.h"
    #undef SLOT_TYPE
    #undef SLOT_TYPE_VALUE
    #undef ATTRIBUTE_VALUE

    JsDiag_Inline ObjectSlotType ObjectSlotType::GetDynamicTypeDefaultAttributes()
    {
        return SlotType::DynamicTypeDefaultAttributes;
    }

    JsDiag_Inline ObjectSlotType::ObjectSlotType() : slotType(SlotType::Var|SlotType::DynamicTypeDefaultAttributes)
    {
        CompileAssert(static_cast<TSize>(-1) >= static_cast<TSize>(0));
        CompileAssert(sizeof(TSize) == sizeof(ObjectSlotType));
        CompileAssert(static_cast<TSize>(SlotType::Var) == 0);
        DebugOnly(CompileAssert(static_cast<TSize>(SlotType::Count) <= (1u << BitSize)));
    }

    JsDiag_Inline ObjectSlotType::ObjectSlotType(const SlotType slotType) : slotType(slotType)
    {
        //Assert(slotType < SlotType::Count);
    }

    JsDiag_Inline ObjectSlotType::ObjectSlotType(const size_t slotType) : slotType(static_cast<SlotType>(slotType))
    {
        //Assert(slotType < static_cast<TSize>(SlotType::Count));
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
        return RequiresWideSlotSupport() && slotTypeWithoutAttributes != SlotType::Var;
    }

    JsDiag_Inline bool ObjectSlotType::IsVar() const
    {
        return slotTypeWithoutAttributes < SlotType::Float;
    }

    JsDiag_Inline bool ObjectSlotType::IsFloat() const
    {
        return slotTypeWithoutAttributes == SlotType::Float;
    }

    JsDiag_Inline bool ObjectSlotType::IsInt() const
    {
        return slotTypeWithoutAttributes == SlotType::Int;
    }

    JsDiag_Inline bool ObjectSlotType::IsWritable() const
    {
        return !!(slotType & SlotType::Writable);
    }

    JsDiag_Inline bool ObjectSlotType::IsEnumerable() const
    {
        return !!(slotType & SlotType::Enumerable);
    }
    
    JsDiag_Inline bool ObjectSlotType::IsConfigurable() const
    {
        return !!(slotType & SlotType::Configurable);
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
        return (*this).slotTypeWithoutAttributes == other.slotTypeWithoutAttributes || RequiresWideSlotSupport() && IsVar() && other.IsVar();
    }

    bool ObjectSlotType::IsValueTypeMoreConvervativeThan(const ObjectSlotType &other) const
    {
        return slotTypeWithoutAttributes < other.slotTypeWithoutAttributes && !(RequiresWideSlotSupport() && other.IsVar());
    }

    ObjectSlotType ObjectSlotType::ToNormalizedValueType() const
    {
        // Intended for cases that don't care about the slot width, normalize wide var to narrow var
        return RequiresWideSlotSupport() && slotTypeWithoutAttributes == SlotType::ConvertedVar ? SlotType::Var : slotTypeWithoutAttributes;
    }

    ObjectSlotType ObjectSlotType::MergeValueType(const ObjectSlotType &other) const
    {
        return ObjectSlotType(min(slotTypeWithoutAttributes, other.slotTypeWithoutAttributes)).ToNormalizedValueType();
    }

    ObjectSlotType ObjectSlotType::Merge(const ObjectSlotType &other) const
    {
        if(RequiresWideSlotSupport() && (slotTypeWithoutAttributes == SlotType::ConvertedVar || other.slotTypeWithoutAttributes == SlotType::ConvertedVar))
            return SlotType::ConvertedVar;
        return MergeValueType(other);
    }
    
    const ObjectSlotType ObjectSlotType::GetSlotTypeWithoutAttributes() const
    {
        return slotTypeWithoutAttributes;
    }

    PropertyAttributes ObjectSlotType::GetAttributes()
    {
        return (PropertyAttributes)attributes;
    }

    ObjectSlotType ObjectSlotType::GetAttributesMask()
    {
        return (ObjectSlotType)(SlotType::DynamicTypeDefaultAttributes << ObjectSlotType::BitSize);
    }

    ObjectSlotType ObjectSlotType::GetAttributesBitsInSlotType(PropertyAttributes attributes)
    {
        return (ObjectSlotType)((attributes & PropertyDynamicTypeDefaults) << ObjectSlotType::BitSize);
    }

    bool ObjectSlotType::HasDefaultAttributes() const
    {
        return IsWritable() && IsEnumerable() && IsConfigurable();
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
        #define ATTRIBUTE_VALUE(name, value)
        #include "ObjectSlotTypes.h"
        #undef SLOT_TYPE
        #undef SLOT_TYPE_VALUE
        #undef ATTRIBUTE_VALUE
    };

    const char *ObjectSlotType::ToString() const
    {
        return SlotTypeNames[static_cast<TSize>(slotTypeWithoutAttributes)];
    }

    #endif
}
