\//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// JScriptDiag does not link with Runtime.lib and does not include .cpp files, so this file will be included as a header
#pragma once

#include "RuntimeTypePch.h"

namespace Js
{
    PathTypeHandler::PathTypeHandler(
        PathTypeTransitionInfo *const transitionInfo,
        const PropertyIndex slotCount,
        const PropertyIndex propertyCount,
        const PropertyIndex slotCapacity,
        uint16 inlineSlotCapacity,
        uint16 offsetOfInlineSlots,
        const bool disableNativeFields,
        bool isShared)
        :
        DynamicTypeHandler(slotCapacity, inlineSlotCapacity, offsetOfInlineSlots, DefaultFlags | IsLockedFlag | MayBecomeSharedFlag | (isShared ? IsSharedFlag : 0)),
        slotCount(static_cast<PathTypePropertyIndex>(slotCount)),
        propertyCount(static_cast<PathTypePropertyIndex>(propertyCount)),
        disableNativeFields(disableNativeFields),
        hasWastedInlineSlot(false),
        transitionInfo(transitionInfo)
    {
        Assert(static_cast<PathTypePropertyIndex>(slotCount) == slotCount);
        Assert(static_cast<PathTypePropertyIndex>(propertyCount) == propertyCount);
        Assert(propertyCount <= slotCount);
        Assert(slotCount == propertyCount || ObjectSlotType::RequiresWideSlotSupport() && !disableNativeFields);
        Assert(slotCount <= GetTypePath()->GetSlotCount());
        Assert(slotCount <= slotCapacity);
        Assert(inlineSlotCapacity <= slotCapacity);
        DynamicType *predecessorType = transitionInfo->GetPredecessor() ? transitionInfo->GetPredecessor()->GetType() : nullptr;
        isNotPathTypeHandlerOrHasUserDefinedCtor = predecessorType == nullptr ? false : predecessorType->GetTypeHandler()->GetIsNotPathTypeHandlerOrHasUserDefinedCtor();

        // IsLockedFlag
        //     - PathTypeHandlers and their corresponding types are always locked. This flag is more meaningful for other type
        //       handlers that can mutate upon adding a property, to disallow the mutation when isLocked == true.
    }

    BigPropertyIndex PathTypeHandler::GetSlotCount()
    {
        return GetSlotCountInternal();
    }

    int PathTypeHandler::GetPropertyCount()
    {
        return GetPropertyCountInternal();
    }

    BigPropertyIndex PathTypeHandler::GetSlotCountAndPropertyCount(int *const propertyCountRef)
    {
        Assert(propertyCountRef);

        *propertyCountRef = GetPropertyCountInternal();
        return GetSlotCountInternal();
    }

    PropertyId PathTypeHandler::GetPropertyId(ScriptContext* scriptContext, PropertyIndex index)
    {
        if (index < GetSlotCountInternal())
        {
            return GetTypePath()->GetPropertyId(index)->GetPropertyId();
        }
        else
        {
            return Constants::NoProperty;
        }
    }

    PropertyId PathTypeHandler::GetPropertyId(ScriptContext* scriptContext, BigPropertyIndex index)
    {
        if (index < GetSlotCountInternal())
        {
            return GetTypePath()->GetPropertyId(static_cast<PropertyIndex>(index))->GetPropertyId();
        }
        else
        {
            return Constants::NoProperty;
        }
    }

    BOOL PathTypeHandler::FindNextProperty(ScriptContext* scriptContext, PropertyIndex& index, JavascriptString** propertyStringName, PropertyId* propertyId, 
        PropertyAttributes* attributes, Type* type, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols)
    {
        Assert(propertyStringName);
        Assert(propertyId);
        Assert(type);

        TypePath *const typePath = GetTypePath();

        if(ObjectSlotType::RequiresWideSlotSupport() &&
            index != 0 &&
            index < GetSlotCountInternal() &&
            !typePath->IsValidSlotIndex(index))
        {
            // This function is used in different ways. The index passed in may be one of the following:
            // - The previously returned index. The purpose is to return the same index, along with the property info.
            // - The previously returned index + 1. The purpose is to find the next index.
            //
            // Due to this difference, when wide slots may be involved, the index to start the search from needs to be adjusted
            // if it currently points to the middle of a wide slot, where the intention is to find the next property.
            index = typePath->GetNextSlotIndex(index - 1);
        }

        for (; index < GetSlotCountInternal(); index = typePath->GetNextSlotIndex(index))
        {
            const PropertyRecord* propertyRecord = typePath->GetPropertyId(index);

            // Skip this property if it is a symbol and we are not including symbol properties
            if (!enumSymbols && propertyRecord->IsSymbol())
            {
                continue;
            }

            if (attributes)
            {
                *attributes = PropertyEnumerable;
            }

            *propertyId = propertyRecord->GetPropertyId();
            PropertyString* propertyString = type->GetScriptContext()->GetPropertyString(*propertyId);
            *propertyStringName = propertyString;

            uint16 inlineOrAuxSlotIndex;
            bool isInlineSlot;
            PropertyIndexToInlineOrAuxSlotIndex(index, &inlineOrAuxSlotIndex, &isInlineSlot);

            propertyString->UpdateCache(
                type,
                inlineOrAuxSlotIndex,
                typePath->GetSlotType(index),
                isInlineSlot,
                CanStorePropertyValueDirectly(index));

            return TRUE;
        }

        return FALSE;
    }

    PropertyIndex PathTypeHandler::GetPropertyIndex(const PropertyRecord* propertyRecord)
    {
        return GetTypePath()->LookupInline(propertyRecord->GetPropertyId(), GetSlotCountInternal());
    }

    PropertyIndex PathTypeHandler::GetPropertyIndex(PropertyId propertyId)
    {
        return GetTypePath()->LookupInline(propertyId, GetSlotCountInternal());
    }

    ObjectSlotType PathTypeHandler::GetSlotType(const BigPropertyIndex bigSlotIndex) const
    {
        const PropertyIndex slotIndex = static_cast<PropertyIndex>(bigSlotIndex);
        Assert(static_cast<BigPropertyIndex>(slotIndex) == bigSlotIndex);
        Assert(slotIndex < GetSlotCountInternal());
        const ObjectSlotType slotType = GetTypePath()->GetSlotType(slotIndex);
        Assert(DoNativeFields() || slotType == ObjectSlotType::GetVar());
        return slotType;
    }

    bool PathTypeHandler::GetPropertyEquivalenceInfo(PropertyRecord const* propertyRecord, PropertyEquivalenceInfo& info) 
    {
        TypePath *const typePath = GetTypePath();
        Js::PropertyIndex absSlotIndex = typePath->LookupInline(propertyRecord->GetPropertyId(), GetSlotCountInternal());
        info.slotIndex = AdjustSlotIndexForInlineSlots(absSlotIndex);
        info.slotType = typePath->GetSlotType(absSlotIndex);
        info.isAuxSlot = absSlotIndex >= GetInlineSlotCapacity();
        info.isWritable = info.slotIndex != Constants::NoSlot;
        return info.slotIndex != Constants::NoSlot;
    }

#if ENABLE_NATIVE_CODEGEN
    IR::BailOutKind PathTypeHandler::IsObjTypeSpecEquivalent(DynamicObject *const object, const TypeEquivalenceRecord& record, uint& failedPropertyIndex)
    {
        uint propertyCount = record.propertyCount;
        Js::EquivalentPropertyEntry* properties = record.properties;
        PropertyIdToSlotTypeMap propertyIdToNewSlotTypeMap(object->GetRecycler());
        for (uint pi = 0; pi < propertyCount; pi++)
        {
            const EquivalentPropertyEntry* entry = &properties[pi];
            bool mayBeEquivalentWithSlotTypeChange = false;
            if (!IsObjTypeSpecEquivalent(object, entry, &mayBeEquivalentWithSlotTypeChange, &propertyIdToNewSlotTypeMap) &&
                !mayBeEquivalentWithSlotTypeChange)
            {
                failedPropertyIndex = pi;
                return IR::BailOutFailedEquivalentTypeCheck;
            }
        }

        if(propertyIdToNewSlotTypeMap.Count() != 0)
        {
            // Not equivalent, but may be equivalent after changing slot types. Change the slot types and reevaluate.
            ChangeSlotTypes(object, &propertyIdToNewSlotTypeMap);
            PathTypeHandler *const newTypeHandler = FromTypeHandler(object->GetTypeHandler());
            for(uint pi = 0; pi < propertyCount; pi++)
            {
                const EquivalentPropertyEntry* entry = &properties[pi];
                if(!newTypeHandler->IsObjTypeSpecEquivalent(object, entry))
                {
                    failedPropertyIndex = pi;
                    return IR::BailOutFailedEquivalentTypeCheck;
                }
            }

            // Since the new type is equivalent to what is expected, there's no need to rejit. Jitted code still needs to bail
            // out though, as it does not expect a type change.
            return IR::BailOutOnUnexpectedSlotTypeChange;
        }

        return IR::BailOutInvalid;
    }
#endif

    bool PathTypeHandler::IsObjTypeSpecEquivalent(
        DynamicObject *const object,
        const EquivalentPropertyEntry *entry,
        bool *const mayBeEquivalentWithSlotTypeChangeRef,
        PropertyIdToSlotTypeMap *const propertyIdToNewSlotTypeMap)
    {
        Assert(!mayBeEquivalentWithSlotTypeChangeRef || !*mayBeEquivalentWithSlotTypeChangeRef);
        Assert(!propertyIdToNewSlotTypeMap == !mayBeEquivalentWithSlotTypeChangeRef);

        TypePath *const typePath = GetTypePath();
        Js::PropertyIndex absSlotIndex = typePath->LookupInline(entry->propertyId, GetSlotCountInternal());

        if (absSlotIndex != Constants::NoSlot)
        {
            Js::PropertyIndex relSlotIndex = AdjustValidSlotIndexForInlineSlots(absSlotIndex);
            if (relSlotIndex != entry->slotIndex || (absSlotIndex >= GetInlineSlotCapacity()) != entry->isAuxSlot)
            {
                return false;
            }

            if (entry->mustBeWritable && !CanStorePropertyValueDirectly(absSlotIndex))
            {
                return false;
            }

            ObjectSlotType actualSlotType = typePath->GetSlotType(absSlotIndex);
            const ObjectSlotType expectedSlotType = entry->slotType;
            if(!actualSlotType.IsValueTypeEqualTo(expectedSlotType))
            {
                if(mayBeEquivalentWithSlotTypeChangeRef && expectedSlotType.IsValueTypeMoreConvervativeThan(actualSlotType))
                {
                    *mayBeEquivalentWithSlotTypeChangeRef = true;
                    propertyIdToNewSlotTypeMap->Add(entry->propertyId, expectedSlotType);
                }
                return false;
            }
        }
        else
        {
            if (entry->slotIndex != Constants::NoSlot || entry->mustBeWritable)
            {
                return false;
            }
        }

        return true;
    }

    BOOL PathTypeHandler::HasProperty(DynamicObject* instance, PropertyId propertyId, __out_opt bool *noRedecl)
    {
        uint32 indexVal;
        if (noRedecl != nullptr)
        {
            *noRedecl = false;
        }

        if (GetPropertyIndex(propertyId) != Constants::NoSlot)
        {
            return true;
        }

        // Check numeric propertyId only if objectArray is available
        ScriptContext* scriptContext = instance->GetScriptContext();
        if (instance->HasObjectArray() && scriptContext->IsNumericPropertyId(propertyId, &indexVal))
        {
            return HasItem(instance, indexVal);
        }

        return false;
    }

    BOOL PathTypeHandler::HasProperty(DynamicObject* instance, JavascriptString* propertyNameString)
    {
        // Consider: Implement actual string hash lookup
        PropertyRecord const* propertyRecord;
        instance->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return HasProperty(instance, propertyRecord->GetPropertyId());
    }

    BOOL PathTypeHandler::GetProperty(DynamicObject* instance, Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) 
    {
        TypePath *const typePath = GetTypePath();
        PropertyIndex index = typePath->LookupInline(propertyId, GetSlotCountInternal());
        if (index != Constants::NoSlot)
        {
            const ObjectSlotType slotType = typePath->GetSlotType(index);
            *value = instance->GetSlot(index, slotType);
            PropertyValueInfo::Set(info, instance, index, slotType);
            if (!CanStorePropertyValueDirectly(index))
            {
                PropertyValueInfo::DisableStoreFieldCache(info);
            }
            return true;
        }

        // Check numeric propertyId only if objectArray available
        uint32 indexVal;
        ScriptContext* scriptContext = instance->GetScriptContext();
        if (instance->HasObjectArray() && scriptContext->IsNumericPropertyId(propertyId, &indexVal))
        {
            return GetItem(instance, originalInstance, indexVal, value, requestContext);
        }

        return false;
    }

    BOOL PathTypeHandler::GetProperty(DynamicObject* instance, Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) 
    {
        // Consider: Implement actual string hash lookup
        Assert(requestContext);
        PropertyRecord const* propertyRecord;
        wchar_t const * propertyName = propertyNameString->GetString();
        charcount_t const propertyNameLength = propertyNameString->GetLength();

        if (instance->HasObjectArray())
        {
            requestContext->GetOrAddPropertyRecord(propertyName, propertyNameLength, &propertyRecord);
        }
        else
        {
            requestContext->FindPropertyRecord(propertyName, propertyNameLength, &propertyRecord);
            if (propertyRecord == nullptr)
            {
                return false;
            }
        }
        return GetProperty(instance, originalInstance, propertyRecord->GetPropertyId(), value, info, requestContext);
    }

    BOOL PathTypeHandler::SetProperty(DynamicObject* instance, PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        return SetPropertyInternal(instance, propertyId, value, info, flags, SideEffects_Any);
    }

    BOOL PathTypeHandler::SetProperty(DynamicObject* instance, JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info)
    {
        // Consider: Implement actual string hash lookup
        PropertyRecord const* propertyRecord;
        instance->GetScriptContext()->GetOrAddPropertyRecord(propertyNameString->GetString(), propertyNameString->GetLength(), &propertyRecord);
        return SetProperty(instance, propertyRecord->GetPropertyId(), value, flags, info);
    }

    BOOL PathTypeHandler::SetPropertyInternal(DynamicObject* instance, PropertyId propertyId, Var value, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        // Path type handler doesn't support pre-initialization (PropertyOperation_PreInit). Pre-initialized properties
        // will get marked as fixed when pre-initialized and then as non-fixed when their actual values are set.

        Assert(instance->GetTypeHandler() == this);
        Assert(value != nullptr || IsInternalPropertyId(propertyId));
        VerifySlotCapacities();

        PropertyIndex index = GetPropertyIndex(propertyId);
        
        if (index != Constants::NoSlot)
        {
            // If type is shared then the handler must be shared as well.  This is a weaker invariant than in AddPropertyInternal,
            // because the type coming in here may be the result of DynamicObject::ChangeType(). In that case the handler may have
            // already been shared, but the newly created type isn't - and likely never will be - shared (is typically unreachable).
            // In CacheOperators::CachePropertyWrite we ensure that we never cache property adds for types that aren't shared.
            Assert(!instance->GetDynamicType()->GetIsShared() || GetIsShared());

            // Setting the slot may cause the field type to change, which in turn would change the type and handler
            const ObjectSlotType slotTypeBeforeSet = GetTypePath()->GetSlotType(index);
            SetSlotUnchecked(instance, index, slotTypeBeforeSet, value);

            // Ensure that all subsequent state changes are made to the new type or handler
            PathTypeHandler *const newTypeHandler =
                slotTypeBeforeSet.IsVar() ? this : FromTypeHandler(instance->GetDynamicType()->GetTypeHandler());
            Assert(newTypeHandler == instance->GetDynamicType()->GetTypeHandler());
            Assert((newTypeHandler == this) == (newTypeHandler->GetTypePath()->GetSlotType(index) == slotTypeBeforeSet));
            Assert(!instance->GetDynamicType()->GetIsShared() || newTypeHandler->GetIsShared());

            newTypeHandler->SetExistingProperty_UpdateStateAfterSet(
                instance,
                propertyId,
                index,
                value,
                newTypeHandler != this,
                info,
                flags,
                possibleSideEffects);
            return true;
        }

        // Always check numeric propertyId. This may create an objectArray.
        ScriptContext* scriptContext = instance->GetScriptContext();
        uint32 indexVal;
        if (scriptContext->IsNumericPropertyId(propertyId, &indexVal))
        {
            return SetItem(instance, indexVal, value, PropertyOperation_None);
        }

        return AddPropertyInternal(instance, propertyId, value, info, flags, possibleSideEffects);
    }

