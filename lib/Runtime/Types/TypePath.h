//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

// If this value is modified, the copy in the JD project (TypePath::MaxSlotCapacity) needs to be updated as well
#define MAX_SLOT_CAPACITY (128)

namespace Js
{
    class TinyDictionary
    {
        static const int PowerOf2_BUCKETS = 8;
        static const byte NIL = 0xff;
        static const int NEXTPTRCOUNT = MAX_SLOT_CAPACITY;

        byte buckets[PowerOf2_BUCKETS];
        byte next[NEXTPTRCOUNT];

public:
        TinyDictionary()
        {
            DWORD* init = (DWORD*)buckets;
            init[0] = init[1] = 0xffffffff;
        }

        void Add(PropertyId key, byte value)
        {
            Assert(value < NEXTPTRCOUNT);
            __analysis_assume(value < NEXTPTRCOUNT);

            uint32 bucketIndex = key&(PowerOf2_BUCKETS-1);

            byte i = buckets[bucketIndex];
            buckets[bucketIndex] = value;
            next[value] = i;
        }

        // Template shared with diagnostics
        template <class Data>
        __inline bool TryGetValue(PropertyId key, PropertyIndex* index, const Data& data) const
        {
            uint32 bucketIndex = key&(PowerOf2_BUCKETS-1);

            for (byte i = buckets[bucketIndex] ; i != NIL ; i = next[i])
            {
                Assert(i < NEXTPTRCOUNT);
                __analysis_assume(i < NEXTPTRCOUNT);

                if (data[i]->GetPropertyId()== key)
                {
                    *index = i;
                    return true;
                }
                Assert(i != next[i]);
            }
            return false;
        }
    };

    class TypePath
    {
        friend class DynamicObject;
        friend class PathTypeTransitionInfo;
        friend class PathTypeSingleSuccessorTransitionInfo;
        friend class PathTypeMultipleSuccessorTransitionInfo;
        friend class PathTypeHandler;

    public:
        static const PropertyIndex MaxSlotCapacity = MAX_SLOT_CAPACITY;

    private:
        static const PropertyIndex InitialSlotCapacity = 16;

    private:
        TinyDictionary map;
        PathTypePropertyIndex slotCount;        // Entries in use
        PathTypePropertyIndex slotCapacity;     // Allocated entries

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        // We sometimes set up PathTypeHandlers and associate TypePaths before we create any instances
        // that populate the corresponding slots, e.g. for object literals or constructors with only
        // this statements. This field keeps track of the longest instance associated with the given
        // TypePath.
        PathTypePropertyIndex maxInitializedSlotCount;
        RecyclerWeakReference<DynamicObject>* singletonInstance;
        BVStatic<MaxSlotCapacity> fixedFields;
        BVStatic<MaxSlotCapacity> usedFixedFields;
#endif

        JsUtil::FixedMultibitVector<ObjectSlotType, ObjectSlotType::TSize, ObjectSlotType::BitSize, MaxSlotCapacity> slotTypes;
        bool hasAllPropertiesWithDefaultAttributes;

        // PropertyRecord assignments are allocated off the end of the structure
        const PropertyRecord * assignments[0];

        TypePath(const PropertyIndex slotCapacity) : slotCapacity(static_cast<PathTypePropertyIndex>(slotCapacity)), hasAllPropertiesWithDefaultAttributes(true)
        {
            CompileAssert(static_cast<PathTypePropertyIndex>(-1) >= static_cast<PathTypePropertyIndex>(0)); // must be unsigned
            CompileAssert(static_cast<PropertyIndex>(-1) >= static_cast<PropertyIndex>(0)); // must be unsigned

            CompileAssert(InitialSlotCapacity != 0);
            CompileAssert(!(InitialSlotCapacity & InitialSlotCapacity - 1)); // is power of 2

            CompileAssert(MaxSlotCapacity >= InitialSlotCapacity);
            CompileAssert(MaxSlotCapacity <= static_cast<PathTypePropertyIndex>(-1));
            CompileAssert(!(MaxSlotCapacity & MaxSlotCapacity - 1)); // is power of 2

            Assert(slotCapacity <= MaxSlotCapacity);

            // The instance is zero-allocated in New
        }

