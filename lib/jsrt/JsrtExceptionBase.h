//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

class JsrtExceptionBase : public Js::ExceptionBase
{
public:
    virtual JsErrorCode GetJsErrorCode() = 0;
};