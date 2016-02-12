//-------------------------------------------------------------------------------------------------------
// Copyright (C) 2016 Intel Corporation.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
this.WScript.LoadScriptFile("..\\UnitTestFramework\\SimdJsHelpers.js");
function asmModule(stdlib, imports) {
    "use asm";

    var ui8 = stdlib.SIMD.Uint16x8;
    var ui8check = ui8.check;
    var ui8min = ui8.min;
    var ui8max = ui8.max;

    var globImportui8 = ui8check(imports.g1);    

    var ui8g1 = ui8(106, 0, 10, 1082, 3192, 32, 19, 5);         
    var ui8g2 = ui8(353, 4929, 1128, 0, 3692, 3937, 9755, 2638);          

    var loopCOUNT = 3;

    function testMinLocal()
    {
        var a = ui8(5000, 3401, 665, 3224, 948, 2834, 7748, -25);
        var b = ui8(4999, 3401, 0, 1000, 1000, 9299, 0, 0);
        var result = ui8(0, 0, 0, 0, 0, 0, 0, 0);

        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {
            result = ui8min(a, b);
            loopIndex = (loopIndex + 1) | 0;
        }

        return ui8check(result);
    }
    
    function testMaxLocal()
    {
        var a = ui8(5000, 3401, 665, 3224, 948, 2834, 7748, 25);
        var b = ui8(4999, 3401, 0, 1000, 1000, 9299, 0, 0);
        var result = ui8(0, 0, 0, 0, 0, 0, 0, 0)

        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {
            result = ui8max(a, b);
            loopIndex = (loopIndex + 1) | 0;
        }

        return ui8check(result);
    }

    function testMinGlobal()
    {
        var result = ui8(0, 0, 0, 0, 0, 0, 0, 0);

        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {
            result = ui8min(ui8g1, ui8g2);
            loopIndex = (loopIndex + 1) | 0;
        }

        return ui8check(result);
    }

    function testMaxGlobal()
    {
        var result = ui8(0, 0, 0, 0, 0, 0, 0, 0);

        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {
            result = ui8max(ui8g1, ui8g2);
            loopIndex = (loopIndex + 1) | 0;
        }

        return ui8check(result);
    }

    function testMinGlobalImport()
    {
        var a = ui8(5033, 3401, 665, 3224, 948, 2834, 7748, 25);
        var result = ui8(0, 0, 0, 0, 0, 0, 0, 0);

        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {
            result = ui8min(globImportui8, a);
            loopIndex = (loopIndex + 1) | 0;
        }

        return ui8check(result);
    }

    function testMaxGlobalImport()
    {
        var a = ui8(5033, 3401, 665, 3224, 948, 2834, 7748, 25);
        var result = ui8(0, 0, 0, 0, 0, 0, 0, 0);

        var loopIndex = 0;
        while ( (loopIndex|0) < (loopCOUNT|0)) {
            result = ui8max(globImportui8, a);
            loopIndex = (loopIndex + 1) | 0;
        }

        return ui8check(result);
    }
    
    return { testMinLocal: testMinLocal, testMaxLocal: testMaxLocal, testMinGlobal: testMinGlobal, testMaxGlobal: testMaxGlobal, testMinGlobalImport: testMinGlobalImport, testMaxGlobalImport: testMaxGlobalImport };
}

var m = asmModule(this, {g1:SIMD.Uint16x8(50, 1000, 3092, 3393, 0, 39283838, 0, 838)});

equalSimd([4999, 3401, 0, 1000, 948, 2834, 0, 0], m.testMinLocal(), SIMD.Uint16x8, "testMinLocal");
equalSimd([5000, 3401, 665, 3224, 1000, 9299, 7748, 25], m.testMaxLocal(), SIMD.Uint16x8, "testMaxLocal");
equalSimd([106, 0, 10, 0, 3192, 32, 19, 5], m.testMinGlobal(), SIMD.Uint16x8, "testMinGlobal");
equalSimd([353, 4929, 1128, 1082, 3692, 3937, 9755, 2638], m.testMaxGlobal(), SIMD.Uint16x8, "testMaxGlobal");
equalSimd([50, 1000, 665, 3224, 0, 2834, 0, 25], m.testMinGlobalImport(), SIMD.Uint16x8, "testMinGlobalImport");
equalSimd([5033, 3401, 3092, 3393, 948, 27774, 7748, 838], m.testMaxGlobalImport(), SIMD.Uint16x8, "testMaxGlobalImport");
print("PASS");
