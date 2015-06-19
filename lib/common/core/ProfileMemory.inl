//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#ifdef PROFILE_MEM
template<typename THandler>
void
MemoryProfiler::WithArenaUsageSummary(bool liveOnly, THandler handler)
{
    // This is debug only code, we don't care if we catch the right exception        
    AUTO_NESTED_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);

    PageAllocator tempPageAlloc(NULL, Js::Configuration::Global.flags);
    ArenaAllocator tempAlloc(L"MemoryProfiler", &tempPageAlloc, Js::Throw::OutOfMemory);
    JsUtil::List<LPWSTR, ArenaAllocator> * name;
    JsUtil::List<ArenaMemoryDataSummary *, ArenaAllocator> * summaries;
    CreateArenaUsageSummary(&tempAlloc, liveOnly, name, summaries);
    handler(name, summaries);
}
#endif
