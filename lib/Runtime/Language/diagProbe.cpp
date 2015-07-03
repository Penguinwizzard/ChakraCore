//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------
#include "stdafx.h"

#define InvalidScriptId 0xFFFFFFFF

namespace Js
{

    InterpreterHaltState::InterpreterHaltState(StopType _stopType, const FunctionBody* _executingFunction, MutationBreakpoint* _activeMutationBP/*= nullptr*/) :
        stopType(_stopType),
        executingFunction(_executingFunction),
        topFrame(nullptr),
        framePointers(nullptr),
        referencedDiagnosticArena(nullptr),
        exceptionObject(nullptr),
        stringBuilder(nullptr),
        activeMutationBP(_activeMutationBP)
    {
        Assert(executingFunction || (stopType == STOP_EXCEPTIONTHROW || stopType == STOP_MUTATIONBREAKPOINT));
    }

    FunctionBody* InterpreterHaltState::GetFunction()
    {
        Assert(IsValid());
        return this->topFrame->GetFunction();
    }

    int InterpreterHaltState::GetCurrentOffset()
    {
        Assert(IsValid());
        return this->topFrame->GetByteCodeOffset();
    }

    void InterpreterHaltState::SetCurrentOffset(int offset)
    {
        Assert(IsValid());
        if (this->topFrame->IsInterpreterFrame())
        {
            // For interpreter frames, actual scenarios we need changed offset are: set next in topmost frame, ignore exception.
            // For throw exception we don't need it, but it doesn't hurt because interpreter will ignore the offset
            // and rather just throw the exception.
            this->topFrame->AsInterpreterFrame()->GetReader()->SetCurrentOffset(offset);
        }
        else
        {
            // For native frames, the only scenario we need to record changed offset is when we ignore exception.
            if (this->exceptionObject && this->exceptionObject->IsDebuggerSkip())
            {
                this->exceptionObject->SetByteCodeOffsetAfterDebuggerSkip(offset);
            }
        }
    }

    bool InterpreterHaltState::IsValid() const
    {
        // "executingFunction == nullptr" when dispatching exception or mutation bp.
        return topFrame && (topFrame->GetFunction() == executingFunction || executingFunction == nullptr);
    }


    StepController::StepController()
        : stepType(STEP_NONE),
        byteOffset(0),
        statementMap(NULL),
        frameCountWhenSet(0),
        frameAddrWhenSet((size_t)-1),
        stepCompleteOnInlineBreakpoint(false),
        pActivatedContext(NULL),
        scriptIdWhenSet(InvalidScriptId),
        returnedValueRecordingDepth(0),
        returnedValueList(nullptr)
    {
    }

    bool StepController::IsActive()
    {
        return stepType != STEP_NONE;
    }

    void StepController::Activate(StepType stepType, InterpreterHaltState* haltState)
    {
        this->stepType = stepType;
        this->byteOffset = haltState->GetCurrentOffset();
        this->pActivatedContext = haltState->framePointers->Peek()->GetScriptContext();
        Assert(this->pActivatedContext);

        Js::FunctionBody* functionBody = haltState->GetFunction();

        this->body.Root(functionBody, this->pActivatedContext->GetRecycler());
        this->statementMap = body->GetMatchingStatementMapFromByteCode(byteOffset, false);
        this->frameCountWhenSet = haltState->framePointers->Count();

        if (stepType != STEP_DOCUMENT)
        {
            this->frameAddrWhenSet = (size_t)haltState->framePointers->Peek(0)->GetStackAddress();
        }
        else
        {
            // for doc mode, do not bail out automatically on frame changes
            this->frameAddrWhenSet = (size_t)-1;
        }

        this->scriptIdWhenSet = GetScriptId(functionBody);

        if (this->returnedValueList == nullptr)
        {
            this->returnedValueList = JsUtil::List<ReturnedValue*>::New(this->pActivatedContext->GetRecycler());
            this->pActivatedContext->GetThreadContext()->SetReturnedValueList(this->returnedValueList);
        }
    }

    void StepController::AddToReturnedValueContainer(Js::Var returnValue, Js::JavascriptFunction * function, bool isValueOfReturnStatement)
    {
        if (this->pActivatedContext != nullptr) // This will be null when we execute scripts when on break.
        {
            ReturnedValue *valuePair = RecyclerNew(pActivatedContext->GetRecycler(), ReturnedValue, returnValue, function, isValueOfReturnStatement);
            this->returnedValueList->Add(valuePair);
        }
    }

