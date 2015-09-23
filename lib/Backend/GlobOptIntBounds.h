//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

class IntBoundedValueInfo : public ValueInfo
{
private:
    const IntBounds *const bounds;

    // Definitely-int values are inherently not negative zero. This member variable, if true, indicates that this value was
    // produced by an int-specialized instruction that prevented a negative zero result using a negative zero bailout
    // (BailOutOnNegativeZero). Negative zero tracking in the dead-store phase tracks this information to see if some of these
    // negative zero bailout checks can be removed.
    bool wasNegativeZeroPreventedByBailout;

protected:
    IntBoundedValueInfo(const ValueType type, const IntBounds *const bounds, const bool wasNegativeZeroPreventedByBailout)
        :
        ValueInfo(
            type.IsInt()
                ? bounds->ConstantBounds().GetValueType()
                : bounds->ConstantBounds().IsLikelyTaggable() ? type : type.ToLikelyUntaggedInt(),
            ValueStructureKind::IntBounded),
        bounds(bounds),
        wasNegativeZeroPreventedByBailout(wasNegativeZeroPreventedByBailout)
    {
        Assert(type.IsLikelyInt());
        Assert(Type().IsLikelyInt());
        Assert(Type().IsInt() == type.IsInt());
        bounds->Verify();

        // Bounds for definitely int values should have relative bounds, otherwise those values should use one of the other
        // value infos
        Assert(bounds->RequiresIntBoundedValueInfo(Type()));

        Assert(!wasNegativeZeroPreventedByBailout || type.IsInt());
        Assert(!wasNegativeZeroPreventedByBailout || bounds->ConstantLowerBound() <= 0);
        Assert(!wasNegativeZeroPreventedByBailout || bounds->ConstantUpperBound() >= 0);
    }

public:
    static IntBoundedValueInfo *New(
        const ValueType type,
        const IntBounds *const bounds,
        const bool wasNegativeZeroPreventedByBailout,
        JitArenaAllocator *const allocator)
    {
        Assert(allocator);
        return JitAnew(allocator, IntBoundedValueInfo, type, bounds, wasNegativeZeroPreventedByBailout);
    }

    IntBoundedValueInfo *Copy(JitArenaAllocator *const allocator) const
    {
        Assert(allocator);
        return JitAnew(allocator, IntBoundedValueInfo, *this);
    }

public:
    const IntBounds *Bounds() const
    {
        return bounds;
    }

    bool WasNegativeZeroPreventedByBailout() const
    {
        return wasNegativeZeroPreventedByBailout;
    }
};

class LoopCount
{
private:
    bool hasBeenGenerated;

    // Information needed to generate the loop count instructions
    //     loopCountMinusOne = (left - right + offset) / minMagnitudeChange
    StackSym *leftSym, *rightSym;
    int offset, minMagnitudeChange;

    // Information needed to use the computed loop count
    StackSym *loopCountMinusOneSym;
    int loopCountMinusOneConstantValue;

public:
    LoopCount(StackSym *const leftSym, StackSym *const rightSym, const int offset, const int minMagnitudeChange)
        : leftSym(leftSym), rightSym(rightSym), offset(offset), minMagnitudeChange(minMagnitudeChange), hasBeenGenerated(false)
    {
        Assert(leftSym || rightSym);
        Assert(!leftSym || leftSym->GetType() == TyInt32 || leftSym->GetType() == TyUint32);
        Assert(!rightSym || rightSym->GetType() == TyInt32 || rightSym->GetType() == TyUint32);
        Assert(minMagnitudeChange > 0);
    }

    LoopCount(StackSym *const loopCountMinusOneSym) : loopCountMinusOneSym(loopCountMinusOneSym), hasBeenGenerated(true)
    {
        Assert(loopCountMinusOneSym);
    }

    LoopCount(const int loopCountMinusOneConstantValue)
        : loopCountMinusOneSym(nullptr), loopCountMinusOneConstantValue(loopCountMinusOneConstantValue), hasBeenGenerated(true)
    {
        Assert(loopCountMinusOneConstantValue >= 0);
    }

public:
    bool HasBeenGenerated() const
    {
        return hasBeenGenerated;
    }

    StackSym *LeftSym() const
    {
        Assert(!HasBeenGenerated());
        return leftSym;
    }

    StackSym *RightSym() const
    {
        Assert(!HasBeenGenerated());
        return rightSym;
    }

    int Offset() const
    {
        Assert(!HasBeenGenerated());
        return offset;
    }

    int MinMagnitudeChange() const
    {
        Assert(!HasBeenGenerated());
        return minMagnitudeChange;
    }

    StackSym *LoopCountMinusOneSym() const
    {
        Assert(HasBeenGenerated());
        return loopCountMinusOneSym;
    }

    void SetLoopCountMinusOneSym(StackSym *const loopCountMinusOneSym)
    {
        Assert(!HasBeenGenerated());
        Assert(loopCountMinusOneSym);

        hasBeenGenerated = true;
        this->loopCountMinusOneSym = loopCountMinusOneSym;
    }

    int LoopCountMinusOneConstantValue() const
    {
        Assert(!LoopCountMinusOneSym());
        return loopCountMinusOneConstantValue;
    }
};

