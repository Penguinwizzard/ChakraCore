//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

#include "WasmReaderPch.h"

#ifdef ENABLE_WASM

namespace Wasm
{
WasmRegisterSpace::WasmRegisterSpace() :
    m_registerCount(0),
    m_varCount(0),
    m_tmpCount(0),
    m_constCount(0),
    m_nextLocation(0),
    m_firstTmpReg(0),
    m_nextConstLocation(FirstConstReg)
{
}

WasmRegisterSpace::WasmRegisterSpace(Js::RegSlot reservedCount) :
    m_registerCount(reservedCount),
    m_varCount(reservedCount),
    m_tmpCount(0),
    m_constCount(0),
    m_nextLocation(reservedCount),
    m_firstTmpReg(reservedCount),
    m_nextConstLocation(FirstConstReg)
{
}

Js::RegSlot
WasmRegisterSpace::GetConstCount() const
{
    return m_constCount;
}

Js::RegSlot
WasmRegisterSpace::GetFirstTmpRegister() const
{
    return m_firstTmpReg;
}

Js::RegSlot
WasmRegisterSpace::GetTmpCount() const
{
    return m_tmpCount;
}

Js::RegSlot
WasmRegisterSpace::GetVarCount() const
{
    return m_varCount;
}

Js::RegSlot
WasmRegisterSpace::GetTotalVarCount() const
{
    return m_varCount + m_tmpCount;
}

Js::RegSlot
WasmRegisterSpace::GetRegisterCount() const
{
    return m_registerCount;
}

Js::RegSlot
WasmRegisterSpace::AcquireRegister()
{
    // Makes sure no temporary register have been allocated yet
    Assert(m_firstTmpReg == m_varCount && m_nextLocation == m_firstTmpReg);
    ++m_firstTmpReg;
    ++m_varCount;
    ++m_registerCount;
    return m_nextLocation++;
}

Js::RegSlot
WasmRegisterSpace::AcquireConstRegister()
{
    ++m_constCount;
    ++m_registerCount;
    return m_nextConstLocation--;
}

Js::RegSlot
WasmRegisterSpace::AcquireTmpRegister()
{
    // Make sure this function is called correctly
    Assert(m_nextLocation <= m_registerCount && m_nextLocation >= m_firstTmpReg);

    // Allocate a new temp pseudo-register, increasing the locals count if necessary.
    if (m_nextLocation == m_registerCount)
    {
        ++m_registerCount;
        ++m_tmpCount;
    }
    return m_nextLocation++;
}

void
WasmRegisterSpace::ReleaseTmpRegister(Js::RegSlot tmpReg)
{
    // make sure the location released is valid
    Assert(tmpReg != Js::Constants::NoRegister);

    // Put this reg back on top of the temp stack (if it's a temp).
    if (IsTmpReg(tmpReg))
    {
        Assert(tmpReg == m_nextLocation - 1);
        --m_nextLocation;
        --m_tmpCount;
    }
}

bool
WasmRegisterSpace::IsTmpReg(Js::RegSlot tmpReg) const
{
    Assert(m_firstTmpReg != Js::Constants::NoRegister);
    return !IsConstReg(tmpReg) && tmpReg >= m_firstTmpReg;
}

bool
WasmRegisterSpace::IsConstReg(Js::RegSlot reg) const
{
    return reg > m_nextConstLocation && reg < Js::Constants::NoRegister;
}

bool
WasmRegisterSpace::IsVarReg(Js::RegSlot reg) const
{
    return reg < m_firstTmpReg;
}

void
WasmRegisterSpace::ReleaseLocation(const EmitInfo * info)
{
    // Release the temp assigned to this expression so it can be re-used.
    if (info && info->location != Js::Constants::NoRegister)
    {
        ReleaseTmpRegister(info->location);
    }
}

bool
WasmRegisterSpace::IsTmpLocation(const EmitInfo * info)
{
    if (info && info->location != Js::Constants::NoRegister)
    {
        return IsTmpReg(info->location);
    }
    return false;
}

bool
WasmRegisterSpace::IsConstLocation(const EmitInfo * info)
{
    if (info && info->location != Js::Constants::NoRegister)
    {
        return IsConstReg(info->location);
    }
    return false;
}

bool
WasmRegisterSpace::IsVarLocation(const EmitInfo * info)
{
    if (info && info->location != Js::Constants::NoRegister)
    {
        return IsVarReg(info->location);
    }
    return false;
}

bool
WasmRegisterSpace::IsValidLocation(const EmitInfo * info)
{
    if (info && info->location != Js::Constants::NoRegister)
    {
        return IsConstReg(info->location) || info->location < GetTotalVarCount();
    }
    return false;
}

Js::RegSlot
WasmRegisterSpace::AcquireRegisterAndReleaseLocations(const EmitInfo * lhs, const EmitInfo * rhs)
{
    Js::RegSlot tmpRegToUse;
    if (IsTmpLocation(lhs))
    {
        tmpRegToUse = lhs->location;
        ReleaseLocation(rhs);
    }
    else if (IsTmpLocation(rhs))
    {
        tmpRegToUse = rhs->location;
        ReleaseLocation(lhs);
    }
    else
    {
        tmpRegToUse = AcquireTmpRegister();
        ReleaseLocation(rhs);
        ReleaseLocation(lhs);
    }
    return tmpRegToUse;
}

} // namespace Wasm

#endif // ENABLE_WASM
