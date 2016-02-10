//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    template<
        bool CheckLocal,
        bool CheckProto,
        bool CheckAccessor,
        bool CheckMissing,
        bool ReturnOperationInfo>
    bool InlineCache::TryGetProperty(
        Var const instance,
        RecyclableObject *const propertyObject,
        const PropertyId propertyId,
        Var *const propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo)
    {
        CompileAssert(CheckLocal || CheckProto || CheckAccessor);
        Assert(!ReturnOperationInfo || operationInfo);
        CompileAssert(!ReturnOperationInfo || (CheckLocal && CheckProto && CheckAccessor));
        Assert(instance);
        Assert(propertyObject);
        Assert(propertyId != Constants::NoProperty);
        Assert(propertyValue);
        Assert(requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

        Type *const type = propertyObject->GetType();

        if (CheckLocal && type == TypeWithoutAnyTags(u.local.type))
        {
            Assert(propertyObject->GetScriptContext() == requestContext); // we never cache a type from another script context

            const ObjectSlotType slotType = GetSlotType(u.local.type);
            if(ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Local;
                operationInfo->slotType = slotType;
            }

            if(!TypeHasAuxSlotTag(u.local.type))
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Inline;
                }

                *propertyValue = DynamicObject::FromVar(propertyObject)->GetInlineSlot(u.local.slotIndex, slotType);
            }
            else
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Aux;
                }

                *propertyValue = DynamicObject::FromVar(propertyObject)->GetAuxSlot(u.local.slotIndex, slotType);
            }

            Assert(
                requestContext->AreVarsSameTypeAndValue(*propertyValue, JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext)) ||
                requestContext->AreVarsSameTypeAndValue(*propertyValue, JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext)));
            return true;
        }

        if(CheckProto && type == TypeWithoutAnyTags(u.proto.type) && !u.proto.isMissing)
        {
            Assert(GetSlotType(u.proto.type).IsVar());
            Assert(u.proto.prototypeObject->GetScriptContext() == requestContext); // we never cache a type from another script context

            const ObjectSlotType slotType = ObjectSlotType::GetVar();
            if(ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Proto;
                operationInfo->slotType = slotType;
            }

            if(!TypeHasAuxSlotTag(u.proto.type))
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Inline;
                }

                *propertyValue = u.proto.prototypeObject->GetInlineSlot(u.proto.slotIndex, slotType);
            }
            else
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Aux;
                }

                *propertyValue = u.proto.prototypeObject->GetAuxSlot(u.proto.slotIndex, slotType);
            }

            // TODO: This assert often results in Assert(RootObjectBase::Is(object)) inside GetRootProperty, which is misleading 
            // when the problem is that GetProperty returned a different value than propertyValue. Consider reworking it here and elsewhere.
            Assert(
                requestContext->AreVarsSameTypeAndValue(*propertyValue, JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext)) ||
                requestContext->AreVarsSameTypeAndValue(*propertyValue, JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext)));
            return true;
        }

        if (CheckAccessor && type == TypeWithoutAnyTags(u.accessor.type))
        {
            Assert(GetSlotType(u.accessor.type).IsVar());
            Assert(propertyObject->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(u.accessor.flags & InlineCacheGetterFlag);

            const ObjectSlotType slotType = ObjectSlotType::GetVar();
            if(ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Getter;
                operationInfo->slotType = slotType;
            }

            if(!TypeHasAuxSlotTag(u.accessor.type))
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Inline;
                }

                RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetInlineSlot(u.accessor.slotIndex, slotType));

                *propertyValue = JavascriptOperators::CallGetter(function, instance, requestContext);
            }
            else
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Aux;
                }

                RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetAuxSlot(u.accessor.slotIndex, slotType));

                *propertyValue = JavascriptOperators::CallGetter(function, instance, requestContext);
            }

            // Can't assert because the getter could have a side effect
#ifdef CHKGETTER
            Assert(JavascriptOperators::Equal(*propertyValue, JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext), requestContext));
