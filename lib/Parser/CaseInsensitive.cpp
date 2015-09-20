// Copyright (C) Microsoft. All rights reserved. 

//
// Map Unicode characters to their equivalence classes induced by the modified ToUpper map.
// Ie c1 and c2 are in the same class if ToUpper(c1) == ToUpper(c2).
//
// The ToUpper map takes any character to its Unicode upper case eqivalent, with the modification that
// a non-7-bit-ASCII character cannot be mapped to 7-bit-ASCII characters.
//

#include "ParserPch.h"

namespace UnifiedRegex
{
    namespace CaseInsensitive
    {
        struct Transform
        {
            // This skipCount is to help define the range. Ex, given range [0 - 20]
            // If skip count is 1, then all items between 0 and 20 are in the range.
            // If skip count is 2, then every even item is in the range, so 0, 2, 4, 6, 8, etc.
            byte skipCountOfRange;

            // Range of chars this transform applies to
            Chars<codepoint_t>::UChar lo;
            Chars<codepoint_t>::UChar hi;

            MappingSource source;

            // Offsets to add to original character to get each equivalent character
            int delta0;
            int delta1;
            int delta2;
            int delta3;

            template <typename Char>
            inline void Apply(uint c, Char outEquiv[EquivClassSize]) const
            {
                Assert(c >= lo && c <= hi);

                outEquiv[0] = Chars<Char>::UTC((lo + 1) % skipCountOfRange == c % skipCountOfRange ? (int)c + delta0 : c);

                CompileAssert(CaseInsensitive::EquivClassSize == 4);
                if (lo  % skipCountOfRange == c % skipCountOfRange)
                {
                    outEquiv[1] = Chars<Char>::ITC((int)c + delta1);
                    outEquiv[2] = Chars<Char>::ITC((int)c + delta2);
                    outEquiv[3] = Chars<Char>::ITC((int)c + delta3);
                }
                else 
                {
                    outEquiv[1] = outEquiv[2] = outEquiv[3] = Chars<Char>::UTC(c);
                }
            }
        };

/*
We first construct a total map from character codes to equivalence lists such that:
 - if ToUpper(c1) == ToUpper(c2) then c1 has c2 in its equivalence list
 - if c1 and c2 appear in the same equivalence list then c1 and c2 have equal equivalence lists

We then compress the above map such that:
 - characters with singleton equivalence classes are elided
 - consecutive characters with consecutive equivalence lists are represented as a range and delta
 - the result is in strictly increasing range order

Using gawk the above is:
  gawk -f equiv.gawk http://www.unicode.org/Public/UNIDATA/UnicodeData.txt | gawk -f table.gawk

Where equiv.gawk is:
----------------------------------------------------------------------
BEGIN {
  FS = ";";
  previncode = -1;
}
length($1) == 4 {
  incode = strtonum("0x" $1);
  for (i = previncode + 1; i < incode; i++)
    map[i] = i;
  if ($3 == "Ll" && $15 != "")
  {
    map[incode] = strtonum("0x" $15);
    # non-7-bit-ASCII cannot map to 7-bit-ASCII
    if (incode > 127 && map[incode] <= 127)
      map[incode] = incode;
  }
  else
    map[incode] = incode;
  previncode = incode;
}
END {
  for (i = previncode + 1; i <= 0xffff; i++)
    map[i] = i;

  for (i = 0x0000; i <= 0xffff; i++)
    ninv[i] = 0;

  for (i = 0x0000; i <= 0xffff; i++)
  {
    if (map[i] != i)
      ninv[map[i]]++;
  }

  maxninv = 0;
  for (i = 0x0000; i <= 0xffff; i++)
  {
    if (ninv[i] > maxninv)
      maxninv = ninv[i];
  }
  if (maxninv > 2)
    print "ERROR";

  for (i = 0x0000; i <= 0xffff; i++)
    inv[i] = "";

  for (i = 0x0000; i <= 0xffff; i++)
  {
    if (map[i] != i)
      inv[map[i]] = sprintf("%s;0x%04x", inv[map[i]], i);
  }

  for (i = 0x0000; i <= 0xffff; i++)
  {
    if (map[i] != i)
    {
      equiv[i] = sprintf("0x%04x%s", map[i], inv[map[i]]);
      nequiv[i] = 1 + ninv[map[i]];
    }
    else if (inv[i] != "")
    {
      equiv[i] = sprintf("0x%04x%s", i, inv[i]);
      nequiv[i] = 1 + ninv[i];
    }
    else
    {
      equiv[i] = sprintf("0x%04x", i);
      nequiv[i] = 1;
    }
  }

  nentries = 0
  for (i = 0x0000; i <= 0xffff; i++)
  {
    if (nequiv[i] > 1)
    {
      printf("0x%04x;%s\n", i, equiv[i]);
      nentries++;
    }
  }
  #printf("nentries = %d\n", nentries);
}
----------------------------------------------------------------------

And table.gawk is:
----------------------------------------------------------------------
BEGIN {
  FS = ";";
  lastCode = -1;
  currStart = -1;
  for (i = 0; i < 3; i++)
    currDeltas[i] = "";
}
{
  if (NF > 4)
    print "ERROR"

  incode = strtonum($1);
  for (i = 0; i < NF - 1; i++)
    equivs[i] = strtonum($(i+2));
  for (i = NF - 1; i < 3; i++)
    equivs[i] = equivs[i - 1];

  #printf("0x%04x, 0x%04x, 0x%04x, 0x%04x\n", incode, equivs[0], equivs[1], equivs[2]);

  for (i = 0; i < 3; i++)
    deltas[i] = equivs[i] - incode;

  if (currStart < 0)
  {
    # start a new range
    currStart = incode;
    for (i = 0; i < 3; i++)
      currDeltas[i] = deltas[i]
  }
  else if (incode == lastCode + 1 && deltas[0] == currDeltas[0] && deltas[1] == currDeltas[1] && deltas[2] == currDeltas[2])
  {
    # keep accumulating range
  }
  else
  {
    # dump current range and start a new one
    printf("            0x%04x, 0x%04x, %d, %d, %d,\n", currStart, lastCode, currDeltas[0], currDeltas[1], currDeltas[2]);
    currStart = incode;
    for (i = 0; i < 3; i++)
      currDeltas[i] = deltas[i]
  }

  lastCode = incode;
}
END {
  printf("            0x%04x, 0x%04x, %d, %d, %d,\n", currStart, lastCode, currDeltas[0], currDeltas[1], currDeltas[2]);
}
----------------------------------------------------------------------
*/

