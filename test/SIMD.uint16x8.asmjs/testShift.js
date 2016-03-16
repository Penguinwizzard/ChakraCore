//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft Corporation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
this.WScript.LoadScriptFile("..\\UnitTestFramework\\SimdJsHelpers.js");
function asmModule(stdlib, imports) {
    "use asm";

    var ui8 = stdlib.SIMD.Uint16x8;
    var ui8check = ui8.check;
    var ui8shiftLeftByScalar = ui8.shiftLeftByScalar;
    var ui8shiftRightByScalar = ui8.shiftRightByScalar;

    var globImportui8 = ui8check(imports.g1);    

    var ui8g1 = ui8(106, 0, 65535, 1082, 3192, 32, 19, 5);                 

    var loopCOUNT = 8;

    function testShiftLeftScalarLocal()
    {
        var a = ui8(5000, 3401, 665, 3224, 948, 2834, 7748, 25);
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0)
        {
            a = ui8shiftLeftByScalar(a, 1);
        }

        return ui8check(a);
    }
    
    function testShiftRightScalarLocal()
    {
        var a = ui8(5000, 3401, 665, 3224, 948, 2834, 7748, 25);
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            a = ui8shiftRightByScalar(a, 1);
        }

        return ui8check(a);
    }

    function testShiftLeftScalarGlobal()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            ui8g1 = ui8shiftLeftByScalar(ui8g1, 1);
        }
        return ui8check(ui8g1);
    }

    function testShiftRightScalarGlobal()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            ui8g1 = ui8shiftRightByScalar(ui8g1, 1);
        }
        return ui8check(ui8g1);
    }

    function testShiftLeftScalarGlobalImport()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            globImportui8 = ui8shiftLeftByScalar(globImportui8, 1);
        }
        return ui8check(globImportui8);
    }

    function testShiftRightScalarGlobalImport()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            globImportui8 = ui8shiftRightByScalar(globImportui8, 1);
        }
        return ui8check(globImportui8);
    }

    function testShiftLeftScalarLocal1()
    {
        var a = ui8(5000, 3401, 665, 3224, 948, 2834, 7748, 25);
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0)
        {
            a = ui8shiftLeftByScalar(a, 16);
            a = ui8shiftLeftByScalar(a, 17);
        }

        return ui8check(a);
    }
    
    function testShiftRightScalarLocal1()
    {
        var a = ui8(5000, 3401, 665, 3224, 948, 2834, 7748, 25);
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            a = ui8shiftRightByScalar(a, 16);
            a = ui8shiftRightByScalar(a, 17);
        }

        return ui8check(a);
    }

    function testShiftLeftScalarGlobal1()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            ui8g1 = ui8shiftLeftByScalar(ui8g1, 16);
            ui8g1 = ui8shiftLeftByScalar(ui8g1, 17);
        }
        return ui8check(ui8g1);
    }

    function testShiftRightScalarGlobal1()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            ui8g1 = ui8shiftRightByScalar(ui8g1, 16);
            ui8g1 = ui8shiftRightByScalar(ui8g1, 17);
        }
        return ui8check(ui8g1);
    }

    function testShiftLeftScalarGlobalImport1()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            globImportui8 = ui8shiftLeftByScalar(globImportui8, 16);
            globImportui8 = ui8shiftLeftByScalar(globImportui8, 17);
        }
        return ui8check(globImportui8);
    }

    function testShiftRightScalarGlobalImport1()
    {
        var loopIndex = 0;

        for (loopIndex = 0; (loopIndex | 0) < (loopCOUNT | 0) ; loopIndex = (loopIndex + 1) | 0) {
            globImportui8 = ui8shiftRightByScalar(globImportui8, 16);
            globImportui8 = ui8shiftRightByScalar(globImportui8, 17);
        }
        return ui8check(globImportui8);
    }

    return {
        testShiftLeftScalarLocal: testShiftLeftScalarLocal, testShiftRightScalarLocal: testShiftRightScalarLocal, testShiftLeftScalarGlobal: testShiftLeftScalarGlobal,
        testShiftRightScalarGlobal: testShiftRightScalarGlobal, testShiftLeftScalarGlobalImport: testShiftLeftScalarGlobalImport, testShiftRightScalarGlobalImport: testShiftRightScalarGlobalImport,
        testShiftLeftScalarLocal1: testShiftLeftScalarLocal1, testShiftRightScalarLocal1: testShiftRightScalarLocal1, testShiftLeftScalarGlobal1: testShiftLeftScalarGlobal1,
        testShiftRightScalarGlobal1: testShiftRightScalarGlobal1, testShiftLeftScalarGlobalImport1: testShiftLeftScalarGlobalImport1, testShiftRightScalarGlobalImport1: testShiftRightScalarGlobalImport1
    };
}