#endif
            return true;
        }

        if(CheckMissing && type == TypeWithoutAnyTags(u.proto.type) && u.proto.isMissing)
        {
            Assert(GetSlotType(u.proto.type).IsVar());
            Assert(u.proto.prototypeObject->GetScriptContext() == requestContext); // we never cache a type from another script context

            const ObjectSlotType slotType = ObjectSlotType::GetVar();
            if(ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Proto;
                operationInfo->slotType = slotType;
            }

            if(!TypeHasAuxSlotTag(u.proto.type))
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Inline;
                }

                *propertyValue = u.proto.prototypeObject->GetInlineSlot(u.proto.slotIndex, slotType);
            }
            else
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Aux;
                }

                *propertyValue = u.proto.prototypeObject->GetAuxSlot(u.proto.slotIndex, slotType);
            }

            // TODO: This assert often results in Assert(RootObjectBase::Is(object)) inside GetRootProperty, which is misleading 
            // when the problem is that GetProperty returned a different value than propertyValue. Consider reworking it here and elsewhere.
            Assert(
                requestContext->AreVarsSameTypeAndValue(*propertyValue, JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext)) ||
                requestContext->AreVarsSameTypeAndValue(*propertyValue, JavascriptOperators::GetRootProperty(propertyObject, propertyId, requestContext)));

#ifdef MISSING_PROPERTY_STATS
            if (PHASE_STATS1(MissingPropertyCachePhase))
            {
                requestContext->RecordMissingPropertyHit();
            }
