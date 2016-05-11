//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// JScriptDiag does not link with Runtime.lib and does not include .cpp files, so this file will be included as a header
#pragma once

#include "RuntimeTypePch.h"

namespace Js
{
    #ifndef IsJsDiag

    TypePath* TypePath::New(Recycler* recycler, PropertyIndex slotCapacity)
    {
        Assert(slotCapacity <= MaxSlotCapacity);

        if (PHASE_OFF1(Js::TypePathDynamicSizePhase))
        {
            slotCapacity = MaxSlotCapacity;
        }
        else if(slotCapacity <= InitialSlotCapacity)
        {
            slotCapacity = InitialSlotCapacity;
        }
        else
        {
            slotCapacity = static_cast<PropertyIndex>(PowerOf2Policy::GetSize(slotCapacity - TYPE_PATH_ALLOC_GRANULARITY_GAP));
            if (slotCapacity < MaxSlotCapacity)
            {
                slotCapacity += TYPE_PATH_ALLOC_GRANULARITY_GAP;
            }
        }

        Assert(slotCapacity <= MaxSlotCapacity);
        
        TypePath * newTypePath = RecyclerNewPlusZ(recycler, sizeof(PropertyRecord *) * slotCapacity, TypePath);
        // Allocate enough space for the "next" for the TinyDictionary;
        newTypePath->data = RecyclerNewPlusLeafZ(recycler, slotCapacity, TypePath::Data, (uint8)slotCapacity);

        return newTypePath;
    }

    bool TypePath::IsValidSlotIndex(const PropertyIndex slotIndex)
    {
        Assert(slotIndex < GetSlotCount());

        // The index should not point to the middle of a wide slot. The property record assignments corresponding to the
        // middle portions of a wide slot are always null.
        return !!assignments[slotIndex];
    }

    void TypePath::VerifySlotIndex(const PropertyIndex slotIndex)
    {
        Assert(IsValidSlotIndex(slotIndex));
    }

    void TypePath::VerifySlotIndex(const PropertyIndex slotIndex, const PropertyIndex objectSlotCount)
    {
        Assert(objectSlotCount <= GetSlotCount());
        Assert(slotIndex < objectSlotCount);
        VerifySlotIndex(slotIndex);
    }

    ObjectSlotType TypePath::GetSlotType(const PropertyIndex slotIndex)
    {
        VerifySlotIndex(slotIndex);
        return this->GetData()->slotTypes.Item(slotIndex);
    }

    #endif
    #ifdef IsJsDiag

    inline ObjectSlotType TypePath::GetSlotType_JsDiag(const PropertyIndex slotIndex)
    {
        // Cannot access 'assignments' in JsDiag mode
        Assert(slotIndex < GetSlotCount());
        return this->GetData()->slotTypes.Item(slotIndex);
    }

    #endif
    #ifndef IsJsDiag

    void TypePath::SetSlotType(const PropertyIndex slotIndex, const ObjectSlotType slotType)
    {
        VerifySlotIndex(slotIndex);
        this->GetData()->slotTypes.Item(slotIndex, slotType);
    }

    PropertyIndex TypePath::GetNextSlotIndex(const PropertyIndex slotIndex)
    {
        VerifySlotIndex(slotIndex);
        const PropertyIndex nextSlotIndex = GetSlotType(slotIndex).GetNextSlotIndexOrCount(slotIndex);
        if(nextSlotIndex != GetSlotCount())
        {
            VerifySlotIndex(nextSlotIndex);
        }
        return nextSlotIndex;
    }

    #endif
    #ifdef IsJsDiag

    inline PropertyIndex TypePath::GetNextSlotIndex_JsDiag(const PropertyIndex slotIndex)
    {
        // Cannot access 'assignments' in JsDiag mode
        Assert(slotIndex < GetSlotCount());
        const PropertyIndex nextSlotIndex = GetSlotType_JsDiag(slotIndex).GetNextSlotIndexOrCount(slotIndex);
        Assert(nextSlotIndex <= GetSlotCount());
        return nextSlotIndex;
    }

