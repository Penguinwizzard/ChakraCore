//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class BreakpointProbe : public Probe
    {
        int characterOffset;
        int byteOffset;
        DebugDocument* debugDocument;
        FunctionBody* functionBody;

    public:
        BreakpointProbe(DebugDocument* debugDocument, StatementLocation &statement);

        virtual bool Install(ScriptContext* pScriptContext);
        virtual bool Uninstall(ScriptContext* pScriptContext);
        virtual bool CanHalt(InterpreterHaltState* pHaltState);
        virtual void DispatchHalt(InterpreterHaltState* pHaltState);
        virtual void CleanupHalt();

        bool Matches(FunctionBody* _pBody, int characterPosition);
    };

    typedef JsUtil::List<BreakpointProbe*, ArenaAllocator> BreakpointProbeList;
}