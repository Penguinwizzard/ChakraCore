// Copyright (C) Microsoft. All rights reserved.

#if !defined(SLOT_TYPE) || !defined(SLOT_TYPE_VALUE) || !defined(ATTRIBUTE_VALUE)
    #error SLOT_TYPE and SLOT_TYPE_VALUE and ATTRIBUTE_VALUE must be defined.
#endif

SLOT_TYPE(Var)
#if OBJECT_WIDE_SLOTS
    SLOT_TYPE(ConvertedVar)
#else
    SLOT_TYPE_VALUE(ConvertedVar, Var)
#endif
SLOT_TYPE(Float)
SLOT_TYPE(Int)

#if defined(SLOT_TYPE_FAKE)
    SLOT_TYPE_FAKE(Count)
#endif

ATTRIBUTE_VALUE(Writable, static_cast<ObjectSlotType::TSize>(1 << 3))
ATTRIBUTE_VALUE(Enumerable, static_cast<ObjectSlotType::TSize>(1 << 4))
ATTRIBUTE_VALUE(Configurable, static_cast<ObjectSlotType::TSize>(1 << 5))
