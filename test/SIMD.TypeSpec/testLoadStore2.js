function func1(arr)
{
// Mix normal and SIMD array access
// loops
// different SIMD types
    var i;
    var f4, f4_2;
    var byteSize = arr.length * arr.BYTES_PER_ELEMENT;
    
    for (i = 0; i < byteSize - 16; i++)
    {
        //print(i);
        f4 = SIMD.Float32x4(1111, 2222, 3, 4);    
    
        //print(f4.toString());
        SIMD.Float32x4.store2(arr, i, f4);
        //f4 = arr[i];
        f4 = SIMD.Float32x4.load(arr, i);
        print(f4.toString());
    }

}




// Add different typed arrays
arrBuff = new ArrayBuffer(32);
arr = new Int8Array(arrBuff);
for (i = 0; i < arr.length; i++)
{
    arr[i] = 0;
}
func1(arr);
for (i = 0; i < arr.length; i++)
{
    arr[i] = 0;
}
func1(arr);