class GlobOpt::AddSubConstantInfo
{
private:
    StackSym *srcSym;
    Value *srcValue;
    bool srcValueIsLikelyConstant;
    IntConstType offset;

public:
    AddSubConstantInfo() : srcSym(nullptr)
    {
    }

public:
    bool HasInfo() const
    {
        return !!srcSym;
    }

    StackSym *SrcSym() const
    {
        Assert(HasInfo());
        return srcSym;
    }

    Value *SrcValue() const
    {
        Assert(HasInfo());
        return srcValue;
    }

    bool SrcValueIsLikelyConstant() const
    {
        Assert(HasInfo());
        return srcValueIsLikelyConstant;
    }

    IntConstType Offset() const
    {
        Assert(HasInfo());
        return offset;
    }

public:
    void Set(StackSym *const srcSym, Value *const srcValue, const bool srcValueIsLikelyConstant, const IntConstType offset);
};

class GlobOpt::ArrayLowerBoundCheckHoistInfo
{
protected:
    BasicBlock *compatibleBoundCheckBlock;
    Loop *loop;

    // Info populated for a compatible bound check and for hoisting out of loop
    StackSym *indexSym;
    int offset;
    ValueNumber indexValueNumber;

    // Info populated for hoisting out of loop
    Value *indexValue;
    IntConstantBounds indexConstantBounds; // also populated for constant index, when there's a compatible bound check
    bool isLoopCountBasedBound;

    // Info populated for hoisting out of loop using a loop count based bound, when the bound needs to be generated
    LoopCount *loopCount;
    int maxMagnitudeChange;

public:
    ArrayLowerBoundCheckHoistInfo() : compatibleBoundCheckBlock(nullptr), loop(nullptr)
    {
    }

public:
    bool HasAnyInfo() const
    {
        return CompatibleBoundCheckBlock() || Loop();
    }

    BasicBlock *CompatibleBoundCheckBlock() const
    {
        Assert(!(compatibleBoundCheckBlock && loop));
        return compatibleBoundCheckBlock;
    }

    Loop *Loop() const
    {
        Assert(!(compatibleBoundCheckBlock && loop));
        return loop;
    }

    StackSym *IndexSym() const
    {
        Assert(HasAnyInfo());
        return indexSym;
    }

    int Offset() const
    {
        Assert(HasAnyInfo());
        return offset;
    }

    ValueNumber IndexValueNumber() const
    {
        Assert(HasAnyInfo());
        Assert(IndexSym());
        return indexValueNumber;
    }

    Value *IndexValue() const
    {
        Assert(Loop());
        return indexValue;
    }

    const IntConstantBounds &IndexConstantBounds() const
    {
        Assert(Loop() || CompatibleBoundCheckBlock() && !IndexSym());
        return indexConstantBounds;
    }

    bool IsLoopCountBasedBound() const
    {
        Assert(Loop());
        return isLoopCountBasedBound;
    }

    LoopCount *LoopCount() const
    {
        Assert(Loop());
        return loopCount;
    }

    int MaxMagnitudeChange() const
    {
        Assert(LoopCount());
        return maxMagnitudeChange;
    }

public:
    void SetCompatibleBoundCheck(BasicBlock *const compatibleBoundCheckBlock, StackSym *const indexSym, const int offset, const ValueNumber indexValueNumber);
    void SetLoop(::Loop *const loop, const int indexConstantValue, const bool isLoopCountBasedBound = false);
    void SetLoop(::Loop *const loop, StackSym *const indexSym, const int offset, Value *const indexValue, const IntConstantBounds &indexConstantBounds, const bool isLoopCountBasedBound = false);
    void SetLoopCount(::LoopCount *const loopCount, const int maxMagnitudeChange);
};

class GlobOpt::ArrayUpperBoundCheckHoistInfo : protected ArrayLowerBoundCheckHoistInfo
{
private:
    typedef ArrayLowerBoundCheckHoistInfo Base;

private:
    // Info populated for hoisting out of loop
    Value *headSegmentLengthValue;
    IntConstantBounds headSegmentLengthConstantBounds;

public:
    using Base::HasAnyInfo;
    using Base::CompatibleBoundCheckBlock;
    using Base::Loop;
    using Base::IndexSym;
    using Base::Offset;
    using Base::IndexValueNumber;
    using Base::IndexValue;
    using Base::IndexConstantBounds;
    using Base::IsLoopCountBasedBound;
    using Base::LoopCount;
    using Base::MaxMagnitudeChange;

public:
    Value *HeadSegmentLengthValue() const
    {
        Assert(Loop());
        return headSegmentLengthValue;
    }

    const IntConstantBounds &HeadSegmentLengthConstantBounds() const
    {
        Assert(Loop());
        return headSegmentLengthConstantBounds;
    }

public:
    using Base::SetCompatibleBoundCheck;
    using Base::SetLoopCount;

protected:
    using Base::SetLoop;

public:
    void SetCompatibleBoundCheck(BasicBlock *const compatibleBoundCheckBlock, const int indexConstantValue);
    void SetLoop(::Loop *const loop, const int indexConstantValue, Value *const headSegmentLengthValue, const IntConstantBounds &headSegmentLengthConstantBounds, const bool isLoopCountBasedBound = false);
    void SetLoop(::Loop *const loop, StackSym *const indexSym, const int offset, Value *const indexValue, const IntConstantBounds &indexConstantBounds, Value *const headSegmentLengthValue, const IntConstantBounds &headSegmentLengthConstantBounds, const bool isLoopCountBasedBound = false);
};
