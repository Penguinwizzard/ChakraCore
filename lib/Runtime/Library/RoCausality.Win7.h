//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
// This is a generated file built from a snap of //fbl_rex_dev1/com/manifests/Microsoft.Windows.COM.Base.man#6

#pragma once
#include <wmistr.h>
#include <evntrace.h>
#include "evntprov.h"
//
//  Initial Defs
//
#if !defined(ETW_INLINE)
#define ETW_INLINE DECLSPEC_NOINLINE __inline
#endif

#if defined(__cplusplus)
extern "C" {
#endif

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION
#if  !defined(McGenDebug)
#define McGenDebug(a,b)
#endif 


#if !defined(MCGEN_TRACE_CONTEXT_DEF)
#define MCGEN_TRACE_CONTEXT_DEF
typedef struct _MCGEN_TRACE_CONTEXT
{
    TRACEHANDLE            RegistrationHandle;
    TRACEHANDLE            Logger;
    ULONGLONG              MatchAnyKeyword;
    ULONGLONG              MatchAllKeyword;
    ULONG                  Flags;
    ULONG                  IsEnabled;
    UCHAR                  Level; 
    UCHAR                  Reserve;
    USHORT                 EnableBitsCount;
    PULONG                 EnableBitMask;
    const ULONGLONG*       EnableKeyWords;
    const UCHAR*           EnableLevel;
} MCGEN_TRACE_CONTEXT, *PMCGEN_TRACE_CONTEXT;
#endif

#if !defined(MCGEN_LEVEL_KEYWORD_ENABLED_DEF)
#define MCGEN_LEVEL_KEYWORD_ENABLED_DEF
FORCEINLINE
BOOLEAN
McGenLevelKeywordEnabled(
    _In_ PMCGEN_TRACE_CONTEXT EnableInfo,
    _In_ UCHAR Level,
    _In_ ULONGLONG Keyword
    )
{
    //
    // Check if the event Level is lower than the level at which
    // the channel is enabled.
    // If the event Level is 0 or the channel is enabled at level 0,
    // all levels are enabled.
    //

    if ((Level <= EnableInfo->Level) || // This also covers the case of Level == 0.
        (EnableInfo->Level == 0)) {

        //
        // Check if Keyword is enabled
        //

        if ((Keyword == (ULONGLONG)0) ||
            ((Keyword & EnableInfo->MatchAnyKeyword) &&
             ((Keyword & EnableInfo->MatchAllKeyword) == EnableInfo->MatchAllKeyword))) {
            return TRUE;
        }
    }

    return FALSE;

}
#endif

#if !defined(MCGEN_EVENT_ENABLED_DEF)
#define MCGEN_EVENT_ENABLED_DEF
FORCEINLINE
BOOLEAN
McGenEventEnabled(
    _In_ PMCGEN_TRACE_CONTEXT EnableInfo,
    _In_ PCEVENT_DESCRIPTOR EventDescriptor
    )
{

    return McGenLevelKeywordEnabled(EnableInfo, EventDescriptor->Level, EventDescriptor->Keyword);

}
#endif


//
// EnableCheckMacro
//
#ifndef MCGEN_ENABLE_CHECK
#define MCGEN_ENABLE_CHECK(Context, Descriptor) (Context.IsEnabled &&  McGenEventEnabled(&Context, &Descriptor))
#endif

#if !defined(MCGEN_CONTROL_CALLBACK)
#define MCGEN_CONTROL_CALLBACK

DECLSPEC_NOINLINE __inline
VOID
__stdcall
McGenControlCallbackV2(
    _In_ LPCGUID SourceId,
    _In_ ULONG ControlCode,
    _In_ UCHAR Level,
    _In_ ULONGLONG MatchAnyKeyword,
    _In_ ULONGLONG MatchAllKeyword,
    _In_opt_ PEVENT_FILTER_DESCRIPTOR FilterData,
    _Inout_opt_ PVOID CallbackContext
    )
/*++

Routine Description:

    This is the notification callback for Vista.

Arguments:

    SourceId - The GUID that identifies the session that enabled the provider. 

    ControlCode - The parameter indicates whether the provider 
                  is being enabled or disabled.

    Level - The level at which the event is enabled.

    MatchAnyKeyword - The bitmask of keywords that the provider uses to 
                      determine the category of events that it writes.

    MatchAllKeyword - This bitmask additionally restricts the category 
                      of events that the provider writes. 

    FilterData - The provider-defined data.

    CallbackContext - The context of the callback that is defined when the provider 
                      called EtwRegister to register itself.

Remarks:

    ETW calls this function to notify provider of enable/disable

--*/
{
    PMCGEN_TRACE_CONTEXT Ctx = (PMCGEN_TRACE_CONTEXT)CallbackContext;
    ULONG Ix;
#ifndef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    UNREFERENCED_PARAMETER(SourceId);
    UNREFERENCED_PARAMETER(FilterData);
#endif

    if (Ctx == NULL) {
        return;
    }

    switch (ControlCode) {

        case EVENT_CONTROL_CODE_ENABLE_PROVIDER:
            Ctx->Level = Level;
            Ctx->MatchAnyKeyword = MatchAnyKeyword;
            Ctx->MatchAllKeyword = MatchAllKeyword;
            Ctx->IsEnabled = EVENT_CONTROL_CODE_ENABLE_PROVIDER;

            for (Ix = 0; Ix < Ctx->EnableBitsCount; Ix += 1) {
                if (McGenLevelKeywordEnabled(Ctx, Ctx->EnableLevel[Ix], Ctx->EnableKeyWords[Ix]) != FALSE) {
                    Ctx->EnableBitMask[Ix >> 5] |= (1 << (Ix % 32));
                } else {
                    Ctx->EnableBitMask[Ix >> 5] &= ~(1 << (Ix % 32));
                }
            }
            break;

        case EVENT_CONTROL_CODE_DISABLE_PROVIDER:
            Ctx->IsEnabled = EVENT_CONTROL_CODE_DISABLE_PROVIDER;
            Ctx->Level = 0;
            Ctx->MatchAnyKeyword = 0;
            Ctx->MatchAllKeyword = 0;
            if (Ctx->EnableBitsCount > 0) {
                RtlZeroMemory(Ctx->EnableBitMask, (((Ctx->EnableBitsCount - 1) / 32) + 1) * sizeof(ULONG));
            }
            break;
 
        default:
            break;
    }

#ifdef MCGEN_PRIVATE_ENABLE_CALLBACK_V2
    //
    // Call user defined callback
    //
    MCGEN_PRIVATE_ENABLE_CALLBACK_V2(
        SourceId,
        ControlCode,
        Level,
        MatchAnyKeyword,
        MatchAllKeyword,
        FilterData,
        CallbackContext
        );
#endif
   
    return;
}

#endif
#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION
//+
// Provider Microsoft-Windows-COM-Perf Event Count 46
//+
EXTERN_C __declspec(selectany) const GUID COM_PERFORMANCE_PROVIDER = {0xb8d6861b, 0xd20f, 0x4eec, {0xbb, 0xae, 0x87, 0xe0, 0xdd, 0x80, 0x60, 0x2b}};

//
// Channel
//
#define chidWinRTActivation 0x10
#define chidCOMActivation 0x11
#define chidExtensionCatalog 0x12
#define chidCOMCall 0x13

//
// Tasks
//
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_ActivateInstance_InProc 0x1
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_ActivateInstance_OutOfProc 0x2
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_GetActivationFactory_InProc 0x3
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_GetActivationFactory_OutOfProc 0x4
#define COM_PERFORMANCE_PROVIDER_TASK_COM_CreateInstance 0x5
#define COM_PERFORMANCE_PROVIDER_TASK_COM_CreateInstance_App 0x6
#define COM_PERFORMANCE_PROVIDER_TASK_COM_GetClassObject 0x7
#define COM_PERFORMANCE_PROVIDER_TASK_ExtensionCatalog_QueryCatalog 0x8
#define COM_PERFORMANCE_PROVIDER_TASK_ExtensionCatalog_QueryCatalogByPackageFamily 0x9
#define COM_PERFORMANCE_PROVIDER_TASK_ExtensionCatalog_RoGetExtensionRegistration 0xa
#define COM_PERFORMANCE_PROVIDER_TASK_ExtensionCatalog_RoGetExtensionRegistrationByExtensionId 0xb
#define COM_PERFORMANCE_PROVIDER_TASK_ExtensionCatalog_CustomPropertyLookup 0xc
#define COM_PERFORMANCE_PROVIDER_TASK_ExtensionCatalog_MoveNext 0xd
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_BrokeredActivateInstance 0xe
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_BrokeredGetActivationFactory 0xf
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_GetActivationFactory_InprocServerImpl 0x10
#define COM_PERFORMANCE_PROVIDER_TASK_Windows_Runtime_ActivateInstance_InprocServerImpl 0x11
#define COM_PERFORMANCE_PROVIDER_TASK_COM_GetClassFactory_InprocServerImpl 0x12
#define COM_PERFORMANCE_PROVIDER_TASK_COM_CreateInstance_InprocServerImpl 0x13
#define COM_PERFORMANCE_PROVIDER_TASK_COM_ClientSyncCall 0x14
#define COM_PERFORMANCE_PROVIDER_TASK_COM_ClientAsyncCall 0x15
#define COM_PERFORMANCE_PROVIDER_TASK_COM_ServerSyncCall 0x16
#define COM_PERFORMANCE_PROVIDER_TASK_COM_ServerAsyncCall 0x17

//
// Event Descriptors
//
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_Inproc_ActivateInstance_Start = {0x1, 0x0, 0x10, 0x4, 0x1, 0x1, 0x8000000000000000};
#define WinRT_Inproc_ActivateInstance_Start_value 0x1
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_Inproc_ActivateInstance_End = {0x2, 0x0, 0x10, 0x4, 0x2, 0x1, 0x8000000000000000};
#define WinRT_Inproc_ActivateInstance_End_value 0x2
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_OutOfProc_ActivateInstance_Start = {0x3, 0x0, 0x10, 0x4, 0x1, 0x2, 0x8000000000000000};
#define WinRT_OutOfProc_ActivateInstance_Start_value 0x3
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_OutOfProc_ActivateInstance_End = {0x4, 0x0, 0x10, 0x4, 0x2, 0x2, 0x8000000000000000};
#define WinRT_OutOfProc_ActivateInstance_End_value 0x4
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_CreateInstance_Start = {0x5, 0x0, 0x11, 0x4, 0x1, 0x5, 0x4000000000000000};
#define COM_CreateInstance_Start_value 0x5
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_CreateInstance_End = {0x6, 0x0, 0x11, 0x4, 0x2, 0x5, 0x4000000000000000};
#define COM_CreateInstance_End_value 0x6
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_Inproc_GetActivationFactory_Start = {0x7, 0x0, 0x10, 0x4, 0x1, 0x3, 0x8000000000000000};
#define WinRT_Inproc_GetActivationFactory_Start_value 0x7
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_Inproc_GetActivationFactory_End = {0x8, 0x0, 0x10, 0x4, 0x2, 0x3, 0x8000000000000000};
#define WinRT_Inproc_GetActivationFactory_End_value 0x8
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_OutOfProc_GetActivationFactory_Start = {0x9, 0x0, 0x10, 0x4, 0x1, 0x4, 0x8000000000000000};
#define WinRT_OutOfProc_GetActivationFactory_Start_value 0x9
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_OutOfProc_GetActivationFactory_End = {0xa, 0x0, 0x10, 0x4, 0x2, 0x4, 0x8000000000000000};
#define WinRT_OutOfProc_GetActivationFactory_End_value 0xa
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_GetClassObject_Start = {0xb, 0x0, 0x11, 0x4, 0x1, 0x7, 0x4000000000000000};
#define COM_GetClassObject_Start_value 0xb
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_GetClassObject_End = {0xc, 0x0, 0x11, 0x4, 0x2, 0x7, 0x4000000000000000};
#define COM_GetClassObject_End_value 0xc
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_RoGetExtensionRegistration_Start = {0xd, 0x0, 0x12, 0x4, 0x1, 0xa, 0x2000000000000000};
#define ExtensionCatalog_RoGetExtensionRegistration_Start_value 0xd
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_RoGetExtensionRegistration_End = {0xe, 0x0, 0x12, 0x4, 0x2, 0xa, 0x2000000000000000};
#define ExtensionCatalog_RoGetExtensionRegistration_End_value 0xe
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start = {0xf, 0x0, 0x12, 0x4, 0x1, 0xb, 0x2000000000000000};
#define ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start_value 0xf
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End = {0x10, 0x0, 0x12, 0x4, 0x2, 0xb, 0x2000000000000000};
#define ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End_value 0x10
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_QueryCatalog_Start = {0x11, 0x0, 0x12, 0x4, 0x1, 0x8, 0x2000000000000000};
#define ExtensionCatalog_QueryCatalog_Start_value 0x11
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_QueryCatalog_End = {0x12, 0x0, 0x12, 0x4, 0x2, 0x8, 0x2000000000000000};
#define ExtensionCatalog_QueryCatalog_End_value 0x12
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_QueryCatalogByPackageFamily_Start = {0x13, 0x0, 0x12, 0x4, 0x1, 0x9, 0x2000000000000000};
#define ExtensionCatalog_QueryCatalogByPackageFamily_Start_value 0x13
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_QueryCatalogByPackageFamily_End = {0x14, 0x0, 0x12, 0x4, 0x2, 0x9, 0x2000000000000000};
#define ExtensionCatalog_QueryCatalogByPackageFamily_End_value 0x14
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_CustomPropertyLookup_Start = {0x15, 0x0, 0x12, 0x4, 0x1, 0xc, 0x2000000000000000};
#define ExtensionCatalog_CustomPropertyLookup_Start_value 0x15
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_CustomPropertyLookup_End = {0x16, 0x0, 0x12, 0x4, 0x2, 0xc, 0x2000000000000000};
#define ExtensionCatalog_CustomPropertyLookup_End_value 0x16
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_MoveNext_Start = {0x17, 0x0, 0x12, 0x4, 0x1, 0xd, 0x2000000000000000};
#define ExtensionCatalog_MoveNext_Start_value 0x17
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ExtensionCatalog_MoveNext_End = {0x18, 0x0, 0x12, 0x4, 0x2, 0xd, 0x2000000000000000};
#define ExtensionCatalog_MoveNext_End_value 0x18
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_CreateInstance_App_Start = {0x19, 0x0, 0x11, 0x4, 0x1, 0x6, 0x4000000000000000};
#define COM_CreateInstance_App_Start_value 0x19
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_CreateInstance_App_End = {0x1a, 0x0, 0x11, 0x4, 0x2, 0x6, 0x4000000000000000};
#define COM_CreateInstance_App_End_value 0x1a
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_BrokeredActivateInstance_Start = {0x1b, 0x0, 0x10, 0x4, 0x1, 0xe, 0x8000000000000000};
#define WinRT_BrokeredActivateInstance_Start_value 0x1b
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_BrokeredActivateInstance_Stop = {0x1c, 0x0, 0x10, 0x4, 0x2, 0xe, 0x8000000000000000};
#define WinRT_BrokeredActivateInstance_Stop_value 0x1c
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_BrokeredGetActivationFactory_Start = {0x1d, 0x0, 0x10, 0x4, 0x1, 0xf, 0x8000000000000000};
#define WinRT_BrokeredGetActivationFactory_Start_value 0x1d
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_BrokeredGetActivationFactory_Stop = {0x1e, 0x0, 0x10, 0x4, 0x2, 0xf, 0x8000000000000000};
#define WinRT_BrokeredGetActivationFactory_Stop_value 0x1e
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_GetActivationFactory_InprocServerImpl_Start = {0x1f, 0x0, 0x10, 0x4, 0x1, 0x10, 0x8000000000000000};
#define WinRT_GetActivationFactory_InprocServerImpl_Start_value 0x1f
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_GetActivationFactory_InprocServerImpl_Stop = {0x20, 0x0, 0x10, 0x4, 0x2, 0x10, 0x8000000000000000};
#define WinRT_GetActivationFactory_InprocServerImpl_Stop_value 0x20
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_ActivateInstance_InprocServerImpl_Start = {0x21, 0x0, 0x10, 0x4, 0x1, 0x11, 0x8000000000000000};
#define WinRT_ActivateInstance_InprocServerImpl_Start_value 0x21
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRT_ActivateInstance_InprocServerImpl_Stop = {0x22, 0x0, 0x10, 0x4, 0x2, 0x11, 0x8000000000000000};
#define WinRT_ActivateInstance_InprocServerImpl_Stop_value 0x22
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_GetClassFactory_InprocServerImpl_Start = {0x23, 0x0, 0x11, 0x4, 0x1, 0x12, 0x4000000000000000};
#define COM_GetClassFactory_InprocServerImpl_Start_value 0x23
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_GetClassFactory_InprocServerImpl_Stop = {0x24, 0x0, 0x11, 0x4, 0x2, 0x12, 0x4000000000000000};
#define COM_GetClassFactory_InprocServerImpl_Stop_value 0x24
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_CreateInstance_InprocServerImpl_Start = {0x25, 0x0, 0x11, 0x4, 0x1, 0x13, 0x4000000000000000};
#define COM_CreateInstance_InprocServerImpl_Start_value 0x25
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_CreateInstance_InprocServerImpl_Stop = {0x26, 0x0, 0x11, 0x4, 0x2, 0x13, 0x4000000000000000};
#define COM_CreateInstance_InprocServerImpl_Stop_value 0x26
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ClientSyncCall_Start = {0x27, 0x0, 0x13, 0x4, 0x1, 0x14, 0x1000000000000000};
#define COM_ClientSyncCall_Start_value 0x27
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ClientSyncCall_Stop = {0x28, 0x0, 0x13, 0x4, 0x2, 0x14, 0x1000000000000000};
#define COM_ClientSyncCall_Stop_value 0x28
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ClientAsyncCall_Start = {0x29, 0x0, 0x13, 0x4, 0x1, 0x15, 0x1000000000000000};
#define COM_ClientAsyncCall_Start_value 0x29
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ClientAsyncCall_Stop = {0x2a, 0x0, 0x13, 0x4, 0x2, 0x15, 0x1000000000000000};
#define COM_ClientAsyncCall_Stop_value 0x2a
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ServerSyncCall_Start = {0x2b, 0x1, 0x13, 0x4, 0x1, 0x16, 0x1000000000000000};
#define COM_ServerSyncCall_Start_value 0x2b
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ServerSyncCall_Stop = {0x2c, 0x0, 0x13, 0x4, 0x2, 0x16, 0x1000000000000000};
#define COM_ServerSyncCall_Stop_value 0x2c
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ServerAsyncCall_Start = {0x2d, 0x0, 0x13, 0x4, 0x1, 0x17, 0x1000000000000000};
#define COM_ServerAsyncCall_Start_value 0x2d
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR COM_ServerAsyncCall_Stop = {0x2e, 0x0, 0x13, 0x4, 0x2, 0x17, 0x1000000000000000};
#define COM_ServerAsyncCall_Stop_value 0x2e

//
// Note on Generate Code from Manifest Windows Vista and above
//
//Structures :  are handled as a size and pointer pairs. The macro for the event will have an extra 
//parameter for the size in bytes of the structure. Make sure that your structures have no extra padding.
//
//Strings: There are several cases that can be described in the manifest. For array of variable length 
//strings, the generated code will take the count of characters for the whole array as an input parameter. 
//
//SID No support for array of SIDs, the macro will take a pointer to the SID and use appropriate 
//GetLengthSid function to get the length.
//

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Globals 
//


//
// Event Enablement Bits
//

EXTERN_C __declspec(selectany) DECLSPEC_CACHEALIGN ULONG Microsoft_Windows_COM_PerfEnableBits[1];
EXTERN_C __declspec(selectany) const ULONGLONG Microsoft_Windows_COM_PerfKeywords[4] = {0x8000000000000000, 0x4000000000000000, 0x2000000000000000, 0x1000000000000000};
EXTERN_C __declspec(selectany) const UCHAR Microsoft_Windows_COM_PerfLevels[4] = {4, 4, 4, 4};
EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT COM_PERFORMANCE_PROVIDER_Context = {0, 0, 0, 0, 0, 0, 0, 0, 4, Microsoft_Windows_COM_PerfEnableBits, Microsoft_Windows_COM_PerfKeywords, Microsoft_Windows_COM_PerfLevels};

EXTERN_C __declspec(selectany) REGHANDLE Microsoft_Windows_COM_PerfHandle = (REGHANDLE)0;

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    _In_ LPCGUID ProviderId,
    _In_opt_ PENABLECALLBACK EnableCallback,
    _In_opt_ PVOID CallbackContext,
    _Inout_ PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW USER mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG errorCode;


    if (*RegHandle) {
        //
        // already registered
        //
        return ERROR_SUCCESS;
    }

    errorCode = EventRegister( ProviderId, EnableCallback, CallbackContext, RegHandle);

    return errorCode;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(_Inout_ PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW USER mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG errorCode;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return ERROR_SUCCESS;
    }

    errorCode = EventUnregister(*RegHandle);
    *RegHandle = (REGHANDLE)0;
    
    return errorCode;
}
#endif
//
// Register with ETW Vista +
//
#ifndef EventRegisterMicrosoft_Windows_COM_Perf
#define EventRegisterMicrosoft_Windows_COM_Perf() McGenEventRegister(&COM_PERFORMANCE_PROVIDER, McGenControlCallbackV2, &COM_PERFORMANCE_PROVIDER_Context, &Microsoft_Windows_COM_PerfHandle) 
#endif

