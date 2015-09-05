//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class JavascriptIterator
    {
    public:
        class EntryInfo
        {
        public:
            static FunctionInfo SymbolIterator;
        };

        static Var EntrySymbolIterator(RecyclableObject* function, CallInfo callInfo, ...);
    };
} // namespace Js
