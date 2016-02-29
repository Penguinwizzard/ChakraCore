//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

#include "Common/MathUtil.h"

#pragma warning(push)
#pragma warning(disable:6200) // C6200: Index is out of valid index range, compiler complains here we use variable length array

namespace Js
{
    template<class T, typename CountT = T::CounterFields>
    struct MixCounter
    {
        union
        {
            uint8 smallUnsignedFields[CountT::Max];
            int8 smallSignedFields[CountT::Max];
        };
        union
        {
            uint32 bigUnsignedFields[1];
            int32 bigSignedFields[1];
        };
    };

    template<class T, typename CountT = T::CounterFields>
    struct Counter
    {
        typedef MixCounter<T, CountT> MixCounterT;
        volatile uint8 bigFieldsCount;
        union {
            uint8 u8fields[CountT::Max];
            int8 i8fields[CountT::Max];
            struct
            {
                BVStatic<(uint8)CountT::Max> bigFields;
                WriteBarrierPtr<MixCounterT> mixCounter;
            };
        };


        Counter()
            :bigFieldsCount(0)
        {
            memset(u8fields, 0, (uint8)CountT::Max);
        }

        void Promote(T*host)
        {
            MixCounterT* newCounter = RecyclerNewStruct(host->GetRecycler(), MixCounterT);
            memcpy(&newCounter->smallUnsignedFields, &this->u8fields, (uint8)CountT::Max);
            this->mixCounter = newCounter;
            this->bigFields.ClearAll();
        }

        void Expand(T*host, uint8 newCount)
        {
            if ((newCount - 1) % (16 / sizeof(uint32)) == 1) 
            {
                uint8 newSize = newCount*sizeof(uint32) + sizeof(MixCounterT);
                MixCounterT* newCounter = RecyclerNewPlusLeaf(host->GetRecycler(), newCount*sizeof(uint32), MixCounterT);
                memcpy(newCounter, this->mixCounter, newSize - sizeof(uint32));
                this->mixCounter = newCounter;
            }
        }

        uint32 Get(CountT typeEnum) const
        {
            uint8 type = static_cast<uint8>(typeEnum);
            uint8 localbigFieldsCount = this->bigFieldsCount;
            uint32 value = 0;
            if (localbigFieldsCount == 0)
            {
                value = this->u8fields[type];
            }
            else
            {
                value = this->mixCounter->smallUnsignedFields[type];
                if (this->bigFields.Test(type))
                {
                    // the small field saves the position in the big fields array
                    Assert(value < this->bigFields.Count());
                    value = this->mixCounter->bigUnsignedFields[value];
                }
            }

            // check if main thread has updated the structure
            if (localbigFieldsCount == this->bigFieldsCount)
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
            uint8 localbigFieldsCount = this->bigFieldsCount;
            int32 value = 0;
            if (localbigFieldsCount == 0)
            {
                value = this->i8fields[type];
            }
            else
            {
                value = this->mixCounter->smallUnsignedFields[type];
                if (this->bigFields.Test(type))
                {
                    // the small field saves the position in the big fields array
                    Assert(value < (int32)this->bigFields.Count());
                    value = this->mixCounter->bigSignedFields[value];
                }
            }

            // check if main thread has updated the structure
            if (localbigFieldsCount == this->bigFieldsCount)
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
            if (bigFieldsCount == 0)
            {
                if (val <= UINT8_MAX)
                {
                    return this->u8fields[type] = static_cast<uint8>(val);
                }
                else
                {
                    AutoCriticalSection autocs(T::GetLock());
                    this->bigFieldsCount++;
                    Promote(host);
                    this->bigFields.Set(type);
                    Assert(this->bigFieldsCount == this->bigFields.Count());
                    this->mixCounter->smallUnsignedFields[type] = 0;
                    return this->mixCounter->bigUnsignedFields[0] = val;
                }
            }
            else
            {
                if (this->bigFields.Test(type))
                {
                    uint8 pos = this->mixCounter->smallUnsignedFields[type];
                    return this->mixCounter->bigUnsignedFields[pos] = val;
                }

                if (val <= UINT8_MAX)
                {
                    return this->mixCounter->smallUnsignedFields[type] = static_cast<uint8>(val);
                }
                else
                {
                    // expand
                    AutoCriticalSection autocs(T::GetLock());
                    this->bigFieldsCount++;
                    Expand(host, this->bigFieldsCount);
                    this->bigFields.Set(type);
                    Assert(this->bigFieldsCount == this->bigFields.Count());
                    this->mixCounter->smallUnsignedFields[type] = this->bigFieldsCount-1;
                    return this->mixCounter->bigUnsignedFields[this->bigFieldsCount-1] = val;
                }
            }
        }

        int32 SetSigned(CountT typeEnum, int32 val, T* host)
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (bigFieldsCount == 0)
            {
                if (val <= INT8_MAX && val >= INT8_MIN)
                {
                    return this->i8fields[type] = static_cast<int8>(val);
                }
                else
                {
                    AutoCriticalSection autocs(T::GetLock());
                    this->bigFieldsCount++;
                    Promote(host);
                    this->bigFields.Set(type);
                    Assert(this->bigFieldsCount == this->bigFields.Count());
                    this->mixCounter->smallUnsignedFields[type] = 0;
                    return this->mixCounter->bigSignedFields[0] = val;
                }
            }
            else
            {
                if (this->bigFields.Test(type))
                {
                    uint8 pos = this->mixCounter->smallUnsignedFields[type];
                    return this->mixCounter->bigSignedFields[pos] = val;
                }

                if (val <= INT8_MAX && val >= INT8_MIN)
                {
                    return this->mixCounter->smallSignedFields[type] = static_cast<int8>(val);
                }
                else
                {
                    // expand
                    AutoCriticalSection autocs(T::GetLock());
                    this->bigFieldsCount++;
                    Expand(host, this->bigFieldsCount);
                    this->bigFields.Set(type);
                    Assert(this->bigFieldsCount == this->bigFields.Count());
                    this->mixCounter->smallUnsignedFields[type] = this->bigFieldsCount-1;
                    return this->mixCounter->bigSignedFields[this->bigFieldsCount-1] = val;
                }
            }
        }

        uint32 Increase(CountT typeEnum, T* host)
        {
            uint32 val = this->Get(typeEnum);
            this->Set(typeEnum, val + 1, host);
            return val;
        }
    };
}
#pragma warning(pop)