//
// UnRegister with ETW
//
#ifndef EventUnregisterMicrosoft_Windows_COM_Perf
#define EventUnregisterMicrosoft_Windows_COM_Perf() McGenEventUnregister(&Microsoft_Windows_COM_PerfHandle) 
#endif

//
// Enablement check macro for WinRT_Inproc_ActivateInstance_Start
//

#define EventEnabledWinRT_Inproc_ActivateInstance_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_Inproc_ActivateInstance_Start
//
#define EventWriteWinRT_Inproc_ActivateInstance_Start()\
        EventEnabledWinRT_Inproc_ActivateInstance_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_Inproc_ActivateInstance_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_Inproc_ActivateInstance_End
//

#define EventEnabledWinRT_Inproc_ActivateInstance_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_Inproc_ActivateInstance_End
//
#define EventWriteWinRT_Inproc_ActivateInstance_End()\
        EventEnabledWinRT_Inproc_ActivateInstance_End() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_Inproc_ActivateInstance_End)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_OutOfProc_ActivateInstance_Start
//

#define EventEnabledWinRT_OutOfProc_ActivateInstance_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_OutOfProc_ActivateInstance_Start
//
#define EventWriteWinRT_OutOfProc_ActivateInstance_Start()\
        EventEnabledWinRT_OutOfProc_ActivateInstance_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_OutOfProc_ActivateInstance_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_OutOfProc_ActivateInstance_End
//

#define EventEnabledWinRT_OutOfProc_ActivateInstance_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_OutOfProc_ActivateInstance_End
//
#define EventWriteWinRT_OutOfProc_ActivateInstance_End()\
        EventEnabledWinRT_OutOfProc_ActivateInstance_End() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_OutOfProc_ActivateInstance_End)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_CreateInstance_Start
//

#define EventEnabledCOM_CreateInstance_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_CreateInstance_Start
//
#define EventWriteCOM_CreateInstance_Start()\
        EventEnabledCOM_CreateInstance_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_CreateInstance_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_CreateInstance_End
//

#define EventEnabledCOM_CreateInstance_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_CreateInstance_End
//
#define EventWriteCOM_CreateInstance_End()\
        EventEnabledCOM_CreateInstance_End() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_CreateInstance_End)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_Inproc_GetActivationFactory_Start
//

#define EventEnabledWinRT_Inproc_GetActivationFactory_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_Inproc_GetActivationFactory_Start
//
#define EventWriteWinRT_Inproc_GetActivationFactory_Start()\
        EventEnabledWinRT_Inproc_GetActivationFactory_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_Inproc_GetActivationFactory_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_Inproc_GetActivationFactory_End
//

#define EventEnabledWinRT_Inproc_GetActivationFactory_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_Inproc_GetActivationFactory_End
//
#define EventWriteWinRT_Inproc_GetActivationFactory_End()\
        EventEnabledWinRT_Inproc_GetActivationFactory_End() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_Inproc_GetActivationFactory_End)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_OutOfProc_GetActivationFactory_Start
//

#define EventEnabledWinRT_OutOfProc_GetActivationFactory_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_OutOfProc_GetActivationFactory_Start
//
#define EventWriteWinRT_OutOfProc_GetActivationFactory_Start()\
        EventEnabledWinRT_OutOfProc_GetActivationFactory_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_OutOfProc_GetActivationFactory_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_OutOfProc_GetActivationFactory_End
//

#define EventEnabledWinRT_OutOfProc_GetActivationFactory_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_OutOfProc_GetActivationFactory_End
//
#define EventWriteWinRT_OutOfProc_GetActivationFactory_End()\
        EventEnabledWinRT_OutOfProc_GetActivationFactory_End() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_OutOfProc_GetActivationFactory_End)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_GetClassObject_Start
//

#define EventEnabledCOM_GetClassObject_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_GetClassObject_Start
//
#define EventWriteCOM_GetClassObject_Start()\
        EventEnabledCOM_GetClassObject_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_GetClassObject_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_GetClassObject_End
//

#define EventEnabledCOM_GetClassObject_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_GetClassObject_End
//
#define EventWriteCOM_GetClassObject_End()\
        EventEnabledCOM_GetClassObject_End() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_GetClassObject_End)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_RoGetExtensionRegistration_Start
//

