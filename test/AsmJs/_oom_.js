print("hello");
var buffer = [];
for (var i = 0; ; ++i) {
    print(i);
    buffer[i] = new ArrayBuffer(1<<30);
}
print("world");
