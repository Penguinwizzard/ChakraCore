//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "RuntimeTypePch.h"

namespace Js
{
    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    JavascriptEnumerator* DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::New(ScriptContext* scriptContext, DynamicObject* object)
    {
        DynamicObjectEnumerator* enumerator = RecyclerNew(scriptContext->GetRecycler(), DynamicObjectEnumerator, scriptContext);
        enumerator->Initialize(object);
        return enumerator;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    DynamicType *DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::GetTypeToEnumerate() const
    {
        return
            snapShotSemantics &&
            initialType->GetIsLocked() &&
            CONFIG_FLAG(TypeSnapshotEnumeration)
                ? initialType
                : object->GetDynamicType();
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    Var DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::GetCurrentIndex()
    {
        if (arrayEnumerator)
        {
            return arrayEnumerator->GetCurrentIndex();
        }

        FixObjectSlotIndexIfNecessary();
        JavascriptString* propertyString = nullptr;
        PropertyId propertyId = Constants::NoProperty;
        if (!object->FindNextProperty(objectIndex, &propertyString, &propertyId, nullptr, GetTypeToEnumerate(), !enumNonEnumerable, enumSymbols))
        {
            return this->GetLibrary()->GetUndefined();
        }

        Assert(propertyId == Constants::NoProperty || !Js::IsInternalPropertyId(propertyId));

        return propertyString;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    Var DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::GetCurrentValue()
    {
        if (arrayEnumerator)
        {
            return arrayEnumerator->GetCurrentValue();
        }

        FixObjectSlotIndexIfNecessary();
        return object->GetNextProperty(objectIndex, GetTypeToEnumerate(), !enumNonEnumerable, enumSymbols);
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    BOOL DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::MoveNext(PropertyAttributes* attributes)
    {
        PropertyId propId;
        return GetCurrentAndMoveNext(propId, attributes) != NULL;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    bool DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::GetCurrentPropertyId(PropertyId *pPropertyId)
    {
        if (arrayEnumerator)
        {
            return arrayEnumerator->GetCurrentPropertyId(pPropertyId);
        }
        Js::PropertyId propertyId = objectPropertyId;

        if ((enumNonEnumerable || (propertyId != Constants::NoProperty && object->IsEnumerable(propertyId))))
        {
            *pPropertyId = propertyId;
            return true;
        }
        else
        {
            return false;
        }
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    uint32 DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::GetCurrentItemIndex()
    {
        if (arrayEnumerator)
        {
            return arrayEnumerator->GetCurrentItemIndex();
        }
        else
        {
            return JavascriptArray::InvalidIndex;
        }
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    void DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::Reset()
    {
        ResetHelper();
    }

    // Initialize (or reuse) this enumerator for a given object.
    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    void DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::Initialize(DynamicObject* object)
    {
        this->object = object;
        ResetHelper();
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    Var DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::GetCurrentAndMoveNext(PropertyId& propertyId, PropertyAttributes* attributes)
    {
        if (arrayEnumerator)
        {
            Var currentIndex = arrayEnumerator->GetCurrentAndMoveNext(propertyId, attributes);
            if(currentIndex != NULL)
            {
                return currentIndex;
            }
            arrayEnumerator = NULL;
        }

        FixObjectSlotIndexIfNecessary();
        JavascriptString* propertyString;
        do
        {
            objectIndex++;
            propertyString = nullptr;
            if (!object->FindNextProperty(objectIndex, &propertyString, &propertyId, attributes, GetTypeToEnumerate(), !enumNonEnumerable, enumSymbols))
            {
                // No more properties
                objectIndex--;
                break;
            }
        }
        while (Js::IsInternalPropertyId(propertyId));

        objectPropertyId = propertyId;
        return propertyString;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    void DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::FixObjectSlotIndexIfNecessary()
    {
        // This function should be called before passing 'objectIndex' to an object or type handler to handle type changes

        if(snapShotSemantics ||
            !ObjectSlotType::RequiresWideSlotSupport() ||
            !previousPathTypeHandlerWithNativeFields ||
            objectIndex == static_cast<T>(-1))
        {
            return;
        }

        DynamicTypeHandler *const typeHandler = object->GetTypeHandler();
        if(typeHandler == previousPathTypeHandlerWithNativeFields)
            return;

        if(typeHandler->IsPathTypeHandler())
        {
            PathTypeHandler *const pathTypeHandler = PathTypeHandler::FromTypeHandler(typeHandler);
            if(pathTypeHandler->DoNativeFields())
            {
                previousPathTypeHandlerWithNativeFields = pathTypeHandler;
                return;
            }
        }

        // The object's type handler changed from a PathTypeHandler with native fields enabled, to some other type handler that
        // does not have native fields enabled, when the architecture requires wide slots. Such a type change will narrow all of
        // the wide slots, along with changing the slot types to Var. Since the enumerator tracks the current slot index, fix
        // the slot index for the new type handler.
        TypePath *const typePath = previousPathTypeHandlerWithNativeFields->GetTypePath();
        PropertyIndex oldSlotIndex = 0, newSlotIndex = 0;
        while(oldSlotIndex < objectIndex)
        {
            ++newSlotIndex;
            oldSlotIndex = typePath->GetNextSlotIndex(oldSlotIndex);
        }
        Assert(oldSlotIndex == objectIndex);
        objectIndex = newSlotIndex;
        previousPathTypeHandlerWithNativeFields = nullptr;
    }

    template <typename T, bool enumNonEnumerable, bool enumSymbols, bool snapShotSemantics>
    void DynamicObjectEnumerator<T, enumNonEnumerable, enumSymbols, snapShotSemantics>::ResetHelper()
    {
        if (object->HasObjectArray())
        {
            // Pass "object" as originalInstance to objectArray enumerator
            BOOL result = object->GetObjectArrayOrFlagsAsArray()->GetEnumerator(object, enumNonEnumerable, (Var*)&arrayEnumerator, GetScriptContext(), snapShotSemantics, enumSymbols);
            Assert(result);
        }
        else
        {
            arrayEnumerator = nullptr;
        }
        initialType = object->GetDynamicType();

        previousPathTypeHandlerWithNativeFields = nullptr;
        if(!snapShotSemantics && ObjectSlotType::RequiresWideSlotSupport() && object->GetScriptContext()->DoNativeFields())
        {
            DynamicTypeHandler *const typeHandler = initialType->GetTypeHandler();
            if(typeHandler->IsPathTypeHandler())
            {
                PathTypeHandler *const pathTypeHandler = PathTypeHandler::FromTypeHandler(typeHandler);
                if(pathTypeHandler->DoNativeFields())
                    previousPathTypeHandlerWithNativeFields = pathTypeHandler;
            }
        }

        objectPropertyId = Constants::NoProperty;
        objectIndex = (T)-1; // This is Constants::NoSlot or Constants::NoBigSlot
    }

    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/true, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/true, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/true, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/true, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/true, /*snapShotSemantics*/true>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/true, /*snapShotSemantics*/true>;
    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/false, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/false, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/false, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/false, /*snapShotSemantics*/false>;
    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/false, /*snapShotSemantics*/true>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/false, /*enumSymbols*/false, /*snapShotSemantics*/true>;
    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/false, /*snapShotSemantics*/true>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/false, /*snapShotSemantics*/true>;
    template class DynamicObjectEnumerator<PropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/true, /*snapShotSemantics*/true>;
    template class DynamicObjectEnumerator<BigPropertyIndex, /*enumNonEnumerable*/true, /*enumSymbols*/true, /*snapShotSemantics*/true>;
}