#define EventEnabledExtensionCatalog_RoGetExtensionRegistration_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_RoGetExtensionRegistration_Start
//
#define EventWriteExtensionCatalog_RoGetExtensionRegistration_Start(ContractId, PackageId, ActivatableClassId)\
        EventEnabledExtensionCatalog_RoGetExtensionRegistration_Start() ?\
        Template_zzz(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_RoGetExtensionRegistration_Start, ContractId, PackageId, ActivatableClassId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_RoGetExtensionRegistration_End
//

#define EventEnabledExtensionCatalog_RoGetExtensionRegistration_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_RoGetExtensionRegistration_End
//
#define EventWriteExtensionCatalog_RoGetExtensionRegistration_End(HRESULT, ContractId, PackageId, ActivatableClassId, InboxAppsRegistrationScope)\
        EventEnabledExtensionCatalog_RoGetExtensionRegistration_End() ?\
        Template_dzzzt(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_RoGetExtensionRegistration_End, HRESULT, ContractId, PackageId, ActivatableClassId, InboxAppsRegistrationScope)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start
//

#define EventEnabledExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start
//
#define EventWriteExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start(ContractId, ExtensionId)\
        EventEnabledExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start() ?\
        Template_zz(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_Start, ContractId, ExtensionId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End
//

#define EventEnabledExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End
//
#define EventWriteExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End(HRESULT, ContractId, ExtensionId, InboxAppsRegistrationScope)\
        EventEnabledExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End() ?\
        Template_dzzt(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_RoGetExtensionRegistrationByExtensionId_End, HRESULT, ContractId, ExtensionId, InboxAppsRegistrationScope)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_QueryCatalog_Start
//

#define EventEnabledExtensionCatalog_QueryCatalog_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_QueryCatalog_Start
//
#define EventWriteExtensionCatalog_QueryCatalog_Start(ContractId)\
        EventEnabledExtensionCatalog_QueryCatalog_Start() ?\
        Template_z(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_QueryCatalog_Start, ContractId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_QueryCatalog_End
//

#define EventEnabledExtensionCatalog_QueryCatalog_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_QueryCatalog_End
//
#define EventWriteExtensionCatalog_QueryCatalog_End(HRESULT, ContractId)\
        EventEnabledExtensionCatalog_QueryCatalog_End() ?\
        Template_dz(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_QueryCatalog_End, HRESULT, ContractId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_QueryCatalogByPackageFamily_Start
//

#define EventEnabledExtensionCatalog_QueryCatalogByPackageFamily_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_QueryCatalogByPackageFamily_Start
//
#define EventWriteExtensionCatalog_QueryCatalogByPackageFamily_Start(ContractId, PackageFamilyName)\
        EventEnabledExtensionCatalog_QueryCatalogByPackageFamily_Start() ?\
        Template_zz(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_QueryCatalogByPackageFamily_Start, ContractId, PackageFamilyName)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_QueryCatalogByPackageFamily_End
//

#define EventEnabledExtensionCatalog_QueryCatalogByPackageFamily_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_QueryCatalogByPackageFamily_End
//
#define EventWriteExtensionCatalog_QueryCatalogByPackageFamily_End(HRESULT, ContractId, PackageFamilyName)\
        EventEnabledExtensionCatalog_QueryCatalogByPackageFamily_End() ?\
        Template_dzz(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_QueryCatalogByPackageFamily_End, HRESULT, ContractId, PackageFamilyName)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_CustomPropertyLookup_Start
//

#define EventEnabledExtensionCatalog_CustomPropertyLookup_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_CustomPropertyLookup_Start
//
#define EventWriteExtensionCatalog_CustomPropertyLookup_Start(CustomPropertyKey)\
        EventEnabledExtensionCatalog_CustomPropertyLookup_Start() ?\
        Template_z(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_CustomPropertyLookup_Start, CustomPropertyKey)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_CustomPropertyLookup_End
//

#define EventEnabledExtensionCatalog_CustomPropertyLookup_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_CustomPropertyLookup_End
//
#define EventWriteExtensionCatalog_CustomPropertyLookup_End(HRESULT, CustomPropertyKey)\
        EventEnabledExtensionCatalog_CustomPropertyLookup_End() ?\
        Template_dz(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_CustomPropertyLookup_End, HRESULT, CustomPropertyKey)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_MoveNext_Start
//

#define EventEnabledExtensionCatalog_MoveNext_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_MoveNext_Start
//
#define EventWriteExtensionCatalog_MoveNext_Start(ContractId)\
        EventEnabledExtensionCatalog_MoveNext_Start() ?\
        Template_z(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_MoveNext_Start, ContractId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ExtensionCatalog_MoveNext_End
//

#define EventEnabledExtensionCatalog_MoveNext_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for ExtensionCatalog_MoveNext_End
//
#define EventWriteExtensionCatalog_MoveNext_End(HRESULT, ContractId, PackageId, ActivatableClassId, InboxAppsRegistrationScope)\
        EventEnabledExtensionCatalog_MoveNext_End() ?\
        Template_dzzzt(Microsoft_Windows_COM_PerfHandle, &ExtensionCatalog_MoveNext_End, HRESULT, ContractId, PackageId, ActivatableClassId, InboxAppsRegistrationScope)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_CreateInstance_App_Start
//

#define EventEnabledCOM_CreateInstance_App_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_CreateInstance_App_Start
//
#define EventWriteCOM_CreateInstance_App_Start()\
        EventEnabledCOM_CreateInstance_App_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_CreateInstance_App_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_CreateInstance_App_End
//

#define EventEnabledCOM_CreateInstance_App_End() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_CreateInstance_App_End
//
#define EventWriteCOM_CreateInstance_App_End()\
        EventEnabledCOM_CreateInstance_App_End() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_CreateInstance_App_End)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_BrokeredActivateInstance_Start
//

#define EventEnabledWinRT_BrokeredActivateInstance_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_BrokeredActivateInstance_Start
//
#define EventWriteWinRT_BrokeredActivateInstance_Start()\
        EventEnabledWinRT_BrokeredActivateInstance_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_BrokeredActivateInstance_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_BrokeredActivateInstance_Stop
//

#define EventEnabledWinRT_BrokeredActivateInstance_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_BrokeredActivateInstance_Stop
//
#define EventWriteWinRT_BrokeredActivateInstance_Stop()\
        EventEnabledWinRT_BrokeredActivateInstance_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_BrokeredActivateInstance_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_BrokeredGetActivationFactory_Start
//

#define EventEnabledWinRT_BrokeredGetActivationFactory_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_BrokeredGetActivationFactory_Start
//
#define EventWriteWinRT_BrokeredGetActivationFactory_Start()\
        EventEnabledWinRT_BrokeredGetActivationFactory_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_BrokeredGetActivationFactory_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_BrokeredGetActivationFactory_Stop
//

#define EventEnabledWinRT_BrokeredGetActivationFactory_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_BrokeredGetActivationFactory_Stop
//
#define EventWriteWinRT_BrokeredGetActivationFactory_Stop()\
        EventEnabledWinRT_BrokeredGetActivationFactory_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_BrokeredGetActivationFactory_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_GetActivationFactory_InprocServerImpl_Start
//

#define EventEnabledWinRT_GetActivationFactory_InprocServerImpl_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_GetActivationFactory_InprocServerImpl_Start
//
#define EventWriteWinRT_GetActivationFactory_InprocServerImpl_Start()\
        EventEnabledWinRT_GetActivationFactory_InprocServerImpl_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_GetActivationFactory_InprocServerImpl_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_GetActivationFactory_InprocServerImpl_Stop
//

#define EventEnabledWinRT_GetActivationFactory_InprocServerImpl_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_GetActivationFactory_InprocServerImpl_Stop
//
#define EventWriteWinRT_GetActivationFactory_InprocServerImpl_Stop()\
        EventEnabledWinRT_GetActivationFactory_InprocServerImpl_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_GetActivationFactory_InprocServerImpl_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_ActivateInstance_InprocServerImpl_Start
//

#define EventEnabledWinRT_ActivateInstance_InprocServerImpl_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_ActivateInstance_InprocServerImpl_Start
//
#define EventWriteWinRT_ActivateInstance_InprocServerImpl_Start()\
        EventEnabledWinRT_ActivateInstance_InprocServerImpl_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_ActivateInstance_InprocServerImpl_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRT_ActivateInstance_InprocServerImpl_Stop
//

#define EventEnabledWinRT_ActivateInstance_InprocServerImpl_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for WinRT_ActivateInstance_InprocServerImpl_Stop
//
#define EventWriteWinRT_ActivateInstance_InprocServerImpl_Stop()\
        EventEnabledWinRT_ActivateInstance_InprocServerImpl_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &WinRT_ActivateInstance_InprocServerImpl_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_GetClassFactory_InprocServerImpl_Start
//

#define EventEnabledCOM_GetClassFactory_InprocServerImpl_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_GetClassFactory_InprocServerImpl_Start
//
#define EventWriteCOM_GetClassFactory_InprocServerImpl_Start()\
        EventEnabledCOM_GetClassFactory_InprocServerImpl_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_GetClassFactory_InprocServerImpl_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_GetClassFactory_InprocServerImpl_Stop
//

#define EventEnabledCOM_GetClassFactory_InprocServerImpl_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_GetClassFactory_InprocServerImpl_Stop
//
#define EventWriteCOM_GetClassFactory_InprocServerImpl_Stop()\
        EventEnabledCOM_GetClassFactory_InprocServerImpl_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_GetClassFactory_InprocServerImpl_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_CreateInstance_InprocServerImpl_Start
//

#define EventEnabledCOM_CreateInstance_InprocServerImpl_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_CreateInstance_InprocServerImpl_Start
//
#define EventWriteCOM_CreateInstance_InprocServerImpl_Start()\
        EventEnabledCOM_CreateInstance_InprocServerImpl_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_CreateInstance_InprocServerImpl_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_CreateInstance_InprocServerImpl_Stop
//

#define EventEnabledCOM_CreateInstance_InprocServerImpl_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for COM_CreateInstance_InprocServerImpl_Stop
//
#define EventWriteCOM_CreateInstance_InprocServerImpl_Stop()\
        EventEnabledCOM_CreateInstance_InprocServerImpl_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_COM_PerfHandle, &COM_CreateInstance_InprocServerImpl_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ClientSyncCall_Start
//

#define EventEnabledCOM_ClientSyncCall_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ClientSyncCall_Start
//
#define EventWriteCOM_ClientSyncCall_Start(CallTraceId, TargetProcessId, TargetThreadId, CausalityId, TargetMethod, TargetInterface)\
        EventEnabledCOM_ClientSyncCall_Start() ?\
        Template_qqqjqj(Microsoft_Windows_COM_PerfHandle, &COM_ClientSyncCall_Start, CallTraceId, TargetProcessId, TargetThreadId, CausalityId, TargetMethod, TargetInterface)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ClientSyncCall_Stop
//

#define EventEnabledCOM_ClientSyncCall_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ClientSyncCall_Stop
//
#define EventWriteCOM_ClientSyncCall_Stop(CallTraceId, HRESULT, SourceOfHRESULT)\
        EventEnabledCOM_ClientSyncCall_Stop() ?\
        Template_qdq(Microsoft_Windows_COM_PerfHandle, &COM_ClientSyncCall_Stop, CallTraceId, HRESULT, SourceOfHRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ClientAsyncCall_Start
//

#define EventEnabledCOM_ClientAsyncCall_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ClientAsyncCall_Start
//
#define EventWriteCOM_ClientAsyncCall_Start(CallTraceId, TargetProcessId, TargetThreadId, CausalityId, TargetMethod, TargetInterface, TargetInterfaceAsyncUuid)\
        EventEnabledCOM_ClientAsyncCall_Start() ?\
        Template_qqqjqjj(Microsoft_Windows_COM_PerfHandle, &COM_ClientAsyncCall_Start, CallTraceId, TargetProcessId, TargetThreadId, CausalityId, TargetMethod, TargetInterface, TargetInterfaceAsyncUuid)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ClientAsyncCall_Stop
//

#define EventEnabledCOM_ClientAsyncCall_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ClientAsyncCall_Stop
//
#define EventWriteCOM_ClientAsyncCall_Stop(CallTraceId, HRESULT, SourceOfHRESULT)\
        EventEnabledCOM_ClientAsyncCall_Stop() ?\
        Template_qdq(Microsoft_Windows_COM_PerfHandle, &COM_ClientAsyncCall_Stop, CallTraceId, HRESULT, SourceOfHRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ServerSyncCall_Start
//

#define EventEnabledCOM_ServerSyncCall_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ServerSyncCall_Start
//
#define EventWriteCOM_ServerSyncCall_Start(CallTraceId, SourceProcessId, SourceThreadId, CausalityId, TargetMethod, TargetInterface, InterfacePointer, MethodAddress)\
        EventEnabledCOM_ServerSyncCall_Start() ?\
        Template_qqqjqjpp(Microsoft_Windows_COM_PerfHandle, &COM_ServerSyncCall_Start, CallTraceId, SourceProcessId, SourceThreadId, CausalityId, TargetMethod, TargetInterface, InterfacePointer, MethodAddress)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ServerSyncCall_Stop
//

#define EventEnabledCOM_ServerSyncCall_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ServerSyncCall_Stop
//
#define EventWriteCOM_ServerSyncCall_Stop(CallTraceId, HRESULT, ServerException)\
        EventEnabledCOM_ServerSyncCall_Stop() ?\
        Template_qdq(Microsoft_Windows_COM_PerfHandle, &COM_ServerSyncCall_Stop, CallTraceId, HRESULT, ServerException)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ServerAsyncCall_Start
//

#define EventEnabledCOM_ServerAsyncCall_Start() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ServerAsyncCall_Start
//
#define EventWriteCOM_ServerAsyncCall_Start(CallTraceId, SourceProcessId, SourceThreadId, CausalityId, TargetMethod, TargetInterface, TargetInterfaceAsyncUuid)\
        EventEnabledCOM_ServerAsyncCall_Start() ?\
        Template_qqqjqjj(Microsoft_Windows_COM_PerfHandle, &COM_ServerAsyncCall_Start, CallTraceId, SourceProcessId, SourceThreadId, CausalityId, TargetMethod, TargetInterface, TargetInterfaceAsyncUuid)\
        : ERROR_SUCCESS\

//
// Enablement check macro for COM_ServerAsyncCall_Stop
//

#define EventEnabledCOM_ServerAsyncCall_Stop() ((Microsoft_Windows_COM_PerfEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for COM_ServerAsyncCall_Stop
//
#define EventWriteCOM_ServerAsyncCall_Stop(CallTraceId, HRESULT, ServerException)\
        EventEnabledCOM_ServerAsyncCall_Stop() ?\
        Template_qdq(Microsoft_Windows_COM_PerfHandle, &COM_ServerAsyncCall_Stop, CallTraceId, HRESULT, ServerException)\
        : ERROR_SUCCESS\

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//***
// Warning : Classic Crimson Provider Ignored 
// Provider : Microsoft-Windows-DistributedCOM
//***
//+
// Provider Microsoft-Windows-DistributedCOM Event Count 33
//+
EXTERN_C __declspec(selectany) const GUID S_olemsg_DCOM = {0x1b562e86, 0xb7aa, 0x4131, {0xba, 0xdc, 0xb6, 0xf3, 0xa0, 0x01, 0x40, 0x7e}};

//
// Channel
//
#define S_olemsg_DCOM_CHANNEL_SYS 0x8

//
// Event Descriptors
//
#define EVENT_RPCSS_CREATEPROCESS_FAILURE 0x2710L
#define EVENT_RPCSS_RUNAS_CREATEPROCESS_FAILURE 0x2711L
#define EVENT_RPCSS_LAUNCH_ACCESS_DENIED 0xC0002712L
#define EVENT_RPCSS_DEFAULT_LAUNCH_ACCESS_DENIED 0xC0002713L
#define EVENT_RPCSS_RUNAS_CANT_LOGIN 0x2714L
#define EVENT_RPCSS_START_SERVICE_FAILURE 0x2715L
#define EVENT_RPCSS_REMOTE_SIDE_ERROR 0x2716L
#define EVENT_RPCSS_ACTIVATION_ERROR 0xC0002717L
#define EVENT_RPCSS_REMOTE_SIDE_ERROR_WITH_FILE 0x2718L
#define EVENT_RPCSS_REMOTE_SIDE_UNAVAILABLE 0xC0002719L
#define EVENT_RPCSS_SERVER_START_TIMEOUT 0x271AL
#define EVENT_RPCSS_SERVER_NOT_RESPONDING 0xC000271BL
#define EVENT_DCOM_ASSERTION_FAILURE 0xC000271CL
#define EVENT_DCOM_COMPLUS_DISABLED 0x271EL
#define EVENT_DCOM_ACCESS_DENIED_IN_MACHINE_LAUNCH_RESTRICTION 0x271FL
#define EVENT_DCOM_ACCESS_DENIED_IN_SERVER_ACTIVATION_OR_LAUNCH_PERMISSION 0x2720L
#define EVENT_DCOM_ACCESS_DENIED_IN_SERVER_ACCESS_PERMISSION 0x2721L
#define EVENT_DCOM_ACCESS_DENIED_IN_SERVER_ACCESS_PERMISSION_PROGRAMMATIC 0x2722L
#define EVENT_DCOM_ACCESS_DENIED_IN_SERVER_MACHINE_ACCESS_RESTRICTION 0x2723L
#define EVENT_DCOM_INVALID_MACHINE_SECURITY_DESCRIPTOR 0x2724L
#define EVENT_DCOM_INVALID_SERVER_ACTIVATION_SECURITY_DESCRIPTOR 0x2725L
#define EVENT_DCOM_INVALID_SERVER_ACCESS_SECURITY_DESCRIPTOR 0x2726L
#define EVENT_DCOM_INVALID_SERVER_ACCESS_SECURITY_DESCRIPTOR_PROGRAMMATIC 0x2727L
#define EVENT_DCOM_INVALID_MACHINE_SECURITY_DESCRIPTOR_SDDL 0x2728L
#define EVENT_DCOM_EVENTLOG_SUPPRESSION_PERIOD_IN_EFFECT 0x4000272AL
#define EVENT_DCOM_ACCESS_DENIED_IN_MACHINE_ACTIVATION_OR_ACCESS_RESTRICTION 0x272BL
#define EVENT_RPCSS_REMOTE_SIDE_UNAVAILABLE_WITH_CLIENT_INFO 0x272CL
#define EVENT_RPCSS_STOP_SERVICE_FAILURE 0x272DL
#define EVENT_RPCSS_CREATEDEBUGGERPROCESS_FAILURE 0x272EL
#define EVENT_COM_UNMARSHALING_POLICY_BLOCKED_UNMARSHALER 0x272FL
#define EVENT_COM_UNMARSHALING_POLICY_BLOCKED_HANDLER 0x2730L
#define EVENT_COM_UNMARSHALING_POLICY_BLOCKED_ENVOY 0x2731L
#define EVENT_COM_UNMARSHALING_POLICY_BLOCKED_ACTIVATION_WITH_FLAG 0x2732L

//
// Note on Generate Code from Manifest Windows Vista and above
//
//Structures :  are handled as a size and pointer pairs. The macro for the event will have an extra 
//parameter for the size in bytes of the structure. Make sure that your structures have no extra padding.
//
//Strings: There are several cases that can be described in the manifest. For array of variable length 
//strings, the generated code will take the count of characters for the whole array as an input parameter. 
//
//SID No support for array of SIDs, the macro will take a pointer to the SID and use appropriate 
//GetLengthSid function to get the length.
//

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Globals 
//


//
// Event Enablement Bits
//

EXTERN_C __declspec(selectany) DECLSPEC_CACHEALIGN ULONG Microsoft_Windows_DistributedCOMEnableBits[1];
EXTERN_C __declspec(selectany) const ULONGLONG Microsoft_Windows_DistributedCOMKeywords[2] = {0x80000000000000, 0x80000000000000};
EXTERN_C __declspec(selectany) const UCHAR Microsoft_Windows_DistributedCOMLevels[2] = {2, 0};
EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT S_olemsg_DCOM_Context = {0, 0, 0, 0, 0, 0, 0, 0, 2, Microsoft_Windows_DistributedCOMEnableBits, Microsoft_Windows_DistributedCOMKeywords, Microsoft_Windows_DistributedCOMLevels};

EXTERN_C __declspec(selectany) REGHANDLE Microsoft_Windows_DistributedCOMHandle = (REGHANDLE)0;

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    _In_ LPCGUID ProviderId,
    _In_opt_ PENABLECALLBACK EnableCallback,
    _In_opt_ PVOID CallbackContext,
    _Inout_ PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW USER mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG Error;


    if (*RegHandle) {
        //
        // already registered
        //
        return ERROR_SUCCESS;
    }

    Error = EventRegister( ProviderId, EnableCallback, CallbackContext, RegHandle); 

    return Error;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(_Inout_ PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW USER mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG Error;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return ERROR_SUCCESS;
    }

    Error = EventUnregister(*RegHandle); 
    *RegHandle = (REGHANDLE)0;
    
    return Error;
}
#endif
//
// Register with ETW Vista +
//
#ifndef EventRegisterMicrosoft_Windows_DistributedCOM
#define EventRegisterMicrosoft_Windows_DistributedCOM() McGenEventRegister(&S_olemsg_DCOM, McGenControlCallbackV2, &S_olemsg_DCOM_Context, &Microsoft_Windows_DistributedCOMHandle) 
#endif

//
// UnRegister with ETW
//
#ifndef EventUnregisterMicrosoft_Windows_DistributedCOM
#define EventUnregisterMicrosoft_Windows_DistributedCOM() McGenEventUnregister(&Microsoft_Windows_DistributedCOMHandle) 
#endif

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//***
// Warning : Classic Crimson Provider Ignored 
// Provider : Microsoft-Windows-COMRuntime
//***
//+
// Provider Microsoft-Windows-COMRuntime Event Count 21
//+
EXTERN_C __declspec(selectany) const GUID S_olemsg_COM = {0xbf406804, 0x6afa, 0x46e7, {0x8a, 0x48, 0x6c, 0x35, 0x7e, 0x1d, 0x6d, 0x61}};

//
// Channel
//
#define S_olemsg_COM_CHANNEL_APP 0x9
#define S_olemsg_COM_CHANNEL_defch 0x10
#define S_olemsg_COM_CHANNEL_actch 0x11
#define S_olemsg_COM_CHANNEL_msgch 0x12

//
// Tasks
//
#define S_olemsg_COM_TASK_CliModalLoopDelay 0x1
#define S_olemsg_COM_TASK_ComCallDelay 0x2
#define S_olemsg_COM_TASK_DropKeyboardMessage 0x3
#define S_olemsg_COM_TASK_DropMouseMessage 0x4
#define S_olemsg_COM_TASK_DropQueueSyncMessage 0x5
#define S_olemsg_COM_TASK_DropPointerMessage 0x6
//
// Keyword
//
#define S_olemsg_COM_KEYWORD_CliModalLoop 0x1
#define S_olemsg_COM_KEYWORD_ComCallWaitAnalysis 0x2
#define S_olemsg_COM_KEYWORD_DropInputMessage 0x4

//
// Event Descriptors
//
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CliModalLoopDelayEvent = {0x1, 0x0, 0x10, 0x4, 0x0, 0x1, 0x4000000000000001};
#define CliModalLoopDelayEvent_value 0x1
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR ComCallDelayEvent = {0x2, 0x0, 0x10, 0x4, 0x0, 0x2, 0x4000000000000002};
#define ComCallDelayEvent_value 0x2
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR DropKeyboardMessage = {0x3, 0x0, 0x12, 0x4, 0x0, 0x3, 0x1000000000000004};
#define DropKeyboardMessage_value 0x3
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR DropMouseMessage = {0x4, 0x0, 0x12, 0x4, 0x0, 0x4, 0x1000000000000004};
#define DropMouseMessage_value 0x4
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR DropQueueSyncMessage = {0x5, 0x0, 0x12, 0x4, 0x0, 0x5, 0x1000000000000004};
#define DropQueueSyncMessage_value 0x5
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR DropPointerMessage = {0x6, 0x0, 0x12, 0x4, 0x0, 0x6, 0x1000000000000004};
#define DropPointerMessage_value 0x6
#define EVENT_COM_INVALID_ENDPOINT_DATA 0x471DL
#define EVENT_COM_ACCESS_DENIED_IN_MACHINE_LAUNCH_RESTRICTION 0x471FL
#define EVENT_COM_ACCESS_DENIED_IN_SERVER_ACTIVATION_OR_LAUNCH_PERMISSION 0x4720L
#define EVENT_COM_ACCESS_DENIED_IN_SERVER_ACCESS_PERMISSION 0x4721L
#define EVENT_COM_ACCESS_DENIED_IN_SERVER_ACCESS_PERMISSION_PROGRAMMATIC 0x4722L
#define EVENT_COM_ACCESS_DENIED_IN_SERVER_MACHINE_ACCESS_RESTRICTION 0x4723L
#define EVENT_COM_INVALID_MACHINE_SECURITY_DESCRIPTOR 0x4724L
#define EVENT_COM_INVALID_SERVER_ACTIVATION_SECURITY_DESCRIPTOR 0x4725L
#define EVENT_COM_INVALID_SERVER_ACCESS_SECURITY_DESCRIPTOR 0x4726L
#define EVENT_COM_INVALID_SERVER_ACCESS_SECURITY_DESCRIPTOR_PROGRAMMATIC 0x4727L
#define EVENT_COM_INVALID_MACHINE_SECURITY_DESCRIPTOR_SDDL 0x4728L
#define EVENT_COM_ACCESS_DENIED_IN_MACHINE_ACTIVATION_OR_ACCESS_RESTRICTION 0x472BL
#define EVENT_COM_CANNOT_CONVERT_EMBEDDED_CONTENT 0xC000472CL
#define EVENT_COM_ACCESS_DENIED_IN_CONNECTING_TO_RPCSS 0x472DL
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR WinRTInstantiateActivatableClass = {0x8001, 0x0, 0x11, 0x4, 0x0, 0x0, 0x2000000000000000};
#define WinRTInstantiateActivatableClass_value 0x8001

//
// Note on Generate Code from Manifest Windows Vista and above
//
//Structures :  are handled as a size and pointer pairs. The macro for the event will have an extra 
//parameter for the size in bytes of the structure. Make sure that your structures have no extra padding.
//
//Strings: There are several cases that can be described in the manifest. For array of variable length 
//strings, the generated code will take the count of characters for the whole array as an input parameter. 
//
//SID No support for array of SIDs, the macro will take a pointer to the SID and use appropriate 
//GetLengthSid function to get the length.
//

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Globals 
//


//
// Event Enablement Bits
//

EXTERN_C __declspec(selectany) DECLSPEC_CACHEALIGN ULONG Microsoft_Windows_COMRuntimeEnableBits[1];
EXTERN_C __declspec(selectany) const ULONGLONG Microsoft_Windows_COMRuntimeKeywords[6] = {0x4000000000000001, 0x4000000000000002, 0x1000000000000004, 0x80000000000000, 0x80000000000000, 0x2000000000000000};
EXTERN_C __declspec(selectany) const UCHAR Microsoft_Windows_COMRuntimeLevels[6] = {4, 4, 4, 2, 0, 4};
EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT S_olemsg_COM_Context = {0, 0, 0, 0, 0, 0, 0, 0, 6, Microsoft_Windows_COMRuntimeEnableBits, Microsoft_Windows_COMRuntimeKeywords, Microsoft_Windows_COMRuntimeLevels};

EXTERN_C __declspec(selectany) REGHANDLE Microsoft_Windows_COMRuntimeHandle = (REGHANDLE)0;

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    _In_ LPCGUID ProviderId,
    _In_opt_ PENABLECALLBACK EnableCallback,
    _In_opt_ PVOID CallbackContext,
    _Inout_ PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW USER mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG Error;


    if (*RegHandle) {
        //
        // already registered
        //
        return ERROR_SUCCESS;
    }

    Error = EventRegister( ProviderId, EnableCallback, CallbackContext, RegHandle); 

    return Error;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(_Inout_ PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW USER mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG Error;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return ERROR_SUCCESS;
    }

    Error = EventUnregister(*RegHandle); 
    *RegHandle = (REGHANDLE)0;
    
    return Error;
}
#endif
//
// Register with ETW Vista +
//
#ifndef EventRegisterMicrosoft_Windows_COMRuntime
#define EventRegisterMicrosoft_Windows_COMRuntime() McGenEventRegister(&S_olemsg_COM, McGenControlCallbackV2, &S_olemsg_COM_Context, &Microsoft_Windows_COMRuntimeHandle) 
#endif

//
// UnRegister with ETW
//
#ifndef EventUnregisterMicrosoft_Windows_COMRuntime
#define EventUnregisterMicrosoft_Windows_COMRuntime() McGenEventUnregister(&Microsoft_Windows_COMRuntimeHandle) 
#endif

//
// Enablement check macro for CliModalLoopDelayEvent
//

#define EventEnabledCliModalLoopDelayEvent() ((Microsoft_Windows_COMRuntimeEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for CliModalLoopDelayEvent
//
#define EventWriteCliModalLoopDelayEvent(Flags, BlockTimeMs, TotalTimeMs)\
        EventEnabledCliModalLoopDelayEvent() ?\
        Template_qqq(Microsoft_Windows_COMRuntimeHandle, &CliModalLoopDelayEvent, Flags, BlockTimeMs, TotalTimeMs)\
        : ERROR_SUCCESS\

//
// Enablement check macro for ComCallDelayEvent
//

#define EventEnabledComCallDelayEvent() ((Microsoft_Windows_COMRuntimeEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for ComCallDelayEvent
//
#define EventWriteComCallDelayEvent(Flags, CallTimeMs, CallResult, TargetThreadId, TargetProcessId, TargetMethod, TargetInterface)\
        EventEnabledComCallDelayEvent() ?\
        Template_qqqqqqj(Microsoft_Windows_COMRuntimeHandle, &ComCallDelayEvent, Flags, CallTimeMs, CallResult, TargetThreadId, TargetProcessId, TargetMethod, TargetInterface)\
        : ERROR_SUCCESS\

//
// Enablement check macro for DropKeyboardMessage
//

#define EventEnabledDropKeyboardMessage() ((Microsoft_Windows_COMRuntimeEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for DropKeyboardMessage
//
#define EventWriteDropKeyboardMessage(ApartmentKind)\
        EventEnabledDropKeyboardMessage() ?\
        Template_q(Microsoft_Windows_COMRuntimeHandle, &DropKeyboardMessage, ApartmentKind)\
        : ERROR_SUCCESS\

//
// Enablement check macro for DropMouseMessage
//

#define EventEnabledDropMouseMessage() ((Microsoft_Windows_COMRuntimeEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for DropMouseMessage
//
#define EventWriteDropMouseMessage(ApartmentKind)\
        EventEnabledDropMouseMessage() ?\
        Template_q(Microsoft_Windows_COMRuntimeHandle, &DropMouseMessage, ApartmentKind)\
        : ERROR_SUCCESS\

//
// Enablement check macro for DropQueueSyncMessage
//

#define EventEnabledDropQueueSyncMessage() ((Microsoft_Windows_COMRuntimeEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for DropQueueSyncMessage
//
#define EventWriteDropQueueSyncMessage(ApartmentKind)\
        EventEnabledDropQueueSyncMessage() ?\
        Template_q(Microsoft_Windows_COMRuntimeHandle, &DropQueueSyncMessage, ApartmentKind)\
        : ERROR_SUCCESS\

//
// Enablement check macro for DropPointerMessage
//

#define EventEnabledDropPointerMessage() ((Microsoft_Windows_COMRuntimeEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for DropPointerMessage
//
#define EventWriteDropPointerMessage(ApartmentKind, Message)\
        EventEnabledDropPointerMessage() ?\
        Template_qq(Microsoft_Windows_COMRuntimeHandle, &DropPointerMessage, ApartmentKind, Message)\
        : ERROR_SUCCESS\

//
// Enablement check macro for WinRTInstantiateActivatableClass
//

#define EventEnabledWinRTInstantiateActivatableClass() ((Microsoft_Windows_COMRuntimeEnableBits[0] & 0x00000020) != 0)

//
// Event Macro for WinRTInstantiateActivatableClass
//
#define EventWriteWinRTInstantiateActivatableClass(param1, param2, param3)\
        EventEnabledWinRTInstantiateActivatableClass() ?\
        Template_zzz(Microsoft_Windows_COMRuntimeHandle, &WinRTInstantiateActivatableClass, param1, param2, param3)\
        : ERROR_SUCCESS\

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//+
// Provider Microsoft-Windows-AsynchronousCausality Event Count 63
//+
EXTERN_C __declspec(selectany) const GUID ASYNCHRONOUS_CAUSALITY_PROVIDER = {0x19a4c69a, 0x28eb, 0x4d4b, {0x8d, 0x94, 0x5f, 0x19, 0x05, 0x5a, 0x1b, 0x5c}};

//
// Channel
//
#define chidAsynchronousCausality 0x10
//
// Keyword
//
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_AsynchronousOperation 0x1
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_SynchronousWorkItem 0x2
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_Relation 0x4
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_OperationWork 0x10
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_SystemOperation 0x100
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_LibraryOperation 0x200
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_ApplicationOperation 0x400
#define ASYNCHRONOUS_CAUSALITY_PROVIDER_KEYWORD_OperationCreate 0x800

//
// Event Descriptors
//
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateSystemAsynchronousOperation_Required = {0x1010, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000901};
#define CreateSystemAsynchronousOperation_Required_value 0x1010
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateLibraryAsynchronousOperation_Required = {0x1011, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000a01};
#define CreateLibraryAsynchronousOperation_Required_value 0x1011
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateApplicationAsynchronousOperation_Required = {0x1012, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000c01};
#define CreateApplicationAsynchronousOperation_Required_value 0x1012
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateSystemAsynchronousOperation_Important = {0x1030, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000901};
#define CreateSystemAsynchronousOperation_Important_value 0x1030
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateLibraryAsynchronousOperation_Important = {0x1031, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000a01};
#define CreateLibraryAsynchronousOperation_Important_value 0x1031
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateApplicationAsynchronousOperation_Important = {0x1032, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000c01};
#define CreateApplicationAsynchronousOperation_Important_value 0x1032
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateSystemAsynchronousOperation_Verbose = {0x1050, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000901};
#define CreateSystemAsynchronousOperation_Verbose_value 0x1050
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateLibraryAsynchronousOperation_Verbose = {0x1051, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000a01};
#define CreateLibraryAsynchronousOperation_Verbose_value 0x1051
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CreateApplicationAsynchronousOperation_Verbose = {0x1052, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000c01};
#define CreateApplicationAsynchronousOperation_Verbose_value 0x1052
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemAsynchronousOperation_Required = {0x2010, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000101};
#define CompleteSystemAsynchronousOperation_Required_value 0x2010
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibraryAsynchronousOperation_Required = {0x2011, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000201};
#define CompleteLibraryAsynchronousOperation_Required_value 0x2011
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationAsynchronousOperation_Required = {0x2012, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000401};
#define CompleteApplicationAsynchronousOperation_Required_value 0x2012
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemAsynchronousOperation_Important = {0x2030, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000101};
#define CompleteSystemAsynchronousOperation_Important_value 0x2030
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibraryAsynchronousOperation_Important = {0x2031, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000201};
#define CompleteLibraryAsynchronousOperation_Important_value 0x2031
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationAsynchronousOperation_Important = {0x2032, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000401};
#define CompleteApplicationAsynchronousOperation_Important_value 0x2032
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemAsynchronousOperation_Verbose = {0x2050, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000101};
#define CompleteSystemAsynchronousOperation_Verbose_value 0x2050
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibraryAsynchronousOperation_Verbose = {0x2051, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000201};
#define CompleteLibraryAsynchronousOperation_Verbose_value 0x2051
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationAsynchronousOperation_Verbose = {0x2052, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000401};
#define CompleteApplicationAsynchronousOperation_Verbose_value 0x2052
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateSystemAsynchronousOperation_Required = {0x3010, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000104};
#define RelateSystemAsynchronousOperation_Required_value 0x3010
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateLibraryAsynchronousOperation_Required = {0x3011, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000204};
#define RelateLibraryAsynchronousOperation_Required_value 0x3011
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateApplicationAsynchronousOperation_Required = {0x3012, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000404};
#define RelateApplicationAsynchronousOperation_Required_value 0x3012
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateSystemAsynchronousOperation_Important = {0x3030, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000104};
#define RelateSystemAsynchronousOperation_Important_value 0x3030
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateLibraryAsynchronousOperation_Important = {0x3031, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000204};
#define RelateLibraryAsynchronousOperation_Important_value 0x3031
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateApplicationAsynchronousOperation_Important = {0x3032, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000404};
#define RelateApplicationAsynchronousOperation_Important_value 0x3032
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateSystemAsynchronousOperation_Verbose = {0x3050, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000104};
#define RelateSystemAsynchronousOperation_Verbose_value 0x3050
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateLibraryAsynchronousOperation_Verbose = {0x3051, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000204};
#define RelateLibraryAsynchronousOperation_Verbose_value 0x3051
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR RelateApplicationAsynchronousOperation_Verbose = {0x3052, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000404};
#define RelateApplicationAsynchronousOperation_Verbose_value 0x3052
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartSystemSynchronousWorkItem_Required = {0x4010, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000102};
#define StartSystemSynchronousWorkItem_Required_value 0x4010
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartLibrarySynchronousWorkItem_Required = {0x4011, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000202};
#define StartLibrarySynchronousWorkItem_Required_value 0x4011
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartApplicationSynchronousWorkItem_Required = {0x4012, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000402};
#define StartApplicationSynchronousWorkItem_Required_value 0x4012
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartSystemSynchronousWorkItem_Important = {0x4030, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000102};
#define StartSystemSynchronousWorkItem_Important_value 0x4030
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartLibrarySynchronousWorkItem_Important = {0x4031, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000202};
#define StartLibrarySynchronousWorkItem_Important_value 0x4031
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartApplicationSynchronousWorkItem_Important = {0x4032, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000402};
#define StartApplicationSynchronousWorkItem_Important_value 0x4032
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartSystemSynchronousWorkItem_Verbose = {0x4050, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000102};
#define StartSystemSynchronousWorkItem_Verbose_value 0x4050
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartLibrarySynchronousWorkItem_Verbose = {0x4051, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000202};
#define StartLibrarySynchronousWorkItem_Verbose_value 0x4051
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartApplicationSynchronousWorkItem_Verbose = {0x4052, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000402};
#define StartApplicationSynchronousWorkItem_Verbose_value 0x4052
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartSystemSynchronousOperationWorkItem_Required = {0x4110, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000110};
#define StartSystemSynchronousOperationWorkItem_Required_value 0x4110
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartLibrarySynchronousOperationWorkItem_Required = {0x4111, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000210};
#define StartLibrarySynchronousOperationWorkItem_Required_value 0x4111
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartApplicationSynchronousOperationWorkItem_Required = {0x4112, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000410};
#define StartApplicationSynchronousOperationWorkItem_Required_value 0x4112
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartSystemSynchronousOperationWorkItem_Important = {0x4130, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000110};
#define StartSystemSynchronousOperationWorkItem_Important_value 0x4130
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartLibrarySynchronousOperationWorkItem_Important = {0x4131, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000210};
#define StartLibrarySynchronousOperationWorkItem_Important_value 0x4131
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartApplicationSynchronousOperationWorkItem_Important = {0x4132, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000410};
#define StartApplicationSynchronousOperationWorkItem_Important_value 0x4132
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartSystemSynchronousOperationWorkItem_Verbose = {0x4150, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000110};
#define StartSystemSynchronousOperationWorkItem_Verbose_value 0x4150
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartLibrarySynchronousOperationWorkItem_Verbose = {0x4151, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000210};
#define StartLibrarySynchronousOperationWorkItem_Verbose_value 0x4151
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR StartApplicationSynchronousOperationWorkItem_Verbose = {0x4152, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000410};
#define StartApplicationSynchronousOperationWorkItem_Verbose_value 0x4152
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemSynchronousWorkItem_Required = {0x5010, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000102};
#define CompleteSystemSynchronousWorkItem_Required_value 0x5010
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibrarySynchronousWorkItem_Required = {0x5011, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000202};
#define CompleteLibrarySynchronousWorkItem_Required_value 0x5011
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationSynchronousWorkItem_Required = {0x5012, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000402};
#define CompleteApplicationSynchronousWorkItem_Required_value 0x5012
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemSynchronousWorkItem_Important = {0x5030, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000102};
#define CompleteSystemSynchronousWorkItem_Important_value 0x5030
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibrarySynchronousWorkItem_Important = {0x5031, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000202};
#define CompleteLibrarySynchronousWorkItem_Important_value 0x5031
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationSynchronousWorkItem_Important = {0x5032, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000402};
#define CompleteApplicationSynchronousWorkItem_Important_value 0x5032
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemSynchronousWorkItem_Verbose = {0x5050, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000102};
#define CompleteSystemSynchronousWorkItem_Verbose_value 0x5050
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibrarySynchronousWorkItem_Verbose = {0x5051, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000202};
#define CompleteLibrarySynchronousWorkItem_Verbose_value 0x5051
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationSynchronousWorkItem_Verbose = {0x5052, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000402};
#define CompleteApplicationSynchronousWorkItem_Verbose_value 0x5052
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemSynchronousOperationWorkItem_Required = {0x5110, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000110};
#define CompleteSystemSynchronousOperationWorkItem_Required_value 0x5110
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibrarySynchronousOperationWorkItem_Required = {0x5111, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000210};
#define CompleteLibrarySynchronousOperationWorkItem_Required_value 0x5111
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationSynchronousOperationWorkItem_Required = {0x5112, 0x1, 0x10, 0x1, 0x0, 0x0, 0x8000000000000410};
#define CompleteApplicationSynchronousOperationWorkItem_Required_value 0x5112
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemSynchronousOperationWorkItem_Important = {0x5130, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000110};
#define CompleteSystemSynchronousOperationWorkItem_Important_value 0x5130
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibrarySynchronousOperationWorkItem_Important = {0x5131, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000210};
#define CompleteLibrarySynchronousOperationWorkItem_Important_value 0x5131
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationSynchronousOperationWorkItem_Important = {0x5132, 0x1, 0x10, 0x3, 0x0, 0x0, 0x8000000000000410};
#define CompleteApplicationSynchronousOperationWorkItem_Important_value 0x5132
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteSystemSynchronousOperationWorkItem_Verbose = {0x5150, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000110};
#define CompleteSystemSynchronousOperationWorkItem_Verbose_value 0x5150
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteLibrarySynchronousOperationWorkItem_Verbose = {0x5151, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000210};
#define CompleteLibrarySynchronousOperationWorkItem_Verbose_value 0x5151
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR CompleteApplicationSynchronousOperationWorkItem_Verbose = {0x5152, 0x1, 0x10, 0x5, 0x0, 0x0, 0x8000000000000410};
#define CompleteApplicationSynchronousOperationWorkItem_Verbose_value 0x5152

//
// Note on Generate Code from Manifest Windows Vista and above
//
//Structures :  are handled as a size and pointer pairs. The macro for the event will have an extra 
//parameter for the size in bytes of the structure. Make sure that your structures have no extra padding.
//
//Strings: There are several cases that can be described in the manifest. For array of variable length 
//strings, the generated code will take the count of characters for the whole array as an input parameter. 
//
//SID No support for array of SIDs, the macro will take a pointer to the SID and use appropriate 
//GetLengthSid function to get the length.
//

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Globals 
//


//
// Event Enablement Bits
//

EXTERN_C __declspec(selectany) DECLSPEC_CACHEALIGN ULONG Microsoft_Windows_AsynchronousCausalityEnableBits[2];
EXTERN_C __declspec(selectany) const ULONGLONG Microsoft_Windows_AsynchronousCausalityKeywords[45] = {0x8000000000000901, 0x8000000000000a01, 0x8000000000000c01, 0x8000000000000901, 0x8000000000000a01, 0x8000000000000c01, 0x8000000000000901, 0x8000000000000a01, 0x8000000000000c01, 0x8000000000000101, 0x8000000000000201, 0x8000000000000401, 0x8000000000000101, 0x8000000000000201, 0x8000000000000401, 0x8000000000000101, 0x8000000000000201, 0x8000000000000401, 0x8000000000000104, 0x8000000000000204, 0x8000000000000404, 0x8000000000000104, 0x8000000000000204, 0x8000000000000404, 0x8000000000000104, 0x8000000000000204, 0x8000000000000404, 0x8000000000000102, 0x8000000000000202, 0x8000000000000402, 0x8000000000000102, 0x8000000000000202, 0x8000000000000402, 0x8000000000000102, 0x8000000000000202, 0x8000000000000402, 0x8000000000000110, 0x8000000000000210, 0x8000000000000410, 0x8000000000000110, 0x8000000000000210, 0x8000000000000410, 0x8000000000000110, 0x8000000000000210, 0x8000000000000410};
EXTERN_C __declspec(selectany) const UCHAR Microsoft_Windows_AsynchronousCausalityLevels[45] = {1, 1, 1, 3, 3, 3, 5, 5, 5, 1, 1, 1, 3, 3, 3, 5, 5, 5, 1, 1, 1, 3, 3, 3, 5, 5, 5, 1, 1, 1, 3, 3, 3, 5, 5, 5, 1, 1, 1, 3, 3, 3, 5, 5, 5};
EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT ASYNCHRONOUS_CAUSALITY_PROVIDER_Context = {0, 0, 0, 0, 0, 0, 0, 0, 45, Microsoft_Windows_AsynchronousCausalityEnableBits, Microsoft_Windows_AsynchronousCausalityKeywords, Microsoft_Windows_AsynchronousCausalityLevels};

EXTERN_C __declspec(selectany) REGHANDLE Microsoft_Windows_AsynchronousCausalityHandle = (REGHANDLE)0;

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    _In_ LPCGUID ProviderId,
    _In_opt_ PENABLECALLBACK EnableCallback,
    _In_opt_ PVOID CallbackContext,
    _Inout_ PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW USER mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG Error;


    if (*RegHandle) {
        //
        // already registered
        //
        return ERROR_SUCCESS;
    }

    Error = EventRegister( ProviderId, EnableCallback, CallbackContext, RegHandle); 

    return Error;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(_Inout_ PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW USER mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG Error;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return ERROR_SUCCESS;
    }

    Error = EventUnregister(*RegHandle); 
    *RegHandle = (REGHANDLE)0;
    
    return Error;
}
#endif
//
// Register with ETW Vista +
//
#ifndef EventRegisterMicrosoft_Windows_AsynchronousCausality
#define EventRegisterMicrosoft_Windows_AsynchronousCausality() McGenEventRegister(&ASYNCHRONOUS_CAUSALITY_PROVIDER, McGenControlCallbackV2, &ASYNCHRONOUS_CAUSALITY_PROVIDER_Context, &Microsoft_Windows_AsynchronousCausalityHandle) 
#endif

//
// UnRegister with ETW
//
#ifndef EventUnregisterMicrosoft_Windows_AsynchronousCausality
#define EventUnregisterMicrosoft_Windows_AsynchronousCausality() McGenEventUnregister(&Microsoft_Windows_AsynchronousCausalityHandle) 
#endif

//
// Enablement check macro for CreateSystemAsynchronousOperation_Required
//

#define EventEnabledCreateSystemAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for CreateSystemAsynchronousOperation_Required
//
#define EventWriteCreateSystemAsynchronousOperation_Required(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateSystemAsynchronousOperation_Required() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateSystemAsynchronousOperation_Required, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateLibraryAsynchronousOperation_Required
//

#define EventEnabledCreateLibraryAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000002) != 0)

//
// Event Macro for CreateLibraryAsynchronousOperation_Required
//
#define EventWriteCreateLibraryAsynchronousOperation_Required(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateLibraryAsynchronousOperation_Required() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateLibraryAsynchronousOperation_Required, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateApplicationAsynchronousOperation_Required
//

#define EventEnabledCreateApplicationAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000004) != 0)

//
// Event Macro for CreateApplicationAsynchronousOperation_Required
//
#define EventWriteCreateApplicationAsynchronousOperation_Required(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateApplicationAsynchronousOperation_Required() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateApplicationAsynchronousOperation_Required, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateSystemAsynchronousOperation_Important
//

#define EventEnabledCreateSystemAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000008) != 0)

//
// Event Macro for CreateSystemAsynchronousOperation_Important
//
#define EventWriteCreateSystemAsynchronousOperation_Important(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateSystemAsynchronousOperation_Important() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateSystemAsynchronousOperation_Important, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateLibraryAsynchronousOperation_Important
//

#define EventEnabledCreateLibraryAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000010) != 0)

//
// Event Macro for CreateLibraryAsynchronousOperation_Important
//
#define EventWriteCreateLibraryAsynchronousOperation_Important(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateLibraryAsynchronousOperation_Important() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateLibraryAsynchronousOperation_Important, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateApplicationAsynchronousOperation_Important
//

#define EventEnabledCreateApplicationAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000020) != 0)

//
// Event Macro for CreateApplicationAsynchronousOperation_Important
//
#define EventWriteCreateApplicationAsynchronousOperation_Important(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateApplicationAsynchronousOperation_Important() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateApplicationAsynchronousOperation_Important, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateSystemAsynchronousOperation_Verbose
//

#define EventEnabledCreateSystemAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000040) != 0)

//
// Event Macro for CreateSystemAsynchronousOperation_Verbose
//
#define EventWriteCreateSystemAsynchronousOperation_Verbose(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateSystemAsynchronousOperation_Verbose() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateSystemAsynchronousOperation_Verbose, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateLibraryAsynchronousOperation_Verbose
//

#define EventEnabledCreateLibraryAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000080) != 0)

//
// Event Macro for CreateLibraryAsynchronousOperation_Verbose
//
#define EventWriteCreateLibraryAsynchronousOperation_Verbose(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateLibraryAsynchronousOperation_Verbose() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateLibraryAsynchronousOperation_Verbose, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CreateApplicationAsynchronousOperation_Verbose
//

#define EventEnabledCreateApplicationAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000100) != 0)

//
// Event Macro for CreateApplicationAsynchronousOperation_Verbose
//
#define EventWriteCreateApplicationAsynchronousOperation_Verbose(PlatformId, OperationId, OperationName, RelatedId)\
        EventEnabledCreateApplicationAsynchronousOperation_Verbose() ?\
        Template_jxzx(Microsoft_Windows_AsynchronousCausalityHandle, &CreateApplicationAsynchronousOperation_Verbose, PlatformId, OperationId, OperationName, RelatedId)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemAsynchronousOperation_Required
//

#define EventEnabledCompleteSystemAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000200) != 0)

//
// Event Macro for CompleteSystemAsynchronousOperation_Required
//
#define EventWriteCompleteSystemAsynchronousOperation_Required(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteSystemAsynchronousOperation_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemAsynchronousOperation_Required, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibraryAsynchronousOperation_Required
//

#define EventEnabledCompleteLibraryAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000400) != 0)

//
// Event Macro for CompleteLibraryAsynchronousOperation_Required
//
#define EventWriteCompleteLibraryAsynchronousOperation_Required(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteLibraryAsynchronousOperation_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibraryAsynchronousOperation_Required, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationAsynchronousOperation_Required
//

#define EventEnabledCompleteApplicationAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00000800) != 0)

//
// Event Macro for CompleteApplicationAsynchronousOperation_Required
//
#define EventWriteCompleteApplicationAsynchronousOperation_Required(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteApplicationAsynchronousOperation_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationAsynchronousOperation_Required, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemAsynchronousOperation_Important
//

#define EventEnabledCompleteSystemAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00001000) != 0)

//
// Event Macro for CompleteSystemAsynchronousOperation_Important
//
#define EventWriteCompleteSystemAsynchronousOperation_Important(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteSystemAsynchronousOperation_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemAsynchronousOperation_Important, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibraryAsynchronousOperation_Important
//

#define EventEnabledCompleteLibraryAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00002000) != 0)

//
// Event Macro for CompleteLibraryAsynchronousOperation_Important
//
#define EventWriteCompleteLibraryAsynchronousOperation_Important(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteLibraryAsynchronousOperation_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibraryAsynchronousOperation_Important, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationAsynchronousOperation_Important
//

#define EventEnabledCompleteApplicationAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00004000) != 0)

//
// Event Macro for CompleteApplicationAsynchronousOperation_Important
//
#define EventWriteCompleteApplicationAsynchronousOperation_Important(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteApplicationAsynchronousOperation_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationAsynchronousOperation_Important, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemAsynchronousOperation_Verbose
//

#define EventEnabledCompleteSystemAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00008000) != 0)

//
// Event Macro for CompleteSystemAsynchronousOperation_Verbose
//
#define EventWriteCompleteSystemAsynchronousOperation_Verbose(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteSystemAsynchronousOperation_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemAsynchronousOperation_Verbose, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibraryAsynchronousOperation_Verbose
//

#define EventEnabledCompleteLibraryAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00010000) != 0)

//
// Event Macro for CompleteLibraryAsynchronousOperation_Verbose
//
#define EventWriteCompleteLibraryAsynchronousOperation_Verbose(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteLibraryAsynchronousOperation_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibraryAsynchronousOperation_Verbose, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationAsynchronousOperation_Verbose
//

#define EventEnabledCompleteApplicationAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00020000) != 0)

//
// Event Macro for CompleteApplicationAsynchronousOperation_Verbose
//
#define EventWriteCompleteApplicationAsynchronousOperation_Verbose(PlatformId, OperationId, CompletionStatus)\
        EventEnabledCompleteApplicationAsynchronousOperation_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationAsynchronousOperation_Verbose, PlatformId, OperationId, CompletionStatus)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateSystemAsynchronousOperation_Required
//

#define EventEnabledRelateSystemAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00040000) != 0)

//
// Event Macro for RelateSystemAsynchronousOperation_Required
//
#define EventWriteRelateSystemAsynchronousOperation_Required(PlatformId, OperationId, Relation)\
        EventEnabledRelateSystemAsynchronousOperation_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateSystemAsynchronousOperation_Required, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateLibraryAsynchronousOperation_Required
//

#define EventEnabledRelateLibraryAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00080000) != 0)

//
// Event Macro for RelateLibraryAsynchronousOperation_Required
//
#define EventWriteRelateLibraryAsynchronousOperation_Required(PlatformId, OperationId, Relation)\
        EventEnabledRelateLibraryAsynchronousOperation_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateLibraryAsynchronousOperation_Required, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateApplicationAsynchronousOperation_Required
//

#define EventEnabledRelateApplicationAsynchronousOperation_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00100000) != 0)

//
// Event Macro for RelateApplicationAsynchronousOperation_Required
//
#define EventWriteRelateApplicationAsynchronousOperation_Required(PlatformId, OperationId, Relation)\
        EventEnabledRelateApplicationAsynchronousOperation_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateApplicationAsynchronousOperation_Required, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateSystemAsynchronousOperation_Important
//

#define EventEnabledRelateSystemAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00200000) != 0)

//
// Event Macro for RelateSystemAsynchronousOperation_Important
//
#define EventWriteRelateSystemAsynchronousOperation_Important(PlatformId, OperationId, Relation)\
        EventEnabledRelateSystemAsynchronousOperation_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateSystemAsynchronousOperation_Important, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateLibraryAsynchronousOperation_Important
//

#define EventEnabledRelateLibraryAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00400000) != 0)

//
// Event Macro for RelateLibraryAsynchronousOperation_Important
//
#define EventWriteRelateLibraryAsynchronousOperation_Important(PlatformId, OperationId, Relation)\
        EventEnabledRelateLibraryAsynchronousOperation_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateLibraryAsynchronousOperation_Important, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateApplicationAsynchronousOperation_Important
//

#define EventEnabledRelateApplicationAsynchronousOperation_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x00800000) != 0)

//
// Event Macro for RelateApplicationAsynchronousOperation_Important
//
#define EventWriteRelateApplicationAsynchronousOperation_Important(PlatformId, OperationId, Relation)\
        EventEnabledRelateApplicationAsynchronousOperation_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateApplicationAsynchronousOperation_Important, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateSystemAsynchronousOperation_Verbose
//

#define EventEnabledRelateSystemAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x01000000) != 0)

//
// Event Macro for RelateSystemAsynchronousOperation_Verbose
//
#define EventWriteRelateSystemAsynchronousOperation_Verbose(PlatformId, OperationId, Relation)\
        EventEnabledRelateSystemAsynchronousOperation_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateSystemAsynchronousOperation_Verbose, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateLibraryAsynchronousOperation_Verbose
//

#define EventEnabledRelateLibraryAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x02000000) != 0)

//
// Event Macro for RelateLibraryAsynchronousOperation_Verbose
//
#define EventWriteRelateLibraryAsynchronousOperation_Verbose(PlatformId, OperationId, Relation)\
        EventEnabledRelateLibraryAsynchronousOperation_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateLibraryAsynchronousOperation_Verbose, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for RelateApplicationAsynchronousOperation_Verbose
//

#define EventEnabledRelateApplicationAsynchronousOperation_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x04000000) != 0)

//
// Event Macro for RelateApplicationAsynchronousOperation_Verbose
//
#define EventWriteRelateApplicationAsynchronousOperation_Verbose(PlatformId, OperationId, Relation)\
        EventEnabledRelateApplicationAsynchronousOperation_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &RelateApplicationAsynchronousOperation_Verbose, PlatformId, OperationId, Relation)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartSystemSynchronousWorkItem_Required
//

#define EventEnabledStartSystemSynchronousWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x08000000) != 0)

//
// Event Macro for StartSystemSynchronousWorkItem_Required
//
#define EventWriteStartSystemSynchronousWorkItem_Required(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartSystemSynchronousWorkItem_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartSystemSynchronousWorkItem_Required, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartLibrarySynchronousWorkItem_Required
//

#define EventEnabledStartLibrarySynchronousWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x10000000) != 0)

//
// Event Macro for StartLibrarySynchronousWorkItem_Required
//
#define EventWriteStartLibrarySynchronousWorkItem_Required(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartLibrarySynchronousWorkItem_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartLibrarySynchronousWorkItem_Required, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartApplicationSynchronousWorkItem_Required
//

#define EventEnabledStartApplicationSynchronousWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x20000000) != 0)

//
// Event Macro for StartApplicationSynchronousWorkItem_Required
//
#define EventWriteStartApplicationSynchronousWorkItem_Required(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartApplicationSynchronousWorkItem_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartApplicationSynchronousWorkItem_Required, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartSystemSynchronousWorkItem_Important
//

#define EventEnabledStartSystemSynchronousWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x40000000) != 0)

//
// Event Macro for StartSystemSynchronousWorkItem_Important
//
#define EventWriteStartSystemSynchronousWorkItem_Important(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartSystemSynchronousWorkItem_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartSystemSynchronousWorkItem_Important, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartLibrarySynchronousWorkItem_Important
//

#define EventEnabledStartLibrarySynchronousWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x80000000) != 0)

//
// Event Macro for StartLibrarySynchronousWorkItem_Important
//
#define EventWriteStartLibrarySynchronousWorkItem_Important(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartLibrarySynchronousWorkItem_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartLibrarySynchronousWorkItem_Important, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartApplicationSynchronousWorkItem_Important
//

#define EventEnabledStartApplicationSynchronousWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000001) != 0)

//
// Event Macro for StartApplicationSynchronousWorkItem_Important
//
#define EventWriteStartApplicationSynchronousWorkItem_Important(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartApplicationSynchronousWorkItem_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartApplicationSynchronousWorkItem_Important, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartSystemSynchronousWorkItem_Verbose
//

#define EventEnabledStartSystemSynchronousWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000002) != 0)

//
// Event Macro for StartSystemSynchronousWorkItem_Verbose
//
#define EventWriteStartSystemSynchronousWorkItem_Verbose(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartSystemSynchronousWorkItem_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartSystemSynchronousWorkItem_Verbose, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartLibrarySynchronousWorkItem_Verbose
//

#define EventEnabledStartLibrarySynchronousWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000004) != 0)

//
// Event Macro for StartLibrarySynchronousWorkItem_Verbose
//
#define EventWriteStartLibrarySynchronousWorkItem_Verbose(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartLibrarySynchronousWorkItem_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartLibrarySynchronousWorkItem_Verbose, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartApplicationSynchronousWorkItem_Verbose
//

#define EventEnabledStartApplicationSynchronousWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000008) != 0)

//
// Event Macro for StartApplicationSynchronousWorkItem_Verbose
//
#define EventWriteStartApplicationSynchronousWorkItem_Verbose(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartApplicationSynchronousWorkItem_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartApplicationSynchronousWorkItem_Verbose, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartSystemSynchronousOperationWorkItem_Required
//

#define EventEnabledStartSystemSynchronousOperationWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000010) != 0)

//
// Event Macro for StartSystemSynchronousOperationWorkItem_Required
//
#define EventWriteStartSystemSynchronousOperationWorkItem_Required(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartSystemSynchronousOperationWorkItem_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartSystemSynchronousOperationWorkItem_Required, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartLibrarySynchronousOperationWorkItem_Required
//

#define EventEnabledStartLibrarySynchronousOperationWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000020) != 0)

//
// Event Macro for StartLibrarySynchronousOperationWorkItem_Required
//
#define EventWriteStartLibrarySynchronousOperationWorkItem_Required(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartLibrarySynchronousOperationWorkItem_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartLibrarySynchronousOperationWorkItem_Required, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartApplicationSynchronousOperationWorkItem_Required
//

#define EventEnabledStartApplicationSynchronousOperationWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000040) != 0)

//
// Event Macro for StartApplicationSynchronousOperationWorkItem_Required
//
#define EventWriteStartApplicationSynchronousOperationWorkItem_Required(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartApplicationSynchronousOperationWorkItem_Required() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartApplicationSynchronousOperationWorkItem_Required, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartSystemSynchronousOperationWorkItem_Important
//

#define EventEnabledStartSystemSynchronousOperationWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000080) != 0)

//
// Event Macro for StartSystemSynchronousOperationWorkItem_Important
//
#define EventWriteStartSystemSynchronousOperationWorkItem_Important(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartSystemSynchronousOperationWorkItem_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartSystemSynchronousOperationWorkItem_Important, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartLibrarySynchronousOperationWorkItem_Important
//

#define EventEnabledStartLibrarySynchronousOperationWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000100) != 0)

//
// Event Macro for StartLibrarySynchronousOperationWorkItem_Important
//
#define EventWriteStartLibrarySynchronousOperationWorkItem_Important(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartLibrarySynchronousOperationWorkItem_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartLibrarySynchronousOperationWorkItem_Important, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartApplicationSynchronousOperationWorkItem_Important
//

#define EventEnabledStartApplicationSynchronousOperationWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000200) != 0)

//
// Event Macro for StartApplicationSynchronousOperationWorkItem_Important
//
#define EventWriteStartApplicationSynchronousOperationWorkItem_Important(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartApplicationSynchronousOperationWorkItem_Important() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartApplicationSynchronousOperationWorkItem_Important, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartSystemSynchronousOperationWorkItem_Verbose
//

#define EventEnabledStartSystemSynchronousOperationWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000400) != 0)

//
// Event Macro for StartSystemSynchronousOperationWorkItem_Verbose
//
#define EventWriteStartSystemSynchronousOperationWorkItem_Verbose(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartSystemSynchronousOperationWorkItem_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartSystemSynchronousOperationWorkItem_Verbose, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartLibrarySynchronousOperationWorkItem_Verbose
//

#define EventEnabledStartLibrarySynchronousOperationWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000800) != 0)

//
// Event Macro for StartLibrarySynchronousOperationWorkItem_Verbose
//
#define EventWriteStartLibrarySynchronousOperationWorkItem_Verbose(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartLibrarySynchronousOperationWorkItem_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartLibrarySynchronousOperationWorkItem_Verbose, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for StartApplicationSynchronousOperationWorkItem_Verbose
//

#define EventEnabledStartApplicationSynchronousOperationWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00001000) != 0)

//
// Event Macro for StartApplicationSynchronousOperationWorkItem_Verbose
//
#define EventWriteStartApplicationSynchronousOperationWorkItem_Verbose(PlatformId, OperationId, WorkItemType)\
        EventEnabledStartApplicationSynchronousOperationWorkItem_Verbose() ?\
        Template_jxc(Microsoft_Windows_AsynchronousCausalityHandle, &StartApplicationSynchronousOperationWorkItem_Verbose, PlatformId, OperationId, WorkItemType)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemSynchronousWorkItem_Required
//

#define EventEnabledCompleteSystemSynchronousWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x08000000) != 0)

//
// Event Macro for CompleteSystemSynchronousWorkItem_Required
//
#define EventWriteCompleteSystemSynchronousWorkItem_Required()\
        EventEnabledCompleteSystemSynchronousWorkItem_Required() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemSynchronousWorkItem_Required)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibrarySynchronousWorkItem_Required
