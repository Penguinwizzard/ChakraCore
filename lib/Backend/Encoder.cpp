//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "BackEnd.h"

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
    NoRecoverMemoryArenaAllocator localAlloc(L"BE-Encoder", m_func->m_alloc->GetPageAllocator(), Js::Throw::OutOfMemory);
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
                        encoderMD->AppendRelocEntry(RelocTypeLabelUse, (void*) (offset));
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

#if DBG_DUMP
            if (PHASE_TRACE(Js::EncoderPhase, this->m_func))
            {
                instr->Dump((IRDumpFlags)(IRDumpFlags_SimpleForm | IRDumpFlags_SkipEndLine | IRDumpFlags_SkipByteCodeOffset));
                Output::SkipToColumn(80);
                for (BYTE * current = m_pc; current < m_pc + count; current++)
                {
                    Output::Print(L"%02X ", *current);
                }
                Output::Print(L"\n");
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

#if defined(_M_IX86) || defined(_M_X64)
    BOOL isSuccessBrShortAndLoopAlign = false;
    // Shorten branches. ON by default
    if (!PHASE_OFF(Js::BrShortenPhase, m_func))
    {
        isSuccessBrShortAndLoopAlign = ShortenBranchesAndLabelAlign(&m_encodeBuffer, &codeSize);
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
    OUTPUT_VERBOSE_TRACE(Js::EmitterPhase, L"PDATA count:%u\n", pdataCount);
    OUTPUT_VERBOSE_TRACE(Js::EmitterPhase, L"Size of XDATA:%u\n", xdataSize);
    OUTPUT_VERBOSE_TRACE(Js::EmitterPhase, L"Size of code:%u\n", codeSize);

    TryCopyAndAddRelocRecordsForSwitchJumpTableEntries(m_encodeBuffer, codeSize, jumpTableListForSwitchStatement, totalJmpTableSizeInBytes);

    workItem->RecordNativeCodeSize(m_func, (DWORD)codeSize, pdataCount, xdataSize);

    this->m_bailoutRecordMap->MapAddress([=](int index, LazyBailOutRecord* record)
    {
        this->m_encoderMD.AddLabelReloc((BYTE*)&record->instructionPointer);
    });

    // Relocs
    m_encoderMD.ApplyRelocs((size_t) workItem->GetCodeAddress());

    BYTE * buff2 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4, codeSize, buff2, codeSize);

    BYTE * buff5 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5, codeSize, buff3, codeSize);
    BYTE * buff6 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6, codeSize, buff2, codeSize);
    BYTE * buff7 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7, codeSize, buff4, codeSize);
    BYTE * buff8 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8, codeSize, buff7, codeSize);

    BYTE * buff9 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9, codeSize, buff4, codeSize);
    BYTE * buff10 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10, codeSize, buff3, codeSize);
    BYTE * buff11 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11, codeSize, buff8, codeSize);
    BYTE * buff12 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12, codeSize, buff3, codeSize);

    BYTE * buff13 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13, codeSize, buff8, codeSize);
    BYTE * buff14 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14, codeSize, buff7, codeSize);
    BYTE * buff15 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15, codeSize, buff6, codeSize);
    BYTE * buff16 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16, codeSize, buff4, codeSize);

    BYTE * buff20 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff20, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff30 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff30, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff40 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff40, codeSize, buff2, codeSize);

    BYTE * buff50 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff50, codeSize, buff3, codeSize);
    BYTE * buff60 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff60, codeSize, buff2, codeSize);
    BYTE * buff70 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff70, codeSize, buff4, codeSize);
    BYTE * buff80 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff80, codeSize, buff7, codeSize);

    BYTE * buff90 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff90, codeSize, buff4, codeSize);
    BYTE * buff100 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff100, codeSize, buff3, codeSize);
    BYTE * buff110 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff110, codeSize, buff8, codeSize);
    BYTE * buff120 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff120, codeSize, buff3, codeSize);

    BYTE * buff130 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff130, codeSize, buff8, codeSize);
    BYTE * buff140 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff140, codeSize, buff7, codeSize);
    BYTE * buff150 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff150, codeSize, buff6, codeSize);
    BYTE * buff160 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff160, codeSize, buff4, codeSize);

    BYTE * buff200 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff200, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff300 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff300, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff400 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff400, codeSize, buff2, codeSize);

    BYTE * buff500 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff500, codeSize, buff3, codeSize);
    BYTE * buff600 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff600, codeSize, buff2, codeSize);
    BYTE * buff700 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff700, codeSize, buff4, codeSize);
    BYTE * buff800 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff800, codeSize, buff7, codeSize);

    BYTE * buff900 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff900, codeSize, buff4, codeSize);
    BYTE * buff1000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1000, codeSize, buff3, codeSize);
    BYTE * buff1100 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1100, codeSize, buff8, codeSize);
    BYTE * buff1200 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1200, codeSize, buff3, codeSize);

    BYTE * buff1300 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1300, codeSize, buff8, codeSize);
    BYTE * buff1400 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1400, codeSize, buff7, codeSize);
    BYTE * buff1500 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1500, codeSize, buff6, codeSize);
    BYTE * buff1600 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1600, codeSize, buff4, codeSize);

    BYTE * buff2000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2000, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3000, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4000, codeSize, buff2, codeSize);

    BYTE * buff5000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5000, codeSize, buff3, codeSize);
    BYTE * buff6000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6000, codeSize, buff2, codeSize);
    BYTE * buff7000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7000, codeSize, buff4, codeSize);
    BYTE * buff8000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8000, codeSize, buff7, codeSize);

    BYTE * buff9000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9000, codeSize, buff4, codeSize);
    BYTE * buff10000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10000, codeSize, buff3, codeSize);
    BYTE * buff11000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11000, codeSize, buff8, codeSize);
    BYTE * buff12000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12000, codeSize, buff3, codeSize);

    BYTE * buff13000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13000, codeSize, buff8, codeSize);
    BYTE * buff14000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14000, codeSize, buff7, codeSize);
    BYTE * buff15000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15000, codeSize, buff6, codeSize);
    BYTE * buff16000 = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16000, codeSize, buff4, codeSize);

    BYTE * buff2a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4a, codeSize, buff2, codeSize);

    BYTE * buff5a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5a, codeSize, buff3, codeSize);
    BYTE * buff6a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6a, codeSize, buff2, codeSize);
    BYTE * buff7a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7a, codeSize, buff4, codeSize);
    BYTE * buff8a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8a, codeSize, buff7, codeSize);

    BYTE * buff9a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9a, codeSize, buff4, codeSize);
    BYTE * buff10a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10a, codeSize, buff3, codeSize);
    BYTE * buff11a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11a, codeSize, buff8, codeSize);
    BYTE * buff12a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12a, codeSize, buff3, codeSize);

    BYTE * buff13a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13a, codeSize, buff8, codeSize);
    BYTE * buff14a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14a, codeSize, buff7, codeSize);
    BYTE * buff15a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15a, codeSize, buff6, codeSize);
    BYTE * buff16a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16a, codeSize, buff4, codeSize);

    BYTE * buff20a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff20a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff30a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff30a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff40a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff40a, codeSize, buff2, codeSize);

    BYTE * buff50a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff50a, codeSize, buff3, codeSize);
    BYTE * buff60a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff60a, codeSize, buff2, codeSize);
    BYTE * buff70a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff70a, codeSize, buff4, codeSize);
    BYTE * buff80a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff80a, codeSize, buff7, codeSize);

    BYTE * buff90a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff90a, codeSize, buff4, codeSize);
    BYTE * buff100a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff100a, codeSize, buff3, codeSize);
    BYTE * buff110a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff110a, codeSize, buff8, codeSize);
    BYTE * buff120a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff120a, codeSize, buff3, codeSize);

    BYTE * buff130a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff130a, codeSize, buff8, codeSize);
    BYTE * buff140a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff140a, codeSize, buff7, codeSize);
    BYTE * buff150a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff150a, codeSize, buff6, codeSize);
    BYTE * buff160a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff160a, codeSize, buff4, codeSize);

    BYTE * buff200a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff200a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff300a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff300a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff400a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff400a, codeSize, buff2, codeSize);

    BYTE * buff500a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff500a, codeSize, buff3, codeSize);
    BYTE * buff600a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff600a, codeSize, buff2, codeSize);
    BYTE * buff700a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff700a, codeSize, buff4, codeSize);
    BYTE * buff800a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff800a, codeSize, buff7, codeSize);

    BYTE * buff900a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff900a, codeSize, buff4, codeSize);
    BYTE * buff1000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1000a, codeSize, buff3, codeSize);
    BYTE * buff1100a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1100a, codeSize, buff8, codeSize);
    BYTE * buff1200a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1200a, codeSize, buff3, codeSize);

    BYTE * buff1300a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1300a, codeSize, buff8, codeSize);
    BYTE * buff1400a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1400a, codeSize, buff7, codeSize);
    BYTE * buff1500a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1500a, codeSize, buff6, codeSize);
    BYTE * buff1600a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1600a, codeSize, buff4, codeSize);

    BYTE * buff2000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2000a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3000a, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4000a, codeSize, buff2, codeSize);

    BYTE * buff5000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5000a, codeSize, buff3, codeSize);
    BYTE * buff6000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6000a, codeSize, buff2, codeSize);
    BYTE * buff7000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7000a, codeSize, buff4, codeSize);
    BYTE * buff8000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8000a, codeSize, buff7, codeSize);

    BYTE * buff9000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9000a, codeSize, buff4, codeSize);
    BYTE * buff10000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10000a, codeSize, buff3, codeSize);
    BYTE * buff11000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11000a, codeSize, buff8, codeSize);
    BYTE * buff12000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12000a, codeSize, buff3, codeSize);

    BYTE * buff13000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13000a, codeSize, buff8, codeSize);
    BYTE * buff14000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14000a, codeSize, buff7, codeSize);
    BYTE * buff15000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15000a, codeSize, buff6, codeSize);
    BYTE * buff16000a = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16000a, codeSize, buff4, codeSize);

    BYTE * buff2b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4b, codeSize, buff2, codeSize);

    BYTE * buff5b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5b, codeSize, buff3, codeSize);
    BYTE * buff6b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6b, codeSize, buff2, codeSize);
    BYTE * buff7b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7b, codeSize, buff4, codeSize);
    BYTE * buff8b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8b, codeSize, buff7, codeSize);

    BYTE * buff9b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9b, codeSize, buff4, codeSize);
    BYTE * buff10b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10b, codeSize, buff3, codeSize);
    BYTE * buff11b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11b, codeSize, buff8, codeSize);
    BYTE * buff12b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12b, codeSize, buff3, codeSize);

    BYTE * buff13b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13b, codeSize, buff8, codeSize);
    BYTE * buff14b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14b, codeSize, buff7, codeSize);
    BYTE * buff15b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15b, codeSize, buff6, codeSize);
    BYTE * buff16b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16b, codeSize, buff4, codeSize);

    BYTE * buff20b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff20b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff30b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff30b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff40b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff40b, codeSize, buff2, codeSize);

    BYTE * buff50b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff50b, codeSize, buff3, codeSize);
    BYTE * buff60b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff60b, codeSize, buff2, codeSize);
    BYTE * buff70b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff70b, codeSize, buff4, codeSize);
    BYTE * buff80b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff80b, codeSize, buff7, codeSize);

    BYTE * buff90b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff90b, codeSize, buff4, codeSize);
    BYTE * buff100b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff100b, codeSize, buff3, codeSize);
    BYTE * buff110b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff110b, codeSize, buff8, codeSize);
    BYTE * buff120b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff120b, codeSize, buff3, codeSize);

    BYTE * buff130b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff130b, codeSize, buff8, codeSize);
    BYTE * buff140b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff140b, codeSize, buff7, codeSize);
    BYTE * buff150b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff150b, codeSize, buff6, codeSize);
    BYTE * buff160b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff160b, codeSize, buff4, codeSize);

    BYTE * buff200b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff200b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff300b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff300b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff400b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff400b, codeSize, buff2, codeSize);

    BYTE * buff500b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff500b, codeSize, buff3, codeSize);
    BYTE * buff600b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff600b, codeSize, buff2, codeSize);
    BYTE * buff700b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff700b, codeSize, buff4, codeSize);
    BYTE * buff800b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff800b, codeSize, buff7, codeSize);

    BYTE * buff900b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff900b, codeSize, buff4, codeSize);
    BYTE * buff1000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1000b, codeSize, buff3, codeSize);
    BYTE * buff1100b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1100b, codeSize, buff8, codeSize);
    BYTE * buff1200b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1200b, codeSize, buff3, codeSize);
    /*
    BYTE * buff1300b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1300b, codeSize, buff8, codeSize);
    BYTE * buff1400b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1400b, codeSize, buff7, codeSize);
    BYTE * buff1500b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1500b, codeSize, buff6, codeSize);
    BYTE * buff1600b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1600b, codeSize, buff4, codeSize);

    BYTE * buff2000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2000b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3000b, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4000b, codeSize, buff2, codeSize);

    BYTE * buff5000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5000b, codeSize, buff3, codeSize);
    BYTE * buff6000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6000b, codeSize, buff2, codeSize);
    BYTE * buff7000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7000b, codeSize, buff4, codeSize);
    BYTE * buff8000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8000b, codeSize, buff7, codeSize);

    BYTE * buff9000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9000b, codeSize, buff4, codeSize);
    BYTE * buff10000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10000b, codeSize, buff3, codeSize);
    BYTE * buff11000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11000b, codeSize, buff8, codeSize);
    BYTE * buff12000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12000b, codeSize, buff3, codeSize);

    BYTE * buff13000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13000b, codeSize, buff8, codeSize);
    BYTE * buff14000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14000b, codeSize, buff7, codeSize);
    BYTE * buff15000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15000b, codeSize, buff6, codeSize);
    BYTE * buff16000b = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16000b, codeSize, buff4, codeSize);

    BYTE * buff2ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4ab, codeSize, buff2, codeSize);

    BYTE * buff5ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5ab, codeSize, buff3, codeSize);
    BYTE * buff6ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6ab, codeSize, buff2, codeSize);
    BYTE * buff7ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7ab, codeSize, buff4, codeSize);
    BYTE * buff8ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8ab, codeSize, buff7, codeSize);

    BYTE * buff9ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9ab, codeSize, buff4, codeSize);
    BYTE * buff10ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10ab, codeSize, buff3, codeSize);
    BYTE * buff11ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11ab, codeSize, buff8, codeSize);
    BYTE * buff12ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12ab, codeSize, buff3, codeSize);

    BYTE * buff13ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13ab, codeSize, buff8, codeSize);
    BYTE * buff14ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14ab, codeSize, buff7, codeSize);
    BYTE * buff15ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15ab, codeSize, buff6, codeSize);
    BYTE * buff16ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16ab, codeSize, buff4, codeSize);

    BYTE * buff20ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff20ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff30ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff30ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff40ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff40ab, codeSize, buff2, codeSize);

    BYTE * buff50ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff50ab, codeSize, buff3, codeSize);
    BYTE * buff60ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff60ab, codeSize, buff2, codeSize);
    BYTE * buff70ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff70ab, codeSize, buff4, codeSize);
    BYTE * buff80ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff80ab, codeSize, buff7, codeSize);

    BYTE * buff90ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff90ab, codeSize, buff4, codeSize);
    BYTE * buff100ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff100ab, codeSize, buff3, codeSize);
    BYTE * buff110ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff110ab, codeSize, buff8, codeSize);
    BYTE * buff120ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff120ab, codeSize, buff3, codeSize);

    BYTE * buff130ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff130ab, codeSize, buff8, codeSize);
    BYTE * buff140ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff140ab, codeSize, buff7, codeSize);
    BYTE * buff150ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff150ab, codeSize, buff6, codeSize);
    BYTE * buff160ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff160ab, codeSize, buff4, codeSize);

    BYTE * buff200ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff200ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff300ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff300ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff400ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff400ab, codeSize, buff2, codeSize);

    BYTE * buff500ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff500ab, codeSize, buff3, codeSize);
    BYTE * buff600ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff600ab, codeSize, buff2, codeSize);
    BYTE * buff700ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff700ab, codeSize, buff4, codeSize);
    BYTE * buff800ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff800ab, codeSize, buff7, codeSize);

    BYTE * buff900ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff900ab, codeSize, buff4, codeSize);
    BYTE * buff1000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1000ab, codeSize, buff3, codeSize);
    BYTE * buff1100ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1100ab, codeSize, buff8, codeSize);
    BYTE * buff1200ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1200ab, codeSize, buff3, codeSize);

    BYTE * buff1300ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1300ab, codeSize, buff8, codeSize);
    BYTE * buff1400ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1400ab, codeSize, buff7, codeSize);
    BYTE * buff1500ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1500ab, codeSize, buff6, codeSize);
    BYTE * buff1600ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff1600ab, codeSize, buff4, codeSize);

    BYTE * buff2000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff2000ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff3000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff3000ab, codeSize, m_encodeBuffer, codeSize);
    BYTE * buff4000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff4000ab, codeSize, buff2, codeSize);

    BYTE * buff5000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff5000ab, codeSize, buff3, codeSize);
    BYTE * buff6000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff6000ab, codeSize, buff2, codeSize);
    BYTE * buff7000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff7000ab, codeSize, buff4, codeSize);
    BYTE * buff8000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff8000ab, codeSize, buff7, codeSize);

    BYTE * buff9000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff9000ab, codeSize, buff4, codeSize);
    BYTE * buff10000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff10000ab, codeSize, buff3, codeSize);
    BYTE * buff11000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff11000ab, codeSize, buff8, codeSize);
    BYTE * buff12000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff12000ab, codeSize, buff3, codeSize);

    BYTE * buff13000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff13000ab, codeSize, buff8, codeSize);
    BYTE * buff14000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff14000ab, codeSize, buff7, codeSize);
    BYTE * buff15000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff15000ab, codeSize, buff6, codeSize);
    BYTE * buff16000ab = AnewArray(m_tempAlloc, BYTE, codeSize);
    memcpy_s(buff16000ab, codeSize, buff4, codeSize);*/

    workItem->RecordNativeCode(m_func, m_encodeBuffer);

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
        AssertMsg(!this->m_func->IsLoopBody(), "Loop body does not support inlining.");
        Js::FunctionEntryPointInfo* functionEntryPointInfo = static_cast<Js::FunctionEntryPointInfo*>(entryPointInfo);
        functionEntryPointInfo->RecordInlineeFrameMap(m_inlineeFrameMap);
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
            wchar_t debugStringBuffer[MAX_FUNCTION_BODY_DEBUG_STRING_SIZE];
            Output::Print(L"PinnedTypes: function %s(%s) pinned %d types.\n",
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
            Output::Print(L"%12IX  ", m_offsetBuffer[m_instrNumber++] + (BYTE *)workItem->GetCodeAddress());
#else
            Output::Print(L"%8IX  ", m_offsetBuffer[m_instrNumber++] + (BYTE *)workItem->GetCodeAddress());
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
    JitArenaAllocator * allocator = this->m_func->m_alloc;
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
            encoderMD->AppendRelocEntry(RelocTypeLabelUse, addressOfJmpTableEntry);
#endif
        }

        jmpTableStartAddress += (jmpTableSizeInBytes);

        BranchJumpTableWrapper::Delete(allocator, branchJumpTableWrapper);
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
    if (!this->m_func->IsLoopBody() && !this->m_func->IsSimpleJit())
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
///----------------------------------------------------------------------------
///
/// EncoderMD::ShortenBranchesAndLabelAlign
/// We try to shorten branches if the label instr is within 8-bits target range (-128 to 127)
/// and fix the relocList accordingly.
/// Also align LoopTop Label and TryCatchLabel
///----------------------------------------------------------------------------
BOOL
Encoder::ShortenBranchesAndLabelAlign(BYTE **codeStart, ptrdiff_t *codeSize)
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

    RelocList* relocList = m_encoderMD.GetRelocList();
    Assert(relocList != nullptr);
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
        OUTPUT_VERBOSE_TRACE(Js::BrShortenPhase, L"func: %s, bytes saved: %d, bytes saved %%:%.2f, total bytes saved: %d, total bytes saved%%: %.2f, BR shortened: %d\n",
            this->m_func->GetJnFunction()->GetDisplayName(), (*codeSize - newCodeSize), ((float)*codeSize - newCodeSize) / *codeSize * 100,
            globalTotalBytesSaved, ((float)globalTotalBytesSaved) / globalTotalBytesWithoutShortening * 100 , brShortenedCount);
        Output::Flush();
    }