    void PathTypeHandler::SetExistingProperty_UpdateStateAfterSet(
        DynamicObject *const instance,
        const PropertyId propertyId,
        const PropertyIndex slotIndex,
        const Var value,
        const bool slotTypeWasChanged,
        PropertyValueInfo *const info,
        const PropertyOperationFlags flags,
        const SideEffects possibleSideEffects)
    {
        Assert(instance->GetTypeHandler() == this);

        bool populateInlineCache = true;
        if(FixPropsOnPathTypes())
        {
            Assert(!GetIsShared() || !GetTypePath()->GetIsFixedFieldAt(slotIndex, GetSlotCountInternal()));
            if(!GetIsShared())
            {
                if(!slotTypeWasChanged)
                {
                    ProcessFixedFieldChange(
                        instance,
                        propertyId,
                        slotIndex,
                        value,
                        (flags & PropertyOperation_NonFixedValue) != 0);
                }
                else if(GetTypePath()->GetIsFixedFieldAt(slotIndex, GetSlotCountInternal()))
                {
                    populateInlineCache = false;
                }
            }

            Assert(populateInlineCache == !GetTypePath()->GetIsFixedFieldAt(slotIndex, GetSlotCountInternal()));
            Assert(populateInlineCache == CanStorePropertyValueDirectly(slotIndex));
        }

        if(populateInlineCache)
        {
            Assert((instance->GetDynamicType()->GetIsShared()) || (FixPropsOnPathTypes() && GetIsOrMayBecomeShared()));
            // Can't assert the following.  With NewScObject we can jump to the type handler at the tip (where the singleton is),
            // even though we haven't yet initialized the properties all the way to the tip, and we don't want to kill
            // the singleton in that case yet.  It's basically a transient inconsistent state, but we have to live with it.
            // The user's code will never see the object in this state.
            //Assert(!instance->GetTypeHandler()->HasSingletonInstance());
            PropertyValueInfo::Set(info, instance, slotIndex, GetTypePath()->GetSlotType(slotIndex));
        }
        else
        {
            PropertyValueInfo::SetNoCache(info, instance);
        }

        SetPropertyUpdateSideEffect(instance, propertyId, value, possibleSideEffects);
        VerifySlotCapacities();
    }

    BOOL PathTypeHandler::DeleteProperty(DynamicObject* instance, PropertyId propertyId, PropertyOperationFlags flags)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount2++;
#endif
        // Check numeric propertyId only if objectArray available
        ScriptContext* scriptContext = instance->GetScriptContext();
        uint32 indexVal;
        if (instance->HasObjectArray() && scriptContext->IsNumericPropertyId(propertyId, &indexVal))
        {
            return DeleteItem(instance, indexVal, flags);
        }
        
        return  ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal())->DeleteProperty(instance, propertyId, flags);
    }

    BOOL PathTypeHandler::IsFixedProperty(const DynamicObject* instance, PropertyId propertyId)
    {
        if (!FixPropsOnPathTypes())
        {
            return false;
        }

        PropertyIndex index = GetPropertyIndex(propertyId);
        Assert(index != Constants::NoSlot);

        return this->GetTypePath()->GetIsFixedFieldAt(index, GetSlotCountInternal());
    }

    BOOL PathTypeHandler::IsEnumerable(DynamicObject* instance, PropertyId propertyId)
    {
        return true;
    }

    BOOL PathTypeHandler::IsWritable(DynamicObject* instance, PropertyId propertyId)
    {
        return true;
    }

    BOOL PathTypeHandler::IsConfigurable(DynamicObject* instance, PropertyId propertyId)
    {
        return true;
    }

    BOOL PathTypeHandler::SetEnumerable(DynamicObject* instance, PropertyId propertyId, BOOL value)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount3++;
#endif
        return value || ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal())->SetEnumerable(instance, propertyId, value);
    }

    BOOL PathTypeHandler::SetWritable(DynamicObject* instance, PropertyId propertyId, BOOL value)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount3++;
#endif
        return value || ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal())->SetWritable(instance, propertyId, value);
    }

    BOOL PathTypeHandler::SetConfigurable(DynamicObject* instance, PropertyId propertyId, BOOL value)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount3++;
#endif
        return value || ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal())->SetConfigurable(instance, propertyId, value);
    }

    BOOL PathTypeHandler::SetAccessors(DynamicObject* instance, PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount4++;
#endif
        return ConvertToDictionaryType(instance)->SetAccessors(instance, propertyId, getter, setter, flags);
    }

    BOOL PathTypeHandler::PreventExtensions(DynamicObject* instance)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount4++;
#endif
        BOOL tempResult = this->ConvertToSharedNonExtensibleTypeIfNeededAndCallOperation(instance, InternalPropertyRecords::NonExtensibleType,
            [&](SimpleDictionaryTypeHandlerWithNontExtensibleSupport* newTypeHandler)
            {
                return newTypeHandler->PreventExtensionsInternal(instance);
            });

        Assert(tempResult);
        if (tempResult)
        {
            // Call preventExtensions on the objectArray -- which will create different type for array type handler.
            ArrayObject * objectArray = instance->GetObjectArray();
            if (objectArray)
            {
                objectArray->PreventExtensions();
            }
        }

        return tempResult;
    }

    BOOL PathTypeHandler::Seal(DynamicObject* instance)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount4++;
