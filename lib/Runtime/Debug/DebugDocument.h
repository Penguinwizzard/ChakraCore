//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

struct StatementSpan
{
    long ich;
    long cch;
};

// A Document in Engine means a file, eval code or new function code. For each of these there is a Utf8SourceInfo
// This class represents a DebugDocument corresponding to a Utf8SourceInfo
// Debug operations like adding/removing breakpoints related to a document are handled by this class.

namespace Js
{
    class DebugDocument
    {
    public:
        DebugDocument(Utf8SourceInfo* utf8SourceInfo, Js::FunctionBody* functionBody);
        ~DebugDocument();
        virtual void CloseDocument();

        HRESULT SetBreakPoint(long ibos, BREAKPOINT_STATE bps);
        void RemoveBreakpointProbe(BreakpointProbe *probe);
        void ClearAllBreakPoints(void);

        BOOL GetStatementSpan(long ibos, StatementSpan* pBos);
        BOOL GetStatementLocation(long ibos, StatementLocation* plocation);

        // ToDo (SAagarwa) : Fix for JsRT Debugging, for PDM this is IScriptDebugDocuemntText*
        virtual bool HasDocumentText() const
        {
            Assert(false);
            return false;
        }
        virtual void* GetDocumentText() const
        {
            Assert(false);
            return nullptr;
        };

        Js::FunctionBody * GetFunctionBodyAt(long ibos);

    private:
        Utf8SourceInfo* utf8SourceInfo;
        RecyclerRootPtr<Js::FunctionBody> functionBody;
        BreakpointProbeList* m_breakpointList;

        BreakpointProbeList* NewBreakpointList(ArenaAllocator* arena);
        BreakpointProbeList* GetBreakpointList();

        BOOL HasLineBreak(long _start, long _end);
    };
}