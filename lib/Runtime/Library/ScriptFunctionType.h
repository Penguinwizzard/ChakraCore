//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    struct ScriptFunctionTypeExtra
    {
        ScriptFunctionTypeExtra* next;
        uint32 codePath;
        bool cleanedUpEntryPoint;
        CONTEXT ctx;
        PVOID stack[2048];
    };
    class ScriptFunctionType : public DynamicType
    {
    public:        
        static ScriptFunctionType * New(FunctionProxy * proxy, bool isShared);
        static DWORD GetEntryPointInfoOffset() { return offsetof(ScriptFunctionType, entryPointInfo); }
        ProxyEntryPointInfo * GetEntryPointInfo() const { return entryPointInfo; }
        void SetEntryPointInfo(ProxyEntryPointInfo * entryPointInfo, uint32 codePath, bool cleanedUpEntryPoint = false)
        {
            this->entryPointInfo = entryPointInfo;
            if (cleanedUpEntryPoint)
            {
                auto tmp = (ScriptFunctionTypeExtra*)malloc(sizeof(ScriptFunctionTypeExtra));
                auto e = this->extra;
                if (e == nullptr)
                {
                    this->extra = tmp;
                }
                else
                {
                    while (e->next != nullptr)e = e->next;
                    e->next = tmp;
                }
                tmp->next = nullptr;
                tmp->codePath = codePath;
                tmp->cleanedUpEntryPoint = cleanedUpEntryPoint;

#if _M_IX86
                // record stack data
                RtlCaptureContext(&tmp->ctx);
                auto len = (DWORD)((PNT_TIB)NtCurrentTeb())->StackBase - tmp->ctx.Esp;
                len = len > sizeof(tmp->stack) ? sizeof(tmp->stack) : len;
                memcpy(tmp->stack, (void*)tmp->ctx.Esp, len);
                tmp->ctx.Ebp = tmp->ctx.Ebp + (DWORD)tmp->stack - tmp->ctx.Esp;
                tmp->ctx.Esp = (DWORD)tmp->stack; // for .cxr switching to this state
#endif
            }
        }
    private:
        ScriptFunctionType(ScriptFunctionType * type);
        ScriptFunctionType(ScriptContext* scriptContext, RecyclableObject* prototype, 
            JavascriptMethod entryPoint, ProxyEntryPointInfo * entryPointInfo, DynamicTypeHandler * typeHandler, 
            bool isLocked, bool isShared);
        ProxyEntryPointInfo * entryPointInfo;
        ScriptFunctionTypeExtra* extra;

        friend class ScriptFunction;
        friend class JavascriptLibrary;
    };
};