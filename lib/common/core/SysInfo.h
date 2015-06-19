/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

//----------------------------------------------------------------------------
// Automatic system info getter at startup
//----------------------------------------------------------------------------
class AutoSystemInfo : public SYSTEM_INFO
{
public:
    static AutoSystemInfo Data;       
    uint GetAllocationGranularityPageCount() const;
    uint GetAllocationGranularityPageSize() const;

    HANDLE GetProcessHandle() { return this->processHandle; }
    bool DisableDebugScopeCapture() const { return this->disableDebugScopeCapture; }
    bool IsCFGEnabled();
    bool IsWin8OrLater();
    bool IsWinThresholdOrLater();
#if defined(_M_IX86) || defined(_M_X64)
    bool VirtualSseAvailable(const int sseLevel) const;
#endif
    BOOL SSE2Available() const;
#if defined(_M_IX86) || defined(_M_X64)
    BOOL SSE3Available() const;
    BOOL SSE4_1Available() const;
    BOOL PopCntAvailable() const;
    BOOL LZCntAvailable() const;
    bool IsAtomPlatform() const;
#endif
    DWORD GetNumberOfLogicalProcessors() const { return this->dwNumberOfProcessors; }
    DWORD GetNumberOfPhysicalProcessors() const { return this->dwNumberOfPhyscialProcessors; }

#if defined(_M_ARM32_OR_ARM64)
    bool ArmDivAvailable() const { return this->armDivAvailable; }
#endif
    static DWORD SaveModuleFileName(HANDLE hMod);
    static LPCWSTR GetJscriptDllFileName();
    static HRESULT GetJscriptFileVersion(DWORD* majorVersion, DWORD* minorVersion);
#if DBG
    static bool IsInitialized() { return AutoSystemInfo::Data.initialized; }
#endif
    static bool IsJscriptModulePointer(void * ptr);
    static DWORD const PageSize = 4096;
    UINT_PTR dllLoadAddress;
    UINT_PTR dllHighAddress;
private:
    AutoSystemInfo() { Initialize(); }
    void Initialize();
    uint allocationGranularityPageCount;
    HANDLE processHandle;
#if defined(_M_IX86) || defined(_M_X64)
    int CPUInfo[4];
#endif
#if defined(_M_ARM32_OR_ARM64)
    bool armDivAvailable;
#endif
    OSVERSIONINFO versionInfo;
    DWORD dwNumberOfPhyscialProcessors;

    bool disableDebugScopeCapture;
#if DBG
    bool initialized;
#endif

private:
#if defined(_M_IX86) || defined(_M_X64)
    bool isAtom;
    bool CheckForAtom() const;
#endif

    bool InitPhysicalProcessorCount();

    static WCHAR binaryName[MAX_PATH + 1];
    static DWORD majorVersion;
    static DWORD minorVersion;
    static HRESULT GetVersionInfo(__in LPCWSTR pszPath, DWORD* majorVersion, DWORD* minorVersion);

    static const DWORD INVALID_VERSION = (DWORD)-1;

    ULONGLONG UAPInfo;
    ULONG DeviceFamily;
    ULONG DeviceForm;
    bool deviceInfoRetrived;

public:
    static bool ShouldQCMoreFrequently()
    { 
        return Data.deviceInfoRetrived
            && (Data.DeviceFamily == 0x00000004 /*DEVICEFAMILYINFOENUM_MOBILE*/);
    }

    static bool SupportsOnlyMultiThreadedCOM()
    {
        return Data.deviceInfoRetrived
            && (Data.DeviceFamily == 0x00000004 /*DEVICEFAMILYINFOENUM_MOBILE*/); //TODO: pick some other platform to the list
    }

    static bool IsLowMemoryDevice()
    {
        return Data.deviceInfoRetrived
            && (Data.DeviceFamily == 0x00000004 /*DEVICEFAMILYINFOENUM_MOBILE*/); //TODO: pick some other platform to the list
    }
};


// For Prefast where it doesn't like symbolic constants
CompileAssert(AutoSystemInfo::PageSize == 4096);
#define __in_ecount_pagesize __in_ecount(4096)
#define __in_ecount_twopagesize __in_ecount(8192)