    #endif
    #ifndef IsJsDiag

    PropertyIndex TypePath::GetPreviousSlotIndex(const PropertyIndex slotIndexOrCount)
    {
        Assert(slotIndexOrCount > 0);
        if(slotIndexOrCount < GetSlotCount())
        {
            VerifySlotIndex(slotIndexOrCount);
        }
        else
        {
            Assert(slotIndexOrCount == GetSlotCount());
        }

        PropertyIndex previousSlotIndex = slotIndexOrCount - 1;
        if(!assignments[previousSlotIndex])
        {
            Assert(previousSlotIndex > 0);
            --previousSlotIndex;
        }
        Assert(GetNextSlotIndex(previousSlotIndex) == slotIndexOrCount);
        return previousSlotIndex;
    }

    bool TypePath::IsLastInlineSlotWasted(const PropertyIndex inlineSlotCapacity)
    {
        Assert(inlineSlotCapacity >= 0);
        Assert(inlineSlotCapacity <= GetSlotCount());

        if(!ObjectSlotType::RequiresWideSlotSupport() || inlineSlotCapacity == 0)
        {
            return false;
        }

        const PropertyIndex previousSlotIndex = inlineSlotCapacity - 1;
        if(assignments[previousSlotIndex])
        {
            return GetNextSlotIndex(previousSlotIndex) > inlineSlotCapacity;
        }

        Assert(previousSlotIndex != 0);
        Assert(GetNextSlotIndex(previousSlotIndex - 1) == inlineSlotCapacity);
        return false;
    }

    bool TypePath::HasCapacityForNewSlot(const ObjectSlotType slotType)
    {
        return slotType.GetNextSlotIndexOrCount(GetSlotCount()) <= GetSlotCapacity();
    }

    bool TypePath::CanObjectGrowForNewSlot(const PropertyIndex objectSlotCount, const ObjectSlotType slotType)
    {
        return slotType.GetNextSlotIndexOrCount(objectSlotCount) <= MaxSlotCapacity;
    }

    PropertyIndex TypePath::Lookup(PropertyId propId, PropertyIndex objectSlotCount)
    {
        return LookupInline(propId, objectSlotCount);
    }

    __inline PropertyIndex TypePath::LookupInline(PropertyId propId, PropertyIndex objectSlotCount)
    {
        if (propId == Constants::NoProperty) {
           return Constants::NoSlot;
        }
        PropertyIndex propIndex = Constants::NoSlot;
        if (this->GetData()->map.TryGetValue(propId, &propIndex, assignments)) {
            if (propIndex < objectSlotCount) {
                VerifySlotIndex(propIndex, objectSlotCount);
                return propIndex;
            }
        }
        return Constants::NoSlot;
    }

