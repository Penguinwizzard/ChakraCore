//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#if defined(_M_IX86_OR_ARM32)
#define PolymorphicInlineCacheShift 5 // On 32 bit architectures, the least 5 significant bits of a DynamicTypePointer is 0
#else
#define PolymorphicInlineCacheShift 6 // On 64 bit architectures, the least 6 significant bits of a DynamicTypePointer is 0
#endif

namespace Js
{
    enum CacheType : byte
    {
        CacheType_None,
        CacheType_Local,
        CacheType_Proto,
        CacheType_LocalWithoutProperty,
        CacheType_Getter,
        CacheType_Setter,
        CacheType_TypeProperty,
    };

    enum SlotLocation : byte
    {
        SlotLocation_None,
        SlotLocation_Inline,
        SlotLocation_Aux,
    };

    struct PropertyCacheOperationInfo
    {
        PropertyCacheOperationInfo()
            : cacheType(CacheType_None), slotLocation(SlotLocation_None), slotType(ObjectSlotType::GetInt()), isPolymorphic(false)
        {
        }

        CacheType cacheType;
        SlotLocation slotLocation;
        ObjectSlotType slotType;
        bool isPolymorphic;
    };

    class InlineCacheHitSlotTypeChangeInfo
    {
    private:
        ObjectSlotType slotType;
        bool isInlineSlot;
        PropertyIndex slotIndex;

    public:
        InlineCacheHitSlotTypeChangeInfo() : slotType(ObjectSlotType::GetInt())
        {
            Assert(!WasSlotTypeChanged());
        }

        InlineCacheHitSlotTypeChangeInfo(const ObjectSlotType slotType, const bool isInlineSlot, const PropertyIndex slotIndex)
            : slotType(slotType), isInlineSlot(isInlineSlot), slotIndex(slotIndex)
        {
            Assert(WasSlotTypeChanged());
        }

    public:
        bool WasSlotTypeChanged() const
        {
            return !slotType.IsInt();
        }

        ObjectSlotType GetSlotType() const
        {
            Assert(WasSlotTypeChanged());
            return slotType;
        }

        bool IsInlineSlot() const
        {
            Assert(WasSlotTypeChanged());
            return isInlineSlot;
        }

        PropertyIndex GetSlotIndex() const
        {
            Assert(WasSlotTypeChanged());
            return slotIndex;
        }
    };

    struct InlineCacheTypeTagger
    {
        static Type *TypeWithAllTags(const Type *const type, const ObjectSlotType slotType, const bool isInlineSlot);
        static Type *TypeWithoutAnyTags(const Type *const type);

        static bool TypeHasAuxSlotTag(const Type *const type);
        static Type *TypeWithAuxSlotTag(const Type *const type);

        static ObjectSlotType GetSlotType(const Type *const type);
        static Type *TypeWithSlotType(const Type *const type, const ObjectSlotType slotType);
        static Type *TypeWithoutSlotType(const Type *const type);
    };

    struct InlineCache sealed : protected InlineCacheTypeTagger
    {
        static const int CacheLayoutSelectorBitCount = 1;
        static const int RequiredAuxSlotCapacityBitCount = 15;
        static const bool IsPolymorphic = false;

        InlineCache() {}

        union
        {
            // Invariants:
            // - Type* fields do not overlap.
            // - "next" field is non-null iff the cache is linked in a list of proto-caches
            //   (see ScriptContext::RegisterProtoInlineCache and ScriptContext::InvalidateProtoCaches).

            struct s_local
            {
                Type* type;

                // PatchPutValue caches here the type the object has before a new property is added.
                // If this type is hit again we can immediately change the object's type to "type"
                // and store the value into the slot "slotIndex".
                Type* typeWithoutProperty;

