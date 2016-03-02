//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    typedef JsUtil::BaseDictionary<PropertyId, ObjectSlotType, Recycler> PropertyIdToSlotTypeMap;

    class PathTypeHandler sealed : public DynamicTypeHandler
    {
        friend class DynamicObject;
        friend class PathTypeTransitionInfo;
        friend class PathTypeSingleSuccessorTransitionInfo;

    private:
        PathTypePropertyIndex slotCount;
        PathTypePropertyIndex propertyCount;
        bool disableNativeFields;
        bool hasWastedInlineSlot;
        PathTypeTransitionInfo *transitionInfo;

    public:
        DEFINE_GETCPPNAME();

    private:
        PathTypeHandler(PathTypeTransitionInfo *const transitionInfo, const PropertyIndex slotCount, const PropertyIndex propertyCount, const PropertyIndex slotCapacity, uint16 inlineSlotCapacity, uint16 offsetOfInlineSlots, const bool disableNativeFields, bool isShared);

        DEFINE_VTABLE_CTOR_NO_REGISTER(PathTypeHandler, DynamicTypeHandler, transitionInfo(nullptr));

    public:
        virtual BOOL IsLockable() const override { return true; }
        virtual BOOL IsSharable() const override { return true; }
        virtual void DoShareTypeHandler(ScriptContext* scriptContext) override;

        static bool UsePathTypeHandlerForObjectLiteral(const PropertyIdArray *const propIds, ScriptContext *const scriptContext, bool *const check__proto__Ref = nullptr);
        static DynamicType* CreateTypeForNewScObject(FunctionBody *const functionBody, const PropertyIdArray *const propIds, const uint objectLiteralIndex);
        static DynamicType* CreateNewScopeObject(ScriptContext* scriptContext, DynamicType* type, const Js::PropertyIdArray *propIds, PropertyAttributes extraAttributes = PropertyNone, uint extraAttributesSlotCount = UINT_MAX);

        static const PathTypeHandler *FromTypeHandler(const DynamicTypeHandler *const typeHandler) { Assert(typeHandler->IsPathTypeHandler()); return static_cast<const PathTypeHandler *>(typeHandler); }
        static PathTypeHandler *FromTypeHandler(DynamicTypeHandler *const typeHandler) { return const_cast<PathTypeHandler *>(FromTypeHandler(const_cast<const DynamicTypeHandler *>(typeHandler))); }

        virtual BigPropertyIndex GetSlotCount() override;
        virtual int GetPropertyCount() override;
        virtual BigPropertyIndex GetSlotCountAndPropertyCount(int *const propertyCountRef) override;
        virtual PropertyId GetPropertyId(ScriptContext* scriptContext, PropertyIndex index) override;
        virtual PropertyId GetPropertyId(ScriptContext* scriptContext, BigPropertyIndex index) override;
        virtual PropertyIndex GetPropertyIndex(const PropertyRecord* propertyRecord) override;
        virtual ObjectSlotType GetSlotType(const BigPropertyIndex bigSlotIndex) const override;
        virtual bool GetPropertyEquivalenceInfo(PropertyRecord const* propertyRecord, PropertyEquivalenceInfo& info) override;

    public:
#if ENABLE_NATIVE_CODEGEN
        virtual IR::BailOutKind IsObjTypeSpecEquivalent(DynamicObject *const object, const TypeEquivalenceRecord& record, uint& failedPropertyIndex) override;
#endif
    private:
        bool IsObjTypeSpecEquivalent(DynamicObject *const object, const EquivalentPropertyEntry* entry, bool *const mayBeEquivalentWithSlotTypeChangeRef = nullptr, PropertyIdToSlotTypeMap *const propertyIdToNewSlotTypeMap = nullptr);

    public:
        virtual BOOL FindNextProperty(ScriptContext* scriptContext, PropertyIndex& index, JavascriptString** propertyString,
            PropertyId* propertyId, PropertyAttributes* attributes, Type* type, DynamicType *typeToEnumerate, bool requireEnumerable, bool enumSymbols = false) override;
        virtual BOOL HasProperty(DynamicObject* instance, PropertyId propertyId, __out_opt bool *noRedecl = nullptr) override;
        virtual BOOL HasProperty(DynamicObject* instance, JavascriptString* propertyNameString) override;
        virtual BOOL GetProperty(DynamicObject* instance, Var originalInstance, PropertyId propertyId, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL GetProperty(DynamicObject* instance, Var originalInstance, JavascriptString* propertyNameString, Var* value, PropertyValueInfo* info, ScriptContext* requestContext) override;
        virtual BOOL SetProperty(DynamicObject* instance, PropertyId propertyId, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL SetProperty(DynamicObject* instance, JavascriptString* propertyNameString, Var value, PropertyOperationFlags flags, PropertyValueInfo* info) override;
        virtual BOOL DeleteProperty(DynamicObject* instance, PropertyId propertyId, PropertyOperationFlags flags) override;
        virtual BOOL IsFixedProperty(const DynamicObject* instance, PropertyId propertyId) override;
        virtual BOOL IsEnumerable(DynamicObject* instance, PropertyId propertyId) override;
        virtual BOOL IsWritable(DynamicObject* instance, PropertyId propertyId) override;
        virtual BOOL IsConfigurable(DynamicObject* instance, PropertyId propertyId) override;
        virtual BOOL SetEnumerable(DynamicObject* instance, PropertyId propertyId, BOOL value) override;
        virtual BOOL SetWritable(DynamicObject* instance, PropertyId propertyId, BOOL value) override;
        virtual BOOL SetConfigurable(DynamicObject* instance, PropertyId propertyId, BOOL value) override;
        virtual BOOL SetAccessors(DynamicObject* instance, PropertyId propertyId, Var getter, Var setter, PropertyOperationFlags flags = PropertyOperation_None) override;
        virtual BOOL PreventExtensions(DynamicObject *instance) override;
        virtual BOOL Seal(DynamicObject* instance) override;
        virtual BOOL SetPropertyWithAttributes(DynamicObject* instance, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags = PropertyOperation_None, SideEffects possibleSideEffects = SideEffects_Any) override;
        virtual BOOL SetAttributes(DynamicObject* instance, PropertyId propertyId, PropertyAttributes attributes) override;
        virtual BOOL GetAttributesWithPropertyIndex(DynamicObject * instance, PropertyId propertyId, BigPropertyIndex index, PropertyAttributes * attributes) override;

        virtual void ResetTypeHandler(DynamicObject * instance) override;
        virtual void SetAllPropertiesToUndefined(DynamicObject* instance, bool invalidateFixedFields) override;
        virtual void MarshalAllPropertiesToScriptContext(DynamicObject* instance, ScriptContext* targetScriptContext, bool invalidateFixedFields) override;
        virtual DynamicTypeHandler* ConvertToTypeWithItemAttributes(DynamicObject* instance) override;
        virtual BOOL AllPropertiesAreEnumerable() override { return true; }
        virtual BOOL IsPathTypeHandler() const override { return TRUE; }

    private:
        template<class F> static bool ForEachSuccessorTypeHandler(PathTypeTransitionInfo *const transitionInfo, const F Visit);
    public:
        virtual void ShrinkSlotAndInlineSlotCapacity() override;
    private:
        void ShrinkSlotAndInlineSlotCapacity(uint16 newInlineSlotCapacity);
    public:
        virtual void LockInlineSlotCapacity() override;
        virtual void EnsureInlineSlotCapacityIsLocked() override;
        virtual void VerifyInlineSlotCapacityIsLocked() override;
        bool GetMaxSlotCount(uint16 * maxClotCount);

    public:
        virtual void SetPrototype(DynamicObject* instance, RecyclableObject* newPrototype) override;

        virtual void SetIsPrototype(DynamicObject* instance) override;

    public:
#if DBG
        virtual bool SupportsPrototypeInstances() const override { return !IsolatePrototypes(); }
#endif

        virtual bool HasSingletonInstance() const override;
        virtual bool TryUseFixedProperty(PropertyRecord const * propertyRecord, Var * pProperty, FixedPropertyKind propertyType, ScriptContext * requestContext) override;
        virtual bool TryUseFixedAccessor(PropertyRecord const * propertyRecord, Var * pAccessor, FixedPropertyKind propertyType, bool getter, ScriptContext * requestContext) override;

    public:
        bool CanStorePropertyValueDirectly(const PropertyIndex slotIndex) const;

#if DBG
    public:
        virtual bool CanStorePropertyValueDirectly(const DynamicObject* instance, PropertyId propertyId, bool allowLetConst) override;
        bool HasOnlyInitializedNonFixedProperties();
        static bool HasOnlyInitializedNonFixedProperties(TypePath *const typePath, const PropertyIndex objectSlotCount);
        virtual bool CheckFixedProperty(PropertyRecord const * propertyRecord, Var * pProperty, ScriptContext * requestContext) override;
        virtual bool HasAnyFixedProperties() const override;
#endif

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
        virtual void DumpFixedFields() const override;
        static void TraceFixedFieldsBeforeTypeHandlerChange(
            const wchar_t* conversionName, const wchar_t* oldTypeHandlerName, const wchar_t* newTypeHandlerName,
            DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, DynamicType* oldType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore);
        static void TraceFixedFieldsAfterTypeHandlerChange(
            DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, DynamicTypeHandler* newTypeHandler,
            DynamicType* oldType, DynamicType* newType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore);
        static void TraceFixedFieldsBeforeSetIsProto(
            DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, DynamicType* oldType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore);
        static void TraceFixedFieldsAfterSetIsProto(
            DynamicObject* instance, DynamicTypeHandler* oldTypeHandler, DynamicTypeHandler* newTypeHandler,
            DynamicType* oldType, DynamicType* newType, RecyclerWeakReference<DynamicObject>* oldSingletonInstanceBefore);
#endif

    public:
        static bool FixPropsOnPathTypes()
        {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            return CONFIG_FLAG(FixPropsOnPathTypes);
#else
            return false;
#endif
        }

    private:
        template <bool allowNonExistent, bool markAsUsed>
        bool TryGetFixedProperty(PropertyRecord const * propertyRecord, Var * pProperty, Js::FixedPropertyKind propertyType, ScriptContext * requestContext);

    public:
        virtual RecyclerWeakReference<DynamicObject>* GetSingletonInstance() const override { return HasSingletonInstance() ? this->GetTypePath()->GetSingletonInstance() : nullptr; }

        virtual void SetSingletonInstanceUnchecked(RecyclerWeakReference<DynamicObject>* instance) override
        {
            Assert(!GetIsShared());
            this->GetTypePath()->SetSingletonInstance(instance, GetSlotCountInternal());
        }

        virtual void ClearSingletonInstance() override { Assert(false); }

#if DBG
        bool HasSingletonInstanceOnlyIfNeeded() const
        {
            return HasSingletonInstanceOnlyIfNeeded(GetTypePath());
        }

        static bool HasSingletonInstanceOnlyIfNeeded(TypePath *const typePath)
        {
#ifdef SUPPORT_FIXED_FIELDS_ON_PATH_TYPES
            return AreSingletonInstancesNeeded() || !typePath->HasSingletonInstance();
#else
            return true;
#endif
        }
#endif

    private:
        void CopySingletonInstance(DynamicObject* instance, DynamicTypeHandler* typeHandler);
        void InvalidateFixedFieldAt(Js::PropertyId propertyId, Js::PropertyIndex index, ScriptContext* scriptContext);
        void ProcessFixedFieldChange(DynamicObject* instance, PropertyId propertyId, PropertyIndex slotIndex, Var value, bool isNonFixed);

    public:
        void GetOriginalInlineSlotCapacityAndSlotCapacity(PropertyIndex *const inlineSlotCapacityRef, PropertyIndex *const slotCapacityRef) const;

    private:
        template <typename T>
        T* ConvertToTypeHandler(DynamicObject* instance);

        DynamicType* PromoteType(DynamicObject* instance, const PropertyRecord* propertyId, const ObjectSlotType slotType, const Var value, const PropertyOperationFlags flags);

        DictionaryTypeHandler* ConvertToDictionaryType(DynamicObject* instance);
        ES5ArrayTypeHandler* ConvertToES5ArrayType(DynamicObject* instance);

        template <typename T> T*
        ConvertToSimpleDictionaryType(DynamicObject* instance, int propertyCapacity, bool mayBecomeShared = false);

        SimpleDictionaryTypeHandler* ConvertToSimpleDictionaryType(DynamicObject* instance, int propertyCapacity, bool mayBecomeShared = false)
        {
            return ConvertToSimpleDictionaryType<SimpleDictionaryTypeHandler>(instance, propertyCapacity, mayBecomeShared);
        }

        BOOL AddPropertyInternal(DynamicObject * instance, PropertyId propertyId, Js::Var value, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects, PropertyAttributes attributes = PropertyDynamicTypeDefaults);
        BOOL AddProperty(DynamicObject* instance, PropertyId propertyId, Var value, PropertyAttributes attributes, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects);
        BOOL SetPropertyInternal(DynamicObject* instance, PropertyId propertyId, Var value, PropertyValueInfo* info, PropertyOperationFlags flags, SideEffects possibleSideEffects, PropertyAttributes attributes = PropertyDynamicTypeDefaults);
        BOOL SetAttribute(DynamicObject* instance, PropertyId propertyId, PropertyAttributes attribute);
        BOOL ClearAttribute(DynamicObject* instance, PropertyId propertyId, PropertyAttributes attribute);
        PropertyAttributes GetAttributesForProperty(PropertyId propertyId);
        void SetExistingProperty_UpdateStateAfterSet(DynamicObject *const instance, const PropertyId propertyId, const PropertyIndex slotIndex, const Var value, const bool slotTypeWasChanged, PropertyValueInfo *const info, const PropertyOperationFlags flags, const SideEffects possibleSideEffects);
        virtual BOOL FreezeImpl(DynamicObject* instance, bool isConvertedType) override;

        // Checks whether conversion to shared type is needed and performs it, then calls actual operation on the shared type.
        // Template method used for PreventExtensions, Seal, Freeze.
        // FType is functor/lambda to perform actual forced operation (such as PreventExtensionsInternal) on the shared type.
        template<typename FType>
        BOOL ConvertToSharedNonExtensibleTypeIfNeededAndCallOperation(DynamicObject* instance, const PropertyRecord* operationInternalPropertyId, FType operation);
        DynamicType* PromoteType(DynamicObject* object, const PropertyRecord* propertyId, const ObjectSlotType slotType, const Var value, const PropertyOperationFlags flags, ScriptContext* scriptContext);

    private:
        void InitializeNewType(const bool shareType, PathTypeHandler *const baseTypeHandler, ScriptContext *const scriptContext);
        void InitializeExistingType(PathTypeHandler *const baseTypeHandler);
        void InitializeTypeCommon(PathTypeHandler *const baseTypeHandler);

    private:
        PropertyIndex GetPropertyIndex(PropertyId propertyId);

    public:
        TypePath * GetTypePath() const { return GetTransitionInfo()->GetTypePath(); }
        PathTypeTransitionInfo *GetRootTransitionInfo() const;
    private:
        PathTypeHandler* GetRootPathTypeHandler() const;

    public:
        static PathTypeHandler *New(ScriptContext *const scriptContext, const bool disableNativeFields = false, bool isShared = true);
        static PathTypeHandler *New(ScriptContext *const scriptContext, const PropertyIndex slotCapacity, uint16 inlineSlotCapacity, uint16 offsetOfInlineSlots, const bool disableNativeFields = false, bool isShared = true);
        static PathTypeHandler *New(ScriptContext *const scriptContext, TypePath *const typePath, const PropertyIndex slotCount, const PropertyIndex propertyCount, const PropertyIndex slotCapacity, uint16 inlineSlotCapacity, uint16 offsetOfInlineSlots, const bool disableNativeFields = false, bool isShared = true);
        static PathTypeHandler *New(ScriptContext *const scriptContext, PathTypeTransitionInfo *const transitionInfo, const PropertyIndex slotCount, const PropertyIndex propertyCount, const PropertyIndex slotCapacity, uint16 inlineSlotCapacity, uint16 offsetOfInlineSlots, const bool disableNativeFields, bool isShared);

    public:
        DynamicType *DeoptimizeObjectHeaderInlining(DynamicObject *const object);

    public:
        bool DoNativeFields() const;

    private:
        PropertyIndex GetSlotCountInternal() const
        {
            return slotCount;
        }

        PropertyIndex GetPropertyCountInternal() const
        {
            return propertyCount;
        }

    public:
        bool HasWastedInlineSlot() const;
    private:
        void SetHasWastedInlineSlot();

    public:
        TypePath *GetSlotIterationInfo(BigPropertyIndex *const slotCountRef, BigPropertyIndex *const propertyCountRef) const;

    private:
        PathTypeTransitionInfo *GetTransitionInfo() const;
        void SetTransitionInfo(PathTypeTransitionInfo *const transitionInfo);

    public:
        ObjectSlotType GetPreferredSuccessorSlotType(const PropertyId propertyId, const ObjectSlotType requiredSlotType) const;

    public:
        DynamicType *GetType() const;
        void SetType(DynamicType *const type);
    private:
        static DynamicType *GetOrCreateType(PathTypeTransitionInfo *const transitionInfo, const PropertyIndex slotCount, const PropertyIndex propertyCount, const bool shareType, DynamicObject *const object = nullptr);

    public:
        void RegisterObjectCreationSiteInfo(ObjectCreationSiteInfo *const objectCreationSiteInfo) const;

    private:
        void ChangeAllSlotStorageToVar(DynamicObject *const object);
    public:
        ObjectSlotType ChangeSlotType(DynamicObject *const object, const PropertyIndex slotIndex, ObjectSlotType newSlotType);
        void ChangeSlotTypes(DynamicObject *const object, PropertyIdToSlotTypeMap *const propertyIdToNewSlotTypeMap);

    private:
        void VerifySlotCapacities() const;
    };

    typedef SimpleDictionaryTypeHandlerBase<PropertyIndex, const PropertyRecord*, true> SimpleDictionaryTypeHandlerWithNontExtensibleSupport;
}
