//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace Js
{
    uint ByteBlock::GetLength() const
    {
        return m_contentSize;
    }

    const byte* ByteBlock::GetBuffer() const
    {
        return m_content;
    }

    byte* ByteBlock::GetBuffer()
    {
        return m_content;
    }

    const byte ByteBlock::operator[](uint itemIndex) const
    {
        AssertMsg(itemIndex < m_contentSize, "Ensure valid offset");

        return m_content[itemIndex];
    }

    byte& ByteBlock::operator[] (uint itemIndex)
    {
        AssertMsg(itemIndex < m_contentSize, "Ensure valid offset");

        return m_content[itemIndex];
    }

    ByteBlock *ByteBlock::New(Recycler *alloc, const byte * initialContent, int initialContentSize, ScriptContext * scriptContext)
    {
        // initialContent may be 'null' if no data to copy
        AssertMsg(initialContentSize > 0, "Must have valid data size");
 
        ByteBlock *newBlock = RecyclerNew(alloc,ByteBlock,initialContentSize, scriptContext->GetByteCodeAllocator());
        //
        // Copy any optional data into the block:
        // - If this was not provided, the block's contents will be uninitialized.
        //

        if (initialContent != null)
        {
            {
                AutoCriticalSection cs(scriptContext->GetByteCodeAllocator()->GetCriticalSection());
                scriptContext->EnsureByteCodeAllocationReadWrite(newBlock->GetAllocation());
                js_memcpy_s(newBlock->m_content, newBlock->GetLength(), initialContent, initialContentSize);
                scriptContext->EnsureByteCodeAllocationReadOnly(newBlock->GetAllocation());
            }
        }

        return newBlock;
    }

    ByteBlock *ByteBlock::NewFromArena(ArenaAllocator *alloc,const byte * initialContent,int initialContentSize)  
    {
        // initialContent may be 'null' if no data to copy
        AssertMsg(initialContentSize > 0, "Must have valid data size");
 
        ByteBlock *newBlock = Anew(alloc,ByteBlock,initialContentSize,alloc);
        //
        // Copy any optional data into the block:
        // - If this was not provided, the block's contents will be uninitialized.
        //

        if (initialContent != null)
        {
            js_memcpy_s(newBlock->m_content, newBlock->GetLength(), initialContent, initialContentSize);
        }

        return newBlock;
    }

    ByteBlock * ByteBlock::Clone(Recycler* alloc, ScriptContext * scriptContext)
    {
        return ByteBlock::New(alloc, this->m_content, this->m_contentSize, scriptContext);
    }

    ByteBlock *ByteBlock::New(Recycler *alloc, const byte * initialContent, int initialContentSize, ScriptContext * requestContext, ScriptContext * scriptContext)
    {
        // initialContent may be 'null' if no data to copy
        AssertMsg(initialContentSize > 0, "Must have valid data size");
 
        ByteBlock *newBlock = RecyclerNew(alloc, ByteBlock, initialContentSize, scriptContext->GetByteCodeAllocator());

        //
        // Copy any optional data into the block:
        // - If this was not provided, the block's contents will be uninitialized.
        //

        if (initialContent != null)
        {
            //
            // Treat initialcontent as array of vars
            // Clone vars to the requestContext
            //

            Var *src = (Var*)initialContent;
            Var *dst = (Var*)newBlock->m_content;
            size_t count = initialContentSize / sizeof(Var);

            AutoCriticalSection cs(scriptContext->GetByteCodeAllocator()->GetCriticalSection());
            {
                scriptContext->EnsureByteCodeAllocationReadWrite(newBlock->GetAllocation());

                for (size_t i = 0; i < count; i++)
                {
                    if (TaggedInt::Is(src[i]))
                    {
                        dst[i] = src[i];
                    }
                    else
                    {
                        //
                        // Currently only numbers are put into AuxiliaryContext data
                        //
                        Assert(JavascriptNumber::Is(src[i]));
                        dst[i] = JavascriptNumber::CloneToScriptContext(src[i], requestContext);
                        requestContext->BindReference(dst[i]);
                    }
                }

                scriptContext->EnsureByteCodeAllocationReadOnly(newBlock->GetAllocation());
            }
        }

        return newBlock;
    }

    //
    // Create a copy of buffer
    //  Each Var is cloned on the requestContext
    //
    ByteBlock * ByteBlock::Clone(Recycler* alloc, ScriptContext * requestContext, ScriptContext * scriptContext)
    {
        return ByteBlock::New(alloc, this->m_content, this->m_contentSize, requestContext, scriptContext);
    }
}
