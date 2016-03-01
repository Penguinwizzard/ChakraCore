// Copyright (C) Microsoft. All rights reserved.

#include "RuntimeTypePch.h"

namespace Js
{
    ObjectSlotIterator::ObjectSlotIterator(Type *const type)
        :
        slotIndex(0)
    #if DBG
        ,
        slotType(ObjectSlotType::GetVar())
    #endif
    {
        Initialize(type, nullptr);
    }

    ObjectSlotIterator::ObjectSlotIterator(DynamicType *const type, const BigPropertyIndex slotIndex)
        :
        slotIndex(slotIndex)
    #if DBG
        ,
        slotType(ObjectSlotType::GetVar())
    #endif
    {
        Initialize(type, nullptr);
    }

    ObjectSlotIterator::ObjectSlotIterator(RecyclableObject *const object)
        :
        slotIndex(0)
    #if DBG
        ,
        slotType(ObjectSlotType::GetVar())
    #endif
    {
        Assert(object);
        Initialize(object->GetType(), object);
    }

    ObjectSlotIterator::ObjectSlotIterator(DynamicObject *const object, const BigPropertyIndex slotIndex)
        :
        slotIndex(slotIndex)
    #if DBG
        ,
        slotType(ObjectSlotType::GetVar())
    #endif
    {
        Assert(object);
        Assert(slotIndex >= 0);

        Initialize(object->GetDynamicType(), object);
    }

    void ObjectSlotIterator::Initialize(Type *const type, RecyclableObject *const object)
    {
        Assert(type);
        Assert(!object || type == object->GetType());

        if(!DynamicType::Is(type->GetTypeId()))
        {
            typeHandler = nullptr;
            Assert(object->GetPropertyCount() == 0);
            propertyCount = slotCount = 0;
            return;
        }

        Initialize(static_cast<DynamicType *>(type), static_cast<DynamicObject *>(object));
    }

    void ObjectSlotIterator::Initialize(DynamicType *const type, DynamicObject *const object)
    {
        Assert(type);
        Assert(!object || type == object->GetType());

        typeHandler = type->GetTypeHandler();
        if(typeHandler->IsPathTypeHandler())
        {
            typePath = PathTypeHandler::FromTypeHandler(typeHandler)->GetSlotIterationInfo(&slotCount, &propertyCount);
            Assert(typePath || slotCount == propertyCount);
        }
        else
        {
            typePath = nullptr;
            if(object)
                typeHandler->EnsureObjectReady(object);
            propertyCount = slotCount = typeHandler->GetPropertyCount();
            Assert(typeHandler->GetSlotCount() == slotCount);
        }

        Assert(slotIndex <= slotCount);
        if(IsValid())
            DebugOnly(slotType = CurrentSlotType());
    }

    DynamicTypeHandler *ObjectSlotIterator::GetTypeHandler() const
    {
        return typeHandler;
    }

    BigPropertyIndex ObjectSlotIterator::GetSlotCount() const
    {
        return slotCount;
    }

    BigPropertyIndex ObjectSlotIterator::GetPropertyCount() const
    {
        return propertyCount;
    }

    bool ObjectSlotIterator::IsValid() const
    {
        if(slotIndex >= GetSlotCount())
            return false;

        Assert(typeHandler->GetSlotCount() == GetSlotCount());
        Assert(typeHandler->GetPropertyCount() == GetPropertyCount());
        return true;
    }

    void ObjectSlotIterator::MoveNext()
    {
        Assert(IsValid());
        Assert(CurrentSlotType() == slotType);

        SetCurrentSlotIndex(typePath ? typePath->GetNextSlotIndex(static_cast<PropertyIndex>(slotIndex)) : slotIndex + 1);
    }

    BigPropertyIndex ObjectSlotIterator::CurrentSlotIndex() const
    {
        Assert(IsValid());
        return slotIndex;
    }

    void ObjectSlotIterator::SetCurrentSlotIndex(const BigPropertyIndex slotIndex)
    {
        Assert(slotIndex >= 0);
        Assert(slotIndex <= GetSlotCount());

        this->slotIndex = slotIndex;
        if(IsValid())
            DebugOnly(slotType = CurrentSlotType());
    }

    void ObjectSlotIterator::SetCurrentSlotIndexFromPropertyIndex(const BigPropertyIndex propertyIndex)
    {
        Assert(propertyIndex >= 0);
        Assert(propertyIndex <= GetPropertyCount());

        if(!typePath)
        {
            SetCurrentSlotIndex(propertyIndex);
            return;
        }

        if(propertyIndex == GetPropertyCount())
        {
            SetCurrentSlotIndex(GetSlotCount());
            return;
        }

        PropertyIndex si = 0;
        BigPropertyIndex pi = 0;
        while(pi < propertyIndex)
        {
            si = typePath->GetNextSlotIndex(si);
            ++pi;
        }
        Assert(static_cast<BigPropertyIndex>(si) >= pi);
        SetCurrentSlotIndex(si);
    }

    ObjectSlotType ObjectSlotIterator::CurrentSlotType() const
    {
        Assert(IsValid());
        Assert(!typePath || slotIndex < typePath->GetSlotCount());

        return typePath ? typePath->GetSlotType(static_cast<PropertyIndex>(slotIndex)) : ObjectSlotType::GetVar();
    }

    PropertyId ObjectSlotIterator::CurrentPropertyId(ScriptContext *const scriptContext) const
    {
        Assert(IsValid());
        Assert(!typePath || slotIndex < typePath->GetSlotCount());

        return
            typePath
                ? typePath->GetPropertyId(static_cast<PropertyIndex>(slotIndex))->GetPropertyId()
                : typeHandler->GetPropertyId(scriptContext, slotIndex);
    }
}
