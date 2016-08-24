//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "Backend.h"

///----------------------------------------------------------------------------
///
/// Encoder::Encode
///
///     Main entrypoint of encoder.  Encode each IR instruction into the
///     appropriate machine encoding.
///
///----------------------------------------------------------------------------
void
Encoder::Encode()
{
    NoRecoverMemoryArenaAllocator localAlloc(_u("BE-Encoder"), m_func->m_alloc->GetPageAllocator(), Js::Throw::OutOfMemory);
    m_tempAlloc = &localAlloc;

    uint32 instrCount = m_func->GetInstrCount();
    size_t totalJmpTableSizeInBytes = 0;

    JmpTableList * jumpTableListForSwitchStatement = nullptr;

    m_encoderMD.Init(this);
    m_encodeBufferSize = UInt32Math::Mul(instrCount, MachMaxInstrSize);
    m_encodeBufferSize += m_func->m_totalJumpTableSizeInBytesForSwitchStatements;
    m_encodeBuffer = AnewArray(m_tempAlloc, BYTE, m_encodeBufferSize);
#if DBG_DUMP
    m_instrNumber = 0;
    m_offsetBuffer = AnewArray(m_tempAlloc, uint, instrCount);
#endif

    m_pragmaInstrToRecordMap    = Anew(m_tempAlloc, PragmaInstrList, m_tempAlloc);
    if (DoTrackAllStatementBoundary())
    {
        // Create a new list, if we are tracking all statement boundaries.
        m_pragmaInstrToRecordOffset = Anew(m_tempAlloc, PragmaInstrList, m_tempAlloc);
    }
    else
    {
        // Set the list to the same as the throw map list, so that processing of the list
        // of pragma are done on those only.
        m_pragmaInstrToRecordOffset = m_pragmaInstrToRecordMap;
    }

#if defined(_M_IX86) || defined(_M_X64)
    // for BR shortening
    m_inlineeFrameRecords       = Anew(m_tempAlloc, InlineeFrameRecords, m_tempAlloc);
#endif

    m_pc = m_encodeBuffer;
    m_inlineeFrameMap = Anew(m_tempAlloc, InlineeFrameMap, m_tempAlloc);
    m_bailoutRecordMap = Anew(m_tempAlloc, BailoutRecordMap, m_tempAlloc);

    CodeGenWorkItem* workItem = m_func->m_workItem;
    uint loopNum = Js::LoopHeader::NoLoop;

    if (workItem->Type() == JsLoopBodyWorkItemType)
    {
        loopNum = ((JsLoopBodyCodeGen*)workItem)->GetLoopNumber();
    }

    Js::SmallSpanSequenceIter iter;
    IR::PragmaInstr* pragmaInstr = nullptr;
    uint32 pragmaOffsetInBuffer = 0;

#ifdef _M_X64
    bool inProlog = false;
#endif
    bool isCallInstr = false;

    // CRC Check to ensure the integrity of the encoded bytes.
    uint initialCRCSeed = 0;
    errno_t err = rand_s(&initialCRCSeed);

    if (err != 0)
    {
        Fatal();
    }

    uint bufferCRC = initialCRCSeed;  

    FOREACH_INSTR_IN_FUNC(instr, m_func)
    {
        Assert(Lowerer::ValidOpcodeAfterLower(instr, m_func));

        if (GetCurrentOffset() + MachMaxInstrSize < m_encodeBufferSize)
        {
            ptrdiff_t count;

#if DBG_DUMP
            AssertMsg(m_instrNumber < instrCount, "Bad instr count?");
            __analysis_assume(m_instrNumber < instrCount);
            m_offsetBuffer[m_instrNumber++] = GetCurrentOffset();
#endif
            if (instr->IsPragmaInstr())
            {
                switch(instr->m_opcode)
                {
#ifdef _M_X64
                case Js::OpCode::PrologStart:
                    inProlog = true;
                    continue;

                case Js::OpCode::PrologEnd:
                    inProlog = false;
                    continue;
#endif
                case Js::OpCode::StatementBoundary:
                    pragmaOffsetInBuffer = GetCurrentOffset();
                    pragmaInstr = instr->AsPragmaInstr();
                    pragmaInstr->m_offsetInBuffer = pragmaOffsetInBuffer;

                    // will record after BR shortening with adjusted offsets
                    if (DoTrackAllStatementBoundary())
                    {
                        m_pragmaInstrToRecordOffset->Add(pragmaInstr);
                    }

                    break;

                default:
                    continue;
                }
            }
            else if (instr->IsBranchInstr() && instr->AsBranchInstr()->IsMultiBranch())
            {
                Assert(instr->GetSrc1() && instr->GetSrc1()->IsRegOpnd());
                IR::MultiBranchInstr * multiBranchInstr = instr->AsBranchInstr()->AsMultiBrInstr();

                if (multiBranchInstr->m_isSwitchBr &&
                    (multiBranchInstr->m_kind == IR::MultiBranchInstr::IntJumpTable || multiBranchInstr->m_kind == IR::MultiBranchInstr::SingleCharStrJumpTable))
                {
                    BranchJumpTableWrapper * branchJumpTableWrapper = multiBranchInstr->GetBranchJumpTable();
                    if (jumpTableListForSwitchStatement == nullptr)
                    {
                        jumpTableListForSwitchStatement = Anew(m_tempAlloc, JmpTableList, m_tempAlloc);
                    }
                    jumpTableListForSwitchStatement->Add(branchJumpTableWrapper);

                    totalJmpTableSizeInBytes += (branchJumpTableWrapper->tableSize * sizeof(void*));
                }
                else
                {
                    //Reloc Records
                    EncoderMD * encoderMD = &(this->m_encoderMD);
                    multiBranchInstr->MapMultiBrTargetByAddress([=](void ** offset) -> void
                    {
#if defined(_M_ARM32_OR_ARM64)
                        encoderMD->AddLabelReloc((byte*) offset);
#else
                        encoderMD->AppendRelocEntry(RelocTypeLabelUse, (void*) (offset), *(IR::LabelInstr**)(offset));
                        *((size_t*)offset) = 0;
#endif
                    });
                }
            }
            else
            {
                isCallInstr = LowererMD::IsCall(instr);
                if (pragmaInstr && (instr->isInlineeEntryInstr || isCallInstr))
                {
                    // will record throw map after BR shortening with adjusted offsets
                    m_pragmaInstrToRecordMap->Add(pragmaInstr);
                    pragmaInstr = nullptr; // Only once per pragma instr -- do we need to make this record?
                }

                if (instr->HasBailOutInfo())
                {
                    Assert(this->m_func->hasBailout);
                    Assert(LowererMD::IsCall(instr));
                    instr->GetBailOutInfo()->FinalizeBailOutRecord(this->m_func);
                }

                if (instr->isInlineeEntryInstr)
                {

                    m_encoderMD.EncodeInlineeCallInfo(instr, GetCurrentOffset());
                }

                if (instr->m_opcode == Js::OpCode::InlineeStart)
                {
                    Assert(!instr->isInlineeEntryInstr);
                    if (pragmaInstr)
                    {
                        m_pragmaInstrToRecordMap->Add(pragmaInstr);
                        pragmaInstr = nullptr;
                    }
                    Func* inlinee = instr->m_func;
                    if (inlinee->frameInfo && inlinee->frameInfo->record)
                    {
                        inlinee->frameInfo->record->Finalize(inlinee, GetCurrentOffset());

#if defined(_M_IX86) || defined(_M_X64)
                        // Store all records to be adjusted for BR shortening
                        m_inlineeFrameRecords->Add(inlinee->frameInfo->record);
#endif
                    }
                    continue;
                }
            }

            count = m_encoderMD.Encode(instr, m_pc, m_encodeBuffer);
#if defined(_M_IX86) || defined(_M_X64)
            bufferCRC = CalculateCRC(bufferCRC, count, m_pc);
#endif

#if DBG_DUMP
            if (PHASE_TRACE(Js::EncoderPhase, this->m_func))
            {
                instr->Dump((IRDumpFlags)(IRDumpFlags_SimpleForm | IRDumpFlags_SkipEndLine | IRDumpFlags_SkipByteCodeOffset));
                Output::SkipToColumn(80);
                for (BYTE * current = m_pc; current < m_pc + count; current++)
                {
                    Output::Print(_u("%02X "), *current);
                }
                Output::Print(_u("\n"));
                Output::Flush();
            }
#endif
#ifdef _M_X64
            if (inProlog)
                m_func->m_prologEncoder.EncodeInstr(instr, count & 0xFF);
#endif
            m_pc += count;

#if defined(_M_IX86) || defined(_M_X64)
            // for BR shortening.
            if (instr->isInlineeEntryInstr)
                m_encoderMD.AppendRelocEntry(RelocType::RelocTypeInlineeEntryOffset, (void*) (m_pc - MachPtr));
#endif
            if (isCallInstr)
            {
                isCallInstr = false;
                this->RecordInlineeFrame(instr->m_func, GetCurrentOffset());
            }
            if (instr->HasBailOutInfo() && Lowerer::DoLazyBailout(this->m_func))
            {
                this->RecordBailout(instr, (uint32)(m_pc - m_encodeBuffer));
            }
        }
        else
        {
            Fatal();
        }
    } NEXT_INSTR_IN_FUNC;
    
    ptrdiff_t codeSize = m_pc - m_encodeBuffer + totalJmpTableSizeInBytes;

    BOOL isSuccessBrShortAndLoopAlign = false;

#if defined(_M_IX86) || defined(_M_X64)
    // Shorten branches. ON by default
    if (!PHASE_OFF(Js::BrShortenPhase, m_func))
    {
        uint brShortenedbufferCRC = initialCRCSeed;
        isSuccessBrShortAndLoopAlign = ShortenBranchesAndLabelAlign(&m_encodeBuffer, &codeSize, &brShortenedbufferCRC, bufferCRC, totalJmpTableSizeInBytes);
        if (isSuccessBrShortAndLoopAlign)
        {
            bufferCRC = brShortenedbufferCRC;
        }
    }
#endif
#if DBG_DUMP | defined(VTUNE_PROFILING)
    if (this->m_func->DoRecordNativeMap())
    {
        // Record PragmaInstr offsets and throw maps
        for (int32 i = 0; i < m_pragmaInstrToRecordOffset->Count(); i++)
        {
            IR::PragmaInstr *inst = m_pragmaInstrToRecordOffset->Item(i);
            inst->Record(inst->m_offsetInBuffer);
        }
    }
#endif
    for (int32 i = 0; i < m_pragmaInstrToRecordMap->Count(); i ++)
    {
        IR::PragmaInstr *inst = m_pragmaInstrToRecordMap->Item(i);
        inst->RecordThrowMap(iter, inst->m_offsetInBuffer);
    }

    BEGIN_CODEGEN_PHASE(m_func, Js::EmitterPhase);

    // Copy to permanent buffer.

    Assert(Math::FitsInDWord(codeSize));

    ushort xdataSize;
    ushort pdataCount;
#ifdef _M_X64
    pdataCount = 1;
    xdataSize = (ushort)m_func->m_prologEncoder.SizeOfUnwindInfo();
#elif _M_ARM
    pdataCount = (ushort)m_func->m_unwindInfo.GetPDataCount(codeSize);
    xdataSize = (UnwindInfoManager::MaxXdataBytes + 3) * pdataCount;
#else
    xdataSize = 0;
    pdataCount = 0;
#endif
    OUTPUT_VERBOSE_TRACE(Js::EmitterPhase, _u("PDATA count:%u\n"), pdataCount);
    OUTPUT_VERBOSE_TRACE(Js::EmitterPhase, _u("Size of XDATA:%u\n"), xdataSize);
    OUTPUT_VERBOSE_TRACE(Js::EmitterPhase, _u("Size of code:%u\n"), codeSize);

    TryCopyAndAddRelocRecordsForSwitchJumpTableEntries(m_encodeBuffer, codeSize, jumpTableListForSwitchStatement, totalJmpTableSizeInBytes);

    workItem->RecordNativeCodeSize(m_func, (DWORD)codeSize, pdataCount, xdataSize);

    this->m_bailoutRecordMap->MapAddress([=](int index, LazyBailOutRecord* record)
    {
        this->m_encoderMD.AddLabelReloc((BYTE*)&record->instructionPointer);
    });

    // Relocs
    m_encoderMD.ApplyRelocs((size_t) workItem->GetCodeAddress(), codeSize, &bufferCRC, isSuccessBrShortAndLoopAlign);

    workItem->RecordNativeCode(m_func, m_encodeBuffer);

#if defined(_M_IX86) || defined(_M_X64)
    ValidateCRCOnFinalBuffer((BYTE*)workItem->GetCodeAddress(), codeSize, totalJmpTableSizeInBytes, m_encodeBuffer, initialCRCSeed, bufferCRC, isSuccessBrShortAndLoopAlign);
#endif

    m_func->GetScriptContext()->GetThreadContext()->SetValidCallTargetForCFG((PVOID) workItem->GetCodeAddress());

#ifdef _M_X64
    m_func->m_prologEncoder.FinalizeUnwindInfo();
    workItem->RecordUnwindInfo(0, m_func->m_prologEncoder.GetUnwindInfo(), m_func->m_prologEncoder.SizeOfUnwindInfo());
#elif _M_ARM
    m_func->m_unwindInfo.EmitUnwindInfo(workItem);
    workItem->SetCodeAddress(workItem->GetCodeAddress() | 0x1); // Set thumb mode
#endif

    Js::EntryPointInfo* entryPointInfo = this->m_func->m_workItem->GetEntryPoint();
    const bool isSimpleJit = m_func->IsSimpleJit();
    Assert(
        isSimpleJit ||
        entryPointInfo->GetJitTransferData() != nullptr && !entryPointInfo->GetJitTransferData()->GetIsReady());

    if (this->m_inlineeFrameMap->Count() > 0 &&
        !(this->m_inlineeFrameMap->Count() == 1 && this->m_inlineeFrameMap->Item(0).record == nullptr))
    {
        entryPointInfo->RecordInlineeFrameMap(m_inlineeFrameMap);
    }

    if (this->m_bailoutRecordMap->Count() > 0)
    {
        entryPointInfo->RecordBailOutMap(m_bailoutRecordMap);
    }

    if (this->m_func->pinnedTypeRefs != nullptr)
    {
        Assert(!isSimpleJit);

        Func::TypeRefSet* pinnedTypeRefs = this->m_func->pinnedTypeRefs;
        int pinnedTypeRefCount = pinnedTypeRefs->Count();
        void** compactPinnedTypeRefs = HeapNewArrayZ(void*, pinnedTypeRefCount);

        int index = 0;
        pinnedTypeRefs->Map([compactPinnedTypeRefs, &index](void* typeRef) -> void
        {
            compactPinnedTypeRefs[index++] = typeRef;
        });

        if (PHASE_TRACE(Js::TracePinnedTypesPhase, this->m_func))
        {
            char16 debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
            Output::Print(_u("PinnedTypes: function %s(%s) pinned %d types.\n"),
                this->m_func->GetJnFunction()->GetDisplayName(), this->m_func->GetJnFunction()->GetDebugNumberSet(debugStringBuffer), pinnedTypeRefCount);
            Output::Flush();
        }

        entryPointInfo->GetJitTransferData()->SetRuntimeTypeRefs(compactPinnedTypeRefs, pinnedTypeRefCount);
    }

    // Save all equivalent type guards in a fixed size array on the JIT transfer data
    if (this->m_func->equivalentTypeGuards != nullptr)
    {
        AssertMsg(!PHASE_OFF(Js::EquivObjTypeSpecPhase, this->m_func), "Why do we have equivalent type guards if we don't do equivalent object type spec?");

        int count = this->m_func->equivalentTypeGuards->Count();
        Js::JitEquivalentTypeGuard** guards = HeapNewArrayZ(Js::JitEquivalentTypeGuard*, count);
        Js::JitEquivalentTypeGuard** dstGuard = guards;
        this->m_func->equivalentTypeGuards->Map([&dstGuard](Js::JitEquivalentTypeGuard* srcGuard) -> void
        {
            *dstGuard++ = srcGuard;
        });
        entryPointInfo->GetJitTransferData()->SetEquivalentTypeGuards(guards, count);
    }

    if (this->m_func->lazyBailoutProperties.Count() > 0)
    {
        int count = this->m_func->lazyBailoutProperties.Count();
        Js::PropertyId* lazyBailoutProperties = HeapNewArrayZ(Js::PropertyId, count);
        Js::PropertyId* dstProperties = lazyBailoutProperties;
        this->m_func->lazyBailoutProperties.Map([&](Js::PropertyId propertyId)
        {
            *dstProperties++ = propertyId;
        });
        entryPointInfo->GetJitTransferData()->SetLazyBailoutProperties(lazyBailoutProperties, count);
    }

    // Save all property guards on the JIT transfer data in a map keyed by property ID. We will use this map when installing the entry
    // point to register each guard for invalidation.
    if (this->m_func->propertyGuardsByPropertyId != nullptr)
    {
        Assert(!isSimpleJit);

        AssertMsg(!(PHASE_OFF(Js::ObjTypeSpecPhase, this->m_func) && PHASE_OFF(Js::FixedMethodsPhase, this->m_func)),
            "Why do we have type guards if we don't do object type spec or fixed methods?");

        int propertyCount = this->m_func->propertyGuardsByPropertyId->Count();
        Assert(propertyCount > 0);

#if DBG
        int totalGuardCount = (this->m_func->singleTypeGuards != nullptr ? this->m_func->singleTypeGuards->Count() : 0)
            + (this->m_func->equivalentTypeGuards != nullptr ? this->m_func->equivalentTypeGuards->Count() : 0);
        Assert(totalGuardCount > 0);
        Assert(totalGuardCount == this->m_func->indexedPropertyGuardCount);
#endif

        int guardSlotCount = 0;
        this->m_func->propertyGuardsByPropertyId->Map([&guardSlotCount](Js::PropertyId propertyId, Func::IndexedPropertyGuardSet* set) -> void
        {
            guardSlotCount += set->Count();
        });

        size_t typeGuardTransferSize =                              // Reserve enough room for:
            propertyCount * sizeof(Js::TypeGuardTransferEntry) +    //   each propertyId,
            propertyCount * sizeof(Js::JitIndexedPropertyGuard*) +  //   terminating nullptr guard for each propertyId,
            guardSlotCount * sizeof(Js::JitIndexedPropertyGuard*);  //   a pointer for each guard we counted above.

        // The extra room for sizeof(Js::TypePropertyGuardEntry) allocated by HeapNewPlus will be used for the terminating invalid propertyId.
        // Review (jedmiad): Skip zeroing?  This is heap allocated so there shouldn't be any false recycler references.
        Js::TypeGuardTransferEntry* typeGuardTransferRecord = HeapNewPlusZ(typeGuardTransferSize, Js::TypeGuardTransferEntry);

        Func* func = this->m_func;

        Js::TypeGuardTransferEntry* dstEntry = typeGuardTransferRecord;
        this->m_func->propertyGuardsByPropertyId->Map([func, &dstEntry](Js::PropertyId propertyId, Func::IndexedPropertyGuardSet* srcSet) -> void
        {
            dstEntry->propertyId = propertyId;

            int guardIndex = 0;

            srcSet->Map([dstEntry, &guardIndex](Js::JitIndexedPropertyGuard* guard) -> void
            {
                dstEntry->guards[guardIndex++] = guard;
            });

            dstEntry->guards[guardIndex++] = nullptr;
            dstEntry = reinterpret_cast<Js::TypeGuardTransferEntry*>(&dstEntry->guards[guardIndex]);
        });
        dstEntry->propertyId = Js::Constants::NoProperty;
        dstEntry++;

        Assert(reinterpret_cast<char*>(dstEntry) <= reinterpret_cast<char*>(typeGuardTransferRecord) + typeGuardTransferSize + sizeof(Js::TypeGuardTransferEntry));

        entryPointInfo->RecordTypeGuards(this->m_func->indexedPropertyGuardCount, typeGuardTransferRecord, typeGuardTransferSize);
    }

    // Save all constructor caches on the JIT transfer data in a map keyed by property ID. We will use this map when installing the entry
    // point to register each cache for invalidation.
    if (this->m_func->ctorCachesByPropertyId != nullptr)
    {
        Assert(!isSimpleJit);

        AssertMsg(!(PHASE_OFF(Js::ObjTypeSpecPhase, this->m_func) && PHASE_OFF(Js::FixedMethodsPhase, this->m_func)),
            "Why do we have constructor cache guards if we don't do object type spec or fixed methods?");

        int propertyCount = this->m_func->ctorCachesByPropertyId->Count();
        Assert(propertyCount > 0);

#if DBG
        int cacheCount = entryPointInfo->GetConstructorCacheCount();
        Assert(cacheCount > 0);
#endif

        int cacheSlotCount = 0;
        this->m_func->ctorCachesByPropertyId->Map([&cacheSlotCount](Js::PropertyId propertyId, Func::CtorCacheSet* cacheSet) -> void
        {
            cacheSlotCount += cacheSet->Count();
        });

        size_t ctorCachesTransferSize =                                // Reserve enough room for:
            propertyCount * sizeof(Js::CtorCacheGuardTransferEntry) +  //   each propertyId,
            propertyCount * sizeof(Js::ConstructorCache*) +            //   terminating null cache for each propertyId,
            cacheSlotCount * sizeof(Js::JitIndexedPropertyGuard*);     //   a pointer for each cache we counted above.

        // The extra room for sizeof(Js::CtorCacheGuardTransferEntry) allocated by HeapNewPlus will be used for the terminating invalid propertyId.
        // Review (jedmiad): Skip zeroing?  This is heap allocated so there shouldn't be any false recycler references.
        Js::CtorCacheGuardTransferEntry* ctorCachesTransferRecord = HeapNewPlusZ(ctorCachesTransferSize, Js::CtorCacheGuardTransferEntry);

        Func* func = this->m_func;

        Js::CtorCacheGuardTransferEntry* dstEntry = ctorCachesTransferRecord;
        this->m_func->ctorCachesByPropertyId->Map([func, &dstEntry](Js::PropertyId propertyId, Func::CtorCacheSet* srcCacheSet) -> void
        {
            dstEntry->propertyId = propertyId;

            int cacheIndex = 0;

            srcCacheSet->Map([dstEntry, &cacheIndex](Js::ConstructorCache* cache) -> void
            {
                dstEntry->caches[cacheIndex++] = cache;
            });

            dstEntry->caches[cacheIndex++] = nullptr;
            dstEntry = reinterpret_cast<Js::CtorCacheGuardTransferEntry*>(&dstEntry->caches[cacheIndex]);
        });
        dstEntry->propertyId = Js::Constants::NoProperty;
        dstEntry++;

        Assert(reinterpret_cast<char*>(dstEntry) <= reinterpret_cast<char*>(ctorCachesTransferRecord) + ctorCachesTransferSize + sizeof(Js::CtorCacheGuardTransferEntry));

        entryPointInfo->RecordCtorCacheGuards(ctorCachesTransferRecord, ctorCachesTransferSize);
    }

    if(!isSimpleJit)
    {
        entryPointInfo->GetJitTransferData()->SetIsReady();
    }

    workItem->FinalizeNativeCode(m_func);

    END_CODEGEN_PHASE(m_func, Js::EmitterPhase);

#if DBG_DUMP

    m_func->m_codeSize = codeSize;
    if (PHASE_DUMP(Js::EncoderPhase, m_func) || PHASE_DUMP(Js::BackEndPhase, m_func))
    {
        bool dumpIRAddressesValue = Js::Configuration::Global.flags.DumpIRAddresses;
        Js::Configuration::Global.flags.DumpIRAddresses = true;

        this->m_func->DumpHeader();

        m_instrNumber = 0;
        FOREACH_INSTR_IN_FUNC(instr, m_func)
        {
            __analysis_assume(m_instrNumber < instrCount);
            instr->DumpGlobOptInstrString();
#ifdef _WIN64
            Output::Print(_u("%12IX  "), m_offsetBuffer[m_instrNumber++] + (BYTE *)workItem->GetCodeAddress());
#else
            Output::Print(_u("%8IX  "), m_offsetBuffer[m_instrNumber++] + (BYTE *)workItem->GetCodeAddress());
#endif
            instr->Dump();
        } NEXT_INSTR_IN_FUNC;
        Output::Flush();

        Js::Configuration::Global.flags.DumpIRAddresses = dumpIRAddressesValue;
    }

    if (PHASE_DUMP(Js::EncoderPhase, m_func) && Js::Configuration::Global.flags.Verbose)
    {
        workItem->DumpNativeOffsetMaps();
        workItem->DumpNativeThrowSpanSequence();
        this->DumpInlineeFrameMap(workItem->GetCodeAddress());
        Output::Flush();
    }
#endif
}