    void StepController::AddReturnToReturnedValueContainer()
    {
        AddToReturnedValueContainer(nullptr/*returnValue*/, nullptr/*function*/, true/*isValueOfReturnStatement*/);
    }

    void StepController::StartRecordingCall()
    {
        returnedValueRecordingDepth++;
    }

    void StepController::EndRecordingCall(Js::Var returnValue, Js::JavascriptFunction * function)
    {
        if (IsActive() && this->pActivatedContext != nullptr && returnValue != nullptr)
        {
            if (this->pActivatedContext->GetThreadContext()->Diagnostics->isAtDispatchHalt)
            {
                // OS bug 3050302 - Keeping this FatalError for finding other issues where we can record when we are at break
                Js::Throw::FatalInternalError();
            }
            bool isStepOut = stepType == STEP_OUT || stepType == STEP_DOCUMENT;

            // Record when :
            // If step-out/document : we need to record calls only which are already on the stack, that means the recording-depth is zero or negative.
            // if not step-out (step-in and step-over). only for those, which are called from the current call-site or the ones as if we step-out
            if ((!isStepOut && returnedValueRecordingDepth <= 1) || (isStepOut && returnedValueRecordingDepth <= 0))
            {
                // if we are step_document, we should be removing whatever we have collected so-far,
                // since they belong to the current document which is a library code
                if (stepType == STEP_DOCUMENT)
                {
                    this->returnedValueList->ClearAndZero();
                }

                AddToReturnedValueContainer(returnValue, function, false/*isValueOfReturnStatement*/);
            }
        }
        returnedValueRecordingDepth--;
    }

    void StepController::ResetReturnedValueList() 
    {
        returnedValueRecordingDepth = 0;
        if (this->returnedValueList != nullptr)
        {
            this->returnedValueList->ClearAndZero();
        }
    }

    void StepController::HandleResumeAction(Js::InterpreterHaltState* haltState, BREAKRESUMEACTION resumeAction)
    {
        ResetReturnedValueList();
        switch (resumeAction)
        {
        case BREAKRESUMEACTION_STEP_INTO:
            Activate(Js::STEP_IN, haltState);
            break;
        case BREAKRESUMEACTION_STEP_OVER:
            Activate(Js::STEP_OVER, haltState);
            break;
        case BREAKRESUMEACTION_STEP_OUT:
            Activate(Js::STEP_OUT, haltState);
            break;
        case BREAKRESUMEACTION_STEP_DOCUMENT:
            Activate(Js::STEP_DOCUMENT, haltState);
            break;
        }
    }


    void StepController::Deactivate(InterpreterHaltState* haltState /*=nullptr*/)
    {
        // If we are deactivating the step controller during ProbeContainer close or attach/detach we should clear return value list
        // If we break other than step -> clear the list.
        // If we step in and we land on different function (we are in recording phase the current function) -> clear the list
        if ((haltState == nullptr) || (haltState->stopType != Js::STOP_STEPCOMPLETE || this->stepType == STEP_IN && this->returnedValueRecordingDepth > 0))
        {
            ResetReturnedValueList();
        }

        if (this->body)
        {
            Assert(this->pActivatedContext);
            this->body.Unroot(this->pActivatedContext->GetRecycler());
        }
        this->pActivatedContext = NULL;
        stepType = STEP_NONE;
        byteOffset = Js::Constants::NoByteCodeOffset;
        statementMap = NULL;
        
        frameCountWhenSet = 0;
        scriptIdWhenSet = InvalidScriptId;
        frameAddrWhenSet = (size_t)-1;
    }

    bool StepController::IsStepComplete_AllowingFalsePositives(InterpreterStackFrame * stackFrame)
    {
        Assert(stackFrame);
        if (stepType == STEP_IN)
        {
            return true;
        }
        else if (stepType == STEP_DOCUMENT)
        {
            Assert(stackFrame->GetFunctionBody());
            return GetScriptId(stackFrame->GetFunctionBody()) != this->scriptIdWhenSet;
        }

        // A STEP_OUT or a STEP_OVER has not completed if we are currently deeper on the callstack.
        return this->frameAddrWhenSet <= stackFrame->GetStackAddress();
    }

