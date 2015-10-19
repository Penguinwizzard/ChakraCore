//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------

// switches: -loopinterpretcount:0
const w = 0;
var z = {};
z.__proto__ = this;
w;
for (p in [0]) {  }  // ASSERTION: (!(descriptor->Attributes & PropertyLetConstGlobal)) : SimpleDictionaryTypeHandler.cpp, line 2922
print("pass");
