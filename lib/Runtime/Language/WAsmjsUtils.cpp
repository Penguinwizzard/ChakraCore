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
#if DBG_DUMP

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

    void RegisterSpace::PrintTmpRegisterAllocation(RegSlot loc)
    {
        if (PHASE_ON1(Js::AsmjsTmpRegisterAllocationPhase))
        {
            // Since this needs to be done manually for each type, this assert will make sure to not forget to update this if a new type is added
            CompileAssert(RegisterSpace::LIMIT == 5);
            switch (mType)
            {
            case INT32  : Output::Print(_u("+I32 %d\n"), loc); break;
            case INT64  : Output::Print(_u("+I64 %d\n"), loc); break;
            case FLOAT32: Output::Print(_u("+F32 %d\n"), loc); break;
            case FLOAT64: Output::Print(_u("+F64 %d\n"), loc); break;
            case SIMD   : Output::Print(_u("+SIMD %d\n"), loc); break;
            default     : break;
            }
        }
    }

    void RegisterSpace::PrintTmpRegisterDeAllocation(RegSlot loc)
    {
        if (PHASE_ON1(Js::AsmjsTmpRegisterAllocationPhase))
        {
            // Since this needs to be done manually for each type, this assert will make sure to not forget to update this if a new type is added
            CompileAssert(RegisterSpace::LIMIT == 5);
            switch (mType)
            {
            case INT32  : Output::Print(_u("-I32 %d\n"), loc); break;
            case INT64  : Output::Print(_u("-I64 %d\n"), loc); break;
            case FLOAT32: Output::Print(_u("-F32 %d\n"), loc); break;
            case FLOAT64: Output::Print(_u("-F64 %d\n"), loc); break;
            case SIMD   : Output::Print(_u("-SIMD %d\n"), loc); break;
            default     : break;
            }
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

    uint32 TypedRegisterAllocator::GetJsVarCount(bool constOnly) const
    {
        uint32 total = 0;
        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;
            if (IsTypeUsed(type))
            {
                RegisterSpace* registerSpace = GetRegisterSpace(type);
                uint32 typeSize = RegisterSpace::GetTypeByteSize(type);
                uint32 count = constOnly ? registerSpace->GetConstCount() : registerSpace->GetTotalVariablesCount();
                uint32 totalVar = UInt32Math::Mul(count, typeSize);
                totalVar = Math::AlignOverflowCheck<uint32>(totalVar, Js::AsmJsMath::Max<uint32>(sizeof(Js::Var), typeSize));
                total = UInt32Math::Add(total, totalVar / sizeof(Js::Var));
            }
        }
        return total;
    }

    void TypedRegisterAllocator::CommitToFunctionInfo(Js::AsmJsFunctionInfo* funcInfo)
    {
        uint32 offset = Js::AsmJsFunctionMemory::RequiredVarConstants * sizeof(Js::Var);
#if ENABLE_DEBUG_CONFIG_OPTIONS
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
                slotInfo.constCount = registerSpace->GetConstCount();
                slotInfo.varCount = registerSpace->GetVarCount();
                slotInfo.tmpCount = registerSpace->GetTmpCount();
                offset = Math::AlignOverflowCheck(offset, RegisterSpace::GetTypeByteSize(type));
                slotInfo.offset = offset;

#if ENABLE_DEBUG_CONFIG_OPTIONS
                if (PHASE_TRACE1(Js::AsmjsInterpreterStackPhase))
                {
                    switch(type)
                    {
                    case RegisterSpace::INT32: Output::Print(_u("INT32 ")); break;
                    case RegisterSpace::INT64: Output::Print(_u("INT64 ")); break;
                    case RegisterSpace::FLOAT32: Output::Print(_u("FLOAT32 ")); break;
                    case RegisterSpace::FLOAT64: Output::Print(_u("FLOAT64 ")); break;
                    case RegisterSpace::SIMD: Output::Print(_u("SIMD ")); break;
                    default: Output::Print(_u("Unknown ")); break;
                    }
                    Output::Print(_u("Offset:%d  ConstCount:%d  VarCount:%d  TmpCount:%d\n"),
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
#if ENABLE_DEBUG_CONFIG_OPTIONS
        if (PHASE_TRACE1(Js::AsmjsInterpreterStackPhase))
        {
            Output::Print(_u("\n"));
        }
#endif
    }

    void TypedRegisterAllocator::WriteConstToTable(void* table)
    {
#define IterateConstMapForType(primitiveType, zero) {\
        primitiveType* tmpTable = (primitiveType*)table;\
        RegisterSpace* registerSpace = GetRegisterSpace(type);\
        auto map = ((ConstRegisterSpace<primitiveType>*)GetConstSpace(type))->GetConstMap();\
        for (int j = map.Count() - registerSpace->GetConstCount(); j >= 0; --j)\
        {\
            *tmpTable = zero;\
            ++tmpTable;\
        }\
        for (auto it = map.GetIterator(); it.IsValid(); it.MoveNext())\
        {\
            auto &entry = it.Current();\
            *tmpTable = entry.Key();\
            tmpTable++;\
        }\
        table = (void*)tmpTable;\
    }

        for (int i = 0; i < RegisterSpace::LIMIT; ++i)
        {
            RegisterSpace::Types type = (RegisterSpace::Types)i;

            // Check if we don't want to commit this type
            if (IsTypeUsed(type))
            {
                switch(type)
                {
                case RegisterSpace::INT32:
                    IterateConstMapForType(int, 0);
                    break;
                case RegisterSpace::INT64:
                    IterateConstMapForType(int64, 0);
                    break;
                case RegisterSpace::FLOAT32:
                    IterateConstMapForType(float, 0.0f);
                    break;
                case RegisterSpace::FLOAT64:
                    IterateConstMapForType(double, 0.0);
                    break;
                case RegisterSpace::SIMD:
                    AsmJsSIMDValue zero;
                    zero.Zero();
                    IterateConstMapForType(AsmJsSIMDValue, zero);
                    break;
                case RegisterSpace::LIMIT:
                default:
                    break;
                }
            }
        }
#undef IterateConstMapForType
    }

    bool TypedRegisterAllocator::IsValidType(RegisterSpace::Types type) const
    {
        return (uint)type < RegisterSpace::LIMIT;
    }

    bool TypedRegisterAllocator::IsTypeUsed(RegisterSpace::Types type) const
    {
        return GetRegisterSpace(type)->GetTotalVariablesCount() > 0;
    }

    void* TypedRegisterAllocator::GetConstSpace(RegisterSpace::Types type) const
    {
        AssertMsg(mConstRegisterSpaces[type], "Did you forget to allocate const space in TypedRegisterAllocator constructor?");
        if (IsValidType(type) || !mConstRegisterSpaces[type])
        {
            Assert("Invalid type for RegisterSpace in TypedMemoryStructure");
            Js::Throw::InternalError();
        }

        return mConstRegisterSpaces[type];
    }

    RegisterSpace* TypedRegisterAllocator::GetRegisterSpace(RegisterSpace::Types type) const
    {
        if (IsValidType(type))
        {
            Assert("Invalid type for RegisterSpace in TypedMemoryStructure");
            Js::Throw::InternalError();
        }
        return mTypeSpaces[type];
    }
};

#endif