//

#define EventEnabledCompleteLibrarySynchronousWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x10000000) != 0)

//
// Event Macro for CompleteLibrarySynchronousWorkItem_Required
//
#define EventWriteCompleteLibrarySynchronousWorkItem_Required()\
        EventEnabledCompleteLibrarySynchronousWorkItem_Required() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibrarySynchronousWorkItem_Required)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationSynchronousWorkItem_Required
//

#define EventEnabledCompleteApplicationSynchronousWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x20000000) != 0)

//
// Event Macro for CompleteApplicationSynchronousWorkItem_Required
//
#define EventWriteCompleteApplicationSynchronousWorkItem_Required()\
        EventEnabledCompleteApplicationSynchronousWorkItem_Required() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationSynchronousWorkItem_Required)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemSynchronousWorkItem_Important
//

#define EventEnabledCompleteSystemSynchronousWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x40000000) != 0)

//
// Event Macro for CompleteSystemSynchronousWorkItem_Important
//
#define EventWriteCompleteSystemSynchronousWorkItem_Important()\
        EventEnabledCompleteSystemSynchronousWorkItem_Important() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemSynchronousWorkItem_Important)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibrarySynchronousWorkItem_Important
//

#define EventEnabledCompleteLibrarySynchronousWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[0] & 0x80000000) != 0)

