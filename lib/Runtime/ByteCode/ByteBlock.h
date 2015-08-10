//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class ByteBlock 
    {
        DECLARE_OBJECT(ByteBlock)

    private:
        uint m_contentSize;      // Length of block, in bytes
        CustomHeap::Allocation * allocation;
        byte* m_content;      // Block's content

        static ByteBlock* New(Recycler* alloc, const byte * initialContent, int initialContentSize, ScriptContext * requestContext, ScriptContext * scriptContext);

    public:
        ByteBlock(uint size, byte * content)
            : m_contentSize(size), m_content(content), allocation(nullptr)
        { }
        ByteBlock(uint size, CustomHeap::Heap *alloc) : m_contentSize(size)
        {
            // The New function below will copy over a buffer into this so 
            // we don't need to zero it out
            allocation = alloc->Alloc(size, 0, 0, false, false, nullptr);
#if DBG
            allocation->isAllocationUsed = true;
#endif
            m_content = (byte*)allocation->address;
        }

        ByteBlock(uint size, ArenaAllocator* alloc) : m_contentSize(size), allocation(nullptr)
        {
            m_content = AnewArray(alloc, byte, size);
        }
        
        static DWORD GetBufferOffset() { return offsetof(ByteBlock, m_content); }

        static ByteBlock* New(Recycler* alloc, const byte * initialContent, int initialContentSize, ScriptContext * scriptContext);

        // This is needed just for the debugger since it allocates
        // the byte block on a seperate thread, which the recycler doesn't like
        // To remove when the recycler supports multi-threaded allocation
        static ByteBlock* NewFromArena(ArenaAllocator* alloc,const byte * initialContent,int initialContentSize);

        inline uint GetLength() const;
        inline byte* GetBuffer();
        inline const byte* GetBuffer() const;
        inline const byte operator[](uint itemIndex) const;
        inline byte& operator[] (uint itemIndex);
        inline CustomHeap::Allocation * GetAllocation() { return allocation; }

        ByteBlock * Clone(Recycler* alloc, ScriptContext * scriptContext);

        //
        // Create a copy of buffer
        //  Each Var is cloned on the requestContext
        //
        ByteBlock * Clone(Recycler* alloc, ScriptContext * requestContext, ScriptContext * scriptContext);
    };
} // namespace Js