#endif
        // For seal we need an array with non-default attributes, which is ES5Array,
        // and in current design ES5Array goes side-by-side with DictionaryTypeHandler.
        // Note that 2 instances can have same PathTypehandler but still different objectArray items, e.g. {x:0, 0:0} and {x:0, 1:0}.
        // Technically we could change SimpleDictionaryTypehandler to override *Item* methods,
        // similar to DictionaryTypeHandler, but objects with numeric properties are currently seen as low priority,
        // so just don't share the type.
        if (instance->HasObjectArray())
        {
            return this->ConvertToDictionaryType(instance)->Seal(instance);
        }
        else
        {
            return this->ConvertToSharedNonExtensibleTypeIfNeededAndCallOperation(instance, InternalPropertyRecords::SealedType,
                [&](SimpleDictionaryTypeHandlerWithNontExtensibleSupport* newTypeHandler)
                {
                    return newTypeHandler->SealInternal(instance);
                });
        }
    }

    BOOL PathTypeHandler::FreezeImpl(DynamicObject* instance, bool isConvertedType)
    {
#ifdef PROFILE_TYPES
        instance->GetScriptContext()->convertPathToDictionaryCount4++;
#endif
        // See the comment inside Seal WRT HasObjectArray branch.
        if (instance->HasObjectArray())
        {
            return this->ConvertToDictionaryType(instance)->Freeze(instance, isConvertedType);
        }
        else
        {
            return this->ConvertToSharedNonExtensibleTypeIfNeededAndCallOperation(instance, InternalPropertyRecords::FrozenType,
                [&](SimpleDictionaryTypeHandlerWithNontExtensibleSupport* newTypeHandler)
                {
                    return newTypeHandler->FreezeInternal(instance, true);  // true: we don't want to change type in FreezeInternal.
                });
        }
    }

    // Checks whether conversion to shared type is needed and performs it, then calls actual operation on the shared type.
    // Template method used for PreventExtensions, Seal, Freeze.
    // Parameters:
    // - instance: object instance to operate on.
    // - operationInternalPropertyRecord: the internal property record for preventExtensions/seal/freeze.
    // - FType: functor/lambda to perform actual forced operation (such as PreventExtensionsInternal) on the shared type.
    template<typename FType> 
    BOOL PathTypeHandler::ConvertToSharedNonExtensibleTypeIfNeededAndCallOperation(DynamicObject* instance, const PropertyRecord* operationInternalPropertyRecord, FType operation)
    {
        AssertMsg(operationInternalPropertyRecord == InternalPropertyRecords::NonExtensibleType ||
            operationInternalPropertyRecord == InternalPropertyRecords::SealedType ||
            operationInternalPropertyRecord == InternalPropertyRecords::FrozenType,
            "Wrong/unsupported value of operationInternalPropertyRecord.");

        // See if we already have shared type for this type and convert to it, otherwise create a new one.
        PathTypeTransitionInfo *transitionInfo = GetTransitionInfo();
        const PathTypeSuccessorKey successorKey(operationInternalPropertyRecord->GetPropertyId(), ObjectSlotType::GetVar());
        PathTypeTransitionInfo *const successor = transitionInfo->GetSuccessor(successorKey);
        if (!successor)
        {
            // Convert to new shared type with shared simple dictionary type handler and call operation on it.
            SimpleDictionaryTypeHandlerWithNontExtensibleSupport* newTypeHandler = ConvertToSimpleDictionaryType
                <SimpleDictionaryTypeHandlerWithNontExtensibleSupport>(instance, this->GetPropertyCountInternal(), true);

            Assert(newTypeHandler->GetMayBecomeShared() && !newTypeHandler->GetIsShared());
            DynamicType* newType = instance->GetDynamicType();
            newType->LockType();
            Assert(!newType->GetIsShared());

            ScriptContext * scriptContext = instance->GetScriptContext();
            Recycler * recycler = scriptContext->GetRecycler();
            transitionInfo->SetSuccessor(
                successorKey,
                PathTypeSingleSuccessorTransitionInfo::New(nullptr, newType, transitionInfo, recycler),
                recycler);
            transitionInfo = GetTransitionInfo();
            return operation(newTypeHandler);
        }
        else
        {
            DynamicType* newType = successor->GetType();
            DynamicTypeHandler* newTypeHandler = newType->GetTypeHandler();

            // Consider: Consider doing something special for frozen objects, whose values cannot
            // change and so we could retain them as fixed, even when the type becomes shared.
            newType->ShareType();
            // Consider: If we isolate prototypes, we should never get here with the prototype flag set.
            // There should be nothing to transfer.
            // Assert(!IsolatePrototypes() || (this->GetFlags() & IsPrototypeFlag) == 0);
            newTypeHandler->SetFlags(IsPrototypeFlag, this->GetFlags());
            Assert(!newTypeHandler->HasSingletonInstance());

            if(instance->IsObjectHeaderInlinedTypeHandler())
            {
                const PropertyIndex newInlineSlotCapacity = newTypeHandler->GetInlineSlotCapacity();
                AdjustSlots(instance, newInlineSlotCapacity, newTypeHandler->GetSlotCapacity() - newInlineSlotCapacity);
            }
            ReplaceInstanceType(instance, newType);

            ChangeAllSlotStorageToVar(instance);
        }

        return TRUE;
    }

    DynamicType* PathTypeHandler::PromoteType(
        DynamicObject* instance,
        const PropertyRecord* propertyRecord,
        const ObjectSlotType slotType,
        const Var value,
        const PropertyOperationFlags flags)
    {
        ScriptContext* scriptContext = instance->GetScriptContext();

        DynamicType* nextType = this->PromoteType(instance, propertyRecord, slotType, value, flags, scriptContext);
        PathTypeHandler* nextPath = (PathTypeHandler*) nextType->GetTypeHandler();

        instance->EnsureSlots(this->GetSlotCapacity(), nextPath->GetSlotCapacity(), scriptContext, nextType->GetTypeHandler());

        ReplaceInstanceType(instance, nextType);
        return nextType;
    }

    void PathTypeHandler::GetOriginalInlineSlotCapacityAndSlotCapacity(
        PropertyIndex *const inlineSlotCapacityRef,
        PropertyIndex *const slotCapacityRef) const
    {
        Assert(inlineSlotCapacityRef);
        Assert(slotCapacityRef);

        PropertyIndex inlineSlotCapacity = GetInlineSlotCapacity();
        PropertyIndex slotCapacity = static_cast<PropertyIndex>(GetSlotCapacity());
        if(ObjectSlotType::RequiresWideSlotSupport() && HasWastedInlineSlot())
        {
            // Due to a wide native field, the inline slot capacity was adjusted. Recover the original slot capacities.
            Assert(GetTypePath()->IsLastInlineSlotWasted(inlineSlotCapacity + 1));
            ++inlineSlotCapacity;
            ++slotCapacity;
        }

        Assert(inlineSlotCapacity <= slotCapacity);
        *inlineSlotCapacityRef = inlineSlotCapacity;
        *slotCapacityRef = slotCapacity;
    }

    template <typename T>
    T* PathTypeHandler::ConvertToTypeHandler(DynamicObject* instance)
    {
        Assert(instance);
        ScriptContext* scriptContext = instance->GetScriptContext();
        Recycler* recycler = scriptContext->GetRecycler();
                
        PathTypeHandler * oldTypeHandler;

        // Ideally 'this' and oldTypeHandler->GetTypeHandler() should be same
        // But we can have calls from external DOM objects, which requests us to replace the type of the
        // object with a new type. And in such cases, this API gets called with oldTypeHandler and the
        // new type (obtained from the External DOM object)
        // We use the duplicated typeHandler, if we deOptimized the object successfully, else we retain the earlier
        // behavior of using 'this' pointer.

        VerifySlotCapacities();
        if (instance->DeoptimizeObjectHeaderInlining())
        {
            oldTypeHandler = FromTypeHandler(instance->GetTypeHandler());
            VerifySlotCapacities();
            oldTypeHandler->VerifySlotCapacities();
        }
        else
        {
            oldTypeHandler = this;
        }

        Assert(oldTypeHandler);

        PropertyIndex inlineSlotCapacity, slotCapacity;
        oldTypeHandler->GetOriginalInlineSlotCapacityAndSlotCapacity(&inlineSlotCapacity, &slotCapacity);

        T* newTypeHandler = RecyclerNew(recycler, T, recycler, slotCapacity, inlineSlotCapacity, oldTypeHandler->GetOffsetOfInlineSlots());
        // We expect the new type handler to start off marked as having only writable data properties.
        Assert(newTypeHandler->GetHasOnlyWritableDataProperties());

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        DynamicType* oldType = instance->GetDynamicType();
        RecyclerWeakReference<DynamicObject>* oldSingletonInstance = oldTypeHandler->GetSingletonInstance();
        oldTypeHandler->TraceFixedFieldsBeforeTypeHandlerChange(L"converting", L"PathTypeHandler", L"DictionaryTypeHandler", instance, oldTypeHandler, oldType, oldSingletonInstance);
#endif

        bool const canBeSingletonInstance = DynamicTypeHandler::CanBeSingletonInstance(instance);
        // If this type had been installed on a stack instance it shouldn't have a singleton Instance
        Assert(canBeSingletonInstance || !oldTypeHandler->HasSingletonInstance());

        // This instance may not be the singleton instance for this handler. There may be a singleton at the tip
        // and this instance may be getting initialized via an object literal and one of the properties may
        // be an accessor.  In this case we will convert to a DictionaryTypeHandler and it's correct to
        // transfer this instance, even tough different from the singleton. Ironically, this instance
        // may even appear to be at the tip along with the other singleton, because the set of properties (by
        // name, not value) may be identical.
        // Consider: Consider threading PropertyOperation_Init through InitProperty and SetAccessors,
        // to be sure that we don't assert only in this narrow case.
        // Assert(this->GetTypePath()->GetSingletonInstance() == instance);

        Assert(oldTypeHandler->HasSingletonInstanceOnlyIfNeeded());

        // Don't install stack instance as singleton instance
        TypePath *const oldTypePath = oldTypeHandler->GetTypePath();
        if (canBeSingletonInstance)
        {
            if (DynamicTypeHandler::AreSingletonInstancesNeeded())
            {
                RecyclerWeakReference<DynamicObject>* curSingletonInstance = oldTypePath->GetSingletonInstance();
                if (curSingletonInstance != nullptr && curSingletonInstance->Get() == instance)
                {
                    newTypeHandler->SetSingletonInstance(curSingletonInstance);
                }
                else
                {
                    newTypeHandler->SetSingletonInstance(instance->CreateWeakReferenceToSelf());
                }
            }
        }

        bool transferFixed = canBeSingletonInstance;

        // If we are a prototype or may become a prototype we must transfer used as fixed bits.  See point 4 in ConvertToSimpleDictionaryType.
        Assert(!DynamicTypeHandler::IsolatePrototypes() || ((oldTypeHandler->GetFlags() & IsPrototypeFlag) == 0));
        bool transferUsedAsFixed = ((oldTypeHandler->GetFlags() & IsPrototypeFlag) != 0 || (oldTypeHandler->GetIsOrMayBecomeShared() && !DynamicTypeHandler::IsolatePrototypes()));

        Assert(oldTypeHandler->GetSlotCountInternal() <= oldTypePath->GetMaxInitializedSlotCount());
        for (PropertyIndex i = 0; i < oldTypeHandler->GetSlotCountInternal(); i = oldTypePath->GetNextSlotIndex(i))
        {
            // Consider: As noted in point 2 in ConvertToSimpleDictionaryType, when converting to non-shared handler we could be more
            // aggressive and mark every field as fixed, because we will always take a type transition. We have to remember to respect
            // the switches as to which kinds of properties we should fix, and for that we need the values from the instance. Even if
            // the type handler says the property is initialized, the current instance may not have a value for it. Check for value != null.
            if (FixPropsOnPathTypes())
            {
                newTypeHandler->Add(oldTypePath->GetPropertyId(i), PropertyDynamicTypeDefaults,
                    true,
                    transferFixed && oldTypePath->GetIsFixedFieldAt(i, oldTypeHandler->GetSlotCountInternal()),
                    transferUsedAsFixed && oldTypePath->GetIsUsedFixedFieldAt(i, oldTypeHandler->GetSlotCountInternal()),
                    scriptContext);
            }
            else
            {
                newTypeHandler->Add(oldTypePath->GetPropertyId(i), PropertyDynamicTypeDefaults, true, false, false, scriptContext);
            }
        }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        if (FixPropsOnPathTypes())
        {
            Assert(oldTypeHandler->HasSingletonInstanceOnlyIfNeeded());
            oldTypePath->ClearSingletonInstanceIfSame(instance);
        }
#endif

        // PathTypeHandlers are always shared, so if we're isolating prototypes, a PathTypeHandler should
        // never have the prototype flag set.
        Assert(!DynamicTypeHandler::IsolatePrototypes() || ((oldTypeHandler->GetFlags() & IsPrototypeFlag) == 0));
        AssertMsg(!newTypeHandler->GetIsPrototype(), "Why did we create a brand new type handler with a prototype flag set?");
        newTypeHandler->SetFlags(IsPrototypeFlag, oldTypeHandler->GetFlags());

        // Any new type handler we expect to see here should have inline slot capacity locked.  If this were to change, we would need
        // to update our shrinking logic (see ShrinkSlotAndInlineSlotCapacity).
        Assert(newTypeHandler->GetIsInlineSlotCapacityLocked());
        newTypeHandler->SetPropertyTypes(PropertyTypesWritableDataOnly | PropertyTypesWritableDataOnlyDetection, oldTypeHandler->GetPropertyTypes());
        newTypeHandler->SetInstanceTypeHandler(instance);
        Assert(!newTypeHandler->HasSingletonInstance() || !instance->HasSharedType());

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        TraceFixedFieldsAfterTypeHandlerChange(instance, oldTypeHandler, newTypeHandler, oldType, instance->GetDynamicType(), oldSingletonInstance);
#endif

        oldTypeHandler->ChangeAllSlotStorageToVar(instance);
        return newTypeHandler;
    }

    DictionaryTypeHandler* PathTypeHandler::ConvertToDictionaryType(DynamicObject* instance)
    {
        return ConvertToTypeHandler<DictionaryTypeHandler>(instance);
    }

    ES5ArrayTypeHandler* PathTypeHandler::ConvertToES5ArrayType(DynamicObject* instance)
    {
        return ConvertToTypeHandler<ES5ArrayTypeHandler>(instance);
    }

    template <typename T>
    T* PathTypeHandler::ConvertToSimpleDictionaryType(DynamicObject* instance, int propertyCapacity, bool mayBecomeShared)
    {
        Assert(instance);
        ScriptContext* scriptContext = instance->GetScriptContext();
        Recycler* recycler = scriptContext->GetRecycler();

        // Ideally 'this' and oldTypeHandler->GetTypeHandler() should be same
        // But we can have calls from external DOM objects, which requests us to replace the type of the
        // object with a new type. And in such cases, this API gets called with oldTypeHandler and the
        // new type (obtained from the External DOM object)
        // We use the duplicated typeHandler, if we deOptimized the object successfully, else we retain the earlier
        // behavior of using 'this' pointer.

        PathTypeHandler * oldTypeHandler = nullptr;

        VerifySlotCapacities();
        if (instance->DeoptimizeObjectHeaderInlining())
        {
            oldTypeHandler = FromTypeHandler(instance->GetTypeHandler());
            VerifySlotCapacities();
            oldTypeHandler->VerifySlotCapacities();
        }
        else
        {
            oldTypeHandler = this;
        }

        Assert(oldTypeHandler);

        PropertyIndex inlineSlotCapacity, slotCapacity;
        oldTypeHandler->GetOriginalInlineSlotCapacityAndSlotCapacity(&inlineSlotCapacity, &slotCapacity);

        T* newTypeHandler = RecyclerNew(recycler, T, recycler, slotCapacity, propertyCapacity, inlineSlotCapacity, oldTypeHandler->GetOffsetOfInlineSlots());
        // We expect the new type handler to start off marked as having only writable data properties.
        Assert(newTypeHandler->GetHasOnlyWritableDataProperties());

        // Care must be taken to correctly set up fixed field bits whenever a type's handler is changed.  Exactly what needs to
        // be done depends on whether the current handler is shared, whether the new handler is shared, whether the current
        // handler has the prototype flag set, and even whether we take a type transition as part of the process.
        //
        // 1. Can we set fixed bits on new handler for the fields that are marked as fixed on current handler?
        //
        //    Yes, if the new type handler is not shared.  If the handler is not shared, we know that only this instance will
        //    ever use it.  Otherwise, a different instance could transition to the same type handler, but have different values
        //    for fields marked as fixed.
        //
        // 2. Can we set fixed bits on new handler even for the fields that are not marked as fixed on current handler?
        //
        //    Yes, if the new type handler is not shared and we take a type transition during conversion.  The first condition
        //    is required for the same reason as in point 1 above.  The type transition is needed to ensure that any store
        //    field fast paths for this instance get invalidated.  If they didn't, then the newly fixed field could get
        //    overwritten on the fast path without triggering necessary invalidation.
        //
        //    Note that it's desirable to mark additional fields as fixed (particularly when the instance becomes a prototype)
        //    to counteract the effect of false type sharing, which may unnecessarily turn off some fixed field bits.
        //
        // 3. Do we need to clear any fixed field bits on the old or new type handler?
        //
        //    Yes, we must clear fixed fields bits for properties that aren't also used as fixed, but only if both type handlers
        //    are shared and we don't isolate prototypes.  This is rather tricky and results from us pre-creating certain handlers
        //    even before any instances actually have values for all represented properties.  We must avoid the situation, in which
        //    one instance switched to a new type handler with some fixed field not yet used as fixed, and later the second
        //    instance follows the same handler evolution with the same field used as fixed.  Upon switching to the new handler
        //    the second instance would "forget" that the field was used as fixed and fail to invalidate when overwritten.
        //
        //    Example: Instance A with TH1 has a fixed method FOO, which has not been used as fixed yet.  Then instance B gets
        //    pre-created and lands on TH1 (and so far assumes FOO is fixed).  As B's pre-creation continues, it moves to TH2, but
        //    thus far FOO has not been used as fixed.  Now instance A becomes a prototype, and its method FOO is used in a hard-coded
        //    JIT sequence, thus marking it as used as fixed.  Instance A then transitions to TH2 and we lose track of FOO being used
        //    as fixed.  If FOO is then overwritten on A, the hard-coded JIT sequence does not get invalidated and continues to call
        //    the old method FOO.
        //
        // 4. Can we avoid setting used as fixed bits on new handler for fields marked as used as fixed on current handler?
        //
        //    Yes, if the current type handler doesn't have the prototype flag and current handler is not shared or new handler
        //    is not shared or we isolate prototypes, and we take a type transition as part of the conversion.
        //
        //    Type transition ensures that any field loads from the instance are invalidated (including
        //    any that may have hard-coded the fixed field's value).  Hence, if the fixed field on this instance were to be later
        //    overwritten it will not cause any functional issues.  On the other hand, field loads from prototype are not affected
        //    by the prototype object's type change.  Therefore, if this instance is a prototype we must carry the used as fixed
        //    bits forward to ensure that if we overwrite any fixed field we explicitly trigger invalidation.
        //
        //    Review: Actually, the comment below is overly conservative.  If the second instance that became a prototype
        //    followed the same type evolution path, it would have to have invalidated all fixed fields, so there should be no need
        //    to transfer used as fixed bits, unless the current instance is already a prototype.
        //    In addition, if current handler is shared and the new handler is shared, a different instance with the current handler
        //    may later become a prototype (if we don't isolate prototypes) and follow the same conversion to the new handler, even
        //    if the current instance is not a prototype.  Hence, the new type handler must retain the used as fixed bits, so that
        //    proper invalidation can be triggered later, if overwritten.
        //
        //    Note that this may lead to the new type handler with some fields not marked as fixed, but marked as used as fixed.
        //
        //    Note also that if we isolate prototypes, we guarantee that no prototype instance will share a type handler with any
        //    other instance.  Hence, the problem sequence above could not take place.
        //
        // 5. Do we need to invalidate JIT-ed code for any fields marked as used as fixed on current handler?
        //
        //    No.  With the rules above any necessary invalidation will be triggered when the value actually gets overwritten.
        //

        DynamicType* oldType = instance->GetDynamicType();
#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        RecyclerWeakReference<DynamicObject>* oldSingletonInstance = oldTypeHandler->GetSingletonInstance();
        oldTypeHandler->TraceFixedFieldsBeforeTypeHandlerChange(L"converting", L"PathTypeHandler", L"SimpleDictionaryTypeHandler", instance, oldTypeHandler, oldType, oldSingletonInstance);
#endif

        bool const canBeSingletonInstance = DynamicTypeHandler::CanBeSingletonInstance(instance);
        // If this type had been installed on a stack instance it shouldn't have a singleton Instance
        Assert(canBeSingletonInstance || !oldTypeHandler->HasSingletonInstance());

        // Consider: It looks like we're delaying sharing of these type handlers until the second instance arrives, so we could
        // set the singleton here and zap it later.
        TypePath *const oldTypePath = oldTypeHandler->GetTypePath();
        if (!mayBecomeShared && canBeSingletonInstance)
        {
            Assert(oldTypeHandler->HasSingletonInstanceOnlyIfNeeded());
            if (DynamicTypeHandler::AreSingletonInstancesNeeded())
            {
                RecyclerWeakReference<DynamicObject>* curSingletonInstance = oldTypePath->GetSingletonInstance();
                if (curSingletonInstance != nullptr && curSingletonInstance->Get() == instance)
                {
                    newTypeHandler->SetSingletonInstance(curSingletonInstance);
                }
                else
                {
                    newTypeHandler->SetSingletonInstance(instance->CreateWeakReferenceToSelf());
                }
            }
        }

        // It would be nice to transfer fixed fields if the new type handler may become fixed later (but isn't yet).  This would allow
        // singleton instances to retain fixed fields.  It would require that when we do actually share the target type (when the second
        // instance arrives), we clear (and invalidate, if necessary) any fixed fields.  This may be a reasonable trade-off.
        bool transferIsFixed = !mayBecomeShared && canBeSingletonInstance;

        // If we are a prototype or may become a prototype we must transfer used as fixed bits.  See point 4 above.
        Assert(!DynamicTypeHandler::IsolatePrototypes() || ((oldTypeHandler->GetFlags() & IsPrototypeFlag) == 0));
        // For the global object we don't emit a type check before a hard-coded use of a fixed field.  Therefore a type transition isn't sufficient to
        // invalidate any used fixed fields, and we must continue tracking them on the new type handler.  The global object should never have a path
        // type handler.
        Assert(instance->GetTypeId() != TypeIds_GlobalObject);
        // If the type isn't locked, we may not change the type of the instance, and we must also track the used fixed fields on the new handler.
        bool transferUsedAsFixed = !instance->GetDynamicType()->GetIsLocked() || ((oldTypeHandler->GetFlags() & IsPrototypeFlag) != 0 || (oldTypeHandler->GetIsOrMayBecomeShared() && !DynamicTypeHandler::IsolatePrototypes()));

        // Consider: As noted in point 2 above, when converting to non-shared SimpleDictionaryTypeHandler we could be more aggressive
        // and mark every field as fixed, because we will always take a type transition.  We have to remember to respect the switches as
        // to which kinds of properties we should fix, and for that we need the values from the instance.  Even if the type handler
        // says the property is initialized, the current instance may not have a value for it.  Check for value != null.
        Assert(oldTypeHandler->GetSlotCountInternal() <= oldTypePath->GetMaxInitializedSlotCount());
        for (PropertyIndex i = 0; i < oldTypeHandler->GetSlotCountInternal(); i = oldTypePath->GetNextSlotIndex(i))
        {
            if (FixPropsOnPathTypes())
            {
                newTypeHandler->Add(oldTypePath->GetPropertyId(i), PropertyDynamicTypeDefaults,
                    true,
                    transferIsFixed && oldTypePath->GetIsFixedFieldAt(i, oldTypeHandler->GetSlotCountInternal()),
                    transferUsedAsFixed && oldTypePath->GetIsUsedFixedFieldAt(i, oldTypeHandler->GetSlotCountInternal()),
                    scriptContext);
            }
            else
            {
                newTypeHandler->Add(oldTypePath->GetPropertyId(i), PropertyDynamicTypeDefaults, true, false, false, scriptContext);
            }

            // No need to clear fixed fields not used as fixed, because we never convert during pre-creation of type handlers and we always
            // add properties in order they appear on the type path.  Hence, any existing fixed fields will be turned off by any other
            // instance following this type path.  See point 3 above.
        }

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        // Clear the singleton from this handler regardless of mayBecomeShared, because this instance no longer uses this handler.
        if (FixPropsOnPathTypes())
        {
            Assert(oldTypeHandler->HasSingletonInstanceOnlyIfNeeded());
            oldTypePath->ClearSingletonInstanceIfSame(instance);
        }
#endif

        if (mayBecomeShared)
        {
            newTypeHandler->SetFlags(IsLockedFlag | MayBecomeSharedFlag);
        }

        Assert(!DynamicTypeHandler::IsolatePrototypes() || !oldTypeHandler->GetIsOrMayBecomeShared() || ((oldTypeHandler->GetFlags() & IsPrototypeFlag) == 0));
        AssertMsg((newTypeHandler->GetFlags() & IsPrototypeFlag) == 0, "Why did we create a brand new type handler with a prototype flag set?");
        newTypeHandler->SetFlags(IsPrototypeFlag, oldTypeHandler->GetFlags());

        // Any new type handler we expect to see here should have inline slot capacity locked.  If this were to change, we would need
        // to update our shrinking logic (see ShrinkSlotAndInlineSlotCapacity).
        Assert(newTypeHandler->GetIsInlineSlotCapacityLocked());
        newTypeHandler->SetPropertyTypes(PropertyTypesWritableDataOnly | PropertyTypesWritableDataOnlyDetection, oldTypeHandler->GetPropertyTypes());
        newTypeHandler->SetInstanceTypeHandler(instance);
        Assert(!newTypeHandler->HasSingletonInstance() || !instance->HasSharedType());
        // We assumed that we don't need to transfer used as fixed bits unless we are a prototype, which is only valid if we also changed the type.
        Assert(transferUsedAsFixed || (instance->GetType() != oldType && oldType->GetTypeId() != TypeIds_GlobalObject));

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        TraceFixedFieldsAfterTypeHandlerChange(instance, oldTypeHandler, newTypeHandler, oldType, instance->GetDynamicType(), oldSingletonInstance);
#endif

#ifdef PROFILE_TYPES
        scriptContext->convertPathToSimpleDictionaryCount++;
#endif

        oldTypeHandler->ChangeAllSlotStorageToVar(instance);
        return newTypeHandler;
    }

    BOOL PathTypeHandler::SetPropertyWithAttributes(DynamicObject* instance, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        if (attributes == PropertyDynamicTypeDefaults)
        {
            return SetPropertyInternal(instance, propertyId, value, info, flags, possibleSideEffects);
        }
        else
        {
            return ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal() + 1)->SetPropertyWithAttributes(instance, propertyId, value, attributes, info, flags, possibleSideEffects);
        }
    }

    BOOL PathTypeHandler::SetAttributes(DynamicObject* instance, PropertyId propertyId, PropertyAttributes attributes)
    {
        if ( (attributes & PropertyDynamicTypeDefaults) != PropertyDynamicTypeDefaults)
        {
#ifdef PROFILE_TYPES
            instance->GetScriptContext()->convertPathToDictionaryCount3++;
#endif

            return ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal())->SetAttributes(instance, propertyId, attributes);
        }

        return true;
    }

    BOOL PathTypeHandler::GetAttributesWithPropertyIndex(DynamicObject * instance, PropertyId propertyId, BigPropertyIndex index, PropertyAttributes * attributes)
    {
        if (index < this->GetSlotCountInternal())
        {
            Assert(this->GetPropertyId(instance->GetScriptContext(), index) == propertyId);
            *attributes = PropertyDynamicTypeDefaults;
            return true;
        }
        return false;
    }

    bool PathTypeHandler::UsePathTypeHandlerForObjectLiteral(
        const PropertyIdArray *const propIds,
        ScriptContext *const scriptContext,
        bool *const check__proto__Ref)
    {
        Assert(propIds);
        Assert(scriptContext);

        // Always check __proto__ entry, now that object literals always honor __proto__
        const bool check__proto__ = propIds->has__proto__ && scriptContext->GetConfig()->Is__proto__Enabled();
        if(check__proto__Ref)
        {
            *check__proto__Ref = check__proto__;
        }

        return
            !check__proto__ &&
            propIds->count <= TypePath::MaxSlotCapacity &&
            !propIds->hadDuplicates &&
            !propIds->hasAccessors;
    }

    DynamicType* PathTypeHandler::CreateTypeForNewScObject(
        FunctionBody *const functionBody,
        const PropertyIdArray *const propIds,
        const uint objectLiteralIndex)
    {
        Assert(functionBody);
        Assert(propIds);
        Assert(propIds->count != 0);

        ObjectLiteralCreationSiteInfo *const objectLiteralCreationSiteInfo =
            functionBody->GetObjectLiteralCreationSiteInfo(objectLiteralIndex);
        DynamicType *const cachedFinalType = objectLiteralCreationSiteInfo->GetType();
        if(cachedFinalType)
        {
            if(!cachedFinalType->GetIsShared())
                cachedFinalType->ShareType();
            return cachedFinalType;
        }

        ScriptContext *const scriptContext = functionBody->GetScriptContext();
        bool check__proto__;
        if(!UsePathTypeHandlerForObjectLiteral(propIds, scriptContext, &check__proto__))
        {
            DynamicType *const baseType = scriptContext->GetLibrary()->GetObjectLiteralType(static_cast<uint16>(propIds->count));
            DynamicType *newType;
            if(propIds->count <= static_cast<uint>(SimpleDictionaryTypeHandler::MaxPropertyIndexSize))
                newType = SimpleDictionaryTypeHandler::CreateTypeForNewScObject(scriptContext, baseType, propIds, false, check__proto__);
            else if(propIds->count <= static_cast<uint>(BigSimpleDictionaryTypeHandler::MaxPropertyIndexSize))
                newType = BigSimpleDictionaryTypeHandler::CreateTypeForNewScObject(scriptContext, baseType, propIds, false, check__proto__);
            else
                Throw::OutOfMemory();

            objectLiteralCreationSiteInfo->SetType(newType, false);
            return newType;
        }

        // Determine the slot type for each property, and the total number of slots required
        Recycler *const recycler = scriptContext->GetRecycler();
        const PropertyIndex propertyCount = static_cast<PropertyIndex>(propIds->count);
        const PropertyIndex initialFieldCount = objectLiteralCreationSiteInfo->GetInitialFieldCount();
        Assert(initialFieldCount <= propertyCount);
        ObjectSlotType *slotTypes = nullptr;
        PropertyIndex slotCount = propertyCount;
        PropertyIndex evolvedPropertyCount = 0;
        do
        {
            if(initialFieldCount == 0)
                break;

            PathTypeTransitionInfo *const lastUsedRootTransitionInfo =
                objectLiteralCreationSiteInfo->GetLastUsedRootTransitionInfo();
            if(!lastUsedRootTransitionInfo)
                break;

            // Use slot types from the type that was used last time
            slotTypes = ObjectSlotType::NewArrayOfVarSlotTypes(initialFieldCount, recycler);
            PropertyIndex evolvedSlotCount;
            lastUsedRootTransitionInfo->GetPreferredSlotTypes(
                propIds,
                slotTypes,
                initialFieldCount,
                &evolvedSlotCount,
                &evolvedPropertyCount);

            Assert(evolvedPropertyCount <= initialFieldCount);
            if(evolvedPropertyCount == 0)
                break;

            slotCount = evolvedSlotCount;
            if(evolvedPropertyCount < propertyCount)
            {
                // For the remaining properties, assume a Var slot type
                Assert(ObjectSlotType::GetVar().GetNextSlotIndexOrCount(0) == 1);
                slotCount += propertyCount - evolvedPropertyCount;
            }
            if(slotCount > TypePath::MaxSlotCapacity)
                slotCount = TypePath::MaxSlotCapacity;
        } while(false);

        Assert(slotCount >= propertyCount);
    #ifdef PROFILE_OBJECT_LITERALS
        scriptContext->objectLiteralCount[slotCount - 1]++;
    #endif

        // Get the base type based on the number of slots required
        DynamicType *const rootType =
            FunctionBody::DoObjectHeaderInliningForObjectLiteral(slotCount)
                ? scriptContext->GetLibrary()->GetObjectHeaderInlinedLiteralType(slotCount)
                : scriptContext->GetLibrary()->GetObjectLiteralType(slotCount);
        PathTypeHandler *const rootTypeHandler = PathTypeHandler::FromTypeHandler(rootType->GetTypeHandler());
        Assert(rootTypeHandler->GetPropertyCountInternal() == 0);
        Assert(rootTypeHandler->GetSlotCountInternal() == 0);
        if(initialFieldCount == 0)
        {
            objectLiteralCreationSiteInfo->SetType(rootType, false);
            return rootType;
        }
        if(evolvedPropertyCount == 0)
        {
            objectLiteralCreationSiteInfo->SetLastUsedType(rootType);
            return rootType;
        }

        // Transition the base type for each property based on the property ID and slot type
        PathTypeTransitionInfo *transitionInfo = rootTypeHandler->GetTransitionInfo();
        slotCount = 0;
        PropertyIndex initialFieldIndex = 0;
        PropertyIndex firstNativeSlotInitialFieldIndex = Constants::NoSlot;
        do
        {
            const ObjectSlotType slotType = slotTypes[initialFieldIndex];
            const PropertyIndex nextSlotCount = slotType.GetNextSlotIndexOrCount(slotCount);
            if(nextSlotCount > TypePath::MaxSlotCapacity)
                break;

            const PropertyId propertyId = propIds->elements[initialFieldIndex];
        #ifdef PROFILE_OBJECT_LITERALS
            if(!transitionInfo->GetSuccessor(PathTypeSuccessorKey(propertyId, slotType)))
                scriptContext->objectLiteralPathCount++;
        #endif

            transitionInfo =
                transitionInfo->AddProperty(
                    scriptContext->GetPropertyName(propertyId),
                    slotType,
                    slotCount,
                    true,
                    rootTypeHandler->GetIsOrMayBecomeShared() && !IsolatePrototypes(),
                    scriptContext);
            if(!slotType.IsVar() && firstNativeSlotInitialFieldIndex == Constants::NoSlot)
                firstNativeSlotInitialFieldIndex = initialFieldIndex;

            slotCount = nextSlotCount;
        } while(++initialFieldIndex < evolvedPropertyCount);

        DynamicType *const newType = GetOrCreateType(transitionInfo, slotCount, initialFieldIndex, true);
        if(evolvedPropertyCount < initialFieldCount)
        {
            // The last-used type has not fully evolved before reaching the object literal creation site again. The function is
            // probably recursive. Use the new type as the last-used type next time for determining how the new type evolves for
            // the remaining properties.
            objectLiteralCreationSiteInfo->SetLastUsedType(newType);
            return newType;
        }

        objectLiteralCreationSiteInfo->SetType(newType, firstNativeSlotInitialFieldIndex != Constants::NoSlot);
        if(firstNativeSlotInitialFieldIndex == Constants::NoSlot)
            return newType;

        Assert(transitionInfo == FromTypeHandler(newType->GetTypeHandler())->GetTransitionInfo());
        while(true)
        {
            --initialFieldIndex;

            const ObjectSlotType slotType = slotTypes[initialFieldIndex];
            if(!slotType.IsVar())
            {
                // Register the object literal creation site info with the transition info for native slots. If the slot type is
                // changed in the future, it will invalidate the cached final type and cause the type to be reevaluated for the
                // creation site next time around, taking the slot type change into account.
                transitionInfo->RegisterObjectCreationSiteInfo(objectLiteralCreationSiteInfo, recycler);
            }

            if(initialFieldIndex == firstNativeSlotInitialFieldIndex)
                break;
            transitionInfo = transitionInfo->GetPredecessor();
        }
        return newType;
    }

    DynamicType *
    PathTypeHandler::CreateNewScopeObject(ScriptContext *scriptContext, DynamicType *type, const PropertyIdArray *propIds, PropertyAttributes extraAttributes, uint extraAttributesSlotCount)
    {
        uint count = propIds->count;

        Recycler* recycler = scriptContext->GetRecycler();

        SimpleDictionaryTypeHandler* typeHandler = SimpleDictionaryTypeHandler::New(recycler, count, 0, 0, true, true);

        for (uint i = 0; i < count; i++)
        {
            PropertyId propertyId = propIds->elements[i];
            const PropertyRecord* propertyRecord = propertyId == Constants::NoProperty ? NULL : scriptContext->GetPropertyName(propertyId);
            // This will add the property as initialized and non-fixed.  That's fine because we will populate the property values on the
            // scope object right after this (see JavascriptOperators::OP_InitCachedScope).  We will not treat these properties as fixed.
            PropertyAttributes attributes = PropertyWritable | PropertyEnumerable;
            if (i < extraAttributesSlotCount)
            {
                attributes |= extraAttributes;
            }
            typeHandler->Add(propertyRecord, attributes, scriptContext);
        }
        AssertMsg((typeHandler->GetFlags() & IsPrototypeFlag) == 0, "Why does a newly created type handler have the IsPrototypeFlag set?");

 #ifdef PROFILE_OBJECT_LITERALS
        scriptContext->objectLiteralSimpleDictionaryCount++;
 #endif

        type = RecyclerNew(recycler, DynamicType, type, typeHandler, /* isLocked = */ true, /* isShared = */ true);

        return type;
    }

    DynamicType* PathTypeHandler::PromoteType(
        DynamicObject* instance,
        const PropertyRecord* propertyRecord,
        const ObjectSlotType slotType,
        const Var value,
        const PropertyOperationFlags flags,
        ScriptContext* scriptContext)
    {
        Assert(instance != nullptr);

        const PropertyIndex slotCount = GetSlotCountInternal();
        PathTypeTransitionInfo *const nextTransitionInfo =
            GetTransitionInfo()->AddProperty(
                instance,
                propertyRecord,
                slotType,
                slotCount,
                false,
                GetIsOrMayBecomeShared() && !IsolatePrototypes(),
                scriptContext,
                [=]()
                {
                    return
                        !(flags & PropertyOperation_NonFixedValue) &&
                        !IsInternalPropertyId(propertyRecord->GetPropertyId()) &&
                        (
                            JavascriptFunction::Is(value)
                                ?   ShouldFixMethodProperties() || ShouldFixAccessorProperties()
                                :   ShouldFixDataProperties() &&
                                    CheckHeuristicsForFixedDataProps(
                                        instance,
                                        propertyRecord,
                                        propertyRecord->GetPropertyId(),
                                        value)
                        );
                });
        return
            GetOrCreateType(
                nextTransitionInfo,
                slotType.GetNextSlotIndexOrCount(slotCount),
                GetPropertyCountInternal() + 1,
                false,
                instance);
    }

    void PathTypeHandler::InitializeNewType(
        const bool shareType,
        PathTypeHandler *const baseTypeHandler,
        ScriptContext *const scriptContext)
    {
        // The new type is locked, but isn't shared yet.  We will make it shared when the second instance attains it.
        bool markTypeAsShared = !FixPropsOnPathTypes() || shareType;
        if(markTypeAsShared)
        {
            SetFlags(IsSharedFlag);
            GetType()->SetIsShared();
        }

        InitializeTypeCommon(baseTypeHandler);
    }

    void PathTypeHandler::InitializeExistingType(PathTypeHandler *const baseTypeHandler)
    {
        Assert(baseTypeHandler);

        // Now that the second (or subsequent) instance reached this type, make sure that it's shared.
        Assert(GetIsInlineSlotCapacityLocked() == baseTypeHandler->GetIsInlineSlotCapacityLocked());
        DynamicType *const type = GetType();
        Assert(type);
        Assert((FixPropsOnPathTypes() && GetMayBecomeShared()) || (GetIsShared() && type->GetIsShared()));
        if (FixPropsOnPathTypes() && !type->GetIsShared())
        {
            type->ShareType();
        }

        InitializeTypeCommon(baseTypeHandler);
    }

    void PathTypeHandler::InitializeTypeCommon(PathTypeHandler *const baseTypeHandler)
    {
        Assert(
            !baseTypeHandler->IsolatePrototypes() ||
            !baseTypeHandler->GetIsOrMayBecomeShared() ||
            !baseTypeHandler->GetIsPrototype());
        SetFlags(IsPrototypeFlag, baseTypeHandler->GetFlags());
        Assert(baseTypeHandler->GetHasOnlyWritableDataProperties() == GetHasOnlyWritableDataProperties());
        Assert(baseTypeHandler->GetIsInlineSlotCapacityLocked() == GetIsInlineSlotCapacityLocked());
        SetPropertyTypes(PropertyTypesWritableDataOnlyDetection, baseTypeHandler->GetPropertyTypes());
        Assert(baseTypeHandler->DoNativeFields() == DoNativeFields());
        VerifySlotCapacities();
    }

    void
    PathTypeHandler::ResetTypeHandler(DynamicObject * instance)
    {
        // The type path is allocated in the type allocator associated with the script context.
        // So we can't reuse it in other context.  Just convert the type to a simple dictionary type
        this->ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal());
    }

    void PathTypeHandler::SetAllPropertiesToUndefined(DynamicObject* instance, bool invalidateFixedFields) 
    {
        // Note: This method is currently only called from ResetObject, which in turn only applies to external objects.
        // Before using for other purposes, make sure the assumptions made here make sense in the new context.  In particular,
        // the invalidateFixedFields == false is only correct if a) the object is known not to have any, or b) the type of the
        // object has changed and/or property guards have already been invalidated through some other means.
        int slotCount = GetSlotCountInternal();

        TypePath *typePath = GetTypePath();
        if (invalidateFixedFields)
        {
            Js::ScriptContext* scriptContext = instance->GetScriptContext();
            for (PropertyIndex slotIndex = 0; slotIndex < slotCount; slotIndex = typePath->GetNextSlotIndex(slotIndex))
            {
                PropertyId propertyId = typePath->GetPropertyIdUnchecked(slotIndex)->GetPropertyId();
                InvalidateFixedFieldAt(propertyId, slotIndex, scriptContext);
            }
        }

        Js::RecyclableObject* undefined = instance->GetLibrary()->GetUndefined();
        for (PropertyIndex slotIndex = 0; slotIndex < slotCount; slotIndex = typePath->GetNextSlotIndex(slotIndex))
        {
            // Setting the slot may cause the field type to change, which in turn would change the type and handler
            const ObjectSlotType slotTypeBeforeSet = typePath->GetSlotType(slotIndex);
            SetSlotUnchecked(instance, slotIndex, slotTypeBeforeSet, undefined);
            if(slotTypeBeforeSet.IsVar())
            {
                Assert(typePath == FromTypeHandler(instance->GetDynamicType()->GetTypeHandler())->GetTypePath());
            }
            else
            {
                typePath = FromTypeHandler(instance->GetDynamicType()->GetTypeHandler())->GetTypePath();
            }
        }
    }

    void PathTypeHandler::MarshalAllPropertiesToScriptContext(DynamicObject* instance, ScriptContext* targetScriptContext, bool invalidateFixedFields) 
    {
        // Note: This method is currently only called from ResetObject, which in turn only applies to external objects.
        // Before using for other purposes, make sure the assumptions made here make sense in the new context.  In particular,
        // the invalidateFixedFields == false is only correct if a) the object is known not to have any, or b) the type of the
        // object has changed and/or property guards have already been invalidated through some other means.
        int slotCount = GetSlotCountInternal();

        TypePath *const typePath = GetTypePath();
        if (invalidateFixedFields)
        {
            ScriptContext* scriptContext = instance->GetScriptContext();
            for (PropertyIndex slotIndex = 0; slotIndex < slotCount; slotIndex = typePath->GetNextSlotIndex(slotIndex))
            {
                if(!typePath->GetSlotType(slotIndex).IsVar())
                {
                    continue;
                }
                PropertyId propertyId = typePath->GetPropertyIdUnchecked(slotIndex)->GetPropertyId();
                InvalidateFixedFieldAt(propertyId, slotIndex, scriptContext);
            }
        }

        for (PropertyIndex slotIndex = 0; slotIndex < slotCount; slotIndex = typePath->GetNextSlotIndex(slotIndex))
        {
            if(!typePath->GetSlotType(slotIndex).IsVar())
            {
                continue;
            }
            void *const slot = GetSlotAddress(instance, slotIndex);
            SetVarSlotAtAddress(
                slot,
                CrossSite::MarshalVar(targetScriptContext, GetSlotAtAddress(slot, ObjectSlotType::GetVar(), instance, slotIndex)),
                instance);
        }
    }

    BOOL PathTypeHandler::AddProperty(DynamicObject * instance, PropertyId propertyId, Js::Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        if (attributes != PropertyDynamicTypeDefaults)
        {
            Assert(propertyId != Constants::NoProperty);
            PropertyRecord const* propertyRecord = instance->GetScriptContext()->GetPropertyName(propertyId);
            return ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal() + 1)->AddProperty(instance, propertyRecord, value, attributes, info, flags, possibleSideEffects);
        }
        return AddPropertyInternal(instance, propertyId, value, info, flags, possibleSideEffects);
    }

    BOOL PathTypeHandler::AddPropertyInternal(DynamicObject * instance, PropertyId propertyId, Js::Var value, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects)
    {
        VerifySlotCapacities();
        ScriptContext* scriptContext = instance->GetScriptContext();

#if DBG
        uint32 indexVal;
        Assert(GetPropertyIndex(propertyId) == Constants::NoSlot);
        Assert(!scriptContext->IsNumericPropertyId(propertyId, &indexVal));
#endif

        Assert(propertyId != Constants::NoProperty);
        PropertyRecord const* propertyRecord = instance->GetScriptContext()->GetPropertyName(propertyId);

        int32 intValue = 0;
        double floatValue = 0;
        const ObjectSlotType requiredSlotType =
            ObjectSlotType::FromVar(DoNativeFields() && !(flags & PropertyOperation_SpecialValue), 
                                    value, &intValue, &floatValue);
        const ObjectSlotType slotType = GetPreferredSuccessorSlotType(propertyRecord->GetPropertyId(), requiredSlotType);
        Assert(!requiredSlotType.IsValueTypeMoreConvervativeThan(slotType));
        if(requiredSlotType.IsInt() && slotType.IsFloat())
            floatValue = static_cast<double>(intValue);

        if (!TypePath::CanObjectGrowForNewSlot(GetSlotCountInternal(), slotType))
        {
#ifdef PROFILE_TYPES
            scriptContext->convertPathToDictionaryCount1++;
#endif
            return ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal() + 1)->AddProperty(instance, propertyRecord, value, PropertyDynamicTypeDefaults, info, PropertyOperation_None, possibleSideEffects);
        }

        PropertyIndex index = GetSlotCountInternal();
        DynamicType* newType = PromoteType(instance, propertyRecord, slotType, value, flags);

        Assert(instance->GetTypeHandler()->IsPathTypeHandler());
        PathTypeHandler* newTypeHandler = (PathTypeHandler*)newType->GetTypeHandler();
        if (propertyId == PropertyIds::constructor)
        {
            newTypeHandler->isNotPathTypeHandlerOrHasUserDefinedCtor = true;
        }

        Assert(newType->GetIsShared() == newTypeHandler->GetIsShared());

        void *const slot = GetSlotAddress(instance, index);
        if(slotType.IsVar())
        {
            SetVarSlotAtAddress(slot, value, instance);
        }
        else if(slotType.IsInt())
        {
            SetIntSlotAtAddress(slot, intValue);
        }
        else
        {
            Assert(slotType.IsFloat());
            SetFloatSlotAtAddress(slot, floatValue);
        }

        // Don't populate inline cache if this handler isn't yet shared.  If we did, a new instance could
        // reach this handler without us noticing and we could fail to release the old singleton instance, which may later
        // become collectible (not referenced by anything other than this handler), thus we would leak the old singleton instance.
        const bool populateInlineCache =
            newTypeHandler->GetIsShared() ||
            !newTypeHandler->GetTypePath()->GetIsFixedFieldAt(index, newTypeHandler->GetSlotCountInternal());
        Assert(
            populateInlineCache ==
            !newTypeHandler->GetTypePath()->GetIsFixedFieldAt(index, newTypeHandler->GetSlotCountInternal()));
        Assert(populateInlineCache == newTypeHandler->CanStorePropertyValueDirectly(index));

        if (populateInlineCache)
        {
            Assert((instance->GetDynamicType()->GetIsShared()) || (FixPropsOnPathTypes() && instance->GetDynamicType()->GetTypeHandler()->GetIsOrMayBecomeShared()));
            // Can't assert this.  With NewScObject we can jump to the type handler at the tip (where the singleton is),
            // even though we haven't yet initialized the properties all the way to the tip, and we don't want to kill
            // the singleton in that case yet.  It's basically a transient inconsistent state, but we have to live with it.
            // Assert(!instance->GetTypeHandler()->HasSingletonInstance());
            PropertyValueInfo::Set(info, instance, index, newTypeHandler->GetTypePath()->GetSlotType(index));
        }
        else
        {
            PropertyValueInfo::SetNoCache(info, instance);
        }

        Assert(!IsolatePrototypes() || ((this->GetFlags() & IsPrototypeFlag) == 0));
        if (this->GetFlags() & IsPrototypeFlag)
        {
            scriptContext->InvalidateProtoCaches(propertyId);
        }
        SetPropertyUpdateSideEffect(instance, propertyId, value, possibleSideEffects);
        return true;
    }

    DynamicTypeHandler* PathTypeHandler::ConvertToTypeWithItemAttributes(DynamicObject* instance)
    {
        return JavascriptArray::Is(instance) ?
            ConvertToES5ArrayType(instance) : ConvertToDictionaryType(instance);
    }

    void PathTypeHandler::SetPrototype(DynamicObject* instance, RecyclableObject* newPrototype)
    {
        ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal())->SetPrototype(instance, newPrototype);
    }

    void PathTypeHandler::SetIsPrototype(DynamicObject* instance)
    {
        // Don't return if IsPrototypeFlag is set, because we may still need to do a type transition and
        // set fixed bits.  If this handler is shared, this instance may not even be a prototype yet.
        // In this case we may need to convert to a non-shared type handler.
        if (!ChangeTypeOnProto() && !(GetIsOrMayBecomeShared() && IsolatePrototypes()))
        {
            SetFlags(IsPrototypeFlag);
            return;
        }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            DynamicType* oldType = instance->GetDynamicType();
            RecyclerWeakReference<DynamicObject>* oldSingletonInstance = GetSingletonInstance();
#endif

        if ((GetIsOrMayBecomeShared() && IsolatePrototypes()))
        {
            // The type coming in may not be shared or even locked (i.e. might have been created via DynamicObject::ChangeType()).
            // In that case the type handler change below won't change the type on the object, so we have to force it.

            DynamicType* oldType = instance->GetDynamicType();
            ConvertToSimpleDictionaryType(instance, GetPropertyCountInternal());
            
            if (ChangeTypeOnProto() && instance->GetDynamicType() == oldType)
            {
                instance->ChangeType();
            }
        }
        else
        {

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            DynamicType* oldType = instance->GetDynamicType();
            RecyclerWeakReference<DynamicObject>* oldSingletonInstance = GetSingletonInstance();
            TraceFixedFieldsBeforeSetIsProto(instance, this, oldType, oldSingletonInstance);
#endif

            if (ChangeTypeOnProto())
            {
                // If this handler is shared and we don't isolate prototypes, it's possible that the handler has
                // the prototype flag, but this instance may not yet be a prototype and may not have taken
                // the required type transition.  It would be nice to have a reliable flag on the object
                // indicating whether it's a prototype to avoid multiple type transitions if the same object
                // with shared type handler is used as prototype multiple times.
                if (((GetFlags() & IsPrototypeFlag) == 0) || (GetIsShared() && !IsolatePrototypes()))
                {
                    // We're about to split out the type.  If the original type was shared the handler better be shared as well.
                    // Otherwise, the handler would lose track of being shared between different types and instances.
                    Assert(!instance->HasSharedType() || instance->GetDynamicType()->GetTypeHandler()->GetIsShared());

                    instance->ChangeType();
                    Assert(!instance->HasLockedType() && !instance->HasSharedType());
                }
            }
        }

        DynamicTypeHandler* typeHandler = GetCurrentTypeHandler(instance);
        if (typeHandler != this)
        {
            typeHandler->SetIsPrototype(instance);
        }
        else
        {
            SetFlags(IsPrototypeFlag);

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            TraceFixedFieldsAfterSetIsProto(instance, this, typeHandler, oldType, instance->GetDynamicType(), oldSingletonInstance);
#endif

        }
    }

    bool PathTypeHandler::HasSingletonInstance() const
    {
        Assert(HasSingletonInstanceOnlyIfNeeded());
        if (!FixPropsOnPathTypes())
        {
            return false;
        }

        TypePath *const typePath = GetTypePath();
        Assert(GetSlotCountInternal() <= typePath->GetMaxInitializedSlotCount());
        return typePath->HasSingletonInstance() && GetSlotCountInternal() == typePath->GetMaxInitializedSlotCount();
    }

    void PathTypeHandler::DoShareTypeHandler(ScriptContext* scriptContext) 
    {
        Assert((GetFlags() & (IsLockedFlag | MayBecomeSharedFlag | IsSharedFlag)) == (IsLockedFlag | MayBecomeSharedFlag));
        Assert(!IsolatePrototypes() || !GetIsOrMayBecomeShared() || !GetIsPrototype());

#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        // If this handler is becoming shared we need to remove the singleton instance (so that it can be collected
        // if no longer referenced by anything else) and invalidate any fixed fields.
        if (FixPropsOnPathTypes())
        {
            TypePath *const typePath = GetTypePath();
            Assert(GetSlotCountInternal() <= typePath->GetMaxInitializedSlotCount());
            for (PropertyIndex index = 0; index < this->GetSlotCountInternal(); index = typePath->GetNextSlotIndex(index))
            {
                InvalidateFixedFieldAt(typePath->GetPropertyIdUnchecked(index)->GetPropertyId(), index, scriptContext);
            }

            Assert(HasOnlyInitializedNonFixedProperties());
            Assert(HasSingletonInstanceOnlyIfNeeded());
            if (HasSingletonInstance())
            {
                typePath->ClearSingletonInstance();
            }
        }
#endif
    }

    void PathTypeHandler::InvalidateFixedFieldAt(Js::PropertyId propertyId, Js::PropertyIndex index, ScriptContext* scriptContext)
    {
        if (!FixPropsOnPathTypes())
        {
            return;
        }

        // We are adding a new value where some other instance already has an existing value.  If this is a fixed
        // field we must clear the bit. If the value was hard coded in the JIT-ed code, we must invalidate the guards.
        TypePath *const typePath = GetTypePath();
        if (typePath->GetIsUsedFixedFieldAt(index, GetSlotCountInternal()))
        {
            // We may be a second instance chasing the singleton and invalidating fixed fields along the way.
            // Assert(newTypeHandler->typePath->GetSingletonInstance() == instance);

            // Invalidate any JIT-ed code that hard coded this method. No need to invalidate store field
            // inline caches (which might quietly overwrite this fixed fields, because they have never been populated.
#if ENABLE_NATIVE_CODEGEN
            scriptContext->GetThreadContext()->InvalidatePropertyGuards(propertyId);
#endif
        }

        // If we're overwriting an existing value of this property, we don't consider the new one fixed.
        // This also means that it's ok to populate the inline caches for this property from now on.
        typePath->ClearIsFixedFieldAt(index, GetSlotCountInternal());
    }

    void PathTypeHandler::ProcessFixedFieldChange(DynamicObject* instance, PropertyId propertyId, PropertyIndex slotIndex, Var value, bool isNonFixed)
    {
        Assert(instance->GetTypeHandler() == this);
        Assert(!GetIsShared());
        // We don't want fixed properties on external objects, either external properties or expando properties.
        // See DynamicObject::ResetObject for more information.
        Assert(!instance->IsExternal() || isNonFixed);

        Assert(FixPropsOnPathTypes());

        TypePath *const typePath = GetTypePath();
        Assert(slotIndex < typePath->GetMaxInitializedSlotCount());

        // Review (jedmiad): Do we want to check the value assigned and not invalidate if it's the same as before?
        InvalidateFixedFieldAt(propertyId, slotIndex, instance->GetScriptContext());

        // We have now reached the most advanced instance along this path.  If this instance is not the singleton instance, 
        // then the former singleton instance (if any) is no longer a singleton.  This instance could be the singleton 
        // instance, if we just happen to set (overwrite) its last property.
        if (typePath->GetNextSlotIndex(slotIndex) == typePath->GetMaxInitializedSlotCount())
        {
            // This is perhaps the most fragile point of fixed fields on path types.  If we cleared the singleton instance
            // while some fields remained fixed, the instance would be collectible, and yet some code would expect to see
            // values and call methods on it.  Clearly, a recipe for disaster.  We rely on the fact that we always add 
            // properties to (pre-initialized) type handlers in the order they appear on the type path.  By the time 
            // we reach the singleton instance, all fixed fields will have been invalidated.  Otherwise, some fields 
            // could remain fixed (or even uninitialized) and we would have to spin off a loop here to invalidate any 
            // remaining fixed fields - a rather unfortunate overhead.
            auto singletonWeakRef = typePath->GetSingletonInstance();
            if (singletonWeakRef != nullptr && instance != singletonWeakRef->Get())
            {
                Assert(HasSingletonInstanceOnlyIfNeeded());
                typePath->ClearSingletonInstance();
            }
        }
    }

    bool PathTypeHandler::TryUseFixedProperty(PropertyRecord const * propertyRecord, Var * pProperty, FixedPropertyKind propertyType, ScriptContext * requestContext)
    {
        bool result = TryGetFixedProperty<false, true>(propertyRecord, pProperty, propertyType, requestContext);
        TraceUseFixedProperty(propertyRecord, pProperty, result, L"PathTypeHandler", requestContext);
        return result;
    }

    bool PathTypeHandler::TryUseFixedAccessor(PropertyRecord const * propertyRecord, Var * pAccessor, FixedPropertyKind propertyType, bool getter, ScriptContext * requestContext) 
    {
        if (PHASE_VERBOSE_TRACE1(Js::FixedMethodsPhase) || PHASE_VERBOSE_TESTTRACE1(Js::FixedMethodsPhase) ||
            PHASE_VERBOSE_TRACE1(Js::UseFixedDataPropsPhase) || PHASE_VERBOSE_TESTTRACE1(Js::UseFixedDataPropsPhase))
        {
            Output::Print(L"FixedFields: attempt to use fixed accessor %s from PathTypeHandler returned false.\n", propertyRecord->GetBuffer());
            if (this->HasSingletonInstance() && this->GetSingletonInstance()->Get()->GetScriptContext() != requestContext)
            {
                Output::Print(L"FixedFields: Cross Site Script Context is used for property %s. \n", propertyRecord->GetBuffer());
            }
            Output::Flush();
        }
        return false;
    }

    bool PathTypeHandler::CanStorePropertyValueDirectly(const PropertyIndex slotIndex) const
    {
        Assert(slotIndex < GetSlotCountInternal());

        if(!FixPropsOnPathTypes())
        {
            return true;
        }

        Assert(GetSlotCountInternal() <= GetTypePath()->GetMaxInitializedSlotCount());
        return !GetTypePath()->GetIsFixedFieldAt(slotIndex, GetSlotCountInternal());
    }