    public:
        static TypePath* New(Recycler* recycler, PropertyIndex slotCapacity = InitialSlotCapacity);

    public:
        bool IsValidSlotIndex(const PropertyIndex slotIndex) const;
    private:
        void VerifySlotIndex(const PropertyIndex slotIndex) const;
        void VerifySlotIndex(const PropertyIndex slotIndex, const PropertyIndex objectSlotCount) const;

    public:
        ObjectSlotType GetSlotType(const PropertyIndex slotIndex) const;
    #ifdef IsJsDiag
        ObjectSlotType GetSlotType_JsDiag(const PropertyIndex slotIndex) const;
    #endif
    private:
        void SetSlotType(const PropertyIndex slotIndex, const ObjectSlotType slotType);

    public:
        PropertyIndex GetNextSlotIndex(const PropertyIndex slotIndex) const;
    #ifdef IsJsDiag
        PropertyIndex GetNextSlotIndex_JsDiag(const PropertyIndex slotIndex);
    #endif
        PropertyIndex GetPreviousSlotIndex(const PropertyIndex slotIndexOrCount) const;
        bool IsLastInlineSlotWasted(const PropertyIndex inlineSlotCapacity) const;
        bool HasCapacityForNewSlot(const ObjectSlotType slotType) const;
        static bool CanObjectGrowForNewSlot(const PropertyIndex objectSlotCount, const ObjectSlotType slotType);

    public:
        TypePath * Branch(Recycler * alloc, PropertyIndex objectSlotCount, const ObjectSlotType slotType, bool couldSeeProto);

        TypePath * Grow(Recycler * alloc);

        const PropertyRecord* GetPropertyIdUnchecked(PropertyIndex index) const
        {
            VerifySlotIndex(index);
            return assignments[index];
        }

        const PropertyRecord* GetPropertyId(PropertyIndex index) const
        {
            if (index < GetSlotCount())
                return GetPropertyIdUnchecked(index);
            else
                return nullptr;
        }

        bool GetHasAllPropertiesWithDefaultAttributes()
        {
            return hasAllPropertiesWithDefaultAttributes;
        }

        void SetHasAllPropertiesWithDefaultAttributes(bool value)
        {
            hasAllPropertiesWithDefaultAttributes = value;
        }

        const PropertyRecord ** GetPropertyAssignments()
        {
            return assignments;
        }

        PropertyIndex Add(const PropertyRecord * propertyRecord, const ObjectSlotType slotType)
        {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            Assert(this->slotCount == this->maxInitializedSlotCount);
#endif

            const PropertyIndex newSlotIndex = AddInternal(propertyRecord, slotType);

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            SetMaxInitializedSlotCount(slotCount);
#endif

            return newSlotIndex;
        }

        PropertyIndex GetSlotCount() const { return this->slotCount; }

        PropertyIndex GetSlotCapacity() const
        {
            Assert(slotCapacity >= InitialSlotCapacity);
            Assert(slotCapacity <= MaxSlotCapacity);
            Assert(!(slotCapacity & slotCapacity - 1)); // is power of 2

            return this->slotCapacity;
        }
        
        PropertyIndex Lookup(PropertyId propId, PropertyIndex objectSlotCount);
        PropertyIndex LookupInline(PropertyId propId, PropertyIndex objectSlotCount);