                union
                {
                    struct
                    {
                        uint16 isLocal : 1;
                        uint16 requiredAuxSlotCapacity : 15;     // Maximum auxiliary slot capacity (for a path type) must be < 2^16
                    };
                    uint16 rawUInt16;                           // Required for access from JIT-ed code
                };
                uint16 slotIndex;
            } local;

            struct s_proto
            {
                uint16 isProto : 1;
                uint16 isMissing : 1;
                uint16 unused : 14;
                uint16 slotIndex;

                // It's OK for the type in proto layout to overlap with typeWithoutProperty in the local layout, because
                // we only use typeWithoutProperty on field stores, which can never have a proto layout.
                Type* type;

                DynamicObject* prototypeObject;
            } proto;

            struct s_accessor
            {
                DynamicObject *object;

                union
                {
                    struct {
                        uint16 isAccessor : 1;
                        uint16 flags : 2;
                        uint16 isOnProto : 1;
                        uint16 unused : 12;
                    };
                    uint16 rawUInt16;
                };
                uint16 slotIndex;

                Type * type;
            } accessor;

            CompileAssert(sizeof(s_local) == sizeof(s_proto));
            CompileAssert(sizeof(s_local) == sizeof(s_accessor));
        } u;

        InlineCache** invalidationListSlotPtr;

        bool IsEmpty() const
        {
            return u.local.type == nullptr;
        }

        bool IsLocal() const
        {
            return u.local.isLocal;
        }

        bool IsProto() const
        {
            return u.proto.isProto;
        }

        bool IsAccessor() const
        {
            return u.accessor.isAccessor;
        }

        bool IsAccessorOnProto() const
        {
            return IsAccessor() && u.accessor.isOnProto;
        }

        bool IsGetterAccessor() const
        {
            return IsAccessor() && !!(u.accessor.flags & InlineCacheGetterFlag);
        }

        bool IsGetterAccessorOnProto() const
        {
            return IsGetterAccessor() && u.accessor.isOnProto;
        }

        bool IsSetterAccessor() const
        {
            return IsAccessor() && !!(u.accessor.flags & InlineCacheSetterFlag);
        }

        bool IsSetterAccessorOnProto() const
        {
            return IsSetterAccessor() && u.accessor.isOnProto;
        }

        Type* GetRawType() const
        {
            return IsLocal() ? u.local.type : (IsProto() ? u.proto.type : (IsAccessor() ? u.accessor.type : nullptr));
        }

        Type* GetType() const
        {
            return TypeWithoutAnyTags(GetRawType());
        }

        Type *GetTypeForHashing() const
        {
            return
                TypeWithoutAnyTags(
                    IsLocal()
                        ? u.local.typeWithoutProperty ? u.local.typeWithoutProperty : u.local.type
                    : IsProto()
                        ? u.proto.type
                    : IsAccessor()
                        ? u.accessor.type
                    : nullptr);
        }

    public:
        using InlineCacheTypeTagger::GetSlotType;

    public:
        ObjectSlotType GetSlotType() const;
        void InitializeSlotType(FunctionBody *const functionBody, const InlineCacheIndex cacheIndex);
    private:
        bool HasSlotTypeForEmptyCache() const;
        ObjectSlotType GetSlotTypeForEmptyCache() const;
        void SetSlotTypeForEmptyCache(const ObjectSlotType slotType);

