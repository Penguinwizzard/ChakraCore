//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    typedef JsUtil::MruDictionary<UnifiedRegex::RegexKey, UnifiedRegex::RegexPattern*> RegexPatternMruMap;
};

namespace JsUtil
{
    template <>
    class ValueEntry<Js::RegexPatternMruMap::MruDictionaryData>: public BaseValueEntry<Js::RegexPatternMruMap::MruDictionaryData>
    {
    public:
        void Clear()
        {
            this->value = 0;
        }
    };

};