    private:
        PropertyIndex AddInternal(const PropertyRecord* propId, const ObjectSlotType slotType)
        {
            Assert(HasCapacityForNewSlot(slotType));
            if (!HasCapacityForNewSlot(slotType))
            {
                Throw::InternalError();
            }

            const PathTypePropertyIndex newSlotIndex = slotCount;

            // The previous dictionary did not replace on dupes.
            // I believe a dupe here would be a bug, but to be conservative
            // replicate the exact previous behavior.
#if DBG
            PropertyIndex temp;
            if (map.TryGetValue(propId->GetPropertyId(), &temp, assignments))
            {
                AssertMsg(false, "Adding a duplicate to the type path");
            }
#endif 

            map.Add(propId->GetPropertyId(), (byte)newSlotIndex);

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddInternal: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != nullptr ? this->singletonInstance->Get() : nullptr);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetSlotCount(); i = GetNextSlotIndex(i))
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedSlotCount() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetSlotCount()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetSlotCount()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif

            slotCount = slotType.GetNextSlotIndexOrCount(newSlotIndex);
            assignments[newSlotIndex] = propId;
            SetSlotType(newSlotIndex, slotType.GetSlotTypeWithoutAttributes());
            return newSlotIndex;
        }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        PropertyIndex GetMaxInitializedSlotCount() { return this->maxInitializedSlotCount; }
        void SetMaxInitializedSlotCount(PropertyIndex newMaxInitializedSlotCount)
        {
            Assert(newMaxInitializedSlotCount <= MaxSlotCapacity);
            Assert(this->maxInitializedSlotCount <= newMaxInitializedSlotCount);
            this->maxInitializedSlotCount = static_cast<PathTypePropertyIndex>(newMaxInitializedSlotCount);
            Assert(GetMaxInitializedSlotCount() <= GetSlotCount());
        }

        Var GetSingletonFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount, ScriptContext * requestContext);

        bool HasSingletonInstance() const
        {
            return this->singletonInstance != nullptr;
        }

        RecyclerWeakReference<DynamicObject>* GetSingletonInstance() const
        {
            return this->singletonInstance;
        }

        void SetSingletonInstance(RecyclerWeakReference<DynamicObject>* instance, PropertyIndex objectSlotCount)
        {
            Assert(this->singletonInstance == nullptr && instance != nullptr);
            Assert(objectSlotCount >= this->maxInitializedSlotCount);
            this->singletonInstance = instance;
        }

        void ClearSingletonInstance()
        {
            this->singletonInstance = nullptr;
        }

        void ClearSingletonInstanceIfSame(DynamicObject* instance)
        {
            if (this->singletonInstance != nullptr && this->singletonInstance->Get() == instance)
            {
                ClearSingletonInstance();
            }
        }

        void ClearSingletonInstanceIfDifferent(DynamicObject* instance)
        {
            if (this->singletonInstance != nullptr && this->singletonInstance->Get() != instance)
            {
                ClearSingletonInstance();
            }
        }

        bool GetIsFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            return this->fixedFields.Test(index) != 0;
        }

        bool GetIsUsedFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            return this->usedFixedFields.Test(index) != 0;
        }

        void SetIsUsedFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            Assert(index < this->maxInitializedSlotCount);
            Assert(CanHaveFixedFields(objectSlotCount));
            this->usedFixedFields.Set(index);
        }

        void ClearIsFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            Assert(index < this->maxInitializedSlotCount);

            this->fixedFields.Clear(index);
            this->usedFixedFields.Clear(index);
        }

        bool CanHaveFixedFields(PropertyIndex objectSlotCount)
        {
            // We only support fixed fields on singleton instances.
            // If the instance in question is a singleton, it must be the tip of the type path.
            return this->singletonInstance != nullptr && objectSlotCount >= this->maxInitializedSlotCount;
        }

        void AddBlankFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            Assert(index >= this->maxInitializedSlotCount);
            SetMaxInitializedSlotCount(GetNextSlotIndex(index));

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddBlankFieldAt: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != nullptr ? this->singletonInstance->Get() : nullptr);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetSlotCount(); i = GetNextSlotIndex(i))
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedSlotCount() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetSlotCount()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetSlotCount()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif
        }

        void AddSingletonInstanceFieldAt(DynamicObject* instance, PropertyIndex index, bool isFixed, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            Assert(objectSlotCount >= this->maxInitializedSlotCount);
            Assert(index >= this->maxInitializedSlotCount);
            // This invariant is predicated on the properties getting initialized in the order of indexes in the type handler.
            Assert(instance != nullptr);
            Assert(this->singletonInstance == nullptr || this->singletonInstance->Get() == instance);
            Assert(!fixedFields.Test(index) && !usedFixedFields.Test(index));

            if (this->singletonInstance == nullptr)
            {
                this->singletonInstance = instance->CreateWeakReferenceToSelf();
            }

            SetMaxInitializedSlotCount(GetNextSlotIndex(index));

            if (isFixed)
            {
                this->fixedFields.Set(index);
            }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddSingletonInstanceFieldAt: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != nullptr ? this->singletonInstance->Get() : nullptr);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetSlotCount(); i = GetNextSlotIndex(i))
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedSlotCount() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetSlotCount()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetSlotCount()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif
        }

        void AddSingletonInstanceFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            Assert(objectSlotCount >= this->maxInitializedSlotCount);
            Assert(index >= this->maxInitializedSlotCount);
            Assert(!fixedFields.Test(index) && !usedFixedFields.Test(index));

            SetMaxInitializedSlotCount(GetNextSlotIndex(index));

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(L"FixedFields: TypePath::AddSingletonInstanceFieldAt: singleton = 0x%p(0x%p)\n", 
                    this->singletonInstance, this->singletonInstance != nullptr ? this->singletonInstance->Get() : nullptr);
                Output::Print(L"   fixed fields:");

                for (PropertyIndex i = 0; i < GetSlotCount(); i = GetNextSlotIndex(i))
                {
                    Output::Print(L" %s %d%d%d,", GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedSlotCount() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetSlotCount()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetSlotCount()) ? 1 : 0);
                }
                
                Output::Print(L"\n");
            }
