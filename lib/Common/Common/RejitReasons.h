//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#if !defined(REJIT_REASON)
    #error REJIT_REASON must be defined before including this file.
#endif

REJIT_REASON(None)
REJIT_REASON(Forced)
REJIT_REASON(RuntimeStatsEnabled)
REJIT_REASON(AggressiveIntTypeSpecDisabled)
REJIT_REASON(AggressiveMulIntTypeSpecDisabled)
REJIT_REASON(DivIntTypeSpecDisabled)
REJIT_REASON(TrackIntOverflowDisabled)
REJIT_REASON(FloatTypeSpecDisabled)
REJIT_REASON(ImplicitCallFlagsChanged)
REJIT_REASON(ImplicitCallFlagsAccessor)
REJIT_REASON(FailedPolymorphicInlineeTypeCheck)
REJIT_REASON(InlineeChanged)
REJIT_REASON(CheckThisDisabled)
REJIT_REASON(LossyIntTypeSpecDisabled)
REJIT_REASON(MemOpDisabled)
REJIT_REASON(FailedTypeCheck)
REJIT_REASON(FailedFixedFieldTypeCheck)
REJIT_REASON(FailedFixedFieldCheck)
REJIT_REASON(FailedEquivalentTypeCheck)
REJIT_REASON(FailedEquivalentFixedFieldTypeCheck)
REJIT_REASON(CtorGuardInvalidated)
REJIT_REASON(ArrayCheckHoistDisabled)
REJIT_REASON(ArrayMissingValueCheckHoistDisabled)
REJIT_REASON(ExpectingNativeArray)
REJIT_REASON(ConvertedNativeArray)
REJIT_REASON(ArrayAccessNeededHelperCall)
REJIT_REASON(JsArraySegmentHoistDisabled)
REJIT_REASON(TypedArrayTypeSpecDisabled)
REJIT_REASON(ExpectingConventionalNativeArrayAccess)
REJIT_REASON(LdLenIntSpecDisabled)
REJIT_REASON(FailedTagCheck)
REJIT_REASON(BoundCheckHoistDisabled)
REJIT_REASON(LoopCountBasedBoundCheckHoistDisabled)
REJIT_REASON(AfterLoopBodyRejit)
REJIT_REASON(DisableSwitchOptExpectingInteger)
REJIT_REASON(DisableSwitchOptExpectingString)
REJIT_REASON(InlineApplyDisabled)
REJIT_REASON(InlineSpreadDisabled)
REJIT_REASON(FloorInliningDisabled)
REJIT_REASON(ModByPowerOf2)
REJIT_REASON(NoProfile)
REJIT_REASON(PowIntIntTypeSpecDisabled)
