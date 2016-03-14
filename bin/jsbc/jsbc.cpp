//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "jsbc.h"
#include "Helpers.h"
#include "Base/ThreadContextTlsEntry.h"
#include "Base/ThreadBoundThreadContextManager.h"
#include "Parser.h"
#include "screrror.h"

// For converting from ANSI to UTF16
#ifndef _WIN32
#include <src/include/pal/utils.h>
#endif

void usage(const WCHAR* self)
{
    wprintf(
        _u("Usage: %s [-?|-bc|-pt] {JavaScript File}.js\n")
        _u("  -bc\n\tGenerates byte code\n")
        _u("  -pt\n\tGenerates parse tree\n"),
        self);
}

void GenerateByteCodeFor(WCHAR* /*jsFileName*/)
{
    // TODO: Implement this function
    wprintf(_u("This is not yet implemented"));
}

void GenerateParseTreeFor(WCHAR* jsFileName)
{
    LPCWSTR scriptContents = NULL;
    bool isUtf8 = true;
    LPCWSTR rawFileContents = NULL;
    UINT byteCount = 0;

    HRESULT hr = Helpers::LoadScriptFromFile(jsFileName, scriptContents, &isUtf8, &rawFileContents, &byteCount);

    if (FAILED(hr))
    {
        wprintf(_u("Failed to load script file. Error code is 0x%x"), hr);
        exit(1);
    }

    // Kind of wonky but we don't know the char count yet in the UTF8 case
    // For this tool, it probably doesn't matter
    UINT scriptCharCount = byteCount;
    if (!isUtf8)
    {
        scriptCharCount = wcslen(scriptContents);
    }

    ThreadContext* localThreadContext = ThreadBoundThreadContextManager::EnsureContextForCurrentThread();
    AutoPtr<ThreadContext> threadContext(localThreadContext);

    Js::ScriptContext* localScriptContext = Js::ScriptContext::New(threadContext);
    AutoPtr<Js::ScriptContext> scriptContext(localScriptContext);

    localScriptContext->Initialize();

    DWORD_PTR hostSourceCtx = (DWORD_PTR)-1;
    SourceContextInfo* sourceContextInfo = scriptContext->CreateSourceContextInfo(hostSourceCtx, jsFileName, wcslen(jsFileName), nullptr);
    SRCINFO si = {
        /* sourceContextInfo   */ sourceContextInfo,
        /* dlnHost             */ 0,
        /* ulColumnHost        */ 0,
        /* lnMinHost           */ 0,
        /* ichMinHost          */ 0,
        /* ichLimHost          */ static_cast<ULONG>(scriptCharCount), // OK to truncate since this is used to limit sourceText in debugDocument/compilation errors.
        /* ulCharOffset        */ 0,
        /* mod                 */ kmodGlobal,
        /* grfsi               */ 0
    }; 

    Parser parser(localScriptContext);
    hr = S_FALSE;
    ParseNodePtr ast = nullptr;
    CompileScriptException cse;
    Js::LocalFunctionId nextFunctionId = 0;

    if (isUtf8)
    {
        hr = parser.ParseUtf8Source(&ast, (LPCUTF8)scriptContents, byteCount, 0, &cse, &nextFunctionId, sourceContextInfo);
        if (FAILED(hr))
        {
            wprintf(_u("Error: Parsing file failede with HRESULT 0x%x"), hr);
        }
    }
    else
    {
        wprintf(_u("Error: Source is not utf8- unsupported for now\n"));
    }

    ast->Dump();
    
    if (rawFileContents != NULL)
    {
        free((void*) rawFileContents);
    }
    if (scriptContents != NULL)
    {
        free((void*)scriptContents);
    }
}

enum GenerateMode
{
    GenerateParseTree,
    GenerateByteCode
};

int __cdecl wmain(int argc, __in_ecount(argc) WCHAR* argv[])
{
    GenerateMode jsbcMode = GenerateMode::GenerateParseTree;    
    WCHAR* fileName = NULL;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (wcscmp(argv[i], _u("-?")) == 0)
            {
                usage(argv[0]);
                exit(1);
            }
            else if (wcscmp(argv[i], _u("-pt")) == 0)
            {
                jsbcMode = GenerateMode::GenerateParseTree;
            }
            else if (wcscmp(argv[i], _u("-bc")) == 0)
            {
                jsbcMode = GenerateMode::GenerateByteCode;
                break;
            }
            else 
            {
                wprintf(_u("Error: Unknown argument '%s'\n\n"), argv[i]);
                usage(argv[0]);
                exit(1);
            }
        }
        else
        {
            fileName = argv[i];
        }
    }

    if (fileName == NULL)
    {
        wprintf(_u("Error: Missing js filename argument\n\n"));
        usage(argv[0]);
        exit(1);
    }

    if (jsbcMode == GenerateMode::GenerateByteCode)
    {
        GenerateByteCodeFor(fileName);
    }
    else
    {
        GenerateParseTreeFor(fileName);
    }

    return 0;
}

#ifndef _WIN32
int main(int argc, char** argv)
{
    // Ignoring mem-alloc failures here as this is
    // simply a test tool. We can add more error checking
    // here later if desired.
    char16** args = new char16*[argc];
    for (int i = 0; i < argc; i++)
    {
        args[i] = UTIL_MBToWC_Alloc(argv[i], -1);
    }
    
    int ret = wmain(argc, args);

    for (int i = 0; i < argc; i++)
    {
        free(args[i]);
    }
    delete[] args;
    
    PAL_Shutdown();
    return ret;
}
#endif
