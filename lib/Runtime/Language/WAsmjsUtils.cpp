//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
// Portions of this file are copyright 2014 Mozilla Foundation, available under the Apache 2.0 license.
//-------------------------------------------------------------------------------------------------------

#pragma once
#include "RuntimeLanguagePch.h"

#ifndef TEMP_DISABLE_ASMJS

namespace WAsmJs
{
    uint32 RegisterSpace::GetTypeByteSize(RegisterSpace::Types type)
    {
        // Since this needs to be done manually for each type, this assert will make sure to not forget to update this if a new type is added
        CompileAssert(RegisterSpace::LIMIT == 5);
        switch (type)
        {
        case INT32  : return sizeof(int32);
        case INT64  : return sizeof(int64);
        case FLOAT32: return sizeof(float);
        case FLOAT64: return sizeof(double);
        case SIMD   : return sizeof(AsmJsSIMDValue);
        default     : break;
        }
        Js::Throw::InternalError();
    }

#if DBG_DUMP
    void RegisterSpace::GetTypeDebugName(Types type, char16* buf, uint bufsize, bool shortName)
    {
        // Since this needs to be done manually for each type, this assert will make sure to not forget to update this if a new type is added
        CompileAssert(LIMIT == 5);

        switch (type)
        {
        case INT32: wcscpy_s(buf, bufsize  , shortName ? _u("I32"): _u("INT32")); break;
        case INT64: wcscpy_s(buf, bufsize  , shortName ? _u("I64"): _u("INT64")); break;
        case FLOAT32: wcscpy_s(buf, bufsize, shortName ? _u("F32"): _u("FLOAT32")); break;
        case FLOAT64: wcscpy_s(buf, bufsize, shortName ? _u("F64"): _u("FLOAT64")); break;
        case SIMD: wcscpy_s(buf, bufsize   , _u("SIMD")); break;
        default: wcscpy_s(buf, bufsize     , _u("UNKNOWN")); break;
        }
    }

    void RegisterSpace::PrintTmpRegisterAllocation(RegSlot loc, bool deallocation)
    {
        if (PHASE_TRACE1(Js::AsmjsTmpRegisterAllocationPhase))
        {
            char16 buf[16];
            GetTypeDebugName(mType, buf, 16, true);
            Output::Print(_u("%s%s %d\n"), deallocation ? _u("-") : _u("+"), buf, loc);
        }
    }
#endif

    TypedRegisterAllocator::TypedRegisterAllocator(ArenaAllocator* allocator, uint32 reservedSpace, bool allocateConstSpace)
    {
        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            mConstRegisterSpaces[i] = nullptr;
            mTypeSpaces[i] = Anew(allocator, RegisterSpace, reservedSpace);
#if DBG_DUMP
            mTypeSpaces[i]->mType = (RegisterSpace::Types)i;
#endif
        }

        if (allocateConstSpace)
        {
            mConstRegisterSpaces[RegisterSpace::INT32] = Anew(allocator, ConstRegisterSpace<int32>, allocator);
            mConstRegisterSpaces[RegisterSpace::INT64] = Anew(allocator, ConstRegisterSpace<int64>, allocator);
            mConstRegisterSpaces[RegisterSpace::FLOAT32] = Anew(allocator, ConstRegisterSpace<float>, allocator);
            mConstRegisterSpaces[RegisterSpace::FLOAT64] = Anew(allocator, ConstRegisterSpace<double>, allocator);
            mConstRegisterSpaces[RegisterSpace::SIMD] = Anew(allocator, ConstRegisterSpace<AsmJsSIMDValue>, allocator);
            // Since this needs to be done manually for each type, this assert will make sure to not forget to update this if a new type is added
            CompileAssert(RegisterSpace::LIMIT == 5);
        }
    }

    uint32 TypedRegisterAllocator::GetJsVarCount(RegisterSpace::Types type, bool constOnly /*= false*/) const
    {
        if (IsTypeUsed(type))
        {
            RegisterSpace* registerSpace = GetRegisterSpace(type);
            uint32 typeSize = RegisterSpace::GetTypeByteSize(type);
            uint32 count = constOnly ? registerSpace->GetConstCount() : registerSpace->GetTotalVariablesCount();
            return ConvertOffset<Js::Var>(count, typeSize);
        }
        return 0;
    }

    uint32 TypedRegisterAllocator::GetTotalJsVarCount(bool constOnly) const
    {
        uint32 total = 0;
        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;
            total = UInt32Math::Add(total, GetJsVarCount(type, constOnly));
        }
        return total;
    }

    void TypedRegisterAllocator::CommitToFunctionInfo(Js::AsmJsFunctionInfo* funcInfo)
    {
        uint32 offset = Js::AsmJsFunctionMemory::RequiredVarConstants * sizeof(Js::Var);
#if DBG_DUMP
        if (PHASE_TRACE1(Js::AsmjsInterpreterStackPhase))
        {
            Output::Print(_u("ASMFunctionInfo Stack Data\n"));
            Output::Print(_u("==========================\n"));
            Output::Print(_u("RequiredVarConstants:%d\n"), Js::AsmJsFunctionMemory::RequiredVarConstants);
        }
#endif

        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;

            // Check if we don't want to commit this type
            if (IsTypeUsed(type))
            {
                RegisterSpace* registerSpace = GetRegisterSpace(type);
                TypedSlotInfo slotInfo;
                slotInfo.reservedCount = registerSpace->GetReservedCount();
                slotInfo.constCount = registerSpace->GetConstCount();
                slotInfo.varCount = registerSpace->GetVarCount();
                slotInfo.tmpCount = registerSpace->GetTmpCount();
                offset = Math::AlignOverflowCheck(offset, RegisterSpace::GetTypeByteSize(type));
                slotInfo.offset = offset;

                funcInfo->SetTypedSlotInfo(type, slotInfo);
#if DBG_DUMP
                if (PHASE_TRACE1(Js::AsmjsInterpreterStackPhase))
                {
                    char16 buf[16];
                    RegisterSpace::GetTypeDebugName(type, buf, 16);
                    Output::Print(_u("%s Offset:%d  ConstCount:%d  VarCount:%d  TmpCount:%d\n"),
                                  buf,
                                  slotInfo.offset,
                                  slotInfo.constCount,
                                  slotInfo.varCount,
                                  slotInfo.tmpCount);
                }
#endif

                // Update offset for next type
                uint32 totalTypeCount = 0;
                totalTypeCount = UInt32Math::Add(totalTypeCount, slotInfo.constCount);
                totalTypeCount = UInt32Math::Add(totalTypeCount, slotInfo.varCount);
                totalTypeCount = UInt32Math::Add(totalTypeCount, slotInfo.tmpCount);

                offset = UInt32Math::Add(offset, UInt32Math::Mul(totalTypeCount, RegisterSpace::GetTypeByteSize(type)));
            }
        }
