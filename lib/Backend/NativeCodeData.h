//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#define NativeCodeDataNew(alloc, T, ...) AllocatorNew(NativeCodeData::AllocatorT<T>, alloc, T, __VA_ARGS__)
#define NativeCodeDataNewZ(alloc, T, ...) AllocatorNewZ(NativeCodeData::AllocatorT<T>, alloc, T, __VA_ARGS__)
#define NativeCodeDataNewArray(alloc, T, count) AllocatorNewArray(NativeCodeData::AllocatorT<NativeCodeData::Array<T>>, alloc, T, count)
#define NativeCodeDataNewArrayZ(alloc, T, count) AllocatorNewArrayZ(NativeCodeData::AllocatorT<NativeCodeData::Array<T>>, alloc, T, count)
#define NativeCodeDataNewPlusZ(size, alloc, T, ...) AllocatorNewPlusZ(NativeCodeData::AllocatorT<T>, alloc, size, T, __VA_ARGS__)

#define NativeCodeDataNewNoFixup(alloc, T, ...) AllocatorNew(NativeCodeData::AllocatorNoFixup<T>, alloc, T, __VA_ARGS__)
#define NativeCodeDataNewZNoFixup(alloc, T, ...) AllocatorNewZ(NativeCodeData::AllocatorNoFixup<T>, alloc, T, __VA_ARGS__)
#define NativeCodeDataNewArrayNoFixup(alloc, T, count) AllocatorNewArray(NativeCodeData::AllocatorNoFixup<NativeCodeData::Array<T>>, alloc, T, count)
#define NativeCodeDataNewArrayZNoFixup(alloc, T, count) AllocatorNewArrayZ(NativeCodeData::AllocatorNoFixup<NativeCodeData::Array<T>>, alloc, T, count)
#define NativeCodeDataNewPlusZNoFixup(size, alloc, T, ...) AllocatorNewPlusZ(NativeCodeData::AllocatorNoFixup<T>, alloc, size, T, __VA_ARGS__)

#define FixupNativeDataPointer(field, chunkList) NativeCodeData::AddFixupEntry(this->field, &this->field, this, chunkList)

class CodeGenAllocators;

class NativeCodeData
{

public:

    struct DataChunk
    {
        unsigned int len;
        unsigned int allocIndex;
        unsigned int offset; // offset to the aggregated buffer
#if DBG
        const char* dataType;
#endif

        // todo: use union?
        void(*fixupFunc)(void* _this, NativeCodeData::DataChunk*);
        NativeDataFixupEntry *fixupList;

        DataChunk * next;
        char data[0];
    };

    static DataChunk* GetDataChunk(void* data)
    {
        return (NativeCodeData::DataChunk*)((char*)data - offsetof(NativeCodeData::DataChunk, data));
    }

    static wchar_t* GetDataDescription(void* data, JitArenaAllocator * alloc);

    static unsigned int GetDataTotalOffset(void* data)
    {
        return GetDataChunk(data)->offset;
    }

    NativeCodeData(DataChunk * chunkList);
    DataChunk * chunkList;

#ifdef PERF_COUNTERS
    size_t size;
#endif
public:

    static void VerifyExistFixupEntry(void* targetAddr, void* addrToFixup, void* startAddress);
    static void AddFixupEntry(void* targetAddr, void* addrToFixup, void* startAddress, DataChunk * chunkList);
    static void AddFixupEntry(void* targetAddr, void* targetStartAddr, void* addrToFixup, void* startAddress, DataChunk * chunkList);
    static void AddFixupEntryForPointerArray(void* startAddress, DataChunk * chunkList);
    static void DeleteChunkList(DataChunk * chunkList);
public:
    class Allocator
    {
    public:
        static const bool FakeZeroLengthArray = false;

        Allocator();
        ~Allocator();

        char * Alloc(size_t requestedBytes);
        char * AllocZero(size_t requestedBytes);
        NativeCodeData * Finalize();
        void Free(void * buffer, size_t byteSize);

        DataChunk * chunkList;
        DataChunk * lastChunkList;
        unsigned int totalSize;
        unsigned int allocCount;

#ifdef TRACK_ALLOC
        // Doesn't support tracking information, dummy implementation
        Allocator * TrackAllocInfo(TrackAllocData const& data) { return this; }
        void ClearTrackAllocInfo(TrackAllocData* data = NULL) {}
#endif
    private:

#if DBG
        bool finalized;
#endif
#ifdef PERF_COUNTERS
        size_t size;
#endif
    };

    template<typename T>
    class Array
    {
    public:
        void Fixup(NativeCodeData::DataChunk* chunkList)
        {
            int count = NativeCodeData::GetDataChunk(this)->len / sizeof(T);
            while (count-- > 0) 
            {
                (((T*)this) + count)->Fixup(chunkList);
            }
        }
    };

