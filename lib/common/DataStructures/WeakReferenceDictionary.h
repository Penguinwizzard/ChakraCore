//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

#pragma once

namespace JsUtil
{
    interface IWeakReferenceDictionary
    {
        virtual void Cleanup() = 0;
    };    

    template <
        class TKey,
        class TValue,
        class SizePolicy = PowerOf2SizePolicy,
        template <typename ValueOrKey> class Comparer = DefaultComparer
    >
    class WeakReferenceDictionary: public BaseDictionary<TKey, RecyclerWeakReference<TValue>*, RecyclerNonLeafAllocator, SizePolicy, Comparer, WeakRefValueDictionaryEntry>,
                                   public IWeakReferenceDictionary
    {
    public:

        WeakReferenceDictionary(Recycler* recycler, int capacity = 0):
          BaseDictionary(recycler, capacity)
        {
            Assert(reinterpret_cast<void*>(this) == reinterpret_cast<void*>((IWeakReferenceDictionary*) this));
        }

        virtual void Cleanup() override
        {
            this->MapAndRemoveIf([](EntryType& entry)
            {
                return (EntryType::NeedsCleanup(entry));
            });
        }

    private:
        using BaseDictionary::Clone;
        using BaseDictionary::Copy;

        PREVENT_COPY(WeakReferenceDictionary);
    };
};
