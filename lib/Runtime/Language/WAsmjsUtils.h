//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
// Portions of this file are copyright 2014 Mozilla Foundation, available under the Apache 2.0 license.
//-------------------------------------------------------------------------------------------------------

#pragma once

#ifndef TEMP_DISABLE_ASMJS

namespace WAsmJs
{
    typedef Js::RegSlot RegSlot;

    template<typename ToType> uint32 ConvertOffset(uint32 ptr, uint32 fromSize)
    {
        if (fromSize == sizeof(ToType))
        {
            return ptr;
        }
        uint64 tmp = ptr * fromSize;
        tmp = Math::Align<uint64>(tmp, sizeof(ToType));
        tmp /= sizeof(ToType);
        if (tmp > (uint64)UINT32_MAX)
        {
            Math::DefaultOverflowPolicy();
        }
        return (uint32)tmp;
    }
    template<typename FromType, typename ToType> uint32 ConvertOffset(uint32 ptr)
    {
        return ConvertOffset<ToType>(ptr, sizeof(FromType));
    }
    template<typename T> uint32 ConvertToJsVarOffset(uint32 ptr)
    {
        return ConvertOffset<T, Js::Var>(ptr);
    }
    template<typename T> uint32 ConvertFromJsVarOffset(uint32 ptr)
    {
        return ConvertOffset<Js::Var, T>(ptr);
    }

    struct EmitInfoBase
    {
        EmitInfoBase(RegSlot location_) : location(location_) {}
        EmitInfoBase() : location(Js::Constants::NoRegister) {}

        RegSlot location;
    };

    /// Register space for const, parameters, variables and tmp values
    ///     --------------------------------------------------------
    ///     | Reserved | Consts  | Parameters | Variables | Tmp
    ///     --------------------------------------------------------
    ///     Cannot allocate in any different order
    class RegisterSpace
    {
        // number of const, includes the reserved slots
        RegSlot    mNbConst;

        // Total number of register allocated
        RegSlot   mRegisterCount;

        // location of the first temporary register and last variable + 1
        RegSlot   mFirstTmpReg;

        // Location of the next register to be allocated
        RegSlot   mNextLocation;

        // Number of slots reserved at the beggining of the space
        const RegSlot mReservedCount;
    public:
        // Constructor
        RegisterSpace(RegSlot reservedSlotsCount = 0) :
            mRegisterCount(reservedSlotsCount)
            , mFirstTmpReg(reservedSlotsCount)
            , mNextLocation(reservedSlotsCount)
            , mNbConst(reservedSlotsCount)
            , mReservedCount(reservedSlotsCount)
        {
            Assert(reservedSlotsCount >= 0);
        }

        // Get the number of reserved slots
        RegSlot GetReservedCount() const   { return mReservedCount; }
        // Get the number of const allocated
        RegSlot GetConstCount() const      { return mNbConst; }
        // Get the location of the first temporary register
        RegSlot GetFirstTmpRegister() const{ return mFirstTmpReg; }
        // Get the total number of temporary register allocated
        RegSlot GetTmpCount() const        { return mRegisterCount-mFirstTmpReg; }
        // Get number of local variables
        RegSlot GetVarCount() const        { return mFirstTmpReg - mNbConst; }
        // Get the total number of variable allocated (including temporaries)
        RegSlot GetTotalVariablesCount() const   { return mRegisterCount - mNbConst; }
        RegSlot GetRegisterCount() const   { return mRegisterCount; }

        // Acquire a location for a register. Use only for arguments and Variables
        RegSlot AcquireRegister()
        {
            // Makes sure no temporary register have been allocated yet
            Assert(mFirstTmpReg == mRegisterCount && mNextLocation == mFirstTmpReg);
            ++mFirstTmpReg;
            ++mRegisterCount;
            return mNextLocation++;
        }

        // Acquire a location for a constant
        RegSlot AcquireConstRegister()
        {
            ++mNbConst;
            return AcquireRegister();
        }