#endif

    // At this point BRs are marked to be shortened, and relocList offsets are adjusted to new instruction length.
    // Next, we re-write the code to shorten the BRs and adjust relocList offsets to point to new buffer.
    // We also write NOPs for aligned loops.
    BYTE* tmpBuffer = AnewArray(m_tempAlloc, BYTE, newCodeSize);

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
            Assert(dst_size >= src_size);

            memcpy_s(dst_p, dst_size, from, src_size);
            dst_p += src_size;
            dst_size -= src_size;

            // fix the BR
            // write new opcode
            *dst_p = (*opcodeByte == 0xe9) ? (BYTE)0xeb : (BYTE)(*opcodeByte - 0x10);
            dst_p += 2; // 1 byte for opcode + 1 byte for imm8
            dst_size -= 2;
            from = (BYTE*)reloc.m_origPtr + 4;
        }
        // insert NOPs for aligned labels
        else if ((!PHASE_OFF(Js::LoopAlignPhase, m_func) && reloc.isAlignedLabel()) && reloc.getLabelNopCount() > 0)
        {
            IR::LabelInstr *label = reloc.getLabel();
            BYTE nop_count = reloc.getLabelNopCount();

            AssertMsg((BYTE*)label < buffStart || (BYTE*)label >= buffEnd, "Invalid label pointer.");
            AssertMsg((((uint32)(label->GetPC() - buffStart)) & 0xf) == 0, "Misaligned Label");

            to = reloc.getLabelOrigPC() - 1;
            CopyPartialBuffer(&dst_p, dst_size, from, to);

#ifdef  ENABLE_DEBUG_CONFIG_OPTIONS
            if (PHASE_TRACE(Js::LoopAlignPhase, this->m_func))
            {
                globalTotalBytesInserted += nop_count;

                OUTPUT_VERBOSE_TRACE(Js::LoopAlignPhase, L"func: %s, bytes inserted: %d, bytes inserted %%:%.4f, total bytes inserted:%d, total bytes inserted %%:%.4f\n",
                    this->m_func->GetJnFunction()->GetDisplayName(), nop_count, (float)nop_count / newCodeSize * 100, globalTotalBytesInserted, (float)globalTotalBytesInserted / (globalTotalBytesWithoutShortening - globalTotalBytesSaved) * 100);
                Output::Flush();
            }
#endif
            InsertNopsForLabelAlignment(nop_count, &dst_p);

            dst_size -= nop_count;
            from = to + 1;
        }
    }
    // copy last chunk
    CopyPartialBuffer(&dst_p, dst_size, from, buffStart + *codeSize - 1);

    m_encoderMD.UpdateRelocListWithNewBuffer(relocList, tmpBuffer, buffStart, buffEnd);

    // switch buffers
    *codeStart = tmpBuffer;
    *codeSize = newCodeSize;

    return true;
}

BYTE Encoder::FindNopCountFor16byteAlignment(size_t address)
{
    return (16 - (BYTE) (address & 0xf)) % 16;
}

void Encoder::CopyPartialBuffer(BYTE ** ptrDstBuffer, size_t &dstSize, BYTE * srcStart, BYTE * srcEnd)
{
    BYTE * destBuffer = *ptrDstBuffer;

    size_t srcSize = srcEnd - srcStart + 1;
    Assert(dstSize >= srcSize);
    memcpy_s(destBuffer, dstSize, srcStart, srcSize);
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
        Assert(m_origOffsetBuffer)
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
        Output::Print(L"Offset: %u Instr: ", currentOffset);
        instr->Dump();
        Output::Print(L"Bailout label: ");
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
    Output::Print(L"Inlinee frame info mapping\n");
    Output::Print(L"---------------------------------------\n");
    m_inlineeFrameMap->Map([=](uint index, NativeOffsetInlineeFramePair& pair) {
        Output::Print(L"%Ix", baseAddress + pair.offset);
        Output::SkipToColumn(20);
        if (pair.record)
        {
            pair.record->Dump();
        }
        else
        {
            Output::Print(L"<NULL>");
        }
        Output::Print(L"\n");
    });
}
#endif
