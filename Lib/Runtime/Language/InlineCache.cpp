//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeLanguagePch.h"

namespace Js
{
    void InlineCache::CacheLocal(
        Type *const type,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const ObjectSlotType slotType,
        const bool isInlineSlot,
        Type *const typeWithoutProperty,
        int requiredAuxSlotCapacity,
        ScriptContext *const requestContext)
    {
        Assert(type);
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyIndex != Constants::NoSlot);
        Assert(requestContext);
        Assert(type->GetScriptContext() == requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));
        Assert(requiredAuxSlotCapacity >= 0 && requiredAuxSlotCapacity < 0x01 << RequiredAuxSlotCapacityBitCount);
        // Store field and load field caches are never shared so we should never have a prototype cache morphing into an add property cache.
        // We may, however, have a flags cache (setter) change to add property cache.
        Assert(typeWithoutProperty == nullptr || !IsProto());

        requestContext->RegisterAsScriptContextWithInlineCaches();

        // Add cache into a store field cache list if required, but not there yet.
        if (typeWithoutProperty != nullptr)
        {
            // Note, this can throw due to OOM, so we need to do it before the inline cache is set below.
            RegisterStoreFieldInlineCache(requestContext, propertyId);
        }

        u.local.type = TypeWithAllTags(type, slotType, isInlineSlot);
        u.local.typeWithoutProperty =
            typeWithoutProperty ? TypeWithAllTags(typeWithoutProperty, slotType, isInlineSlot) : nullptr;
        u.local.isLocal = true;
        u.local.slotIndex = propertyIndex;
        u.local.requiredAuxSlotCapacity = requiredAuxSlotCapacity;

        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            Output::Print(L"IC::CacheLocal, %s: ", requestContext->GetPropertyName(propertyId)->GetBuffer());
            Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    void InlineCache::CacheProto(
        DynamicObject *const prototypeObjectWithProperty,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        const bool isMissing,
        Type *const type,
        ScriptContext *const requestContext)
    {
        Assert(prototypeObjectWithProperty);
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyIndex != Constants::NoSlot);
        Assert(type);
        Assert(requestContext);
        Assert(prototypeObjectWithProperty->GetScriptContext() == requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

        // This is an interesting quirk.  In the browser Chakra's global object cannot be used directly as a prototype, because
        // the global object (referenced either as window or as this) always points to the host object.  Thus, when we retrieve
        // a property from Chakra's global object the prototypeObjectWithProperty != info->GetInstance() and we will never cache
        // such property loads (see CacheOperators::CachePropertyRead).  However, in jc.exe or jshost.exe the only global object
        // is Chakra's global object, and so prototypeObjectWithProperty == info->GetInstance() and we can cache.  Hence, the
        // assert below is only correct when running in the browser.
        // Assert(prototypeObjectWithProperty != prototypeObjectWithProperty->type->GetLibrary()->GetGlobalObject());

        // Store field and load field caches are never shared so we should never have an add property cache morphing into a prototype cache.
        Assert(!IsLocal() || u.local.typeWithoutProperty == nullptr);

        requestContext->RegisterAsScriptContextWithInlineCaches();

        // Add cache into a proto cache list if not there yet.
        // Note, this can throw due to OOM, so we need to do it before the inline cache is set below.
        RegisterProtoInlineCache(requestContext, propertyId);

        u.proto.prototypeObject = prototypeObjectWithProperty;
        u.proto.isProto = true;
        u.proto.isMissing = isMissing;
        u.proto.slotIndex = propertyIndex;
        if (isInlineSlot)
        {
            u.proto.type = type;
        }
        else
        {
            u.proto.type = TypeWithAuxSlotTag(type);
        }
        Assert(GetSlotType(u.proto.type).IsVar());

        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));
        Assert(u.proto.isMissing == (uint16)(u.proto.prototypeObject == requestContext->GetLibrary()->GetMissingPropertyHolder()));

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            Output::Print(L"IC::CacheProto, %s: ", requestContext->GetPropertyName(propertyId)->GetBuffer());
            Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    // TODO (InlineCacheCleanup): When simplifying inline caches due to not sharing between loads and stores, create two
    // separate methods CacheSetter and CacheGetter.
    void InlineCache::CacheAccessor(
        const bool isGetter,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        Type *const type,
        DynamicObject *const object,
        const bool isOnProto,
        ScriptContext *const requestContext)
    {
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyIndex != Constants::NoSlot);
        Assert(type);
        Assert(object);
        Assert(requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));
        // It is possible that prototype is from a different scriptContext than the original instance. We don't want to cache
        // in this case.
        Assert(type->GetScriptContext() == requestContext);

        requestContext->RegisterAsScriptContextWithInlineCaches();

        if (isOnProto)
        {
            // Note, this can throw due to OOM, so we need to do it before the inline cache is set below.
            if (!isGetter)
            {
                // If the setter is on a prototype, this cache must be invalidated whenever proto
                // caches are invalidated, so we must register it here.  Note that store field inline
                // caches are invalidated any time proto caches are invalidated.
                RegisterStoreFieldInlineCache(requestContext, propertyId);
            }
            else
            {
                RegisterProtoInlineCache(requestContext, propertyId);
            }
        }

        u.accessor.isAccessor = true;
        // TODO (PersistentInlineCaches): Consider removing the flag altogether and just have a bit indicating
        // whether the cache itself is a store field cache (isStore?).
        u.accessor.flags = isGetter ? InlineCacheGetterFlag : InlineCacheSetterFlag;
        u.accessor.isOnProto = isOnProto;
        u.accessor.type = isInlineSlot ? type : TypeWithAuxSlotTag(type);
        Assert(GetSlotType(u.accessor.type).IsVar());
        u.accessor.slotIndex = propertyIndex;
        u.accessor.object = object;

        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::InlineCachePhase))
        {
            Output::Print(L"IC::CacheAccessor, %s: ", requestContext->GetPropertyName(propertyId)->GetBuffer());
            Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
    }

    bool InlineCache::PretendTryGetProperty(Type *const type, PropertyCacheOperationInfo * operationInfo)
    {
        if (type == TypeWithoutAnyTags(u.local.type))
        {
            operationInfo->cacheType = CacheType_Local;
            operationInfo->slotLocation = TypeHasAuxSlotTag(u.local.type) ? SlotLocation_Aux : SlotLocation_Inline;
            operationInfo->slotType = GetSlotType(u.local.type);
            return true;
        }

        if (type == TypeWithoutAnyTags(u.proto.type))
        {
            Assert(GetSlotType(u.proto.type).IsVar());
            operationInfo->cacheType = CacheType_Proto;
            operationInfo->slotLocation = TypeHasAuxSlotTag(u.proto.type) ? SlotLocation_Aux : SlotLocation_Inline;
            operationInfo->slotType = ObjectSlotType::GetVar();
            return true;
        }

        if (type == TypeWithoutAnyTags(u.accessor.type))
        {
            Assert(GetSlotType(u.accessor.type).IsVar());
            Assert(u.accessor.flags & InlineCacheGetterFlag);

            operationInfo->cacheType = CacheType_Getter;
            operationInfo->slotLocation = TypeHasAuxSlotTag(u.accessor.type) ? SlotLocation_Aux : SlotLocation_Inline;
            operationInfo->slotType = ObjectSlotType::GetVar();
            return true;
        }

        return false;
    }

    bool InlineCache::GetGetterSetter(Type *const type, RecyclableObject **callee)
    {
        Type *const taggedType = TypeWithAuxSlotTag(type);
        *callee = nullptr;

        if (u.accessor.flags & (InlineCacheGetterFlag | InlineCacheSetterFlag))
        {
            if (type == u.accessor.type)
            {
                *callee = RecyclableObject::FromVar(u.accessor.object->GetInlineSlot(u.accessor.slotIndex, ObjectSlotType::GetVar()));
                return true;
            }
            else if (taggedType == u.accessor.type)
            {
                *callee = RecyclableObject::FromVar(u.accessor.object->GetAuxSlot(u.accessor.slotIndex, ObjectSlotType::GetVar()));
                return true;
            }
        }
        return false;
    }

    bool InlineCache::GetCallApplyTarget(RecyclableObject* obj, RecyclableObject **callee)
    {
        Type *const type = obj->GetType();
        Type *const taggedType = TypeWithAuxSlotTag(type);
        *callee = nullptr;

        if (IsLocal())
        {
            if (type == u.local.type)
            {
                const Var objectAtInlineSlot = DynamicObject::FromVar(obj)->GetInlineSlot(u.local.slotIndex, ObjectSlotType::GetVar());
                if (!Js::TaggedNumber::Is(objectAtInlineSlot))
                {
                    *callee = RecyclableObject::FromVar(objectAtInlineSlot);
                    return true;
                }
            }
            else if (taggedType == u.local.type)
            {
                const Var objectAtAuxSlot = DynamicObject::FromVar(obj)->GetAuxSlot(u.local.slotIndex, ObjectSlotType::GetVar());
                if (!Js::TaggedNumber::Is(objectAtAuxSlot))
                {
                    *callee = RecyclableObject::FromVar(objectAtAuxSlot);
                    return true;
                }
            }
            return false;
        }
        else if (IsProto())
        {
            if (type == u.proto.type)
            {
                const Var objectAtInlineSlot = u.proto.prototypeObject->GetInlineSlot(u.proto.slotIndex, ObjectSlotType::GetVar());
                if (!Js::TaggedNumber::Is(objectAtInlineSlot))
                {
                    *callee = RecyclableObject::FromVar(objectAtInlineSlot);
                    return true;
                }
            }
            else if (taggedType == u.proto.type)
            {
                const Var objectAtAuxSlot = u.proto.prototypeObject->GetAuxSlot(u.proto.slotIndex, ObjectSlotType::GetVar());
                if (!Js::TaggedNumber::Is(objectAtAuxSlot))
                {
                    *callee = RecyclableObject::FromVar(objectAtAuxSlot);
                    return true;
                }
            }
            return false;
        }
        return false;
    }

    void InlineCache::Clear()
    {
        // Preserve the slot type 
        const ObjectSlotType slotType = GetSlotType();

        // IsEmpty() is a quick check to see that the cache is not populated, it only checks u.local.type, which does not
        // guarantee that the proto or flags cache would not hit. So Clear() must still clear everything.
        u.local.type = nullptr;
        u.proto.type = nullptr;
        u.accessor.type = nullptr;
        RemoveFromInvalidationList();

        SetSlotTypeForEmptyCache(slotType);
    }

    InlineCache *InlineCache::Clone(Js::PropertyId propertyId, ScriptContext* scriptContext)
    {
        Assert(scriptContext);

        InlineCacheAllocator* allocator = scriptContext->GetInlineCacheAllocator();
        // Important to zero the allocated cache to be sure CopyTo doesn't see garbage
        // when it uses the next pointer.
        InlineCache* clone = AllocatorNewZ(InlineCacheAllocator, allocator, InlineCache);
        CopyTo(propertyId, scriptContext, clone);
        return clone;
    }

    bool InlineCache::TryGetFixedMethodFromCache(Js::FunctionBody* functionBody, uint cacheId, Js::JavascriptFunction** pFixedMethod)
    {
        Assert(pFixedMethod);

        if (IsEmpty())
        {
            return false;
        }
        Js::Type * propertyOwnerType = nullptr;
        bool isLocal = IsLocal();
        bool isProto = IsProto();
        if (isLocal)
        {
            propertyOwnerType = TypeWithoutAnyTags(this->u.local.type);
        }
        else if (isProto)
        {
            // TODO (InlineCacheCleanup): For loads from proto, we could at least grab the value from protoObject's slot
            // (given by the cache) and see if its a function. Only then, does it make sense to check with the type handler.
            propertyOwnerType = this->u.proto.prototypeObject->GetType();
        }
        else
        {
            propertyOwnerType = this->u.accessor.object->GetType();
        }

        Assert(propertyOwnerType != nullptr);

        if (Js::DynamicType::Is(propertyOwnerType->GetTypeId()))
        {
            Js::DynamicTypeHandler* propertyOwnerTypeHandler = ((Js::DynamicType*)propertyOwnerType)->GetTypeHandler();
            Js::PropertyId propertyId = functionBody->GetPropertyIdFromCacheId(cacheId);
            Js::PropertyRecord const * const methodPropertyRecord = functionBody->GetScriptContext()->GetPropertyName(propertyId);

            Var fixedMethod = nullptr;
            bool isUseFixedProperty;
            if (isLocal || isProto)
            {
                isUseFixedProperty = propertyOwnerTypeHandler->TryUseFixedProperty(methodPropertyRecord, &fixedMethod, Js::FixedPropertyKind::FixedMethodProperty, functionBody->GetScriptContext());
            }
            else
            {
                isUseFixedProperty = propertyOwnerTypeHandler->TryUseFixedAccessor(methodPropertyRecord, &fixedMethod, Js::FixedPropertyKind::FixedAccessorProperty, this->IsGetterAccessor(), functionBody->GetScriptContext());
            }
            AssertMsg(fixedMethod == nullptr || Js::JavascriptFunction::Is(fixedMethod), "The fixed value should have been a Method !!!");
            *pFixedMethod = reinterpret_cast<JavascriptFunction*>(fixedMethod);
            return isUseFixedProperty;
        }

        return false;
    }

    void InlineCache::CopyTo(PropertyId propertyId, ScriptContext * scriptContext, InlineCache * const clone)
    {
        DebugOnly(VerifyRegistrationForInvalidation(this, scriptContext, propertyId));
        DebugOnly(VerifyRegistrationForInvalidation(clone, scriptContext, propertyId));
        Assert(clone != nullptr);

        clone->RemoveFromInvalidationList();

        // Note, the Register methods can throw due to OOM, so we need to do it before the inline cache is copied below.
        if (this->invalidationListSlotPtr != nullptr)
        {
            if (this->NeedsToBeRegisteredForProtoInvalidation())
            {
                clone->RegisterProtoInlineCache(scriptContext, propertyId);
            }
            else if (this->NeedsToBeRegisteredForStoreFieldInvalidation())
            {
                clone->RegisterStoreFieldInlineCache(scriptContext, propertyId);
            }
        }

        clone->u = this->u;
        
        DebugOnly(VerifyRegistrationForInvalidation(clone, scriptContext, propertyId));
    }   

    Type *InlineCacheTypeTagger::TypeWithAllTags(const Type *const type, const ObjectSlotType slotType, const bool isInlineSlot)
    {
        Assert(!TypeHasAuxSlotTag(type));

        Type *taggedType = TypeWithSlotType(type, slotType);
        if(!isInlineSlot)
            taggedType = TypeWithAuxSlotTag(taggedType);
        return taggedType;
    }

    Type *InlineCacheTypeTagger::TypeWithoutAnyTags(const Type *const type)
    {
        return
            reinterpret_cast<Type *>(
                reinterpret_cast<size_t>(type) & ~(InlineCacheAuxSlotTypeTag | InlineCacheSlotTypeMask));
    }

    bool InlineCacheTypeTagger::TypeHasAuxSlotTag(const Type *const type)
    {
        return !!(reinterpret_cast<size_t>(type) & InlineCacheAuxSlotTypeTag);
    }

    Type *InlineCacheTypeTagger::TypeWithAuxSlotTag(const Type *const type)
    {
        Assert(!TypeHasAuxSlotTag(type));
        return reinterpret_cast<Type *>(reinterpret_cast<size_t>(type) | InlineCacheAuxSlotTypeTag);
    }

    ObjectSlotType InlineCacheTypeTagger::GetSlotType(const Type *const type)
    {
        CompileAssert(
            (InlineCacheSlotTypeMask >> InlineCacheSlotTypeShift) ==
            ((static_cast<size_t>(1) << ObjectSlotType::BitSize) - 1));
        CompileAssert(!(InlineCacheAuxSlotTypeTag & InlineCacheSlotTypeMask));

        return ObjectSlotType((reinterpret_cast<size_t>(type) & InlineCacheSlotTypeMask) >> InlineCacheSlotTypeShift);
    }

    Type *InlineCacheTypeTagger::TypeWithSlotType(const Type *const type, const ObjectSlotType slotType)
    {
        Assert(static_cast<ObjectSlotType::TSize>(GetSlotType(type)) == 0);
        Assert(GetSlotType(type) == ObjectSlotType::GetVar());

        if(slotType.IsVar())
            return const_cast<Type *>(type);
        return
            reinterpret_cast<Type *>(
                reinterpret_cast<size_t>(type) | (static_cast<size_t>(slotType) << InlineCacheSlotTypeShift));
    }

    Type *InlineCacheTypeTagger::TypeWithoutSlotType(const Type *const type)
    {
        return reinterpret_cast<Type *>(reinterpret_cast<size_t>(type) & ~InlineCacheSlotTypeMask);
    }

    ObjectSlotType InlineCache::GetSlotType() const
    {
        if(IsLocal())
        {
            Assert(!IsEmpty());
            return GetSlotType(u.local.type);
        }

        return IsEmpty() ? GetSlotTypeForEmptyCache() : ObjectSlotType::GetVar();
    }

    void InlineCache::InitializeSlotType(FunctionBody *const functionBody, const InlineCacheIndex cacheIndex)
    {
        Assert(functionBody);
        Assert(cacheIndex < functionBody->GetInlineCacheCount());

#if ENABLE_NATIVE_CODEGEN       
        if(!IsEmpty() || HasSlotTypeForEmptyCache() || !functionBody->HasDynamicProfileInfo())
        {
            return;
        }

        ObjectSlotType slotType = functionBody->GetAnyDynamicProfileInfo()->GetFldInfo(functionBody, cacheIndex)->slotType;
        PolymorphicInlineCache *const polymorphicInlineCache = functionBody->GetPolymorphicInlineCache(cacheIndex);
        if(polymorphicInlineCache)
        {
            slotType = slotType.MergeValueType(polymorphicInlineCache->GetSlotType());
            polymorphicInlineCache->SetSlotType(slotType);
        }
        SetSlotTypeForEmptyCache(slotType);
#else
        return;
#endif
    }

    bool InlineCache::HasSlotTypeForEmptyCache() const
    {
        Assert(IsEmpty());

        // See SetSlotTypeForEmptyCache()
        return static_cast<ObjectSlotType::TSize>(u.local.requiredAuxSlotCapacity) != 0;
    }

    ObjectSlotType InlineCache::GetSlotTypeForEmptyCache() const
    {
        Assert(IsEmpty());

        // See SetSlotTypeForEmptyCache()
        const ObjectSlotType::TSize slotTypeValue = static_cast<ObjectSlotType::TSize>(u.local.requiredAuxSlotCapacity);
        return slotTypeValue == 0 ? ObjectSlotType::GetInt() : ObjectSlotType(slotTypeValue - 1);
    }

    void InlineCache::SetSlotTypeForEmptyCache(const ObjectSlotType slotType)
    {
        Assert(!u.local.type);
        Assert(!u.proto.type);
        Assert(!u.accessor.type);
        Assert(slotType == slotType.ToNormalizedValueType());

        // We want an originally empty (zerored) cache to allow the most aggressive slot type initially (Int), but invalidated
        // caches that call this function to allow only the specified slot type or a more conservative slot type such that
        // invalidation does not lose the slot type convergence that has taken place so far. To tell the difference, add one to
        // the slot type value. Preserve the slot type in u.local.requiredAuxSlotCapacity in empty caches.
        u.local.requiredAuxSlotCapacity = static_cast<ObjectSlotType::TSize>(slotType) + 1;
    }

    bool InlineCache::NeedsToBeRegisteredForProtoInvalidation() const
    {
        return (IsProto() || IsGetterAccessorOnProto());
    }

    bool InlineCache::NeedsToBeRegisteredForStoreFieldInvalidation() const
    {
        return (IsLocal() && this->u.local.typeWithoutProperty != nullptr) || IsSetterAccessorOnProto();
    }

    void InlineCache::RegisterStoreFieldInlineCache(ScriptContext *const requestContext, const PropertyId propertyId)
    {
        if(invalidationListSlotPtr)
        {
            if(NeedsToBeRegisteredForStoreFieldInvalidation())
                return;
            RemoveFromInvalidationList();
        }
        requestContext->RegisterStoreFieldInlineCache(this, propertyId);
    }

    void InlineCache::RegisterProtoInlineCache(ScriptContext *const requestContext, const PropertyId propertyId)
    {
        if(invalidationListSlotPtr)
        {
            if(NeedsToBeRegisteredForProtoInvalidation())
                return;
            RemoveFromInvalidationList();
        }
        requestContext->RegisterProtoInlineCache(this, propertyId);
    }

