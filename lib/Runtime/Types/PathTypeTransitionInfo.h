// Copyright (C) Microsoft. All rights reserved.

#pragma once

namespace Js
{
    #pragma region PathTypeSuccessorKey
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class PathTypeSuccessorKey
    {
    private:
        PropertyId propertyId;
        ObjectSlotType slotType;

    public:
        PathTypeSuccessorKey();
        PathTypeSuccessorKey(const PropertyId propertyId, const ObjectSlotType slotType);

    public:
        bool HasInfo() const;
        void Clear();
        PropertyId GetPropertyId() const;
        ObjectSlotType GetSlotType() const;

    public:
        bool operator ==(const PathTypeSuccessorKey &other) const;
        bool operator !=(const PathTypeSuccessorKey &other) const;
        hash_t GetHashCode() const;
    };

    typedef JsUtil::WeaklyReferencedKeyDictionary<ObjectCreationSiteInfo, bool> ObjectCreationSiteInfoWeakRefHashSet;
    typedef
        JsUtil::WeakReferenceDictionary<
            PathTypeSuccessorKey,
            PathTypeTransitionInfo,
            DictionarySizePolicy<PowerOf2Policy, 1>>
        PathTypeSuccessorKeyToTransitionInfoMap;

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region PathTypeTransitionInfo
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class PathTypeTransitionInfo abstract
    {
    public:
        class SuccessorIterator;

    private:
        TypePath *typePath;
        DynamicType *type;
        PathTypeTransitionInfo *predecessor;
        ObjectCreationSiteInfoWeakRefHashSet *objectCreationSiteInfos;

    protected:
        PathTypeTransitionInfo(TypePath *const typePath, DynamicType *const type, PathTypeTransitionInfo *const predecessor);

    public:
        bool IsForPathTypeHandler() const;
        TypePath *GetTypePath() const;
        DynamicType *GetType() const;
        void SetType(DynamicType *const type);
        DynamicType *GetBaseType(bool *const baseTypeIsPredecessorRef) const;
        DynamicType *CreateType(DynamicObject *const object, DynamicType *const baseType, const PropertyIndex slotCount, const PropertyIndex propertyCount, const bool disallowObjectHeaderInlining = false);
        PathTypeTransitionInfo *GetPredecessor() const;
        void SetPredecessor(PathTypeTransitionInfo *const predecessor);

    public:
        virtual PathTypeTransitionInfo *GetSuccessor(const PathTypeSuccessorKey &successorKey) const = 0;
        virtual void SetSuccessor(const PathTypeSuccessorKey &successorKey, PathTypeTransitionInfo *const transitionInfo, Recycler *const recycler) = 0;
        virtual void ReplaceSuccessor(PathTypeTransitionInfo *const oldTransitionInfo, PathTypeTransitionInfo *const newTransitionInfo, Recycler *const recycler) = 0;
        virtual SuccessorIterator GetSuccessorIterator() const = 0;
        virtual ObjectSlotType GetPreferredSuccessorSlotType(const PropertyId propertyId, const ObjectSlotType requiredSlotType = ObjectSlotType::GetInt(), PathTypeTransitionInfo * *const successorRef = nullptr) const = 0;

    public:
        void GetPreferredSlotTypes(const PropertyIdArray *const propIds, ObjectSlotType *const preferredSlotTypes, const PropertyIndex propertyCount, PropertyIndex *const evolvedSlotCountRef, PropertyIndex *const evolvedPropertyCountRef);

    public:
        void RegisterObjectCreationSiteInfo(ObjectCreationSiteInfo *const objectCreationSiteInfo, Recycler *const recycler);
        static void UnregisterObjectCreationSiteInfo(ObjectCreationSiteInfo *const objectCreationSiteInfo);
        void ClearObjectCreationSiteInfoTypes();

    public:
        PathTypeTransitionInfo *AddProperty(const PropertyRecord *const propertyRecord, const ObjectSlotType slotType, const PropertyIndex objectSlotCount, const bool isObjectLiteral, const bool couldSeeProto, ScriptContext *const scriptContext);
        template<class FMarkAsFixed> PathTypeTransitionInfo *AddProperty(DynamicObject *const object, const PropertyRecord *const propertyRecord, const ObjectSlotType slotType, const PropertyIndex objectSlotCount, const bool isObjectLiteral, const bool couldSeeProto, ScriptContext *const scriptContext, const FMarkAsFixed MarkAsFixed);
    protected:
        virtual PathTypeTransitionInfo *AddSuccessor(const PropertyRecord *const propertyRecord, const ObjectSlotType slotType, const PropertyIndex objectSlotCount, const bool isObjectLiteral, const bool couldSeeProto, ScriptContext *const scriptContext);

    private:
        template<class FMarkAsFixed> void InitializePath(DynamicObject *const object, const PropertyIndex slotIndex, const PropertyIndex objectSlotCount, ScriptContext *const scriptContext, const FMarkAsFixed MarkAsFixed);
        template<class FMarkAsFixed> void InitializeNewPath(DynamicObject *const object, const PropertyIndex slotIndex, const PropertyIndex objectSlotCount, const FMarkAsFixed MarkAsFixed);
        void InitializeExistingPath(const PropertyIndex slotIndex, const PropertyIndex objectSlotCount, ScriptContext *const scriptContext);

    public:
        class SuccessorIterator
        {
        private:
            typedef
                PathTypeSuccessorKeyToTransitionInfoMap::EntryIterator<const PathTypeSuccessorKeyToTransitionInfoMap>
                TSuccessorDictionaryIterator;
            TSuccessorDictionaryIterator it;
            PathTypeSuccessorKey successorKey;
            PathTypeTransitionInfo *successor;
            const bool usingIterator;

        public:
            SuccessorIterator();
            SuccessorIterator(const PathTypeSuccessorKey successorKey, PathTypeTransitionInfo *const successor);
            SuccessorIterator(const TSuccessorDictionaryIterator &it);

        public:
            bool IsValid() const;
            void MoveNext();
        private:
            bool SetCurrentInfo(const PathTypeSuccessorKeyToTransitionInfoMap::EntryType &entry);

        public:
            PathTypeSuccessorKey CurrentSuccessorKey();
            PathTypeTransitionInfo *CurrentSuccessor();
        };

        PREVENT_COPY(PathTypeTransitionInfo);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region PathTypeSingleSuccessorTransitionInfo
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class PathTypeSingleSuccessorTransitionInfo sealed : public PathTypeTransitionInfo
    {
    private:
        PathTypeSuccessorKey successorKey;
        RecyclerWeakReference<PathTypeTransitionInfo> *successorWeakRef;

    private:
        PathTypeSingleSuccessorTransitionInfo(TypePath *const typePath, DynamicType *const type, PathTypeTransitionInfo *const predecessor);
    public:
        static PathTypeSingleSuccessorTransitionInfo *New(TypePath *const typePath, DynamicType *const type, PathTypeTransitionInfo *const predecessor, Recycler *const recycler);

    public:
        virtual PathTypeTransitionInfo *GetSuccessor(const PathTypeSuccessorKey &successorKey) const override;
        virtual void SetSuccessor(const PathTypeSuccessorKey &successorKey, PathTypeTransitionInfo *const transitionInfo, Recycler *const recycler) override;
        virtual void ReplaceSuccessor(PathTypeTransitionInfo *const oldTransitionInfo, PathTypeTransitionInfo *const newTransitionInfo, Recycler *const recycler) override;
        virtual SuccessorIterator GetSuccessorIterator() const override;
        virtual ObjectSlotType GetPreferredSuccessorSlotType(const PropertyId propertyId, const ObjectSlotType requiredSlotType = ObjectSlotType::GetInt(), PathTypeTransitionInfo * *const successorRef = nullptr) const override;

    protected:
        virtual PathTypeTransitionInfo *AddSuccessor(const PropertyRecord *const propertyRecord, const ObjectSlotType slotType, const PropertyIndex objectSlotCount, const bool isObjectLiteral, const bool couldSeeProto, ScriptContext *const scriptContext) override;

        PREVENT_COPY(PathTypeSingleSuccessorTransitionInfo);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion

    #pragma region PathTypeMultipleSuccessorTransitionInfo
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class PathTypeMultipleSuccessorTransitionInfo sealed : public PathTypeTransitionInfo
    {
    private:
        PathTypeSuccessorKeyToTransitionInfoMap successors;

    private:
        PathTypeMultipleSuccessorTransitionInfo(TypePath *const typePath, DynamicType *const type, PathTypeTransitionInfo *const predecessor, Recycler *const recycler);
    public:
        static PathTypeMultipleSuccessorTransitionInfo *New(TypePath *const typePath, DynamicType *const type, PathTypeTransitionInfo *const predecessor, Recycler *const recycler);

    public:
        virtual PathTypeTransitionInfo *GetSuccessor(const PathTypeSuccessorKey &successorKey) const override;
        virtual void SetSuccessor(const PathTypeSuccessorKey &successorKey, PathTypeTransitionInfo *const transitionInfo, Recycler *const recycler) override;
        void SetSuccessor(const PathTypeSuccessorKey &successorKey, RecyclerWeakReference<PathTypeTransitionInfo> *const transitionInfoWeakRef);
        virtual void ReplaceSuccessor(PathTypeTransitionInfo *const oldTransitionInfo, PathTypeTransitionInfo *const newTransitionInfo, Recycler *const recycler) override;
        virtual SuccessorIterator GetSuccessorIterator() const override;
        virtual ObjectSlotType GetPreferredSuccessorSlotType(const PropertyId propertyId, const ObjectSlotType requiredSlotType = ObjectSlotType::GetInt(), PathTypeTransitionInfo * *const successorRef = nullptr) const override;

    protected:
        virtual PathTypeTransitionInfo *AddSuccessor(const PropertyRecord *const propertyRecord, const ObjectSlotType slotType, const PropertyIndex objectSlotCount, const bool isObjectLiteral, const bool couldSeeProto, ScriptContext *const scriptContext) override;

        PREVENT_COPY(PathTypeMultipleSuccessorTransitionInfo);
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion
}

#pragma region DefaultComparer<Js::PathTypeSuccessorKey>
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<>
struct DefaultComparer<Js::PathTypeSuccessorKey>
{
    static bool Equals(
        const Js::PathTypeSuccessorKey &successorKey0,
        const Js::PathTypeSuccessorKey &successorKey1)
    {
        return successorKey0 == successorKey1;
    }

    static hash_t GetHashCode(const Js::PathTypeSuccessorKey &successorKey)
    {
        return successorKey.GetHashCode();
    }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma endregion

namespace Js
{
    #pragma region PathTypeTransitionInfo definition
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template<class FMarkAsFixed>
    PathTypeTransitionInfo *PathTypeTransitionInfo::AddProperty(
        DynamicObject *const object,
        const PropertyRecord *const propertyRecord,
        const ObjectSlotType slotType,
        const PropertyIndex objectSlotCount,
        const bool isObjectLiteral,
        const bool couldSeeProto,
        ScriptContext *const scriptContext,
        const FMarkAsFixed MarkAsFixed)
    {
        PathTypeTransitionInfo *const successor =
            AddSuccessor(
                propertyRecord,
                slotType,
                objectSlotCount,
                isObjectLiteral,
                couldSeeProto,
                scriptContext);
        successor->InitializePath(
            object,
            objectSlotCount,
            slotType.GetNextSlotIndexOrCount(objectSlotCount),
            scriptContext,
            MarkAsFixed);
        return successor;
    }

    template<class FMarkAsFixed>
    void PathTypeTransitionInfo::InitializePath(
        DynamicObject *const object,
        const PropertyIndex slotIndex,
        const PropertyIndex objectSlotCount,
        ScriptContext *const scriptContext,
        const FMarkAsFixed MarkAsFixed)
    {
        Assert(slotIndex < objectSlotCount);
        Assert(objectSlotCount == GetTypePath()->GetNextSlotIndex(slotIndex));

        if(!PathTypeHandler::FixPropsOnPathTypes())
            return;

        if(objectSlotCount <= GetTypePath()->GetMaxInitializedSlotCount())
        {
            InitializeExistingPath(slotIndex, objectSlotCount, scriptContext);
            return;
        }
        InitializeNewPath(object, slotIndex, objectSlotCount, MarkAsFixed);
    }

    template<class FMarkAsFixed>
    void PathTypeTransitionInfo::InitializeNewPath(
        DynamicObject *const object,
        const PropertyIndex slotIndex,
        const PropertyIndex objectSlotCount,
        const FMarkAsFixed MarkAsFixed)
    {
        TypePath *const typePath = GetTypePath();
        Assert(slotIndex == typePath->GetMaxInitializedSlotCount());
        Assert(objectSlotCount > typePath->GetMaxInitializedSlotCount());

        // We are a adding a property where no instance property has been set before.  We rely on properties being
        // added in order of indexes to be sure that we don't leave any uninitialized properties interspersed with
        // initialized ones, which could lead to incorrect behavior.  See comment in TypePath::Branch.

        if(!object)
        {
            typePath->AddBlankFieldAt(slotIndex, objectSlotCount);
            return;
        }

        // Consider: It would be nice to assert the slot is actually null.  However, we sometimes pre-initialize to
        // undefined or even some other special illegal value (for let or const, currently == null)
        // Assert(object->GetSlot(index) == null);

        if(PathTypeHandler::ShouldFixAnyProperties() && PathTypeHandler::CanBeSingletonInstance(object))
        {
            typePath->AddSingletonInstanceFieldAt(object, slotIndex, MarkAsFixed(), objectSlotCount);
            return;
        }

        typePath->AddSingletonInstanceFieldAt(slotIndex, objectSlotCount);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    #pragma endregion
}
