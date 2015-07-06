//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

#if DEBUG
#include <stdarg.h>
#endif //DEBUG

void ErrHandler::Throw(HRESULT hr)
{
    Assert(fInited);
    Assert(FAILED(hr));
    m_hr = hr;
    throw ParseExceptionObject(hr);
}
