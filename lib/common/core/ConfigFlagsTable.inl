//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{

    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// class String
    ///
    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
   
    inline 
    String::String()
    {
        this->pszValue = NULL;
    }

    inline 
    String::String(__in LPWSTR psz)
    {
        this->pszValue = NULL;
        Set(psz);
    }

    inline 
    String::~String()
    {
        if(NULL != this->pszValue)
        {
            NoCheckHeapDeleteArray(wcslen(this->pszValue) + 1, this->pszValue);
        }
    }


    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// class RangeBase
    ///
    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------

    template <typename TRangeUnitData>
    inline void
    RangeBase<TRangeUnitData>::Add(TRangeUnitData i)
    {
        Add(i, i);
    }

    template <typename TRangeUnitData>
    inline void 
    RangeBase<TRangeUnitData>::Add(TRangeUnitData i, TRangeUnitData j)
    {
        range.Add(RangeUnit<TRangeUnitData>(i, j));
    }

    template <typename TRangeUnitData>
    inline bool
    RangeBase<TRangeUnitData>::ContainsAll()
    {
        return range.Count() == 0;
    }

    ///----------------------------------------------------------------------------
    ///
    /// RangeBase::InRange
    ///
    /// Searches for each element in the list of UnitRanges. If the given integer
    /// is between the 2 values, then return true; If no element is present in range
    /// then, then we return true
    ///
    ///----------------------------------------------------------------------------

    template <typename TRangeUnitData>
    inline bool RangeBase<TRangeUnitData>::InRange(TRangeUnitData n)
    {
        if (range.Count() == 0)
        {
            return true;
        }
        else
        {
            return range.MapUntil([n](int, RangeUnit<TRangeUnitData> const& unit)
            {
                return RangeUnitContains(unit, n);
            });
        }
    }


    template <>
    inline bool RangeUnitContains<SourceFunctionNode>(RangeUnit<SourceFunctionNode> unit, SourceFunctionNode n)
    {
        Assert(n.functionId != (uint32)-1);

        if ((n.sourceContextId >= unit.i.sourceContextId) &&
            (n.sourceContextId <= unit.j.sourceContextId)
            )
        {
            if ((n.sourceContextId == unit.j.sourceContextId) && (-2 == unit.j.functionId) ||  //#.#-#.* case
                (n.sourceContextId == unit.i.sourceContextId) && (-2 == unit.i.functionId)     //#.*-#.# case
                )
            {
                return true;
            }

            if ((n.sourceContextId == unit.j.sourceContextId) && (-1 == unit.j.functionId) || //#.#-#.+ case
                (n.sourceContextId == unit.i.sourceContextId) && (-1 == unit.i.functionId)     //#.+-#.# case
                )
            {
                return n.functionId != 0;
            }

            if (((n.sourceContextId == unit.i.sourceContextId) && (n.functionId < unit.i.functionId)) || //excludes all values less than functionId LHS
                ((n.sourceContextId == unit.j.sourceContextId) && (n.functionId > unit.j.functionId))) ////excludes all values greater than functionId RHS
            {
                return false;
            }

            return true;
        }

        return false;
    }

    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// class Phases
    ///
    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------

    inline bool 
    Phases::IsEnabled(Phase phase)
    {
        return this->phaseList[(int)phase].valid;
    }

    inline bool
    Phases::IsEnabled(Phase phase, uint soruceContextId, Js::LocalFunctionId functionId)
    {
        return  this->phaseList[(int)phase].valid &&
                this->phaseList[(int)phase].range.InRange(SourceFunctionNode(soruceContextId, functionId));
    }

    inline bool 
    Phases::IsEnabledForAll(Phase phase)
    {
        return  this->phaseList[(int)phase].valid && 
                this->phaseList[(int)phase].range.ContainsAll();
    }

    inline Range * 
    Phases::GetRange(Phase phase)
    {
        return &this->phaseList[(int)phase].range;
    }


    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// class ConfigFlagsTable
    ///
    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------

    inline String *
    ConfigFlagsTable::GetAsString(Flag flag) const 
    { 
        return reinterpret_cast<String* >(GetProperty(flag)); 
    }

    inline Phases *
    ConfigFlagsTable::GetAsPhase(Flag flag) const
    {
        return reinterpret_cast<Phases*>(GetProperty(flag));
    }

    inline Boolean *
    ConfigFlagsTable::GetAsBoolean(Flag flag)  const
    { 
        return reinterpret_cast<Boolean*>(GetProperty(flag)); 
    }

    inline Number *
    ConfigFlagsTable::GetAsNumber(Flag flag)  const
    { 
        return reinterpret_cast<Number* >(GetProperty(flag)); 
    }

    inline NumberSet *
    ConfigFlagsTable::GetAsNumberSet(Flag flag)  const
    { 
        return reinterpret_cast<NumberSet* >(GetProperty(flag)); 
    }

    inline NumberPairSet *
    ConfigFlagsTable::GetAsNumberPairSet(Flag flag)  const
    { 
        return reinterpret_cast<NumberPairSet* >(GetProperty(flag)); 
    }

    inline NumberRange *
        ConfigFlagsTable::GetAsNumberRange(Flag flag)  const
    {
        return reinterpret_cast<NumberRange* >(GetProperty(flag));
    }

    inline void
    ConfigFlagsTable::Enable(Flag flag) 
    { 
        this->flagPresent[flag] = true; 
    }

    inline void 
    ConfigFlagsTable::Disable(Flag flag)
    {
        this->flagPresent[flag] = false;
    }

    inline bool
    ConfigFlagsTable::IsEnabled(Flag flag) 
    { 
        return this->flagPresent[flag];
    }

    inline bool
    ConfigFlagsTable::IsParentFlag(Flag flag) const
    {
        return this->flagIsParent[flag];
    }

} // namespace Js