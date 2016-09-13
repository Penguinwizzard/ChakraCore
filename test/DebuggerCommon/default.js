//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// Default debugger cases
// We are validating that 
// 1. halting and stepping at formals
// 2. variables at body not visible at formals
// 2. excercising in multiple storage container, regslot, slotarray or actiation object 

var glob = {}; // This is just a dummy variable for log purpose

glob; /**bp:logJson("Stepping on default params - formals on regslot")**/
(function() {
    function f1(a = 1, b, c = 2) {
        var m = 3;
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');locals();dumpBreak();evaluate('a = 11');evaluate('a')**/
})();

glob; /**bp:logJson("Stepping on default params - formals on regslot and slot array")**/
(function() {
    function f1(a = 1, b, c = 2) {
        var m = 3;
        function bar() {
            a;
            m;
        }
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');locals();dumpBreak();evaluate('a = 11');evaluate('a')**/
})();

glob; /**bp:logJson("Stepping on default params - formals on activation object")**/
(function() {
    function f1(a = 1, b, c = 2) {
        var m = 3;
        eval('');
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');locals();dumpBreak();evaluate('a = 11');evaluate('a')**/
})();

glob; /**bp:logJson("Stepping on default params - has mixed destructuring patterns")**/
(function() {
    function f1({a} = {a:1}, b, [c] = [2]) {
        var m = 3;
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - one formal has function as default")**/
(function() {
    function f1(a = 1, b = function () { return 2; }, c = 3) {
        var m = 3;
    }
    f1(); /**bp:resume('step_into');resume('step_into');resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - one formal has function as default but formals on slot array")**/
(function() {
    function f1(a = 1, b = function () { return 2; }, c = 3) {
        function bar() {
            a;b;c;
        }
    }
    f1(); /**bp:resume('step_into');resume('step_into');resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - one formal has function as default but formals on activation object")**/
(function() {
    function f1(a = 1, b = function () { return 2; }, c = 3) {
        function bar() {
            a;b;c;
        }
        eval('');
    }
    f1(); /**bp:resume('step_into');resume('step_into');resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - break at function at formals and do setframe")**/
(function() {
    function f1(a = 1, b = function () { 
        return 2; /**bp:setFrame(1);locals()**/  
    }, c = b()) {
        var m = 3;
    }
    f1();
})();

glob; /**bp:logJson("Stepping on default params - split scope due to function at formals")**/
(function() {
    function f1(a = 1, b = function () { return a; }, c = 3) {
        var m = 3;
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');resume('step_into');locals();dumpBreak();resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - Breakpoint at the function defined in param scope when called from param scope")**/
(function() {
    function f1(a = 1, b = function () { 
        return a; /**bp:setFrame(1);locals()**/  
    }, c = b()) {
        var m = 3;
        m; /**bp:locals()**/
    }
    f1();
})();

glob; /**bp:logJson("Stepping on default params - Breakpoint at the function defined in param scope when called from body scope")**/
(function() {
    function f1(a = 1, b = function () { 
        return a; /**bp:setFrame(1);locals()**/  
    }) {
        var m = 3;
        var c = b();
    }
    f1();
})();

glob; /**bp:logJson("Stepping on default params - Breakpoint at the function defined in param scope to print the default frame")**/
(function() {
    function f1(a = 1, b = function () { 
        return a; /**bp:locals()**/  
    }) {
        var m = 3;
        var c = b();
    }
    f1();
})();

glob; /**bp:logJson("Stepping on default params - Split scope function with zero reg slots in param")**/
(function() {
    function f1(a = 1, b = function () { 
        return a + c(); /**bp:setFrame(1);locals()**/  
    }, c = () => {
        return b.length + a; /**bp:setFrame(1);locals()**/
    }) {
        var m = 3; /**bp:locals()**/
        var n = c();
        var o = b();
        o; /**bp:locals()**/
    }
    f1();
})();

glob; /**bp:logJson("Stepping on default params - Split scope function with zero reg slots in param and eval in both scopes")**/
(function() {
    function f1(a = 1, b = function () { 
        return a + c(); /**bp:setFrame(1);locals()**/  
    }, c = () => {
        return eval("b.length + a"); /**bp:setFrame(1);locals()**/
    }) {
        var m = 3; /**bp:locals()**/
        var n = c();
        var o = eval("b()");
        o; /**bp:locals()**/
    }
    f1();
})();

glob; /**bp:logJson("Stepping on default params - Split scope function with zero reg slots in param and print the body scope")**/
(function() {
    function f1(a = 1, b = function () { 
        return a + c();
    }, c = () => {
        return eval("b.length + a"); /**bp:stack();setFrame(3);locals()**/
    }) {
        var m = eval("b()");
    }
    f1(10);
})();

glob; /**bp:logJson("Stepping on default params - split scope due to function at formals and few var captured in inner function")**/
(function() {
    function f1(a = 1, b = function () { return a; }, c) {
        var m = 3;
        (function () {
            a;
            m;
        })();
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - split scope and body has eval")**/
(function() {
    function f1(a = 1, b = function () { return a; }, c) {
        var m = 3;
        (function () {
        })();
        eval('');
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - split scope and body has eval inside inner function")**/
(function() {
    function f1(a = 1, b = function () { return a; }, c) {
        var m = 3;
        (function () {
            eval('');
        })();
    }
    f1(); /**bp:resume('step_into');locals();dumpBreak();resume('step_into');resume('step_into');locals();dumpBreak();**/
})();

glob; /**bp:logJson("Stepping on default params - Split scope with eval in param")**/
(function() {
    function f1(a = 1, b = function () { 
        return eval("a"); /**bp:setFrame(1);locals()**/  
    }, c = b()) {
        var m = b();
        m; /**bp:locals()**/
    }
    f1();
})();

print("Pass");
