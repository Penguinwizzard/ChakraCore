//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeTypePch.h"

namespace Js
{
    // -------------------------------------------------------------------------------------------------------------------------
    // TypePropertyCacheElement
    // -------------------------------------------------------------------------------------------------------------------------

    TypePropertyCacheElement::TypePropertyCacheElement()
        : prototypeObjectWithProperty(nullptr), id(Constants::NoProperty), index(0), slotType(0), bits(0)
    {
    }

    PropertyId TypePropertyCacheElement::Id() const
    {
        return id;
    }

    PropertyIndex TypePropertyCacheElement::Index() const
    {
        return index;
    }

    ObjectSlotType TypePropertyCacheElement::SlotType() const
    {
        return slotType;
    }

    bool TypePropertyCacheElement::IsInlineSlot() const
    {
        return isInlineSlot;
    }

    bool TypePropertyCacheElement::IsSetPropertyAllowed() const
    {
        return isSetPropertyAllowed;
    }

    bool TypePropertyCacheElement::IsMissing() const
    {
        return isMissing;
    }

    DynamicObject *TypePropertyCacheElement::PrototypeObjectWithProperty() const
    {
        return prototypeObjectWithProperty;
    }

    void TypePropertyCacheElement::Cache(
        const PropertyId id,
        const PropertyIndex index,
        const ObjectSlotType slotType,
        const bool isInlineSlot,
        const bool isSetPropertyAllowed)
    {
        Assert(id != Constants::NoProperty);
        Assert(index != Constants::NoSlot);

        this->id = id;
        this->index = index;
        this->slotType = slotType;
        this->isInlineSlot = isInlineSlot;
        this->isSetPropertyAllowed = isSetPropertyAllowed;
        this->isMissing = false;
        this->prototypeObjectWithProperty = nullptr;
    }

    void TypePropertyCacheElement::Cache(
        const PropertyId id,
        const PropertyIndex index,
        const ObjectSlotType slotType,
        const bool isInlineSlot,
        const bool isSetPropertyAllowed,
        const bool isMissing,
        DynamicObject *const prototypeObjectWithProperty,
        Type *const myParentType)
    {
        Assert(id != Constants::NoProperty);
        Assert(index != Constants::NoSlot);
        Assert(prototypeObjectWithProperty);
        Assert(myParentType);

        if(this->id != id || !this->prototypeObjectWithProperty)
            myParentType->GetScriptContext()->GetThreadContext()->RegisterTypeWithProtoPropertyCache(id, myParentType);

        this->id = id;
        this->index = index;
        this->slotType = slotType;
        this->isInlineSlot = isInlineSlot;
        this->isSetPropertyAllowed = isSetPropertyAllowed;
        this->isMissing = isMissing;
        this->prototypeObjectWithProperty = prototypeObjectWithProperty;
        Assert(
            this->isMissing ==
            (this->prototypeObjectWithProperty == this->prototypeObjectWithProperty->GetLibrary()->GetMissingPropertyHolder()));
    }

    void TypePropertyCacheElement::Clear()
    {
        id = Constants::NoProperty;
    }

    // -------------------------------------------------------------------------------------------------------------------------
    // TypePropertyCache
    // -------------------------------------------------------------------------------------------------------------------------

    size_t TypePropertyCache::ElementIndex(const PropertyId id)
    {
        Assert(id != Constants::NoProperty);
        Assert((TypePropertyCache_NumElements & TypePropertyCache_NumElements - 1) == 0);

        return id & TypePropertyCache_NumElements - 1;
    }

    __inline bool TypePropertyCache::TryGetIndexForLoad(
        const bool checkMissing,
        const PropertyId id,
        PropertyIndex *const indexRef,
        ObjectSlotType *const slotTypeRef,
        bool *const isInlineSlotRef,
        bool *const isMissingRef,
        DynamicObject * *const prototypeObjectWithPropertyRef) const
    {
        Assert(indexRef);
        Assert(isInlineSlotRef);
        Assert(isMissingRef);
        Assert(prototypeObjectWithPropertyRef);

        const TypePropertyCacheElement &element = elements[ElementIndex(id)];
        if(element.Id() != id || !checkMissing && element.IsMissing())
            return false;

        *indexRef = element.Index();
        *slotTypeRef = element.SlotType();
        *isInlineSlotRef = element.IsInlineSlot();
        *isMissingRef = checkMissing ? element.IsMissing() : false;
        *prototypeObjectWithPropertyRef = element.PrototypeObjectWithProperty();
        return true;
    }