#endif
            return true;
        }

        return false;
    }

    template<
        bool CheckLocal,
        bool CheckLocalTypeWithoutProperty,
        bool CheckAccessor,
        bool ReturnOperationInfo>
    bool InlineCache::TrySetProperty(
        RecyclableObject *const object,
        const PropertyId propertyId,
        Var propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        InlineCacheHitSlotTypeChangeInfo &cacheHitSlotTypeChangeInfo,
        const PropertyOperationFlags propertyOperationFlags)
    {
        CompileAssert(CheckLocal || CheckLocalTypeWithoutProperty || CheckAccessor);
        Assert(!ReturnOperationInfo || operationInfo);
        CompileAssert(!ReturnOperationInfo || (CheckLocal && CheckLocalTypeWithoutProperty && CheckAccessor));
        Assert(object);
        Assert(propertyId != Constants::NoProperty);
        Assert(requestContext);
        DebugOnly(VerifyRegistrationForInvalidation(this, requestContext, propertyId));

#if DBG
        const bool isRoot = (propertyOperationFlags & PropertyOperation_Root) != 0;

        bool canSetField; // To verify if we can set a field on the object
        Var setterValue = nullptr;
        { 
            // We need to disable implicit call to ensure the check doesn't cause unwanted side effects in debug code
            // Save old disableImplicitFlags and implicitCallFlags and disable implicit call and exception
            ThreadContext * threadContext = requestContext->GetThreadContext();
            DisableImplicitFlags disableImplicitFlags = *threadContext->GetAddressOfDisableImplicitFlags();
            Js::ImplicitCallFlags implicitCallFlags = threadContext->GetImplicitCallFlags();
            threadContext->ClearImplicitCallFlags();
            *threadContext->GetAddressOfDisableImplicitFlags() = DisableImplicitCallAndExceptionFlag;

            DescriptorFlags flags = DescriptorFlags::None;
            canSetField = !JavascriptOperators::CheckPrototypesForAccessorOrNonWritablePropertySlow(object, propertyId, &setterValue, &flags, isRoot, requestContext);
            if (threadContext->GetImplicitCallFlags() != Js::ImplicitCall_None)
            {
                canSetField = true; // If there was an implicit call, inconclusive. Disable debug check.
                setterValue = nullptr;
            }
            else 
                if ((flags & Accessor) == Accessor)
            {
                Assert(setterValue != nullptr);
            }

            // Restore old disableImplicitFlags and implicitCallFlags
            *threadContext->GetAddressOfDisableImplicitFlags() = disableImplicitFlags;
            threadContext->SetImplicitCallFlags(implicitCallFlags);
        }
#endif

        Type *const type = object->GetType();

        if (CheckLocal && type == TypeWithoutAnyTags(u.local.type))
        {
            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(isRoot || object->GetPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(u.local.slotIndex, !TypeHasAuxSlotTag(u.local.type)));
            Assert(!isRoot || RootObjectBase::FromVar(object)->GetRootPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(u.local.slotIndex, !TypeHasAuxSlotTag(u.local.type)));
            Assert(object->CanStorePropertyValueDirectly(propertyId, isRoot));

            if(ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Local;
            }

            const PropertyIndex slotIndex = u.local.slotIndex;
            const ObjectSlotType slotTypeBeforeSet = GetSlotType(u.local.type);
            if(!TypeHasAuxSlotTag(u.local.type))
            {
                const ObjectSlotType slotTypeAfterSet =
                    DynamicObject::FromVar(object)->SetInlineSlot(
                        SetSlotArgumentsRoot(propertyId, isRoot, slotIndex, slotTypeBeforeSet, propertyValue));

                if(slotTypeAfterSet != slotTypeBeforeSet)
                {
                    Assert(!slotTypeBeforeSet.IsValueTypeMoreConvervativeThan(slotTypeAfterSet));
                    cacheHitSlotTypeChangeInfo = InlineCacheHitSlotTypeChangeInfo(slotTypeAfterSet, true, slotIndex);
                }

                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Inline;
                    operationInfo->slotType = slotTypeAfterSet;
                }
            }
            else
            {
                const ObjectSlotType slotTypeAfterSet =
                    DynamicObject::FromVar(object)->SetAuxSlot(
                        SetSlotArgumentsRoot(propertyId, isRoot, slotIndex, slotTypeBeforeSet, propertyValue));

                if(slotTypeAfterSet != slotTypeBeforeSet)
                {
                    Assert(!slotTypeBeforeSet.IsValueTypeMoreConvervativeThan(slotTypeAfterSet));
                    cacheHitSlotTypeChangeInfo = InlineCacheHitSlotTypeChangeInfo(slotTypeAfterSet, false, slotIndex);
                }

                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Aux;
                    operationInfo->slotType = slotTypeAfterSet;
                }
            }
            Assert(canSetField);
            return true;
        }

        if (CheckLocalTypeWithoutProperty && type == TypeWithoutAnyTags(u.local.typeWithoutProperty))
        {
            // CAREFUL! CheckIfPrototypeChainHasOnlyWritableDataProperties or AdjustSlots may do allocation that triggers GC and
            // clears this cache, so save any info that is needed from the cache before calling those functions.
            Type *const typeWithProperty = TypeWithoutAnyTags(u.local.type);
            const PropertyIndex slotIndex = u.local.slotIndex;

            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(typeWithProperty);
            Assert(DynamicType::Is(typeWithProperty->GetTypeId()));
            Assert(((DynamicType*)typeWithProperty)->GetIsShared());
            Assert(((DynamicType*)typeWithProperty)->GetTypeHandler()->IsPathTypeHandler());
            AssertMsg(!((DynamicType*)TypeWithoutAnyTags(u.local.typeWithoutProperty))->GetTypeHandler()->GetIsPrototype(),  "Why did we cache a property add for a prototype?");
            Assert(((DynamicType*)typeWithProperty)->GetTypeHandler()->CanStorePropertyValueDirectly((const DynamicObject*)object, propertyId, isRoot));

            if(ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_LocalWithoutProperty;
            }

            DynamicObject *const dynamicObject = DynamicObject::FromVar(object);

            const ObjectSlotType slotTypeBeforeSet = GetSlotType(u.local.type);
            if(!TypeHasAuxSlotTag(u.local.type))
            {
                // If we're adding a property to an inlined slot, we should never need to adjust auxiliary slot array size.
                Assert(u.local.requiredAuxSlotCapacity == 0);

                dynamicObject->type = typeWithProperty;

                Assert(isRoot || object->GetPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(slotIndex, true));
                Assert(!isRoot || RootObjectBase::FromVar(object)->GetRootPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(slotIndex, true));

                const ObjectSlotType slotTypeAfterSet =
                    dynamicObject->SetInlineSlot(
                        SetSlotArgumentsRoot(propertyId, isRoot, slotIndex, slotTypeBeforeSet, propertyValue));
                Assert(!slotTypeBeforeSet.IsValueTypeMoreConvervativeThan(slotTypeAfterSet));

                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Inline;
                    operationInfo->slotType = slotTypeAfterSet;
                }
            }
            else
            {
                uint16 newAuxSlotCapacity = u.local.requiredAuxSlotCapacity;
                if (newAuxSlotCapacity > 0)
                {
                    DynamicTypeHandler::AdjustSlots(
                        dynamicObject,
                        static_cast<DynamicType *>(typeWithProperty)->GetTypeHandler()->GetInlineSlotCapacity(),
                        newAuxSlotCapacity);
                }

                dynamicObject->type = typeWithProperty;

                Assert(isRoot || object->GetPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(slotIndex, false));
                Assert(!isRoot || RootObjectBase::FromVar(object)->GetRootPropertyIndex(propertyId) == DynamicObject::FromVar(object)->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(slotIndex, false));

                const ObjectSlotType slotTypeAfterSet =
                    dynamicObject->SetAuxSlot(
                        SetSlotArgumentsRoot(propertyId, isRoot, slotIndex, slotTypeBeforeSet, propertyValue));
                Assert(!slotTypeBeforeSet.IsValueTypeMoreConvervativeThan(slotTypeAfterSet));

                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Aux;
                    operationInfo->slotType = slotTypeAfterSet;
                }
            }
            Assert(canSetField);
            return true;
        }

        if (CheckAccessor && type == TypeWithoutAnyTags(u.accessor.type))
        {
            Assert(GetSlotType(u.accessor.type).IsVar());
            Assert(object->GetScriptContext() == requestContext); // we never cache a type from another script context
            Assert(u.accessor.flags & InlineCacheSetterFlag);

            const ObjectSlotType slotType = ObjectSlotType::GetVar();
            if(ReturnOperationInfo)
            {
                operationInfo->cacheType = CacheType_Setter;
                operationInfo->slotType = slotType;
            }

            if(!TypeHasAuxSlotTag(u.accessor.type))
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Inline;
                }

                RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetInlineSlot(u.accessor.slotIndex, slotType));

                Assert(setterValue == nullptr || setterValue == function);
                Js::JavascriptOperators::CallSetter(function, object, propertyValue, requestContext);
            }
            else
            {
                if(ReturnOperationInfo)
                {
                    operationInfo->slotLocation = SlotLocation_Aux;
                }

                RecyclableObject *const function = RecyclableObject::FromVar(u.accessor.object->GetAuxSlot(u.accessor.slotIndex, slotType));

                Assert(setterValue == nullptr || setterValue == function);
                Js::JavascriptOperators::CallSetter(function, object, propertyValue, requestContext);
            }
            return true;
        }

        return false;
    }  

    template<
        bool CheckLocalAndAccessor,
        bool CheckLocalTypeWithoutProperty>
    bool InlineCache::PretendTrySetProperty(Type *const type, Type *const oldType, PropertyCacheOperationInfo * operationInfo)
    {
        if (CheckLocalAndAccessor && type == TypeWithoutAnyTags(u.local.type))
        {
            operationInfo->cacheType = CacheType_Local;
            operationInfo->slotLocation = TypeHasAuxSlotTag(u.local.type) ? SlotLocation_Aux : SlotLocation_Inline;
            operationInfo->slotType = GetSlotType(u.local.type);
            return true;
        }

        if (CheckLocalTypeWithoutProperty && oldType != type && oldType == TypeWithoutAnyTags(u.local.typeWithoutProperty))
        {
            operationInfo->cacheType = CacheType_LocalWithoutProperty;
            operationInfo->slotLocation = 
                TypeHasAuxSlotTag(u.local.typeWithoutProperty) ? SlotLocation_Aux : SlotLocation_Inline;
            operationInfo->slotType = GetSlotType(u.local.typeWithoutProperty);
            return true;
        }

        if (CheckLocalAndAccessor && type == TypeWithoutAnyTags(u.accessor.type))
        {
            Assert(GetSlotType(u.accessor.type).IsVar());
            if (u.accessor.flags & InlineCacheSetterFlag)
            {
                operationInfo->cacheType = CacheType_Setter;
                operationInfo->slotLocation = TypeHasAuxSlotTag(u.accessor.type) ? SlotLocation_Aux : SlotLocation_Inline;
                operationInfo->slotType = ObjectSlotType::GetVar();
                return true;
            }
        }

        return false;
    }

    template<bool IsCachingAccessor>
    __inline bool PolymorphicInlineCache::ShouldBecomeMorePolymorphic(
        const bool isCachingProto,
        const Type *const type,
        const Type *const typeWithoutProperty,
        const ObjectSlotType slotType)
    {
        Assert(!IsCachingAccessor && !isCachingProto || !typeWithoutProperty);

        const uint inlineCacheIndex =
            GetInlineCacheIndexForType(IsCachingAccessor || !typeWithoutProperty ? type : typeWithoutProperty);
        return
            inlineCaches[inlineCacheIndex].ShouldBecomeMorePolymorphic<IsCachingAccessor>(
                isCachingProto,
                type,
                typeWithoutProperty,
                slotType);
    }

    __inline bool PolymorphicInlineCache::HasType_Accessor(const Type * type) const
    {
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        return inlineCaches[inlineCacheIndex].HasType_Accessor(type);
    }

    template<
        bool CheckLocal,
        bool CheckProto,
        bool CheckAccessor>
    void PolymorphicInlineCache::CloneInlineCacheToEmptySlotInCollision(Type * const type, uint inlineCacheIndex)
    {
        if (CheckLocal && (inlineCaches[inlineCacheIndex].u.local.type == type || inlineCaches[inlineCacheIndex].u.local.type == TypeWithAuxSlotTag(type)))
        {
            return;
        }
        if (CheckProto && (inlineCaches[inlineCacheIndex].u.proto.type == type || inlineCaches[inlineCacheIndex].u.proto.type == TypeWithAuxSlotTag(type)))
        {
            return;
        }
        if (CheckAccessor && (inlineCaches[inlineCacheIndex].u.accessor.type == type || inlineCaches[inlineCacheIndex].u.accessor.type == TypeWithAuxSlotTag(type)))
        {
            return;
        }

        if (this->IsFull())
        {
            // If the cache is full, we won't find an empty slot to move the contents of the colliding inline cache to.
            return;
        }

        // Collision is with a cache having a different type.

        uint tryInlineCacheIndex = GetNextInlineCacheIndex(inlineCacheIndex);

        // Iterate over the inline caches in the polymorphic cache, stop when:
        //   1. an empty inline cache is found, or
        //   2. a cache already populated with the incoming type is found, or
        //   3. all the inline caches have been looked at.
        while (!inlineCaches[tryInlineCacheIndex].IsEmpty() && tryInlineCacheIndex != inlineCacheIndex)
        {
            if (CheckLocal && (inlineCaches[tryInlineCacheIndex].u.local.type == type || inlineCaches[tryInlineCacheIndex].u.local.type == TypeWithAuxSlotTag(type)))
            {
                break;
            }
            if (CheckProto && (inlineCaches[tryInlineCacheIndex].u.proto.type == type || inlineCaches[tryInlineCacheIndex].u.proto.type == TypeWithAuxSlotTag(type)))
            {
                Assert(GetInlineCacheIndexForType(inlineCaches[tryInlineCacheIndex].u.proto.type) == inlineCacheIndex);
                break;
            }
            if (CheckAccessor && (inlineCaches[tryInlineCacheIndex].u.accessor.type == type || inlineCaches[tryInlineCacheIndex].u.accessor.type == TypeWithAuxSlotTag(type)))
            {
                Assert(GetInlineCacheIndexForType(inlineCaches[tryInlineCacheIndex].u.accessor.type) == inlineCacheIndex);
                break;
            }
            tryInlineCacheIndex = GetNextInlineCacheIndex(tryInlineCacheIndex);
        }
        if (tryInlineCacheIndex != inlineCacheIndex)
        {
            if (inlineCaches[inlineCacheIndex].invalidationListSlotPtr != nullptr)
            {
                Assert(*(inlineCaches[inlineCacheIndex].invalidationListSlotPtr) == &inlineCaches[inlineCacheIndex]);
                if (inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr != nullptr)
                {
                    Assert(*(inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr) == &inlineCaches[tryInlineCacheIndex]);
                }
                else
                {
                    inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr = inlineCaches[inlineCacheIndex].invalidationListSlotPtr;
                    *(inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr) = &inlineCaches[tryInlineCacheIndex];
                    inlineCaches[inlineCacheIndex].invalidationListSlotPtr = nullptr;
                }
            }
            inlineCaches[tryInlineCacheIndex].u = inlineCaches[inlineCacheIndex].u;
            UpdateInlineCachesFillInfo(tryInlineCacheIndex, true /*set*/);
            // Let's clear the cache slot on which we had the collision. We might have stolen the invalidationListSlotPtr,
            // so it may not pass VerifyRegistrationForInvalidation. Besides, it will be repopulated with the incoming data,
            // and registered for invalidation, if necessary.
            inlineCaches[inlineCacheIndex].Clear();
            Assert((this->inlineCachesFillInfo & (1 << inlineCacheIndex)) != 0);
            UpdateInlineCachesFillInfo(inlineCacheIndex, false /*set*/);
        }
    }

