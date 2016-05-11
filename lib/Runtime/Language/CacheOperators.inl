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
        bool CheckPolymorphicInlineCache,
        bool CheckTypePropertyCache,
        bool IsInlineCacheAvailable,
        bool IsPolymorphicInlineCacheAvailable,
        bool ReturnOperationInfo>
    __inline bool CacheOperators::TryGetProperty(
        Var const instance,
        const bool isRoot,
        RecyclableObject *const object,
        const PropertyId propertyId,
        Var *const propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo * operationInfo,
        PropertyValueInfo *const propertyValueInfo)
    {
        CompileAssert(IsInlineCacheAvailable || IsPolymorphicInlineCacheAvailable);
        Assert(!CheckTypePropertyCache || !isRoot);
        Assert(propertyValueInfo);
        Assert(IsInlineCacheAvailable == !!propertyValueInfo->GetInlineCache());
        Assert(IsPolymorphicInlineCacheAvailable == !!propertyValueInfo->GetPolymorphicInlineCache());
        Assert(!propertyValueInfo->IsInitField());
        Assert(!ReturnOperationInfo || operationInfo);

        if(CheckLocal || CheckProto || CheckAccessor)
        {
            InlineCache *const inlineCache = IsInlineCacheAvailable ? propertyValueInfo->GetInlineCache() : nullptr;
            if(IsInlineCacheAvailable &&
                inlineCache->TryGetProperty<CheckLocal, CheckProto, CheckAccessor, CheckMissing, ReturnOperationInfo>(
                    instance,
                    object,
                    propertyId,
                    propertyValue,
                    requestContext,
                    operationInfo))
            {
                return true;
            }

            if(CheckPolymorphicInlineCache)
            {
                Assert(IsPolymorphicInlineCacheAvailable || propertyValueInfo->GetFunctionBody());
                PolymorphicInlineCache *const polymorphicInlineCache =
                    IsPolymorphicInlineCacheAvailable
                        ?   propertyValueInfo->GetPolymorphicInlineCache()
                        :   propertyValueInfo->GetFunctionBody()->GetPolymorphicInlineCache(
                                propertyValueInfo->GetInlineCacheIndex());
                if((IsPolymorphicInlineCacheAvailable || polymorphicInlineCache) &&
                    polymorphicInlineCache->TryGetProperty<
                            CheckLocal,
                            CheckProto,
                            CheckAccessor,
                            CheckMissing,
                            IsInlineCacheAvailable,
                            ReturnOperationInfo
                        >(
                            instance,
                            object,
                            propertyId,
                            propertyValue,
                            requestContext,
                            operationInfo,
                            inlineCache
                        ))
                {
                    if(ReturnOperationInfo)
                    {
                        operationInfo->isPolymorphic = true;
                    }
                    return true;
                }
            }
        }

        if(!CheckTypePropertyCache)
        {
            return false;
        }

        TypePropertyCache *const typePropertyCache = object->GetType()->GetPropertyCache();
        if(!typePropertyCache ||
            !typePropertyCache->TryGetProperty(
                    CheckMissing,
                    object,
                    propertyId,
                    propertyValue,
                    requestContext,
                    ReturnOperationInfo ? operationInfo : nullptr,
                    propertyValueInfo))
        {
            return false;
        }

        if(!ReturnOperationInfo || operationInfo->cacheType == CacheType_TypeProperty)
        {
            return true;
        }

        // The property access was cached in an inline cache. Get the proper property operation info.
        const bool success =
            PretendTryGetProperty<IsInlineCacheAvailable, IsPolymorphicInlineCacheAvailable>(
                object->GetType(),
                operationInfo,
                propertyValueInfo);
        Assert(success);
        return true;
    }

    template<
        bool CheckLocal,
        bool CheckLocalTypeWithoutProperty,
        bool CheckAccessor,
        bool CheckPolymorphicInlineCache,
        bool CheckTypePropertyCache,
        bool IsInlineCacheAvailable,
        bool IsPolymorphicInlineCacheAvailable,
        bool ReturnOperationInfo>
    __inline bool CacheOperators::TrySetProperty(
        RecyclableObject *const object,
        const bool isRoot,
        const PropertyId propertyId,
        Var propertyValue,
        ScriptContext *const requestContext,
        const PropertyOperationFlags propertyOperationFlags,
        PropertyCacheOperationInfo * operationInfo,
        PropertyValueInfo *const propertyValueInfo)
    {
        CompileAssert(IsInlineCacheAvailable || IsPolymorphicInlineCacheAvailable);
        Assert(!CheckTypePropertyCache || !isRoot);
        Assert(propertyValueInfo);
        Assert(IsInlineCacheAvailable == !!propertyValueInfo->GetInlineCache());
        Assert(IsPolymorphicInlineCacheAvailable == !!propertyValueInfo->GetPolymorphicInlineCache());
        Assert(!ReturnOperationInfo || operationInfo);

        if(CheckLocal || CheckLocalTypeWithoutProperty || CheckAccessor)
        {
            do
            {
                InlineCache *const inlineCache = IsInlineCacheAvailable ? propertyValueInfo->GetInlineCache() : nullptr;
                InlineCacheHitSlotTypeChangeInfo cacheHitSlotTypeChangeInfo;
                if(IsInlineCacheAvailable &&
                    inlineCache->TrySetProperty<CheckLocal, CheckLocalTypeWithoutProperty, CheckAccessor, ReturnOperationInfo>(
                        object,
                        propertyId,
                        propertyValue,
                        requestContext,
                        operationInfo,
                        cacheHitSlotTypeChangeInfo,
                        propertyOperationFlags))
                {
                    if(!cacheHitSlotTypeChangeInfo.WasSlotTypeChanged())
                        return true;
                }
                else if(CheckPolymorphicInlineCache)
                {
                    Assert(IsPolymorphicInlineCacheAvailable || propertyValueInfo->GetFunctionBody());
                    PolymorphicInlineCache *const polymorphicInlineCache =
                        IsPolymorphicInlineCacheAvailable
                            ?   propertyValueInfo->GetPolymorphicInlineCache()
                            :   propertyValueInfo->GetFunctionBody()->GetPolymorphicInlineCache(
                                    propertyValueInfo->GetInlineCacheIndex());
                    if ((IsPolymorphicInlineCacheAvailable || polymorphicInlineCache) &&
                        polymorphicInlineCache->TrySetProperty<
                                CheckLocal,
                                CheckLocalTypeWithoutProperty,
                                CheckAccessor,
                                IsInlineCacheAvailable,
                                ReturnOperationInfo
                            >(
                                object,
                                propertyId,
                                propertyValue,
                                requestContext,
                                operationInfo,
                                cacheHitSlotTypeChangeInfo,
                                inlineCache,
                                propertyOperationFlags
                            ))
                    {
                        if(ReturnOperationInfo)
                        {
                            operationInfo->isPolymorphic = true;
                        }

                        if(!cacheHitSlotTypeChangeInfo.WasSlotTypeChanged())
                            return true;
                    }
                    else
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }

                Assert(cacheHitSlotTypeChangeInfo.WasSlotTypeChanged());
                DynamicObject *const dynamicObject = DynamicObject::FromVar(object);
                DynamicType *const type = dynamicObject->GetDynamicType();
                PropertyValueInfo::Set(
                    propertyValueInfo,
                    object,
                    type->GetTypeHandler()->InlineOrAuxSlotIndexToPropertyIndex(
                        cacheHitSlotTypeChangeInfo.GetSlotIndex(),
                        cacheHitSlotTypeChangeInfo.IsInlineSlot()),
                    cacheHitSlotTypeChangeInfo.GetSlotType());
                CacheOperators::Cache<false, false, true>(
                    false,
                    dynamicObject,
                    false,
                    type,
                    nullptr,
                    propertyId,
                    cacheHitSlotTypeChangeInfo.GetSlotIndex(),
                    cacheHitSlotTypeChangeInfo.IsInlineSlot(),
                    false,
                    0,
                    propertyValueInfo,
                    requestContext);
                return true;
            } while(false);
        }

        if(!CheckTypePropertyCache)
        {
            return false;
        }

        TypePropertyCache *const typePropertyCache = object->GetType()->GetPropertyCache();
        if(!typePropertyCache ||
            !typePropertyCache->TrySetProperty(
                object,
                propertyId,
                propertyValue,
                requestContext,
                ReturnOperationInfo ? operationInfo : nullptr,
                propertyValueInfo))
        {
            return false;
        }

        if(!ReturnOperationInfo || operationInfo->cacheType == CacheType_TypeProperty)
        {
            return true;
        }

        // The property access was cached in an inline cache. Get the proper property operation info.
        const bool success =
            PretendTrySetProperty<IsInlineCacheAvailable, IsPolymorphicInlineCacheAvailable>(
                object->GetType(),
                object->GetType(),
                operationInfo,
                propertyValueInfo);
        Assert(success);
        return true;
    }

    template<
        bool IsInlineCacheAvailable,
        bool IsPolymorphicInlineCacheAvailable>
    bool CacheOperators::PretendTryGetProperty(
        Type *const type,
        PropertyCacheOperationInfo *operationInfo,
        PropertyValueInfo *const propertyValueInfo)
    {
        CompileAssert(IsInlineCacheAvailable || IsPolymorphicInlineCacheAvailable);
        Assert(propertyValueInfo);
        Assert(IsInlineCacheAvailable == !!propertyValueInfo->GetInlineCache());
        Assert(!IsPolymorphicInlineCacheAvailable || propertyValueInfo->GetPolymorphicInlineCache());
        Assert(operationInfo);

        if (IsInlineCacheAvailable && propertyValueInfo->GetInlineCache()->PretendTryGetProperty(type, operationInfo))
        {
            return true;
        }

        Assert(IsPolymorphicInlineCacheAvailable || propertyValueInfo->GetFunctionBody());
        PolymorphicInlineCache *const polymorphicInlineCache =
            IsPolymorphicInlineCacheAvailable
                ? propertyValueInfo->GetPolymorphicInlineCache()
                : propertyValueInfo->GetFunctionBody()->GetPolymorphicInlineCache(propertyValueInfo->GetInlineCacheIndex());
        if ((IsPolymorphicInlineCacheAvailable || polymorphicInlineCache) &&
            polymorphicInlineCache->PretendTryGetProperty(type, operationInfo))
        {
            operationInfo->isPolymorphic = true;
            return true;
        }

        return false;
    }

    template<
        bool IsInlineCacheAvailable,
        bool IsPolymorphicInlineCacheAvailable>
    bool CacheOperators::PretendTrySetProperty(
        Type *const type,
        Type *const oldType,
        PropertyCacheOperationInfo * operationInfo,
        PropertyValueInfo *const propertyValueInfo)
    {
        CompileAssert(IsInlineCacheAvailable || IsPolymorphicInlineCacheAvailable);
        Assert(propertyValueInfo);
        Assert(IsInlineCacheAvailable == !!propertyValueInfo->GetInlineCache());
        Assert(!IsPolymorphicInlineCacheAvailable || propertyValueInfo->GetPolymorphicInlineCache());
        Assert(operationInfo);

        if (IsInlineCacheAvailable &&
            propertyValueInfo->GetInlineCache()->PretendTrySetProperty<true, true>(type, oldType, operationInfo))
        {
            return true;
        }

        Assert(IsPolymorphicInlineCacheAvailable || propertyValueInfo->GetFunctionBody());
        PolymorphicInlineCache *const polymorphicInlineCache =
            IsPolymorphicInlineCacheAvailable
                ? propertyValueInfo->GetPolymorphicInlineCache()
                : propertyValueInfo->GetFunctionBody()->GetPolymorphicInlineCache(propertyValueInfo->GetInlineCacheIndex());
        return
            (IsPolymorphicInlineCacheAvailable || polymorphicInlineCache) &&
            polymorphicInlineCache->PretendTrySetProperty(type, oldType, operationInfo);
    }

    template<
        bool IsAccessor,
        bool IsRead,
        bool IncludeTypePropertyCache>
    __inline void CacheOperators::Cache(
        bool isProto,
        DynamicObject *const objectWithProperty,
        const bool isRoot,
        Type *type,
        Type *typeWithoutProperty,
        const PropertyId propertyId,
        const PropertyIndex propertyIndex,
        const bool isInlineSlot,
        const bool isMissing,
        int requiredAuxSlotCapacity,
        PropertyValueInfo *const info,
        ScriptContext *const requestContext)
    {
        CompileAssert(!IsAccessor || !IncludeTypePropertyCache);
        Assert(info);
        Assert(objectWithProperty);

        if(!IsAccessor)
        {
            if(!IsRead)
            {
                Assert(!isProto);
                isProto = false;
            }
            if(!isProto)
            {
                Assert(type == objectWithProperty->GetType());
            }
            else
            {
                Assert(type != objectWithProperty->GetType());
                Assert(info->GetSlotType().IsVar());
            }
        }
        else
        {
            Assert(!isRoot); // could still be root object, but the parameter will be false and shouldn't be used for accessors
            Assert(!typeWithoutProperty);
            Assert(requiredAuxSlotCapacity == 0);
            Assert(info->GetSlotType().IsVar());
            Assert(!info->IsInitField());
        }

        if(IsRead)
        {
            Assert(!typeWithoutProperty);
            Assert(requiredAuxSlotCapacity == 0);
            Assert(CanCachePropertyRead(objectWithProperty, requestContext));
            Assert(!info->IsInitField());

            if(!IsAccessor && isProto && PropertyValueInfo::PrototypeCacheDisabled(info))
            {
                return;
            }
        }
        else
        {
            Assert(CanCachePropertyWrite(objectWithProperty, requestContext));

            // TODO(ianhall): the following assert would let global const properties slip through when they shadow
            // a global property. Reason being DictionaryTypeHandler::IsWritable cannot tell if it should check
            // the global property or the global let/const.  Fix this by updating IsWritable to recognize isRoot.

            // Built-in Function.prototype properties 'length', 'arguments', and 'caller' are special cases.
            Assert(
                objectWithProperty->IsWritable(propertyId) ||
                (isRoot && RootObjectBase::FromVar(objectWithProperty)->IsLetConstGlobal(propertyId)) ||
                JavascriptFunction::IsBuiltinProperty(objectWithProperty, propertyId));
        }

        const bool includeTypePropertyCache = IncludeTypePropertyCache && !isRoot;
        bool createTypePropertyCache = false;
        PolymorphicInlineCache *polymorphicInlineCache = info->GetPolymorphicInlineCache();
        if(!polymorphicInlineCache && info->GetFunctionBody())
        {
            polymorphicInlineCache = info->GetFunctionBody()->GetPolymorphicInlineCache(info->GetInlineCacheIndex());
        }
        InlineCache *const inlineCache = info->GetInlineCache();

        ObjectSlotType slotType = ObjectSlotType::GetVar();
        if(!IsAccessor && !isProto)
        {
            slotType = info->GetSlotType();
            if(!slotType.IsVar())
            {
                ObjectSlotType cachedSlotType = ObjectSlotType::GetInt();
                if(inlineCache)
                {
                    if(info->GetFunctionBody())
                        inlineCache->InitializeSlotType(info->GetFunctionBody(), info->GetInlineCacheIndex());
                    cachedSlotType = inlineCache->GetSlotType();
                }
                if(polymorphicInlineCache)
                    cachedSlotType = cachedSlotType.MergeValueType(polymorphicInlineCache->GetSlotType());

                if(cachedSlotType.IsValueTypeMoreConvervativeThan(slotType))
                {
                    // Converge the object's slot type to the most convervative type for this call site, and recheck and update
                    // state based on the new slot type
                    Assert(cachedSlotType.IsFloat() || cachedSlotType.IsVar());
                    PathTypeHandler *const typeHandler =
                        PathTypeHandler::FromTypeHandler(objectWithProperty->GetDynamicType()->GetTypeHandler());
                    slotType = typeHandler->ChangeSlotType(objectWithProperty, info->GetPropertyIndex(), cachedSlotType);
                    Assert(!cachedSlotType.IsValueTypeMoreConvervativeThan(slotType));
                    info->SetSlotType(slotType);
                    DynamicType *const newType = objectWithProperty->GetDynamicType();
                    PathTypeHandler *const newTypeHandler = PathTypeHandler::FromTypeHandler(newType->GetTypeHandler());
                    if(!newTypeHandler->CanStorePropertyValueDirectly(info->GetPropertyIndex()))
                    {
                        if(!IsRead)
                        {
                            PropertyValueInfo::SetNoCache(info, info->GetInstance());
                            return;
                        }
                        PropertyValueInfo::DisableStoreFieldCache(info);
                    }
                    if(!IsRead && typeWithoutProperty)
                    {
                        int newRequiredAuxSlotCapacity;
                        if(CanCachePropertyAdd(
                                objectWithProperty,
                                typeWithoutProperty,
                                isInlineSlot,
                                propertyIndex,
                                info,
                                &newRequiredAuxSlotCapacity))
                        {
                            Assert(newRequiredAuxSlotCapacity == requiredAuxSlotCapacity);
                        }
                        else
                        {
                            typeWithoutProperty = nullptr;
                            requiredAuxSlotCapacity = 0;
                        }
                    }
                    type = newType;
                }
            }
        }

        bool polymorphicInlineCacheSlotTypeUpdated = false;
        if(inlineCache)
        {
            const bool tryCreatePolymorphicInlineCache = !polymorphicInlineCache && info->GetFunctionBody();
            if((includeTypePropertyCache || tryCreatePolymorphicInlineCache) &&
                inlineCache->ShouldBecomeMorePolymorphic<IsAccessor>(isProto, type, typeWithoutProperty, slotType))
            {
                if(tryCreatePolymorphicInlineCache)
                {
                    polymorphicInlineCache =
                        info->GetFunctionBody()->CreateNewPolymorphicInlineCache(
                            info->GetInlineCacheIndex(),
                            propertyId,
                            slotType,
                            inlineCache);
                    polymorphicInlineCacheSlotTypeUpdated = true;
                }
                if(includeTypePropertyCache)
                {
                    createTypePropertyCache = true;
                }
            }

            if(!IsAccessor)
            {
                if(!isProto)
                {
                    inlineCache->CacheLocal(
                        type,
                        propertyId,
                        propertyIndex,
                        slotType,
                        isInlineSlot,
                        typeWithoutProperty,
                        requiredAuxSlotCapacity,
                        requestContext);
                }
                else
                {
                    inlineCache->CacheProto(
                        objectWithProperty,
                        propertyId,
                        propertyIndex,
                        isInlineSlot,
                        isMissing,
                        type,
                        requestContext);
                }
            }
            else
            {
                inlineCache->CacheAccessor(
                    IsRead,
                    propertyId,
                    propertyIndex,
                    isInlineSlot,
                    type,
                    objectWithProperty,
                    isProto,
                    requestContext);
            }
        }

        if(polymorphicInlineCache)
        {
            // Don't resize a polymorphic inline cache from full JIT because it currently doesn't rejit to use the new
            // polymorphic inline cache. Once resized, bailouts would populate only the new set of caches and full JIT would
            // continue to use to old set of caches.
            Assert(!info->AllowResizingPolymorphicInlineCache() || info->GetFunctionBody());
            if((includeTypePropertyCache && !createTypePropertyCache || info->AllowResizingPolymorphicInlineCache()) &&
                polymorphicInlineCache->ShouldBecomeMorePolymorphic<IsAccessor>(isProto, type, typeWithoutProperty, slotType))
            {
                if(info->AllowResizingPolymorphicInlineCache() && polymorphicInlineCache->CanAllocateBigger())
                {
                    polymorphicInlineCache =
                        info->GetFunctionBody()->CreateBiggerPolymorphicInlineCache(
                            info->GetInlineCacheIndex(),
                            propertyId,
                            slotType);
                    polymorphicInlineCacheSlotTypeUpdated = true;
                }
                if(includeTypePropertyCache)
                {
                    createTypePropertyCache = true;
                }
            }

            if(!polymorphicInlineCacheSlotTypeUpdated)
            {
                polymorphicInlineCache->SetSlotType(slotType);
            }

            if(!IsAccessor)
            {
                if(!isProto)
                {
                    polymorphicInlineCache->CacheLocal(
                        type,
                        propertyId,
                        propertyIndex,
                        slotType,
                        isInlineSlot,
                        typeWithoutProperty,
                        requiredAuxSlotCapacity,
                        requestContext);
                }
                else
                {
                    polymorphicInlineCache->CacheProto(
                        objectWithProperty,
                        propertyId,
                        propertyIndex,
                        isInlineSlot,
                        isMissing,
                        type,
                        requestContext);
                }
            }
            else
            {
                polymorphicInlineCache->CacheAccessor(
                    IsRead,
                    propertyId,
                    propertyIndex,
                    isInlineSlot,
                    type,
                    objectWithProperty,
                    isProto,
                    requestContext);
            }
        }

        if(!includeTypePropertyCache)
        {
            return;
        }
        Assert(!IsAccessor);

        TypePropertyCache *typePropertyCache = type->GetPropertyCache();
        if(!typePropertyCache)
        {
            if(!createTypePropertyCache)
            {
                return;
            }
            typePropertyCache = type->CreatePropertyCache();
        }

        if(isProto)
        {
            typePropertyCache->Cache(
                propertyId,
                propertyIndex,
                slotType,
                isInlineSlot,
                info->IsWritable() && info->IsStoreFieldCacheEnabled(),
                isMissing,
                objectWithProperty,
                type);

            typePropertyCache = objectWithProperty->GetType()->GetPropertyCache();
            if(!typePropertyCache)
            {
                return;
            }
        }

        typePropertyCache->Cache(
            propertyId,
            propertyIndex,
            slotType,
            isInlineSlot,
            info->IsWritable() && info->IsStoreFieldCacheEnabled());
    }
}
