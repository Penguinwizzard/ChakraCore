//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    template<typename T>
    struct AuxArray
    {
        uint32 count;
        T elements[];

        AuxArray(uint32 count) : count(count)
        {
        }

        void SetCount(uint count) { this->count = count; }
        uint32 GetDataSize() const { return sizeof(AuxArray) + sizeof(T) * count; }
    };
    typedef AuxArray<Var> VarArray;


    struct FuncInfoEntry
    {
        uint nestedIndex;
        uint scopeSlot;
    };
    typedef AuxArray<FuncInfoEntry> FuncInfoArray;

};
