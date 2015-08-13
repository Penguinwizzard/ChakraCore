//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"
#include "JSONStack.h"
namespace JSON
{   
    bool StrictEqualsObjectComparer::Equals(Js::Var x, Js::Var y)
    {
        return JSONStack::Equals(x,y);
    }

    JSONStack::JSONStack(ArenaAllocator *allocator, Js::ScriptContext *context) : jsObjectStack(allocator), domObjectStack(null), alloc(allocator), scriptContext(context)
    {
    }

    bool JSONStack::Equals(Js::Var x, Js::Var y)
    {
        return Js::JavascriptOperators::StrictEqual(x, y, ((Js::RecyclableObject *)x)->GetScriptContext()) == TRUE;
    }

    bool JSONStack::Has(Js::Var data, bool bJsObject) const
    {
        if (bJsObject)
        {
            return jsObjectStack.Has(data);
        }
        else if (domObjectStack)
        {
            return domObjectStack->Contains(data);
        }
        return false;
    }

    bool JSONStack::Push(Js::Var data, bool bJsObject)
    {
        if (bJsObject)
        {
            return jsObjectStack.Push(data);
        }
        EnsuresDomObjectStack();
        domObjectStack->Add(data);
        return true;
    }

    void JSONStack::Pop(bool bJsObject)
    {
        if (bJsObject)
        {
            jsObjectStack.Pop();
            return;
        }
        AssertMsg(domObjectStack != NULL, "Misaligned pop");
        domObjectStack->RemoveAtEnd();
    }

    void JSONStack::EnsuresDomObjectStack(void)
    {
        if (!domObjectStack)
        {
            domObjectStack = DOMObjectStack::New(alloc);
        }
    }
}// namespace JSON