//
// Event Macro for CompleteLibrarySynchronousWorkItem_Important
//
#define EventWriteCompleteLibrarySynchronousWorkItem_Important()\
        EventEnabledCompleteLibrarySynchronousWorkItem_Important() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibrarySynchronousWorkItem_Important)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationSynchronousWorkItem_Important
//

#define EventEnabledCompleteApplicationSynchronousWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000001) != 0)

//
// Event Macro for CompleteApplicationSynchronousWorkItem_Important
//
#define EventWriteCompleteApplicationSynchronousWorkItem_Important()\
        EventEnabledCompleteApplicationSynchronousWorkItem_Important() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationSynchronousWorkItem_Important)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemSynchronousWorkItem_Verbose
//

#define EventEnabledCompleteSystemSynchronousWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000002) != 0)

//
// Event Macro for CompleteSystemSynchronousWorkItem_Verbose
//
#define EventWriteCompleteSystemSynchronousWorkItem_Verbose()\
        EventEnabledCompleteSystemSynchronousWorkItem_Verbose() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemSynchronousWorkItem_Verbose)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibrarySynchronousWorkItem_Verbose
//

#define EventEnabledCompleteLibrarySynchronousWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000004) != 0)

//
// Event Macro for CompleteLibrarySynchronousWorkItem_Verbose
//
#define EventWriteCompleteLibrarySynchronousWorkItem_Verbose()\
        EventEnabledCompleteLibrarySynchronousWorkItem_Verbose() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibrarySynchronousWorkItem_Verbose)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationSynchronousWorkItem_Verbose