#if DEBUG
    bool InlineCache::NeedsToBeRegisteredForInvalidation() const
    {
        int howManyInvalidationsNeeded =
            (int)NeedsToBeRegisteredForProtoInvalidation() +
            (int)NeedsToBeRegisteredForStoreFieldInvalidation();
        Assert(howManyInvalidationsNeeded <= 1);
        return howManyInvalidationsNeeded > 0;
    }

    void InlineCache::VerifyRegistrationForInvalidation(const InlineCache* cache, ScriptContext* scriptContext, Js::PropertyId propertyId)
    {
        bool needsProtoInvalidation = cache->NeedsToBeRegisteredForProtoInvalidation();
        bool needsStoreFieldInvalidation = cache->NeedsToBeRegisteredForStoreFieldInvalidation();
        int howManyInvalidationsNeeded = (int)needsProtoInvalidation + (int)needsStoreFieldInvalidation;
        bool hasListSlotPtr = cache->invalidationListSlotPtr != nullptr;
        bool isProtoRegistered = hasListSlotPtr ? scriptContext->GetThreadContext()->IsProtoInlineCacheRegistered(cache, propertyId) : false;
        bool isStoreFieldRegistered = hasListSlotPtr ? scriptContext->GetThreadContext()->IsStoreFieldInlineCacheRegistered(cache, propertyId) : false;
        int howManyRegistrations = (int)isProtoRegistered + (int)isStoreFieldRegistered;

        Assert(howManyInvalidationsNeeded <= 1);
        Assert((howManyInvalidationsNeeded == 0) || hasListSlotPtr);
        Assert(!needsProtoInvalidation || isProtoRegistered);
        Assert(!needsStoreFieldInvalidation || isStoreFieldRegistered);
        Assert(!hasListSlotPtr || howManyRegistrations > 0);
        Assert(!hasListSlotPtr || (*cache->invalidationListSlotPtr) == cache);
    }

    // Confirm inline cache miss against instance property lookup info.
    bool InlineCache::ConfirmCacheMiss(const Type * oldType, const PropertyValueInfo* info) const
    {
        return u.local.type != oldType
            && u.proto.type != oldType
            && (u.accessor.type != oldType || info == NULL || u.accessor.flags != info->GetFlags());
    }