    public:
        template<bool IsCachingAccessor>
        __inline bool ShouldBecomeMorePolymorphic(
            const bool isCachingProto,
            const Type *const type,
            const Type *const typeWithoutProperty,
            const ObjectSlotType slotType)
        {
            Assert(!IsCachingAccessor && !isCachingProto || !typeWithoutProperty);

            if(IsEmpty())
            {
                return false;
            }

            if(IsCachingAccessor)
            {
                return TypeWithoutAnyTags(u.accessor.type) != type && !IsEmpty();
            }

            if(isCachingProto)
            {
                return TypeWithoutAnyTags(u.proto.type) != type && !IsEmpty();
            }

            // If the new type matches the cached type, the types without property must also match (unless one of
            // them is null).
            const bool hasDifferentTypeWithProperty = TypeWithoutAnyTags(u.local.type) != type;
            Assert(
                hasDifferentTypeWithProperty ||
                u.local.typeWithoutProperty == nullptr ||
                typeWithoutProperty == nullptr ||
                TypeWithoutAnyTags(u.local.typeWithoutProperty) == typeWithoutProperty);

            // Don't consider a cache polymorphic, if it differs only by the typeWithoutProperty.  We can handle
            // this case with the monomorphic cache.
            return hasDifferentTypeWithProperty && (!IsLocal() || GetSlotType(u.local.type).IsValueTypeEqualTo(slotType));
        }

        bool HasType_Accessor(const Type * type) const
        {
            return TypeWithoutAnyTags(u.accessor.type) == type;
        }

        bool RemoveFromInvalidationList()
        {
            if (this->invalidationListSlotPtr == nullptr)
            {
                return false;
            }

            *this->invalidationListSlotPtr = nullptr;
            this->invalidationListSlotPtr = nullptr;
            return true;
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        const wchar_t *LayoutString() const
        {
            if (IsEmpty())
            {
                return L"Empty";
            }
            if (IsLocal())
            {
                return L"Local";
            }
            if (IsAccessor())
            {
                return L"Accessor";
            }
            return L"Proto";
        }
#endif

    public:
        void CacheLocal(
            Type *const type,
            const PropertyId propertyId,
            const PropertyIndex propertyIndex,
            const ObjectSlotType slotType,
            const bool isInlineSlot,
            Type *const typeWithoutProperty,
            int requiredAuxSlotCapacity,
            ScriptContext *const requestContext);

        void CacheProto(
            DynamicObject *const prototypeObjectWithProperty,
            const PropertyId propertyId,
            const PropertyIndex propertyIndex,
            const bool isInlineSlot,
            const bool isMissing,
            Type *const type,
            ScriptContext *const requestContext);

        void CacheMissing(
            DynamicObject *const missingPropertyHolder,
            const PropertyId propertyId,
            const PropertyIndex propertyIndex,
            const bool isInlineSlot,
            Type *const type,
            ScriptContext *const requestContext);

        void CacheAccessor(
            const bool isGetter,
            const PropertyId propertyId,
            const PropertyIndex propertyIndex,
            const bool isInlineSlot,
            Type *const type,
            DynamicObject *const object,
            const bool isOnProto,
            ScriptContext *const requestContext);

        template<
            bool CheckLocal,
            bool CheckProto,
            bool CheckAccessor,
            bool CheckMissing,
            bool ReturnOperationInfo>
        bool TryGetProperty(
            Var const instance,
            RecyclableObject *const propertyObject,
            const PropertyId propertyId,
            Var *const propertyValue,
            ScriptContext *const requestContext,
            PropertyCacheOperationInfo *const operationInfo);

        template<
            bool CheckLocal,
            bool CheckLocalTypeWithoutProperty,
            bool CheckAccessor,
            bool ReturnOperationInfo>
        bool TrySetProperty(
            RecyclableObject *const object,
            const PropertyId propertyId,
            Var propertyValue,
            ScriptContext *const requestContext,
            PropertyCacheOperationInfo *const operationInfo,
            InlineCacheHitSlotTypeChangeInfo &cacheHitSlotTypeChangeInfo,
            const PropertyOperationFlags propertyOperationFlags = PropertyOperation_None);

        bool PretendTryGetProperty(Type *const type, PropertyCacheOperationInfo * operationInfo);
        template<
            bool CheckLocalAndAccessor,
            bool CheckLocalTypeWithoutProperty>
        bool PretendTrySetProperty(Type *const type, Type *const oldType, PropertyCacheOperationInfo * operationInfo);

        void Clear();
        template <class TAllocator>
        InlineCache *Clone(TAllocator *const allocator);
        InlineCache *Clone(Js::PropertyId propertyId, ScriptContext* scriptContext);
        void CopyTo(PropertyId propertyId, ScriptContext * scriptContext, InlineCache * const clone);
        bool TryGetFixedMethodFromCache(Js::FunctionBody* functionBody, uint cacheId, Js::JavascriptFunction** pFixedMethod);

        bool GetGetterSetter(Type *const type, RecyclableObject **callee);
        bool GetCallApplyTarget(RecyclableObject* obj, RecyclableObject **callee);

        static uint GetGetterFlagMask()
        {
            // First bit is marked for isAccessor in the accessor cache layout.
            return  InlineCacheGetterFlag << 1;
        }

        static uint GetSetterFlagMask()
        {
            // First bit is marked for isAccessor in the accessor cache layout.
            return  InlineCacheSetterFlag << 1;
        }

        static uint GetGetterSetterFlagMask()
        {
            // First bit is marked for isAccessor in the accessor cache layout.
            return  (InlineCacheGetterFlag | InlineCacheSetterFlag) << 1;
        }

        bool NeedsToBeRegisteredForProtoInvalidation() const;
        bool NeedsToBeRegisteredForStoreFieldInvalidation() const;

        void RegisterStoreFieldInlineCache(ScriptContext *const requestContext, const PropertyId propertyId);
        void RegisterProtoInlineCache(ScriptContext *const requestContext, const PropertyId propertyId);

#if DEBUG
        bool ConfirmCacheMiss(const Type * oldType, const PropertyValueInfo* info) const;
        bool NeedsToBeRegisteredForInvalidation() const;
        static void VerifyRegistrationForInvalidation(const InlineCache* cache, ScriptContext* scriptContext, Js::PropertyId propertyId);
#endif

#if DBG_DUMP
        void Dump();
#endif
    };

#if defined(_M_IX86_OR_ARM32)
    CompileAssert(sizeof(InlineCache) == 0x10);
#else
    CompileAssert(sizeof(InlineCache) == 0x20);
#endif

