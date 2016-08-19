//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

WScript.LoadScriptFile("..\\UnitTestFramework\\UnitTestFramework.js");

var tests = [
  {
    name: "super in direct eval inside an object method",
    body: function () {
        var value;
        var obj = {
            m() { value = eval('super.value;'); }
        };

        assert.areEqual(undefined, value);
        obj.m();
        assert.areEqual(undefined, value);
        Object.setPrototypeOf(obj, { value: "super" });
        obj.m();
        assert.areEqual("super", value);
    }
  },
  {
    name: "home object's prototype is null",
    body: function () {
        var value;
        var obj = {
            m() {
                value = "identifier";
                super.prop;
             },
            n() {
                value = "expression";
                super["prop"];
            }
        };
        
        Object.setPrototypeOf(obj, null);
        assert.throws(obj.m, TypeError, "identifier", "Unable to get property 'prop' of undefined or null reference");
        assert.throws(obj.n, TypeError, "expression", "Unable to get property 'prop' of undefined or null reference");
    }
  },
  {
    name: "super in strict mode - object",
    body: function () {
        "use strict";
        var obj0 = {
            m() {
                super.prop = "identifier";
                Object.freeze(obj0);
                try {
                    super.prop = "super";
                    throw Error("Expected to throw");
                } catch (e) {
                    assert.areEqual("TypeError", e.name);
                    assert.areEqual("Assignment to read-only properties is not allowed in strict mode", e.description);
                }
            }
        };
        
        var obj1 = {
            m() {
                super['prop'] = "expression";
                Object.freeze(obj1);
                try {
                    super['prop'] = "super";
                    throw Error("Expected to throw");
                } catch (e) {
                    assert.areEqual("TypeError", e.name);
                    assert.areEqual("Assignment to read-only properties is not allowed in strict mode", e.description);
                }
            }
        };
        
        var base = {};
        Object.setPrototypeOf(obj0, base);
        Object.setPrototypeOf(obj1, base);
        obj0.m();
        assert.areEqual("identifier", obj0.prop);
        obj1.m();
        assert.areEqual("expression", obj1.prop);
    }
  },
  {
    name: "super in strict mode - class",
    body: function () {
        class A {
            m() {
                super.prop = "identifier";
                Object.freeze(A.prototype);
                try {
                    super.prop = "super";
                    throw Error("Expected to throw");
                } catch (e) {
                    assert.areEqual("TypeError", e.name);
                    assert.areEqual("Assignment to read-only properties is not allowed in strict mode", e.description);
                }
            }
        }
        A.prototype.m();
        assert.areEqual("identifier", A.prototype.prop);

        class B {
            m() {
                super['prop'] = "expression";
                Object.freeze(B.prototype);
                try {
                    super['prop'] = "super";
                    throw Error("Expected to throw");
                } catch (e) {
                    assert.areEqual("TypeError", e.name);
                    assert.areEqual("Assignment to read-only properties is not allowed in strict mode", e.description);
                }
            }
        }
        B.prototype.m();
        assert.areEqual("expression", B.prototype.prop);
    }
  },
  {
    name: "super property in eval",
    body: function () {
        var valuex, valuey, valuez, valuet;
        
        var a = { x: 'a', y: 'a', z: 'a', t: 'a' };
        var b = { y: 'b', t: 'b' };
        Object.setPrototypeOf(b, a);
        
        var obj = {
            x : 'obj',
            y : 'obj',
            z : 'obj',
            t : 'obj',
            m() {
                valuex = eval('super.x');
                valuey = eval('super.y');
            },
            n() {
                valuez = eval('super["z"]');
                valuet = eval('super["t"]');
            }
        };
        
        Object.setPrototypeOf(obj, b);
        obj.m();
        assert.areEqual("a", valuex, "value x == 'a'");
        assert.areEqual("b", valuey, "value y == 'b'");
        obj.n();
        assert.areEqual("a", valuez, "value z == 'a'");
        assert.areEqual("b", valuet, "value t == 'b'");
    }
  },
  {
    name: "evaluation of expression before making super property reference",
    body: function () {
        var value = undefined;
        
        assert.throws(function(){ eval('super[value = 0]'); }, ReferenceError, "super[value = 0]", "Missing or invalid 'super' binding");
        assert.areEqual(0, value);
    }
  },
  {
    name: "evaluation of argument list after getting super constructor",
    body: function () {
        var value = undefined;
        
        class A extends Object { constructor() { super(value = 1); } }
        Object.setPrototypeOf(A, toString);
        assert.throws(()=>new A(), TypeError, "Function is not a constructor", "Function is not a constructor");
        assert.areEqual(undefined, value);
    }
  },
  {
    name: "super reference may not be deleted",
    body: function () {
        assert.throws(
            function() {
                class A extends Object {
                    constructor() { delete super.prop; }
                }
                new A();
            }, ReferenceError, "attempts to delete super property", "Unable to delete property with a super reference");
    }
  },
  {
    name: "super reference may not be deleted",
    body: function () {
        var value = 0;
        assert.throws(
            function() {
                class A extends null {
                    constructor() { value++; super(); value++; }
                }
                new A();
            }, TypeError, "attempts to call a null super constructor", "Function is not a constructor");
    }
  },
];

testRunner.runTests(tests, { verbose: WScript.Arguments[0] != "summary" });