    TypePath * TypePath::Branch(Recycler * recycler, PropertyIndex objectSlotCount, const ObjectSlotType slotType, bool couldSeeProto)
    {
        AssertMsg(objectSlotCount < GetSlotCount(), "Why are we branching at the tip of the type path?");
        Assert(CanObjectGrowForNewSlot(objectSlotCount, slotType));
        
        // Ensure there is at least one free entry in the new path, so we can extend it.
        // TypePath::New will take care of aligning this appropriately.
        TypePath * branchedPath = TypePath::New(recycler, slotType.GetNextSlotIndexOrCount(objectSlotCount));

        for (PropertyIndex i = 0; i < objectSlotCount; i = GetNextSlotIndex(i))
        {
            branchedPath->AddInternal(assignments[i], GetSlotType(i));

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (couldSeeProto)
            {
                if (this->GetData()->usedFixedFields.Test(i))
                {
                    // Review (jedmiad): This is actually too conservative.  See point 4 in PathTypeHandler::ConvertToSimpleDictionaryType.
                    // We must conservatively copy all used as fixed bits if some prototype instance could also take
                    // this transition.  See comment in PathTypeHandler::ConvertToSimpleDictionaryType.
                    // Yes, we could devise a more efficient way of copying bits 1 through objectSlotCount, if performance of this
                    // code path proves important enough.
                    branchedPath->GetData()->usedFixedFields.Set(i);
                }
                else if (this->GetData()->fixedFields.Test(i))
                {
                    // We must clear any fixed fields that are not also used as fixed if some prototype instance could also take 
                    // this transition.  See comment in PathTypeHandler::ConvertToSimpleDictionaryType.
                    this->GetData()->fixedFields.Clear(i);
                }
            }
#endif
        }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        // When branching, we must ensure that fixed field values on the prefix shared by the two branches are always
        // consistent.  Hence, we can't leave any of them uninitialized, because they could later get initialized to
        // different values, by two different instances (one on the old branch and one on the new branch).  If that happened
        // and the instance from the old branch later switched to the new branch, it would magically gain a different set 
        // of fixed properties!  
        if (this->GetMaxInitializedSlotCount() < objectSlotCount)
        {
            this->SetMaxInitializedSlotCount(objectSlotCount);
        }
        branchedPath->SetMaxInitializedSlotCount(objectSlotCount);
#endif

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
        {
            Output::Print(_u("FixedFields: TypePath::Branch: singleton: 0x%p(0x%p)\n"), this->singletonInstance, this->singletonInstance->Get());
            Output::Print(_u("   fixed fields:"));

            for (PropertyIndex i = 0; i < GetSlotCount(); i = GetNextSlotIndex(i))
            {
                Output::Print(_u(" %s %d%d%d,"), GetPropertyId(i)->GetBuffer(),
                    i < GetMaxInitializedSlotCount() ? 1 : 0,
                    GetIsFixedFieldAt(i, GetSlotCount()) ? 1 : 0,
                    GetIsUsedFixedFieldAt(i, GetSlotCount()) ? 1 : 0);
            }

            Output::Print(_u("\n"));
        }
#endif

        return branchedPath;
    }

    TypePath * TypePath::Grow(Recycler * recycler)
    {
        uint currentSlotCount = this->GetSlotCount();
        Assert(
            GetSlotCount() == GetSlotCapacity() ||
            ObjectSlotType::RequiresWideSlotSupport() && GetSlotCount() + 1 == GetSlotCapacity());

        // Ensure there is at least one free entry in the new path, so we can extend it.
        // TypePath::New will take care of aligning this appropriately.
        TypePath * clonedPath = TypePath::New(recycler, static_cast<PropertyIndex>(currentSlotCount + 2));

        for (PropertyIndex i = 0; i < currentSlotCount; i = GetNextSlotIndex(i))
        {
            clonedPath->AddInternal(assignments[i], GetSlotType(i));
        }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        // Copy fixed field info
        clonedPath->SetMaxInitializedSlotCount(GetMaxInitializedSlotCount());
        clonedPath->singletonInstance = this->singletonInstance;
        clonedPath->GetData()->fixedFields = this->GetData()->fixedFields;
        clonedPath->GetData()->usedFixedFields = this->GetData()->usedFixedFields;
#endif

        return clonedPath;
    }

#if DBG
    bool TypePath::HasSingletonInstanceOnlyIfNeeded()
    {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        return DynamicTypeHandler::AreSingletonInstancesNeeded() || this->singletonInstance == nullptr;
#else
        return true;
#endif
    }
#endif

    Var TypePath::GetSingletonFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount, ScriptContext * requestContext)
    {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        VerifySlotIndex(index, objectSlotCount);

        if (!CanHaveFixedFields(objectSlotCount))
        {
            return nullptr;
        }

        DynamicObject* localSingletonInstance = this->singletonInstance->Get();

        return localSingletonInstance != nullptr && localSingletonInstance->GetScriptContext() == requestContext && this->GetData()->fixedFields.Test(index) ? localSingletonInstance->GetSlot(index, GetSlotType(index)) : nullptr;
#else
        return nullptr;
#endif
    }

    #endif
}