    bool StepController::IsStepComplete(InterpreterHaltState* haltState, HaltCallback * haltCallback, OpCode originalOpcode)
    {
        int currentFrameCount = haltState->framePointers->Count();
        AssertMsg(currentFrameCount > 0, "In IsStepComplete we must have at least one frame.");

        FunctionBody* body = haltState->framePointers->Peek()->GetJavascriptFunction()->GetFunctionBody();
        bool canPossiblyHalt = haltCallback->CanHalt(haltState);

        OUTPUT_TRACE(Js::DebuggerPhase, L"StepController::IsStepComplete(): stepType = %d ", stepType);

        uint scriptId = GetScriptId(body);
        AssertMsg(scriptId != InvalidScriptId, "scriptId cannot be 'invalid-reserved'");

        int byteOffset = haltState->GetCurrentOffset();
        bool fCanHalt = false;

        if (this->frameCountWhenSet > currentFrameCount && STEP_DOCUMENT != stepType)
        {
            // all steps match once the frame they started on has popped.
            fCanHalt = canPossiblyHalt;
        }
        else if (STEP_DOCUMENT == stepType)
        {
            OUTPUT_TRACE(Js::DebuggerPhase, L"StepController::IsStepComplete(): docId when set=%d, currentDocId = %d, can Halt = %d, will halt = %d ", this->scriptIdWhenSet, scriptId, canPossiblyHalt, fCanHalt);
            fCanHalt = (scriptId != this->scriptIdWhenSet) && canPossiblyHalt;
        }
        else if (STEP_IN != stepType && this->frameCountWhenSet < currentFrameCount)
        {
            // Only step into allows the stack to be deeper
            OUTPUT_TRACE(Js::DebuggerPhase, L"StepController::IsStepComplete(stepType = %d) returning false ", stepType);
            return false;
        }
        else if (STEP_OUT == stepType) 
        {
            fCanHalt = this->frameCountWhenSet > currentFrameCount && canPossiblyHalt;
        }
        else if (nullptr != this->statementMap && this->statementMap->isSubexpression && STEP_IN != stepType)
        {
            // Only step into started from subexpression is allowed to stop on another subexpression
            Js::FunctionBody* pCurrentFuncBody = haltState->GetFunction();
            Js::FunctionBody::StatementMap* map = pCurrentFuncBody->GetMatchingStatementMapFromByteCode(byteOffset, false);
            if (nullptr != map && map->isSubexpression)    // Execute remaining Subexpressions
            {
                fCanHalt = false;
            }
            else
            {
                Js::FunctionBody::StatementMap* outerMap = pCurrentFuncBody->GetMatchingStatementMapFromByteCode(this->statementMap->byteCodeSpan.begin, true);
                if (nullptr != outerMap && map == outerMap) // Execute the rest of current regular statement
                {
                    fCanHalt = false;
                }
                else
                {
                    fCanHalt = canPossiblyHalt;
                }
            }
        }
        else
        {
            // Match if we are no longer on the original statement.  Stepping means move off current statement.
            if (body != this->body || NULL == this->statementMap ||
                !this->statementMap->byteCodeSpan.Includes(byteOffset))
            {
                fCanHalt = canPossiblyHalt;
            }
        }
        // At this point we are verifying of global return opcode.
        // The global returns are alway added as a zero range begin with zero.

        if (fCanHalt && originalOpcode == OpCode::Ret)
        {
            Js::FunctionBody* pCurrentFuncBody = haltState->GetFunction();
            Js::FunctionBody::StatementMap* map = pCurrentFuncBody->GetMatchingStatementMapFromByteCode(byteOffset, true);

            fCanHalt = !FunctionBody::IsDummyGlobalRetStatement(&map->sourceSpan);
            if (fCanHalt)
            {
                // We are breaking at last line of function, imagine '}'
                AddReturnToReturnedValueContainer();
            }
        }

        OUTPUT_TRACE(Js::DebuggerPhase, L"StepController::IsStepComplete(stepType = %d) returning %d ", stepType, fCanHalt);
        return fCanHalt;
    }

    bool StepController::ContinueFromInlineBreakpoint()
    {
        bool ret = stepCompleteOnInlineBreakpoint;
        stepCompleteOnInlineBreakpoint = false;
        return ret;
    }

    uint StepController::GetScriptId(_In_ FunctionBody* body)
    {
        // safe value
        uint retValue = BuiltInFunctionsScriptId;

        if (body != nullptr)
        {
            // FYI - Different script blocks within a html page will have different source Info ids even though they have the same backing file.
            // It might imply we notify the debugger a bit more than needed - thus can be TODO for performance improvements of the Just-My-Code
            // or step to next document boundary mode.
            AssertMsg(body->GetUtf8SourceInfo() != nullptr, "body->GetUtf8SourceInfo() == nullptr");
            retValue = body->GetUtf8SourceInfo()->GetSourceInfoId();
        }

        return retValue;
    }

