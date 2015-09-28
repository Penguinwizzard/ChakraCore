// switches: -loopinterpretcount:0
const w = 0;
var z = {};
z.__proto__ = this;
w;
for (p in [0]) {  }  // ASSERTION: (!(descriptor->Attributes & PropertyLetConstGlobal)) : SimpleDictionaryTypeHandler.cpp, line 2922
print("pass");