#if DBG
    bool PathTypeHandler::CanStorePropertyValueDirectly(const DynamicObject* instance, PropertyId propertyId, bool allowLetConst)
    {
        Assert(!allowLetConst);
        // We pass Constants::NoProperty for ActivationObjects for functions with same named formals, but we don't
        // use PathTypeHandlers for those.
        Assert(propertyId != Constants::NoProperty);
        Js::PropertyIndex index = GetPropertyIndex(propertyId);
        if (index != Constants::NoSlot)
        {
            return CanStorePropertyValueDirectly(index);
        }
        else
        {
            AssertMsg(false, "Asking about a property this type handler doesn't know about?");
            return false;
        }
    }

    bool PathTypeHandler::HasOnlyInitializedNonFixedProperties()
    {
        return HasOnlyInitializedNonFixedProperties(GetTypePath(), GetSlotCountInternal());
    }

    bool PathTypeHandler::HasOnlyInitializedNonFixedProperties(TypePath *const typePath, const PropertyIndex objectSlotCount)
    {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
        Assert(objectSlotCount <= typePath->GetMaxInitializedSlotCount());

        for (PropertyIndex index = 0; index < objectSlotCount; index = typePath->GetNextSlotIndex(index))
        {
            if (typePath->GetIsFixedFieldAt(index, objectSlotCount))
            {
                return false;
            }
        }
#endif

        return true;
    }

    bool PathTypeHandler::CheckFixedProperty(PropertyRecord const * propertyRecord, Var * pProperty, ScriptContext * requestContext)
    {
        return TryGetFixedProperty<true, false>(propertyRecord, pProperty, (Js::FixedPropertyKind)(Js::FixedPropertyKind::FixedMethodProperty | Js::FixedPropertyKind::FixedDataProperty), requestContext);
    }

    bool PathTypeHandler::HasAnyFixedProperties() const 
    {
        const PropertyIndex slotCount = GetSlotCountInternal();
        TypePath *const typePath = GetTypePath();
        for (PropertyIndex i = 0; i < slotCount; i = typePath->GetNextSlotIndex(i))
        {
            if (typePath->GetIsFixedFieldAt(i, slotCount))
            {
                return true;
            }
        }
        return false;
    }
