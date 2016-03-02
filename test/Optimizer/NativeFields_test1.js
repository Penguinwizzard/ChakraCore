function test0(o0, o1) {
    var v0 = o0.v;
    var f1 = getf(o1);
    var f0 = o0.f;
    return [v0, f0, f1];
}
function getf(o) {
    return o.f;
}
var o = { v: null, f: 0 };
o = { v: null, f: 1 };
WScript.Echo(test0(o, o));
o = { v: null, f: 1 };
var o2 = { v: null, f: null };
getf(o2);
WScript.Echo(test0(o, o));