    __inline bool TypePropertyCache::TryGetIndexForStore(
        const PropertyId id,
        PropertyIndex *const indexRef,
        ObjectSlotType *const slotTypeRef,
        bool *const isInlineSlotRef) const
    {
        Assert(indexRef);
        Assert(isInlineSlotRef);

        const TypePropertyCacheElement &element = elements[ElementIndex(id)];
        if(element.Id() != id ||
            !element.IsSetPropertyAllowed() ||
            element.PrototypeObjectWithProperty())
        {
            return false;
        }

        Assert(!element.IsMissing());
        *indexRef = element.Index();
        *slotTypeRef = element.SlotType();
        *isInlineSlotRef = element.IsInlineSlot();
        return true;
    }

    bool TypePropertyCache::TryGetProperty(
        const bool checkMissing,
        RecyclableObject *const propertyObject,
        const PropertyId propertyId,
        Var *const propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        PropertyValueInfo *const propertyValueInfo)
    {
        Assert(propertyValueInfo);
        Assert(propertyValueInfo->GetInlineCache() || propertyValueInfo->GetPolymorphicInlineCache());

        PropertyIndex propertyIndex;
        ObjectSlotType slotType = ObjectSlotType::GetVar();
        DynamicObject *prototypeObjectWithProperty;
        bool isInlineSlot, isMissing;
        if(!TryGetIndexForLoad(
                checkMissing,
                propertyId,
                &propertyIndex,
                &slotType,
                &isInlineSlot,
                &isMissing,
                &prototypeObjectWithProperty))
        {
        #if DBG_DUMP
            if(PHASE_TRACE1(TypePropertyCachePhase))
            {
                CacheOperators::TraceCache(
                    static_cast<InlineCache *>(nullptr),
                    L"TypePropertyCache get miss",
                    propertyId,
                    requestContext,
                    propertyObject);
            }
        #endif
            return false;
        }

        if(!prototypeObjectWithProperty)
        {
        #if DBG_DUMP
            if(PHASE_TRACE1(TypePropertyCachePhase))
            {
                CacheOperators::TraceCache(
                    static_cast<InlineCache *>(nullptr),
                    L"TypePropertyCache get hit",
                    propertyId,
                    requestContext,
                    propertyObject);
            }
        #endif

        #if DBG
            {
                DynamicTypeHandler *const typeHandler =
                    DynamicObject::FromVar(propertyObject)->GetDynamicType()->GetTypeHandler();
                const PropertyIndex typeHandlerPropertyIndex =
                    typeHandler->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, isInlineSlot);
                Assert(typeHandlerPropertyIndex == propertyObject->GetPropertyIndex(propertyId));
                Assert(slotType.IsValueTypeEqualTo(typeHandler->GetSlotType(typeHandlerPropertyIndex)));
            }
        #endif

            *propertyValue =
                isInlineSlot
                    ? DynamicObject::FromVar(propertyObject)->GetInlineSlot(propertyIndex, slotType)
                    : DynamicObject::FromVar(propertyObject)->GetAuxSlot(propertyIndex, slotType);
            if(propertyObject->GetScriptContext() == requestContext)
            {
                Assert(
                    requestContext->AreVarsSameTypeAndValue(
                        *propertyValue,
                        JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext)));

                DynamicTypeHandler *const typeHandler =
                    DynamicObject::FromVar(propertyObject)->GetDynamicType()->GetTypeHandler();
                const PropertyIndex typeHandlerPropertyIndex =
                    typeHandler->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, isInlineSlot);
                PropertyValueInfo::Set(propertyValueInfo, propertyObject, typeHandlerPropertyIndex, slotType);

