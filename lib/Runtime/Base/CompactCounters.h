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
        uint8 fieldSize;
        union {
            uint8 fields1[1];
            uint16 fields2[1];
            uint32 fields4[1];
            int8 signedFields1[1];
            int16 signedFields2[1];
            int32 signedFields4[1];
        };

        Counter(uint8 size)
        {
            fieldSize = size;
        }

        static void AllocCounters(T* host, uint8 newSize);

        uint32 Get(CountT typeEnum) const
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                return this->fields1[type];
            }
            else if (fieldSize == 2)
            {
                return this->fields2[type];
            }
            else
            {
                Assert(fieldSize == 4);
                return this->fields4[type];
            }
        }

        int32 GetSigned(CountT typeEnum) const
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                return this->signedFields1[type];
            }
            else if (fieldSize == 2)
            {
                return this->signedFields2[type];
            }
            else
            {
                Assert(fieldSize == 4);
                return this->signedFields4[type];
            }
        }

        uint32 Set(CountT typeEnum, uint32 val, T* host)
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                if (val <= UINT8_MAX)
                {
                    return fields1[type] = static_cast<uint8>(val);
                }
                else
                {
                    AllocCounters(host, val <= UINT16_MAX ? 2 : 4);
                }
                return host->counters->Set(typeEnum, val, host);
            }

            if (fieldSize == 2)
            {
                if (val <= UINT16_MAX)
                {
                    return fields2[type] = static_cast<uint16>(val);
                }
                else
                {
                    AllocCounters(host, 4);
                }
                return host->counters->Set(typeEnum, val, host);
            }

            Assert(fieldSize == 4);
            return fields4[type] = val;
        }

        int32 SetSigned(CountT typeEnum, int32 val, T* host)
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                if (val <= INT8_MAX && val >= INT8_MIN)
                {
                    return signedFields1[type] = static_cast<uint8>(val);
                }
                else
                {
                    AllocCounters(host, (val <= INT16_MAX && val >= INT16_MIN) ? 2 : 4);
                }
                return host->counters->SetSigned(typeEnum, val, host);
            }

            if (fieldSize == 2)
            {
                if (val <= INT16_MAX && val >= INT16_MIN)
                {
                    return signedFields2[type] = static_cast<uint16>(val);
                }
                else
                {
                    AllocCounters(host, 4);
                }
                return host->counters->SetSigned(typeEnum, val, host);
            }

            Assert(fieldSize == 4);
            return signedFields4[type] = val;
        }

        uint32 Increase(CountT typeEnum, T* host)
        {
            uint8 type = static_cast<uint8>(typeEnum);
            if (fieldSize == 1)
            {
                if (fields1[type] < UINT8_MAX)
                {
                    return fields1[type]++;
                }
                else
                {
                    AllocCounters(host, fields1[type] < UINT16_MAX ? 2 : 4);
                }
                return host->counters->Increase(typeEnum, host);
            }

            if (fieldSize == 2)
            {
                if (fields1[type] < UINT16_MAX)
                {
                    return fields2[type]++;
                }
                else
                {
                    AllocCounters(host, 4);
                }
                return host->counters->Increase(typeEnum, host);
            }

            Assert(fieldSize == 4);
            return fields4[type]++;
        }
    };


    template<class T, typename CountT>
    void Counter<T, CountT>::AllocCounters(T* host, uint8 newSize)
    {
        typedef Counter<T, CountT> CounterT;
        Assert(host->GetRecycler() != nullptr);

        const uint8 signedStart = static_cast<uint8>(CountT::SignedFieldsStart);
        const uint8 max = static_cast<uint8>(CountT::Max);
        auto plusSize = (max - 1)*newSize;
        CounterT* newCounters = RecyclerNewPlusLeafZ(host->GetRecycler(), plusSize, CounterT, newSize);

        if (host->counters != nullptr)
        {
            uint8 i = 0;
            if (host->counters->fieldSize == 1)
            {
                if (newSize == 2)
                {
                    for (; i < signedStart; i++)
                    {
                        newCounters->fields2[i] = host->counters->fields1[i];
                    }
                    for (; i < max; i++)
                    {
                        newCounters->signedFields2[i] = host->counters->signedFields1[i];
                    }
                }
                else
                {
                    for (; i < signedStart; i++)
                    {
                        newCounters->fields4[i] = host->counters->fields1[i];
                    }
                    for (; i < max; i++)
                    {
                        newCounters->signedFields4[i] = host->counters->signedFields1[i];
                    }
                }
            }
            else if (host->counters->fieldSize == 2)
            {
                for (; i < signedStart; i++)
                {
                    newCounters->fields4[i] = host->counters->fields2[i];
                }
                for (; i < max; i++)
                {
                    newCounters->signedFields4[i] = host->counters->signedFields2[i];
                }
            }
            else
            {
                Assert(false);
            }

        }
        host->counters = newCounters;
    }
}
#pragma warning(pop)