        // Acquire a location for a temporary register
        RegSlot AcquireTmpRegister()
        {
            // Make sure this function is called correctly
            Assert(mNextLocation <= mRegisterCount && mNextLocation >= mFirstTmpReg);

            // Allocate a new temp pseudo-register, increasing the locals count if necessary.
            if(mNextLocation == mRegisterCount)
            {
                ++mRegisterCount;
            }
#if DBG_DUMP
            PrintTmpRegisterAllocation(mNextLocation);
#endif
            return mNextLocation++;
        }

        // Release a location for a temporary register, must be the last location acquired
        void ReleaseTmpRegister(RegSlot tmpReg, bool unused = false)
        {
            // make sure the location released is valid
            Assert(tmpReg != Js::Constants::NoRegister);

            // Put this reg back on top of the temp stack (if it's a temp).
            if(IsTmpReg(tmpReg))
            {
                Assert(tmpReg == mNextLocation - 1);
                // If this register hasn't been used and no one requested a register since it was acquired
                // Reduce the space used by this type
                if (unused && tmpReg == mRegisterCount - 1)
                {
                    --mRegisterCount;
                }
#if DBG_DUMP
                PrintTmpRegisterAllocation(mNextLocation - 1, true);
#endif
                mNextLocation--;
            }
        }

        // Checks if the register is a temporary register
        bool IsTmpReg(RegSlot tmpReg)
        {
            Assert(mFirstTmpReg != Js::Constants::NoRegister);
            return !IsConstReg(tmpReg) && tmpReg >= mFirstTmpReg;
        }

        // Checks if the register is a const register
        bool IsConstReg(RegSlot reg)
        {
            // a register is const if it is between the first register and the end of consts
            return reg < mNbConst && reg != 0;
        }

        // Checks if the register is a variable register
        bool IsVarReg(RegSlot reg)
        {
            // a register is a var if it is between the last const and the end
            // equivalent to  reg>=mNbConst && reg<mRegisterCount
            // forcing unsigned, if reg < mNbConst then reg-mNbConst = 0xFFFFF..
            return (uint32_t)(reg - mNbConst) < (uint32_t)(mRegisterCount - mNbConst);
        }

        // Releases a location if its a temporary, safe to call with any expression
        void ReleaseLocation(const EmitInfoBase *pnode)
        {
            // Release the temp assigned to this expression so it can be re-used.
            if(pnode && pnode->location != Js::Constants::NoRegister)
            {
                ReleaseTmpRegister(pnode->location);
            }
        }

        // Checks if the location points to a temporary register
        bool IsTmpLocation(const EmitInfoBase* pnode)
        {
            if(pnode && pnode->location != Js::Constants::NoRegister)
            {
                return IsTmpReg(pnode->location);
            }
            return false;
        }

        // Checks if the location points to a constant register
        bool IsConstLocation(const EmitInfoBase* pnode)
        {
            if(pnode && pnode->location != Js::Constants::NoRegister)
            {
                return IsConstReg(pnode->location);
            }
            return false;
        }

        // Checks if the location points to a variable register
        bool IsVarLocation(const EmitInfoBase* pnode)
        {
            if(pnode && pnode->location != Js::Constants::NoRegister)
            {
                return IsVarReg(pnode->location);
            }
            return false;
        }

        // Checks if the location is valid (within bounds of already allocated registers)
        bool IsValidLocation(const EmitInfoBase* pnode)
        {
            if(pnode && pnode->location != Js::Constants::NoRegister)
            {
                return pnode->location < mRegisterCount;
            }
            return false;
        }

        enum Types
        {
            // sorted by byte size
            INT32,
            FLOAT32,
            INT64,
            FLOAT64,
            SIMD,
            LIMIT
        };
        template<typename T> static RegisterSpace::Types GetRegisterSpaceType(){return RegisterSpace::LIMIT;}
        template<> static RegisterSpace::Types GetRegisterSpaceType<int32>(){return RegisterSpace::INT32;}
        template<> static RegisterSpace::Types GetRegisterSpaceType<int64>(){return RegisterSpace::INT64;}
        template<> static RegisterSpace::Types GetRegisterSpaceType<float>(){return RegisterSpace::FLOAT32;}
        template<> static RegisterSpace::Types GetRegisterSpaceType<double>(){return RegisterSpace::FLOAT64;}
        template<> static RegisterSpace::Types GetRegisterSpaceType<AsmJsSIMDValue>(){return RegisterSpace::SIMD;}
        static uint32 GetTypeByteSize(RegisterSpace::Types type);
#if DBG_DUMP
        // Used for debugging
        Types mType;
        static void GetTypeDebugName(RegisterSpace::Types type, char16* buf, uint bufsize, bool shortName = false);
        void PrintTmpRegisterAllocation(RegSlot loc, bool deallocation = false);
#endif
    };

