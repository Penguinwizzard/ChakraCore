//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
#pragma once

class HostSystemInfo
{
public:
    static bool SupportsOnlyMultiThreadedCOM()
    {
        return Data.deviceInfoRetrieved
            && (Data.deviceFamily == 0x00000004 /*DEVICEFAMILYINFOENUM_MOBILE*/);
    }

    HostSystemInfo();

private:
    static HostSystemInfo Data;
    bool   deviceInfoRetrieved; 
    ULONG  deviceFamily;
    ULONGLONG uapInfo;
    ULONG  deviceForm;
};