#if DBG_DUMP
        if (PHASE_TRACE1(Js::AsmjsInterpreterStackPhase))
        {
            Output::Print(_u("\n"));
        }
#endif
    }

    template<typename T> void WriteConstToTableForType(void** table, RegisterSpace* registerSpace, void* constSpace, T zero)
    {
        T* tmpTable = (T*)*table;
        auto map = ((ConstRegisterSpace<T>*)constSpace)->GetConstMap();
        for (int j = registerSpace->GetReservedCount(); j > 0; --j)
        {
            *tmpTable = zero;
            ++tmpTable;
        }
        for (auto it = map.GetIterator(); it.IsValid(); it.MoveNext())
        {
            auto &entry = it.Current();
            *tmpTable = entry.Key();
            tmpTable++;
        }
        *table = (void*)tmpTable;
    }

    void TypedRegisterAllocator::WriteConstToTable(void* table)
    {
        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;

            // Check if we don't want to commit this type
            if (IsTypeUsed(type))
            {
                RegisterSpace* registerSpace = GetRegisterSpace(type);
                void* constSpace = GetConstSpace(type);
                switch(type)
                {
                case RegisterSpace::INT32:
                    WriteConstToTableForType<int>(&table, registerSpace, constSpace, 0);
                    break;
                case RegisterSpace::INT64:
                    WriteConstToTableForType<int64>(&table, registerSpace, constSpace, 0);
                    break;
                case RegisterSpace::FLOAT32:
                    WriteConstToTableForType<float>(&table, registerSpace, constSpace, 0.f);
                    break;
                case RegisterSpace::FLOAT64:
                    WriteConstToTableForType<double>(&table, registerSpace, constSpace, 0.0);
                    break;
                case RegisterSpace::SIMD:
                    AsmJsSIMDValue zero;
                    zero.Zero();
                    WriteConstToTableForType<AsmJsSIMDValue>(&table, registerSpace, constSpace, zero);
                    break;
                case RegisterSpace::LIMIT:
                default:
                    break;
                }
            }
        }
    }

    bool TypedRegisterAllocator::IsValidType(RegisterSpace::Types type) const
    {
        return (uint)type < RegisterSpace::LIMIT;
    }

    bool TypedRegisterAllocator::IsTypeUsed(RegisterSpace::Types type) const
    {
        return GetRegisterSpace(type)->GetTotalVariablesCount() > 0;
    }