    CompileAssert(sizeof(InlineCache) == sizeof(InlineCacheAllocator::CacheLayout));
    CompileAssert(offsetof(InlineCache, invalidationListSlotPtr) == offsetof(InlineCacheAllocator::CacheLayout, strongRef));

    struct PolymorphicInlineCache sealed : protected InlineCacheTypeTagger, public FinalizableObject
    {
#ifdef INLINE_CACHE_STATS
        friend class Js::ScriptContext;
#endif

    public:
        static const bool IsPolymorphic = true;

    private:
        InlineCache * inlineCaches;
        FunctionBody * functionBody;
        uint16 size;
        ObjectSlotType slotType;
        bool ignoreForEquivalentObjTypeSpec : 1;
        bool cloneForJitTimeUse : 1;

        int32 inlineCachesFillInfo;

        // DList chaining all polymorphic inline caches of a FunctionBody together.
        // Since PolymorphicInlineCache is a leaf object, these references do not keep
        // the polymorphic inline caches alive. When a PolymorphicInlineCache is finalized,
        // it removes itself from the list and deletes its inline cache array.
        PolymorphicInlineCache * next;
        PolymorphicInlineCache * prev;

        PolymorphicInlineCache(InlineCache * inlineCaches, uint16 size, FunctionBody * functionBody, const ObjectSlotType slotType)
            : inlineCaches(inlineCaches), functionBody(functionBody), size(size),
            slotType(slotType.ToNormalizedValueType()),
            ignoreForEquivalentObjTypeSpec(false), cloneForJitTimeUse(true), inlineCachesFillInfo(0), next(nullptr), prev(nullptr)
        {
            Assert((size == 0 && inlineCaches == nullptr) ||
                (inlineCaches != nullptr && size >= MinPolymorphicInlineCacheSize && size <= MaxPolymorphicInlineCacheSize));
        }