//

#define EventEnabledCompleteApplicationSynchronousWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000008) != 0)

//
// Event Macro for CompleteApplicationSynchronousWorkItem_Verbose
//
#define EventWriteCompleteApplicationSynchronousWorkItem_Verbose()\
        EventEnabledCompleteApplicationSynchronousWorkItem_Verbose() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationSynchronousWorkItem_Verbose)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemSynchronousOperationWorkItem_Required
//

#define EventEnabledCompleteSystemSynchronousOperationWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000010) != 0)

//
// Event Macro for CompleteSystemSynchronousOperationWorkItem_Required
//
#define EventWriteCompleteSystemSynchronousOperationWorkItem_Required()\
        EventEnabledCompleteSystemSynchronousOperationWorkItem_Required() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemSynchronousOperationWorkItem_Required)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibrarySynchronousOperationWorkItem_Required
//

#define EventEnabledCompleteLibrarySynchronousOperationWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000020) != 0)

//
// Event Macro for CompleteLibrarySynchronousOperationWorkItem_Required
//
#define EventWriteCompleteLibrarySynchronousOperationWorkItem_Required()\
        EventEnabledCompleteLibrarySynchronousOperationWorkItem_Required() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibrarySynchronousOperationWorkItem_Required)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationSynchronousOperationWorkItem_Required
//

#define EventEnabledCompleteApplicationSynchronousOperationWorkItem_Required() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000040) != 0)

//
// Event Macro for CompleteApplicationSynchronousOperationWorkItem_Required
//
#define EventWriteCompleteApplicationSynchronousOperationWorkItem_Required()\
        EventEnabledCompleteApplicationSynchronousOperationWorkItem_Required() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationSynchronousOperationWorkItem_Required)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemSynchronousOperationWorkItem_Important
//

#define EventEnabledCompleteSystemSynchronousOperationWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000080) != 0)

//
// Event Macro for CompleteSystemSynchronousOperationWorkItem_Important
//
#define EventWriteCompleteSystemSynchronousOperationWorkItem_Important()\
        EventEnabledCompleteSystemSynchronousOperationWorkItem_Important() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemSynchronousOperationWorkItem_Important)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibrarySynchronousOperationWorkItem_Important
//

#define EventEnabledCompleteLibrarySynchronousOperationWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000100) != 0)

//
// Event Macro for CompleteLibrarySynchronousOperationWorkItem_Important
//
#define EventWriteCompleteLibrarySynchronousOperationWorkItem_Important()\
        EventEnabledCompleteLibrarySynchronousOperationWorkItem_Important() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibrarySynchronousOperationWorkItem_Important)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationSynchronousOperationWorkItem_Important
//

#define EventEnabledCompleteApplicationSynchronousOperationWorkItem_Important() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000200) != 0)

//
// Event Macro for CompleteApplicationSynchronousOperationWorkItem_Important
//
#define EventWriteCompleteApplicationSynchronousOperationWorkItem_Important()\
        EventEnabledCompleteApplicationSynchronousOperationWorkItem_Important() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationSynchronousOperationWorkItem_Important)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteSystemSynchronousOperationWorkItem_Verbose
//

#define EventEnabledCompleteSystemSynchronousOperationWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000400) != 0)

//
// Event Macro for CompleteSystemSynchronousOperationWorkItem_Verbose
//
#define EventWriteCompleteSystemSynchronousOperationWorkItem_Verbose()\
        EventEnabledCompleteSystemSynchronousOperationWorkItem_Verbose() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteSystemSynchronousOperationWorkItem_Verbose)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteLibrarySynchronousOperationWorkItem_Verbose
//

#define EventEnabledCompleteLibrarySynchronousOperationWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00000800) != 0)

//
// Event Macro for CompleteLibrarySynchronousOperationWorkItem_Verbose
//
#define EventWriteCompleteLibrarySynchronousOperationWorkItem_Verbose()\
        EventEnabledCompleteLibrarySynchronousOperationWorkItem_Verbose() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteLibrarySynchronousOperationWorkItem_Verbose)\
        : ERROR_SUCCESS\

//
// Enablement check macro for CompleteApplicationSynchronousOperationWorkItem_Verbose
//

#define EventEnabledCompleteApplicationSynchronousOperationWorkItem_Verbose() ((Microsoft_Windows_AsynchronousCausalityEnableBits[1] & 0x00001000) != 0)

//
// Event Macro for CompleteApplicationSynchronousOperationWorkItem_Verbose
//
#define EventWriteCompleteApplicationSynchronousOperationWorkItem_Verbose()\
        EventEnabledCompleteApplicationSynchronousOperationWorkItem_Verbose() ?\
        TemplateEventDescriptor(Microsoft_Windows_AsynchronousCausalityHandle, &CompleteApplicationSynchronousOperationWorkItem_Verbose)\
        : ERROR_SUCCESS\

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//+
// Provider Microsoft-Windows-OLE-Perf Event Count 28
//+
EXTERN_C __declspec(selectany) const GUID OLE_PERFORMANCE_PROVIDER = {0x84958368, 0x7da7, 0x49a0, {0xb3, 0x3d, 0x07, 0xfa, 0xbb, 0x87, 0x96, 0x26}};

//
// Channel
//
#define chidOLEClipboard 0x10

//
// Tasks
//
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_Clipboard_GetClipboard 0x1
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_Clipboard_SetClipboard 0x2
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_Clipboard_Flush 0x3
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_Clipboard_IsCurrentClipboard 0x4
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_DoDragDrop 0x5
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_Register 0x6
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_Unregister 0x7
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_RegisterInternal 0x8
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_UnregisterInternal 0x9
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_CreateDragOperationInternal 0xa
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_DragOperation_UpdatePosition 0xb
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_DragOperation_SetDataObject 0xc
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_DragOperation_DoDrop 0xd
#define OLE_PERFORMANCE_PROVIDER_TASK_OLE_DragDrop_DragOperation_Cancel 0xe