#if DBG_DUMP
    template<typename T, typename F> void DumpConstantsForType(void** table, char16* typeName, char16* shortTypeName, uint32 start, uint32 constCount, F printFn)
    {
        T* typedTable = (T*)*table;
        if (constCount > 0) 
        {
            Output::Print(_u("    Constant %s:\n    ======== =======\n    "));
            for (uint32 i = 0; i < constCount; i++)
            {
                // Do not print reserved slots
                if (i >= start)
                {
                    Output::Print(_u(" %s "), shortTypeName);
                    printFn(i, *typedTable);
                }
                ++typedTable;
            }
        }
        *table = (void*)typedTable;
    }
    void TypedRegisterAllocator::DumpConstants(void* table) const
    {
        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;

            if (IsTypeUsed(type))
            {
                RegisterSpace* registerSpace = GetRegisterSpace(type);
                uint32 start = registerSpace->GetReservedCount();
                uint32 constCount = registerSpace->GetConstCount();
                if (start >= constCount)
                {
                    // No constants to print
                    continue;
                }
                char16 typeName[16];
                char16 shortTypeName[16];
                RegisterSpace::GetTypeDebugName(type, typeName, 16);
                RegisterSpace::GetTypeDebugName(type, shortTypeName, 16, true);
                switch(type)
                {
                case RegisterSpace::INT32:
                    DumpConstantsForType<int>(&table, typeName, shortTypeName, start, constCount, [](int i, int val) -> void {
                        Output::Print(_u("%d  %d\n    "), i, val);
                    });
                    break;
                case RegisterSpace::INT64:
                    DumpConstantsForType<int64>(&table, typeName, shortTypeName, start, constCount, [](int i, int64 val) {
                        Output::Print(_u("%d  %lld\n    "), i, val);
                    });
                    break;
                case RegisterSpace::FLOAT32:
                    DumpConstantsForType<float>(&table, typeName, shortTypeName, start, constCount, [](int i, float val) {
                        Output::Print(_u("%d  %.4f\n    "), i, val);
                    });
                    break;
                case RegisterSpace::FLOAT64:
                    DumpConstantsForType<double>(&table, typeName, shortTypeName, start, constCount, [](int i, double val) {
                        Output::Print(_u("%d  %.4f\n    "), i, val);
                    });
                    break;
                case RegisterSpace::SIMD:
                    DumpConstantsForType<SIMDValue>(&table, typeName, shortTypeName, start, constCount, [](int i, SIMDValue val) {
                        SIMDValue* value = &val;
                        Output::Print(_u("%d\n"), i);
                        Output::Print(_u("    I4(%d, %d, %d, %d),\n"), value->i32[SIMD_X], value->i32[SIMD_Y], value->i32[SIMD_Z], value->i32[SIMD_W]);
                        Output::Print(_u("    F4(%.4f, %.4f, %.4f, %.4f),\n"), value->f32[SIMD_X], value->f32[SIMD_Y], value->f32[SIMD_Z], value->f32[SIMD_W]);
                        Output::Print(_u("    D2(%.4f, %.4f)\n"), value->f64[SIMD_X], value->f64[SIMD_Y]);
                        Output::Print(_u("    I8(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d )\n"), 
                                      value->i8[0], value->i8[1], value->i8[2], value->i8[3], value->i8[4], value->i8[5], value->i8[6], value->i8[7],
                                      value->i8[8], value->i8[9], value->i8[10], value->i8[11], value->i8[12], value->i8[13], value->i8[14], value->i8[15]);
                    });
                    break;
                case RegisterSpace::LIMIT:
                default:
                    break;
                }
                Output::Print(_u("\n"));
            }
        }
    }

    void TypedRegisterAllocator::DumpLocalsInfo() const
    {
        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;
            if (IsTypeUsed(type))
            {
                char16 typeName[16];
                char16 shortTypeName[16];
                RegisterSpace::GetTypeDebugName(type, typeName, 16);
                RegisterSpace::GetTypeDebugName(type, shortTypeName, 16, true);
                RegisterSpace* registerSpace = GetRegisterSpace(type);
                Output::Print(
                    _u("     %-10s : %u locals (%u temps from %s %u)\n"),
                    typeName,
                    registerSpace->GetVarCount(),
                    registerSpace->GetTmpCount(),
                    shortTypeName,
                    registerSpace->GetFirstTmpRegister());
            }
        }
    }

    void TypedRegisterAllocator::GetArgumentStartIndex(uint32* indexes) const
    {
        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;
            // Arguments starts right after the consts
            indexes[i] = GetRegisterSpace(type)->GetConstCount();
        }
    }
#endif

    void* TypedRegisterAllocator::GetConstSpace(RegisterSpace::Types type) const
    {
        AssertMsg(mConstRegisterSpaces[type], "Did you forget to allocate const space in TypedRegisterAllocator constructor?");
        if (!IsValidType(type) || !mConstRegisterSpaces[type])
        {
            Assert("Invalid type for RegisterSpace in TypedMemoryStructure");
            Js::Throw::InternalError();
        }

        return mConstRegisterSpaces[type];
    }

    RegisterSpace* TypedRegisterAllocator::GetRegisterSpace(RegisterSpace::Types type) const
    {
        if (!IsValidType(type))
        {
            Assert("Invalid type for RegisterSpace in TypedMemoryStructure");
            Js::Throw::InternalError();
        }
        return mTypeSpaces[type];
    }
};

#endif
