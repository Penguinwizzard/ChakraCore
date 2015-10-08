//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct")
    else
        WScript.Echo(">> Fail!")
}
var si = SIMD.Int32x4(10, -20, 30, 0);
function testScalarGetters() {
    WScript.Echo('Int32x4 scalar getters');
    var a = SIMD.Int32x4(1, 2, 3, 4);
    equal(1, SIMD.Int32x4.extractLane(a, 0));
    equal(2, SIMD.Int32x4.extractLane(a, 1));
    equal(3, SIMD.Int32x4.extractLane(a, 2));
    equal(4, SIMD.Int32x4.extractLane(a, 3));
}

function testExtractLane1() {
    WScript.Echo("I4 ExtractLane");

    WScript.Echo(typeof si);
    WScript.Echo(si.toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 0));
    WScript.Echo(SIMD.Int32x4.extractLane(si, 0).toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 1))
    WScript.Echo(SIMD.Int32x4.extractLane(si, 1).toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 2));
    WScript.Echo(SIMD.Int32x4.extractLane(si, 2).toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 3));
    WScript.Echo(SIMD.Int32x4.extractLane(si, 3).toString());
}

function testReplaceLane1() {
    WScript.Echo("I4 ReplaceLane");

    var v = SIMD.Int32x4.replaceLane(si, 0, 10)
    WScript.Echo(v.toString());

    v = SIMD.Int32x4.replaceLane(si, 1, 12)
    WScript.Echo(v.toString());

    v = SIMD.Int32x4.replaceLane(si, 2, -30)
    WScript.Echo(v.toString());

    v = SIMD.Int32x4.replaceLane(si, 3, 0)
    WScript.Echo(v.toString());
}

testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();


testExtractLane1();
WScript.Echo();
testReplaceLane1();
WScript.Echo();