#endif

    template <bool allowNonExistent, bool markAsUsed>
    bool PathTypeHandler::TryGetFixedProperty(PropertyRecord const * propertyRecord, Var * pProperty, Js::FixedPropertyKind propertyType, ScriptContext * requestContext)
    {
        if (!FixPropsOnPathTypes())
        {
            return false;
        }

        TypePath *const typePath = GetTypePath();
        PropertyIndex index = typePath->Lookup(propertyRecord->GetPropertyId(), GetSlotCountInternal());
        if (index == Constants::NoSlot)
        {
            AssertMsg(allowNonExistent, "Trying to get a fixed function instance for a non-existent property?");
            return false;
        }

        Var value = typePath->GetSingletonFixedFieldAt(index, GetSlotCountInternal(), requestContext);
        if (value && ((IsFixedMethodProperty(propertyType) && JavascriptFunction::Is(value)) || IsFixedDataProperty(propertyType))) 
        {
            *pProperty = value;
            if (markAsUsed)
            {
                typePath->SetIsUsedFixedFieldAt(index, GetSlotCountInternal());
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    PathTypeTransitionInfo *PathTypeHandler::GetRootTransitionInfo() const
    {
        PathTypeTransitionInfo *transitionInfo = GetTransitionInfo();
        while(transitionInfo->GetPredecessor())
        {
            transitionInfo = transitionInfo->GetPredecessor();
        }

        // The root transition info must always have a type, as the root type serves as a base for creating successor types, for
        // example, to copy the type ID
        Assert(transitionInfo->GetType());

        return transitionInfo;
    }

    PathTypeHandler* PathTypeHandler::GetRootPathTypeHandler() const
    {
        return PathTypeHandler::FromTypeHandler(GetRootTransitionInfo()->GetType()->GetTypeHandler());
    }

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    void PathTypeHandler::DumpFixedFields() const
    {
        TypePath *const typePath = GetTypePath();
        if (FixPropsOnPathTypes())
        {
            Assert(GetSlotCountInternal() <= typePath->GetMaxInitializedSlotCount());
            for (PropertyIndex i = 0; i < GetSlotCountInternal(); i = typePath->GetNextSlotIndex(i))
            {
                Output::Print(L" %s %d%d%d,", typePath->GetPropertyId(i)->GetBuffer(),
                    1,
                    typePath->GetIsFixedFieldAt(i, GetSlotCountInternal()) ? 1 : 0,
                    typePath->GetIsUsedFixedFieldAt(i, GetSlotCountInternal()) ? 1 : 0);
            }
        }
        else
        {
            for (PropertyIndex i = 0; i < GetSlotCountInternal(); i = typePath->GetNextSlotIndex(i))
            {
                Output::Print(L" %s %d%d%d,", typePath->GetPropertyId(i)->GetBuffer(), 1, 0, 0);
            }
        }
    }

    void PathTypeHandler::TraceFixedFieldsBeforeTypeHandlerChange(
        const wchar_t* conversionName, const wchar_t* oldTypeHandlerName, const wchar_t* newTypeHandlerName, 
        DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, 
        DynamicType* oldType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore)
    {
        if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
        {
            Output::Print(L"FixedFields: %s 0x%p from %s to %s:\n", conversionName, instance, oldTypeHandlerName, newTypeHandlerName);
            Output::Print(L"   before: type = 0x%p, type handler = 0x%p, old singleton = 0x%p(0x%p)\n",
                oldType, oldTypeHandler, oldSingletonInstanceBefore, oldSingletonInstanceBefore != nullptr ? oldSingletonInstanceBefore->Get() : nullptr);
            Output::Print(L"   fixed fields:");
            oldTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
        }
        if (PHASE_VERBOSE_TESTTRACE1(FixMethodPropsPhase))
        {
            Output::Print(L"FixedFields: %s instance from %s to %s:\n", conversionName, oldTypeHandlerName, newTypeHandlerName);
            Output::Print(L"   old singleton before %s null \n", oldSingletonInstanceBefore == nullptr ? L"==" : L"!=");
            Output::Print(L"   fixed fields before:");
            oldTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
        }
    }

    void PathTypeHandler::TraceFixedFieldsAfterTypeHandlerChange(
        DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, DynamicTypeHandler* newTypeHandler, 
        DynamicType* oldType, DynamicType* newType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore)
    {
        if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
        {
            RecyclerWeakReference<DynamicObject>* oldSingletonInstanceAfter = oldTypeHandler->GetSingletonInstance();
            RecyclerWeakReference<DynamicObject>* newSingletonInstanceAfter = newTypeHandler->GetSingletonInstance();
            Output::Print(L"   after: type = 0x%p, type handler = 0x%p, old singleton = 0x%p(0x%p), new singleton = 0x%p(0x%p)\n",
                newType, newTypeHandler, oldSingletonInstanceAfter, oldSingletonInstanceAfter != nullptr ? oldSingletonInstanceAfter->Get() : nullptr,
                newSingletonInstanceAfter, newSingletonInstanceAfter != nullptr ? newSingletonInstanceAfter->Get() : nullptr);
            Output::Print(L"   fixed fields:");
            newTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
            Output::Flush();
        }
        if (PHASE_VERBOSE_TESTTRACE1(FixMethodPropsPhase))
        {
            Output::Print(L"   type %s, typeHandler %s, old singleton after %s null (%s), new singleton after %s null\n",
                oldTypeHandler != newTypeHandler ? L"changed" : L"unchanged",
                oldType != newType ? L"changed" : L"unchanged",
                oldTypeHandler->GetSingletonInstance() == nullptr ? L"==" : L"!=",
                oldSingletonInstanceBefore != oldTypeHandler->GetSingletonInstance() ? L"changed" : L"unchanged",
                newTypeHandler->GetSingletonInstance() == nullptr ? L"==" : L"!=");
            Output::Print(L"   fixed fields after:");
            newTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
            Output::Flush();
        }
    }

    void PathTypeHandler::TraceFixedFieldsBeforeSetIsProto(
        DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, DynamicType* oldType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore)
    {
        if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
        {
            Output::Print(L"FixedFields: PathTypeHandler::SetIsPrototype(0x%p):\n", instance);
            Output::Print(L"   before: type = 0x%p, type handler = 0x%p, old singleton = 0x%p(0x%p)\n",
                oldType, oldTypeHandler, oldSingletonInstanceBefore, oldSingletonInstanceBefore != nullptr ? oldSingletonInstanceBefore->Get() : nullptr);
            Output::Print(L"   fixed fields:");
            oldTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
        }
        if (PHASE_VERBOSE_TESTTRACE1(FixMethodPropsPhase))
        {
            Output::Print(L"FixedFields: PathTypeHandler::SetIsPrototype():\n");
            Output::Print(L"   old singleton before %s null \n", oldSingletonInstanceBefore == nullptr ? L"==" : L"!=");
            Output::Print(L"   fixed fields before:");
            oldTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
        }
    }

    void PathTypeHandler::TraceFixedFieldsAfterSetIsProto(
        DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, DynamicTypeHandler* newTypeHandler, 
        DynamicType* oldType, DynamicType* newType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore)
    {
        if (PHASE_VERBOSE_TRACE1(FixMethodPropsPhase))
        {
            RecyclerWeakReference<DynamicObject>* oldSingletonInstanceAfter = oldTypeHandler->GetSingletonInstance();
            RecyclerWeakReference<DynamicObject>* newSingletonInstanceAfter = newTypeHandler->GetSingletonInstance();
            Output::Print(L"   after: type = 0x%p, type handler = 0x%p, old singleton = 0x%p(0x%p), new singleton = 0x%p(0x%p)\n",
                instance->GetType(), newTypeHandler,
                oldSingletonInstanceAfter, oldSingletonInstanceAfter != nullptr ? oldSingletonInstanceAfter->Get() : nullptr,
                newSingletonInstanceAfter, newSingletonInstanceAfter != nullptr ? newSingletonInstanceAfter->Get() : nullptr);
            Output::Print(L"   fixed fields:");
            newTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
            Output::Flush();
        }
        if (PHASE_VERBOSE_TESTTRACE1(FixMethodPropsPhase))
        {
            Output::Print(L"   type %s, old singleton after %s null (%s)\n",
                oldType != newType ? L"changed" : L"unchanged",
                oldSingletonInstanceBefore == nullptr ? L"==" : L"!=",
                oldSingletonInstanceBefore != oldTypeHandler->GetSingletonInstance() ? L"changed" : L"unchanged");
            Output::Print(L"   fixed fields after:");
            newTypeHandler->DumpFixedFields();
            Output::Print(L"\n");
            Output::Flush();
        }
    }
#endif

    template<class F>
    bool PathTypeHandler::ForEachSuccessorTypeHandler(PathTypeTransitionInfo *const transitionInfo, const F Visit)
    {
        Assert(transitionInfo);

        for(auto it = transitionInfo->GetSuccessorIterator(); it.IsValid(); it.MoveNext())
        {
            PathTypeTransitionInfo *const successor = it.CurrentSuccessor();
            DynamicType *const type = successor->GetType();
            if(type)
            {
                const bool isSuccessorPathTypeHandler = successor->IsForPathTypeHandler();
                const int result = Visit(type->GetTypeHandler(), isSuccessorPathTypeHandler);
                if(result < 0)
                    return false; // abort
                Assert(isSuccessorPathTypeHandler || !successor->GetSuccessorIterator().IsValid());
                if(!isSuccessorPathTypeHandler || result == 0)
                    continue; // skip successors
            }

            // Review (jedmiad): We're doing a recursive walk bounded only by the length of the type path.  Should we probe the stack?
            ForEachSuccessorTypeHandler(successor, Visit);
        }
        return true;
    }

    void PathTypeHandler::ShrinkSlotAndInlineSlotCapacity()
    {
        if (!GetIsInlineSlotCapacityLocked())
        {
            PathTypeHandler * rootTypeHandler = GetRootPathTypeHandler();

            bool shrunk = false;
            uint16 maxSlotCount = 0;
            if (rootTypeHandler->GetMaxSlotCount(&maxSlotCount))
            {
                uint16 newInlineSlotCapacity =
                    IsObjectHeaderInlinedTypeHandler()
                        ? RoundUpObjectHeaderInlinedInlineSlotCapacity(maxSlotCount)
                        : RoundUpInlineSlotCapacity(maxSlotCount);
                if (newInlineSlotCapacity < GetInlineSlotCapacity())
                {
                    VerifySlotCapacities();
                    rootTypeHandler->VerifySlotCapacities();

                    rootTypeHandler->ShrinkSlotAndInlineSlotCapacity(newInlineSlotCapacity);
                    shrunk = true;

                    VerifySlotCapacities();
                    rootTypeHandler->VerifySlotCapacities();
                }
            }

            if (!shrunk)
            {
                rootTypeHandler->LockInlineSlotCapacity();
            }
        }

        DebugOnly(VerifyInlineSlotCapacityIsLocked());
    }

    void PathTypeHandler::ShrinkSlotAndInlineSlotCapacity(uint16 newInlineSlotCapacity)
    {
        const auto Visit = [newInlineSlotCapacity](DynamicTypeHandler *const typeHandler, const bool isPathTypeHandler)
        {
            Assert(typeHandler);
            Assert(isPathTypeHandler);
            Assert(typeHandler->IsPathTypeHandler());

            PathTypeHandler *const pathTypeHandler = FromTypeHandler(typeHandler);
            Assert(!pathTypeHandler->GetIsInlineSlotCapacityLocked());
            pathTypeHandler->SetInlineSlotCapacity(newInlineSlotCapacity);
            //Slot capacity should also be shrunk when the inlineSlotCapacity is shrunk.
            pathTypeHandler->SetSlotCapacity(newInlineSlotCapacity);
            pathTypeHandler->SetIsInlineSlotCapacityLocked();
            pathTypeHandler->VerifySlotCapacities();
            return 1; // include successors
        };

        Visit(this, true);
        ForEachSuccessorTypeHandler(GetTransitionInfo(), Visit);
    }

    void PathTypeHandler::LockInlineSlotCapacity()
    {
        const auto Visit = [](DynamicTypeHandler *const typeHandler, const bool isPathTypeHandler)
        {
            Assert(typeHandler);
            Assert(isPathTypeHandler == !!typeHandler->IsPathTypeHandler());

            if(!isPathTypeHandler)
            {
                typeHandler->LockInlineSlotCapacity();
                return 1;
            }

            PathTypeHandler *const pathTypeHandler = FromTypeHandler(typeHandler);
            Assert(!pathTypeHandler->GetIsInlineSlotCapacityLocked());
            pathTypeHandler->SetIsInlineSlotCapacityLocked();
            return 1;
        };

        Visit(this, true);
        ForEachSuccessorTypeHandler(GetTransitionInfo(), Visit);
    }

    void PathTypeHandler::EnsureInlineSlotCapacityIsLocked()
    {
        const auto Visit = [](DynamicTypeHandler *const typeHandler, const bool isPathTypeHandler)
        {
            Assert(typeHandler);
            Assert(isPathTypeHandler == !!typeHandler->IsPathTypeHandler());

            if(!isPathTypeHandler)
            {
                typeHandler->EnsureInlineSlotCapacityIsLocked();
                return 1;
            }

            PathTypeHandler *const pathTypeHandler = FromTypeHandler(typeHandler);
            if(pathTypeHandler->GetIsInlineSlotCapacityLocked())
                return 0; // skip successors

            pathTypeHandler->SetIsInlineSlotCapacityLocked();
            return 1;
        };

        PathTypeTransitionInfo *const rootTransitionInfo = GetRootTransitionInfo();
        const int result = Visit(rootTransitionInfo->GetType()->GetTypeHandler(), true);
        Assert(result >= 0);
        if(result != 0)
        {
            ForEachSuccessorTypeHandler(rootTransitionInfo, Visit);
        }
        DebugOnly(VerifyInlineSlotCapacityIsLocked());
    }

    void PathTypeHandler::VerifyInlineSlotCapacityIsLocked()
    {
        const auto Visit = [](DynamicTypeHandler *const typeHandler, const bool isPathTypeHandler)
        {
            Assert(typeHandler);
            Assert(isPathTypeHandler == !!typeHandler->IsPathTypeHandler());

            if(!isPathTypeHandler)
            {
                typeHandler->VerifyInlineSlotCapacityIsLocked();
                return 1;
            }

            PathTypeHandler *const pathTypeHandler = FromTypeHandler(typeHandler);
            Assert(pathTypeHandler->GetIsInlineSlotCapacityLocked());
            return 1;
        };

        PathTypeTransitionInfo *const rootTransitionInfo = GetRootTransitionInfo();
        Visit(rootTransitionInfo->GetType()->GetTypeHandler(), true);
        ForEachSuccessorTypeHandler(rootTransitionInfo, Visit);
        Visit(this, true);
    }

    bool PathTypeHandler::GetMaxSlotCount(uint16 * maxSlotCount)
    {
        const auto Visit = [maxSlotCount](DynamicTypeHandler *const typeHandler, const bool isPathTypeHandler)
        {
            Assert(typeHandler);
            Assert(isPathTypeHandler == !!typeHandler->IsPathTypeHandler());

            if(!isPathTypeHandler)
                return -1; // abort

            PathTypeHandler *const pathTypeHandler = FromTypeHandler(typeHandler);
            if (pathTypeHandler->GetSlotCountInternal() > *maxSlotCount)
                *maxSlotCount = pathTypeHandler->GetSlotCountInternal();
            return 1;
        };

        Visit(this, true);
        return ForEachSuccessorTypeHandler(GetTransitionInfo(), Visit);
    }

    PathTypeHandler *PathTypeHandler::New(ScriptContext *const scriptContext, const bool disableNativeFields, bool isShared)
    {
        return New(scriptContext, 0, 0, 0, disableNativeFields, isShared);
    }

    PathTypeHandler *PathTypeHandler::New(
        ScriptContext *const scriptContext,
        const PropertyIndex slotCapacity,
        uint16 inlineSlotCapacity,
        uint16 offsetOfInlineSlots,
        const bool disableNativeFields,
        bool isShared)
    {
        return
            New(scriptContext,
                scriptContext->GetRootPath(),
                0,
                0,
                slotCapacity,
                inlineSlotCapacity,
                offsetOfInlineSlots,
                disableNativeFields,
                isShared);
    }

    PathTypeHandler *PathTypeHandler::New(
        ScriptContext *const scriptContext,
        TypePath *const typePath,
        const PropertyIndex slotCount,
        const PropertyIndex propertyCount,
        const PropertyIndex slotCapacity,
        uint16 inlineSlotCapacity,
        uint16 offsetOfInlineSlots,
        const bool disableNativeFields,
        bool isShared)
    {
        Assert(typePath);
        return
            New(scriptContext,
                PathTypeSingleSuccessorTransitionInfo::New(typePath, nullptr, nullptr, scriptContext->GetRecycler()),
                slotCount,
                propertyCount,
                slotCapacity,
                inlineSlotCapacity,
                offsetOfInlineSlots,
                disableNativeFields,
                isShared);
    }

    PathTypeHandler *PathTypeHandler::New(
        ScriptContext *const scriptContext,
        PathTypeTransitionInfo *const transitionInfo,
        const PropertyIndex slotCount,
        const PropertyIndex propertyCount,
        const PropertyIndex slotCapacity,
        uint16 inlineSlotCapacity,
        uint16 offsetOfInlineSlots,
        const bool disableNativeFields,
        bool isShared)
    {
        Assert(transitionInfo);

#ifdef PROFILE_TYPES
        scriptContext->pathTypeHandlerCount++;
#endif

        return
            RecyclerNew(
                scriptContext->GetRecycler(),
                PathTypeHandler,
                transitionInfo,
                slotCount,
                propertyCount,
                slotCapacity,
                inlineSlotCapacity,
                offsetOfInlineSlots,
                disableNativeFields || !scriptContext->DoNativeFields(),
                isShared);
    }

    DynamicType *PathTypeHandler::DeoptimizeObjectHeaderInlining(DynamicObject *const object)
    {
        VerifySlotCapacities();
        Assert(IsObjectHeaderInlinedTypeHandler());
        Assert(object);
        Assert(object->GetTypeHandler() == this);

        TypePath *const typePath = GetTypePath();
        if(FixPropsOnPathTypes())
        {
            // This object is no longer going to be associated with this type path. If it's currently the singleton instance on the
            // type path for fixed fields, clear the singleton instance.
            Assert(HasSingletonInstanceOnlyIfNeeded());
            typePath->ClearSingletonInstanceIfSame(object);
        }

        // A type handler with native fields must have a root type handler that does not have any native fields, for changing
        // the field's slot type. Determine the number of properties before the first native field.
        const PropertyIndex slotCount = GetSlotCountInternal();
        PropertyIndex rootSlotCount;
        PropertyIndex rootPropertyCount;
        if(DoNativeFields())
        {
            rootPropertyCount = rootSlotCount = 0;
            while(rootSlotCount < slotCount && typePath->GetSlotType(rootSlotCount).IsVar())
            {
                rootSlotCount = typePath->GetNextSlotIndex(rootSlotCount);
                ++rootPropertyCount;
            }
        }
        else
        {
            rootSlotCount = slotCount;
            rootPropertyCount = GetPropertyCountInternal();
        }

        // Clone the type path minus fixed field info for the subset of properties determined above. This type will likely be
        // assigned to multiple objects (as it may be the type used in a constructor cache), so it will be shared fixed field
        // info would be invalid anyway.
        Recycler *const recycler = object->GetRecycler();
        TypePath *const rootTypePath = TypePath::New(recycler, rootSlotCount);
        for(PropertyIndex slotIndex = 0; slotIndex < rootSlotCount; slotIndex = typePath->GetNextSlotIndex(slotIndex))
            rootTypePath->AddInternal(typePath->GetPropertyIdUnchecked(slotIndex), typePath->GetSlotType(slotIndex));
        rootTypePath->SetMaxInitializedSlotCount(rootSlotCount); // type and handler will be shared

        // Create the root type and handler for the subset of properties determined above
        PathTypeTransitionInfo *const rootTransitionInfo =
            PathTypeSingleSuccessorTransitionInfo::New(rootTypePath, nullptr, nullptr, recycler);
        DynamicType *const rootType =
            rootTransitionInfo->CreateType(object, object->GetDynamicType(), rootSlotCount, rootPropertyCount, true);
        PathTypeHandler *const rootTypeHandler = FromTypeHandler(rootType->GetTypeHandler());
        ScriptContext *const scriptContext = object->GetScriptContext();
        rootTypeHandler->InitializeNewType(true, this, scriptContext);
        Assert(rootTypeHandler->GetRootTransitionInfo() == rootTransitionInfo);
        if(rootSlotCount >= slotCount)
        {
            Assert(rootSlotCount == slotCount);
            return rootType;
        }

        // Add the remaining properties
        const bool couldSeeProto = GetIsOrMayBecomeShared() && !IsolatePrototypes();
        PathTypeTransitionInfo *currentTransitionInfo = rootTransitionInfo;
        PropertyIndex slotIndex = rootSlotCount;
        do
        {
            const ObjectSlotType slotType = typePath->GetSlotType(slotIndex);
            currentTransitionInfo =
                currentTransitionInfo->AddProperty(
                    typePath->GetPropertyIdUnchecked(slotIndex),
                    slotType,
                    slotIndex,
                    false,
                    couldSeeProto,
                    scriptContext);
            slotIndex = slotType.GetNextSlotIndexOrCount(slotIndex);
        } while(slotIndex < slotCount);
        Assert(slotIndex == slotCount);

        DynamicType *const type = GetOrCreateType(currentTransitionInfo, slotCount, GetPropertyCountInternal(), true, object);
        PathTypeHandler::FromTypeHandler(type->GetTypeHandler())->VerifySlotCapacities();
        return type;
    }

    bool PathTypeHandler::DoNativeFields() const
    {
        Assert(disableNativeFields || !GetType() || GetType()->GetScriptContext()->DoNativeFields());
        return !disableNativeFields;
    }

    bool PathTypeHandler::HasWastedInlineSlot() const
    {
        Assert(!hasWastedInlineSlot || ObjectSlotType::RequiresWideSlotSupport() && DoNativeFields());
        return hasWastedInlineSlot;
    }

    void PathTypeHandler::SetHasWastedInlineSlot()
    {
        Assert(ObjectSlotType::RequiresWideSlotSupport());
        Assert(DoNativeFields());

        hasWastedInlineSlot = true;
    }

    TypePath *PathTypeHandler::GetSlotIterationInfo(
        BigPropertyIndex *const slotCountRef,
        BigPropertyIndex *const propertyCountRef) const
    {
        Assert(slotCountRef);
        Assert(propertyCountRef);

        *slotCountRef = GetSlotCountInternal();
        *propertyCountRef = GetPropertyCountInternal();
        return DoNativeFields() ? GetTypePath() : nullptr;
    }

    PathTypeTransitionInfo *PathTypeHandler::GetTransitionInfo() const
    {
        Assert(transitionInfo);
        return transitionInfo;
    }

    void PathTypeHandler::SetTransitionInfo(PathTypeTransitionInfo *const transitionInfo)
    {
        Assert(transitionInfo);
        this->transitionInfo = transitionInfo;
    }

    ObjectSlotType PathTypeHandler::GetPreferredSuccessorSlotType(
        const PropertyId propertyId,
        const ObjectSlotType requiredSlotType) const
    {
        if(!DoNativeFields())
            return ObjectSlotType::GetVar();
        return GetTransitionInfo()->GetPreferredSuccessorSlotType(propertyId, requiredSlotType);
    }

    DynamicType *PathTypeHandler::GetType() const
    {
        return GetTransitionInfo()->GetType();
    }

    void PathTypeHandler::SetType(DynamicType *const type)
    {
        Assert(!GetType());
        Assert(type);

        GetTransitionInfo()->SetType(type);
        VerifySlotCapacities();
    }

    DynamicType *PathTypeHandler::GetOrCreateType(
        PathTypeTransitionInfo *const transitionInfo,
        const PropertyIndex slotCount,
        const PropertyIndex propertyCount,
        const bool shareType,
        DynamicObject *const object)
    {
        Assert(transitionInfo);
        Assert(propertyCount <= slotCount);

        bool baseTypeIsPredecessor;
        DynamicType *const baseType = transitionInfo->GetBaseType(&baseTypeIsPredecessor);
        PathTypeHandler *const baseTypeHandler = PathTypeHandler::FromTypeHandler(baseType->GetTypeHandler());
        ScriptContext *const scriptContext = baseType->GetScriptContext();

        DynamicType *newType = transitionInfo->GetType();
        PathTypeHandler *newTypeHandler;
        if(newType)
        {
            newTypeHandler = PathTypeHandler::FromTypeHandler(newType->GetTypeHandler());
            Assert(newTypeHandler->GetSlotCountInternal() >= slotCount);
            Assert(newTypeHandler->GetPropertyCountInternal() == propertyCount);
            newTypeHandler->InitializeExistingType(baseTypeHandler);
        }
        else
        {
        #ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            RecyclerWeakReference<DynamicObject>* oldSingletonInstance = nullptr;
            if(baseTypeIsPredecessor)
            {
                oldSingletonInstance = baseTypeHandler->GetSingletonInstance();
                bool branching = baseTypeHandler->GetTypePath()->GetSlotCount() > baseTypeHandler->GetSlotCountInternal();
                TraceFixedFieldsBeforeTypeHandlerChange(
                    branching ? L"branching" : L"advancing",
                    L"PathTypeHandler",
                    L"PathTypeHandler",
                    object,
                    baseTypeHandler,
                    baseType,
                    oldSingletonInstance);
            }
        #endif

            newType = transitionInfo->CreateType(object, baseType, slotCount, propertyCount);
            newTypeHandler = PathTypeHandler::FromTypeHandler(newType->GetTypeHandler());
            newTypeHandler->InitializeNewType(shareType, baseTypeHandler, scriptContext);

        #ifdef ENABLE_DEBUG_CONFIG_OPTIONS
            if(baseTypeIsPredecessor)
            {
                TraceFixedFieldsAfterTypeHandlerChange(
                    object,
                    baseTypeHandler,
                    newTypeHandler,
                    baseType,
                    newType,
                    oldSingletonInstance);
            }
        #endif
        }
        return newType;
    }

    void PathTypeHandler::RegisterObjectCreationSiteInfo(ObjectCreationSiteInfo *const objectCreationSiteInfo) const
    {
        Assert(objectCreationSiteInfo);
        Assert(objectCreationSiteInfo->GetType());
        Assert(objectCreationSiteInfo->GetType() == GetType());
        Assert(objectCreationSiteInfo->GetType()->GetTypeHandler()->IsPathTypeHandler());

        PropertyIndex slotIndex = GetSlotCountInternal();
        if(slotIndex == 0)
            return;

        Recycler *const recycler = objectCreationSiteInfo->GetType()->GetRecycler();
        PathTypeTransitionInfo *transitionInfo = GetTransitionInfo();
        TypePath *const typePath = transitionInfo->GetTypePath();
        do
        {
            slotIndex = typePath->GetPreviousSlotIndex(slotIndex);
            const ObjectSlotType slotType = typePath->GetSlotType(slotIndex);
            if(!slotType.IsVar())
                transitionInfo->RegisterObjectCreationSiteInfo(objectCreationSiteInfo, recycler);
            if(slotIndex == 0)
                break;
            transitionInfo = transitionInfo->GetPredecessor();
        } while(transitionInfo);
    }

    void PathTypeHandler::ChangeAllSlotStorageToVar(DynamicObject *const object)
    {
        // This function is called upon changing the type handler to one that does not support native fields. The type handler
        // is expected to have already been changed, we just need to change the storage of each native slot to var.
        DynamicTypeHandler *const newTypeHandler = object->GetTypeHandler();
        Assert(newTypeHandler != this);
        Assert(!newTypeHandler->IsPathTypeHandler() || !FromTypeHandler(newTypeHandler)->DoNativeFields());

        if(!DoNativeFields())
            return;

        // Decide which type handler to use to load each property from its slot.
        // If the object has transitioned from object-header-inlined to non-object-header-inlined, then
        // the new type handler will correctly show that all properties are now in aux slots. Otherwise, the old type
        // handler has the correct inline slot capacity, possibly accounting for wide native slots.
        DynamicTypeHandler *const typeHandlerForSlotAddress = 
            (this->IsObjectHeaderInlinedTypeHandler() && !newTypeHandler->IsObjectHeaderInlinedTypeHandler()) 
            ? newTypeHandler : this;

        // Overwrite the native slot with an equivalent var value
        const bool isUsedAsPrototype = !!(GetFlags() & IsPrototypeFlag);
        TypePath *const typePath = GetTypePath();
        const PropertyIndex slotCount = GetSlotCountInternal();
        PropertyIndex newSlotIndex = 0;
        for(PropertyIndex oldSlotIndex = 0;
            oldSlotIndex < slotCount;
            oldSlotIndex = typePath->GetNextSlotIndex(oldSlotIndex), ++newSlotIndex)
        {
            const ObjectSlotType oldSlotType = typePath->GetSlotType(oldSlotIndex);
            Assert(ObjectSlotType::RequiresWideSlotSupport() || newSlotIndex == oldSlotIndex);
            if((!ObjectSlotType::RequiresWideSlotSupport() || newSlotIndex == oldSlotIndex) && oldSlotType.IsVar())
                continue;

            SetSlotUnchecked(
                object,
                newSlotIndex,
                ObjectSlotType::GetVar(),
                GetSlotAsTypeHandler(object, typeHandlerForSlotAddress, oldSlotIndex, oldSlotType
#if DBG
                                     , this
#endif
                    ));

            if(isUsedAsPrototype)
            {
                object->GetScriptContext()->InvalidateProtoCaches(
                    typePath->GetPropertyIdUnchecked(oldSlotIndex)->GetPropertyId());
            }
        }

        const PropertyIndex newSlotCount = newSlotIndex;
        Assert(newSlotCount == GetPropertyCountInternal());

        if(!ObjectSlotType::RequiresWideSlotSupport())
        {
            Assert(newSlotCount == slotCount);
            return;
        }

        // Fill the remaining free slots that are now empty due to wide slots becoming narrow, with undefineds
        const Var undefined = object->GetLibrary()->GetUndefined();
        for(; newSlotIndex < slotCount; ++newSlotIndex)
            SetSlotUnchecked(object, newSlotIndex, ObjectSlotType::GetVar(), undefined);
    }

    ObjectSlotType PathTypeHandler::ChangeSlotType(
        DynamicObject *const object,
        const PropertyIndex slotIndex,
        ObjectSlotType newSlotType)
    {
        Assert(object);
        Assert(slotIndex < GetSlotCountInternal());
        const ObjectSlotType oldSlotType = GetTypePath()->GetSlotType(slotIndex);
        Assert(newSlotType.IsValueTypeMoreConvervativeThan(oldSlotType));
        Assert(DoNativeFields());
        VerifySlotCapacities();

        TypePath *const typePath = GetTypePath();
        const PropertyRecord *const propertyRecord = typePath->GetPropertyIdUnchecked(slotIndex);
        ScriptContext *const scriptContext = object->GetScriptContext();
        if(!IsolatePrototypes() && GetFlags() & IsPrototypeFlag)
        {
            scriptContext->InvalidateProtoCaches(propertyRecord->GetPropertyId());
        }

        // Find the common predecessor for this property index
        PathTypeTransitionInfo *currentTransitionInfo = GetTransitionInfo();
        const PropertyIndex slotCount = GetSlotCountInternal();
        PropertyIndex currentSlotIndex = slotCount;
        {
            PathTypeTransitionInfo *successorTransitionInfo;
            do
            {
                successorTransitionInfo = currentTransitionInfo;
                currentTransitionInfo = currentTransitionInfo->GetPredecessor();
                Assert(currentTransitionInfo);
                currentSlotIndex = typePath->GetPreviousSlotIndex(currentSlotIndex);
            } while(currentSlotIndex > slotIndex);
            Assert(currentSlotIndex == slotIndex);

            // Object creation site infos are registered with a transition info that follows a native slot type transition.
            // Invalidate their final types so that the type can be reevaluated for the creation site next time around, taking
            // the slot type change into account.
            successorTransitionInfo->ClearObjectCreationSiteInfoTypes();

            if(successorTransitionInfo->GetType())
            {
                // Update add-property caches with the new type so that the new slot type will be used when adding the property
                // in the future, and invalidate guards that guard property adds to invalidate jitted code that relies on the
                // cached info to perform well.
                scriptContext->GetThreadContext()->InvalidateAddPropertyInlineCaches(
                    propertyRecord->GetPropertyId(),
                    successorTransitionInfo->GetType());
            }
        }

        if(!newSlotType.IsVar())
        {
            newSlotType =
                currentTransitionInfo->GetPreferredSuccessorSlotType(
                    propertyRecord->GetPropertyId(),
                    newSlotType.ToNormalizedValueType());
        }
        if(newSlotType.IsVar())
            newSlotType = ObjectSlotType::GetConvertedVar();

        // Add the property with the new slot type, and add the remaining properties with their same slot type
        ObjectSlotType currentSlotType = newSlotType;
        const PropertyRecord *currentPropertyRecord = propertyRecord;
        DynamicType *newType;
        while(true)
        {
            const PropertyIndex currentSlotCount = currentSlotType.GetNextSlotIndexOrCount(currentSlotIndex);
            currentTransitionInfo =
                currentTransitionInfo->AddProperty(
                    object,
                    currentPropertyRecord,
                    currentSlotType,
                    currentSlotIndex,
                    false,
                    GetIsOrMayBecomeShared() && !IsolatePrototypes(),
                    scriptContext,
                    [=]() { return typePath->GetIsFixedFieldAt(currentSlotIndex, currentSlotCount); });

            if(currentSlotCount >= slotCount)
            {
                Assert(currentSlotCount == slotCount);

                // If the object is the singleton instance of the old type path, clear it, as the object will be on a new path
                typePath->ClearSingletonInstanceIfSame(object);

                newType = GetOrCreateType(currentTransitionInfo, currentSlotCount, GetPropertyCountInternal(), false, object);
                break;
            }

            currentSlotIndex = currentSlotCount;
            currentSlotType = typePath->GetSlotType(currentSlotIndex);
            currentPropertyRecord = typePath->GetPropertyIdUnchecked(currentSlotIndex);
        }

        if(object->GetDynamicType()->GetIsShared())
            newType->ShareType(); // it's probably going to get shared anyway, allow it to be cached immediately

        scriptContext->SetSlotTypeForAnyTypeChanged(true);
        ReplaceInstanceType(object, newType);

        void *const slot = GetSlotAddress(object, slotIndex);
        if(newSlotType.IsFloat())
            SetFloatSlotAtAddress(slot, static_cast<double>(GetIntSlotAtAddress(slot)));
        else
            SetVarSlotAtAddress(slot, GetNativeSlotAtAddress(slot, oldSlotType, object), object);

        VerifySlotCapacities();
        return newSlotType;
    }

    void PathTypeHandler::ChangeSlotTypes(
        DynamicObject *const object,
        PropertyIdToSlotTypeMap *const propertyIdToNewSlotTypeMap)
    {
        Assert(object);
        Assert(propertyIdToNewSlotTypeMap);
        Assert(propertyIdToNewSlotTypeMap->Count() != 0);
        Assert(DoNativeFields());
        VerifySlotCapacities();

        TypePath *const typePath = GetTypePath();
        const PropertyIndex slotCount = GetSlotCountInternal();
    #if DBG
        for(auto it = propertyIdToNewSlotTypeMap->GetIterator(); it.IsValid(); it.MoveNext())
        {
            const PropertyId propertyId = it.CurrentKey();
            Assert(propertyId != Constants::NoProperty);
            const PropertyIndex slotIndex = typePath->Lookup(propertyId, slotCount);
            Assert(slotIndex != Constants::NoSlot);
            const ObjectSlotType oldSlotType = typePath->GetSlotType(slotIndex);
            const ObjectSlotType newSlotType = it.CurrentValue();
            Assert(newSlotType.IsValueTypeMoreConvervativeThan(oldSlotType));
        }
    #endif

        ScriptContext *const scriptContext = object->GetScriptContext();
        if(!IsolatePrototypes() && GetFlags() & IsPrototypeFlag)
        {
            for(auto it = propertyIdToNewSlotTypeMap->GetIterator(); it.IsValid(); it.MoveNext())
                scriptContext->InvalidateProtoCaches(it.CurrentKey());
        }

        // Find the common predecessor for all property IDs
        PathTypeTransitionInfo *currentTransitionInfo = GetTransitionInfo();
        PropertyIndex currentSlotIndex = slotCount;
        {
            int foundCount = 0;
            while(true)
            {
                PathTypeTransitionInfo *const successorTransitionInfo = currentTransitionInfo;
                currentTransitionInfo = currentTransitionInfo->GetPredecessor();
                Assert(currentTransitionInfo);
                currentSlotIndex = typePath->GetPreviousSlotIndex(currentSlotIndex);
                const PropertyId currentPropertyId = typePath->GetPropertyIdUnchecked(currentSlotIndex)->GetPropertyId();
                ObjectSlotType *newSlotTypeRef;
                if(!propertyIdToNewSlotTypeMap->TryGetReference(currentPropertyId, &newSlotTypeRef))
                    continue;
                ObjectSlotType &newSlotType = *newSlotTypeRef;

                // Object creation site infos are registered with a transition info that follows a native slot type transition.
                // Invalidate their final types so that the type can be reevaluated for the creation site next time around,
                // taking the slot type change into account.
                successorTransitionInfo->ClearObjectCreationSiteInfoTypes();

                if(successorTransitionInfo->GetType())
                {
                    // Update add-property caches with the new type so that the new slot type will be used when adding the
                    // property in the future, and invalidate guards that guard property adds to invalidate jitted code that
                    // relies on the cached info to perform well.
                    scriptContext->GetThreadContext()->InvalidateAddPropertyInlineCaches(
                        currentPropertyId,
                        successorTransitionInfo->GetType());
                }

                if(!newSlotType.IsVar())
                {
                    newSlotType =
                        currentTransitionInfo->GetPreferredSuccessorSlotType(
                            currentPropertyId,
                            newSlotType.ToNormalizedValueType());
                }
                if(newSlotType.IsVar())
                    newSlotType = ObjectSlotType::GetConvertedVar();

                if(++foundCount == propertyIdToNewSlotTypeMap->Count())
                    break;
            }
        }

        // Add the property with the new slot type, and add the remaining properties with their same slot type
        DynamicType *newType;
        while(true)
        {
            const PropertyRecord *const currentPropertyRecord = typePath->GetPropertyIdUnchecked(currentSlotIndex);
            ObjectSlotType currentSlotType = ObjectSlotType::GetVar();
            if(!propertyIdToNewSlotTypeMap->TryGetValue(currentPropertyRecord->GetPropertyId(), &currentSlotType))
                currentSlotType = typePath->GetSlotType(currentSlotIndex);
            const PropertyIndex currentSlotCount = currentSlotType.GetNextSlotIndexOrCount(currentSlotIndex);
            currentTransitionInfo =
                currentTransitionInfo->AddProperty(
                    object,
                    currentPropertyRecord,
                    currentSlotType,
                    currentSlotIndex,
                    false,
                    GetIsOrMayBecomeShared() && !IsolatePrototypes(),
                    scriptContext,
                    [=]() { return typePath->GetIsFixedFieldAt(currentSlotIndex, currentSlotCount); });

            if(currentSlotCount >= slotCount)
            {
                Assert(currentSlotCount == slotCount);

                // If the object is the singleton instance of the old type path, clear it, as the object will be on a new path
                typePath->ClearSingletonInstanceIfSame(object);

                newType = GetOrCreateType(currentTransitionInfo, currentSlotCount, GetPropertyCountInternal(), false, object);
                break;
            }

            currentSlotIndex = currentSlotCount;
        }

        if(object->GetDynamicType()->GetIsShared())
            newType->ShareType(); // it's probably going to get shared anyway, allow it to be cached immediately

        scriptContext->SetSlotTypeForAnyTypeChanged(true);
        ReplaceInstanceType(object, newType);

        for(auto it = propertyIdToNewSlotTypeMap->GetIterator(); it.IsValid(); it.MoveNext())
        {
            const PropertyId propertyId = it.CurrentKey();
            const PropertyIndex slotIndex = typePath->Lookup(propertyId, slotCount);
            void *const slot = GetSlotAddress(object, slotIndex);
            const ObjectSlotType newSlotType = it.CurrentValue();
            if(newSlotType.IsFloat())
                SetFloatSlotAtAddress(slot, static_cast<double>(GetIntSlotAtAddress(slot)));
            else
                SetVarSlotAtAddress(slot, GetNativeSlotAtAddress(slot, typePath->GetSlotType(slotIndex), object), object);
        }

        VerifySlotCapacities();
    }

    void PathTypeHandler::VerifySlotCapacities() const
    {
    #if DBG
        const PropertyIndex inlineSlotCapacity = GetInlineSlotCapacity();
        const PropertyIndex slotCapacity = static_cast<PropertyIndex>(GetSlotCapacity());
        PropertyIndex originalInlineSlotCapacity, originalSlotCapacity;
        GetOriginalInlineSlotCapacityAndSlotCapacity(
            &originalInlineSlotCapacity,
            &originalSlotCapacity);

        Assert(inlineSlotCapacity <= originalInlineSlotCapacity);
        Assert(slotCapacity <= originalSlotCapacity);

        if(inlineSlotCapacity == originalInlineSlotCapacity)
        {
            Assert(slotCapacity == originalSlotCapacity);
            Assert(
                GetSlotCountInternal() < originalInlineSlotCapacity ||
                !GetTypePath()->IsLastInlineSlotWasted(originalInlineSlotCapacity));
            return;
        }

        Assert(ObjectSlotType::RequiresWideSlotSupport());
        Assert(originalInlineSlotCapacity - inlineSlotCapacity == originalSlotCapacity - slotCapacity);
        Assert(GetSlotCountInternal() > originalInlineSlotCapacity);

        const PropertyIndex firstAuxSlotIndex = inlineSlotCapacity;
        const ObjectSlotType firstAuxSlotType = GetTypePath()->GetSlotType(firstAuxSlotIndex);
        Assert(firstAuxSlotType.GetNextSlotIndexOrCount(firstAuxSlotIndex) > originalInlineSlotCapacity);
    #endif
    }
}
