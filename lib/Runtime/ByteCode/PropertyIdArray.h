//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    struct PropertyIdArray
    {
        uint32 count;
        bool   hadDuplicates;
        bool   has__proto__; // Only used for object literal
        bool   hasNonSimpleParams;
        bool   hasAccessors;
        PropertyId elements[];
        PropertyIdArray(uint32 count) : count(count), hasNonSimpleParams(false), hadDuplicates(false), has__proto__(false), hasAccessors(false)
        {
        }

        size_t GetDataSize(uint32 extraSlots) const { return sizeof(PropertyIdArray) + sizeof(PropertyId) * (count + extraSlots); }
    };
};