bool Encoder::DoTrackAllStatementBoundary() const
{
#if DBG_DUMP | defined(VTUNE_PROFILING)
    return this->m_func->DoRecordNativeMap();
#else
    return false;
#endif
}

void Encoder::TryCopyAndAddRelocRecordsForSwitchJumpTableEntries(BYTE *codeStart, size_t codeSize, JmpTableList * jumpTableListForSwitchStatement, size_t totalJmpTableSizeInBytes)
{
    if (jumpTableListForSwitchStatement == nullptr)
    {
        return;
    }

    BYTE * jmpTableStartAddress = codeStart + codeSize - totalJmpTableSizeInBytes;
    EncoderMD * encoderMD = &m_encoderMD;

    jumpTableListForSwitchStatement->Map([&](uint index, BranchJumpTableWrapper * branchJumpTableWrapper) -> void
    {
        Assert(branchJumpTableWrapper != nullptr);

        void ** srcJmpTable = branchJumpTableWrapper->jmpTable;
        size_t jmpTableSizeInBytes = branchJumpTableWrapper->tableSize * sizeof(void*);

        AssertMsg(branchJumpTableWrapper->labelInstr != nullptr, "Label not yet created?");
        Assert(branchJumpTableWrapper->labelInstr->GetPC() == nullptr);

        branchJumpTableWrapper->labelInstr->SetPC(jmpTableStartAddress);
        memcpy(jmpTableStartAddress, srcJmpTable, jmpTableSizeInBytes);

        for (int i = 0; i < branchJumpTableWrapper->tableSize; i++)
        {
            void * addressOfJmpTableEntry = jmpTableStartAddress + (i * sizeof(void*));
            Assert((ptrdiff_t) addressOfJmpTableEntry - (ptrdiff_t) jmpTableStartAddress < (ptrdiff_t) jmpTableSizeInBytes);
#if defined(_M_ARM32_OR_ARM64)
            encoderMD->AddLabelReloc((byte*) addressOfJmpTableEntry);
#else
            encoderMD->AppendRelocEntry(RelocTypeLabelUse, addressOfJmpTableEntry, *(IR::LabelInstr**)addressOfJmpTableEntry);
            *((size_t*)addressOfJmpTableEntry) = 0;
#endif
        }

        jmpTableStartAddress += (jmpTableSizeInBytes);
    });

    Assert(jmpTableStartAddress == codeStart + codeSize);
}

