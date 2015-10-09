function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testShiftleftByScalar() {
    WScript.Echo("Int32x4 shiftLeftByScalar");
    var a = SIMD.Int32x4(0x80000000, 0x7000000, 0xFFFFFFFF, 0x0);
    var b = SIMD.Int32x4.shiftLeftByScalar(a, 1)
    equal(0x0, SIMD.Int32x4.extractLane(b, 0));
    equal(0xE000000, SIMD.Int32x4.extractLane(b, 1));
    equal(-2, SIMD.Int32x4.extractLane(b, 2));
    equal(0x0, SIMD.Int32x4.extractLane(b, 3));
    var c = SIMD.Int32x4(1, 2, 3, 4);
    var d = SIMD.Int32x4.shiftLeftByScalar(c, 1)
    equal(2, SIMD.Int32x4.extractLane(d, 0));
    equal(4, SIMD.Int32x4.extractLane(d, 1));
    equal(6, SIMD.Int32x4.extractLane(d, 2));
    equal(8, SIMD.Int32x4.extractLane(d, 3));
}

function testShiftRightByScalar() {
    WScript.Echo("Int32x4 shiftRightByScalar");
    var a = SIMD.Int32x4(0x80000000, 0x7000000, 0xFFFFFFFF, 0x0);
    var b = SIMD.Int32x4.shiftRightByScalar(a, 1)
    equal(-1073741824, SIMD.Int32x4.extractLane(b, 0));
    equal(0x03800000, SIMD.Int32x4.extractLane(b, 1));
    equal(-1, SIMD.Int32x4.extractLane(b, 2));
    equal(0x0, SIMD.Int32x4.extractLane(b, 3));
    var c = SIMD.Int32x4(1, 2, 3, 4);
    var d = SIMD.Int32x4.shiftRightByScalar(c, 1)
    equal(0, SIMD.Int32x4.extractLane(d, 0));
    equal(1, SIMD.Int32x4.extractLane(d, 1));
    equal(1, SIMD.Int32x4.extractLane(d, 2));
    equal(2, SIMD.Int32x4.extractLane(d, 3));
    var c = SIMD.Int32x4(-1, -2, -3, -4);
    var d = SIMD.Int32x4.shiftRightByScalar(c, 1)
    equal(-1, SIMD.Int32x4.extractLane(d, 0));
    equal(-1, SIMD.Int32x4.extractLane(d, 1));
    equal(-2, SIMD.Int32x4.extractLane(d, 2));
    equal(-2, SIMD.Int32x4.extractLane(d, 3));
}


testShiftleftByScalar();
testShiftleftByScalar();
testShiftleftByScalar();
testShiftleftByScalar();

testShiftRightByScalar();
testShiftRightByScalar();
testShiftRightByScalar();
testShiftRightByScalar();

