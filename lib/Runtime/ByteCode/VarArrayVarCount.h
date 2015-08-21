//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    struct VarArrayVarCount
    {
        Var count;
        Var elements[];

        VarArrayVarCount(Var count) : count(count)
        {
        }

        void SetCount(uint count);
        uint32 GetDataSize() const;
    };
};