uint32 Encoder::GetCurrentOffset() const
{
    Assert(m_pc - m_encodeBuffer <= UINT_MAX);      // encode buffer size is uint32
    return static_cast<uint32>(m_pc - m_encodeBuffer);
}

void Encoder::RecordInlineeFrame(Func* inlinee, uint32 currentOffset)
{
    // The only restriction for not supporting loop bodies is that inlinee frame map is created on FunctionEntryPointInfo & not
    // the base class EntryPointInfo.
    if (!(this->m_func->IsLoopBody() && PHASE_OFF(Js::InlineInJitLoopBodyPhase, this->m_func)) && !this->m_func->IsSimpleJit())
    {
        InlineeFrameRecord* record = nullptr;
        if (inlinee->frameInfo && inlinee->m_hasInlineArgsOpt)
        {
            record = inlinee->frameInfo->record;
            Assert(record != nullptr);
        }
        if (m_inlineeFrameMap->Count() > 0)
        {
            // update existing record if the entry is the same.
            NativeOffsetInlineeFramePair& lastPair = m_inlineeFrameMap->Item(m_inlineeFrameMap->Count() - 1);

            if (lastPair.record == record)
            {
                lastPair.offset = currentOffset;
                return;
            }
        }
        NativeOffsetInlineeFramePair pair = { currentOffset, record };
        m_inlineeFrameMap->Add(pair);
    }
}