    template<typename T>
    class ConstRegisterSpace
    {
        template <typename T>
        struct Comparer : public DefaultComparer<T> {};

        template <>
        struct Comparer<float>
        {
            static bool Equals(float x, float y)
            {
                int32 i32x = *(int32*)&x;
                int32 i32y = *(int32*)&y;
                return i32x == i32y;
            }

            static hash_t GetHashCode(float i)
            {
                return (hash_t)i;
            }
        };

        template <>
        struct Comparer<double>
        {
            static bool Equals(double x, double y)
            {
                int64 i64x = *(int64*)&x;
                int64 i64y = *(int64*)&y;
                return i64x == i64y;
            }

            static hash_t GetHashCode(double d)
            {
                int64 i64 = *(int64*)&d;
                return (uint)((i64 >> 32) ^ (uint)i64);
            }
        };
        typedef JsUtil::BaseDictionary<T, RegSlot, ArenaAllocator, PowerOf2SizePolicy, Comparer> ConstMap;
        ConstMap mConstMap;
    public:
        ConstRegisterSpace(ArenaAllocator* allocator) : mConstMap(allocator) {}

        void AddConst(T val, RegSlot loc)
        {
            if(!mConstMap.ContainsKey(val))
            {
                mConstMap.Add(val, loc);
            }
        }

        RegSlot GetConstRegister(T val) const
        {
            return mConstMap.LookupWithKey(val, Js::Constants::NoRegister);
        }
        const ConstMap GetConstMap()
        {
            return mConstMap;
        }
    };

    struct TypedSlotInfo
    {
        uint32 reservedCount;
        uint32 constCount;
        uint32 varCount;
        uint32 tmpCount;
        uint32 byteOffset;
    };

    struct TypedConstsInfo
    {
        struct TypedConstInfo
        {
            uint32 byteOffset;
            uint32 reserved;
            uint32 count;
        } infos[RegisterSpace::LIMIT];
        TypedConstInfo& operator[](int i) {return infos[i];}
        const TypedConstInfo& operator[](int i) const {return infos[i];}
    };

    class TypedRegisterAllocator
    {
        RegisterSpace* mTypeSpaces[RegisterSpace::LIMIT];
        void* mConstRegisterSpaces[RegisterSpace::LIMIT];
    public:
        TypedRegisterAllocator(ArenaAllocator* allocator, uint32 reservedSpace, bool allocateConstSpace);

        // Register Space manipulations
        RegSlot AcquireRegister       (RegisterSpace::Types type)                            {return GetRegisterSpace(type)->AcquireRegister();}
        RegSlot AcquireConstRegister  (RegisterSpace::Types type)                            {return GetRegisterSpace(type)->AcquireConstRegister();}
        RegSlot AcquireTmpRegister    (RegisterSpace::Types type)                            {return GetRegisterSpace(type)->AcquireTmpRegister();}
        void ReleaseTmpRegister       (RegisterSpace::Types type, RegSlot tmpReg, bool unused = false){GetRegisterSpace(type)->ReleaseTmpRegister(tmpReg, unused);}
        void ReleaseLocation          (RegisterSpace::Types type, const EmitInfoBase* pnode) {GetRegisterSpace(type)->ReleaseLocation(pnode); }
        bool IsTmpLocation            (RegisterSpace::Types type, const EmitInfoBase* pnode) {return GetRegisterSpace(type)->IsTmpLocation(pnode);}
        bool IsConstLocation          (RegisterSpace::Types type, const EmitInfoBase* pnode) {return GetRegisterSpace(type)->IsConstLocation(pnode);}
        bool IsVarLocation            (RegisterSpace::Types type, const EmitInfoBase* pnode) {return GetRegisterSpace(type)->IsVarLocation(pnode);}
        bool IsValidLocation          (RegisterSpace::Types type, const EmitInfoBase* pnode) {return GetRegisterSpace(type)->IsValidLocation(pnode);}
        RegSlot GetTotalVariablesCount(RegisterSpace::Types type) const                      {return GetRegisterSpace(type)->GetTotalVariablesCount();}
        uint32 GetJsVarCount(RegisterSpace::Types type, bool constOnly = false) const;

