//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//---------------------------------------------------------------------------

#pragma once

__inline void Js::Amd64StackFrame::EnsureFunctionEntry()
{
    if (!functionEntry)
    {
        functionEntry = RtlLookupFunctionEntry(currentContext->Rip, &imageBase, nullptr);
    }
}

__inline bool Js::Amd64StackFrame::EnsureCallerContext(bool isCurrentContextNative)
{
    if (!hasCallerContext)
    {
        *callerContext = *currentContext;
        if (isCurrentContextNative)
        {
            if (NextFromNativeAddress(callerContext))
            {
                hasCallerContext = true;
                return true;
            }
            return false;
        }
        EnsureFunctionEntry();
        if (Next(callerContext, imageBase, functionEntry))
        {
            hasCallerContext = true;
            return true;
        }
        else
        {
            return false;
        }
    }

    return true;
}

__inline void Js::Amd64StackFrame::OnCurrentContextUpdated()
{
    imageBase = 0;
    functionEntry = nullptr;
    hasCallerContext = false;
}