#if defined(_M_IX86) || defined(_M_X64)
/*
*   ValidateCRCOnFinalBuffer
*       - Validates the CRC that is last computed (could be either the one after BranchShortening or after encoding itself)
*       - We calculate the CRC for jump table and dictionary after computing the code section.
*       - Also, all reloc data are computed towards the end - after computing the code section - because we don't have to deal with the changes relocs while operating on the code section.
*       - The version of CRC that we are validating with, doesn't have Relocs applied but the final buffer does - So we have to make adjustments while calculating the final buffer's CRC.
*/
void Encoder::ValidateCRCOnFinalBuffer(_In_reads_bytes_(finalCodeSize) BYTE * finalCodeBufferStart, size_t finalCodeSize, size_t jumpTableSize, _In_reads_bytes_(finalCodeSize) BYTE * oldCodeBufferStart, uint initialCrcSeed, uint bufferCrcToValidate, BOOL isSuccessBrShortAndLoopAlign)
{
    RelocList * relocList = m_encoderMD.GetRelocList();

    BYTE * currentStartAddress = finalCodeBufferStart;
    BYTE * currentEndAddress = nullptr;
    size_t crcSizeToCompute = 0;

    size_t finalCodeSizeWithoutJumpTable = finalCodeSize - jumpTableSize;

    uint finalBufferCRC = initialCrcSeed;

    BYTE * oldPtr = nullptr;

    if (relocList != nullptr)
    {
        for (int index = 0; index < relocList->Count(); index++)
        {
            EncodeRelocAndLabels * relocTuple = &relocList->Item(index);

            //We will deal with the jump table and dictionary entries along with other reloc records in ApplyRelocs()
            if ((BYTE*)m_encoderMD.GetRelocBufferAddress(relocTuple) >= oldCodeBufferStart && (BYTE*)m_encoderMD.GetRelocBufferAddress(relocTuple) < (oldCodeBufferStart + finalCodeSizeWithoutJumpTable))
            {
                BYTE* finalBufferRelocTuplePtr = (BYTE*)m_encoderMD.GetRelocBufferAddress(relocTuple) - oldCodeBufferStart + finalCodeBufferStart;
                Assert(finalBufferRelocTuplePtr >= finalCodeBufferStart && finalBufferRelocTuplePtr < (finalCodeBufferStart + finalCodeSizeWithoutJumpTable));
                uint relocDataSize = m_encoderMD.GetRelocDataSize(relocTuple);
                if (relocDataSize != 0)
                {
                    AssertMsg(oldPtr == nullptr || oldPtr < finalBufferRelocTuplePtr, "Assumption here is that the reloc list is strictly increasing in terms of bufferAddress");
                    oldPtr = finalBufferRelocTuplePtr;

                    currentEndAddress = finalBufferRelocTuplePtr;
                    crcSizeToCompute = currentEndAddress - currentStartAddress;
                    
                    Assert(currentEndAddress >= currentStartAddress);

                    finalBufferCRC = CalculateCRC(finalBufferCRC, crcSizeToCompute, currentStartAddress);
                    for (uint i = 0; i < relocDataSize; i++)
                    {
                        finalBufferCRC = CalculateCRC(finalBufferCRC, 0);
                    }
                    currentStartAddress = currentEndAddress + relocDataSize;
                }
            }
        }
    }

    currentEndAddress = finalCodeBufferStart + finalCodeSizeWithoutJumpTable;
    crcSizeToCompute = currentEndAddress - currentStartAddress;

    Assert(currentEndAddress >= currentStartAddress);

    finalBufferCRC = CalculateCRC(finalBufferCRC, crcSizeToCompute, currentStartAddress);

    //Include all offsets from the reloc records to the CRC.
    m_encoderMD.ApplyRelocs((size_t)finalCodeBufferStart, finalCodeSize, &finalBufferCRC, isSuccessBrShortAndLoopAlign, true);

    if (finalBufferCRC != bufferCrcToValidate)
    {
        Assert(false);
        Fatal();
    }
}
#endif

