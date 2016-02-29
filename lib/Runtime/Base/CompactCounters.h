//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#pragma warning(push)
#pragma warning(disable:6200) // C6200: Index is out of valid index range, compiler complains here we use variable length array

namespace Js
{
    template<class T, typename CountT = T::CounterFields>
    struct Counter
    {

        volatile uint8 fieldSize;
        union {
            uint8 u8fields[CountT::Max];
            int8 i8fields[CountT::Max];
            WriteBarrierPtr<uint16> u16Fileds;
            WriteBarrierPtr<uint32> u32Fileds;
            WriteBarrierPtr<int16> i16Fileds;
            WriteBarrierPtr<int32> i32Fileds;
        };


        Counter()
            :fieldSize(1)
        {
            memset(u8fields, 0, (uint8)CountT::Max);
        }

        void AllocCounters(T* host, uint8 newSize);

        uint32 Get(CountT typeEnum) const
        {
            uint8 type = static_cast<uint8>(typeEnum);
            uint8 localFieldSize = fieldSize;
            uint32 value = 0;
            if (localFieldSize == 1)
            {
                value = this->u8fields[type];
            }
            else if (localFieldSize == 2)
            {
                value = this->u16Fileds[type];
            }
            else
            {
                Assert(localFieldSize == 4);
                value = this->u32Fileds[type];
            }

            // check if main thread has updated the structure
            if (localFieldSize == fieldSize) 
            {
                return value;
            }
            else 
            {
                Assert(!ThreadContext::GetContextForCurrentThread());
                AutoCriticalSection autocs(T::GetLock());
                return Get(typeEnum);
            }
        }

        int32 GetSigned(CountT typeEnum) const
        {
            uint8 type = static_cast<uint8>(typeEnum);
            uint8 localFieldSize = fieldSize;
            int32 value = 0;
            if (localFieldSize == 1)
            {
                value = this->i8fields[type];
            }
            else if (localFieldSize == 2)
            {
                value = this->i16Fileds[type];
            }
            else
            {
                Assert(localFieldSize == 4);
                value = this->i32Fileds[type];
            }

            // check if main thread has updated the structure
            if (localFieldSize == fieldSize)
            {
                return value;
            }
            else
            {
                Assert(!ThreadContext::GetContextForCurrentThread());
                AutoCriticalSection autocs(T::GetLock());
                return GetSigned(typeEnum);
            }
        }

        uint32 Set(CountT typeEnum, uint32 val, T* host)
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                if (val <= UINT8_MAX)
                {
                    return this->u8fields[type] = static_cast<uint8>(val);
                }
                else
                {
                    AllocCounters(host, val <= UINT16_MAX ? 2 : 4);
                }
                return this->Set(typeEnum, val, host);
            }

            if (fieldSize == 2)
            {
                if (val <= UINT16_MAX)
                {
                    return this->u16Fileds[type] = static_cast<uint16>(val);
                }
                else
                {
                    AllocCounters(host, 4);
                }
                return this->Set(typeEnum, val, host);
            }

            Assert(fieldSize == 4);
            return this->u32Fileds[type] = val;
        }

        int32 SetSigned(CountT typeEnum, int32 val, T* host)
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                if (val <= INT8_MAX && val >= INT8_MIN)
                {
                    return this->i8fields[type] = static_cast<uint8>(val);
                }
                else
                {
                    AllocCounters(host, (val <= INT16_MAX && val >= INT16_MIN) ? 2 : 4);
                }
                return this->SetSigned(typeEnum, val, host);
            }

            if (fieldSize == 2)
            {
                if (val <= INT16_MAX && val >= INT16_MIN)
                {
                    return this->i16Fileds[type] = static_cast<uint16>(val);
                }
                else
                {
                    AllocCounters(host, 4);
                }
                return this->SetSigned(typeEnum, val, host);
            }

            Assert(fieldSize == 4);
            return this->i32Fileds[type] = val;
        }

        uint32 Increase(CountT typeEnum, T* host)
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                if (this->u8fields[type] < UINT8_MAX)
                {
                    return this->u8fields[type]++;
                }
                else
                {
                    AllocCounters(host, 2);
                }
                return this->Increase(typeEnum, host);
            }

            if (fieldSize == 2)
            {
                if (this->u16Fileds[type] < UINT16_MAX)
                {
                    return this->u16Fileds[type]++;
                }
                else
                {
                    AllocCounters(host, 4);
                }
                return this->Increase(typeEnum, host);
            }

            Assert(fieldSize == 4);
            return this->u32Fileds[type]++;
        }
    };


    template<class T, typename CountT>
    void Counter<T, CountT>::AllocCounters(T* host, uint8 newSize)
    {
        Assert(ThreadContext::GetContextForCurrentThread() || ThreadContext::GetCriticalSection()->IsLocked());

        typedef Counter<T, CountT> CounterT;
        Assert(host->GetRecycler() != nullptr);

        const uint8 signedStart = static_cast<uint8>(CountT::SignedFieldsStart);
        const uint8 max = static_cast<uint8>(CountT::Max);

        void* newFieldsArray = nullptr;
        if (newSize == 2) {
            newFieldsArray = RecyclerNewArrayLeafZ(host->GetRecycler(), uint16, max);
        }
        else {
            Assert(newSize == 4);
            newFieldsArray = RecyclerNewArrayLeafZ(host->GetRecycler(), uint32, max);
        }

        uint8 i = 0;
        if (this->fieldSize == 1)
        {
            if (newSize == 2)
            {
                for (; i < signedStart; i++)
                {
                    ((uint16*)newFieldsArray)[i] = this->u8fields[i];
                }
                for (; i < max; i++)
                {
                    ((int16*)newFieldsArray)[i] = this->i8fields[i];
                }
            }
            else
            {
                for (; i < signedStart; i++)
                {
                    ((uint32*)newFieldsArray)[i] = this->u8fields[i];
                }
                for (; i < max; i++)
                {
                    ((int32*)newFieldsArray)[i] = this->i8fields[i];
                }
            }
        }
        else if (this->fieldSize == 2)
        {
            for (; i < signedStart; i++)
            {
                ((uint32*)newFieldsArray)[i] = this->u16Fileds[i];
            }
            for (; i < max; i++)
            {
                ((int32*)newFieldsArray)[i] = this->i16Fileds[i];
            }
        }
        else
        {
            Assert(false);
        }
        
        AutoCriticalSection autocs(T::GetLock());
        this->fieldSize = newSize;
        this->u16Fileds = (uint16*)newFieldsArray;        

    }

}
#pragma warning(pop)
