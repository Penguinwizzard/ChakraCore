//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in ch
    this.WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");
}

var tests = [
   {
       name: "Concat toLength tests",
       body: function () 
       {
            var c = {};
       }
   },
   {
       name: "IndexOf toLength tests",
       body: function () 
       {
            
            var a = { length : 4294967295, [4294967293] : 2, [4294967294]: 3 };
            assert.areEqual(-1, Array.prototype.indexOf.call(a,2), "");
            assert.areEqual(-1, Array.prototype.indexOf.call(a,3), "");
       }
   },
   {
       name: "Reverse toLength tests",
       body: function () 
       {
            var a = { length : 4294967295, [4294967293] : 2, [4294967294]: 3 };
            Array.prototype.reverse.call(a);
            
       }
   },
   {
       name: "Shift toLength tests",
       body: function () 
       {
            var a = { length : 4294967295, [4294967293] : 2, [4294967294]: 3 };
            Array.prototype.shift.call(s);
       }
   },
   {
       name: "UnShift toLength tests",
       body: function () 
       {
            var o = {
                0 : 0,
                4294967294 : 2,
                4294967295 : 3,
                4294967296 : 4,
                length : 4294967297
                }
            
            Array.prototype.unshift.call(o, -1);
       }
   },
   {
       name: "Slice toLength tests",
       body: function () 
       {
            class MyArray extends Array {
                // Overwrite species to the parent Array constructor
                static get [Symbol.species]() { return Object; }
            }
            var myArr = new MyArray(4294967297);
            myArr[0] = 0;
            myArr[4294967294] = 1;
            myArr[4294967295] = 2;
            Array.prototype.slice.call(myArr);
       }
   },
   {
       name: "Splice toLength tests",
       body: function () 
       {
            //TODO when we change splice
       }
   },
   {
       name: "Every toLength tests",
       body: function () 
       {
            var a = [];
            a[4294967294] = 2;
            a[4294967295] = 3;
            function isEven(element, index, array) 
            {
                return element %2 == 0;
            }
            a.every(isEven);
            
            var o = {
                0 : 0,
                4294967294 : 2,
                4294967295 : 3,
                4294967296 : 4,
                length : 4294967297
                }
            
       }
   },
   {
       name: "Some toLength tests",
       body: function () 
       {
            var a = [];
            a[0] = 1;
            a[4294967294] = 2;
            a[4294967295] = 3;
            function isEven(element, index, array) 
            {
                return element %2 == 0;
            }
            a.some(isEven); 
            
            var o = {
                0 : 0,
                4294967294 : 2,
                4294967295 : 3,
                4294967296 : 4,
                length : 4294967297
                }
            
       }
   },
      {
       name: "ForEach toLength tests",
       body: function () 
       {
            var a = [];
            a[4294967294] = 3;
            a[4294967295] = 4;
            var sum = 0;
            function sumation(element, index, array) {
                sum += element;
            }
            a.forEach(sumation); 
            assert.areEqual(7,sum);
            sum = 0;
            
            var o = {
                0 : 0,
                4294967294 : 2,
                4294967295 : 3,
                4294967296 : 4,
                length : 4294967297
                }
                
            Array.prototype.forEach.call(o, sumation);
            assert.areEqual(9,sum,"");
       }
   },
      {
       name: "Map toLength tests",
       body: function () 
       {
            var a = [];
            a[4294967294] = 9;
            a[4294967295] = 16;
            var b = a.map(Math.sqrt); // same issue as ForEach, Some, & Every
            
            Array.prototype.map.call(o, Math.sqrt));
            
            
            
            class MyArray extends Array {
                // Overwrite species to the parent Array constructor
                static get [Symbol.species]() { return Object; }
            }
            var myArr = new MyArray();
            myArr[0] = 0;
            myArr[4294967294] = 1;
            myArr[4294967295] = 2;
            Array.prototype.map.call(myArr, Math.sqrt);
       }
   },
   {
       name: "Filter toLength tests",
       body: function () 
       {
            var a = [];
            a[4294967294] = 9;
            a[4294967295] = 16;
            
            function biggerThanFive(element) 
            {
                return element > 5;
            }
            a.filter(biggerThanFive); 
            var o = {
                4294967294 : 2,
                4294967295 : 3,
                4294967296 : 4,
                length : 4294967297
                }
                
            Array.prototype.filter.call(o, biggerThanFive);
       }
   },
   {
       name: "Reduce toLength tests",
       body: function () 
       {
            var a = [];
            a[4294967294] = 3;
            a[4294967295] = 4;
            var sum = function(a, b) { 
                return a + b;
            }
            a.reduce(sum); 
            var o = {
                0 : 0,
                4294967294 : 2,
                4294967295 : 3,
                4294967296 : 4,
                length : 4294967297
                }
            Array.prototype.reduce.call(o, sum);
       }
   },
   {
       name: "ReduceRight toLength tests",
       body: function () 
       {
            var a = [];
            a[4294967294] = 3;
            a[4294967295] = 4;
            var sum = function(a, b) { 
                return a + b;
            }
            a.reduceRight(sum); 
            
            var o = {
                4294967294 : 2,
                4294967295 : 3,
                4294967296 : 4,
                length : 4294967297
                }
            Array.prototype.reduceRight.call(o, sum);
       }
   },   
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