/*
*   EnsureRelocEntryIntegrity
*       - We compute the target address as the processor would compute it and check if the target is within the final buffer's bounds.
*       - For relative addressing, Target = current m_pc + offset
*       - For absolute addressing, Target = direct address
*/
void Encoder::EnsureRelocEntryIntegrity(size_t newBufferStartAddress, size_t codeSize, size_t oldBufferAddress, size_t relocAddress, uint offsetBytes, ptrdiff_t opndData, bool isRelativeAddr)
{
    size_t targetBrAddress = 0;
    size_t newBufferEndAddress = newBufferStartAddress + codeSize;
    
    //Handle Dictionary addresses here - The target address will be in the dictionary.
    if (relocAddress < oldBufferAddress || relocAddress >= (oldBufferAddress + codeSize))
    {
        targetBrAddress = (size_t)(*(size_t*)relocAddress);
    }
    else
    {
        size_t newBufferRelocAddr = relocAddress - oldBufferAddress + newBufferStartAddress;

        if (isRelativeAddr)
        {
            targetBrAddress = (size_t)newBufferRelocAddr + offsetBytes + opndData;
        }
        else  // Absolute Address
        {
            targetBrAddress = (size_t)opndData;
        }
    }

    if (targetBrAddress < newBufferStartAddress || targetBrAddress >= newBufferEndAddress)
    {
        Assert(false);
        Fatal();
    }
}

/*
* Pre-populated Table used for calculating CRC32.
*/

static const uint crc_32_tab[] =
{
    0x00000000L, 0x77073096L, 0xEE0E612CL, 0x990951BAL, 0x076DC419L, 0x706AF48FL, 0xE963A535L, 0x9E6495A3L,
    0x0EDB8832L, 0x79DCB8A4L, 0xE0D5E91EL, 0x97D2D988L, 0x09B64C2BL, 0x7EB17CBDL, 0xE7B82D07L, 0x90BF1D91L,
    0x1DB71064L, 0x6AB020F2L, 0xF3B97148L, 0x84BE41DEL, 0x1ADAD47DL, 0x6DDDE4EBL, 0xF4D4B551L, 0x83D385C7L,
    0x136C9856L, 0x646BA8C0L, 0xFD62F97AL, 0x8A65C9ECL, 0x14015C4FL, 0x63066CD9L, 0xFA0F3D63L, 0x8D080DF5L,
    0x3B6E20C8L, 0x4C69105EL, 0xD56041E4L, 0xA2677172L, 0x3C03E4D1L, 0x4B04D447L, 0xD20D85FDL, 0xA50AB56BL,
    0x35B5A8FAL, 0x42B2986CL, 0xDBBBC9D6L, 0xACBCF940L, 0x32D86CE3L, 0x45DF5C75L, 0xDCD60DCFL, 0xABD13D59L,
    0x26D930ACL, 0x51DE003AL, 0xC8D75180L, 0xBFD06116L, 0x21B4F4B5L, 0x56B3C423L, 0xCFBA9599L, 0xB8BDA50FL,
    0x2802B89EL, 0x5F058808L, 0xC60CD9B2L, 0xB10BE924L, 0x2F6F7C87L, 0x58684C11L, 0xC1611DABL, 0xB6662D3DL,
    0x76DC4190L, 0x01DB7106L, 0x98D220BCL, 0xEFD5102AL, 0x71B18589L, 0x06B6B51FL, 0x9FBFE4A5L, 0xE8B8D433L,
    0x7807C9A2L, 0x0F00F934L, 0x9609A88EL, 0xE10E9818L, 0x7F6A0DBBL, 0x086D3D2DL, 0x91646C97L, 0xE6635C01L,
    0x6B6B51F4L, 0x1C6C6162L, 0x856530D8L, 0xF262004EL, 0x6C0695EDL, 0x1B01A57BL, 0x8208F4C1L, 0xF50FC457L,
    0x65B0D9C6L, 0x12B7E950L, 0x8BBEB8EAL, 0xFCB9887CL, 0x62DD1DDFL, 0x15DA2D49L, 0x8CD37CF3L, 0xFBD44C65L,
    0x4DB26158L, 0x3AB551CEL, 0xA3BC0074L, 0xD4BB30E2L, 0x4ADFA541L, 0x3DD895D7L, 0xA4D1C46DL, 0xD3D6F4FBL,
    0x4369E96AL, 0x346ED9FCL, 0xAD678846L, 0xDA60B8D0L, 0x44042D73L, 0x33031DE5L, 0xAA0A4C5FL, 0xDD0D7CC9L,
    0x5005713CL, 0x270241AAL, 0xBE0B1010L, 0xC90C2086L, 0x5768B525L, 0x206F85B3L, 0xB966D409L, 0xCE61E49FL,
    0x5EDEF90EL, 0x29D9C998L, 0xB0D09822L, 0xC7D7A8B4L, 0x59B33D17L, 0x2EB40D81L, 0xB7BD5C3BL, 0xC0BA6CADL,
    0xEDB88320L, 0x9ABFB3B6L, 0x03B6E20CL, 0x74B1D29AL, 0xEAD54739L, 0x9DD277AFL, 0x04DB2615L, 0x73DC1683L,
    0xE3630B12L, 0x94643B84L, 0x0D6D6A3EL, 0x7A6A5AA8L, 0xE40ECF0BL, 0x9309FF9DL, 0x0A00AE27L, 0x7D079EB1L,
    0xF00F9344L, 0x8708A3D2L, 0x1E01F268L, 0x6906C2FEL, 0xF762575DL, 0x806567CBL, 0x196C3671L, 0x6E6B06E7L,
    0xFED41B76L, 0x89D32BE0L, 0x10DA7A5AL, 0x67DD4ACCL, 0xF9B9DF6FL, 0x8EBEEFF9L, 0x17B7BE43L, 0x60B08ED5L,
    0xD6D6A3E8L, 0xA1D1937EL, 0x38D8C2C4L, 0x4FDFF252L, 0xD1BB67F1L, 0xA6BC5767L, 0x3FB506DDL, 0x48B2364BL,
    0xD80D2BDAL, 0xAF0A1B4CL, 0x36034AF6L, 0x41047A60L, 0xDF60EFC3L, 0xA867DF55L, 0x316E8EEFL, 0x4669BE79L,
    0xCB61B38CL, 0xBC66831AL, 0x256FD2A0L, 0x5268E236L, 0xCC0C7795L, 0xBB0B4703L, 0x220216B9L, 0x5505262FL,
    0xC5BA3BBEL, 0xB2BD0B28L, 0x2BB45A92L, 0x5CB36A04L, 0xC2D7FFA7L, 0xB5D0CF31L, 0x2CD99E8BL, 0x5BDEAE1DL,
    0x9B64C2B0L, 0xEC63F226L, 0x756AA39CL, 0x026D930AL, 0x9C0906A9L, 0xEB0E363FL, 0x72076785L, 0x05005713L,
    0x95BF4A82L, 0xE2B87A14L, 0x7BB12BAEL, 0x0CB61B38L, 0x92D28E9BL, 0xE5D5BE0DL, 0x7CDCEFB7L, 0x0BDBDF21L,
    0x86D3D2D4L, 0xF1D4E242L, 0x68DDB3F8L, 0x1FDA836EL, 0x81BE16CDL, 0xF6B9265BL, 0x6FB077E1L, 0x18B74777L,
    0x88085AE6L, 0xFF0F6A70L, 0x66063BCAL, 0x11010B5CL, 0x8F659EFFL, 0xF862AE69L, 0x616BFFD3L, 0x166CCF45L,
    0xA00AE278L, 0xD70DD2EEL, 0x4E048354L, 0x3903B3C2L, 0xA7672661L, 0xD06016F7L, 0x4969474DL, 0x3E6E77DBL,
    0xAED16A4AL, 0xD9D65ADCL, 0x40DF0B66L, 0x37D83BF0L, 0xA9BCAE53L, 0xDEBB9EC5L, 0x47B2CF7FL, 0x30B5FFE9L,
    0xBDBDF21CL, 0xCABAC28AL, 0x53B39330L, 0x24B4A3A6L, 0xBAD03605L, 0xCDD70693L, 0x54DE5729L, 0x23D967BFL,
    0xB3667A2EL, 0xC4614AB8L, 0x5D681B02L, 0x2A6F2B94L, 0xB40BBE37L, 0xC30C8EA1L, 0x5A05DF1BL, 0x2D02EF8DL
};