    template<typename T>
    class AllocatorNoFixup : public Allocator
    {
    public:
        char * Alloc(size_t requestedBytes)
        {
            char* dataBlock = __super::Alloc(requestedBytes);
#if DBG
            DataChunk* chunk = NativeCodeData::GetDataChunk(dataBlock);
            chunk->dataType = typeid(T).name();
            if (PHASE_TRACE1(Js::NativeCodeDataPhase))
            {                
                Output::Print(L"NativeCodeData AllocNoFix: chunk: %p, data: %p, index: %d, len: %x, totalOffset: %x, type: %S\n",
                    chunk, (void*)dataBlock, chunk->allocIndex, chunk->len, chunk->offset, chunk->dataType);
            }
#endif

            return dataBlock;
        }
        char * AllocZero(size_t requestedBytes)
        {
            char* dataBlock = __super::AllocZero(requestedBytes);

#if DBG
            DataChunk* chunk = NativeCodeData::GetDataChunk(dataBlock);
            chunk->dataType = typeid(T).name();
            if (PHASE_TRACE1(Js::NativeCodeDataPhase))
            {
                Output::Print(L"NativeCodeData AllocNoFix: chunk: %p, data: %p, index: %d, len: %x, totalOffset: %x, type: %S\n",
                    chunk, (void*)dataBlock, chunk->allocIndex, chunk->len, chunk->offset, chunk->dataType);
            }
#endif

            return dataBlock;
        }
    };

    template<typename T>
    class AllocatorT : public Allocator
    {
#if DBG
        __declspec(noinline) // compiler inline this function even in chk build... maybe because it's in .h file?
#endif
        char* AddFixup(char* dataBlock)
        {
            DataChunk* chunk = NativeCodeData::GetDataChunk(dataBlock);
            chunk->fixupFunc = &Fixup;
#if DBG
            chunk->dataType = typeid(T).name();
            if (PHASE_TRACE1(Js::NativeCodeDataPhase))
            {
                Output::Print(L"NativeCodeData Alloc: chunk: %p, data: %p, index: %d, len: %x, totalOffset: %x, type: %S\n",
                    chunk, (void*)dataBlock, chunk->allocIndex, chunk->len, chunk->offset, chunk->dataType);
            }
#endif

            return dataBlock;
        }

    public:
        char * Alloc(size_t requestedBytes)
        {
            return AddFixup(__super::Alloc(requestedBytes));
        }
        char * AllocZero(size_t requestedBytes)
        {
            return AddFixup(__super::AllocZero(requestedBytes));
        }

        static void Fixup(void* pThis, NativeCodeData::DataChunk* chunkList)
        {
            ((T*)pThis)->Fixup(chunkList);
        }
    };

    ~NativeCodeData();


    static const char DataDesc_None[];
    static const char DataDesc_InlineeFrameRecord_ArgOffsets[];
    static const char DataDesc_InlineeFrameRecord_Constants[];
    static const char DataDesc_BailoutInfo_CotalOutParamCount[];
    static const char DataDesc_ArgOutOffsetInfo_StartCallOutParamCounts[];
    static const char DataDesc_ArgOutOffsetInfo_StartCallArgRestoreAdjustCounts[];
    static const char DataDesc_LowererMD_LoadFloatValue_Float[];
    static const char DataDesc_LowererMD_LoadFloatValue_Double[];
    static const char DataDesc_LowererMD_EmitLoadFloatCommon_Double[];
    static const char DataDesc_LowererMD_Simd128LoadConst[];

};

template<char const *desc = NativeCodeData::DataDesc_None>
struct IntType 
{ 
    int data; 
};

template<char const *desc = NativeCodeData::DataDesc_None>
struct UIntType
{
    uint data;
};

template<char const *desc = NativeCodeData::DataDesc_None>
struct FloatType
{
    FloatType(float val) :data(val) {}
    float data;
};

template<char const *desc = NativeCodeData::DataDesc_None>
struct DoubleType
{
    DoubleType() {}
    DoubleType(double val) :data(val) {}
    double data;
};

template<char const *desc = NativeCodeData::DataDesc_None>
struct SIMDType
{
    SIMDType() {}
    SIMDType(AsmJsSIMDValue val) :data(val) {}
    AsmJsSIMDValue data;
};

template<char const *desc = NativeCodeData::DataDesc_None>
struct VarType
{
    Js::Var data;
    void Fixup(NativeCodeData::DataChunk* chunkList)
    {
        AssertMsg(false, "Please specialize Fixup method for this Var type or use no-fixup allocator");
    }
};
template<>
void VarType<NativeCodeData::DataDesc_InlineeFrameRecord_Constants>::Fixup(NativeCodeData::DataChunk* chunkList)
{
    AssertMsg(false, "InlineeFrameRecord::constants contains Var from main process, should not fixup");
}

struct GlobalBailOutRecordDataTable;
template<> void NativeCodeData::Array<GlobalBailOutRecordDataTable *>::Fixup(NativeCodeData::DataChunk* chunkList)
{
    NativeCodeData::AddFixupEntryForPointerArray(this, chunkList);
}
