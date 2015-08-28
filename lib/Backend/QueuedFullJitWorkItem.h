//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

struct CodeGenWorkItem;

class QueuedFullJitWorkItem : public JsUtil::DoublyLinkedListElement<QueuedFullJitWorkItem>
{
private:
    CodeGenWorkItem *const workItem;

public:
    QueuedFullJitWorkItem(CodeGenWorkItem *const workItem);

public:
    CodeGenWorkItem *WorkItem() const;
};
