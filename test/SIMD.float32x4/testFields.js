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
var sf = SIMD.Float32x4(1.35, -2.0, 3.4, 0.0);
function testExtractLane() {
    WScript.Echo("F4 ExtractLane");

    WScript.Echo(typeof sf);
    WScript.Echo(sf.toString());

    WScript.Echo(typeof SIMD.Float32x4.extractLane(sf, 0));
    WScript.Echo(SIMD.Float32x4.extractLane(sf, 0).toString());

    WScript.Echo(typeof SIMD.Float32x4.extractLane(sf, 1))
    WScript.Echo(SIMD.Float32x4.extractLane(sf, 1).toString());

    WScript.Echo(typeof SIMD.Float32x4.extractLane(sf, 2));
    WScript.Echo(SIMD.Float32x4.extractLane(sf, 2).toString());

    WScript.Echo(typeof SIMD.Float32x4.extractLane(sf, 3));
    WScript.Echo(SIMD.Float32x4.extractLane(sf, 3).toString());
}

function testReplaceLane() {
    WScript.Echo("F4 ReplaceLane");

    var v = SIMD.Float32x4.replaceLane(sf, 0, 10.2)
    WScript.Echo(v.toString());

    v = SIMD.Float32x4.replaceLane(sf, 1, 12.3)
    WScript.Echo(v.toString());

    v = SIMD.Float32x4.replaceLane(sf, 2, -30.2)
    WScript.Echo(v.toString());

    v = SIMD.Float32x4.replaceLane(sf, 3, 0.0)
    WScript.Echo(v.toString());

}

function testScalarGetters() {
    WScript.Echo('Float32x4 scalar getters');
    var a = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    equal(1.0, SIMD.Float32x4.extractLane(a, 0));
    equal(2.0, SIMD.Float32x4.extractLane(a, 1));
    equal(3.0, SIMD.Float32x4.extractLane(a, 2));
    equal(4.0, SIMD.Float32x4.extractLane(a, 3));
}



testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();

testExtractLane();
WScript.Echo();
testReplaceLane();
WScript.Echo();