#endif
        }

#if DBG
        bool HasSingletonInstanceOnlyIfNeeded();
#endif

#else
        PropertyIndex GetMaxInitializedSlotCount() { Assert(false); return this->slotCount; }

        Var GetSingletonFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount, ScriptContext * requestContext);

        bool HasSingletonInstance() const { Assert(false); return false; }
        RecyclerWeakReference<DynamicObject>* GetSingletonInstance() const { Assert(false); return nullptr; }
        void SetSingletonInstance(RecyclerWeakReference<DynamicObject>* instance, PropertyIndex objectSlotCount) { Assert(false); }
        void ClearSingletonInstance() { Assert(false); }
        void ClearSingletonInstanceIfSame(RecyclerWeakReference<DynamicObject>* instance) { Assert(false); }
        void ClearSingletonInstanceIfDifferent(RecyclerWeakReference<DynamicObject>* instance) { Assert(false); }

        bool GetIsFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount) { Assert(false); return false; }
        bool GetIsUsedFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount) { Assert(false); return false; }
        void SetIsUsedFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount) { Assert(false); }
        void ClearIsFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount) { Assert(false); }
        bool CanHaveFixedFields(PropertyIndex objectSlotCount) { Assert(false); return false; }
        void AddBlankFieldAt(PropertyIndex index, PropertyIndex objectSlotCount) { Assert(false); }
        void AddSingletonInstanceFieldAt(DynamicObject* instance, PropertyIndex index, bool isFixed, PropertyIndex objectSlotCount) { Assert(false); }
        void AddSingletonInstanceFieldAt(PropertyIndex index, PropertyIndex objectSlotCount) { Assert(false); }
#if DBG
        bool HasSingletonInstanceOnlyIfNeeded();
#endif
#endif
    };
}

#undef MAX_SLOT_CAPACITY
