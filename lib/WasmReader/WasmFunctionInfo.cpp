//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "WasmReaderPch.h"

#ifdef ENABLE_WASM

namespace Wasm
{

WasmFunctionInfo::WasmFunctionInfo(ArenaAllocator * alloc) : m_alloc(alloc), m_resultType(WasmTypes::Void)
{
    m_i32Consts = Anew(m_alloc, ConstMap<int32>, m_alloc);
    m_i64Consts = Anew(m_alloc, ConstMap<int64>, m_alloc);
    m_f32Consts = Anew(m_alloc, ConstMap<float>, m_alloc);
    m_f64Consts = Anew(m_alloc, ConstMap<double>, m_alloc);
    m_locals = Anew(m_alloc, WasmTypeArray, m_alloc, 0);
    m_params = Anew(m_alloc, WasmTypeArray, m_alloc, 0);
}

void
WasmFunctionInfo::AddLocal(WasmTypes::WasmType type)
{
    m_locals->Add(type);
}

void
WasmFunctionInfo::AddParam(WasmTypes::WasmType type)
{
    m_locals->Add(type);
    m_params->Add(type);
}

template<>
void
WasmFunctionInfo::AddConst<int32>(int32 constVal, Js::RegSlot reg)
{
    int result = m_i32Consts->Add(constVal, reg);
    Assert(result != -1); // REVIEW: should always succeed (or at least throw OOM)?
}

template<>
void
WasmFunctionInfo::AddConst<int64>(int64 constVal, Js::RegSlot reg)
{
    int result = m_i64Consts->Add(constVal, reg);
    Assert(result != -1);
}

template<>
void
WasmFunctionInfo::AddConst<float>(float constVal, Js::RegSlot reg)
{
    int result = m_f32Consts->Add(constVal, reg);
    Assert(result != -1);
}

template<>
void
WasmFunctionInfo::AddConst<double>(double constVal, Js::RegSlot reg)
{
    int result = m_f64Consts->Add(constVal, reg);
    Assert(result != -1);
}

void
WasmFunctionInfo::SetResultType(WasmTypes::WasmType type)
{
    Assert(m_resultType == WasmTypes::Void);
    m_resultType = type;
}

WasmTypes::WasmType
WasmFunctionInfo::GetLocal(uint index) const
{
    if (index < m_locals->Count())
    {
        return m_locals->GetBuffer()[index];
    }
    return WasmTypes::Limit;
}

WasmTypes::WasmType
WasmFunctionInfo::GetParam(uint index) const
{
    if (index < m_params->Count())
    {
        return m_params->GetBuffer()[index];
    }
    return WasmTypes::Limit;
}

template<>
Js::RegSlot
WasmFunctionInfo::GetConst<int32>(int32 constVal) const
{
    return m_i32Consts->Lookup(constVal, Js::Constants::NoRegister);
}

template<>
Js::RegSlot
WasmFunctionInfo::GetConst<int64>(int64 constVal) const
{
    return m_i64Consts->Lookup(constVal, Js::Constants::NoRegister);
}

template<>
Js::RegSlot
WasmFunctionInfo::GetConst<float>(float constVal) const
{
    return m_f32Consts->Lookup(constVal, Js::Constants::NoRegister);
}

template<>
Js::RegSlot
WasmFunctionInfo::GetConst<double>(double constVal) const
{
    return m_f64Consts->Lookup(constVal, Js::Constants::NoRegister);
}

WasmTypes::WasmType
WasmFunctionInfo::GetResultType() const
{
    return m_resultType;
}

uint32 WasmFunctionInfo::GetLocalCount() const
{
    return m_locals->Count();
}

uint32 WasmFunctionInfo::GetParamCount() const
{
    return m_params->Count();
}

} // namespace Wasm
#endif // ENABLE_WASM
