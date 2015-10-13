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
        void* address;
        void* cleanedUpEntryPoint;
        FunctionEntryPointInfo* oldEntryPointInfo;
        FunctionEntryPointInfo* oldEntryPointInfoData;
        FunctionEntryPointInfo* newEntryPointInfo;
        FunctionEntryPointInfo* newEntryPointInfoData;
        FunctionEntryPointInfo* simpleJitInfoOnFB;
        FunctionEntryPointInfo* simpleJitInfoOnFBData;
        FunctionBody* fbCopy;
        FunctionBody* fbCopyData;
        PVOID stack[16];
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
            memset(tmp, 0, sizeof(ScriptFunctionTypeExtra));
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
            tmp->address = address;
            tmp->oldEntryPointInfo = (FunctionEntryPointInfo*)(void*)this->entryPointInfo;
            if (tmp->oldEntryPointInfo) 
            {
                tmp->oldEntryPointInfoData = (FunctionEntryPointInfo*)malloc(sizeof(FunctionEntryPointInfo));
                memset(tmp->oldEntryPointInfoData, 0, sizeof(FunctionEntryPointInfo));
                memcpy(tmp->oldEntryPointInfoData, tmp->oldEntryPointInfo, 
                    tmp->oldEntryPointInfo->IsFunctionEntryPointInfo()?sizeof(FunctionEntryPointInfo): sizeof(ProxyEntryPointInfo));
            }
            tmp->newEntryPointInfo = (FunctionEntryPointInfo*)entryPointInfo;
            if (tmp->newEntryPointInfo) 
            {
                tmp->newEntryPointInfoData = (FunctionEntryPointInfo*)malloc(sizeof(FunctionEntryPointInfo));
                memset(tmp->newEntryPointInfoData, 0, sizeof(FunctionEntryPointInfo));
                memcpy(tmp->newEntryPointInfoData, tmp->newEntryPointInfo, 
                    tmp->newEntryPointInfo->IsFunctionEntryPointInfo() ? sizeof(FunctionEntryPointInfo) : sizeof(ProxyEntryPointInfo));
            }
            
            if (this->entryPointInfo->IsFunctionEntryPointInfo()) 
            {
                if (((FunctionEntryPointInfo*)(void*)this->entryPointInfo)->functionProxy && ((FunctionEntryPointInfo*)(void*)this->entryPointInfo)->functionProxy->IsFunctionBody())
                {
                    tmp->simpleJitInfoOnFB = ((FunctionEntryPointInfo*)(void*)this->entryPointInfo)->GetFunctionBody()->GetSimpleJitEntryPointInfo();
                    if (tmp->simpleJitInfoOnFB)
                    {
                        tmp->simpleJitInfoOnFBData = (FunctionEntryPointInfo*)malloc(sizeof(FunctionEntryPointInfo));
                        memset(tmp->simpleJitInfoOnFBData, 0, sizeof(FunctionEntryPointInfo));
                        memcpy(tmp->simpleJitInfoOnFBData, tmp->simpleJitInfoOnFB, 
                            tmp->simpleJitInfoOnFB->IsFunctionEntryPointInfo() ? sizeof(FunctionEntryPointInfo) : sizeof(ProxyEntryPointInfo));
                    }
                    tmp->fbCopy = ((FunctionEntryPointInfo*)(void*)this->entryPointInfo)->GetFunctionBody();
                    if (tmp->fbCopy) 
                    {
                        tmp->fbCopyData = (FunctionBody*)malloc(sizeof(FunctionBody));
                        memcpy(tmp->fbCopyData, ((FunctionEntryPointInfo*)(void*)this->entryPointInfo)->GetFunctionBody(), sizeof(FunctionBody));
                    }
                }
            }
            tmp->cleanedUpEntryPoint = cleanedUpEntryPoint;
            
            CaptureStackBackTrace(0, 16, tmp->stack, 0);

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
            //end instrumentation

            this->entryPointInfo = entryPointInfo;
        }
    private:
        ScriptFunctionType(ScriptFunctionType * type);
        ScriptFunctionType(ScriptContext* scriptContext, RecyclableObject* prototype, 
            JavascriptMethod entryPoint, ProxyEntryPointInfo * entryPointInfo, DynamicTypeHandler * typeHandler, 
            bool isLocked, bool isShared);
        PointerTracker<ProxyEntryPointInfo *, 0xdddddddd> entryPointInfo;
        ScriptFunctionTypeExtra* extra;

        friend class ScriptFunction;
        friend class JavascriptLibrary;
    };
};