//
// Event Descriptors
//
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_GetClipboard_Start = {0x1, 0x0, 0x10, 0x4, 0x1, 0x1, 0x8000000000000000};
#define OLE_Clipboard_GetClipboard_Start_value 0x1
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_GetClipboard_Stop = {0x2, 0x0, 0x10, 0x4, 0x2, 0x1, 0x8000000000000000};
#define OLE_Clipboard_GetClipboard_Stop_value 0x2
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_SetClipboard_Start = {0x3, 0x0, 0x10, 0x4, 0x1, 0x2, 0x8000000000000000};
#define OLE_Clipboard_SetClipboard_Start_value 0x3
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_SetClipboard_Stop = {0x4, 0x0, 0x10, 0x4, 0x2, 0x2, 0x8000000000000000};
#define OLE_Clipboard_SetClipboard_Stop_value 0x4
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_Flush_Start = {0x5, 0x0, 0x10, 0x4, 0x1, 0x3, 0x8000000000000000};
#define OLE_Clipboard_Flush_Start_value 0x5
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_Flush_Stop = {0x6, 0x0, 0x10, 0x4, 0x2, 0x3, 0x8000000000000000};
#define OLE_Clipboard_Flush_Stop_value 0x6
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_IsCurrentClipboard_Start = {0x7, 0x0, 0x10, 0x4, 0x1, 0x4, 0x8000000000000000};
#define OLE_Clipboard_IsCurrentClipboard_Start_value 0x7
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_Clipboard_IsCurrentClipboard_Stop = {0x8, 0x0, 0x10, 0x4, 0x2, 0x4, 0x8000000000000000};
#define OLE_Clipboard_IsCurrentClipboard_Stop_value 0x8
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DoDragDrop_Start = {0x9, 0x0, 0x10, 0x4, 0x1, 0x5, 0x8000000000000000};
#define OLE_DragDrop_DoDragDrop_Start_value 0x9
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DoDragDrop_Stop = {0xa, 0x0, 0x10, 0x4, 0x2, 0x5, 0x8000000000000000};
#define OLE_DragDrop_DoDragDrop_Stop_value 0xa
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_Register_Start = {0xb, 0x0, 0x10, 0x4, 0x1, 0x6, 0x8000000000000000};
#define OLE_DragDrop_Register_Start_value 0xb
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_Register_Stop = {0xc, 0x0, 0x10, 0x4, 0x2, 0x6, 0x8000000000000000};
#define OLE_DragDrop_Register_Stop_value 0xc
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_Unregister_Start = {0xd, 0x0, 0x10, 0x4, 0x1, 0x7, 0x8000000000000000};
#define OLE_DragDrop_Unregister_Start_value 0xd
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_Unregister_Stop = {0xe, 0x0, 0x10, 0x4, 0x2, 0x7, 0x8000000000000000};
#define OLE_DragDrop_Unregister_Stop_value 0xe
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_RegisterInternal_Start = {0xf, 0x0, 0x10, 0x4, 0x1, 0x8, 0x8000000000000000};
#define OLE_DragDrop_RegisterInternal_Start_value 0xf
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_RegisterInternal_Stop = {0x10, 0x0, 0x10, 0x4, 0x2, 0x8, 0x8000000000000000};
#define OLE_DragDrop_RegisterInternal_Stop_value 0x10
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_UnregisterInternal_Start = {0x11, 0x0, 0x10, 0x4, 0x1, 0x9, 0x8000000000000000};
#define OLE_DragDrop_UnregisterInternal_Start_value 0x11
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_UnregisterInternal_Stop = {0x12, 0x0, 0x10, 0x4, 0x2, 0x9, 0x8000000000000000};
#define OLE_DragDrop_UnregisterInternal_Stop_value 0x12
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_CreateDragOperationInternal_Start = {0x13, 0x0, 0x10, 0x4, 0x1, 0xa, 0x8000000000000000};
#define OLE_DragDrop_CreateDragOperationInternal_Start_value 0x13
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_CreateDragOperationInternal_Stop = {0x14, 0x0, 0x10, 0x4, 0x2, 0xa, 0x8000000000000000};
#define OLE_DragDrop_CreateDragOperationInternal_Stop_value 0x14
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_UpdatePosition_Start = {0x15, 0x0, 0x10, 0x4, 0x1, 0xb, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_UpdatePosition_Start_value 0x15
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_UpdatePosition_Stop = {0x16, 0x0, 0x10, 0x4, 0x2, 0xb, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_UpdatePosition_Stop_value 0x16
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_SetDataObject_Start = {0x17, 0x0, 0x10, 0x4, 0x1, 0xc, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_SetDataObject_Start_value 0x17
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_SetDataObject_Stop = {0x18, 0x0, 0x10, 0x4, 0x2, 0xc, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_SetDataObject_Stop_value 0x18
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_DoDrop_Start = {0x19, 0x0, 0x10, 0x4, 0x1, 0xd, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_DoDrop_Start_value 0x19
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_DoDrop_Stop = {0x1a, 0x0, 0x10, 0x4, 0x2, 0xd, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_DoDrop_Stop_value 0x1a
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_Cancel_Start = {0x1b, 0x0, 0x10, 0x4, 0x1, 0xe, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_Cancel_Start_value 0x1b
EXTERN_C __declspec(selectany) const EVENT_DESCRIPTOR OLE_DragDrop_DragOperation_Cancel_Stop = {0x1c, 0x0, 0x10, 0x4, 0x2, 0xe, 0x8000000000000000};
#define OLE_DragDrop_DragOperation_Cancel_Stop_value 0x1c

//
// Note on Generate Code from Manifest Windows Vista and above
//
//Structures :  are handled as a size and pointer pairs. The macro for the event will have an extra 
//parameter for the size in bytes of the structure. Make sure that your structures have no extra padding.
//
//Strings: There are several cases that can be described in the manifest. For array of variable length 
//strings, the generated code will take the count of characters for the whole array as an input parameter. 
//
//SID No support for array of SIDs, the macro will take a pointer to the SID and use appropriate 
//GetLengthSid function to get the length.
//

//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Globals 
//


//
// Event Enablement Bits
//

EXTERN_C __declspec(selectany) DECLSPEC_CACHEALIGN ULONG Microsoft_Windows_OLE_PerfEnableBits[1];
EXTERN_C __declspec(selectany) const ULONGLONG Microsoft_Windows_OLE_PerfKeywords[1] = {0x8000000000000000};
EXTERN_C __declspec(selectany) const UCHAR Microsoft_Windows_OLE_PerfLevels[1] = {4};
EXTERN_C __declspec(selectany) MCGEN_TRACE_CONTEXT OLE_PERFORMANCE_PROVIDER_Context = {0, 0, 0, 0, 0, 0, 0, 0, 1, Microsoft_Windows_OLE_PerfEnableBits, Microsoft_Windows_OLE_PerfKeywords, Microsoft_Windows_OLE_PerfLevels};

EXTERN_C __declspec(selectany) REGHANDLE Microsoft_Windows_OLE_PerfHandle = (REGHANDLE)0;

#if !defined(McGenEventRegisterUnregister)
#define McGenEventRegisterUnregister
DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventRegister(
    _In_ LPCGUID ProviderId,
    _In_opt_ PENABLECALLBACK EnableCallback,
    _In_opt_ PVOID CallbackContext,
    _Inout_ PREGHANDLE RegHandle
    )
/*++

Routine Description:

    This function register the provider with ETW USER mode.

Arguments:
    ProviderId - Provider Id to be register with ETW.

    EnableCallback - Callback to be used.

    CallbackContext - Context for this provider.

    RegHandle - Pointer to Registration handle.

Remarks:

    If the handle != NULL will return ERROR_SUCCESS

--*/
{
    ULONG Error;


    if (*RegHandle) {
        //
        // already registered
        //
        return ERROR_SUCCESS;
    }

    Error = EventRegister( ProviderId, EnableCallback, CallbackContext, RegHandle); 

    return Error;
}


DECLSPEC_NOINLINE __inline
ULONG __stdcall
McGenEventUnregister(_Inout_ PREGHANDLE RegHandle)
/*++

Routine Description:

    Unregister from ETW USER mode

Arguments:
            RegHandle this is the pointer to the provider context
Remarks:
            If Provider has not register RegHandle = NULL,
            return ERROR_SUCCESS
--*/
{
    ULONG Error;


    if(!(*RegHandle)) {
        //
        // Provider has not registerd
        //
        return ERROR_SUCCESS;
    }

    Error = EventUnregister(*RegHandle); 
    *RegHandle = (REGHANDLE)0;
    
    return Error;
}
#endif
//
// Register with ETW Vista +
//
#ifndef EventRegisterMicrosoft_Windows_OLE_Perf
#define EventRegisterMicrosoft_Windows_OLE_Perf() McGenEventRegister(&OLE_PERFORMANCE_PROVIDER, McGenControlCallbackV2, &OLE_PERFORMANCE_PROVIDER_Context, &Microsoft_Windows_OLE_PerfHandle) 
#endif

//
// UnRegister with ETW
//
#ifndef EventUnregisterMicrosoft_Windows_OLE_Perf
#define EventUnregisterMicrosoft_Windows_OLE_Perf() McGenEventUnregister(&Microsoft_Windows_OLE_PerfHandle) 
#endif

//
// Enablement check macro for OLE_Clipboard_GetClipboard_Start
//

#define EventEnabledOLE_Clipboard_GetClipboard_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_GetClipboard_Start
//
#define EventWriteOLE_Clipboard_GetClipboard_Start()\
        EventEnabledOLE_Clipboard_GetClipboard_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_GetClipboard_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_Clipboard_GetClipboard_Stop
//

#define EventEnabledOLE_Clipboard_GetClipboard_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_GetClipboard_Stop
//
#define EventWriteOLE_Clipboard_GetClipboard_Stop()\
        EventEnabledOLE_Clipboard_GetClipboard_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_GetClipboard_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_Clipboard_SetClipboard_Start
//

#define EventEnabledOLE_Clipboard_SetClipboard_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_SetClipboard_Start
//
#define EventWriteOLE_Clipboard_SetClipboard_Start()\
        EventEnabledOLE_Clipboard_SetClipboard_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_SetClipboard_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_Clipboard_SetClipboard_Stop
//

#define EventEnabledOLE_Clipboard_SetClipboard_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_SetClipboard_Stop
//
#define EventWriteOLE_Clipboard_SetClipboard_Stop()\
        EventEnabledOLE_Clipboard_SetClipboard_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_SetClipboard_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_Clipboard_Flush_Start
//

#define EventEnabledOLE_Clipboard_Flush_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_Flush_Start
//
#define EventWriteOLE_Clipboard_Flush_Start()\
        EventEnabledOLE_Clipboard_Flush_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_Flush_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_Clipboard_Flush_Stop
//

#define EventEnabledOLE_Clipboard_Flush_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_Flush_Stop
//
#define EventWriteOLE_Clipboard_Flush_Stop()\
        EventEnabledOLE_Clipboard_Flush_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_Flush_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_Clipboard_IsCurrentClipboard_Start
//

#define EventEnabledOLE_Clipboard_IsCurrentClipboard_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_IsCurrentClipboard_Start
//
#define EventWriteOLE_Clipboard_IsCurrentClipboard_Start()\
        EventEnabledOLE_Clipboard_IsCurrentClipboard_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_IsCurrentClipboard_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_Clipboard_IsCurrentClipboard_Stop
//

#define EventEnabledOLE_Clipboard_IsCurrentClipboard_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_Clipboard_IsCurrentClipboard_Stop
//
#define EventWriteOLE_Clipboard_IsCurrentClipboard_Stop()\
        EventEnabledOLE_Clipboard_IsCurrentClipboard_Stop() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_Clipboard_IsCurrentClipboard_Stop)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DoDragDrop_Start
//

#define EventEnabledOLE_DragDrop_DoDragDrop_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DoDragDrop_Start
//
#define EventWriteOLE_DragDrop_DoDragDrop_Start()\
        EventEnabledOLE_DragDrop_DoDragDrop_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DoDragDrop_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DoDragDrop_Stop
//

#define EventEnabledOLE_DragDrop_DoDragDrop_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DoDragDrop_Stop
//
#define EventWriteOLE_DragDrop_DoDragDrop_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_DoDragDrop_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DoDragDrop_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_Register_Start
//

#define EventEnabledOLE_DragDrop_Register_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_Register_Start
//
#define EventWriteOLE_DragDrop_Register_Start()\
        EventEnabledOLE_DragDrop_Register_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_Register_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_Register_Stop
//

#define EventEnabledOLE_DragDrop_Register_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_Register_Stop
//
#define EventWriteOLE_DragDrop_Register_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_Register_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_Register_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_Unregister_Start
//

#define EventEnabledOLE_DragDrop_Unregister_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_Unregister_Start
//
#define EventWriteOLE_DragDrop_Unregister_Start()\
        EventEnabledOLE_DragDrop_Unregister_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_Unregister_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_Unregister_Stop
//

#define EventEnabledOLE_DragDrop_Unregister_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_Unregister_Stop
//
#define EventWriteOLE_DragDrop_Unregister_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_Unregister_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_Unregister_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_RegisterInternal_Start
//

#define EventEnabledOLE_DragDrop_RegisterInternal_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_RegisterInternal_Start
//
#define EventWriteOLE_DragDrop_RegisterInternal_Start()\
        EventEnabledOLE_DragDrop_RegisterInternal_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_RegisterInternal_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_RegisterInternal_Stop
//

#define EventEnabledOLE_DragDrop_RegisterInternal_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_RegisterInternal_Stop
//
#define EventWriteOLE_DragDrop_RegisterInternal_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_RegisterInternal_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_RegisterInternal_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_UnregisterInternal_Start
//

#define EventEnabledOLE_DragDrop_UnregisterInternal_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_UnregisterInternal_Start
//
#define EventWriteOLE_DragDrop_UnregisterInternal_Start()\
        EventEnabledOLE_DragDrop_UnregisterInternal_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_UnregisterInternal_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_UnregisterInternal_Stop
//

#define EventEnabledOLE_DragDrop_UnregisterInternal_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_UnregisterInternal_Stop
//
#define EventWriteOLE_DragDrop_UnregisterInternal_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_UnregisterInternal_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_UnregisterInternal_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_CreateDragOperationInternal_Start
//

#define EventEnabledOLE_DragDrop_CreateDragOperationInternal_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_CreateDragOperationInternal_Start
//
#define EventWriteOLE_DragDrop_CreateDragOperationInternal_Start()\
        EventEnabledOLE_DragDrop_CreateDragOperationInternal_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_CreateDragOperationInternal_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_CreateDragOperationInternal_Stop
//

#define EventEnabledOLE_DragDrop_CreateDragOperationInternal_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_CreateDragOperationInternal_Stop
//
#define EventWriteOLE_DragDrop_CreateDragOperationInternal_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_CreateDragOperationInternal_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_CreateDragOperationInternal_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_UpdatePosition_Start
//

#define EventEnabledOLE_DragDrop_DragOperation_UpdatePosition_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_UpdatePosition_Start
//
#define EventWriteOLE_DragDrop_DragOperation_UpdatePosition_Start()\
        EventEnabledOLE_DragDrop_DragOperation_UpdatePosition_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_UpdatePosition_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_UpdatePosition_Stop
//

#define EventEnabledOLE_DragDrop_DragOperation_UpdatePosition_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_UpdatePosition_Stop
//
#define EventWriteOLE_DragDrop_DragOperation_UpdatePosition_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_DragOperation_UpdatePosition_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_UpdatePosition_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_SetDataObject_Start
//

#define EventEnabledOLE_DragDrop_DragOperation_SetDataObject_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_SetDataObject_Start
//
#define EventWriteOLE_DragDrop_DragOperation_SetDataObject_Start()\
        EventEnabledOLE_DragDrop_DragOperation_SetDataObject_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_SetDataObject_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_SetDataObject_Stop
//

#define EventEnabledOLE_DragDrop_DragOperation_SetDataObject_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_SetDataObject_Stop
//
#define EventWriteOLE_DragDrop_DragOperation_SetDataObject_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_DragOperation_SetDataObject_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_SetDataObject_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_DoDrop_Start
//

#define EventEnabledOLE_DragDrop_DragOperation_DoDrop_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_DoDrop_Start
//
#define EventWriteOLE_DragDrop_DragOperation_DoDrop_Start()\
        EventEnabledOLE_DragDrop_DragOperation_DoDrop_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_DoDrop_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_DoDrop_Stop
//

#define EventEnabledOLE_DragDrop_DragOperation_DoDrop_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_DoDrop_Stop
//
#define EventWriteOLE_DragDrop_DragOperation_DoDrop_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_DragOperation_DoDrop_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_DoDrop_Stop, HRESULT)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_Cancel_Start
//

#define EventEnabledOLE_DragDrop_DragOperation_Cancel_Start() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_Cancel_Start
//
#define EventWriteOLE_DragDrop_DragOperation_Cancel_Start()\
        EventEnabledOLE_DragDrop_DragOperation_Cancel_Start() ?\
        TemplateEventDescriptor(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_Cancel_Start)\
        : ERROR_SUCCESS\

//
// Enablement check macro for OLE_DragDrop_DragOperation_Cancel_Stop
//

#define EventEnabledOLE_DragDrop_DragOperation_Cancel_Stop() ((Microsoft_Windows_OLE_PerfEnableBits[0] & 0x00000001) != 0)

//
// Event Macro for OLE_DragDrop_DragOperation_Cancel_Stop
//
#define EventWriteOLE_DragDrop_DragOperation_Cancel_Stop(HRESULT)\
        EventEnabledOLE_DragDrop_DragOperation_Cancel_Stop() ?\
        Template_d(Microsoft_Windows_OLE_PerfHandle, &OLE_DragDrop_DragOperation_Cancel_Stop, HRESULT)\
        : ERROR_SUCCESS\

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION


//
// Allow Diasabling of code generation
//
#ifndef MCGEN_DISABLE_PROVIDER_CODE_GENERATION

//
// Template Functions 
//
//
//Template from manifest : (null)
//
#ifndef TemplateEventDescriptor_def
#define TemplateEventDescriptor_def


ETW_INLINE
ULONG
TemplateEventDescriptor(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor
    )
{
    return EventWrite(RegHandle, Descriptor, 0, NULL);
}
#endif

//
//Template from manifest : tidRoGetExtensionRegistrationStart
//
#ifndef Template_zzz_def
#define Template_zzz_def
ETW_INLINE
ULONG
Template_zzz(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ PCWSTR  ContractId,
    _In_opt_ PCWSTR  PackageId,
    _In_opt_ PCWSTR  ActivatableClassId
    )
{
#define ARGUMENT_COUNT_zzz 3

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_zzz];

    EventDataDescCreate(&EventData[0], 
                        (ContractId != NULL) ? ContractId : L"NULL",
                        (ContractId != NULL) ? (ULONG)((wcslen(ContractId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[1], 
                        (PackageId != NULL) ? PackageId : L"NULL",
                        (PackageId != NULL) ? (ULONG)((wcslen(PackageId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[2], 
                        (ActivatableClassId != NULL) ? ActivatableClassId : L"NULL",
                        (ActivatableClassId != NULL) ? (ULONG)((wcslen(ActivatableClassId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_zzz, EventData);
}
#endif

//
//Template from manifest : tidRoGetExtensionRegistrationEnd
//
#ifndef Template_dzzzt_def
#define Template_dzzzt_def
ETW_INLINE
ULONG
Template_dzzzt(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const signed int  HRESULT,
    _In_opt_ PCWSTR  ContractId,
    _In_opt_ PCWSTR  PackageId,
    _In_opt_ PCWSTR  ActivatableClassId,
    _In_ const BOOL  InboxAppsRegistrationScope
    )
{
#define ARGUMENT_COUNT_dzzzt 5

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_dzzzt];

    EventDataDescCreate(&EventData[0], &HRESULT, sizeof(const signed int)  );

    EventDataDescCreate(&EventData[1], 
                        (ContractId != NULL) ? ContractId : L"NULL",
                        (ContractId != NULL) ? (ULONG)((wcslen(ContractId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[2], 
                        (PackageId != NULL) ? PackageId : L"NULL",
                        (PackageId != NULL) ? (ULONG)((wcslen(PackageId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[3], 
                        (ActivatableClassId != NULL) ? ActivatableClassId : L"NULL",
                        (ActivatableClassId != NULL) ? (ULONG)((wcslen(ActivatableClassId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[4], &InboxAppsRegistrationScope, sizeof(const BOOL)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_dzzzt, EventData);
}
#endif

//
//Template from manifest : tidRoGetExtensionRegistrationByExtensionIdStart
//
#ifndef Template_zz_def
#define Template_zz_def
ETW_INLINE
ULONG
Template_zz(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ PCWSTR  ContractId,
    _In_opt_ PCWSTR  ExtensionId
    )
{
#define ARGUMENT_COUNT_zz 2

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_zz];

    EventDataDescCreate(&EventData[0], 
                        (ContractId != NULL) ? ContractId : L"NULL",
                        (ContractId != NULL) ? (ULONG)((wcslen(ContractId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[1], 
                        (ExtensionId != NULL) ? ExtensionId : L"NULL",
                        (ExtensionId != NULL) ? (ULONG)((wcslen(ExtensionId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_zz, EventData);
}
#endif

//
//Template from manifest : tidRoGetExtensionRegistrationByExtensionIdEnd
//
#ifndef Template_dzzt_def
#define Template_dzzt_def
ETW_INLINE
ULONG
Template_dzzt(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const signed int  HRESULT,
    _In_opt_ PCWSTR  ContractId,
    _In_opt_ PCWSTR  ExtensionId,
    _In_ const BOOL  InboxAppsRegistrationScope
    )
{
#define ARGUMENT_COUNT_dzzt 4

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_dzzt];

    EventDataDescCreate(&EventData[0], &HRESULT, sizeof(const signed int)  );

    EventDataDescCreate(&EventData[1], 
                        (ContractId != NULL) ? ContractId : L"NULL",
                        (ContractId != NULL) ? (ULONG)((wcslen(ContractId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[2], 
                        (ExtensionId != NULL) ? ExtensionId : L"NULL",
                        (ExtensionId != NULL) ? (ULONG)((wcslen(ExtensionId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[3], &InboxAppsRegistrationScope, sizeof(const BOOL)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_dzzt, EventData);
}
#endif

//
//Template from manifest : tidExtensionCatalogQueryCatalogStart
//
#ifndef Template_z_def
#define Template_z_def
ETW_INLINE
ULONG
Template_z(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_opt_ PCWSTR  ContractId
    )
{
#define ARGUMENT_COUNT_z 1

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_z];

    EventDataDescCreate(&EventData[0], 
                        (ContractId != NULL) ? ContractId : L"NULL",
                        (ContractId != NULL) ? (ULONG)((wcslen(ContractId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_z, EventData);
}
#endif

//
//Template from manifest : tidExtensionCatalogQueryCatalogEnd
//
#ifndef Template_dz_def
#define Template_dz_def
ETW_INLINE
ULONG
Template_dz(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const signed int  HRESULT,
    _In_opt_ PCWSTR  ContractId
    )
{
#define ARGUMENT_COUNT_dz 2

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_dz];

    EventDataDescCreate(&EventData[0], &HRESULT, sizeof(const signed int)  );

    EventDataDescCreate(&EventData[1], 
                        (ContractId != NULL) ? ContractId : L"NULL",
                        (ContractId != NULL) ? (ULONG)((wcslen(ContractId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_dz, EventData);
}
#endif

//
//Template from manifest : tidExtensionCatalogQueryCatalogByPackageFamilyEnd
//
#ifndef Template_dzz_def
#define Template_dzz_def
ETW_INLINE
ULONG
Template_dzz(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const signed int  HRESULT,
    _In_opt_ PCWSTR  ContractId,
    _In_opt_ PCWSTR  PackageFamilyName
    )
{
#define ARGUMENT_COUNT_dzz 3

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_dzz];

    EventDataDescCreate(&EventData[0], &HRESULT, sizeof(const signed int)  );

    EventDataDescCreate(&EventData[1], 
                        (ContractId != NULL) ? ContractId : L"NULL",
                        (ContractId != NULL) ? (ULONG)((wcslen(ContractId) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[2], 
                        (PackageFamilyName != NULL) ? PackageFamilyName : L"NULL",
                        (PackageFamilyName != NULL) ? (ULONG)((wcslen(PackageFamilyName) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_dzz, EventData);
}
#endif

//
//Template from manifest : tidClientSyncCallStart
//
#ifndef Template_qqqjqj_def
#define Template_qqqjqj_def
ETW_INLINE
ULONG
Template_qqqjqj(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  CallTraceId,
    _In_ const unsigned int  TargetProcessId,
    _In_ const unsigned int  TargetThreadId,
    _In_ LPCGUID  CausalityId,
    _In_ const unsigned int  TargetMethod,
    _In_ LPCGUID  TargetInterface
    )
{
#define ARGUMENT_COUNT_qqqjqj 6

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_qqqjqj];

    EventDataDescCreate(&EventData[0], &CallTraceId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[1], &TargetProcessId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[2], &TargetThreadId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[3], CausalityId, sizeof(GUID)  );

    EventDataDescCreate(&EventData[4], &TargetMethod, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[5], TargetInterface, sizeof(GUID)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_qqqjqj, EventData);
}
#endif

//
//Template from manifest : tidClientCallEnd
//
#ifndef Template_qdq_def
#define Template_qdq_def
ETW_INLINE
ULONG
Template_qdq(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  CallTraceId,
    _In_ const signed int  HRESULT,
    _In_ const unsigned int  SourceOfHRESULT
    )
{
#define ARGUMENT_COUNT_qdq 3

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_qdq];

    EventDataDescCreate(&EventData[0], &CallTraceId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[1], &HRESULT, sizeof(const signed int)  );

    EventDataDescCreate(&EventData[2], &SourceOfHRESULT, sizeof(const unsigned int)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_qdq, EventData);
}
#endif

//
//Template from manifest : tidClientAsyncCallStart
//
#ifndef Template_qqqjqjj_def
#define Template_qqqjqjj_def
ETW_INLINE
ULONG
Template_qqqjqjj(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  CallTraceId,
    _In_ const unsigned int  TargetProcessId,
    _In_ const unsigned int  TargetThreadId,
    _In_ LPCGUID  CausalityId,
    _In_ const unsigned int  TargetMethod,
    _In_ LPCGUID  TargetInterface,
    _In_ LPCGUID  TargetInterfaceAsyncUuid
    )
{
#define ARGUMENT_COUNT_qqqjqjj 7

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_qqqjqjj];

    EventDataDescCreate(&EventData[0], &CallTraceId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[1], &TargetProcessId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[2], &TargetThreadId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[3], CausalityId, sizeof(GUID)  );

    EventDataDescCreate(&EventData[4], &TargetMethod, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[5], TargetInterface, sizeof(GUID)  );

    EventDataDescCreate(&EventData[6], TargetInterfaceAsyncUuid, sizeof(GUID)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_qqqjqjj, EventData);
}
#endif

//
//Template from manifest : tidServerSyncCallStart
//
#ifndef Template_qqqjqjpp_def
#define Template_qqqjqjpp_def
ETW_INLINE
ULONG
Template_qqqjqjpp(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  CallTraceId,
    _In_ const unsigned int  SourceProcessId,
    _In_ const unsigned int  SourceThreadId,
    _In_ LPCGUID  CausalityId,
    _In_ const unsigned int  TargetMethod,
    _In_ LPCGUID  TargetInterface,
    _In_opt_ const void *  InterfacePointer,
    _In_opt_ const void *  MethodAddress
    )
{
#define ARGUMENT_COUNT_qqqjqjpp 8

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_qqqjqjpp];

    EventDataDescCreate(&EventData[0], &CallTraceId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[1], &SourceProcessId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[2], &SourceThreadId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[3], CausalityId, sizeof(GUID)  );

    EventDataDescCreate(&EventData[4], &TargetMethod, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[5], TargetInterface, sizeof(GUID)  );

    EventDataDescCreate(&EventData[6], &InterfacePointer, sizeof(PVOID)  );

    EventDataDescCreate(&EventData[7], &MethodAddress, sizeof(PVOID)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_qqqjqjpp, EventData);
}
#endif

//
//Template from manifest : CliModalLoopDelayInfo
//
#ifndef Template_qqq_def
#define Template_qqq_def
ETW_INLINE
ULONG
Template_qqq(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  Flags,
    _In_ const unsigned int  BlockTimeMs,
    _In_ const unsigned int  TotalTimeMs
    )
{
#define ARGUMENT_COUNT_qqq 3

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_qqq];

    EventDataDescCreate(&EventData[0], &Flags, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[1], &BlockTimeMs, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[2], &TotalTimeMs, sizeof(const unsigned int)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_qqq, EventData);
}
#endif

//
//Template from manifest : ComCallDelayInfo
//
#ifndef Template_qqqqqqj_def
#define Template_qqqqqqj_def
ETW_INLINE
ULONG
Template_qqqqqqj(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  Flags,
    _In_ const unsigned int  CallTimeMs,
    _In_ const unsigned int  CallResult,
    _In_ const unsigned int  TargetThreadId,
    _In_ const unsigned int  TargetProcessId,
    _In_ const unsigned int  TargetMethod,
    _In_ LPCGUID  TargetInterface
    )
{
#define ARGUMENT_COUNT_qqqqqqj 7

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_qqqqqqj];

    EventDataDescCreate(&EventData[0], &Flags, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[1], &CallTimeMs, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[2], &CallResult, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[3], &TargetThreadId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[4], &TargetProcessId, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[5], &TargetMethod, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[6], TargetInterface, sizeof(GUID)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_qqqqqqj, EventData);
}
#endif

//
//Template from manifest : DropNonPointerInputMessage
//
#ifndef Template_q_def
#define Template_q_def
ETW_INLINE
ULONG
Template_q(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  ApartmentKind
    )
{
#define ARGUMENT_COUNT_q 1

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_q];

    EventDataDescCreate(&EventData[0], &ApartmentKind, sizeof(const unsigned int)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_q, EventData);
}
#endif

//
//Template from manifest : DropPointerMessage
//
#ifndef Template_qq_def
#define Template_qq_def
ETW_INLINE
ULONG
Template_qq(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const unsigned int  ApartmentKind,
    _In_ const unsigned int  Message
    )
{
#define ARGUMENT_COUNT_qq 2

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_qq];

    EventDataDescCreate(&EventData[0], &ApartmentKind, sizeof(const unsigned int)  );

    EventDataDescCreate(&EventData[1], &Message, sizeof(const unsigned int)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_qq, EventData);
}
#endif

//
//Template from manifest : CreateAsynchronousOperationTemplate
//
#ifndef Template_jxzx_def
#define Template_jxzx_def
ETW_INLINE
ULONG
Template_jxzx(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ LPCGUID  PlatformId,
    _In_ unsigned __int64  OperationId,
    _In_opt_ PCWSTR  OperationName,
    _In_ unsigned __int64  RelatedId
    )
{
#define ARGUMENT_COUNT_jxzx 4

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_jxzx];

    EventDataDescCreate(&EventData[0], PlatformId, sizeof(GUID)  );

    EventDataDescCreate(&EventData[1], &OperationId, sizeof(unsigned __int64)  );

    EventDataDescCreate(&EventData[2], 
                        (OperationName != NULL) ? OperationName : L"NULL",
                        (OperationName != NULL) ? (ULONG)((wcslen(OperationName) + 1) * sizeof(WCHAR)) : (ULONG)sizeof(L"NULL"));

    EventDataDescCreate(&EventData[3], &RelatedId, sizeof(unsigned __int64)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_jxzx, EventData);
}
#endif

//
//Template from manifest : CompleteAsynchronousOperationTemplate
//
#ifndef Template_jxc_def
#define Template_jxc_def
ETW_INLINE
ULONG
Template_jxc(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ LPCGUID  PlatformId,
    _In_ unsigned __int64  OperationId,
    _In_ const UCHAR  CompletionStatus
    )
{
#define ARGUMENT_COUNT_jxc 3

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_jxc];

    EventDataDescCreate(&EventData[0], PlatformId, sizeof(GUID)  );

    EventDataDescCreate(&EventData[1], &OperationId, sizeof(unsigned __int64)  );

    EventDataDescCreate(&EventData[2], &CompletionStatus, sizeof(const UCHAR)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_jxc, EventData);
}
#endif

//
//Template from manifest : tidHRESULTMethodReturn
//
#ifndef Template_d_def
#define Template_d_def
ETW_INLINE
ULONG
Template_d(
    _In_ REGHANDLE RegHandle,
    _In_ PCEVENT_DESCRIPTOR Descriptor,
    _In_ const signed int  HRESULT
    )
{
#define ARGUMENT_COUNT_d 1

    EVENT_DATA_DESCRIPTOR EventData[ARGUMENT_COUNT_d];

    EventDataDescCreate(&EventData[0], &HRESULT, sizeof(const signed int)  );

    return EventWrite(RegHandle, Descriptor, ARGUMENT_COUNT_d, EventData);
}
#endif

#endif // MCGEN_DISABLE_PROVIDER_CODE_GENERATION

#if defined(__cplusplus)
};
#endif

#define MSG_AsynchronousOperationKeyword     0x13000001L
#define MSG_SynchronousWorkItemKeyword       0x13000002L
#define MSG_RelationKeyword                  0x13000003L
#define MSG_OperationWorkKeyword             0x13000005L
#define MSG_SystemOperationKeyword           0x13000009L
#define MSG_LibraryOperationKeyword          0x1300000AL
#define MSG_ApplicationOperationKeyword      0x1300000BL
#define MSG_OperationCreateKeyword           0x1300000CL
#define MSG_COMPerformanceEventProvider      0x90000001L
#define MSG_eventProviderNameDcom            0x91000001L
#define MSG_eventProviderNameCom             0x92000001L
#define MSG_AsynchronousCausalityEventProvider 0x93000001L
#define MSG_OLEPerformanceEventProvider      0x94000001L
#define MSG_CreateAsynchronousOperationEvent 0xB3011010L
#define MSG_CompleteAsynchronousOperationEvent 0xB3012010L
#define MSG_RelateAsynchronousOperationEvent 0xB3013010L
#define MSG_StartSynchronousWorkItemEvent    0xB3014010L
#define MSG_CompleteSynchronousWorkItemEvent 0xB3015010L
