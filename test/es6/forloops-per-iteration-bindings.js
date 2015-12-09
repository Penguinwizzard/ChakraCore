//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// ES6 for/for-in/for-of loops per iteration loop variable bindings tests

WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");

var tests = [
    {
        name: "[slot array] for-in and for-of loops have per iteration bindings for let and const variables",
        body: function () {
            var a = [];
            var i = 0;

            for (let x in { a: 1, b: 2, c: 3 }) {
                a[i++] = function () { return x; };
            }

            for (const x in { a: 1, b: 2, c: 3 }) {
                a[i++] = function () { return x; };
            }

            for (let x of [ 1, 2, 3 ]) {
                a[i++] = function () { return x; };
            }

            for (const x of [ 1, 2, 3 ]) {
                a[i++] = function () { return x; };
            }

            assert.areEqual('a', a[0](), "first for-let-in function captures x when it is equal to 'a'");
            assert.areEqual('b', a[1](), "second for-let-in function captures x when it is equal to 'b'");
            assert.areEqual('c', a[2](), "third for-let-in function captures x when it is equal to 'c'");

            assert.areEqual('a', a[3](), "first for-const-in function captures x when it is equal to 'a'");
            assert.areEqual('b', a[4](), "second for-const-in function captures x when it is equal to 'b'");
            assert.areEqual('c', a[5](), "third for-const-in function captures x when it is equal to 'c'");

            assert.areEqual(1, a[6](), "first for-let-of function captures x when it is equal to 1");
            assert.areEqual(2, a[7](), "second for-let-of function captures x when it is equal to 2");
            assert.areEqual(3, a[8](), "third for-let-of function captures x when it is equal to 3");

            assert.areEqual(1, a[9](), "first for-const-of function captures x when it is equal to 1");
            assert.areEqual(2, a[10](), "second for-const-of function captures x when it is equal to 2");
            assert.areEqual(3, a[11](), "third for-const-of function captures x when it is equal to 3");
        }
    },
    {
        name: "[activation object] for-in and for-of loops have per iteration bindings for let and const variables",
        body: function () {
            var a = [];
            var i = 0;

            for (let x in { a: 1, b: 2, c: 3 }) {
                a[i++] = function () { return eval("x"); };
            }

            for (const x in { a: 1, b: 2, c: 3 }) {
                a[i++] = function () { return eval("x"); };
            }

            for (let x of [ 1, 2, 3 ]) {
                a[i++] = function () { return eval("x"); };
            }

            for (const x of [ 1, 2, 3 ]) {
                a[i++] = function () { return eval("x"); };
            }

            assert.areEqual('a', a[0](), "first for-let-in function captures x when it is equal to 'a'");
            assert.areEqual('b', a[1](), "second for-let-in function captures x when it is equal to 'b'");
            assert.areEqual('c', a[2](), "third for-let-in function captures x when it is equal to 'c'");

            assert.areEqual('a', a[3](), "first for-const-in function captures x when it is equal to 'a'");
            assert.areEqual('b', a[4](), "second for-const-in function captures x when it is equal to 'b'");
            assert.areEqual('c', a[5](), "third for-const-in function captures x when it is equal to 'c'");

            assert.areEqual(1, a[6](), "first for-let-of function captures x when it is equal to 1");
            assert.areEqual(2, a[7](), "second for-let-of function captures x when it is equal to 2");
            assert.areEqual(3, a[8](), "third for-let-of function captures x when it is equal to 3");

            assert.areEqual(1, a[9](), "first for-const-of function captures x when it is equal to 1");
            assert.areEqual(2, a[10](), "second for-const-of function captures x when it is equal to 2");
            assert.areEqual(3, a[11](), "third for-const-of function captures x when it is equal to 3");
        }
    },
    {
        name: "const variables in for-in and for-of loops cannot be assigned",
        body: function () {
            assert.throws(function () { eval("for (const x in { a: 1 }) { x = 1; }"); }, SyntaxError, "assignment to const known at parse time in for-in loop", "Assignment to const");
            assert.throws(function () { for (const x in { a: 1 }) { eval("x = 1;"); } }, ReferenceError, "assignment to const only known at run time in for-in loop", "Assignment to const");
            assert.throws(function () { eval("for (const x of [ 0 ]) { x = 1; }"); }, SyntaxError, "assignment to const known at parse time in for-of loop", "Assignment to const");
            assert.throws(function () { for (const x of [ 0 ]) { eval("x = 1;"); } }, ReferenceError, "assignment to const only known at run time in for-of loop", "Assignment to const");
        }
    },
    {
        name: "Referring to a let or const loop variable in the collection expression of a for-in or for-of loop is a use before declaration",
        body: function () {
            assert.throws(function () { for (let x in { a: x }) { } }, ReferenceError, "for-let-in register use before declaration", "Use before declaration");
            assert.throws(function () { for (const x in { a: x }) { } }, ReferenceError, "for-const-in register use before declaration", "Use before declaration");
            assert.throws(function () { for (let x of [ x ]) { } }, ReferenceError, "for-let-of register use before declaration", "Use before declaration");
            assert.throws(function () { for (const x of [ x ]) { } }, ReferenceError, "for-const-of register use before declaration", "Use before declaration");

            assert.throws(function () { for (let x in { a: (() => x)() }) { } }, ReferenceError, "for-let-in slot array use before declaration", "Use before declaration");
            assert.throws(function () { for (const x in { a: (() => x)() }) { } }, ReferenceError, "for-const-in slot array use before declaration", "Use before declaration");
            assert.throws(function () { for (let x of [ (() => x)() ]) { } }, ReferenceError, "for-let-of slot array use before declaration", "Use before declaration");
            assert.throws(function () { for (const x of [ (() => x)() ]) { } }, ReferenceError, "for-const-of slot array use before declaration", "Use before declaration");

            assert.throws(function () { for (let x in { a: eval("x") }) { } }, ReferenceError, "for-let-in activation object use before declaration", "Use before declaration");
            assert.throws(function () { for (const x in { a: eval("x") }) { } }, ReferenceError, "for-const-in activation object use before declaration", "Use before declaration");
            assert.throws(function () { for (let x of [ eval("x") ]) { } }, ReferenceError, "for-let-of activation object use before declaration", "Use before declaration");
            assert.throws(function () { for (const x of [ eval("x") ]) { } }, ReferenceError, "for-const-of activation object use before declaration", "Use before declaration");

            var rx0 = null, rx1 = null;
            var ry0 = null, ry1 = null;

            function registerVars() {
                for (var x in { a: rx0 = x }) {
                    rx1 = x;
                }

                for (var y of [ ry0 = y ]) {
                    ry1 = y;
                }
            }
            registerVars();

            assert.areEqual(undefined, rx0, "register var declaration in for-in loop can be referenced before initialization");
            assert.areEqual('a', rx1, "sanity check that the for-in loop runs as expected");
            assert.areEqual(undefined, ry0, "register var declaration in for-of loop can be referenced before initialization");
            assert.areEqual(undefined, ry1, "sanity check that the for-of loop runs as expected");

            var sax0 = null, sax1 = null;
            var say0 = null, say1 = null;

            function slotArrayVars() {
                for (var x in { a: sax0 = x }) {
                    sax1 = (function () { return x; })();
                }

                for (var y of [ say0 = y ]) {
                    say1 = (function () { return y; })();
                }
            }
            slotArrayVars();

            assert.areEqual(undefined, sax0, "slot array var declaration in for-in loop can be referenced before initialization");
            assert.areEqual('a', sax1, "sanity check that the for-in loop runs as expected");
            assert.areEqual(undefined, say0, "slot array var declaration in for-of loop can be referenced before initialization");
            assert.areEqual(undefined, say1, "sanity check that the for-of loop runs as expected");

            var aox0 = null, aox1 = null;
            var aoy0 = null, aoy1 = null;

            function activationObjectVars() {
                for (var x in { a: aox0 = x }) {
                    aox1 = eval("x");
                }

                for (var y of [ aoy0 = y ]) {
                    aoy1 = eval("y");
                }
            }
            activationObjectVars();

            assert.areEqual(undefined, aox0, "slot array var declaration in for-in loop can be referenced before initialization");
            assert.areEqual('a', aox1, "sanity check that the for-in loop runs as expected");
            assert.areEqual(undefined, aoy0, "slot array var declaration in for-of loop can be referenced before initialization");
            assert.areEqual(undefined, aoy1, "sanity check that the for-of loop runs as expected");
        }
    },
    {
        name: "Capturing a let or const loop variable in the collection expression of a for-in or for-of loop still leads to a use before declaration",
        body: function () {
            var a = [], b = [];
            var i = 0, j = 0;

            for (let x in { a: a[i++] = () => x }) { b[j++] = () => x; }
            for (const x in { a: a[i++] = () => x }) { b[j++] = () => x; }
            for (let x of [ a[i++] = () => x ]) { b[j++] = () => x; }
            for (const x of [ a[i++] = () => x ]) { b[j++] = () => x; }

            for (let x in { a: a[i++] = () => eval("x") }) { b[j++] = () => eval("x"); }
            for (const x in { a: a[i++] = () => eval("x") }) { b[j++] = () => eval("x"); }
            for (let x of [ a[i++] = () => eval("x") ]) { b[j++] = () => eval("x"); }
            for (const x of [ a[i++] = () => eval("x") ]) { b[j++] = () => eval("x"); }

            assert.throws(a[0], ReferenceError, "for-let-in slot array capture use before declaration", "Use before declaration");
            assert.throws(a[1], ReferenceError, "for-const-in slot array capture use before declaration", "Use before declaration");
            assert.throws(a[2], ReferenceError, "for-let-of slot array capture use before declaration", "Use before declaration");
            assert.throws(a[3], ReferenceError, "for-const-of slot array capture use before declaration", "Use before declaration");

            assert.throws(a[4], ReferenceError, "for-let-in activation object capture use before declaration", "Use before declaration");
            assert.throws(a[5], ReferenceError, "for-const-in activation object capture use before declaration", "Use before declaration");
            assert.throws(a[6], ReferenceError, "for-let-of activation object capture use before declaration", "Use before declaration");
            assert.throws(a[7], ReferenceError, "for-const-of activation object capture use before declaration", "Use before declaration");

            assert.areEqual('a', b[0](), "sanity check for-let-in slot array capture body still initialized", "Use before declaration");
            assert.areEqual('a', b[1](), "sanity check for-const-in slot array capture body still initialized", "Use before declaration");
            assert.areEqual(a[2], b[2](), "sanity check for-let-of slot array capture body still initialized", "Use before declaration");
            assert.areEqual(a[3], b[3](), "sanity check for-const-of slot array capture body still initialized", "Use before declaration");

            assert.areEqual('a', b[4](), "sanity check for-let-in activation object capture body still initialized", "Use before declaration");
            assert.areEqual('a', b[5](), "sanity check for-const-in activation object capture body still initialized", "Use before declaration");
            assert.areEqual(a[6], b[6](), "sanity check for-let-of activation object capture body still initialized", "Use before declaration");
            assert.areEqual(a[7], b[7](), "sanity check for-const-of activation object capture body still initialized", "Use before declaration");
        }
    },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
