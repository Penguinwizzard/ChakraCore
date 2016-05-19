//c:\chakra\Build\VcBuild\bin\x64_debug\jshost.exe c:\work\bailout.js -checkmemoryleak- -off:simplejit -mic:1

function foo(a)
{
    return a+1;
}
print(foo(1));
print(foo(2));
print(foo("x")); // bailout