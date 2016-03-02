// Copyright (C) Microsoft. All rights reserved.

#include "RuntimeTypePch.h"

namespace Js
{
    #pragma region PathTypeSuccessorKey
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PathTypeSuccessorKey::PathTypeSuccessorKey() : propertyId(Constants::NoProperty), slotType(ObjectSlotType::GetVar())
    {
    }

    PathTypeSuccessorKey::PathTypeSuccessorKey(
        const PropertyId propertyId,
        const ObjectSlotType slotType)
        : propertyId(propertyId), slotType(slotType)
    {
    }

    bool PathTypeSuccessorKey::HasInfo() const
    {
        return propertyId != Constants::NoProperty;
    }

    void PathTypeSuccessorKey::Clear()
    {
        propertyId = Constants::NoProperty;
    }

    PropertyId PathTypeSuccessorKey::GetPropertyId() const
    {
        return propertyId;
    }

    ObjectSlotType PathTypeSuccessorKey::GetSlotType() const
    {
        return slotType;
    }

    bool PathTypeSuccessorKey::operator ==(const PathTypeSuccessorKey &other) const
    {
        return propertyId == other.propertyId && slotType == other.slotType;
    }

    bool PathTypeSuccessorKey::operator !=(const PathTypeSuccessorKey &other) const
    {
        return !(*this == other);
    }