#ifdef CLONE_INLINECACHE_TO_EMPTYSLOT
    template <typename TDelegate>
    bool PolymorphicInlineCache::CheckClonedInlineCache(uint inlineCacheIndex, TDelegate mapper)
    {
        bool success = false;
        uint tryInlineCacheIndex = GetNextInlineCacheIndex(inlineCacheIndex);
        do
        {
            if (inlineCaches[tryInlineCacheIndex].IsEmpty())
            {
                break;
            }
            success = mapper(tryInlineCacheIndex);
            if (success)
            {
                Assert(inlineCaches[inlineCacheIndex].invalidationListSlotPtr == nullptr || *inlineCaches[inlineCacheIndex].invalidationListSlotPtr == &inlineCaches[inlineCacheIndex]);
                Assert(inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr == nullptr || *inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr == &inlineCaches[tryInlineCacheIndex]);

                // Swap inline caches, including their invalidationListSlotPtrs.
                InlineCache temp = inlineCaches[tryInlineCacheIndex];
                inlineCaches[tryInlineCacheIndex] = inlineCaches[inlineCacheIndex];
                inlineCaches[inlineCacheIndex] = temp;

                // Fix up invalidationListSlotPtrs to point to their owners.
                if (inlineCaches[inlineCacheIndex].invalidationListSlotPtr != nullptr)
                {
                    *inlineCaches[inlineCacheIndex].invalidationListSlotPtr = &inlineCaches[inlineCacheIndex];
                }
                if (inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr != nullptr)
                {
                    *inlineCaches[tryInlineCacheIndex].invalidationListSlotPtr = &inlineCaches[tryInlineCacheIndex];
                }

                break;
            }
            tryInlineCacheIndex = GetNextInlineCacheIndex(tryInlineCacheIndex);

        } while (tryInlineCacheIndex != inlineCacheIndex);

        return success;
    }
