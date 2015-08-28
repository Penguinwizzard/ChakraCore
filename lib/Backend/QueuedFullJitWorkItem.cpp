//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#include "BackEnd.h"

QueuedFullJitWorkItem::QueuedFullJitWorkItem(CodeGenWorkItem *const workItem) : workItem(workItem)
{
    Assert(workItem->GetJitMode() == ExecutionMode::FullJit);
}

CodeGenWorkItem *QueuedFullJitWorkItem::WorkItem() const
{
    return workItem;
}
