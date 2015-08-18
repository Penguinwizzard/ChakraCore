//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    // These function needs to be in INL file for static lib
    inline bool JavascriptBoolean::Is(Var aValue)
    {
        return JavascriptOperators::GetTypeId(aValue) == TypeIds_Boolean;
    }

    inline JavascriptBoolean* JavascriptBoolean::FromVar(Js::Var aValue)
    {
        AssertMsg(Is(aValue), "Ensure var is actually a 'JavascriptBoolean'");
        
        return static_cast<JavascriptBoolean *>(RecyclableObject::FromVar(aValue));
    }

} // namespace Js