    public:
        static PolymorphicInlineCache * New(uint16 size, FunctionBody * functionBody, const ObjectSlotType slotType);

        static uint16 GetInitialSize() { return MinPolymorphicInlineCacheSize; }
        bool CanAllocateBigger() { return GetSize() < MaxPolymorphicInlineCacheSize; }
        static uint16 GetNextSize(uint16 currentSize)
        {
            if (currentSize == MaxPolymorphicInlineCacheSize)
            {
                return 0;
            }
            else
            {
                Assert(currentSize >= MinPolymorphicInlineCacheSize && currentSize <= (MaxPolymorphicInlineCacheSize / 2));
                return currentSize * 2;
            }
        }

        template<bool IsCachingAccessor>
        bool ShouldBecomeMorePolymorphic(const bool isCachingProto, const Type *const type, const Type *const typeWithoutProperty, const ObjectSlotType slotType);
        bool HasType_Accessor(const Type * type) const;
        
        InlineCache * GetInlineCaches() const { return inlineCaches; }
        uint16 GetSize() const { return size; }
        PolymorphicInlineCache * GetNext() { return next; }
        bool GetIgnoreForEquivalentObjTypeSpec() const { return this->ignoreForEquivalentObjTypeSpec; }
        void SetIgnoreForEquivalentObjTypeSpec(bool value) { this->ignoreForEquivalentObjTypeSpec = value; }
        bool GetCloneForJitTimeUse() const { return this->cloneForJitTimeUse; }
        void SetCloneForJitTimeUse(bool value) { this->cloneForJitTimeUse = value; }
        uint32 GetInlineCachesFillInfo() { return this->inlineCachesFillInfo; }
        void UpdateInlineCachesFillInfo(uint32 index, bool set);
        bool IsFull();

        ObjectSlotType GetSlotType() const;
        void SetSlotType(ObjectSlotType slotType);

        virtual void Finalize(bool isShutdown) override;
        virtual void Dispose(bool isShutdown) override { };
        virtual void Mark(Recycler *recycler) override { AssertMsg(false, "Mark called on object that isn't TrackableObject"); }

        void CacheLocal(
            Type *const type,
            const PropertyId propertyId,
            const PropertyIndex propertyIndex,
            const ObjectSlotType slotType,
            const bool isInlineSlot,
            Type *const typeWithoutProperty,
            int requiredAuxSlotCapacity,
            ScriptContext *const requestContext);

        void CacheProto(
            DynamicObject *const prototypeObjectWithProperty,
            const PropertyId propertyId,
            const PropertyIndex propertyIndex,
            const bool isInlineSlot,
            const bool isMissing,
            Type *const type,
            ScriptContext *const requestContext);

        void CacheAccessor(
            const bool isGetter,
            const PropertyId propertyId,
            const PropertyIndex propertyIndex,
            const bool isInlineSlot,
            Type *const type,
            DynamicObject *const object,
            const bool isOnProto,
            ScriptContext *const requestContext);

        template<
            bool CheckLocal,
            bool CheckProto,
            bool CheckAccessor,
            bool CheckMissing,
            bool IsInlineCacheAvailable,
            bool ReturnOperationInfo>
        bool TryGetProperty(
            Var const instance,
            RecyclableObject *const propertyObject,
            const PropertyId propertyId,
            Var *const propertyValue,
            ScriptContext *const requestContext,
            PropertyCacheOperationInfo *const operationInfo,
            InlineCache *const inlineCacheToPopulate);

        template<
            bool CheckLocal,
            bool CheckLocalTypeWithoutProperty,
            bool CheckAccessor,
            bool ReturnOperationInfo,
            bool PopulateInlineCache>
        bool TrySetProperty(
            RecyclableObject *const object,
            const PropertyId propertyId,
            Var propertyValue,
            ScriptContext *const requestContext,
            PropertyCacheOperationInfo *const operationInfo,
            InlineCacheHitSlotTypeChangeInfo &cacheHitSlotTypeChangeInfo,
            InlineCache *const inlineCacheToPopulate,
            const PropertyOperationFlags propertyOperationFlags = PropertyOperation_None);