    AsyncBreakController::AsyncBreakController()
        : haltCallback(NULL)
    {
    }

    void AsyncBreakController::Activate(HaltCallback* haltCallback)
    {
        InterlockedExchangePointer((PVOID*)&this->haltCallback, haltCallback);
    }

    void AsyncBreakController::Deactivate()
    {
        InterlockedExchangePointer((PVOID*)&this->haltCallback, NULL);
    }

    bool AsyncBreakController::IsBreak()
    {
        return haltCallback != NULL;
    }

    bool AsyncBreakController::IsAtStoppingLocation(InterpreterHaltState* haltState)
    {
        HaltCallback* callback;
        InterlockedExchangePointer((PVOID*)&callback, this->haltCallback);

        if (callback)
        {
            return callback->CanHalt(haltState);
        }
        return false;
    }

    void AsyncBreakController::DispatchAndReset(InterpreterHaltState* haltState)
    {
        HaltCallback* callback;
        InterlockedExchangePointer((PVOID*)&callback, this->haltCallback);
        Deactivate();
        if (callback)
        {
            callback->DispatchHalt(haltState);
        }
    }

    ProbeManager::ProbeManager(ThreadContext* _pThreadContext)
        : pCurrentInterpreterLocation(NULL),
          languageServiceEnabled(false),
          secondaryCurrentSourceContext(0),
          debugSessionNumber(0),
          pThreadContext(_pThreadContext),
#if DBG
          dispatchHaltFrameAddress(nullptr),
#endif
          isAtDispatchHalt(false),
          mutationNewValuePid(Js::Constants::NoProperty),
          mutationPropertyNamePid(Js::Constants::NoProperty),
          mutationTypePid(Js::Constants::NoProperty)
    {
    }

    ProbeManager::~ProbeManager()
    {
        if (this->pConsoleScope)
        {
            this->pConsoleScope.Unroot(this->pThreadContext->GetRecycler());
        }
#if DBG
        this->pThreadContext->EnsureNoReturnedValueList();
#endif
    }

    ReferencedArenaAdapter* ProbeManager::GetDiagnosticArena()
    {
        if (pCurrentInterpreterLocation)
        {
            return pCurrentInterpreterLocation->referencedDiagnosticArena;
        }
        return NULL;
    }
    
    DWORD_PTR ProbeManager::AllocateSecondaryHostSourceContext()
    {

        // The context is not valid, use the secondary context for identify the function body for further use.

        Assert(secondaryCurrentSourceContext < ULONG_MAX);
        return secondaryCurrentSourceContext++;               
    }

    void ProbeManager::SetCurrentInterpreterLocation(InterpreterHaltState* pHaltState)
    {
        Assert(pHaltState);
        Assert(!pCurrentInterpreterLocation);

        pCurrentInterpreterLocation = pHaltState;

        AutoAllocatorObjectPtr<ArenaAllocator, HeapAllocator> pDiagArena(HeapNew(ArenaAllocator, L"DiagHaltState", this->pThreadContext->GetPageAllocator(), Js::Throw::OutOfMemory), &HeapAllocator::Instance);
        AutoAllocatorObjectPtr<ReferencedArenaAdapter, HeapAllocator> referencedDiagnosticArena(HeapNew(ReferencedArenaAdapter, pDiagArena), &HeapAllocator::Instance);
        pCurrentInterpreterLocation->referencedDiagnosticArena = referencedDiagnosticArena;
        
        /* TODO-OOM: check for null return */
        pThreadContext->GetRecycler()->RegisterExternalGuestArena(pDiagArena);
        debugSessionNumber++;

        pDiagArena.Detach();
        referencedDiagnosticArena.Detach();
    }

    void ProbeManager::UnsetCurrentInterpreterLocation()
    {
        Assert(pCurrentInterpreterLocation);

        if (pCurrentInterpreterLocation)
        {
            // pCurrentInterpreterLocation->referencedDiagnosticArena could be null if we ran out of memory during SetCurrentInterpreterLocation
            if (pCurrentInterpreterLocation->referencedDiagnosticArena)
            {
                pThreadContext->GetRecycler()->UnregisterExternalGuestArena(pCurrentInterpreterLocation->referencedDiagnosticArena->Arena());
                pCurrentInterpreterLocation->referencedDiagnosticArena->DeleteArena();
                pCurrentInterpreterLocation->referencedDiagnosticArena->Release();
            }

            pCurrentInterpreterLocation = NULL;
        }
    }

