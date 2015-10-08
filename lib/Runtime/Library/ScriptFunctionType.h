//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

namespace Js
{
    struct StackData
    {
        CONTEXT ctx;
        PVOID stack[2048];
    };

    struct ScriptFunctionTypeExtra
    {
        ScriptFunctionTypeExtra* next;
        uint32 codePath;
        ProxyEntryPointInfo* oldEntryPointInfo;
        ProxyEntryPointInfo* newEntryPointInfo;
        void* address;
        void* nativeAddr;
        void* cleanedUpEntryPoint;
        char fbCopy[sizeof(FunctionBody)];
        PVOID stack[32];
        StackData* stackData;
    };
    class ScriptFunctionType : public DynamicType
    {
    public:        
        static ScriptFunctionType * New(FunctionProxy * proxy, bool isShared);
        static DWORD GetEntryPointInfoOffset() { return offsetof(ScriptFunctionType, entryPointInfo); }
        ProxyEntryPointInfo * GetEntryPointInfo() const { return entryPointInfo; }
        void SetEntryPointInfo(ProxyEntryPointInfo * entryPointInfo, uint32 codePath, void* cleanedUpEntryPoint = nullptr, void* address = nullptr)
        {          
            auto tmp = (ScriptFunctionTypeExtra*)malloc(sizeof(ScriptFunctionTypeExtra));
            memset(tmp, 0, sizeof(tmp));
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
            tmp->oldEntryPointInfo = this->entryPointInfo;
            tmp->newEntryPointInfo = entryPointInfo;
            tmp->address = address;
            if (this->entryPointInfo->IsFunctionEntryPointInfo()) 
            {
                tmp->nativeAddr = (void*)((Js::FunctionEntryPointInfo*)this->entryPointInfo)->GetNativeAddress();
                if (((Js::FunctionEntryPointInfo*)this->entryPointInfo)->functionProxy && ((Js::FunctionEntryPointInfo*)this->entryPointInfo)->functionProxy->IsFunctionBody())
                {
                    memcpy(&tmp->fbCopy, ((Js::FunctionEntryPointInfo*)this->entryPointInfo)->GetFunctionBody(), sizeof(FunctionBody));
                }
            }
            tmp->cleanedUpEntryPoint = cleanedUpEntryPoint;
            
            CaptureStackBackTrace(0, 32, tmp->stack, 0);

            //if (this->entryPointInfo->IsFunctionEntryPointInfo())
            //{
            //    if (((Js::FunctionEntryPointInfo*)this->entryPointInfo)->GetNativeAddress() == (DWORD_PTR)entryPointInfo->address) 
            //    {
            //        DebugBreak();
            //    }
            //}

            if (cleanedUpEntryPoint)
            {
#if _M_IX86
                auto&stackData = tmp->stackData = (StackData*)malloc(sizeof(StackData));
                RtlCaptureContext(&stackData->ctx);
                auto len = (DWORD)((PNT_TIB)NtCurrentTeb())->StackBase - stackData->ctx.Esp;
                len = len > sizeof(stackData->stack) ? sizeof(stackData->stack) : len;
                memcpy(stackData->stack, (void*)stackData->ctx.Esp, len);
                stackData->ctx.Ebp = stackData->ctx.Ebp + (DWORD)stackData->stack - stackData->ctx.Esp;
                stackData->ctx.Esp = (DWORD)stackData->stack; // for .cxr switching to this state
#endif
            }
            else
            {
                tmp->stackData = nullptr;
            }

            this->entryPointInfo = entryPointInfo;
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