        bool PretendTryGetProperty(Type *const type, PropertyCacheOperationInfo * operationInfo);
        bool PretendTrySetProperty(Type *const type, Type *const oldType, PropertyCacheOperationInfo * operationInfo);

        void CopyTo(PropertyId propertyId, ScriptContext* scriptContext, PolymorphicInlineCache *const clone);

#if DBG_DUMP
        void Dump();
#endif

        uint GetInlineCacheIndexForType(const Type * type) const
        {
            return (((size_t)type) >> PolymorphicInlineCacheShift) & (GetSize() - 1);
        }

    private:
        uint GetNextInlineCacheIndex(uint index) const
        {
            if (++index == GetSize())
            {
                index = 0;
            }
            return index;
        }

        template<bool CheckLocal, bool CheckProto, bool CheckAccessor>
        void CloneInlineCacheToEmptySlotInCollision(Type *const type, uint index);

#ifdef CLONE_INLINECACHE_TO_EMPTYSLOT
        template <typename TDelegate>
        bool CheckClonedInlineCache(uint inlineCacheIndex, TDelegate mapper);
#endif
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
        uint GetEntryCount()
        {
            uint count = 0;
            for (uint i = 0; i < size; ++i)
            {
                if (!inlineCaches[i].IsEmpty())
                {
                    count++;
                }
            }
            return count;
        }
#endif
    };

    class EquivalentTypeSet
    {
    private:
        Type** types;
        uint16 count;
        bool sortedAndDuplicatesRemoved;

    public:
        EquivalentTypeSet(Type** types, uint16 count)
            : types(types), count(count), sortedAndDuplicatesRemoved(false) {}

        uint16 GetCount() const
        {
            return this->count;
        }

        Type* GetFirstType() const
        {
            return GetType(0);
        }

        Type* GetType(uint16 index) const
        {
            Assert(this->types != nullptr && this->count > 0 && index < this->count);
            return this->types[index];
        }

        bool GetSortedAndDuplicatesRemoved() const
        {
            return this->sortedAndDuplicatesRemoved;
        }
        bool Contains(const Js::Type * type, uint16 * pIndex = nullptr) const;

        static bool AreIdentical(EquivalentTypeSet * left, EquivalentTypeSet * right);
        static bool IsSubsetOf(EquivalentTypeSet * left, EquivalentTypeSet * right);
        void SortAndRemoveDuplicates();
    };

#define MaxCachedSlotCount 65535

    struct ConstructorCache sealed : public ObjectCreationSiteInfo
    {
    private:
        typedef ObjectCreationSiteInfo Base;

        friend class JavascriptFunction;

    private:
        ScriptContext* scriptContext;

        // We cache only types whose slotCount < 64K to ensure the slotCount field doesn't look like a pointer to the recycler.
        int slotCount;

        // This layout (i.e. one-byte bit fields first, then the one-byte updateAfterCtor, and then the two byte inlineSlotCount) is
        // chosen intentionally to make sure the whole four bytes never look like a pointer and create a false reference pinning something
        // in recycler heap. The _tag bit is always set to prevent it looking like a pointer.
        bool _tag : 1;
        bool ctorHasNoExplicitReturnValue : 1;
        bool skipDefaultNewObject : 1;
        // This field indicates that the type stored in this cache is the final type after constructor. 
        bool typeIsFinal : 1;
        // This field indicates that the constructor cache has been invalidated due to a constructor's prototype property change.
        // We use this flag to determine if we should mark the cache as polymorphic and not attempt subsequent optimizations.
        // The cache may also be invalidated due to some property change (e.g. in proto chain),
        // in which case we won't deem the cache polymorphic.
        bool hasPrototypeChanged : 1;

