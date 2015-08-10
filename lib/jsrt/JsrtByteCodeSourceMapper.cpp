//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "JsrtPch.h"
#include "JsrtByteCodeSourceMapper.h"

namespace Js
{
    // Helper function for converting a Unicode script to utf8.
    // If heapAlloc is true the returned buffer must be freed with HeapDelete.
    // Otherwise scriptContext must be provided and GCed object is returned.
    JsErrorCode ByteCodeSourceMapper::ScriptToUtf8(_When_(heapAlloc, _In_opt_) _When_(!heapAlloc, _In_) Js::ScriptContext *scriptContext, _In_z_ const wchar_t *script, _Outptr_result_buffer_(*utf8Length) utf8char_t **utf8Script, _Out_ size_t *utf8Length, _Out_ size_t *scriptLength, _Out_ size_t *utf8AllocLength, _In_ bool heapAlloc)
    {
        Assert(utf8Script != NULL);
        Assert(utf8Length != NULL);
        Assert(scriptLength != NULL);

        *utf8Script = NULL;
        *utf8Length = 0;
        *scriptLength = 0;

        if (utf8AllocLength != NULL)
        {
            *utf8AllocLength = 0;
        }

        size_t length = wcslen(script);
        if (length > UINT_MAX)
        {
            return JsErrorOutOfMemory;
        }

        size_t cbUtf8Buffer = (length + 1) * 3;
        if (cbUtf8Buffer > UINT_MAX)
        {
            return JsErrorOutOfMemory;
        }

        if (!heapAlloc)
        {
            Assert(scriptContext != NULL);
            *utf8Script = RecyclerNewArrayLeaf(scriptContext->GetRecycler(), utf8char_t, cbUtf8Buffer);
        }
        else
        {
            *utf8Script = HeapNewArray(utf8char_t, cbUtf8Buffer);
        }

        Assert(length < MAXLONG);
        *utf8Length = utf8::EncodeIntoAndNullTerminate(*utf8Script, script, static_cast<charcount_t>(length));
        *scriptLength = length;

        if (utf8AllocLength != NULL)
        {
            *utf8AllocLength = cbUtf8Buffer;
        }

        return JsNoError;
    }

    ByteCodeSourceMapper::ByteCodeSourceMapper(JsSerializedScriptLoadSourceCallback scriptLoadCallback, JsSerializedScriptUnloadCallback scriptUnloadCallback, JsSourceContext sourceContext) :
        m_refCount(1),        
        m_scriptLoadCallback(scriptLoadCallback),
        m_scriptUnloadCallback(scriptUnloadCallback),
        m_sourceContext(sourceContext),
        m_utf8Source(NULL),
        m_utf8AllocaLength(0)
    {
    }
  
    ByteCodeSourceMapper::~ByteCodeSourceMapper()  
    {
        NotifyUnloadScript();
    }
    
    // Calls the JsSerializedScriptLoadSourceCallback to retrieve the Unicode script from the caller.
    // It is converted to Utf8 and returned.
    // Expects to only be called once per instance.
    STDMETHODIMP ByteCodeSourceMapper::MapSourceCode(_Outptr_result_buffer_(*pdwSourceCodeSize) BYTE **sourceCode, _Out_ DWORD *pdwSourceCodeSize)
    {
        if (m_scriptLoadCallback == NULL)
        {
            // Only expect load once.
            return E_UNEXPECTED;
        }

        const wchar_t *source = NULL;
        size_t sourceLength = 0;
            
        utf8char_t *utf8Source = NULL;
        size_t utf8Length = 0;
        size_t utf8AllocLength = 0;

        if (!m_scriptLoadCallback(m_sourceContext, &source)) 
        {
            return E_NOT_SET;
        }

        JsErrorCode errorCode = ScriptToUtf8(NULL, source, &utf8Source, &utf8Length, &sourceLength, &utf8AllocLength, true);
        if (errorCode != JsNoError)
        {
            return HRESULT_FROM_WIN32(errorCode);
        }

        m_utf8Source = utf8Source;
        utf8Source = NULL;
        m_utf8AllocaLength = utf8AllocLength;

        *sourceCode = m_utf8Source;
        *pdwSourceCodeSize = utf8Length;

        m_scriptLoadCallback = NULL;

        return S_OK;
    }

    // Frees the Utf8 copy of the script source and informs the JsSerializedScriptUnloadCallback that 
    // the caller can clean up all script resources.
    void STDMETHODCALLTYPE ByteCodeSourceMapper::UnmapSourceCode()
    {
        NotifyUnloadScript();
    }

    // Helper for UnmapSourceCode which is called also at descruction to ensure the callback is
    // delivered to the caller.
    void ByteCodeSourceMapper::NotifyUnloadScript()
    {
        if (m_scriptUnloadCallback == NULL)
        {
            return;
        }

        m_scriptUnloadCallback(m_sourceContext);
        
        if (m_utf8Source != NULL)
        {
            HeapDeleteArray(m_utf8AllocaLength, m_utf8Source);
            m_utf8Source = NULL;
        }

        // Don't allow load or unload again after told to unload.
        m_scriptLoadCallback = NULL;
        m_scriptUnloadCallback = NULL;
        m_sourceContext = NULL;
    }

    // IUnknown
    STDMETHODIMP ByteCodeSourceMapper::QueryInterface(_In_ REFIID riid, _Outptr_ void ** ppvObject)  
    {  
        if (ppvObject == nullptr)  
        {  
            return E_POINTER;  
        }  
  
        if (IsEqualIID(riid, __uuidof(IUnknown)))  
        {  
            *ppvObject = static_cast<IActiveScriptByteCodeSource *>(this);  
            this->AddRef();  
            return S_OK;  
        }  
  
        if (IsEqualIID(riid, __uuidof(IActiveScriptByteCodeSource)))  
        {  
            *ppvObject = static_cast<IActiveScriptByteCodeSource *>(this);  
            this->AddRef();  
            return S_OK;  
        }

        return E_NOINTERFACE;  
    }  
  
    ULONG STDMETHODCALLTYPE ByteCodeSourceMapper::AddRef()  
    {  
        return InterlockedIncrement(&m_refCount);  
    }  
  
    ULONG STDMETHODCALLTYPE ByteCodeSourceMapper::Release()  
    {  
        ULONG result = InterlockedDecrement(&m_refCount);
        if (result == 0)  
        {  
            delete this;
        }

        return result;  
    }
}