#endif

#if DBG_DUMP
    void InlineCache::Dump()
    {
        if (this->u.local.isLocal)
        {
            Output::Print(L"LOCAL { types: 0x%X -> 0x%X, slot = %d, list slot ptr = 0x%X }",
                this->u.local.typeWithoutProperty,
                this->u.local.type,
                this->u.local.slotIndex,
                this->invalidationListSlotPtr
                );
        }
        else if (this->u.proto.isProto)
        {
            Output::Print(L"PROTO { type = 0x%X, prototype = 0x%X, slot = %d, list slot ptr = 0x%X }",
                this->u.proto.type,
                this->u.proto.prototypeObject,
                this->u.proto.slotIndex,
                this->invalidationListSlotPtr
                );
        }
        else if (this->u.accessor.isAccessor)
        {
            Output::Print(L"FLAGS { type = 0x%X, object = 0x%X, flag = 0x%X, slot = %d, list slot ptr = 0x%X }",
                this->u.accessor.type,
                this->u.accessor.object,
                this->u.accessor.slotIndex,
                this->u.accessor.flags,
                this->invalidationListSlotPtr
                );
        }
        else
        {
            Assert(this->u.accessor.type == 0);
            Assert(this->u.accessor.slotIndex == 0);
            Output::Print(L"uninitialized");
        }
    }