        // For case-folding entries, version 8.0.0 of CaseFolding.txt located at [1] was used.
        // [1] ftp://ftp.unicode.org/Public/UNIDATA/CaseFolding.txt
        static const Transform transforms[] =
        {
            1, 0x0041, 0x004a, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x004b, 0x004b, MappingSource::CaseFolding, 0, 32, 8415, 8415,
            1, 0x004b, 0x0052, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x0053, 0x0053, MappingSource::CaseFolding, 0, 32, 300, 300,
            1, 0x0053, 0x005a, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x0061, 0x006a, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x006b, 0x006b, MappingSource::CaseFolding, -32, 0, 8383, 8383,
            1, 0x006b, 0x0072, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x0073, 0x0073, MappingSource::CaseFolding, -32, 0, 268, 268,
            1, 0x0073, 0x007a, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x00b5, 0x00b5, MappingSource::UnicodeData, 743, 0, 775, 775,
            1, 0x00c0, 0x00c4, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x00c5, 0x00c5, MappingSource::CaseFolding, 0, 32, 8294, 8294,
            1, 0x00c5, 0x00d6, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x00d8, 0x00de, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x00DF, 0x00DF, MappingSource::UnicodeData, 0, 7615, 7615, 7615,
            1, 0x00e0, 0x00e4, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x00e5, 0x00e5, MappingSource::CaseFolding, -32, 0, 8262, 8262,
            1, 0x00e5, 0x00f6, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x00f8, 0x00fe, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x00ff, 0x00ff, MappingSource::UnicodeData, 121, 0, 0, 0,
            2, 0x0100, 0x012f, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0x0132, 0x0137, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0x0139, 0x0148, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0x014a, 0x0177, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x0178, 0x0178, MappingSource::UnicodeData, 0, -121, -121, -121,
            2, 0x0179, 0x017e, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x017f, 0x017f, MappingSource::CaseFolding, -300, -268, 0, 0,
            1, 0x0180, 0x0180, MappingSource::UnicodeData, 195, 0, 0, 0,
            1, 0x0181, 0x0181, MappingSource::UnicodeData, 0, 210, 210, 210,
            2, 0x0182, 0x0185, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x0186, 0x0186, MappingSource::UnicodeData, 0, 206, 206, 206,
            1, 0x0187, 0x0187, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x0188, 0x0188, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x0189, 0x018a, MappingSource::UnicodeData, 0, 205, 205, 205,
            1, 0x018b, 0x018b, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x018c, 0x018c, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x018e, 0x018e, MappingSource::UnicodeData, 0, 79, 79, 79,
            1, 0x018f, 0x018f, MappingSource::UnicodeData, 0, 202, 202, 202,
            1, 0x0190, 0x0190, MappingSource::UnicodeData, 0, 203, 203, 203,
            1, 0x0191, 0x0191, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x0192, 0x0192, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x0193, 0x0193, MappingSource::UnicodeData, 0, 205, 205, 205,
            1, 0x0194, 0x0194, MappingSource::UnicodeData, 0, 207, 207, 207,
            1, 0x0195, 0x0195, MappingSource::UnicodeData, 97, 0, 0, 0,
            1, 0x0196, 0x0196, MappingSource::UnicodeData, 0, 211, 211, 211,
            1, 0x0197, 0x0197, MappingSource::UnicodeData, 0, 209, 209, 209,
            1, 0x0198, 0x0198, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x0199, 0x0199, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x019a, 0x019a, MappingSource::UnicodeData, 163, 0, 0, 0,
            1, 0x019c, 0x019c, MappingSource::UnicodeData, 0, 211, 211, 211,
            1, 0x019d, 0x019d, MappingSource::UnicodeData, 0, 213, 213, 213,
            1, 0x019e, 0x019e, MappingSource::UnicodeData, 130, 0, 0, 0,
            1, 0x019f, 0x019f, MappingSource::UnicodeData, 0, 214, 214, 214,
            2, 0x01a0, 0x01a5, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x01a6, 0x01a6, MappingSource::UnicodeData, 0, 218, 218, 218,
            1, 0x01a7, 0x01a7, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x01a8, 0x01a8, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x01a9, 0x01a9, MappingSource::UnicodeData, 0, 218, 218, 218,
            1, 0x01ac, 0x01ac, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x01ad, 0x01ad, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x01ae, 0x01ae, MappingSource::UnicodeData, 0, 218, 218, 218,
            1, 0x01af, 0x01af, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x01b0, 0x01b0, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x01b1, 0x01b2, MappingSource::UnicodeData, 0, 217, 217, 217,
            1, 0x01b3, 0x01b3, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x01b4, 0x01b4, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x01b5, 0x01b5, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x01b6, 0x01b6, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x01b7, 0x01b7, MappingSource::UnicodeData, 0, 219, 219, 219,
            2, 0x01b8, 0x01bd, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x01bf, 0x01bf, MappingSource::UnicodeData, 56, 0, 0, 0,
            1, 0x01c4, 0x01c4, MappingSource::UnicodeData, 2, 0, 0, 0,
            1, 0x01c5, 0x01c5, MappingSource::UnicodeData, 1, 0, 0, 0,
            1, 0x01c6, 0x01c6, MappingSource::UnicodeData, 0, -2, -1, -1,
            1, 0x01c7, 0x01c7, MappingSource::UnicodeData, 2, 0, 0, 0,
            1, 0x01c8, 0x01c8, MappingSource::UnicodeData, 1, 0, 0, 0,
            1, 0x01c9, 0x01c9, MappingSource::UnicodeData, 0, -2, -1, -1,
            1, 0x01CA, 0x01CA, MappingSource::UnicodeData, 2, 0, 0, 0,
            1, 0x01CB, 0x01CB, MappingSource::UnicodeData, 1, 0, 0, 0,
            1, 0x01CC, 0x01CC, MappingSource::UnicodeData, 0, -2, -1, -1,
            2, 0x01cd, 0x01dc, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x01dd, 0x01dd, MappingSource::UnicodeData, -79, 0, 0, 0,
            2, 0x01de, 0x01f5, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x01F1, 0x01F1, MappingSource::UnicodeData, 2, 0, 0, 0,
            1, 0x01F2, 0x01F2, MappingSource::UnicodeData, 1, 0, 0, 0,
            1, 0x01F3, 0x01F3, MappingSource::UnicodeData, 0, -2, -1, -1,
            2, 0x01f4, 0x01f5, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x01f6, 0x01f6, MappingSource::UnicodeData, 0, -97, -97, -97,
            1, 0x01f7, 0x01f7, MappingSource::UnicodeData, 0, -56, -56, -56,
            2, 0x01f8, 0x021f, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x0220, 0x0220, MappingSource::UnicodeData, 0, -130, -130, -130,
            2, 0x0222, 0x0233, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x023a, 0x023a, MappingSource::UnicodeData, 0, 10795, 10795, 10795,
            1, 0x023b, 0x023b, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x023c, 0x023c, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x023d, 0x023d, MappingSource::UnicodeData, 0, -163, -163, -163,
            1, 0x023e, 0x023e, MappingSource::UnicodeData, 0, 10792, 10792, 10792,
            1, 0x023f, 0x0240, MappingSource::UnicodeData, 10815, 0, 0, 0,
            1, 0x0241, 0x0241, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x0242, 0x0242, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x0243, 0x0243, MappingSource::UnicodeData, 0, -195, -195, -195,
            1, 0x0244, 0x0244, MappingSource::UnicodeData, 0, 69, 69, 69,
            1, 0x0245, 0x0245, MappingSource::UnicodeData, 0, 71, 71, 71,
            2, 0x0246, 0x024f, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x0250, 0x0250, MappingSource::UnicodeData, 10783, 0, 0, 0,
            1, 0x0251, 0x0251, MappingSource::UnicodeData, 10780, 0, 0, 0,
            1, 0x0252, 0x0252, MappingSource::UnicodeData, 10782, 0, 0, 0,
            1, 0x0253, 0x0253, MappingSource::UnicodeData, -210, 0, 0, 0,
            1, 0x0254, 0x0254, MappingSource::UnicodeData, -206, 0, 0, 0,
            1, 0x0256, 0x0257, MappingSource::UnicodeData, -205, 0, 0, 0,
            1, 0x0259, 0x0259, MappingSource::UnicodeData, -202, 0, 0, 0,
            1, 0x025b, 0x025b, MappingSource::UnicodeData, -203, 0, 0, 0,
            1, 0x0260, 0x0260, MappingSource::UnicodeData, -205, 0, 0, 0,
            1, 0x0263, 0x0263, MappingSource::UnicodeData, -207, 0, 0, 0,
            1, 0x0265, 0x0265, MappingSource::UnicodeData, 42280, 0, 0, 0,
            1, 0x0266, 0x0266, MappingSource::UnicodeData, 42308, 0, 0, 0,
            1, 0x0268, 0x0268, MappingSource::UnicodeData, -209, 0, 0, 0,
            1, 0x0269, 0x0269, MappingSource::UnicodeData, -211, 0, 0, 0,
            1, 0x026b, 0x026b, MappingSource::UnicodeData, 10743, 0, 0, 0,
            1, 0x026f, 0x026f, MappingSource::UnicodeData, -211, 0, 0, 0,
            1, 0x0271, 0x0271, MappingSource::UnicodeData, 10749, 0, 0, 0,
            1, 0x0272, 0x0272, MappingSource::UnicodeData, -213, 0, 0, 0,
            1, 0x0275, 0x0275, MappingSource::UnicodeData, -214, 0, 0, 0,
            1, 0x027d, 0x027d, MappingSource::UnicodeData, 10727, 0, 0, 0,
            1, 0x0280, 0x0280, MappingSource::UnicodeData, -218, 0, 0, 0,
            1, 0x0283, 0x0283, MappingSource::UnicodeData, -218, 0, 0, 0,
            1, 0x0288, 0x0288, MappingSource::UnicodeData, -218, 0, 0, 0,
            1, 0x0289, 0x0289, MappingSource::UnicodeData, -69, 0, 0, 0,
            1, 0x028a, 0x028b, MappingSource::UnicodeData, -217, 0, 0, 0,
            1, 0x028c, 0x028c, MappingSource::UnicodeData, -71, 0, 0, 0,
            1, 0x0292, 0x0292, MappingSource::UnicodeData, -219, 0, 0, 0,
            1, 0x0345, 0x0345, MappingSource::CaseFolding, 0, 84, 116, 7289,
            2, 0x0370, 0x0373, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x0376, 0x0376, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x0377, 0x0377, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x037b, 0x037d, MappingSource::UnicodeData, 130, 0, 0, 0,
            1, 0x0386, 0x0386, MappingSource::UnicodeData, 0, 38, 38, 38,
            1, 0x0388, 0x038a, MappingSource::UnicodeData, 0, 37, 37, 37,
            1, 0x038c, 0x038c, MappingSource::UnicodeData, 0, 64, 64, 64,
            1, 0x038e, 0x038f, MappingSource::UnicodeData, 0, 63, 63, 63,
            1, 0x0391, 0x0391, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x0392, 0x0392, MappingSource::UnicodeData, 0, 32, 62, 62,
            1, 0x0393, 0x0394, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x0395, 0x0395, MappingSource::UnicodeData, 0, 32, 96, 96,
            1, 0x0396, 0x0397, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x0398, 0x0398, MappingSource::CaseFolding, 0, 32, 57, 92,
            1, 0x0398, 0x0398, MappingSource::UnicodeData, 0, 32, 57, 57,
            1, 0x0399, 0x0399, MappingSource::CaseFolding, -84, 0, 32, 7205,
            1, 0x0399, 0x0399, MappingSource::UnicodeData, 0, 32, 7205, 7205,
            1, 0x039a, 0x039a, MappingSource::UnicodeData, 0, 32, 86, 86,
            1, 0x039b, 0x039b, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x039c, 0x039c, MappingSource::UnicodeData, 0, -743, 32, 32,
            1, 0x039d, 0x039f, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x03a0, 0x03a0, MappingSource::UnicodeData, 0, 32, 54, 54,
            1, 0x03a1, 0x03a1, MappingSource::UnicodeData, 0, 32, 80, 80,
            1, 0x03a3, 0x03a3, MappingSource::UnicodeData, 0, 31, 32, 32,
            1, 0x03a4, 0x03a5, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x03a6, 0x03a6, MappingSource::UnicodeData, 0, 32, 47, 47,
            1, 0x03a7, 0x03a8, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x03a9, 0x03a9, MappingSource::CaseFolding, 0, 32, 7549, 7549,
            1, 0x03a9, 0x03ab, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x03ac, 0x03ac, MappingSource::UnicodeData, -38, 0, 0, 0,
            1, 0x03ad, 0x03af, MappingSource::UnicodeData, -37, 0, 0, 0,
            1, 0x03b1, 0x03b1, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03b2, 0x03b2, MappingSource::UnicodeData, -32, 0, 30, 30,
            1, 0x03b3, 0x03b4, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03b5, 0x03b5, MappingSource::UnicodeData, -32, 0, 64, 64,
            1, 0x03b6, 0x03b7, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03b8, 0x03b8, MappingSource::CaseFolding, -32, 0, 25, 60,
            1, 0x03b8, 0x03b8, MappingSource::UnicodeData, -32, 0, 25, 25,
            1, 0x03b9, 0x03b9, MappingSource::CaseFolding, -116, -32, 0, 7173,
            1, 0x03b9, 0x03b9, MappingSource::UnicodeData, -32, 0, 7173, 7173,
            1, 0x03ba, 0x03ba, MappingSource::UnicodeData, -32, 0, 54, 54,
            1, 0x03bb, 0x03bb, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03bc, 0x03bc, MappingSource::UnicodeData, -32, -775, 0, 0,
            1, 0x03bd, 0x03bf, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03c0, 0x03c0, MappingSource::UnicodeData, -32, 0, 22, 22,
            1, 0x03c1, 0x03c1, MappingSource::UnicodeData, -32, 0, 48, 48,
            1, 0x03c2, 0x03c2, MappingSource::UnicodeData, -31, 0, 1, 1,
            1, 0x03c3, 0x03c3, MappingSource::UnicodeData, -32, -1, 0, 0,
            1, 0x03c4, 0x03c5, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03c6, 0x03c6, MappingSource::UnicodeData, -32, 0, 15, 15,
            1, 0x03c7, 0x03c8, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03c9, 0x03c9, MappingSource::CaseFolding, -32, 0, 7517, 7517,
            1, 0x03c9, 0x03cb, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x03cc, 0x03cc, MappingSource::UnicodeData, -64, 0, 0, 0,
            1, 0x03cd, 0x03ce, MappingSource::UnicodeData, -63, 0, 0, 0,
            1, 0x03cf, 0x03cf, MappingSource::UnicodeData, 0, 8, 8, 8,
            1, 0x03d0, 0x03d0, MappingSource::UnicodeData, -62, -30, 0, 0,
            1, 0x03d1, 0x03d1, MappingSource::CaseFolding, -57, -25, 0, 35,
            1, 0x03d1, 0x03d1, MappingSource::UnicodeData, -57, -25, 0, 0,
            1, 0x03d5, 0x03d5, MappingSource::UnicodeData, -47, -15, 0, 0,
            1, 0x03d6, 0x03d6, MappingSource::UnicodeData, -54, -22, 0, 0,
            1, 0x03d7, 0x03d7, MappingSource::UnicodeData, -8, 0, 0, 0,
            2, 0x03d8, 0x03ef, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x03f0, 0x03f0, MappingSource::UnicodeData, -86, -54, 0, 0,
            1, 0x03f1, 0x03f1, MappingSource::UnicodeData, -80, -48, 0, 0,
            1, 0x03f2, 0x03f2, MappingSource::UnicodeData, 7, 0, 0, 0,
            1, 0x03f4, 0x03f4, MappingSource::CaseFolding, -92, -60, -35, 0,
            1, 0x03f5, 0x03f5, MappingSource::UnicodeData, -96, -64, 0, 0,
            1, 0x03f7, 0x03f7, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x03f8, 0x03f8, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x03f9, 0x03f9, MappingSource::UnicodeData, 0, -7, -7, -7,
            1, 0x03fa, 0x03fa, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x03fb, 0x03fb, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x03fd, 0x03ff, MappingSource::UnicodeData, 0, -130, -130, -130,
            1, 0x0400, 0x040f, MappingSource::UnicodeData, 0, 80, 80, 80,
            1, 0x0410, 0x042f, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0x0430, 0x044f, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x0450, 0x045f, MappingSource::UnicodeData, -80, 0, 0, 0,
            2, 0x0460, 0x0481, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0x048a, 0x04bf, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x04c0, 0x04c0, MappingSource::UnicodeData, 0, 15, 15, 15,
            2, 0x04c1, 0x04ce, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x04cf, 0x04cf, MappingSource::UnicodeData, -15, 0, 0, 0,
            2, 0x04d0, 0x0527, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x0531, 0x0556, MappingSource::UnicodeData, 0, 48, 48, 48,
            1, 0x0561, 0x0586, MappingSource::UnicodeData, -48, 0, 0, 0,
            1, 0x10a0, 0x10c5, MappingSource::UnicodeData, 0, 7264, 7264, 7264,
            1, 0x10C7, 0x10C7, MappingSource::UnicodeData, 0, 7264, 7264, 7264,
            1, 0x10CD, 0x10CD, MappingSource::UnicodeData, 0, 7264, 7264, 7264,
            1, 0x1d79, 0x1d79, MappingSource::UnicodeData, 35332, 0, 0, 0,
            1, 0x1d7d, 0x1d7d, MappingSource::UnicodeData, 3814, 0, 0, 0,
            2, 0x1e00, 0x1e5f, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x1e60, 0x1e60, MappingSource::UnicodeData, 0, 1, 59, 59,
            1, 0x1e61, 0x1e61, MappingSource::UnicodeData, -1, 0, 58, 58,
            2, 0x1e62, 0x1e95, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x1e9b, 0x1e9b, MappingSource::UnicodeData, -59, -58, 0, 0,
            1, 0x1E9E, 0x1E9E, MappingSource::UnicodeData, -7615, 0, 0, 0,
            2, 0x1ea0, 0x1eff, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x1f00, 0x1f07, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f08, 0x1f0f, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f10, 0x1f15, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f18, 0x1f1d, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f20, 0x1f27, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f28, 0x1f2f, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f30, 0x1f37, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f38, 0x1f3f, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f40, 0x1f45, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f48, 0x1f4d, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f51, 0x1f51, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f53, 0x1f53, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f55, 0x1f55, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f57, 0x1f57, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f59, 0x1f59, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f5b, 0x1f5b, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f5d, 0x1f5d, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f5f, 0x1f5f, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f60, 0x1f67, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f68, 0x1f6f, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f70, 0x1f71, MappingSource::UnicodeData, 74, 0, 0, 0,
            1, 0x1f72, 0x1f75, MappingSource::UnicodeData, 86, 0, 0, 0,
            1, 0x1f76, 0x1f77, MappingSource::UnicodeData, 100, 0, 0, 0,
            1, 0x1f78, 0x1f79, MappingSource::UnicodeData, 128, 0, 0, 0,
            1, 0x1f7a, 0x1f7b, MappingSource::UnicodeData, 112, 0, 0, 0,
            1, 0x1f7c, 0x1f7d, MappingSource::UnicodeData, 126, 0, 0, 0,
            1, 0x1f80, 0x1f87, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f88, 0x1f8f, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1f90, 0x1f97, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1f98, 0x1f9f, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1fa0, 0x1fa7, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1fa8, 0x1faf, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1fb0, 0x1fb1, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1fb3, 0x1fb3, MappingSource::UnicodeData, 9, 0, 0, 0,
            1, 0x1fb8, 0x1fb9, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1fba, 0x1fbb, MappingSource::UnicodeData, 0, -74, -74, -74,
            1, 0x1fbc, 0x1fbc, MappingSource::UnicodeData, 0, -9, -9, -9,
            1, 0x1fbe, 0x1fbe, MappingSource::CaseFolding, -7289, -7205, -7173, 0,
            1, 0x1fbe, 0x1fbe, MappingSource::UnicodeData, -7205, -7173, 0, 0,
            1, 0x1fc3, 0x1fc3, MappingSource::UnicodeData, 9, 0, 0, 0,
            1, 0x1fc8, 0x1fcb, MappingSource::UnicodeData, 0, -86, -86, -86,
            1, 0x1fcc, 0x1fcc, MappingSource::UnicodeData, 0, -9, -9, -9,
            1, 0x1fd0, 0x1fd1, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1fd8, 0x1fd9, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1fda, 0x1fdb, MappingSource::UnicodeData, 0, -100, -100, -100,
            1, 0x1fe0, 0x1fe1, MappingSource::UnicodeData, 8, 0, 0, 0,
            1, 0x1fe5, 0x1fe5, MappingSource::UnicodeData, 7, 0, 0, 0,
            1, 0x1fe8, 0x1fe9, MappingSource::UnicodeData, 0, -8, -8, -8,
            1, 0x1fea, 0x1feb, MappingSource::UnicodeData, 0, -112, -112, -112,
            1, 0x1fec, 0x1fec, MappingSource::UnicodeData, 0, -7, -7, -7,
            1, 0x1ff3, 0x1ff3, MappingSource::UnicodeData, 9, 0, 0, 0,
            1, 0x1ff8, 0x1ff9, MappingSource::UnicodeData, 0, -128, -128, -128,
            1, 0x1ffa, 0x1ffb, MappingSource::UnicodeData, 0, -126, -126, -126,
            1, 0x1ffc, 0x1ffc, MappingSource::UnicodeData, 0, -9, -9, -9,
            1, 0x2126, 0x2126, MappingSource::CaseFolding, -7549, -7517, 0, 0,
            1, 0x212a, 0x212a, MappingSource::CaseFolding, -8415, -8383, 0, 0,
            1, 0x212b, 0x212b, MappingSource::CaseFolding, -8294, -8262, 0, 0,
            1, 0x2132, 0x2132, MappingSource::UnicodeData, 0, 28, 28, 28,
            1, 0x214e, 0x214e, MappingSource::UnicodeData, -28, 0, 0, 0,
            1, 0x2160, 0x216F, MappingSource::UnicodeData, 0, 16, 16, 16,
            1, 0x2170, 0x217F, MappingSource::UnicodeData, -16, 0, 0, 0,
            1, 0x2183, 0x2183, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x2184, 0x2184, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x24B6, 0x24CF, MappingSource::UnicodeData, 0, 26, 26, 26,
            1, 0x24D0, 0x24E9, MappingSource::UnicodeData, -26, 0, 0, 0,
            1, 0x2c00, 0x2c2e, MappingSource::UnicodeData, 0, 48, 48, 48,
            1, 0x2c30, 0x2c5e, MappingSource::UnicodeData, -48, 0, 0, 0,
            1, 0x2c60, 0x2c60, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x2c61, 0x2c61, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x2c62, 0x2c62, MappingSource::UnicodeData, 0, -10743, -10743, -10743,
            1, 0x2c63, 0x2c63, MappingSource::UnicodeData, 0, -3814, -3814, -3814,
            1, 0x2c64, 0x2c64, MappingSource::UnicodeData, 0, -10727, -10727, -10727,
            1, 0x2c65, 0x2c65, MappingSource::UnicodeData, -10795, 0, 0, 0,
            1, 0x2c66, 0x2c66, MappingSource::UnicodeData, -10792, 0, 0, 0,
            2, 0x2c67, 0x2c6c, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x2c6d, 0x2c6d, MappingSource::UnicodeData, 0, -10780, -10780, -10780,
            1, 0x2c6e, 0x2c6e, MappingSource::UnicodeData, 0, -10749, -10749, -10749,
            1, 0x2c6f, 0x2c6f, MappingSource::UnicodeData, 0, -10783, -10783, -10783,
            1, 0x2c70, 0x2c70, MappingSource::UnicodeData, 0, -10782, -10782, -10782,
            1, 0x2c72, 0x2c72, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x2c73, 0x2c73, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x2c75, 0x2c75, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0x2c76, 0x2c76, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0x2c7e, 0x2c7f, MappingSource::UnicodeData, 0, -10815, -10815, -10815,
            2, 0x2c80, 0x2ce3, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0x2ceb, 0x2cee, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0x2CF2, 0x2CF3, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0x2d00, 0x2d25, MappingSource::UnicodeData, -7264, 0, 0, 0,
            1, 0x2D27, 0x2D27, MappingSource::UnicodeData, -7264, 0, 0, 0,
            1, 0x2D2D, 0x2D2D, MappingSource::UnicodeData, -7264, 0, 0, 0,
            2, 0xa640, 0xa66d, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0xa680, 0xa697, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0xa722, 0xa72f, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0xa732, 0xa76f, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0xa779, 0xa77c, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0xa77d, 0xa77d, MappingSource::UnicodeData, 0, -35332, -35332, -35332,
            2, 0xa77e, 0xa787, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0xa78b, 0xa78b, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0xa78c, 0xa78c, MappingSource::UnicodeData, -1, 0, 0, 0,
            1, 0xa78d, 0xa78d, MappingSource::UnicodeData, 0, -42280, -42280, -42280,
            1, 0xa790, 0xa790, MappingSource::UnicodeData, 0, 1, 1, 1,
            1, 0xa791, 0xa791, MappingSource::UnicodeData, -1, 0, 0, 0,
            2, 0xa792, 0xa793, MappingSource::UnicodeData, -1, 1, 1, 1,
            2, 0xa7a0, 0xa7a9, MappingSource::UnicodeData, -1, 1, 1, 1,
            1, 0xA7AA, 0XA7AA, MappingSource::UnicodeData, 0, -42308, -42308, -42308,
            1, 0xff21, 0xff3a, MappingSource::UnicodeData, 0, 32, 32, 32,
            1, 0xff41, 0xff5a, MappingSource::UnicodeData, -32, 0, 0, 0,
            1, 0x10400, 0x10427, MappingSource::CaseFolding, 0, 40, 40, 40,
            1, 0x10428, 0x1044f, MappingSource::CaseFolding, -40, 0, 0, 0,
            1, 0x10c80, 0x10cb2, MappingSource::CaseFolding, 0, 64, 64, 64,
            1, 0x10cc0, 0x10cf2, MappingSource::CaseFolding, -64, 0, 0, 0,
            1, 0x118a0, 0x118bf, MappingSource::CaseFolding, 0, 32, 32, 32,
            1, 0x118c0, 0x118df, MappingSource::CaseFolding, -32, 0, 0, 0,
        };

