//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
class OpCodeUtil
{
public:
    static const uint16 MaxExtendedByteSizedOpcodes = 0x1FF;
    static char16 const * GetOpCodeName(OpCode op);

    static bool IsCallOp(OpCode op);
    static bool IsProfiledCallOp(OpCode op);
    static bool IsProfiledCallOpWithICIndex(OpCode op);
    static bool IsProfiledReturnTypeCallOp(OpCode op);

    // OpCode conversion functions
    static void ConvertOpToNonProfiled(OpCode& op);
    static void ConvertNonCallOpToProfiled(OpCode& op);
    static void ConvertNonCallOpToProfiledWithICIndex(OpCode& op);
    static void ConvertNonCallOpToNonProfiled(OpCode& op);
    static void ConvertNonCallOpToNonProfiledWithICIndex(OpCode& op);

    static OpCode ConvertProfiledCallOpToNonProfiled(OpCode op);
    static OpCode ConvertProfiledReturnTypeCallOpToNonProfiled(OpCode op);
    static OpCode ConvertCallOpToProfiled(OpCode op, bool withICIndex = false);
    static OpCode ConvertCallOpToProfiledReturnType(OpCode op);

    static bool IsValidByteCodeOpcode(OpCode op);
    static bool IsValidOpcode(OpCode op);
    static bool IsPrefixOpcode(OpCode op);
    static constexpr bool IsSmallEncodedOpcode(OpCode op)
    {
        return op <= Js::OpCode::MaxByteSizedOpcodes;
    }
    static constexpr OpCode GetOpCodePrefix(OpCode op, LayoutSize layoutSize)
    {
        CompileAssert((uint16)Js::OpCode::ExtendedOpcodePrefix == 1);
        CompileAssert((uint16)Js::OpCode::WordExtendedOpcodePrefix == 2);
        CompileAssert((uint16)Js::OpCode::MediumLayoutPrefix == 3);
        CompileAssert((uint16)Js::OpCode::ExtendedMediumLayoutPrefix == 4);
        CompileAssert((uint16)Js::OpCode::WordExtendedMediumLayoutPrefix == 5);
        CompileAssert((uint16)Js::OpCode::LargeLayoutPrefix == 6);
        CompileAssert((uint16)Js::OpCode::ExtendedLargeLayoutPrefix == 7);
        CompileAssert((uint16)Js::OpCode::WordExtendedLargeLayoutPrefix == 8);
        CompileAssert(SmallLayout == 0);
        CompileAssert(MediumLayout == 1);
        CompileAssert(LargeLayout == 2);

        // This works because of the way the prefix are ordered
        // layoutSize * 3: SmallLayout => 0, MediumLayout => Js::OpCode::MediumLayoutPrefix, LargeLayout => Js::OpCode::LargeLayoutPrefix
        // The second part is using the second byte to determine if it's a small opcode, extended or word extended
        // 0x00XX => small, 0x01XX => extended, 0xXXXX => word extended
        return (OpCode)((layoutSize * 3) + ((uint16)op >> 8) <= 2 ? (uint16)op >> 8 : 2);
    }
    static constexpr uint EncodedSize(OpCode op, LayoutSize layoutSize)
    {
        // Simple case, only 1 byte
        // Small/Extended OpCode with Medium or Large layout: 1 extra byte for the prefix
        // Word Extended OpCode: Prefix + 2 bytes for the opcode
        return IsSmallEncodedOpcode(op) && layoutSize == SmallLayout ? 1 : (uint16)op <= MaxExtendedByteSizedOpcodes ? 2 : 3;
    }

    static OpLayoutType GetOpCodeLayout(OpCode op);
private:
#if DBG_DUMP || ENABLE_DEBUG_CONFIG_OPTIONS
    static char16 const * const OpCodeNames[(int)Js::OpCode::MaxByteSizedOpcodes + 1];
    static char16 const * const ExtendedOpCodeNames[];
    static char16 const * const BackendOpCodeNames[];
#endif
    static OpLayoutType const OpCodeLayouts[];
    static OpLayoutType const ExtendedOpCodeLayouts[];
    static OpLayoutType const BackendOpCodeLayouts[];
#if DBG
    static OpCode DebugConvertProfiledCallToNonProfiled(OpCode op);
    static OpCode DebugConvertProfiledReturnTypeCallToNonProfiled(OpCode op);
#endif
};
};