#endif

    PolymorphicInlineCache * PolymorphicInlineCache::New(uint16 size, FunctionBody * functionBody, ObjectSlotType slotType)
    {
        ScriptContext * scriptContext = functionBody->GetScriptContext();
        InlineCache * inlineCaches = AllocatorNewArrayZ(InlineCacheAllocator, scriptContext->GetInlineCacheAllocator(), InlineCache, size);
#ifdef POLY_INLINE_CACHE_SIZE_STATS
        scriptContext->GetInlineCacheAllocator()->LogPolyCacheAlloc(size * sizeof(InlineCache));
#endif
        PolymorphicInlineCache * polymorphicInlineCache = RecyclerNewFinalizedLeaf(scriptContext->GetRecycler(), PolymorphicInlineCache, inlineCaches, size, functionBody, slotType);

        // Insert the cache into finalization list.  We maintain this linked list of polymorphic caches because when we allocate
        // a larger cache, the old one might still be used by some code on the stack.  Consequently, we can't release
        // the inline cache array back to the arena allocator.  The list is leaf-allocated and so does not keep the
        // old caches alive.  As soon as they are collectible, their finalizer releases the inline cache array to the arena.
        polymorphicInlineCache->prev = nullptr;
        polymorphicInlineCache->next = functionBody->GetPolymorphicInlineCachesHead();
        if (polymorphicInlineCache->next)
        {
            polymorphicInlineCache->next->prev = polymorphicInlineCache;
        }
        functionBody->SetPolymorphicInlineCachesHead(polymorphicInlineCache);

        return polymorphicInlineCache;
    }

    void PolymorphicInlineCache::UpdateInlineCachesFillInfo(uint index, bool set)
    {
        Assert(index < 0x20);
        if (set)
        {
            this->inlineCachesFillInfo |= 1 << index;
        }
        else
        {
            this->inlineCachesFillInfo &= ~(1 << index);
        }
    }

    bool PolymorphicInlineCache::IsFull()
    {
        Assert(this->size <= 0x20);
        return this->inlineCachesFillInfo == ((1 << (this->size - 1)) << 1) - 1;
    }

    void PolymorphicInlineCache::CacheLocal(
        Type *const type,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const ObjectSlotType slotType,
        const bool isInlineSlot,
        Type *const typeWithoutProperty,
        int requiredAuxSlotCapacity,
        ScriptContext *const requestContext)
    {
        Assert(slotType.IsValueTypeEqualTo(GetSlotType()));

        // Let's not waste polymorphic cache slots by caching both the type without property and type with property. If the
        // cache is used for both adding a property and setting the existing property, then those instances will cause both
        // types to be cached. Until then, caching both types proactively here can unnecessarily trash useful cached info
        // because the types use different slots, unlike a monomorphic inline cache.
        if (!typeWithoutProperty)
        {
            uint inlineCacheIndex = GetInlineCacheIndexForType(type);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
            bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
            if (!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
            {
                if (!inlineCaches[inlineCacheIndex].IsEmpty() && !inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
                {
                    if (inlineCaches[inlineCacheIndex].IsLocal())
                    {
                        CloneInlineCacheToEmptySlotInCollision<true, false, false>(type, inlineCacheIndex);
                    }
                    else if (inlineCaches[inlineCacheIndex].IsProto())
                    {
                        CloneInlineCacheToEmptySlotInCollision<false, true, false>(type, inlineCacheIndex);
                    }
                    else
                    {
                        CloneInlineCacheToEmptySlotInCollision<false, false, true>(type, inlineCacheIndex);
                    }
                }
            }

            inlineCaches[inlineCacheIndex].CacheLocal(
                type, propertyId, propertyIndex, slotType, isInlineSlot, nullptr, requiredAuxSlotCapacity, requestContext);
            UpdateInlineCachesFillInfo(inlineCacheIndex, true /*set*/);

#if DBG_DUMP
            if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
            {
                Output::Print(L"PIC::CacheLocal, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
                inlineCaches[inlineCacheIndex].Dump();
                Output::Print(L"\n");
                Output::Flush();
            }
#endif
            PHASE_PRINT_INTRUSIVE_TESTTRACE1(
                Js::PolymorphicInlineCachePhase,
                L"TestTrace PIC: CacheLocal, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
        }
        else
        {
            uint inlineCacheIndex = GetInlineCacheIndexForType(typeWithoutProperty);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
            bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
            inlineCaches[inlineCacheIndex].CacheLocal(
                type, propertyId, propertyIndex, slotType, isInlineSlot, typeWithoutProperty, requiredAuxSlotCapacity, requestContext);
            UpdateInlineCachesFillInfo(inlineCacheIndex, true /*set*/);

#if DBG_DUMP
            if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
            {
                Output::Print(L"PIC::CacheLocal, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
                inlineCaches[inlineCacheIndex].Dump();
                Output::Print(L"\n");
                Output::Flush();
            }
#endif
            PHASE_PRINT_INTRUSIVE_TESTTRACE1(
                Js::PolymorphicInlineCachePhase,
                L"TestTrace PIC: CacheLocal, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
        }
    }

    void PolymorphicInlineCache::CacheProto(
        DynamicObject *const prototypeObjectWithProperty,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        const bool isMissing,
        Type *const type,
        ScriptContext *const requestContext)
    {
        Assert(GetSlotType().IsVar());

        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
        bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
        if (!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
        {
            if (!inlineCaches[inlineCacheIndex].IsEmpty() && !inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
            {
                if (inlineCaches[inlineCacheIndex].IsLocal())
                {
                    CloneInlineCacheToEmptySlotInCollision<true, false, false>(type, inlineCacheIndex);
                }
                else if (inlineCaches[inlineCacheIndex].IsProto())
                {
                    CloneInlineCacheToEmptySlotInCollision<false, true, false>(type, inlineCacheIndex);
                }
                else
                {
                    CloneInlineCacheToEmptySlotInCollision<false, false, true>(type, inlineCacheIndex);
                }
            }
        }

        inlineCaches[inlineCacheIndex].CacheProto(
            prototypeObjectWithProperty, propertyId, propertyIndex, isInlineSlot, isMissing, type, requestContext);
        UpdateInlineCachesFillInfo(inlineCacheIndex, true /*set*/);

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
        {
            Output::Print(L"PIC::CacheProto, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
            inlineCaches[inlineCacheIndex].Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
        PHASE_PRINT_INTRUSIVE_TESTTRACE1(
            Js::PolymorphicInlineCachePhase,
            L"TestTrace PIC: CacheProto, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
    }

    void PolymorphicInlineCache::CacheAccessor(
        const bool isGetter,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        Type *const type,
        DynamicObject *const object,
        const bool isOnProto,
        ScriptContext *const requestContext)
    {
        Assert(GetSlotType().IsVar());

        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
#if INTRUSIVE_TESTTRACE_PolymorphicInlineCache
        bool collision = !inlineCaches[inlineCacheIndex].IsEmpty();
#endif
        if (!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
        {
            if (!inlineCaches[inlineCacheIndex].IsEmpty() && !inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
            {
                if (inlineCaches[inlineCacheIndex].IsLocal())
                {
                    CloneInlineCacheToEmptySlotInCollision<true, false, false>(type, inlineCacheIndex);
                }
                else if (inlineCaches[inlineCacheIndex].IsProto())
                {
                    CloneInlineCacheToEmptySlotInCollision<false, true, false>(type, inlineCacheIndex);
                }
                else
                {
                    CloneInlineCacheToEmptySlotInCollision<false, false, true>(type, inlineCacheIndex);
                }
            }
        }

        inlineCaches[inlineCacheIndex].CacheAccessor(isGetter, propertyId, propertyIndex, isInlineSlot, type, object, isOnProto, requestContext);
        UpdateInlineCachesFillInfo(inlineCacheIndex, true /*set*/);

#if DBG_DUMP
        if (PHASE_VERBOSE_TRACE1(Js::PolymorphicInlineCachePhase))
        {
            Output::Print(L"PIC::CacheAccessor, %s, %d: ", requestContext->GetPropertyName(propertyId)->GetBuffer(), inlineCacheIndex);
            inlineCaches[inlineCacheIndex].Dump();
            Output::Print(L"\n");
            Output::Flush();
        }
#endif
        PHASE_PRINT_INTRUSIVE_TESTTRACE1(
            Js::PolymorphicInlineCachePhase,
            L"TestTrace PIC: CacheAccessor, 0x%x, entryIndex = %d, collision = %s, entries = %d\n", this, inlineCacheIndex, collision ? L"true" : L"false", GetEntryCount());
    }

    bool PolymorphicInlineCache::PretendTryGetProperty(
        Type *const type,
        PropertyCacheOperationInfo * operationInfo)
    {
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        return inlineCaches[inlineCacheIndex].PretendTryGetProperty(type, operationInfo);
    }

    bool PolymorphicInlineCache::PretendTrySetProperty(
        Type *const type,
        Type *const oldType,
        PropertyCacheOperationInfo * operationInfo)
    {
        return
            inlineCaches[GetInlineCacheIndexForType(type)].PretendTrySetProperty<true, false>(type, oldType, operationInfo) ||
            (
                oldType != type &&
                inlineCaches[GetInlineCacheIndexForType(oldType)].PretendTrySetProperty<false, true>(
                    type,
                    oldType,
                    operationInfo)
            );
    }

    void PolymorphicInlineCache::CopyTo(PropertyId propertyId, ScriptContext* scriptContext, PolymorphicInlineCache *const clone)
    {
        Assert(clone);

        clone->ignoreForEquivalentObjTypeSpec = this->ignoreForEquivalentObjTypeSpec;
        clone->cloneForJitTimeUse = this->cloneForJitTimeUse;

        for (uint i = 0; i < GetSize(); ++i)
        {
            Type * type = inlineCaches[i].GetType();
            if (type)
            {
                uint inlineCacheIndex = clone->GetInlineCacheIndexForType(inlineCaches[i].GetTypeForHashing());

                // When copying inline caches from one polymorphic cache to another, types are again hashed to get the corresponding indices in the new polymorphic cache.
                // This might lead to collision in the new cache. We need to try to resolve that collision.
                if (!PHASE_OFF1(Js::CloneCacheInCollisionPhase))
                {
                    if (!clone->inlineCaches[inlineCacheIndex].IsEmpty() && !clone->inlineCaches[inlineCacheIndex].NeedsToBeRegisteredForStoreFieldInvalidation())
                    {
                        if (clone->inlineCaches[inlineCacheIndex].IsLocal())
                        {
                            clone->CloneInlineCacheToEmptySlotInCollision<true, false, false>(type, inlineCacheIndex);
                        }
                        else if (clone->inlineCaches[inlineCacheIndex].IsProto())
                        {
                            clone->CloneInlineCacheToEmptySlotInCollision<false, true, false>(type, inlineCacheIndex);
                        }
                        else
                        {
                            clone->CloneInlineCacheToEmptySlotInCollision<false, false, true>(type, inlineCacheIndex);
                        }

                    }
                }
                inlineCaches[i].CopyTo(propertyId, scriptContext, &clone->inlineCaches[inlineCacheIndex]);
                clone->UpdateInlineCachesFillInfo(inlineCacheIndex, true /*set*/);
            }
        }
    }

#if DBG_DUMP
    void PolymorphicInlineCache::Dump()
    {
        for (uint i = 0; i < size; ++i)
        {
            if (!inlineCaches[i].IsEmpty())
            {
                Output::Print(L"  %d: ", i);
                inlineCaches[i].Dump();
                Output::Print(L"\n");
            }
        }
    }
#endif

    ObjectSlotType PolymorphicInlineCache::GetSlotType() const
    {
        return slotType;
    }

    void PolymorphicInlineCache::SetSlotType(ObjectSlotType slotType)
    {
        slotType = slotType.ToNormalizedValueType();
        if(this->slotType == slotType)
        {
            return;
        }

        Assert(slotType.IsValueTypeMoreConvervativeThan(this->slotType));
        this->slotType = slotType;

        // The slot type has become more conservative. Going forward, any types with more aggressive slot types will be
        // converted, so to prevent unnecessary collisions, clear the polymorhpic cache
        for(uint16 i = 0; i < size; ++i)
            inlineCaches[i].RemoveFromInvalidationList();
        memset(inlineCaches, 0, size * sizeof(InlineCache));
    }

    bool EquivalentTypeSet::Contains(const Js::Type * type, uint16* pIndex) const
    {
        for (uint16 ti = 0; ti < this->count; ti++)
        {
            if (this->types[ti] == type)
            {
                if (pIndex)
                {
                    *pIndex = ti;
                }
                return true;
            }
        }
        return false;
    }

    bool EquivalentTypeSet::AreIdentical(EquivalentTypeSet * left, EquivalentTypeSet * right)
    {
        if (!left->GetSortedAndDuplicatesRemoved())
        {
            left->SortAndRemoveDuplicates();
        }
        if (!right->GetSortedAndDuplicatesRemoved())
        {
            right->SortAndRemoveDuplicates();
        }

        Assert(left->GetSortedAndDuplicatesRemoved() && right->GetSortedAndDuplicatesRemoved());

        if (left->count != right->count)
        {
            return false;
        }

        if (memcmp(left->types, right->types, left->count * sizeof(Type*)) == 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    bool EquivalentTypeSet::IsSubsetOf(EquivalentTypeSet * left, EquivalentTypeSet * right)
    {
        if (!left->GetSortedAndDuplicatesRemoved())
        {
            left->SortAndRemoveDuplicates();
        }
        if (!right->GetSortedAndDuplicatesRemoved())
        {
            right->SortAndRemoveDuplicates();
        }

        if (left->count > right->count)
        {
            return false;
        }

        // Try to find each left type in the right set.
        int j = 0;
        for (int i = 0; i < left->count; i++)
        {
            bool found = false;
            for (; j < right->count; j++)
            {
                if (left->types[i] < right->types[j])
                {
                    // Didn't find the left type. Fail.
                    return false;
                }
                if (left->types[i] == right->types[j])
                {
                    // Found the left type. Continue to the next left/right pair.
                    found = true;
                    j++;
                    break;
                }
            }
            Assert(j <= right->count);
            if (j == right->count && !found)
            {
                // Exhausted the right set without finding the current left type.
                return false;
            }
        }
        return true;
    }

    void EquivalentTypeSet::SortAndRemoveDuplicates()
    {
        uint16 oldCount = this->count;
        uint16 i;

        // sorting
        for (i = 1; i < oldCount; i++)
        {
            uint16 j = i;
            while (j > 0 && (this->types[j - 1] > this->types[j]))
            {
                Type* tmp = this->types[j];
                this->types[j] = this->types[j - 1];
                this->types[j - 1] = tmp;
            }
        }

        // removing duplicate types from the sorted set
        i = 0;
        for (uint16 j = 1; j < oldCount; j++)
        {
            if (this->types[i] != this->types[j])
            {
                this->types[++i] = this->types[j];
            }
        }
        this->count = ++i;
        for (i; i < oldCount; i++)
        {
            this->types[i] = nullptr;
        }

        this->sortedAndDuplicatesRemoved = true;
    }

    ConstructorCache ConstructorCache::DefaultInstance;

    ConstructorCache* ConstructorCache::EnsureValidInstance(ConstructorCache* currentCache, ScriptContext* scriptContext)
    {
        Assert(currentCache != nullptr);

        ConstructorCache* newCache = currentCache;

        if (ConstructorCache::IsDefault(currentCache))
        {
            newCache = RecyclerNew(scriptContext->GetRecycler(), ConstructorCache);
        }

        return newCache;
    }

    void ConstructorCache::UpdateAfterConstructor(DynamicType* type, ScriptContext* scriptContext)
    {
        Assert(scriptContext == type->GetScriptContext());
        Assert(type->GetTypeHandler()->GetMayBecomeShared());
        Assert(type->GetIsShared());
        Assert(type->GetTypeHandler()->GetSlotCapacity() < Js::PropertyIndexRanges<PropertyIndex>::MaxValue);
        Assert(IsConsistent());
        Assert(GetType());
        Assert(this->scriptContext == scriptContext);
        Assert(this->ctorHasNoExplicitReturnValue);

        const bool updateAfterCtor = this->updateAfterCtor;
        ClearType();
        SetType(type, true);
        this->updateAfterCtor = updateAfterCtor;
        this->typeIsFinal = true;
        this->slotCount = type->GetTypeHandler()->GetSlotCapacity();
        this->inlineSlotCount = type->GetTypeHandler()->GetInlineSlotCapacity();

        DynamicTypeHandler *const typeHandler = type->GetTypeHandler();
        if(typeHandler->IsPathTypeHandler())
            PathTypeHandler::FromTypeHandler(typeHandler)->RegisterObjectCreationSiteInfo(this);

        ThreadContext *const threadContext = scriptContext->GetThreadContext();
        for(ObjectSlotIterator it(type); it.IsValid(); it.MoveNext())
            threadContext->RegisterConstructorCache(it.CurrentPropertyId(scriptContext), this);

        Assert(IsConsistent());
    }

    void ConstructorCache::Unregister()
    {
        Assert(GetType());

        Base::Unregister();

        updateAfterCtor = false;

        DynamicType *const type = GetType();
        ScriptContext *const scriptContext = type->GetScriptContext();
        ThreadContext *const threadContext = scriptContext->GetThreadContext();
        if(!threadContext->ShouldInvalidateConstructorCaches())
            return;
        for(ObjectSlotIterator it(type); it.IsValid(); it.MoveNext())
            threadContext->UnregisterConstructorCache(it.CurrentPropertyId(scriptContext), this);
    }

#if DBG_DUMP
    void ConstructorCache::Dump() const
    {
        Output::Print(L"type = 0x%p, script context = 0x%p, slots = %d, inline slots = %d, polymorphic = %d, update cache = %d, skip default = %d, no return = %d", 
            this->GetType(), this->GetScriptContext(), this->GetSlotCount(), this->GetInlineSlotCount(), 
            this->IsPolymorphic(), this->GetUpdateCacheAfterCtor(),
            this->SkipDefaultNewObject(), this->GetCtorHasNoExplicitReturnValue());
    }
#endif

    void IsInstInlineCache::Set(Type * instanceType, JavascriptFunction * function, JavascriptBoolean * result)
    {
        this->type = instanceType;
        this->function = function;
        this->result = result;
    }

    void IsInstInlineCache::Clear()
    {
        this->type = NULL;
        this->function = NULL;
        this->result = NULL;
    }

    void IsInstInlineCache::Unregister(ScriptContext * scriptContext)
    {
        scriptContext->GetThreadContext()->UnregisterIsInstInlineCache(this, this->function);
    }

    bool IsInstInlineCache::TryGetResult(Var instance, JavascriptFunction * function, JavascriptBoolean ** result)
    {
        // In order to get the result from the cache we must have a function instance.
        Assert(function != NULL);

        if (this->function == function &&
            this->type == RecyclableObject::FromVar(instance)->GetType())
        {
            if (result != nullptr)
            {
                (*result = this->result);
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    void IsInstInlineCache::Cache(Type * instanceType, JavascriptFunction * function, JavascriptBoolean *  result, ScriptContext * scriptContext)
    {
        // In order to populate the cache we must have a function instance.
        Assert(function != nullptr);

        // We assume the following invariant: (cache->function != nullptr) => script context is registered as having some populated instance-of inline caches and
        // this cache is registered with thread context for invalidation.
        if (this->function == function)
        {
            Assert(scriptContext->IsRegisteredAsScriptContextWithIsInstInlineCaches());
            Assert(scriptContext->IsIsInstInlineCacheRegistered(this, function));
            this->Set(instanceType, function, result);
        }
        else
        {
            if (this->function != nullptr)
            {
                Unregister(scriptContext);
                Clear();
            }

            scriptContext->RegisterAsScriptContextWithIsInstInlineCaches();
            // If the cache's function is not null, the cache must have been registered already.  No need to register again.
            // In fact, ThreadContext::RegisterIsInstInlineCache, would assert if we tried to re-register the same cache (to enforce the invariant above).
            // Review (jedmiad): What happens if we run out of memory inside RegisterIsInstInlieCache?
            scriptContext->RegisterIsInstInlineCache(this, function);
            this->Set(instanceType, function, result);
        }
    }
}