        uint8 callCount;

        // Separate from the bit field below for convenient compare from the JIT-ed code. Doesn't currently increase the size.
        // If size becomes an issue, we could merge back into the bit field and use a TEST instead of CMP.
        bool updateAfterCtor;

        int16 inlineSlotCount;

    public:
        static ConstructorCache DefaultInstance;

    public:
        ConstructorCache()
        {
            this->scriptContext = nullptr;
            this->slotCount = 0;
            this->inlineSlotCount = 0;
            this->updateAfterCtor = false;
            this->ctorHasNoExplicitReturnValue = false;
            this->skipDefaultNewObject = false;
            this->_tag = true;
            this->typeIsFinal = false;
            this->hasPrototypeChanged = false;
            this->callCount = 0;
            Assert(IsConsistent());
        }

        ConstructorCache(ConstructorCache const * other) : Base(*other)
        {
            Assert(other != nullptr);
            this->scriptContext = other->scriptContext;
            this->slotCount = other->slotCount;
            this->inlineSlotCount = other->inlineSlotCount;
            this->updateAfterCtor = other->updateAfterCtor;
            this->ctorHasNoExplicitReturnValue = other->ctorHasNoExplicitReturnValue;
            this->skipDefaultNewObject = other->skipDefaultNewObject;
            this->_tag = true;
            this->typeIsFinal = other->typeIsFinal;
            this->hasPrototypeChanged = other->hasPrototypeChanged;
            this->callCount = other->callCount;
            Assert(IsConsistent());
        }

        void Populate(DynamicType* type, ScriptContext* scriptContext, bool ctorHasNoExplicitReturnValue, bool updateAfterCtor)
        {
            Assert(scriptContext == type->GetScriptContext());
            Assert(type->GetIsShared());
            Assert(IsConsistent());
            Assert(!GetType() || this->hasPrototypeChanged);
            Assert(type->GetTypeHandler()->GetSlotCapacity() <= MaxCachedSlotCount);
            SetType(type, true);
            this->scriptContext = scriptContext;
            this->slotCount = type->GetTypeHandler()->GetSlotCapacity();
            this->inlineSlotCount = type->GetTypeHandler()->GetInlineSlotCapacity();
            this->ctorHasNoExplicitReturnValue = ctorHasNoExplicitReturnValue;
            this->updateAfterCtor = updateAfterCtor;
            Assert(IsConsistent());
        }

        void PopulateForSkipDefaultNewObject(ScriptContext* scriptContext)
        {
            Assert(IsConsistent());
            Assert(!GetType());
            this->scriptContext = scriptContext;
            this->skipDefaultNewObject = true;
            Assert(IsConsistent());
        }

        void UpdateAfterConstructor(DynamicType* type, ScriptContext* scriptContext);

        void UpdateInlineSlotCount()
        {
            Assert(IsConsistent());
            DynamicType* type = GetType();
            Assert(type);
            DynamicTypeHandler* typeHandler = type->GetTypeHandler();
            // Inline slot capacity should never grow as a result of shrinking.
            Assert(typeHandler->GetInlineSlotCapacity() <= this->inlineSlotCount);
            // Slot capacity should never grow as a result of shrinking.
            Assert(typeHandler->GetSlotCapacity() <= this->slotCount);
            this->slotCount = typeHandler->GetSlotCapacity();
            this->inlineSlotCount = typeHandler->GetInlineSlotCapacity();
            Assert(IsConsistent());
        }

        ScriptContext* GetScriptContext() const
        {
            return this->scriptContext;
        }

        int GetSlotCount() const
        {
            return this->slotCount;
        }

        int16 GetInlineSlotCount() const
        {
            return this->inlineSlotCount;
        }

        static bool IsDefault(const ConstructorCache* constructorCache)
        {
            return constructorCache == &ConstructorCache::DefaultInstance;
        }

