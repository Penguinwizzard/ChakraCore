//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

enum CodeGenWorkItemType : byte
{
    JsFunctionType = 0,
#ifdef ENABLE_NATIVE_CODE_SERIALIZATION
    JsFunctionSerializedType,
#endif
    JsLoopBodyWorkItemType
};
