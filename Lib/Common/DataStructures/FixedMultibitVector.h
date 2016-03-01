// Copyright (C) Microsoft. All rights reserved. 

#pragma once

namespace JsUtil
{
    template<class T, class TBlockSize, size_t BitSize, size_t Count>
    class FixedMultibitVector
    {
    private:
        static const size_t CountPerBlock = sizeof(TBlockSize) * 8 / BitSize;
        static const size_t BlockCount = (Count - 1) / CountPerBlock + 1;
        static const TBlockSize LowMask = (static_cast<TBlockSize>(1) << BitSize) - 1;

    private:
        TBlockSize blocks[BlockCount];

    #if DBG
    private:
        size_t _uninitializedData;
    #endif

    public:
        FixedMultibitVector()
        {
            CompileAssert(sizeof(TBlockSize) <= sizeof(size_t));
            CompileAssert(static_cast<TBlockSize>(-1) >= static_cast<TBlockSize>(0)); // must be unsigned
            CompileAssert(sizeof(TBlockSize) * 8 <= 64);
            CompileAssert(BitSize >= 2);
            CompileAssert(BitSize != 8);
            CompileAssert(BitSize != 16);
            CompileAssert(BitSize != 32);
            CompileAssert(BitSize < sizeof(TBlockSize) * 8);
            CompileAssert(Count >= 2);

            // If BitSize is not powers of 2, prefer the largest bit size that results in the same count per block, in hopes
            // that the larger bit size would be a power of 2. For instance, if the block size is 8 bits and the element bit
            // size is specified as 3, this assert would trigger because it would be better to use a bit size of 4, which would
            // store the same number of elements per block while being a power of 2.
            CompileAssert(!(BitSize & BitSize - 1) || CountPerBlock != sizeof(TBlockSize) * 8 / (BitSize + 1));
        }

        void Initialize()
        {
            memset(blocks, 0, sizeof(blocks));
            DebugOnly(_uninitializedData = 0);
        }

    #if DBG
    private:
        bool IsValid() const
        {
            // The instance should have been zero-allocated, or Initialize should have been called after construction
            return _uninitializedData == 0;
        }
    #endif

    private:
        static size_t BlockIndex(const size_t index)
        {
            Assert(index < Count);

            const size_t blockIndex = index / CountPerBlock;
            Assert(blockIndex < BlockCount);
            return blockIndex;
        }

        static size_t Shift(const size_t index)
        {
            Assert(index < Count);
            return index % CountPerBlock * BitSize;
        }

        static TBlockSize Mask(const size_t index)
        {
            return LowMask << Shift(index);
        }

    public:
        T Item(const size_t index) const
        {
            Assert(IsValid());
            return static_cast<T>(blocks[BlockIndex(index)] >> Shift(index) & LowMask);
        }

        void Item(const size_t index, const T &value)
        {
            Assert(IsValid());

            const TBlockSize valueData = static_cast<TBlockSize>(value);
            Assert(valueData == static_cast<TBlockSize>(static_cast<T>(valueData)));
            Assert(!(valueData & ~LowMask));

            TBlockSize &block = blocks[BlockIndex(index)];
            block = block & ~Mask(index) | valueData << Shift(index);
        }
    };
}
