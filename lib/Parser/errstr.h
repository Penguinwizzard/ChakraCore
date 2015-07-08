//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

BOOL FGetResourceString(long isz, __out_ecount(cchMax) OLECHAR *psz, int cchMax);
BSTR BstrGetResourceString(long isz);