uint Encoder::CalculateCRC(uint bufferCRC, size_t data)
{
#if defined(_M_IX86)
    if (AutoSystemInfo::Data.SSE4_1Available())
    {
        return _mm_crc32_u32(bufferCRC, data);
    }
#elif defined(_M_X64)
    if (AutoSystemInfo::Data.SSE4_1Available())
    {
        //CRC32 always returns a 32-bit result
        return (uint)_mm_crc32_u64(bufferCRC, data);
    }
#endif
    
    /*
        CRC32 code derived from work by Gary S. Brown.
    */

    /* update running CRC calculation with contents of a buffer */

    bufferCRC = bufferCRC ^ 0xffffffffL;
    bufferCRC = crc_32_tab[(bufferCRC ^ data) & 0xFF] ^ (bufferCRC >> 8);
    return (bufferCRC ^ 0xffffffffL);
}

uint Encoder::CalculateCRC(uint bufferCRC, size_t count, _In_reads_bytes_(count) void * buffer)
{
    for (uint index = 0; index < count; index++)
    {
        bufferCRC = CalculateCRC(bufferCRC, *((BYTE*)buffer + index));
    }
    return bufferCRC;
}

void Encoder::ValidateCRC(uint bufferCRC, uint initialCRCSeed, _In_reads_bytes_(count) void* buffer, size_t count)
{
    uint validationCRC = initialCRCSeed;

    validationCRC = CalculateCRC(validationCRC, count, buffer);

    if (validationCRC != bufferCRC)
    {
        //TODO: This throws internal error. Is this error type, Fine?
        Fatal();
    }
}

