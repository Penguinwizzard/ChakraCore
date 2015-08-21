//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#include "RuntimeLanguagePch.h"

namespace Js
{
#if DBG_DUMP
    FunctionBody* AsmJsJitTemplate::Globals::CurrentEncodingFunction = nullptr;
#endif

    void* AsmJsJitTemplate::InitTemplateData()
    {
        __debugbreak();
        return nullptr;
    }

    void AsmJsJitTemplate::FreeTemplateData(void* userData)
    {
        __debugbreak();
    }
}