                CacheOperators::Cache<false, true, false>(
                    false,
                    DynamicObject::FromVar(propertyObject),
                    false,
                    propertyObject->GetType(),
                    nullptr,
                    propertyId,
                    propertyIndex,
                    isInlineSlot,
                    false,
                    0,
                    propertyValueInfo,
                    requestContext);
                return true;
            }

            *propertyValue = CrossSite::MarshalVar(requestContext, *propertyValue);
            // Cannot use GetProperty and compare results since they may not compare equal when they're marshaled

            if(operationInfo)
            {
                operationInfo->cacheType = CacheType_TypeProperty;
                operationInfo->slotLocation = isInlineSlot ? SlotLocation_Inline : SlotLocation_Aux;
                operationInfo->slotType = slotType;
            }
            return true;
        }

        Assert(slotType.IsVar());
        slotType = ObjectSlotType::GetVar();

    #if DBG_DUMP
        if(PHASE_TRACE1(TypePropertyCachePhase))
        {
            CacheOperators::TraceCache(
                static_cast<InlineCache *>(nullptr),
                L"TypePropertyCache get hit prototype",
                propertyId,
                requestContext,
                propertyObject);
        }
    #endif

    #if DBG
        {
            DynamicTypeHandler *const prototypeTypeHandler = prototypeObjectWithProperty->GetDynamicType()->GetTypeHandler();
            const PropertyIndex prototypeTypeHandlerPropertyIndex =
                prototypeTypeHandler->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, isInlineSlot);
            Assert(prototypeTypeHandlerPropertyIndex == prototypeObjectWithProperty->GetPropertyIndex(propertyId));
            Assert(slotType.IsValueTypeEqualTo(prototypeTypeHandler->GetSlotType(prototypeTypeHandlerPropertyIndex)));
        }
    #endif

        *propertyValue =
            isInlineSlot
                ? prototypeObjectWithProperty->GetInlineSlot(propertyIndex, slotType)
                : prototypeObjectWithProperty->GetAuxSlot(propertyIndex, slotType);
        if(prototypeObjectWithProperty->GetScriptContext() == requestContext)
        {
            Assert(
                requestContext->AreVarsSameTypeAndValue(
                    *propertyValue,
                    JavascriptOperators::GetProperty(propertyObject, propertyId, requestContext)));

            if(propertyObject->GetScriptContext() != requestContext)
            {
                return true;
            }

            DynamicTypeHandler *const prototypeTypeHandler = prototypeObjectWithProperty->GetDynamicType()->GetTypeHandler();
            const PropertyIndex prototypeTypeHandlerPropertyIndex =
                prototypeTypeHandler->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, isInlineSlot);
            PropertyValueInfo::Set(propertyValueInfo, prototypeObjectWithProperty, prototypeTypeHandlerPropertyIndex, slotType);

            CacheOperators::Cache<false, true, false>(
                true,
                prototypeObjectWithProperty,
                false,
                propertyObject->GetType(),
                nullptr,
                propertyId,
                propertyIndex,
                isInlineSlot,
                isMissing,
                0,
                propertyValueInfo,
                requestContext);
            return true;
        }

        *propertyValue = CrossSite::MarshalVar(requestContext, *propertyValue);
        // Cannot use GetProperty and compare results since they may not compare equal when they're marshaled

        if(operationInfo)
        {
            operationInfo->cacheType = CacheType_TypeProperty;
            operationInfo->slotLocation = isInlineSlot ? SlotLocation_Inline : SlotLocation_Aux;
            operationInfo->slotType = slotType;
        }
        return true;
    }

    bool TypePropertyCache::TrySetProperty(
        RecyclableObject *const object,
        const PropertyId propertyId,
        Var propertyValue,
        ScriptContext *const requestContext,
        PropertyCacheOperationInfo *const operationInfo,
        PropertyValueInfo *const propertyValueInfo)
    {
        Assert(propertyValueInfo);
        Assert(propertyValueInfo->GetInlineCache() || propertyValueInfo->GetPolymorphicInlineCache());

        PropertyIndex propertyIndex;
        ObjectSlotType slotTypeBeforeSet = ObjectSlotType::GetVar();
        bool isInlineSlot;
        if(!TryGetIndexForStore(propertyId, &propertyIndex, &slotTypeBeforeSet, &isInlineSlot))
        {
        #if DBG_DUMP
            if(PHASE_TRACE1(TypePropertyCachePhase))
            {
                CacheOperators::TraceCache(
                    static_cast<InlineCache *>(nullptr),
                    L"TypePropertyCache set miss",
                    propertyId,
                    requestContext,
                    object);
            }
        #endif
            return false;
        }

    #if DBG_DUMP
        if(PHASE_TRACE1(TypePropertyCachePhase))
        {
            CacheOperators::TraceCache(
                static_cast<InlineCache *>(nullptr),
                L"TypePropertyCache set hit",
                propertyId,
                requestContext,
                object);
        }
    #endif

    #if DBG
        {
            DynamicTypeHandler *const typeHandler =
                DynamicObject::FromVar(object)->GetDynamicType()->GetTypeHandler();
            const PropertyIndex typeHandlerPropertyIndex =
                typeHandler->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, isInlineSlot);
            Assert(typeHandlerPropertyIndex == object->GetPropertyIndex(propertyId));
            Assert(slotTypeBeforeSet.IsValueTypeEqualTo(typeHandler->GetSlotType(typeHandlerPropertyIndex)));
        }
    #endif
        Assert(!object->IsFixedProperty(propertyId));
        Assert(object->CanStorePropertyValueDirectly(propertyId, false));

        ScriptContext *const objectScriptContext = object->GetScriptContext();
        if(objectScriptContext != requestContext)
        {
            propertyValue = CrossSite::MarshalVar(objectScriptContext, propertyValue);
        }

        ObjectSlotType slotTypeAfterSet = ObjectSlotType::GetVar();
        if(isInlineSlot)
        {
            slotTypeAfterSet =
                DynamicObject::FromVar(object)->SetInlineSlot(
                    SetSlotArguments(propertyId, propertyIndex, slotTypeBeforeSet, propertyValue));
        }
        else
        {
            slotTypeAfterSet =
                DynamicObject::FromVar(object)->SetAuxSlot(
                    SetSlotArguments(propertyId, propertyIndex, slotTypeBeforeSet, propertyValue));
        }
        Assert(!slotTypeBeforeSet.IsValueTypeMoreConvervativeThan(slotTypeAfterSet));

        if(objectScriptContext == requestContext)
        {
            DynamicTypeHandler *const typeHandler =
                DynamicObject::FromVar(object)->GetDynamicType()->GetTypeHandler();
            const PropertyIndex typeHandlerPropertyIndex =
                typeHandler->InlineOrAuxSlotIndexToPropertyIndex(propertyIndex, isInlineSlot);
            PropertyValueInfo::Set(propertyValueInfo, object, typeHandlerPropertyIndex, slotTypeAfterSet);

            CacheOperators::Cache<false, false, false>(
                false,
                DynamicObject::FromVar(object),
                false,
                object->GetType(),
                nullptr,
                propertyId,
                propertyIndex,
                isInlineSlot,
                false,
                0,
                propertyValueInfo,
                requestContext);
            return true;
        }

        if(operationInfo)
        {
            operationInfo->cacheType = CacheType_TypeProperty;
            operationInfo->slotLocation = isInlineSlot ? SlotLocation_Inline : SlotLocation_Aux;
            operationInfo->slotType = slotTypeAfterSet;
        }
        return true;
    }

    void TypePropertyCache::Cache(
        const PropertyId id,
        const PropertyIndex index,
        const ObjectSlotType slotType,
        const bool isInlineSlot,
        const bool isSetPropertyAllowed)
    {
        elements[ElementIndex(id)].Cache(id, index, slotType, isInlineSlot, isSetPropertyAllowed);
    }

    void TypePropertyCache::Cache(
        const PropertyId id,
        const PropertyIndex index,
        const ObjectSlotType slotType,
        const bool isInlineSlot,
        const bool isSetPropertyAllowed,
        const bool isMissing,
        DynamicObject *const prototypeObjectWithProperty,
        Type *const myParentType)
    {
        Assert(myParentType);
        Assert(myParentType->GetPropertyCache() == this);

        elements[ElementIndex(id)].Cache(
            id,
            index,
            slotType,
            isInlineSlot,
            isSetPropertyAllowed,
            isMissing,
            prototypeObjectWithProperty,
            myParentType);
    }

    void TypePropertyCache::ClearIfPropertyIsOnAPrototype(const PropertyId id)
    {
        TypePropertyCacheElement &element = elements[ElementIndex(id)];
        if(element.Id() == id && element.PrototypeObjectWithProperty())
            element.Clear();
    }

    void TypePropertyCache::Clear(const PropertyId id)
    {
        TypePropertyCacheElement &element = elements[ElementIndex(id)];
        if(element.Id() == id)
            element.Clear();
    }
}
