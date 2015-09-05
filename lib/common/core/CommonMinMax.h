//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

template<class T> inline
_Post_equal_to_(a < b ? a : b) _Post_satisfies_(return <= a && return <= b)
    const T& min(const T& a, const T& b) { return a < b ? a : b; }

template<class T> inline
_Post_equal_to_(a > b ? a : b) _Post_satisfies_(return >= a && return >= b)
    const T& max(const T& a, const T& b) { return a > b ? a : b; }