        bool IsDefault() const
        {
            return IsDefault(this);
        }

        bool HasPrototypeChanged() const
        {
            return this->hasPrototypeChanged;
        }

        bool IsPolymorphic() const
        {
            return HasPrototypeChanged();
        }

        bool GetCtorHasNoExplicitReturnValue() const
        {
            return this->ctorHasNoExplicitReturnValue;
        }

        bool GetUpdateCacheAfterCtor() const
        {
            return this->updateAfterCtor;
        }

        uint8 CallCount() const
        {
            return callCount;
        }

        void IncCallCount()
        {
            ++callCount;
            Assert(callCount != 0);
        }

        bool NeedsUpdateAfterCtor() const
        {
            return this->updateAfterCtor;
        }

        bool SkipDefaultNewObject() const
        {
            return this->skipDefaultNewObject;
        }

        bool TypeIsFinal() const
        {
            return typeIsFinal;
        }

        bool IsPopulated() const
        {
            return SkipDefaultNewObject() || IsPolymorphic() || GetType() && !NeedsUpdateAfterCtor();
        }

        bool IsSetUpForJit() const
        {
            return SkipDefaultNewObject() || !IsPolymorphic() && GetType() && !NeedsUpdateAfterCtor();
        }

        void ClearUpdateAfterCtor()
        {
            Assert(IsConsistent());
            Assert(GetType());
            Assert(this->updateAfterCtor);
            this->updateAfterCtor = false;
            Assert(IsConsistent());
        }

        static ConstructorCache* EnsureValidInstance(ConstructorCache* currentCache, ScriptContext* scriptContext);

        static uint32 GetOffsetOfUpdateAfterCtor()
        {
            return offsetof(ConstructorCache, updateAfterCtor);
        }

#if DBG
        bool IsConsistent() const
        {
            return
                SkipDefaultNewObject()
                    ?   !GetType() &&
                        !NeedsUpdateAfterCtor() &&
                        !HasPrototypeChanged() &&
                        GetSlotCount() == 0 &&
                        GetInlineSlotCount() == 0
                    :   !NeedsUpdateAfterCtor() || GetType();
        }
#endif

#if DBG_DUMP
        void Dump() const;
#endif

    protected:
        virtual void Unregister() override;

    private:
        void InvalidateOnPrototypeChange()
        {
            Assert(IsConsistent());

            if(!GetType())
            {
                return;
            }

            Assert(!IsDefault());
            Assert(!SkipDefaultNewObject());

            ClearType();
            this->hasPrototypeChanged = true;

            Assert(IsConsistent());
        }
    };

    // Caches the result of an instanceof operator over a type and a function
    struct IsInstInlineCache
    {
        Type * type;                    // The type of object operand an inline cache caches a result for
        JavascriptFunction * function;  // The function operand an inline cache caches a result for
        JavascriptBoolean * result;     // The result of doing (object instanceof function) where object->type == this->type
        IsInstInlineCache * next;       // Used to link together caches that have the same function operand

    public:
        bool IsEmpty() const { return type == nullptr; }
        bool TryGetResult(Var instance, JavascriptFunction * function, JavascriptBoolean ** result);
        void Cache(Type * instanceType, JavascriptFunction * function, JavascriptBoolean * result, ScriptContext * scriptContext);

    private:
        void Set(Type * instanceType, JavascriptFunction * function, JavascriptBoolean * result);
        void Clear();
        void Unregister(ScriptContext * scriptContext);
    };

#if defined(_M_IX86_OR_ARM32)
    CompileAssert(sizeof(IsInstInlineCache) == 0x10);
#else
    CompileAssert(sizeof(IsInstInlineCache) == 0x20);
#endif

    CompileAssert(sizeof(IsInstInlineCache) == sizeof(IsInstInlineCacheAllocator::CacheLayout));
}