#endif

    template<
        bool CheckLocal,
        bool CheckProto,
        bool CheckAccessor,
        bool CheckMissing,
        bool IsInlineCacheAvailable,
        bool ReturnOperationInfo>
    bool PolymorphicInlineCache::TryGetProperty(
        Var const instance,
        RecyclableObject *const propertyObject,
        const PropertyId propertyId,
        Var *const propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        InlineCache *const inlineCacheToPopulate)
    {
        Assert(!IsInlineCacheAvailable || inlineCacheToPopulate);
        Assert(!ReturnOperationInfo || operationInfo);

        Type * const type = propertyObject->GetType();
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        InlineCache *cache = &inlineCaches[inlineCacheIndex];

#ifdef INLINE_CACHE_STATS
        bool isEmpty = false;
        if (PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            isEmpty = cache->IsEmpty();
        }
#endif
        bool result = cache->TryGetProperty<CheckLocal, CheckProto, CheckAccessor, CheckMissing, ReturnOperationInfo>(
            instance, propertyObject, propertyId, propertyValue, requestContext, operationInfo);

#ifdef CLONE_INLINECACHE_TO_EMPTYSLOT
        if (!result && !cache->IsEmpty())
        {
            result = CheckClonedInlineCache(inlineCacheIndex, [&](uint tryInlineCacheIndex) -> bool
            {
                cache = &inlineCaches[tryInlineCacheIndex];
                return cache->TryGetProperty<CheckLocal, CheckProto, CheckAccessor, CheckMissing, ReturnOperationInfo>(
                    instance, propertyObject, propertyId, propertyValue, requestContext, operationInfo);
            });
        }
#endif

        if (IsInlineCacheAvailable && result)
        {
            cache->CopyTo(propertyId, requestContext, inlineCacheToPopulate);
        }

#ifdef INLINE_CACHE_STATS
        if (PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            bool collision = !result && !isEmpty;
            this->functionBody->GetScriptContext()->LogCacheUsage(this, /*isGet*/ true, propertyId, result, collision);
        }
#endif

        return result;
    }

    template<
        bool CheckLocal,
        bool CheckLocalTypeWithoutProperty,
        bool CheckAccessor,
        bool IsInlineCacheAvailable,
        bool ReturnOperationInfo>
    bool PolymorphicInlineCache::TrySetProperty(
        RecyclableObject *const object,
        const PropertyId propertyId,
        Var propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        InlineCacheHitSlotTypeChangeInfo &cacheHitSlotTypeChangeInfo,
        InlineCache *const inlineCacheToPopulate,
        const PropertyOperationFlags propertyOperationFlags)
    {
        Assert(!IsInlineCacheAvailable || inlineCacheToPopulate);
        Assert(!ReturnOperationInfo || operationInfo);

        Type * const type = object->GetType();
        uint inlineCacheIndex = GetInlineCacheIndexForType(type);
        InlineCache *cache = &inlineCaches[inlineCacheIndex];

#ifdef INLINE_CACHE_STATS
        bool isEmpty = false;
        if (PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            isEmpty = cache->IsEmpty();
        }
#endif
        bool result = cache->TrySetProperty<CheckLocal, CheckLocalTypeWithoutProperty, CheckAccessor, ReturnOperationInfo>(
                object, propertyId, propertyValue, requestContext, operationInfo, cacheHitSlotTypeChangeInfo, propertyOperationFlags);

#ifdef CLONE_INLINECACHE_TO_EMPTYSLOT
        if (!result && !cache->IsEmpty())
        {
            result = CheckClonedInlineCache(inlineCacheIndex, [&](uint tryInlineCacheIndex) -> bool
            {
                cache = &inlineCaches[tryInlineCacheIndex];
                return cache->TrySetProperty<CheckLocal, CheckLocalTypeWithoutProperty, CheckAccessor, ReturnOperationInfo>(
                    object, propertyId, propertyValue, requestContext, operationInfo, cacheHitSlotTypeChangeInfo, propertyOperationFlags);
            });
        }
#endif

        if(IsInlineCacheAvailable && result && !cacheHitSlotTypeChangeInfo.WasSlotTypeChanged())
        {
            cache->CopyTo(propertyId, requestContext, inlineCacheToPopulate);
        }

#ifdef INLINE_CACHE_STATS
        if (PHASE_STATS1(Js::PolymorphicInlineCachePhase))
        {
            bool collision = !result && !isEmpty;
            this->functionBody->GetScriptContext()->LogCacheUsage(this, /*isGet*/ false, propertyId, result, collision);
        }
#endif

        return result;
    }
}