#if defined(_M_IX86) || defined(_M_X64)
///----------------------------------------------------------------------------
///
/// EncoderMD::ShortenBranchesAndLabelAlign
/// We try to shorten branches if the label instr is within 8-bits target range (-128 to 127)
/// and fix the relocList accordingly.
/// Also align LoopTop Label and TryCatchLabel
///----------------------------------------------------------------------------
BOOL
Encoder::ShortenBranchesAndLabelAlign(BYTE **codeStart, ptrdiff_t *codeSize, uint * pShortenedBufferCRC, uint bufferCrcToValidate, size_t jumpTableSize)
{
#ifdef  ENABLE_DEBUG_CONFIG_OPTIONS
    static uint32 globalTotalBytesSaved = 0, globalTotalBytesWithoutShortening = 0;
    static uint32 globalTotalBytesInserted = 0; // loop alignment nops
#endif

    uint32 brShortenedCount = 0;
    bool   codeChange       = false; // any overall BR shortened or label aligned ?

    BYTE* buffStart = *codeStart;
    BYTE* buffEnd = buffStart + *codeSize;
    ptrdiff_t newCodeSize = *codeSize;

    RelocList* relocList = m_encoderMD.GetRelocList();

    if (relocList == nullptr)
    {
        return false;
    }

#if DBG
    // Sanity check
    m_encoderMD.VerifyRelocList(buffStart, buffEnd);
#endif

    // Copy of original maps. Used to revert from BR shortening.
    OffsetList  *m_origInlineeFrameRecords = nullptr,
        *m_origInlineeFrameMap = nullptr,
        *m_origPragmaInstrToRecordOffset = nullptr;

    OffsetList  *m_origOffsetBuffer = nullptr;

    // we record the original maps, in case we have to revert.
    CopyMaps<false>(&m_origInlineeFrameRecords
        , &m_origInlineeFrameMap
        , &m_origPragmaInstrToRecordOffset
        , &m_origOffsetBuffer );

    // Here we mark BRs to be shortened and adjust Labels and relocList entries offsets.
    uint32 offsetBuffIndex = 0, pragmaInstToRecordOffsetIndex = 0, inlineeFrameRecordsIndex = 0, inlineeFrameMapIndex = 0;
    int32 totalBytesSaved = 0;

    // loop over all BRs, find the ones we can convert to short form
    for (int32 j = 0; j < relocList->Count(); j++)
    {
        IR::LabelInstr *targetLabel;
        int32 relOffset;
        uint32 bytesSaved = 0;
        BYTE* labelPc, *opcodeByte;
        BYTE* shortBrPtr, *fixedBrPtr; // without shortening

        EncodeRelocAndLabels &reloc = relocList->Item(j);

        // If not a long branch, just fix the reloc entry and skip.
        if (!reloc.isLongBr())
        {
            // if loop alignment is required, total bytes saved can change
            int32 newTotalBytesSaved = m_encoderMD.FixRelocListEntry(j, totalBytesSaved, buffStart, buffEnd);

            if (newTotalBytesSaved != totalBytesSaved)
            {
                AssertMsg(reloc.isAlignedLabel(), "Expecting aligned label.");
                // we aligned a loop, fix maps
                m_encoderMD.FixMaps((uint32)(reloc.getLabelOrigPC() - buffStart), totalBytesSaved, &inlineeFrameRecordsIndex, &inlineeFrameMapIndex, &pragmaInstToRecordOffsetIndex, &offsetBuffIndex);
                codeChange = true;
            }
            totalBytesSaved = newTotalBytesSaved;
            continue;
        }

        AssertMsg(reloc.isLongBr(), "Cannot shorten already shortened branch.");
        // long branch
        opcodeByte = reloc.getBrOpCodeByte();
        targetLabel = reloc.getBrTargetLabel();
        AssertMsg(targetLabel != nullptr, "Branch to non-existing label");

        labelPc = targetLabel->GetPC();

        // compute the new offset of that Br because of previous shortening/alignment
        shortBrPtr = fixedBrPtr = (BYTE*)reloc.m_ptr - totalBytesSaved;

        if (*opcodeByte == 0xe9 /* JMP rel32 */)
        {
            bytesSaved = 3;
        }
        else if (*opcodeByte >= 0x80 && *opcodeByte < 0x90 /* Jcc rel32 */)
        {
            Assert(*(opcodeByte - 1) == 0x0f);
            bytesSaved = 4;
            // Jcc rel8 is one byte shorter in opcode, fix Br ptr to point to start of rel8
            shortBrPtr--;
        }
        else
        {
            Assert(false);
        }

        // compute current distance to label
        if (labelPc >= (BYTE*) reloc.m_ptr)
        {
            // forward Br. We compare using the unfixed m_ptr, because the label is ahead and its Pc is not fixed it.
            relOffset = (int32)(labelPc - ((BYTE*)reloc.m_ptr + 4));
        }
        else
        {
            // backward Br. We compute relOffset after fixing the Br, since the label is already fixed.
            // We also include the 3-4 bytes saved after shortening the Br since the Br itself is included in the relative offset.
            relOffset =  (int32)(labelPc - (shortBrPtr + 1));
        }

        // update Br offset (overwritten later if Br is shortened)
        reloc.m_ptr = fixedBrPtr;

        // can we shorten ?
        if (relOffset >= -128 && relOffset <= 127)
        {
            uint32 brOffset;

            brShortenedCount++;
            // update with shortened br offset
            reloc.m_ptr = shortBrPtr;

            // fix all maps entries from last shortened br to this one, before updating total bytes saved.
            brOffset = (uint32) ((BYTE*)reloc.m_origPtr - buffStart);
            m_encoderMD.FixMaps(brOffset, totalBytesSaved, &inlineeFrameRecordsIndex, &inlineeFrameMapIndex, &pragmaInstToRecordOffsetIndex, &offsetBuffIndex);
            codeChange = true;
            totalBytesSaved += bytesSaved;

            // mark br reloc entry as shortened
#ifdef _M_IX86
            reloc.setAsShortBr(targetLabel);
#else
            reloc.setAsShortBr();
#endif
        }
    }

    // Fix the rest of the maps, if needed.
    if (totalBytesSaved != 0)
    {
        m_encoderMD.FixMaps((uint32) -1, totalBytesSaved, &inlineeFrameRecordsIndex, &inlineeFrameMapIndex, &pragmaInstToRecordOffsetIndex, &offsetBuffIndex);
        codeChange = true;
        newCodeSize -= totalBytesSaved;
    }

    // no BR shortening or Label alignment happened, no need to copy code
    if (!codeChange)
        return codeChange;

#ifdef ENABLE_DEBUG_CONFIG_OPTIONS
    globalTotalBytesWithoutShortening += (uint32)(*codeSize);
    globalTotalBytesSaved += (uint32)(*codeSize - newCodeSize);

    if (PHASE_TRACE(Js::BrShortenPhase, this->m_func))
    {
        OUTPUT_VERBOSE_TRACE(Js::BrShortenPhase, _u("func: %s, bytes saved: %d, bytes saved %%:%.2f, total bytes saved: %d, total bytes saved%%: %.2f, BR shortened: %d\n"),
            this->m_func->GetJnFunction()->GetDisplayName(), (*codeSize - newCodeSize), ((float)*codeSize - newCodeSize) / *codeSize * 100,
            globalTotalBytesSaved, ((float)globalTotalBytesSaved) / globalTotalBytesWithoutShortening * 100 , brShortenedCount);
        Output::Flush();
    }
#endif

    // At this point BRs are marked to be shortened, and relocList offsets are adjusted to new instruction length.
    // Next, we re-write the code to shorten the BRs and adjust relocList offsets to point to new buffer.
    // We also write NOPs for aligned loops.
    BYTE* tmpBuffer = AnewArray(m_tempAlloc, BYTE, newCodeSize);
    
    uint srcBufferCrc = *pShortenedBufferCRC;   //This has the intial Random CRC seed to start with.

    // start copying to new buffer
    // this can possibly be done during fixing, but there is no evidence it is an overhead to justify the complexity.
    BYTE *from = buffStart, *to = nullptr;
    BYTE *dst_p = (BYTE*)tmpBuffer;
    size_t dst_size = newCodeSize;
    size_t src_size;
    for (int32 i = 0; i < relocList->Count(); i++)
    {
        EncodeRelocAndLabels &reloc = relocList->Item(i);
        // shorten BR and copy
        if (reloc.isShortBr())
        {
            // validate that short BR offset is within 1 byte offset range.
            // This handles the rare case with loop alignment breaks br shortening.
            // Consider:
            //      BR $L1 // shortened
            //      ...
            //      L2:    // aligned, and makes the BR $L1 non-shortable anymore
            //      ...
            //      BR $L2
            //      ...
            //      L1:
            // In this case, we simply give up and revert the relocList.
            if(!reloc.validateShortBrTarget())
            {
                revertRelocList();
                // restore maps
                CopyMaps<true>(&m_origInlineeFrameRecords
                    , &m_origInlineeFrameMap
                    , &m_origPragmaInstrToRecordOffset
                    , &m_origOffsetBuffer
                    );

                return false;
            }

            // m_origPtr points to imm32 field in the original buffer
            BYTE *opcodeByte = (BYTE*)reloc.m_origPtr - 1;

            if (*opcodeByte == 0xe9 /* JMP rel32 */)
            {
                to = opcodeByte - 1;
            }
            else if (*opcodeByte >= 0x80 && *opcodeByte < 0x90 /* Jcc rel32 */)
            {
                Assert(*(opcodeByte - 1) == 0x0f);
                to = opcodeByte - 2;
            }
            else
            {
                Assert(false);
            }

            src_size = to - from + 1;
            AnalysisAssert(dst_size >= src_size);

            memcpy_s(dst_p, dst_size, from, src_size);

            srcBufferCrc = CalculateCRC(srcBufferCrc, (BYTE*)reloc.m_origPtr - from + 4, from);
            *pShortenedBufferCRC = CalculateCRC(*pShortenedBufferCRC, src_size, dst_p);

            dst_p += src_size;
            dst_size -= src_size;

            // fix the BR
            // write new opcode
            AnalysisAssert(dst_p < tmpBuffer + newCodeSize);
            *dst_p = (*opcodeByte == 0xe9) ? (BYTE)0xeb : (BYTE)(*opcodeByte - 0x10);
            *(dst_p + 1) = 0;   // imm8

            *pShortenedBufferCRC = CalculateCRC(*pShortenedBufferCRC, 2, dst_p);
            dst_p += 2; // 1 byte for opcode + 1 byte for imm8
            dst_size -= 2;
            from = (BYTE*)reloc.m_origPtr + 4;
        }
        else if (reloc.m_type == RelocTypeInlineeEntryOffset)
        {
            to = (BYTE*)reloc.m_origPtr - 1;
            CopyPartialBufferAndCalculateCRC(&dst_p, dst_size, from, to, pShortenedBufferCRC);

            *(size_t*)dst_p = reloc.GetInlineOffset();

            *pShortenedBufferCRC = CalculateCRC(*pShortenedBufferCRC, sizeof(size_t), dst_p);

            dst_p += sizeof(size_t);
            dst_size -= sizeof(size_t);

            srcBufferCrc = CalculateCRC(srcBufferCrc, (BYTE*)reloc.m_origPtr + sizeof(size_t) - from , from);

            from = (BYTE*)reloc.m_origPtr + sizeof(size_t);
        }
        // insert NOPs for aligned labels
        else if ((!PHASE_OFF(Js::LoopAlignPhase, m_func) && reloc.isAlignedLabel()) && reloc.getLabelNopCount() > 0)
        {
            IR::LabelInstr *label = reloc.getLabel();
            BYTE nop_count = reloc.getLabelNopCount();

            AssertMsg((BYTE*)label < buffStart || (BYTE*)label >= buffEnd, "Invalid label pointer.");
            AssertMsg((((uint32)(label->GetPC() - buffStart)) & 0xf) == 0, "Misaligned Label");

            to = reloc.getLabelOrigPC() - 1;
            
            CopyPartialBufferAndCalculateCRC(&dst_p, dst_size, from, to, pShortenedBufferCRC);
            srcBufferCrc = CalculateCRC(srcBufferCrc, to - from + 1, from);

#ifdef  ENABLE_DEBUG_CONFIG_OPTIONS
            if (PHASE_TRACE(Js::LoopAlignPhase, this->m_func))
            {
                globalTotalBytesInserted += nop_count;

                OUTPUT_VERBOSE_TRACE(Js::LoopAlignPhase, _u("func: %s, bytes inserted: %d, bytes inserted %%:%.4f, total bytes inserted:%d, total bytes inserted %%:%.4f\n"),
                    this->m_func->GetJnFunction()->GetDisplayName(), nop_count, (float)nop_count / newCodeSize * 100, globalTotalBytesInserted, (float)globalTotalBytesInserted / (globalTotalBytesWithoutShortening - globalTotalBytesSaved) * 100);
                Output::Flush();
            }
#endif
            BYTE * tmpDst_p = dst_p;
            InsertNopsForLabelAlignment(nop_count, &dst_p);
            *pShortenedBufferCRC = CalculateCRC(*pShortenedBufferCRC, nop_count, tmpDst_p);

            dst_size -= nop_count;
            from = to + 1;
        }
    }
    // copy last chunk
    //Exclude jumpTable content from CRC calculation. 
    //Though jumpTable is not part of the encoded bytes, codeSize has jumpTableSize included in it.
    CopyPartialBufferAndCalculateCRC(&dst_p, dst_size, from, buffStart + *codeSize - 1, pShortenedBufferCRC, jumpTableSize);
    srcBufferCrc = CalculateCRC(srcBufferCrc, buffStart + *codeSize - from - jumpTableSize, from);

    m_encoderMD.UpdateRelocListWithNewBuffer(relocList, tmpBuffer, buffStart, buffEnd);

    if (srcBufferCrc != bufferCrcToValidate)
    {
        Assert(false);
        Fatal();
    }

    // switch buffers
    *codeStart = tmpBuffer;
    *codeSize = newCodeSize;

    return true;
}

