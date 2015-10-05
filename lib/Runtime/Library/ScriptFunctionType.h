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
        PVOID stack[64];
    };
    class ScriptFunctionType : public DynamicType
    {
    public:        
        static ScriptFunctionType * New(FunctionProxy * proxy, bool isShared);
        static DWORD GetEntryPointInfoOffset() { return offsetof(ScriptFunctionType, entryPointInfo); }
        ProxyEntryPointInfo * GetEntryPointInfo() const { return entryPointInfo; }
        void SetEntryPointInfo(ProxyEntryPointInfo * entryPointInfo, uint32 codePath) 
        {
            this->entryPointInfo = entryPointInfo; 
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
            CaptureStackBackTrace(0, 64, tmp->stack, 0);
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