//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

function asmModule(stdlib, imports, buffer) {
    "use asm";

    var log = stdlib.Math.log;
    var toF = stdlib.Math.fround;
    var imul = stdlib.Math.imul;

    var i4 = stdlib.SIMD.Int32x4;
    var i4store = i4.store;
    var i4swizzle = i4.swizzle;
    var i4check = i4.check;

    var f4 = stdlib.SIMD.Float32x4;
    var f4equal = f4.equal;
    var f4lessThan = f4.lessThan;
    var f4splat = f4.splat;
    var f4store = f4.store;
    var f4load = f4.load;
    var f4check = f4.check;
    var f4abs = f4.abs;
    var f4add = f4.add;
    var f4sub = f4.sub;

    var Float32Heap = new stdlib.Float32Array(buffer);
    var Int32Heap = new stdlib.Int32Array(buffer);
    var BLOCK_SIZE = 4;

    function matrixSubtraction(aIndex, bIndex, cIndex) {
        var i = 0, dim1 = 0, dim2 = 0, matrixSize = 0;
        var aPiece = f4(0.0, 0.0, 0.0, 0.0), bPiece = f4(0.0, 0.0, 0.0, 0.0);

        dim1 = Int32Heap[aIndex] | 0;
        dim2 = Int32Heap[aIndex + 1] | 0;
        matrixSize = imul(dim1, dim2);

        Int32Heap[cIndex] = dim1;
        Int32Heap[cIndex + 1] = dim2;

        while (i < matrixSize) {
            aPiece = f4load(Float32Heap, aIndex + 2 + i );
            bPiece = f4load(Float32Heap, bIndex + 2 + i );
            var f4v = f4sub(aPiece, bPiece);
            //print("f4v = " + f4v.toString());
            f4store(Float32Heap, cIndex + 2 + i, f4v);

            i = (i + BLOCK_SIZE)|0;
        }

        return 0;
    }

    function new2DMatrix(startIndex, dim1, dim2, bias) {
        var i = 0, matrixSize = 0;
        matrixSize = imul(dim1, dim2);
        Int32Heap[startIndex ] = dim1;
        Int32Heap[startIndex + 1] = dim2;
        for (i = 0; i < matrixSize - BLOCK_SIZE; i = i + BLOCK_SIZE) {
            f4store(Float32Heap, startIndex + 2 + i , f4(i + 1 + bias, i + 2+ bias, i + 3+ bias, i + 4+ bias));
        }
        for (; i < matrixSize; i = i + 1) {
            Float32Heap[(startIndex + 2 + i)] = i + 1;
        }
        return (startIndex + 2 + i);
    }

    return {
        new2DMatrix: new2DMatrix,
        matrixSubtraction: matrixSubtraction
    };
}

function print2DMatrix(buffer, start) {
    var IntHeap32 = new Int32Array(buffer);
    var FloatHeap32 = new Float32Array(buffer);
    var f4;
    var dim1 = IntHeap32[start];
    var dim2 = IntHeap32[start + 1];
    print(dim1 + " by " + dim2 + " matrix");

    for (var i = 0; i < Math.imul(dim1, dim2) ; i += 4) {
        f4 = SIMD.Float32x4.load(FloatHeap32, i + start + 2);
        print(f4.toString());
    }
}

var buffer = new ArrayBuffer(1024);
var m = asmModule(this, null, buffer);

print("2D Matrix Subtraction");
m.new2DMatrix(0, 4, 4, 0);
m.new2DMatrix(100, 4, 4, 10);
m.matrixSubtraction(0, 100, 200);

var Float32Heap = new Float32Array(buffer);
for (var i = 200 + 2; i < 200 + 16 + 2; i++)
    print(Float32Heap[i]);

print2DMatrix(buffer, 200);
    
print("Second time --------------------------");
m.matrixSubtraction(0, 100, 200);
for (var i = 200 + 2; i < 200 + 16 + 2; i++)
    print(Float32Heap[i]);
print2DMatrix(buffer, 200);