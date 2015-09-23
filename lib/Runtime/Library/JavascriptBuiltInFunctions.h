//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{


    class JavascriptBuiltInFunction
    {
    public:
        enum BuiltInEnum
        {
#define BUILTIN(c, n, e, i) c ## _ ## n,
#include "JavascriptBuiltInFunctionList.h"
#undef BUILTIN
#ifdef ENABLE_DOM_FAST_PATH
            // These two are for backend inlining of FTL DOM slot getter/setter
            // they are not externally exposed like other javascriptlibrary builtins.
            DOMFastPathGetter,
            DOMFastPathSetter,
#endif
            MaxBuiltInEnum
        };
        static FunctionInfo * GetFunctionInfo(Js::LocalFunctionId builtinId);
        static bool CanChangeEntryPoint(Js::LocalFunctionId builtInId);

        static bool IsValidId(Js::LocalFunctionId builtInId);

    private:
        static FunctionInfo * builtInFunctionInfo[MaxBuiltInEnum];
    };
};