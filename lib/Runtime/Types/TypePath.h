//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    class TinyDictionary
    {
        static const int PowerOf2_BUCKETS = 8;
        static const byte NIL = 0xff;

        byte buckets[PowerOf2_BUCKETS];
        byte next[0];

public:
        TinyDictionary()
        {
            DWORD* init = (DWORD*)buckets;
            init[0] = init[1] = 0xffffffff;
        }

        void Add(PropertyId key, byte value)
        {
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
        friend class PathTypeTransitionInfo;
        friend class PathTypeSingleSuccessorTransitionInfo;
        friend class PathTypeMultipleSuccessorTransitionInfo;
        friend class PathTypeHandler;

    public:
        // This is the space between the end of the TypePath and the allocation granularity that can be used for assignments too.
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
#if defined(_M_X64_OR_ARM64)
#define TYPE_PATH_ALLOC_GRANULARITY_GAP 0
#else
#define TYPE_PATH_ALLOC_GRANULARITY_GAP 2
#endif
#else
#if defined(_M_X64_OR_ARM64)
#define TYPE_PATH_ALLOC_GRANULARITY_GAP 1
#else
#define TYPE_PATH_ALLOC_GRANULARITY_GAP 3
#endif
#endif
        // Although we can allocate 2 more, this will put struct Data into another bucket.  Just waste some slot in that case for 32-bit
        static const uint MaxSlotCapacity = 128;
        static const uint InitialSlotCapacity = 16 + TYPE_PATH_ALLOC_GRANULARITY_GAP;

    private:

        struct Data
        {
            Data(uint8 slotCapacity) : slotCapacity(slotCapacity), slotCount(0)
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
                , maxInitializedSlotCount(0)
#endif
            {}

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            BVStatic<MaxSlotCapacity> fixedFields;
            BVStatic<MaxSlotCapacity> usedFixedFields;

            // We sometimes set up PathTypeHandlers and associate TypePaths before we create any instances
            // that populate the corresponding slots, e.g. for object literals or constructors with only
            // this statements.  This field keeps track of the longest instance associated with the given
            // TypePath.
            PathTypePropertyIndex maxInitializedSlotCount;
#endif
            PathTypePropertyIndex slotCount;      // Entries in use
            PathTypePropertyIndex slotCapacity;   // Allocated entries

            JsUtil::FixedMultibitVector<ObjectSlotType, ObjectSlotType::TSize, ObjectSlotType::BitSize, MaxSlotCapacity> slotTypes;
            // This map has to be at the end, because TinyDictionary has a zero size array
            TinyDictionary map;

            int Add(const PropertyRecord * propertyId, const PropertyRecord ** assignments);
        } * data;

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        RecyclerWeakReference<DynamicObject>* singletonInstance;
#endif

        // PropertyRecord assignments are allocated off the end of the structure
        const PropertyRecord * assignments[0];


        TypePath() : 
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            singletonInstance(nullptr), 
#endif
            data(nullptr)
        {
        }

        Data * GetData() { return data; }

    public:
        static TypePath* New(Recycler* recycler, PropertyIndex slotCapacity = InitialSlotCapacity);

    public:
        bool IsValidSlotIndex(const PropertyIndex slotIndex);
    private:
        void VerifySlotIndex(const PropertyIndex slotIndex);
        void VerifySlotIndex(const PropertyIndex slotIndex, const PropertyIndex objectSlotCount);

    public:
        ObjectSlotType GetSlotType(const PropertyIndex slotIndex);
    #ifdef IsJsDiag
        ObjectSlotType GetSlotType_JsDiag(const PropertyIndex slotIndex);
    #endif
    private:
        void SetSlotType(const PropertyIndex slotIndex, const ObjectSlotType slotType);

    public:
        PropertyIndex GetNextSlotIndex(const PropertyIndex slotIndex);
    #ifdef IsJsDiag
        PropertyIndex GetNextSlotIndex_JsDiag(const PropertyIndex slotIndex);
    #endif
        PropertyIndex GetPreviousSlotIndex(const PropertyIndex slotIndexOrCount);
        bool IsLastInlineSlotWasted(const PropertyIndex inlineSlotCapacity);
        bool HasCapacityForNewSlot(const ObjectSlotType slotType);
        static bool CanObjectGrowForNewSlot(const PropertyIndex objectSlotCount, const ObjectSlotType slotType);

    public:
        TypePath * Branch(Recycler * alloc, PropertyIndex objectSlotCount, const ObjectSlotType slotType, bool couldSeeProto);

        TypePath * Grow(Recycler * alloc);

        const PropertyRecord* GetPropertyIdUnchecked(PropertyIndex index)
        {
            VerifySlotIndex(index);
            return assignments[index];
        }

        const PropertyRecord* GetPropertyId(PropertyIndex index)
        {
            if (index < GetSlotCount())
            {
                return GetPropertyIdUnchecked(index);
            }
            return nullptr;
        }

        const PropertyRecord ** GetPropertyAssignments()
        {
            return assignments;
        }

        PropertyIndex Add(const PropertyRecord * propertyRecord, const ObjectSlotType slotType)
        {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            Assert(this->GetSlotCount() == this->GetMaxInitializedSlotCount());
#endif

            const PropertyIndex newSlotIndex = AddInternal(propertyRecord, slotType);

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            SetMaxInitializedSlotCount(this->GetSlotCount());
#endif

            return newSlotIndex;
        }

        PathTypePropertyIndex GetSlotCount() { return this->GetData()->slotCount; }
        PathTypePropertyIndex GetSlotCapacity()
        {
            Assert(this->GetData()->slotCapacity >= InitialSlotCapacity);
            Assert(this->GetData()->slotCapacity <= MaxSlotCapacity);

            return this->GetData()->slotCapacity;
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

            const PathTypePropertyIndex newSlotIndex = this->GetSlotCount();

            // The previous dictionary did not replace on dupes.
            // I believe a dupe here would be a bug, but to be conservative
            // replicate the exact previous behavior.
#if DBG
            PropertyIndex temp;
            if (this->GetData()->map.TryGetValue(propId->GetPropertyId(), &temp, assignments))
            {
                AssertMsg(false, "Adding a duplicate to the type path");
            }
#endif 

            this->GetData()->map.Add(propId->GetPropertyId(), (byte)newSlotIndex);

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

            this->GetData()->slotCount = slotType.GetNextSlotIndexOrCount(newSlotIndex);
            assignments[newSlotIndex] = propId;
            SetSlotType(newSlotIndex, slotType);
            return newSlotIndex;
        }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        PropertyIndex GetMaxInitializedSlotCount() { return this->GetData()->maxInitializedSlotCount; }
        void SetMaxInitializedSlotCount(PropertyIndex newMaxInitializedSlotCount)
        {
            Assert(newMaxInitializedSlotCount <= MaxSlotCapacity);
            Assert(this->GetMaxInitializedSlotCount() <= newMaxInitializedSlotCount);
            this->GetData()->maxInitializedSlotCount = static_cast<PathTypePropertyIndex>(newMaxInitializedSlotCount);
            Assert(this->GetMaxInitializedSlotCount() <= GetSlotCount());
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
            Assert(objectSlotCount >= this->GetMaxInitializedSlotCount());
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
            return this->GetData()->fixedFields.Test(index) != 0;
        }

        bool GetIsUsedFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            return this->GetData()->usedFixedFields.Test(index) != 0;
        }

        void SetIsUsedFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            Assert(index < this->GetMaxInitializedSlotCount());
            Assert(CanHaveFixedFields(objectSlotCount));
            this->GetData()->usedFixedFields.Set(index);
        }

        void ClearIsFixedFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            VerifySlotIndex(index, objectSlotCount);
            Assert(index < this->GetMaxInitializedSlotCount());

            this->GetData()->fixedFields.Clear(index);
            this->GetData()->usedFixedFields.Clear(index);
        }

        bool CanHaveFixedFields(PropertyIndex objectSlotCount)
        {
            // We only support fixed fields on singleton instances.
            // If the instance in question is a singleton, it must be the tip of the type path.
            return this->singletonInstance != nullptr && objectSlotCount >= this->GetMaxInitializedSlotCount();
        }

        void AddBlankFieldAt(PropertyIndex index, PropertyIndex objectSlotCount)
        {
            Assert(index >= this->GetMaxInitializedSlotCount());
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
            Assert(objectSlotCount >= this->GetMaxInitializedSlotCount());
            Assert(index >= this->GetMaxInitializedSlotCount());
            // This invariant is predicated on the properties getting initialized in the order of indexes in the type handler.
            Assert(instance != nullptr);
            Assert(this->singletonInstance == nullptr || this->singletonInstance->Get() == instance);
            Assert(!this->GetData()->fixedFields.Test(index) && !this->GetData()->usedFixedFields.Test(index));

            if (this->singletonInstance == nullptr)
            {
                this->singletonInstance = instance->CreateWeakReferenceToSelf();
            }

            SetMaxInitializedSlotCount(GetNextSlotIndex(index));

            if (isFixed)
            {
                this->GetData()->fixedFields.Set(index);
            }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(_u("FixedFields: TypePath::AddSingletonInstanceFieldAt: singleton = 0x%p(0x%p)\n"), 
                    this->singletonInstance, this->singletonInstance != nullptr ? this->singletonInstance->Get() : nullptr);
                Output::Print(_u("   fixed fields:"));

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
            Assert(objectSlotCount >= this->GetMaxInitializedSlotCount());
            Assert(index >= this->GetMaxInitializedSlotCount());
            Assert(!this->GetData()->fixedFields.Test(index) && !this->GetData()->usedFixedFields.Test(index));

            SetMaxInitializedSlotCount(GetNextSlotIndex(index));

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
            {
                Output::Print(_u("FixedFields: TypePath::AddSingletonInstanceFieldAt: singleton = 0x%p(0x%p)\n"), 
                    this->singletonInstance, this->singletonInstance != nullptr ? this->singletonInstance->Get() : nullptr);
                Output::Print(_u("   fixed fields:"));

                for (PropertyIndex i = 0; i < GetSlotCount(); i = GetNextSlotIndex(i))
                {
                    Output::Print(_u(" %s %d%d%d"), GetPropertyId(i)->GetBuffer(),
                        i < GetMaxInitializedSlotCount() ? 1 : 0,
                        GetIsFixedFieldAt(i, GetSlotCount()) ? 1 : 0,
                        GetIsUsedFixedFieldAt(i, GetSlotCount()) ? 1 : 0);
                }
                
                Output::Print(_u("\n"));
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

CompileAssert((sizeof(Js::TypePath) % HeapConstants::ObjectGranularity) / sizeof(void *) == TYPE_PATH_ALLOC_GRANULARITY_GAP);
