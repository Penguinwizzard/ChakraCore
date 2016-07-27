//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#include "Backend.h"


const char NativeCodeData::DataDesc_None[] = "";
const char NativeCodeData::DataDesc_InlineeFrameRecord_ArgOffsets[] = "";
const char NativeCodeData::DataDesc_InlineeFrameRecord_Constants[] = "";
const char NativeCodeData::DataDesc_BailoutInfo_CotalOutParamCount[] = "";
const char NativeCodeData::DataDesc_ArgOutOffsetInfo_StartCallOutParamCounts[] = "";
const char NativeCodeData::DataDesc_ArgOutOffsetInfo_StartCallArgRestoreAdjustCounts[] = "";
const char NativeCodeData::DataDesc_LowererMD_LoadFloatValue_Float[] = "";
const char NativeCodeData::DataDesc_LowererMD_LoadFloatValue_Double[] = "";
const char NativeCodeData::DataDesc_LowererMD_EmitLoadFloatCommon_Double[] = "";
const char NativeCodeData::DataDesc_LowererMD_Simd128LoadConst[] = "";

NativeCodeData::NativeCodeData(DataChunk * chunkList) : chunkList(chunkList)
{
#ifdef PERF_COUNTERS
    this->size = 0;
#endif
}

NativeCodeData::~NativeCodeData()
{
    NativeCodeData::DeleteChunkList(this->chunkList);
    PERF_COUNTER_SUB(Code, DynamicNativeCodeDataSize, this->size);
    PERF_COUNTER_SUB(Code, TotalNativeCodeDataSize, this->size);
}

void
NativeCodeData::AddFixupEntry(void* targetAddr, void* addrToFixup, void* startAddress, DataChunk * chunkList)
{
    return NativeCodeData::AddFixupEntry(targetAddr, targetAddr, addrToFixup, startAddress, chunkList);
}

// targetAddr: target address
// targetStartAddr: target start address, some fied might reference to middle of another data chunk, like outParamOffsets
// startAddress: current data start address
// addrToFixup: address that currently pointing to dataAddr, which need to be updated
void
NativeCodeData::AddFixupEntry(void* targetAddr, void* targetStartAddr, void* addrToFixup, void* startAddress, DataChunk * chunkList)
{
    Assert(addrToFixup >= startAddress);
    Assert(((__int64)addrToFixup) % sizeof(void*) == 0);

    if (targetAddr == nullptr)
    {
        return;
    }

    Assert(targetStartAddr);
    
    unsigned int inDataOffset = (unsigned int)((char*)targetAddr - (char*)targetStartAddr);
    DataChunk* targetChunk = NativeCodeData::GetDataChunk(targetStartAddr);
    Assert(targetChunk->len >= inDataOffset);

#if DBG
    bool foundTargetChunk = false;
    while (chunkList) 
    {
        foundTargetChunk |= (chunkList == targetChunk);
        chunkList = chunkList->next;
    }
    AssertMsg(foundTargetChunk, "current pointer is not allocated with NativeCodeData allocator?"); // change to valid check instead of assertion?
#endif    

    DataChunk* chunk = NativeCodeData::GetDataChunk(startAddress);

    NativeDataFixupEntry* entry = (NativeDataFixupEntry*)midl_user_allocate(sizeof(NativeDataFixupEntry));
    entry->addrOffset = (unsigned int)((__int64)addrToFixup - (__int64)startAddress);
    Assert(entry->addrOffset <= chunk->len - sizeof(void*));    

    entry->targetTotalOffset = targetChunk->offset + inDataOffset;
    entry->next = chunk->fixupList;
    chunk->fixupList = entry;

#if DBG
    if (PHASE_TRACE1(Js::NativeCodeDataPhase))
    {
        Output::Print(L"NativeCodeData Add Fixup: %p(%p+%d, chunk:%p)  -->  %p(chunk:%p)  %S\n", 
            addrToFixup, startAddress, entry->addrOffset, (void*)chunk, targetAddr, (void*)targetChunk, chunk->dataType);
    }
#endif
}

void
NativeCodeData::AddFixupEntryForPointerArray(void* startAddress, DataChunk * chunkList)
{
    DataChunk* chunk = NativeCodeData::GetDataChunk(startAddress);
    Assert(chunk->len % sizeof(void*) == 0);
    for (unsigned int i = 0; i < chunk->len / sizeof(void*); i++)
    {
        size_t offset = i * sizeof(void*);
        void* targetAddr = *(void**)((char*)startAddress + offset);

        if (targetAddr == nullptr)
        {
            continue;
        }

        DataChunk* targetChunk = NativeCodeData::GetDataChunk(targetAddr);

#if DBG
        bool foundTargetChunk = false;
        DataChunk* chunk1 = chunkList;
        while (chunk1 && !foundTargetChunk)
        {
            foundTargetChunk = (chunk1 == targetChunk);
            chunk1 = chunk1->next;
        }
        AssertMsg(foundTargetChunk, "current pointer is not allocated with NativeCodeData allocator?"); // change to valid check instead of assertion?
#endif

        NativeDataFixupEntry* entry = (NativeDataFixupEntry*)midl_user_allocate(sizeof(NativeDataFixupEntry));
        entry->addrOffset = (unsigned int)offset;
        entry->targetTotalOffset = targetChunk->offset;
        entry->next = chunk->fixupList;
        chunk->fixupList = entry;

#if DBG
        if (PHASE_TRACE1(Js::NativeCodeDataPhase))
        {
            Output::Print(L"NativeCodeData Add Fixup: %p[%d](+%d, chunk:%p)  -->  %p(chunk:%p)  %S\n",
                startAddress, i, entry->addrOffset, (void*)chunk, targetAddr, (void*)targetChunk, chunk->dataType);
        }
#endif
    }
}