    hash_t PathTypeSuccessorKey::GetHashCode() const
    {
        return static_cast<hash_t>((propertyId << ObjectSlotType::BitSize) | static_cast<ObjectSlotType::TSize>(slotType));
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region PathTypeTransitionInfo
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PathTypeTransitionInfo::PathTypeTransitionInfo(
        TypePath *const typePath,
        DynamicType *const type,
        PathTypeTransitionInfo *const predecessor)
        : typePath(typePath), type(type), predecessor(predecessor), objectCreationSiteInfos(nullptr)
    {
    }

    bool PathTypeTransitionInfo::IsForPathTypeHandler() const
    {
        return !!typePath;
    }

    TypePath *PathTypeTransitionInfo::GetTypePath() const
    {
        Assert(typePath);
        return typePath;
    }

    DynamicType *PathTypeTransitionInfo::GetType() const
    {
        return type;
    }

    void PathTypeTransitionInfo::SetType(DynamicType *const type)
    {
        Assert(type);
        this->type = type;
    }

    DynamicType *PathTypeTransitionInfo::GetBaseType(bool *const baseTypeIsPredecessorRef) const
    {
        Assert(baseTypeIsPredecessorRef);

        const PathTypeTransitionInfo *transitionInfo = this;
        while(true)
        {
            transitionInfo = transitionInfo->GetPredecessor();
            Assert(transitionInfo);
            DynamicType *const type = transitionInfo->GetType();
            if(!type)
                continue;
            *baseTypeIsPredecessorRef = transitionInfo == GetPredecessor();
            return type;
        }
    }

    DynamicType *PathTypeTransitionInfo::CreateType(
        DynamicObject *const object,
        DynamicType *const baseType,
        const PropertyIndex slotCount,
        const PropertyIndex propertyCount,
        const bool disallowObjectHeaderInlining)
    {
        Assert(!type);
        Assert(baseType);
        Assert(slotCount >= propertyCount);

        // isLocked
        //     - PathTypeHandlers and their corresponding types are always locked. This flag is more meaningful for other type
        //       handlers that can mutate upon adding a property, to disallow the mutation when isLocked == true.
        // isShared
        //     - This is the first type / type handler being created for this set of properties, so it is not shared yet.
        //       PathTypeHandler will update the shared state of the type / type handler as it requires.

        PathTypeHandler *const baseTypeHandler = PathTypeHandler::FromTypeHandler(baseType->GetTypeHandler());
        PropertyIndex inlineSlotCapacity, slotCapacity;
        baseTypeHandler->GetOriginalInlineSlotCapacityAndSlotCapacity(&inlineSlotCapacity, &slotCapacity);
        if(slotCapacity < slotCount)
            slotCapacity = slotCount;
        uint16 offsetOfInlineSlots = baseTypeHandler->GetOffsetOfInlineSlots();
        if(baseTypeHandler->IsObjectHeaderInlinedTypeHandler() &&
            (disallowObjectHeaderInlining || slotCapacity > inlineSlotCapacity))
        {
            inlineSlotCapacity -= DynamicTypeHandler::GetObjectHeaderInlinableSlotCapacity();
            offsetOfInlineSlots = sizeof(DynamicObject);
        }
        if(ObjectSlotType::RequiresWideSlotSupport() &&
            slotCount > inlineSlotCapacity &&
            slotCount == slotCapacity &&
            GetTypePath()->IsLastInlineSlotWasted(inlineSlotCapacity))
        {
            // The property requires a wide slot and does not fit in the last inline slot. Increase the slot capacity to
            // compensate.
            ++slotCapacity;
        }

        PathTypeHandler *const typeHandler =
            PathTypeHandler::New(
                baseType->GetScriptContext(),
                this,
                slotCount,
                propertyCount,
                slotCapacity,
                inlineSlotCapacity,
                offsetOfInlineSlots,
                !baseTypeHandler->DoNativeFields(),
                false /* isShared */);
        Assert(typeHandler->GetInlineSlotCapacity() == inlineSlotCapacity);
        Assert(typeHandler->GetSlotCapacity() >= slotCapacity);

        Assert(typeHandler->GetHasOnlyWritableDataProperties());
        typeHandler->CopyPropertyTypes(
            PropertyTypesWritableDataOnly | PropertyTypesWritableDataOnlyDetection,
            baseTypeHandler->GetPropertyTypes());
        typeHandler->SetPropertyTypes(PropertyTypesInlineSlotCapacityLocked, baseTypeHandler->GetPropertyTypes());

        if(ObjectSlotType::RequiresWideSlotSupport() &&
            slotCount > inlineSlotCapacity &&
            GetTypePath()->IsLastInlineSlotWasted(inlineSlotCapacity))
        {
            // The property requires a wide slot and does not fit in the last inline slot. Decrease the inline slot capacity to
            // the used portion and increase the aux slot capacity to compensate.
            typeHandler->SetInlineSlotCapacity(inlineSlotCapacity - 1);
            typeHandler->SetSlotCapacity(typeHandler->GetSlotCapacity() - 1);
            typeHandler->SetHasWastedInlineSlot();
            Assert(typeHandler->GetSlotCapacity() >= slotCount);
        }
        typeHandler->VerifySlotCapacities();

        if(object)
        {
            // DynamicObject::DuplicateType is virtual, to create an appropriate type for the specific object type
            type = object->DuplicateType();
            type->typeHandler = typeHandler;
            type->SetIsLocked();
            Assert(!type->GetIsShared());
        }
        else
            type = DynamicType::New(baseType, typeHandler, true /* isLocked */, false /* isShared */);
        return type;
    }

    PathTypeTransitionInfo *PathTypeTransitionInfo::GetPredecessor() const
    {
        return predecessor;
    }

    void PathTypeTransitionInfo::SetPredecessor(PathTypeTransitionInfo *const predecessor)
    {
        Assert(predecessor);
        Assert(predecessor != this->predecessor);

        this->predecessor = predecessor;
    }

    void PathTypeTransitionInfo::GetPreferredSlotTypes(
        const PropertyIdArray *const propIds,
        ObjectSlotType *const preferredSlotTypes,
        const PropertyIndex propertyCount,
        PropertyIndex *const evolvedSlotCountRef,
        PropertyIndex *const evolvedPropertyCountRef)
    {
        Assert(!GetPredecessor());
        Assert(GetType());
        Assert(propIds);
        Assert(preferredSlotTypes);
        Assert(propertyCount != 0);
        Assert(propertyCount <= propIds->count);
        Assert(evolvedSlotCountRef);
        Assert(evolvedPropertyCountRef);

        const bool doNativeFields = PathTypeHandler::FromTypeHandler(GetType()->GetTypeHandler())->DoNativeFields();
        PathTypeTransitionInfo *transitionInfo = this;
        PropertyIndex slotCount = 0, propertyIndex = 0;
        do
        {
            const ObjectSlotType preferredSlotType =
                doNativeFields
                    ?   transitionInfo->GetPreferredSuccessorSlotType(
                            propIds->elements[propertyIndex],
                            ObjectSlotType::GetInt(),
                            &transitionInfo)
                    :   ObjectSlotType::GetVar();
            if(!transitionInfo)
                break;
            preferredSlotTypes[propertyIndex] = preferredSlotType;
            slotCount = preferredSlotType.GetNextSlotIndexOrCount(slotCount);
        } while(++propertyIndex < propertyCount);

        *evolvedSlotCountRef = slotCount;
        *evolvedPropertyCountRef = propertyIndex;
    }

    void PathTypeTransitionInfo::RegisterObjectCreationSiteInfo(
        ObjectCreationSiteInfo *const objectCreationSiteInfo,
        Recycler *const recycler)
    {
        Assert(objectCreationSiteInfo);
        Assert(objectCreationSiteInfo->GetType());
        Assert(objectCreationSiteInfo->GetType()->GetTypeHandler()->IsPathTypeHandler());
        Assert(objectCreationSiteInfo->CanTypeChange());

        if(!objectCreationSiteInfos)
            objectCreationSiteInfos = RecyclerNew(recycler, ObjectCreationSiteInfoWeakRefHashSet, recycler, 3); // default initial capacity is 17!
        else
        {
            Assert(objectCreationSiteInfos->Lookup(objectCreationSiteInfo, true));
        }
        objectCreationSiteInfos->UncheckedAdd(objectCreationSiteInfo, false);
    }

    void PathTypeTransitionInfo::UnregisterObjectCreationSiteInfo(ObjectCreationSiteInfo *const objectCreationSiteInfo)
    {
        Assert(objectCreationSiteInfo);

        DynamicType *const type = objectCreationSiteInfo->GetType();
        Assert(type);
        if (!type->GetTypeHandler()->IsPathTypeHandler())
        {
            return;
        }
        PathTypeTransitionInfo *transitionInfo =
            PathTypeHandler::FromTypeHandler(type->GetTypeHandler())->GetTransitionInfo();
        do
        {
            ObjectCreationSiteInfoWeakRefHashSet *const objectCreationSiteInfos = transitionInfo->objectCreationSiteInfos;
            if(objectCreationSiteInfos &&
                objectCreationSiteInfos->Remove(objectCreationSiteInfo) &&
                objectCreationSiteInfos->Count() == 0)
            {
                transitionInfo->objectCreationSiteInfos = nullptr;
            }
            transitionInfo = transitionInfo->GetPredecessor();
        } while(transitionInfo);
    }

    void PathTypeTransitionInfo::ClearObjectCreationSiteInfoTypes()
    {
        ObjectCreationSiteInfoWeakRefHashSet *const objectCreationSiteInfos = this->objectCreationSiteInfos;
        if(!objectCreationSiteInfos)
            return;
        this->objectCreationSiteInfos = nullptr;

        objectCreationSiteInfos->Map(
            [](ObjectCreationSiteInfo *const objectCreationSiteInfo,
                bool,
                const RecyclerWeakReference<ObjectCreationSiteInfo> *)
            {
                objectCreationSiteInfo->ClearType();
            });
    }

    PathTypeTransitionInfo *PathTypeTransitionInfo::AddProperty(const PropertyRecord *const propertyRecord, const ObjectSlotType slotType, const PropertyIndex objectSlotCount, const bool isObjectLiteral, const bool couldSeeProto, ScriptContext *const scriptContext)
    {
        return
            AddProperty(
                nullptr,
                propertyRecord,
                slotType,
                objectSlotCount,
                isObjectLiteral,
                couldSeeProto,
                scriptContext,
                []() { return false; });
    }

    PathTypeTransitionInfo *PathTypeTransitionInfo::AddSuccessor(
        const PropertyRecord *const propertyRecord,
        const ObjectSlotType slotType,
        const PropertyIndex objectSlotCount,
        const bool isObjectLiteral,
        const bool couldSeeProto,
        ScriptContext *const scriptContext)
    {
        Assert(propertyRecord);
        Assert(objectSlotCount <= GetTypePath()->GetSlotCount());
        Assert(GetTypePath()->Lookup(propertyRecord->GetPropertyId(), objectSlotCount) == Constants::NoSlot);
        Assert(scriptContext);

        TypePath *const typePath = GetTypePath();
        Assert(typePath->CanObjectGrowForNewSlot(objectSlotCount, slotType));

        Recycler *const recycler = scriptContext->GetRecycler();
        TypePath *newTypePath;
        if(typePath->GetSlotCount() > objectSlotCount)
        {
            // Branch the type path since it already has more slots than the object. Branching is analogous to cloning, with a
            // slot capacity appropriate for the object's slot count and the new slot.
            newTypePath = typePath->Branch(recycler, objectSlotCount, slotType, couldSeeProto);

        #ifdef PROFILE_TYPES
            scriptContext->branchCount++;
        #endif
        #ifdef PROFILE_OBJECT_LITERALS
            if(isObjectLiteral)
                scriptContext->objectLiteralBranchCount++;
        #endif
        }
        else if(!typePath->HasCapacityForNewSlot(slotType))
        {
            // Grow the type path to accommodate the new slot. Growing is analogous to cloning, with increased slot capacity.
            newTypePath = typePath->Grow(recycler);

            // Update all references to the old type path, so that the old one can be collected. This will also ensure that the
            // fixed field info is correct for types that referenced the old type path.
            this->typePath = newTypePath;
            for(PathTypeTransitionInfo *predecessor = GetPredecessor();
                predecessor && predecessor->GetTypePath() == typePath;
                predecessor = predecessor->GetPredecessor())
            {
                predecessor->typePath = newTypePath;
            }
        }
        else
            newTypePath = typePath;

        newTypePath->AddInternal(propertyRecord, slotType);

    #ifdef PROFILE_TYPES
        scriptContext->promoteCount++;
        scriptContext->maxPathLength =
            max(static_cast<int>(newTypePath->GetNextSlotIndex(objectSlotCount)), scriptContext->maxPathLength);
    #endif
    #ifdef PROFILE_OBJECT_LITERALS
        if(isObjectLiteral)
            scriptContext->objectLiteralPromoteCount++;
    #endif

        return PathTypeSingleSuccessorTransitionInfo::New(newTypePath, nullptr, this, recycler);
    }

    void PathTypeTransitionInfo::InitializeExistingPath(
        const PropertyIndex slotIndex,
        const PropertyIndex objectSlotCount,
        ScriptContext *const scriptContext)
    {
        Assert(scriptContext);

        TypePath *const typePath = GetTypePath();
        Assert(slotIndex < typePath->GetMaxInitializedSlotCount());
        Assert(objectSlotCount <= typePath->GetMaxInitializedSlotCount());

        if(typePath->GetIsUsedFixedFieldAt(slotIndex, objectSlotCount))
        {
            // We are adding a new value where some other instance already has an existing value.  If this is a fixed 
            // field we must clear the bit. If the value was hard coded in the JIT-ed code, we must invalidate the guards.

            // Invalidate any JIT-ed code that hard coded this method. No need to invalidate store field
            // inline caches (which might quitely overwrite this fixed fields, because they have never been populated.
            scriptContext->GetThreadContext()->InvalidatePropertyGuards(typePath->GetPropertyIdUnchecked(slotIndex));
        }

        // If we're overwriting an existing value of this property, we don't consider the new one fixed.
        // This also means that it's ok to populate the inline caches for this property from now on.
        typePath->ClearIsFixedFieldAt(slotIndex, objectSlotCount);

        Assert(PathTypeHandler::HasOnlyInitializedNonFixedProperties(typePath, objectSlotCount));
        Assert(PathTypeHandler::HasSingletonInstanceOnlyIfNeeded(typePath));
        if(objectSlotCount == typePath->GetMaxInitializedSlotCount())
        {
            // We have now reached the most advanced instance along this path.  If this instance is not the singleton instance, 
            // then the former singleton instance (if any) is no longer a singleton.  This instance could be the singleton 
            // instance, if we just happen to set (overwrite) its last property.

            // This is perhaps the most fragile point of fixed fields on path types.  If we cleared the singleton instance
            // while some fields remained fixed, the instance would be collectible, and yet some code would expect to see
            // values and call methods on it.  Clearly, a recipe for disaster.  We rely on the fact that we always add 
            // properties to (pre-initialized) type handlers in the order they appear on the type path.  By the time 
            // we reach the singleton instance, all fixed fields will have been invalidated.  Otherwise, some fields 
            // could remain fixed (or even uninitialized) and we would have to spin off a loop here to invalidate any 
            // remaining fixed fields - a rather unfortunate overhead.
            typePath->ClearSingletonInstance();
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region PathTypeTransitionInfo::SuccessorIterator
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PathTypeTransitionInfo::SuccessorIterator::SuccessorIterator() : successor(nullptr), usingIterator(false)
    {
        Assert(!IsValid());
    }

    PathTypeTransitionInfo::SuccessorIterator::SuccessorIterator(
        const PathTypeSuccessorKey successorKey,
        PathTypeTransitionInfo *const successor)
        : successorKey(successorKey), successor(successor), usingIterator(false)
    {
    }

    PathTypeTransitionInfo::SuccessorIterator::SuccessorIterator(const TSuccessorDictionaryIterator &it)
        : it(it), usingIterator(true)
    {
        for(; this->it.IsValid(); this->it.MoveNext())
        {
            if(SetCurrentInfo(this->it.Current()))
                return;
        }
        successor = nullptr;
    }

    bool PathTypeTransitionInfo::SuccessorIterator::IsValid() const
    {
        Assert(!successor || successorKey.HasInfo());
        return !!successor;
    }

    void PathTypeTransitionInfo::SuccessorIterator::MoveNext()
    {
        Assert(IsValid());

        if(!usingIterator)
        {
            successor = nullptr;
            return;
        }

        while(true)
        {
            it.MoveNext();
            if(!it.IsValid())
                break;
            if(SetCurrentInfo(it.Current()))
                return;
        }
        successor = nullptr;
    }

    bool PathTypeTransitionInfo::SuccessorIterator::SetCurrentInfo(
        const PathTypeSuccessorKeyToTransitionInfoMap::EntryType &entry)
    {
        Assert(usingIterator);

        successor = entry.Value()->Get();
        if(!successor)
            return false;
        successorKey = entry.Key();
        return true;
    }

    PathTypeSuccessorKey PathTypeTransitionInfo::SuccessorIterator::CurrentSuccessorKey()
    {
        Assert(IsValid());
        return successorKey;
    }

    PathTypeTransitionInfo *PathTypeTransitionInfo::SuccessorIterator::CurrentSuccessor()
    {
        Assert(IsValid());
        return successor;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region PathTypeSingleSuccessorTransitionInfo
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PathTypeSingleSuccessorTransitionInfo::PathTypeSingleSuccessorTransitionInfo(
        TypePath *const typePath,
        DynamicType *const type,
        PathTypeTransitionInfo *const predecessor)
        : PathTypeTransitionInfo(typePath, type, predecessor), successorWeakRef(nullptr)
    {
    }

    PathTypeSingleSuccessorTransitionInfo *PathTypeSingleSuccessorTransitionInfo::New(
        TypePath *const typePath,
        DynamicType *const type,
        PathTypeTransitionInfo *const predecessor,
        Recycler *const recycler)
    {
        Assert(recycler);
        return RecyclerNew(recycler, PathTypeSingleSuccessorTransitionInfo, typePath, type, predecessor);
    }

    PathTypeTransitionInfo *PathTypeSingleSuccessorTransitionInfo::GetSuccessor(
        const PathTypeSuccessorKey &successorKey) const
    {
        Assert(successorKey.HasInfo());

        if(this->successorKey != successorKey)
            return nullptr;
        Assert(successorWeakRef);
        return successorWeakRef->Get();
    }

    void PathTypeSingleSuccessorTransitionInfo::SetSuccessor(
        const PathTypeSuccessorKey &successorKey,
        PathTypeTransitionInfo *const transitionInfo,
        Recycler *const recycler)
    {
        Assert(!GetSuccessor(successorKey));
        Assert(transitionInfo);
        Assert(recycler);

        const PathTypeSuccessorKey firstSuccessorKey = this->successorKey;
        RecyclerWeakReference<PathTypeTransitionInfo> *firstSuccessorWeakRef = successorWeakRef;
        PathTypeTransitionInfo *firstSuccessor;
        do
        {
            if(firstSuccessorKey.HasInfo())
            {
                firstSuccessor = firstSuccessorWeakRef->Get();
                if(firstSuccessor)
                    break;
            }

            this->successorKey = successorKey;
            successorWeakRef = recycler->CreateWeakReferenceHandle(transitionInfo);
            return;
        } while(false);

        PathTypeMultipleSuccessorTransitionInfo *const newTransitionInfo =
            PathTypeMultipleSuccessorTransitionInfo::New(GetTypePath(), GetType(), GetPredecessor(), recycler);
        if(GetType())
            PathTypeHandler::FromTypeHandler(GetType()->GetTypeHandler())->SetTransitionInfo(newTransitionInfo);
        if(GetPredecessor())
            GetPredecessor()->ReplaceSuccessor(this, newTransitionInfo, recycler);
        Assert(firstSuccessor->GetPredecessor() == this);
        firstSuccessor->SetPredecessor(newTransitionInfo);
        Assert(transitionInfo->GetPredecessor() == this);
        transitionInfo->SetPredecessor(newTransitionInfo);
        newTransitionInfo->SetSuccessor(firstSuccessorKey, firstSuccessorWeakRef);
        newTransitionInfo->SetSuccessor(successorKey, transitionInfo, recycler);
    }

    void PathTypeSingleSuccessorTransitionInfo::ReplaceSuccessor(
        PathTypeTransitionInfo *const oldTransitionInfo,
        PathTypeTransitionInfo *const newTransitionInfo,
        Recycler *const recycler)
    {
        Assert(successorKey.HasInfo());
        Assert(oldTransitionInfo);
        Assert(oldTransitionInfo->GetPredecessor() == this);
        Assert(newTransitionInfo);
        Assert(newTransitionInfo->GetPredecessor() == this);
        Assert(recycler);

        Assert(successorWeakRef->Get() == oldTransitionInfo);
        successorWeakRef = recycler->CreateWeakReferenceHandle(newTransitionInfo);
    }

    PathTypeTransitionInfo::SuccessorIterator PathTypeSingleSuccessorTransitionInfo::GetSuccessorIterator() const
    {
        if(!successorKey.HasInfo())
            return SuccessorIterator();
        return SuccessorIterator(successorKey, successorWeakRef->Get());
    }

    ObjectSlotType PathTypeSingleSuccessorTransitionInfo::GetPreferredSuccessorSlotType(
        const PropertyId propertyId,
        const ObjectSlotType requiredSlotType,
        PathTypeTransitionInfo * *const successorRef) const
    {
        Assert(propertyId != Constants::NoProperty);
        Assert(requiredSlotType == requiredSlotType.ToNormalizedValueType());

        if(successorKey.GetPropertyId() != propertyId)
        {
            if(successorRef)
                *successorRef = nullptr;
            return requiredSlotType;
        }

        const ObjectSlotType preferredSlotType = requiredSlotType.Merge(successorKey.GetSlotType());
        if(successorRef)
            *successorRef = GetSuccessor(PathTypeSuccessorKey(propertyId, preferredSlotType));
        return preferredSlotType;
    }

    PathTypeTransitionInfo *PathTypeSingleSuccessorTransitionInfo::AddSuccessor(
        const PropertyRecord *const propertyRecord,
        const ObjectSlotType slotType,
        const PropertyIndex objectSlotCount,
        const bool isObjectLiteral,
        const bool couldSeeProto,
        ScriptContext *const scriptContext)
    {
        Assert(propertyRecord);
        Assert(objectSlotCount <= GetTypePath()->GetSlotCount());
        Assert(scriptContext);

        const PropertyId propertyId = propertyRecord->GetPropertyId();
        Assert(GetTypePath()->Lookup(propertyId, objectSlotCount) == Constants::NoSlot);

        const PathTypeSuccessorKey successorKey(propertyId, slotType);
        PathTypeTransitionInfo *successor = GetSuccessor(successorKey);
        if(successor)
        {
            Assert(
                !PathTypeHandler::FixPropsOnPathTypes() ||
                objectSlotCount <= successor->GetTypePath()->GetMaxInitializedSlotCount());

        #ifdef PROFILE_TYPES
            scriptContext->cacheCount++;
        #endif
        }
        else
        {
            successor =
                PathTypeTransitionInfo::AddSuccessor(
                    propertyRecord,
                    slotType,
                    objectSlotCount,
                    isObjectLiteral,
                    couldSeeProto,
                    scriptContext);
            SetSuccessor(successorKey, successor, scriptContext->GetRecycler());
        }

        Assert(
            successor->GetTypePath()->Lookup(propertyRecord->GetPropertyId(), successor->GetTypePath()->GetSlotCount()) ==
            objectSlotCount);
        return successor;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region PathTypeMultipleSuccessorTransitionInfo
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    PathTypeMultipleSuccessorTransitionInfo::PathTypeMultipleSuccessorTransitionInfo(
        TypePath *const typePath,
        DynamicType *const type,
        PathTypeTransitionInfo *const predecessor,
        Recycler *const recycler)
        : PathTypeTransitionInfo(typePath, type, predecessor), successors(recycler)
    {
    }

    PathTypeMultipleSuccessorTransitionInfo *PathTypeMultipleSuccessorTransitionInfo::New(
        TypePath *const typePath,
        DynamicType *const type,
        PathTypeTransitionInfo *const predecessor,
        Recycler *const recycler)
    {
        Assert(recycler);
        return
            RecyclerNew(recycler, PathTypeMultipleSuccessorTransitionInfo, typePath, type, predecessor, recycler);
    }

    PathTypeTransitionInfo *PathTypeMultipleSuccessorTransitionInfo::GetSuccessor(
        const PathTypeSuccessorKey &successorKey) const
    {
        Assert(successorKey.HasInfo());

        RecyclerWeakReference<PathTypeTransitionInfo> *const successorWeakRef = successors.Lookup(successorKey, nullptr);
        return successorWeakRef ? successorWeakRef->Get() : nullptr;
    }

    void PathTypeMultipleSuccessorTransitionInfo::SetSuccessor(
        const PathTypeSuccessorKey &successorKey,
        PathTypeTransitionInfo *const transitionInfo,
        Recycler *const recycler)
    {
        Assert(transitionInfo);
        Assert(recycler);

        SetSuccessor(successorKey, recycler->CreateWeakReferenceHandle(transitionInfo));
    }

    void PathTypeMultipleSuccessorTransitionInfo::SetSuccessor(
        const PathTypeSuccessorKey &successorKey,
        RecyclerWeakReference<PathTypeTransitionInfo> *const transitionInfoWeakRef)
    {
        Assert(!GetSuccessor(successorKey));
        Assert(transitionInfoWeakRef);

        successors.Item(successorKey, transitionInfoWeakRef);
    }

    void PathTypeMultipleSuccessorTransitionInfo::ReplaceSuccessor(
        PathTypeTransitionInfo *const oldTransitionInfo,
        PathTypeTransitionInfo *const newTransitionInfo,
        Recycler *const recycler)
    {
        Assert(oldTransitionInfo);
        Assert(oldTransitionInfo->GetPredecessor() == this);
        Assert(newTransitionInfo);
        Assert(newTransitionInfo->GetPredecessor() == this);
        Assert(recycler);

        for(auto it = successors.GetIterator(); it.IsValid(); it.MoveNext())
        {
            RecyclerWeakReference<PathTypeTransitionInfo> *&successor = it.CurrentValueReference();
            if(successor->Get() == oldTransitionInfo)
            {
                successor = recycler->CreateWeakReferenceHandle(newTransitionInfo);
                return;
            }
        }
        Assert(false);
    }

    PathTypeTransitionInfo::SuccessorIterator PathTypeMultipleSuccessorTransitionInfo::GetSuccessorIterator() const
    {
        return SuccessorIterator(successors.GetIterator());
    }

    ObjectSlotType PathTypeMultipleSuccessorTransitionInfo::GetPreferredSuccessorSlotType(
        const PropertyId propertyId,
        const ObjectSlotType requiredSlotType,
        PathTypeTransitionInfo * *const successorRef) const
    {
        Assert(propertyId != Constants::NoProperty);
        Assert(requiredSlotType == requiredSlotType.ToNormalizedValueType());

        // Look for an existing var transition
        ObjectSlotType preferredSlotType = ObjectSlotType::GetVar();
        if(requiredSlotType.IsVar() && !successorRef)
            return preferredSlotType;
        preferredSlotType = ObjectSlotType::GetConvertedVar();
        PathTypeTransitionInfo *successor = GetSuccessor(PathTypeSuccessorKey(propertyId, preferredSlotType));
        if(ObjectSlotType::RequiresWideSlotSupport() && !successor)
        {
            preferredSlotType = ObjectSlotType::GetVar();
            successor = GetSuccessor(PathTypeSuccessorKey(propertyId, preferredSlotType));
        }
        if(successorRef)
            *successorRef = successor;
        if(requiredSlotType.IsVar() || successor)
            return preferredSlotType;

        // Look for an existing float transition
        preferredSlotType = ObjectSlotType::GetFloat();
        if(requiredSlotType.IsFloat() && !successorRef)
            return preferredSlotType;
        successor = GetSuccessor(PathTypeSuccessorKey(propertyId, preferredSlotType));
        if(successorRef)
            *successorRef = successor;
        if(requiredSlotType.IsFloat() || successor)
            return preferredSlotType;

        // Look for an existing int transition
        Assert(requiredSlotType.IsInt());
        preferredSlotType = ObjectSlotType::GetInt();
        if(!successorRef)
            return preferredSlotType;
        *successorRef = GetSuccessor(PathTypeSuccessorKey(propertyId, preferredSlotType));
        return preferredSlotType;
    }

    PathTypeTransitionInfo *PathTypeMultipleSuccessorTransitionInfo::AddSuccessor(
        const PropertyRecord *const propertyRecord,
        const ObjectSlotType slotType,
        const PropertyIndex objectSlotCount,
        const bool isObjectLiteral,
        const bool couldSeeProto,
        ScriptContext *const scriptContext)
    {
        Assert(propertyRecord);
        Assert(propertyRecord);
        Assert(objectSlotCount <= GetTypePath()->GetSlotCount());
        Assert(scriptContext);

        const PropertyId propertyId = propertyRecord->GetPropertyId();
        Assert(GetTypePath()->Lookup(propertyId, objectSlotCount) == Constants::NoSlot);

        const PathTypeSuccessorKey successorKey(propertyId, slotType);
        PathTypeTransitionInfo *successor = GetSuccessor(successorKey);
        if(successor)
        {
            Assert(
                !PathTypeHandler::FixPropsOnPathTypes() ||
                objectSlotCount <= successor->GetTypePath()->GetMaxInitializedSlotCount());

        #ifdef PROFILE_TYPES
            scriptContext->cacheCount++;
        #endif
        }
        else
        {
            successor =
                PathTypeTransitionInfo::AddSuccessor(
                    propertyRecord,
                    slotType,
                    objectSlotCount,
                    isObjectLiteral,
                    couldSeeProto,
                    scriptContext);
            SetSuccessor(successorKey, successor, scriptContext->GetRecycler());
        }

        Assert(
            successor->GetTypePath()->Lookup(propertyRecord->GetPropertyId(), successor->GetTypePath()->GetSlotCount()) ==
            objectSlotCount);
        return successor;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion
}