        static const int numTransforms = sizeof(transforms) / sizeof(Transform);
        static const Transform lastTransform = transforms[numTransforms - 1];

        template <typename Char, typename Fn>
        bool RangeToEquivClass(uint& tblidx, uint l, uint h, uint& acth, Char equivl[EquivClassSize], Fn acceptSource)
        {
            Assert(l <= h);

            if (lastTransform.hi >= l)
            {
                // Skip transforms which come completely before l
                while (tblidx < numTransforms && (transforms[tblidx].hi < l || !acceptSource(transforms[tblidx].source)))
                {
                    tblidx++;
                }

                if (tblidx < numTransforms)
                {
                    // Does current transform intesect the desired range?
                    uint interl = max(l, static_cast<uint>(transforms[tblidx].lo));
                    uint interh = min(h, static_cast<uint>(transforms[tblidx].skipCountOfRange == 1 ? transforms[tblidx].hi : interl));
                    if (interl <= interh)
                    {
                        if (l < interl)
                        {
                            // Part of input range comes before next table range, so that sub-range has trivial equivalence class
                            acth = interl - 1;
                            for (int i = 0; i < EquivClassSize; i++)
                                equivl[i] = Chars<Char>::UTC(l);
                            return false; // trivial
                        }
                        else
                        {
                            // Input range begins at a table range, so map the character range
                            acth = interh;
                            transforms[tblidx].Apply(interl, equivl);
                            return true; // non-trivial
                        }
                    }
                    // else fall-through: No intersection, so nothing in this range has non-trivial equivalence class
                }
            }
            // else fall-through: No more transforms, so nothing in this range has a non-trivial equivalence class

            acth = h;
            for (int i = 0; i < EquivClassSize; i++)
            {
                equivl[i] = Chars<Char>::UTC(l);
            }
            return false; // trivial
        }

        bool RangeToEquivClass(uint & tblidx, uint l, uint h, uint & acth, __out_ecount(EquivClassSize) wchar_t equivl[EquivClassSize])
        {
            return RangeToEquivClass(tblidx, l, h, acth, equivl, [](MappingSource source) {
                return source == MappingSource::UnicodeData;
            });
        }

        bool RangeToEquivClass(uint & tblidx, uint l, uint h, uint & acth, __out_ecount(EquivClassSize) codepoint_t equivl[EquivClassSize])
        {
            return RangeToEquivClass(tblidx, l, h, acth, equivl, [](MappingSource source) {
                return source == MappingSource::CaseFolding || source == MappingSource::UnicodeData;
            });
        }

        bool RangeToEquivClassOnlyInSource(MappingSource mappingSource, uint& tblidx, uint l, uint h, uint& acth, __out_ecount(EquivClassSize) wchar_t equivl[EquivClassSize])
        {
            return RangeToEquivClass(tblidx, l, h, acth, equivl, [&](MappingSource actualSource) {
                return mappingSource == actualSource;
            });
        }
    }
}