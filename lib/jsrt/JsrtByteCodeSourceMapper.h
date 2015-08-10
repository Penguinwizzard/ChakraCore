//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#include "intermediatecode.h"

namespace Js
{
    // Implementation used to bridge the COM IActiveScriptByteCodeSource contract for 
    // DynamicSourceHolder with the Win32 Js*SerializeScriptWithCallback APIs.
    class ByteCodeSourceMapper : public IActiveScriptByteCodeSource
    {
    private:  
        volatile ULONG m_refCount;  
        
        JsSerializedScriptLoadSourceCallback m_scriptLoadCallback;
        JsSerializedScriptUnloadCallback m_scriptUnloadCallback;
        JsSourceContext m_sourceContext;
        
        utf8char_t *m_utf8Source;
        size_t m_utf8AllocaLength;

        void NotifyUnloadScript();
    public:

        ByteCodeSourceMapper(_In_ JsSerializedScriptLoadSourceCallback scriptLoadCallback, _In_ JsSerializedScriptUnloadCallback scriptUnloadCallback, _In_ JsSourceContext sourceContext);
        ~ByteCodeSourceMapper();

        // IUnknown
        STDMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void ** ppvObject);
        ULONG STDMETHODCALLTYPE AddRef();
        ULONG STDMETHODCALLTYPE Release();

        //IActiveScriptByteCodeSource
        STDMETHODIMP MapSourceCode(_Outptr_result_buffer_(*pdwSourceCodeSize) BYTE **sourceCode, _Out_ DWORD *pdwSourceCodeSize);

        void STDMETHODCALLTYPE UnmapSourceCode();

        static JsErrorCode ScriptToUtf8(_When_(heapAlloc, _In_opt_) _When_(!heapAlloc, _In_) Js::ScriptContext *scriptContext, _In_z_ const wchar_t *script, _Outptr_result_buffer_(*utf8Length) LPUTF8 *utf8Script, _Out_ size_t *utf8Length, _Out_ size_t *scriptLength, _Out_ size_t *utf8AllocLength = NULL, _In_ bool heapAlloc = false);
    };
}
