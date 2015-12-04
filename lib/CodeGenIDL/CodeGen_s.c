

/* this ALWAYS GENERATED file contains the RPC server stubs */


 /* File created by MIDL compiler version 8.00.0613 */
/* at Mon Jan 18 19:14:07 2038
 */
/* Compiler settings for CodeGen.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.00.0613
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data
    VC __declspec() decoration level:
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#if defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */
#if _MSC_VER >= 1200
#pragma warning(push)
#endif

#pragma warning( disable: 4211 )  /* redefine extern to static */
#pragma warning( disable: 4232 )  /* dllimport identity*/
#pragma warning( disable: 4024 )  /* array to pointer mapping*/
#pragma warning( disable: 4100 ) /* unreferenced arguments in x86 call */

#pragma optimize("", off )

#include <string.h>
#include "CodeGen.h"

#define TYPE_FORMAT_STRING_SIZE   3
#define PROC_FORMAT_STRING_SIZE   67
#define EXPR_FORMAT_STRING_SIZE   1
#define TRANSMIT_AS_TABLE_SIZE    0
#define WIRE_MARSHAL_TABLE_SIZE   0

typedef struct _CodeGen_MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } CodeGen_MIDL_TYPE_FORMAT_STRING;

typedef struct _CodeGen_MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } CodeGen_MIDL_PROC_FORMAT_STRING;

typedef struct _CodeGen_MIDL_EXPR_FORMAT_STRING
    {
    long          Pad;
    unsigned char  Format[ EXPR_FORMAT_STRING_SIZE ];
    } CodeGen_MIDL_EXPR_FORMAT_STRING;


static const RPC_SYNTAX_IDENTIFIER  _RpcTransferSyntax =
{{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}};

extern const CodeGen_MIDL_TYPE_FORMAT_STRING CodeGen__MIDL_TypeFormatString;
extern const CodeGen_MIDL_PROC_FORMAT_STRING CodeGen__MIDL_ProcFormatString;
extern const CodeGen_MIDL_EXPR_FORMAT_STRING CodeGen__MIDL_ExprFormatString;

/* Standard interface: CodeGen, ver. 1.0,
   GUID={0xead694ed,0x2243,0x44cb,{0xa9,0xdc,0x85,0xd3,0xba,0x93,0x4d,0xab}} */


extern const MIDL_SERVER_INFO CodeGen_ServerInfo;

extern const RPC_DISPATCH_TABLE CodeGen_v1_0_DispatchTable;

static const RPC_SERVER_INTERFACE CodeGen___RpcServerInterface =
    {
    sizeof(RPC_SERVER_INTERFACE),
    {{0xead694ed,0x2243,0x44cb,{0xa9,0xdc,0x85,0xd3,0xba,0x93,0x4d,0xab}},{1,0}},
    {{0x8A885D04,0x1CEB,0x11C9,{0x9F,0xE8,0x08,0x00,0x2B,0x10,0x48,0x60}},{2,0}},
    (RPC_DISPATCH_TABLE*)&CodeGen_v1_0_DispatchTable,
    0,
    0,
    0,
    &CodeGen_ServerInfo,
    0x04000000
    };
RPC_IF_HANDLE CodeGen_v1_0_s_ifspec = (RPC_IF_HANDLE)& CodeGen___RpcServerInterface;

extern const MIDL_STUB_DESC CodeGen_StubDesc;


