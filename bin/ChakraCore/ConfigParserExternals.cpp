//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "Common.h"
#include "core\ConfigParser.h"

bool ConfigParserAPI::FillConsoleTitle(__ecount(cchBufferSize) LPWSTR buffer, size_t cchBufferSize, __in LPWSTR moduleName)
{
    return false;
}

void ConfigParserAPI::DisplayInitialOutput(__in LPWSTR moduleName)
{
}

LPWSTR JsUtil::ExternalApi::GetFeatureKeyName()
{
    return L"";
}

extern "C"
{
    // TODO: For now, ChakraCore runs only platform that has MessageBoxW API
    bool IsMessageBoxWPresent()
    {
        return true;
    }
}