var m = asmModule(this, {g1:SIMD.Uint16x8(50, 1000, 3092, 3393, 0, 39283838, 0, 838)});

equalSimd([34816, 18688, 39168, 38912, 46080, 4608, 17408, 6400], m.testShiftLeftScalarLocal(), SIMD.Uint16x8, "testShift1");
equalSimd([34816, 18688, 39168, 38912, 46080, 4608, 17408, 6400], m.testShiftLeftScalarLocal(), SIMD.Uint16x8, "testShift1");

equalSimd([19, 13, 2, 12, 3, 11, 30, 0], m.testShiftRightScalarLocal(), SIMD.Uint16x8, "testShift2");
equalSimd([19, 13, 2, 12, 3, 11, 30, 0], m.testShiftRightScalarLocal(), SIMD.Uint16x8, "testShift2");

equalSimd([27136, 0, 65280, 14848, 30720, 8192, 4864, 1280], m.testShiftLeftScalarGlobal(), SIMD.Uint16x8, "testShift3");
equalSimd([106, 0, 255, 58, 120, 32, 19, 5], m.testShiftRightScalarGlobal(), SIMD.Uint16x8, "testShift4");

equalSimd([27136, 0, 65280, 14848, 30720, 8192, 4864, 1280], m.testShiftLeftScalarGlobal(), SIMD.Uint16x8, "testShift3");
equalSimd([106, 0, 255, 58, 120, 32, 19, 5], m.testShiftRightScalarGlobal(), SIMD.Uint16x8, "testShift4");

equalSimd([12800, 59392, 5120, 16640, 0, 32256, 0, 17920], m.testShiftLeftScalarGlobalImport(), SIMD.Uint16x8, "testShift5");
equalSimd([50, 232, 20, 65, 0, 126, 0, 70], m.testShiftRightScalarGlobalImport(), SIMD.Uint16x8, "testShift");

equalSimd([12800, 59392, 5120, 16640, 0, 32256, 0, 17920], m.testShiftLeftScalarGlobalImport(), SIMD.Uint16x8, "testShift5");
equalSimd([50, 232, 20, 65, 0, 126, 0, 70], m.testShiftRightScalarGlobalImport(), SIMD.Uint16x8, "testShift");

equalSimd([34816, 18688, 39168, 38912, 46080, 4608, 17408, 6400], m.testShiftLeftScalarLocal1(), SIMD.Uint16x8, "testShift1_1");
equalSimd([19, 13, 2, 12, 3, 11, 30, 0], m.testShiftRightScalarLocal1(), SIMD.Uint16x8, "testShift2_1");
equalSimd([27136, 0, 65280, 14848, 30720, 8192, 4864, 1280], m.testShiftLeftScalarGlobal1(), SIMD.Uint16x8, "testShift3_1");
equalSimd([106, 0, 255, 58, 120, 32, 19, 5], m.testShiftRightScalarGlobal1(), SIMD.Uint16x8, "testShift4_1");
equalSimd([12800, 59392, 5120, 16640, 0, 32256, 0, 17920], m.testShiftLeftScalarGlobalImport1(), SIMD.Uint16x8, "testShift5_1");
equalSimd([50, 232, 20, 65, 0, 126, 0, 70], m.testShiftRightScalarGlobalImport1(), SIMD.Uint16x8, "testShift_1");

print("PASS");