    MutationBreakpoint* ProbeManager::GetActiveMutationBreakpoint() const
    {
        Assert(this->pCurrentInterpreterLocation);
        return this->pCurrentInterpreterLocation->activeMutationBP;
    }

    DynamicObject* ProbeManager::GetConsoleScope(ScriptContext* scriptContext)
    {
        Assert(scriptContext);

        if (!this->pConsoleScope)
        {
            this->pConsoleScope.Root(scriptContext->GetLibrary()->CreateConsoleScopeActivationObject(), this->pThreadContext->GetRecycler());
        }

        return (DynamicObject*)CrossSite::MarshalVar(scriptContext, (Var)this->pConsoleScope);
    }

    FrameDisplay *ProbeManager::GetFrameDisplay(ScriptContext* scriptContext, DynamicObject* scopeAtZero, DynamicObject* scopeAtOne, bool addGlobalThisAtScopeTwo)
    {
        /*
        The scope chain for console eval looks like
        dummy empty object - new vars, let, consts, functions get added here
        Active scope object containing all globals visible at this break (if at break)
        Global this object so that existing properties are updated here
        Console-1 Scope - all new globals will go here (like x = 1;)
        NullFrameDisplay
        */

        FrameDisplay* environment = JavascriptOperators::OP_LdFrameDisplay(this->GetConsoleScope(scriptContext), const_cast<FrameDisplay *>(&NullFrameDisplay), scriptContext);
        
        if (addGlobalThisAtScopeTwo)
        {
            environment = JavascriptOperators::OP_LdFrameDisplay(scriptContext->GetGlobalObject()->ToThis(), environment, scriptContext);
        }
        
        if (scopeAtOne != nullptr)
        {
            environment = JavascriptOperators::OP_LdFrameDisplay((Var)scopeAtOne, environment, scriptContext);
        }

        environment = JavascriptOperators::OP_LdFrameDisplay((Var)scopeAtZero, environment, scriptContext);
        return environment;
    }

    void ProbeManager::UpdateConsoleScope(DynamicObject* copyFromScope, ScriptContext* scriptContext)
    {
        Assert(copyFromScope != nullptr);
        DynamicObject* consoleScope = this->GetConsoleScope(scriptContext);
        Js::RecyclableObject* recyclableObject = Js::RecyclableObject::FromVar(copyFromScope);

        ulong newPropCount = recyclableObject->GetPropertyCount();
        for (ulong i = 0; i < newPropCount; i++)
        {
            Js::PropertyId propertyId = recyclableObject->GetPropertyId((Js::PropertyIndex)i);
            // For deleted properties we won't have a property id
            if (propertyId != Js::Constants::NoProperty)
            {
                Js::PropertyValueInfo propertyValueInfo;
                Var propertyValue;
                BOOL gotPropertyValue = recyclableObject->GetProperty(recyclableObject, propertyId, &propertyValue, &propertyValueInfo, scriptContext);
                AssertMsg(gotPropertyValue, "ProbeManager::UpdateConsoleScope Should have got valid value?");

                OUTPUT_TRACE(Js::ConsoleScopePhase, L"Adding property '%s'\n", scriptContext->GetPropertyName(propertyId)->GetBuffer());

                BOOL updateSuccess = consoleScope->SetPropertyWithAttributes(propertyId, propertyValue, propertyValueInfo.GetAttributes(), &propertyValueInfo);
                AssertMsg(updateSuccess, "ProbeManager::UpdateConsoleScope Unable to update property value. Am I missing a scenario?");
            }
        }

        OUTPUT_TRACE(Js::ConsoleScopePhase, L"Number of properties on console scope object after update are %d\n", consoleScope->GetPropertyCount());
    }
#if DBG
    void ProbeManager::ValidateDebugAPICall()
    {
        Js::JavascriptStackWalker walker(this->pThreadContext->GetScriptEntryExit()->scriptContext);
        Js::JavascriptFunction* javascriptFunction = nullptr;
        if (walker.GetCaller(&javascriptFunction))
        {
            if (javascriptFunction != nullptr)
            {
                void *topJsFrameAddr = (void *)walker.GetCurrentArgv();
                Assert(this->dispatchHaltFrameAddress != nullptr);
                if (topJsFrameAddr < this->dispatchHaltFrameAddress)
                {
                    // we found the script frame after the break mode.
                    AssertMsg(false, "There are JavaScript frames between current API and dispatch halt");
                }
            }
        }
    }
#endif
}