BYTE Encoder::FindNopCountFor16byteAlignment(size_t address)
{
    return (16 - (BYTE) (address & 0xf)) % 16;
}

void Encoder::CopyPartialBufferAndCalculateCRC(BYTE ** ptrDstBuffer, size_t &dstSize, BYTE * srcStart, BYTE * srcEnd, uint* pBufferCRC, size_t jumpTableSize)
{
    BYTE * destBuffer = *ptrDstBuffer;

    size_t srcSize = srcEnd - srcStart + 1;
    Assert(dstSize >= srcSize);
    memcpy_s(destBuffer, dstSize, srcStart, srcSize);

    Assert(srcSize >= jumpTableSize);

    //Exclude the jump table content (which is at the end of the buffer) for calculating CRC - at this point.
    *pBufferCRC = CalculateCRC(*pBufferCRC, srcSize - jumpTableSize, destBuffer);

    *ptrDstBuffer += srcSize;
    dstSize -= srcSize;
}

void Encoder::InsertNopsForLabelAlignment(int nopCount, BYTE ** ptrDstBuffer)
{
    // write NOPs
    for (int32 i = 0; i < nopCount; i++, (*ptrDstBuffer)++)
    {
        **ptrDstBuffer = 0x90;
    }
}
void Encoder::revertRelocList()
{
    RelocList* relocList = m_encoderMD.GetRelocList();

    for (int32 i = 0; i < relocList->Count(); i++)
    {
        relocList->Item(i).revert();
    }
}

template <bool restore>
void Encoder::CopyMaps(OffsetList **m_origInlineeFrameRecords
    , OffsetList **m_origInlineeFrameMap
    , OffsetList **m_origPragmaInstrToRecordOffset
    , OffsetList **m_origOffsetBuffer
    )
{
    InlineeFrameRecords *recList = m_inlineeFrameRecords;
    InlineeFrameMap *mapList = m_inlineeFrameMap;
    PragmaInstrList *pInstrList = m_pragmaInstrToRecordOffset;

    OffsetList *origRecList, *origMapList, *origPInstrList;
    if (!restore)
    {
        Assert(*m_origInlineeFrameRecords == nullptr);
        Assert(*m_origInlineeFrameMap == nullptr);
        Assert(*m_origPragmaInstrToRecordOffset == nullptr);

        *m_origInlineeFrameRecords = origRecList = Anew(m_tempAlloc, OffsetList, m_tempAlloc);
        *m_origInlineeFrameMap = origMapList = Anew(m_tempAlloc, OffsetList, m_tempAlloc);
        *m_origPragmaInstrToRecordOffset = origPInstrList = Anew(m_tempAlloc, OffsetList, m_tempAlloc);

#if DBG_DUMP
        Assert((*m_origOffsetBuffer) == nullptr);
        *m_origOffsetBuffer = Anew(m_tempAlloc, OffsetList, m_tempAlloc);
#endif
    }
    else
    {
        Assert((*m_origInlineeFrameRecords) && (*m_origInlineeFrameMap) && (*m_origPragmaInstrToRecordOffset));
        origRecList = *m_origInlineeFrameRecords;
        origMapList = *m_origInlineeFrameMap;
        origPInstrList = *m_origPragmaInstrToRecordOffset;
        Assert(origRecList->Count() == recList->Count());
        Assert(origMapList->Count() == mapList->Count());
        Assert(origPInstrList->Count() == pInstrList->Count());

#if DBG_DUMP
        Assert(m_origOffsetBuffer);
        Assert((uint32)(*m_origOffsetBuffer)->Count() == m_instrNumber);
#endif
    }

    for (int i = 0; i < recList->Count(); i++)
    {
        if (!restore)
        {
            origRecList->Add(recList->Item(i)->inlineeStartOffset);
        }
        else
        {
            recList->Item(i)->inlineeStartOffset = origRecList->Item(i);
        }
    }

    for (int i = 0; i < mapList->Count(); i++)
    {
        if (!restore)
        {
            origMapList->Add(mapList->Item(i).offset);
        }
        else
        {
            mapList->Item(i).offset = origMapList->Item(i);
        }
    }

    for (int i = 0; i < pInstrList->Count(); i++)
    {
        if (!restore)
        {
            origPInstrList->Add(pInstrList->Item(i)->m_offsetInBuffer);
        }
        else
        {
            pInstrList->Item(i)->m_offsetInBuffer = origPInstrList->Item(i);
        }
    }

    if (restore)
    {
        (*m_origInlineeFrameRecords)->Delete();
        (*m_origInlineeFrameMap)->Delete();
        (*m_origPragmaInstrToRecordOffset)->Delete();
        (*m_origInlineeFrameRecords) = nullptr;
        (*m_origInlineeFrameMap) = nullptr;
        (*m_origPragmaInstrToRecordOffset) = nullptr;
    }

#if DBG_DUMP
    for (uint i = 0; i < m_instrNumber; i++)
    {
        if (!restore)
        {
            (*m_origOffsetBuffer)->Add(m_offsetBuffer[i]);
        }
        else
        {
            m_offsetBuffer[i] = (*m_origOffsetBuffer)->Item(i);
        }
    }

    if (restore)
    {
        (*m_origOffsetBuffer)->Delete();
        (*m_origOffsetBuffer) = nullptr;
    }
#endif
}

#endif

void Encoder::RecordBailout(IR::Instr* instr, uint32 currentOffset)
{
    BailOutInfo* bailoutInfo = instr->GetBailOutInfo();
    if (bailoutInfo->bailOutRecord == nullptr)
    {
        return;
    }
#if DBG_DUMP
    if (PHASE_DUMP(Js::LazyBailoutPhase, m_func))
    {
        Output::Print(_u("Offset: %u Instr: "), currentOffset);
        instr->Dump();
        Output::Print(_u("Bailout label: "));
        bailoutInfo->bailOutInstr->Dump();
    }
#endif
    Assert(bailoutInfo->bailOutInstr->IsLabelInstr());
    LazyBailOutRecord record(currentOffset, (BYTE*)bailoutInfo->bailOutInstr, bailoutInfo->bailOutRecord);
    m_bailoutRecordMap->Add(record);
}

#if DBG_DUMP
void Encoder::DumpInlineeFrameMap(size_t baseAddress)
{
    Output::Print(_u("Inlinee frame info mapping\n"));
    Output::Print(_u("---------------------------------------\n"));
    m_inlineeFrameMap->Map([=](uint index, NativeOffsetInlineeFramePair& pair) {
        Output::Print(_u("%Ix"), baseAddress + pair.offset);
        Output::SkipToColumn(20);
        if (pair.record)
        {
            pair.record->Dump();
        }
        else
        {
            Output::Print(_u("<NULL>"));
        }
        Output::Print(_u("\n"));
    });
}
#endif
