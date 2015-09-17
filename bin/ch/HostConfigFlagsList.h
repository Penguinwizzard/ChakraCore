//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#ifdef FLAG
FLAG(BSTR, Serialized,                    "If source is UTF8, deserializes from bytecode file", NULL)
FLAG(BSTR, GenerateLibraryByteCodeHeader, "Generate bytecode header file from library code", NULL)
#undef FLAG
#endif