        // Whole allocator methods
        uint32 GetTotalJsVarCount(bool constOnly = false) const;
        void CommitToFunctionInfo(Js::AsmJsFunctionInfo* funcInfo) const;
        void WriteConstToTable(void* table);
        bool IsTypeUsed(RegisterSpace::Types type) const;
        TypedConstsInfo GetTypedConstsInfo() const;
#if DBG_DUMP
        // Dumpers
        void DumpConstants(void* table) const;
        void DumpLocalsInfo() const;
        // indexes' array size must be WAsmJs::RegisterSpace::LIMIT
        void GetArgumentStartIndex(uint32* indexes) const;
#endif

        // Template version for ease of use with current code, should aim at removing uses of these
        template<typename T>RegSlot AcquireRegister     ()                          {return AcquireRegister(RegisterSpace::GetRegisterSpaceType<T>());}
        template<typename T>RegSlot AcquireConstRegister()                          {return AcquireConstRegister(RegisterSpace::GetRegisterSpaceType<T>());}
        template<typename T>RegSlot AcquireTmpRegister  ()                          {return AcquireTmpRegister(RegisterSpace::GetRegisterSpaceType<T>());}
        template<typename T>void ReleaseTmpRegister     (RegSlot tmpReg, bool unused = false){ReleaseTmpRegister(RegisterSpace::GetRegisterSpaceType<T>(), tmpReg, unused);}
        template<typename T>void ReleaseLocation        (const EmitInfoBase* pnode) {ReleaseLocation(RegisterSpace::GetRegisterSpaceType<T>(), pnode); }
        template<typename T>bool IsTmpLocation          (const EmitInfoBase* pnode) {return IsTmpLocation(RegisterSpace::GetRegisterSpaceType<T>(), pnode);}
        template<typename T>bool IsConstLocation        (const EmitInfoBase* pnode) {return IsConstLocation(RegisterSpace::GetRegisterSpaceType<T>(), pnode);}
        template<typename T>bool IsVarLocation          (const EmitInfoBase* pnode) {return IsVarLocation(RegisterSpace::GetRegisterSpaceType<T>(), pnode);}
        template<typename T>bool IsValidLocation        (const EmitInfoBase* pnode) {return IsValidLocation(RegisterSpace::GetRegisterSpaceType<T>(), pnode);}

        // ConstRegisterSpace methods
        template<typename T> void AddConst(T val)
        {
            RegisterSpace::Types type = RegisterSpace::GetRegisterSpaceType<T>();
            ConstRegisterSpace<T>* constSpace = (ConstRegisterSpace<T>*)GetConstSpace(type);
            RegSlot loc = constSpace->GetConstRegister(val);
            if (loc == Js::Constants::NoRegister)
            {
                loc = GetRegisterSpace(type)->AcquireConstRegister();
                constSpace->AddConst(val, loc);
            }
        }

        template<typename T> RegSlot GetConstRegister(T val)
        {
            RegisterSpace::Types type = RegisterSpace::GetRegisterSpaceType<T>();
            ConstRegisterSpace<T>* constSpace = (ConstRegisterSpace<T>*)GetConstSpace(type);
            return constSpace->GetConstRegister(val);
        }
    private:
        bool IsValidType(RegisterSpace::Types type) const;
        void* GetConstSpace(RegisterSpace::Types type) const;
        RegisterSpace* GetRegisterSpace(RegisterSpace::Types type) const;
    };
};

#endif