#if !defined(__RPC_WIN64__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT50_OR_LATER)
#error You need Windows 2000 or later to run this stub because it uses these features:
#error   /robust command line switch.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will fail with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const CodeGen_MIDL_PROC_FORMAT_STRING CodeGen__MIDL_ProcFormatString =
    {
        0,
        {

    /* Procedure Shutdown */

            0x0,        /* 0 */
            0x48,       /* Old Flags:  */
/*  2 */    NdrFcLong( 0x0 ),   /* 0 */
/*  6 */    NdrFcShort( 0x0 ),  /* 0 */
/*  8 */    NdrFcShort( 0x8 ),  /* x86 Stack size/offset = 8 */
/* 10 */    0x32,       /* FC_BIND_PRIMITIVE */
            0x0,        /* 0 */
/* 12 */    NdrFcShort( 0x0 ),  /* x86 Stack size/offset = 0 */
/* 14 */    NdrFcShort( 0x0 ),  /* 0 */
/* 16 */    NdrFcShort( 0x0 ),  /* 0 */
/* 18 */    0x40,       /* Oi2 Flags:  has ext, */
            0x0,        /* 0 */
/* 20 */    0xa,        /* 10 */
            0x1,        /* Ext Flags:  new corr desc, */
/* 22 */    NdrFcShort( 0x0 ),  /* 0 */
/* 24 */    NdrFcShort( 0x0 ),  /* 0 */
/* 26 */    NdrFcShort( 0x0 ),  /* 0 */
/* 28 */    NdrFcShort( 0x0 ),  /* 0 */

    /* Procedure HelloServer */

/* 30 */    0x0,        /* 0 */
            0x48,       /* Old Flags:  */
/* 32 */    NdrFcLong( 0x0 ),   /* 0 */
/* 36 */    NdrFcShort( 0x1 ),  /* 1 */
/* 38 */    NdrFcShort( 0x10 ), /* x86 Stack size/offset = 16 */
/* 40 */    0x32,       /* FC_BIND_PRIMITIVE */
            0x0,        /* 0 */
/* 42 */    NdrFcShort( 0x0 ),  /* x86 Stack size/offset = 0 */
/* 44 */    NdrFcShort( 0x0 ),  /* 0 */
/* 46 */    NdrFcShort( 0x8 ),  /* 8 */
/* 48 */    0x44,       /* Oi2 Flags:  has return, has ext, */
            0x1,        /* 1 */
/* 50 */    0xa,        /* 10 */
            0x1,        /* Ext Flags:  new corr desc, */
/* 52 */    NdrFcShort( 0x0 ),  /* 0 */
/* 54 */    NdrFcShort( 0x0 ),  /* 0 */
/* 56 */    NdrFcShort( 0x0 ),  /* 0 */
/* 58 */    NdrFcShort( 0x0 ),  /* 0 */

    /* Return value */

/* 60 */    NdrFcShort( 0x70 ), /* Flags:  out, return, base type, */
/* 62 */    NdrFcShort( 0x8 ),  /* x86 Stack size/offset = 8 */
/* 64 */    0x8,        /* FC_LONG */
            0x0,        /* 0 */

            0x0
        }
    };

static const CodeGen_MIDL_TYPE_FORMAT_STRING CodeGen__MIDL_TypeFormatString =
    {
        0,
        {
            NdrFcShort( 0x0 ),  /* 0 */

            0x0
        }
    };

static const unsigned short CodeGen_FormatStringOffsetTable[] =
    {
    0,
    30
    };


static const MIDL_STUB_DESC CodeGen_StubDesc =
    {
    (void *)& CodeGen___RpcServerInterface,
    MIDL_user_allocate,
    MIDL_user_free,
    0,
    0,
    0,
    0,
    0,
    CodeGen__MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x50002, /* Ndr library version */
    0,
    0x8000265, /* MIDL Version 8.0.613 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    0x1, /* MIDL flag */
    0, /* cs routines */
    0,   /* proxy/server info */
    0
    };

static const RPC_DISPATCH_FUNCTION CodeGen_table[] =
    {
    NdrServerCall2,
    NdrServerCall2,
    0
    };
static const RPC_DISPATCH_TABLE CodeGen_v1_0_DispatchTable =
    {
    2,
    (RPC_DISPATCH_FUNCTION*)CodeGen_table
    };

static const SERVER_ROUTINE CodeGen_ServerRoutineTable[] =
    {
    (SERVER_ROUTINE)Shutdown,
    (SERVER_ROUTINE)HelloServer
    };

static const MIDL_SERVER_INFO CodeGen_ServerInfo =
    {
    &CodeGen_StubDesc,
    CodeGen_ServerRoutineTable,
    CodeGen__MIDL_ProcFormatString.Format,
    CodeGen_FormatStringOffsetTable,
    0,
    0,
    0,
    0};
#if _MSC_VER >= 1200
#pragma warning(pop)
#endif


#endif /* defined(_M_AMD64)*/