wchar_t* 
NativeCodeData::GetDataDescription(void* data, JitArenaAllocator * alloc)
{
    auto chunk = GetDataChunk(data);
    wchar_t buf[1024] = { 0 };
#if DBG
    swprintf_s(buf, L"%hs, NativeCodeData: index: %x, len: %x, offset: +%x", chunk->dataType, chunk->allocIndex, chunk->len, chunk->offset);
#else
    swprintf_s(buf, L"NativeCodeData: index: %x, len: %x, offset: +%x", chunk->allocIndex, chunk->len, chunk->offset);
#endif
    auto len = wcslen(buf) + 1;
    auto desc = JitAnewArray(alloc, wchar_t, len);
    wcscpy(desc, buf);
    return desc;
}

void
NativeCodeData::VerifyExistFixupEntry(void* targetAddr, void* addrToFixup, void* startAddress)
{
    DataChunk* chunk = NativeCodeData::GetDataChunk(startAddress);
    DataChunk* targetChunk = NativeCodeData::GetDataChunk(targetAddr);
    if (chunk->len == 0) 
    {
        return;
    }
    unsigned int offset = (unsigned int)((char*)addrToFixup - (char*)startAddress);
    Assert(offset <= chunk->len);

    NativeDataFixupEntry* entry = chunk->fixupList;
    while (entry)
    {
        if (entry->addrOffset == offset)
        {
            Assert(entry->targetTotalOffset == targetChunk->offset);
            return;
        }
        entry = entry->next;
    }
    AssertMsg(false, "Data chunk not found");
}

void
NativeCodeData::DeleteChunkList(DataChunk * chunkList)
{
    DataChunk * next = chunkList;
    while (next != nullptr)
    {
        DataChunk * current = next;
        next = next->next;
        delete current;
    }
}

NativeCodeData::Allocator::Allocator() : chunkList(nullptr), lastChunkList(nullptr)
{
    this->totalSize = 0;
    this->allocCount = 0;
#if DBG
    this->finalized = false;
#endif
#ifdef PERF_COUNTERS
    this->size = 0;
#endif
}

NativeCodeData::Allocator::~Allocator()
{
    Assert(!finalized || this->chunkList == nullptr);
    NativeCodeData::DeleteChunkList(this->chunkList);
    PERF_COUNTER_SUB(Code, DynamicNativeCodeDataSize, this->size);
    PERF_COUNTER_SUB(Code, TotalNativeCodeDataSize, this->size);
}

char *
NativeCodeData::Allocator::Alloc(size_t requestSize)
{
    char * data = nullptr;
    Assert(!finalized);    
    requestSize = Math::Align(requestSize, sizeof(void*));
    DataChunk * newChunk = HeapNewStructPlus(requestSize, DataChunk);

#if DBG
    newChunk->dataType = nullptr;
#endif

    newChunk->next = nullptr;
    newChunk->allocIndex = this->allocCount++;
    newChunk->len = (unsigned int)requestSize;
    newChunk->fixupList = nullptr;
    newChunk->fixupFunc = nullptr;
    newChunk->offset = this->totalSize;
    if (this->chunkList == nullptr)
    {
        this->chunkList = newChunk;
        this->lastChunkList = newChunk;
    }
    else
    {
        this->lastChunkList->next = newChunk;
        this->lastChunkList = newChunk;
    }

    data = newChunk->data;
    this->totalSize += (unsigned int)requestSize;

#ifdef PERF_COUNTERS
    this->size += requestSize;
    PERF_COUNTER_ADD(Code, DynamicNativeCodeDataSize, requestSize);
#endif

    PERF_COUNTER_ADD(Code, TotalNativeCodeDataSize, requestSize);
    return data;
}

char *
NativeCodeData::Allocator::AllocZero(size_t requestSize)
{
    char * data = Alloc(requestSize);
    memset(data, 0, requestSize);
    return data;
}

NativeCodeData *
NativeCodeData::Allocator::Finalize()
{
    NativeCodeData * data = nullptr;
    if (this->chunkList != nullptr)
    {
        data = HeapNew(NativeCodeData, this->chunkList);
        this->chunkList = nullptr;
#ifdef PERF_COUNTERS
        data->size = this->size;
        this->size = 0;
#endif
    }
#if DBG
    this->finalized = true;
#endif
    return data;
}

//////////////////////////////////////////////////////////////////////////
//NativeCodeData::Allocator::Free
//This function should not be called at all because the life time is active during the run time
//This function is added to enable Dictionary(has calls to Free() Method - which will never be called as it will be
//allocated as a NativeAllocator to be allocated with NativeAllocator)
//////////////////////////////////////////////////////////////////////////
void
NativeCodeData::Allocator::Free(void * buffer, size_t byteSize)
{
}
