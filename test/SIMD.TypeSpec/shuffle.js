WScript.LoadScriptFile("..\\UnitTestFramework\\SimdJsHelpers.js");

var globTotal;
function increment(a, b, lib)
{
    
    return lib.add(a, b);
}
function func1()
{
    var x, y;
    var i =0;
    var j = 0;
    var z;
    x = SIMD.Float32x4(100,101,102,103);
    y = SIMD.Float32x4(0, 1,2,3);
    
    print("swizzle");
    z = SIMD.Float32x4.shuffle(x, y, 0, 0, 0, 0);
    print(z.toString());
    print("2:2 - 1");
    z = SIMD.Float32x4.shuffle(x, y, 5, 6, 1, 2);
    print(z.toString());
    print("2:2 - 2");
    z = SIMD.Float32x4.shuffle(x, y, 5, 1, 6, 2);
    print(z.toString());
    print("2:2 - 3");
    z = SIMD.Float32x4.shuffle(x, y, 1, 5, 2, 6);
    print(z.toString());
    
    print("3:1 - 1");
    z = SIMD.Float32x4.shuffle(x, y, 1, 5, 6, 7);
    print(z.toString());
    z = SIMD.Float32x4.shuffle(x, y, 5, 1, 6, 7);
    print(z.toString());
    z = SIMD.Float32x4.shuffle(x, y, 5, 1, 3, 2);
    print(z.toString());
    z = SIMD.Float32x4.shuffle(x, y, 1, 5, 0, 1);
    print(z.toString());
    
}

var c = false;
var d = false;
var z;
func1();
func1();
func1();
func1();
func1();
func1();
func1();
func1();
func1();
func1();
print("PASS");
