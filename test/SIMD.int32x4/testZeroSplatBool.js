function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testSplat() {
    var n = SIMD.Int32x4.splat(3);
    WScript.Echo("splat");
    equal(3, SIMD.Int32x4.extractLane(n, 0));
    equal(3, SIMD.Int32x4.extractLane(n, 1));
    equal(3, SIMD.Int32x4.extractLane(n, 2));
    equal(3, SIMD.Int32x4.extractLane(n, 3));
}

function testBool() {
    var n = SIMD.Int32x4.bool(true, false, true, false);
    WScript.Echo("bool");
    equal(-1, SIMD.Int32x4.extractLane(n, 0));
    equal(0, SIMD.Int32x4.extractLane(n, 1));
    equal(-1, SIMD.Int32x4.extractLane(n, 2));
    equal(0, SIMD.Int32x4.extractLane(n, 3));
}

testSplat();
testSplat();
testSplat();
testSplat();
testSplat();
testSplat();
testSplat();
testSplat();

testBool();
testBool();
testBool();
testBool();
testBool();
testBool();
testBool();
testBool();

