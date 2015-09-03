//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "RuntimeLibraryPch.h"

namespace Js
{
    const uint32 SparseArraySegmentBase::MaxLength = static_cast<uint32>(INT32_MAX);

    SparseArraySegmentBase::SparseArraySegmentBase(uint32 left, uint32 length, uint32 size):left(left), length(length), size(size), next(null)
    {
    }

    // "Reduce" size if it exceeds next.left boundary, after operations that shift the following segments.
    void SparseArraySegmentBase::EnsureSizeInBound()
    {
        EnsureSizeInBound(left, length, size, next);
    }

    // Reduce size if it exceeds next.left boundary or MaxArrayLength
    void SparseArraySegmentBase::EnsureSizeInBound(uint32 left, uint32 length, uint32& size, SparseArraySegmentBase* next)
    {
        uint32 nextLeft = next ? next->left : JavascriptArray::MaxArrayLength;
        Assert(nextLeft > left);

        if(size != 0)
        {
            // Avoid writing to 'size' for an empty segment. The empty segment is a constant structure and writing to it (even
            // if it's not being changed) may cause an AV.
            size = min(size, nextLeft - left);
        }
        Assert(length <= size);
    }

    // Test if an element value is null/undefined.
    inline static bool IsMissingOrUndefined(Var value, RecyclableObject *undefined, uint32& countUndefined)
    {
        if (value == SparseArraySegment<Var>::GetMissingItem())
        {
            return true;
        }
        if (JavascriptOperators::IsUndefinedObject(value, undefined))
        {
            ++countUndefined;
            return true;
        }
        return false;
    }

    bool SparseArraySegmentBase::IsLeafSegment(SparseArraySegmentBase *seg, Recycler *recycler)
    {
        if (!DoNativeArrayLeafSegment())
        {
            return false;
        }
        
        RecyclerHeapObjectInfo heapObject;
        if (recycler->FindHeapObject(
                seg, 
                (Memory::FindHeapObjectFlags)(FindHeapObjectFlags_VerifyFreeBitForAttribute | FindHeapObjectFlags_AllowInterior), 
                heapObject))
        {
            return heapObject.IsLeaf();
        }

        return false;
    }

    // Remove null/undefined from this segment. May reorder elements and compact this segment in preparing for sort.
    uint32 SparseArraySegmentBase::RemoveUndefined(ScriptContext* scriptContext)
    {
        SparseArraySegment<Var> *_this = (SparseArraySegment<Var>*)this;
        // Shortcut length==0, otherwise the code below will AV when left==length==0. (WOOB 1114975)
        if (length == 0)
        {
            return 0;
        }

        //remove undefine values
        RecyclableObject *undefined = scriptContext->GetLibrary()->GetUndefined();

        uint32 i = 0;
        uint32 countUndefined = 0;
        uint32 placeIndex = 0;
        uint32 newLength = length;

        while (i < newLength && !IsMissingOrUndefined(_this->elements[i], undefined, countUndefined))
        {
            // for as long as elements are not missing or undefined, we don't want
            // to perform the copy below, since i and placeIndex will be identical
            i++;
        }

        // the above loop will only ever generate countUndefined = 1 for the first
        // item it finds. We reset countUndefined to 0 so we don't double count that element
        countUndefined = 0;

        for (; i < newLength; i++)
        {
            if (!IsMissingOrUndefined(_this->elements[i], undefined, countUndefined))
            {
                _this->elements[placeIndex] = _this->elements[i];
                placeIndex++;
                newLength--;
            }
        }

        if (newLength != length) // Truncate if j has changed
        {
            Assert(newLength < length);
            Assert(countUndefined <= length - newLength);
            _this->Truncate(left + newLength); // Truncate to new length (also clears moved elements)
        }

        Assert(length <= size);
        return countUndefined;
    }
}
