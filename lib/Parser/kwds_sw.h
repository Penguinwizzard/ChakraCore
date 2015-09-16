//-------------------------------------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE.txt file in the project root for full license information.
//-------------------------------------------------------------------------------------------------------
// Generated by switch.exe on Wed Mar 30 14:58:29 2011

#if _WIN32 && _M_IX86
    __asm
        {
        mov eax,luHash
    // 0001 while
        cmp eax, 0x009FF239
        jb L0003
        je LEqual_while
    // 0002 function
        cmp eax, 0x70E47CD6
        jb L0005
        je LEqual_function
    // 0004 package
        cmp eax, 0xA9DEE87C
        jb L0009
        je LEqual_package
    // 0008 invariant
        cmp eax, 0xBEA8C42C
        jb L0011
        je LEqual_invariant
    // 0010 transient
        cmp eax, 0xE10C0168
        jb L0021
        je LEqual_transient
    // 0020 volatile
        cmp eax, 0xEF691640
        je LEqual_volatile
        jmp LDefault
L0021:
    // 0021 abstract
        cmp eax, 0xDBC60B24
        je LEqual_abstract
        jmp LDefault
L0011:
    // 0011 internal
        cmp eax, 0xB0B68A3D
        jb L0023
        je LEqual_internal
    // 0022 interface
        cmp eax, 0xBC1E9451
        je LEqual_interface
        jmp LDefault
L0023:
    // 0023 require
        cmp eax, 0xAD28D1BD
        je LEqual_require
    // 0047 private
        cmp eax, 0xAB57B15B
        je LEqual_private
        jmp LDefault
L0009:
    // 0009 boolean
        cmp eax, 0x96F94400
        jb L0013
        je LEqual_boolean
    // 0012 extends
        cmp eax, 0x9C13124B
        jb L0025
        je LEqual_extends
    // 0024 finally
        cmp eax, 0x9C3672EF
        je LEqual_finally
        jmp LDefault
L0025:
    // 0025 default
        cmp eax, 0x98F50305
        je LEqual_default
    // 004B decimal
        cmp eax, 0x98F1BFFF
        je LEqual_decimal
        jmp LDefault
L0013:
    // 0013 implements
        cmp eax, 0x856AF30E
        jb L0027
        je LEqual_implements
    // 0026 namespace
        cmp eax, 0x9309F69D
        je LEqual_namespace
        jmp LDefault
L0027:
    // 0027 instanceof
        cmp eax, 0x79C146FA
        je LEqual_instanceof
    // 004F protected
        cmp eax, 0x7679AA2A
        je LEqual_protected
        jmp LDefault
L0005:
    // 0005 return
        cmp eax, 0x0A2FCE00
        jb L000B
        je LEqual_return
    // 000A ushort
        cmp eax, 0x0A81B775
        jb L0015
        je LEqual_ushort
    // 0014 debugger
        cmp eax, 0x2806F445
        jb L0029
        je LEqual_debugger
    // 0028 synchronized
        cmp eax, 0x3189C9E0
        je LEqual_synchronized
        jmp LDefault
L0029:
    // 0029 continue
        cmp eax, 0x1EF7AD75
        je LEqual_continue
        jmp LDefault
L0015:
    // 0015 throws
        cmp eax, 0x0A5EC897
        jb L002B
        je LEqual_throws
    // 002A typeof
        cmp eax, 0x0A7440A7
        je LEqual_typeof
        jmp LDefault
L002B:
    // 002B switch
        cmp eax, 0x0A5B93A2
        je LEqual_switch
    // 0057 static
        cmp eax, 0x0A5727B8
        je LEqual_static
        jmp LDefault
L000B:
    // 000B ensure
        cmp eax, 0x09218CF2
        jb L0017
        je LEqual_ensure
    // 0016 native
        cmp eax, 0x09D3FE67
        jb L002D
        je LEqual_native
    // 002C public
        cmp eax, 0x0A17792F
        je LEqual_public
        jmp LDefault
L002D:
    // 002D import
        cmp eax, 0x0976AFAB
        je LEqual_import
    // 005B export
        cmp eax, 0x092E0B32
        je LEqual_export
        jmp LDefault
L0017:
    // 0017 delete
        cmp eax, 0x08FFD213
        jb L002F
        je LEqual_delete
    // 002E double
        cmp eax, 0x090D396B
        je LEqual_double
        jmp LDefault
L002F:
    // 002F assert
        cmp eax, 0x08D130F2
        je LEqual_assert
    // 005F yield
        cmp eax, 0x00A28D67
        je LEqual_yield
        jmp LDefault
L0003:
    // 0003 null
        cmp eax, 0x0008CABB
        jb L0007
        je LEqual_null
    // 0006 false
        cmp eax, 0x0089C56B
        jb L000D
        je LEqual_false
    // 000C short
        cmp eax, 0x009AE070
        jb L0019
        je LEqual_short
    // 0018 throw
        cmp eax, 0x009C29E4
        jb L0031
        je LEqual_throw
    // 0030 ulong
        cmp eax, 0x009DB965
        je LEqual_ulong
        jmp LDefault
L0031:
    // 0031 super
        cmp eax, 0x009BDA2F
        je LEqual_super
        jmp LDefault
L0019:
    // 0019 float
        cmp eax, 0x008A9AC6
        jb L0033
        je LEqual_float
    // 0032 sbyte
        cmp eax, 0x009A78A7
        je LEqual_sbyte
        jmp LDefault
L0033:
    // 0033 final
        cmp eax, 0x008A600A
        je LEqual_final
    // 0067 event
        cmp eax, 0x008A0A02
        je LEqual_event
        jmp LDefault
L000D:
    // 000D with
        cmp eax, 0x00096A6C
        jb L001B
        je LEqual_with
    // 001A class
        cmp eax, 0x0086B966
        jb L0035
        je LEqual_class
    // 0034 const
        cmp eax, 0x008701A7
        je LEqual_const
        jmp LDefault
L0035:
    // 0035 catch
        cmp eax, 0x0085FAA3
        je LEqual_catch
    // 006B break
        cmp eax, 0x0085E995
        je LEqual_break
    // await
        cmp eax, 0x0084FF56
        je LEqual_await
        jmp LDefault
L001B:
    // 001B uint
        cmp eax, 0x000943B0
        jb L0037
        je LEqual_uint
    // 0036 void
        cmp eax, 0x00095D42
        je LEqual_void
        jmp LDefault
L0037:
    // 0037 true
        cmp eax, 0x00093B10
        je LEqual_true
    // 006F this
        cmp eax, 0x00092F08
        je LEqual_this
        jmp LDefault
L0007:
    // 0007 try
        cmp eax, 0x00008AFF
        jb L000F
        je LEqual_try
    // 000E byte
        cmp eax, 0x0007E974
        jb L001D
        je LEqual_byte
    // 001C goto
        cmp eax, 0x00083E29
        jb L0039
        je LEqual_goto
    // 0038 long
        cmp eax, 0x00089DB0
        je LEqual_long
        jmp LDefault
L0039:
    // 0039 enum
        cmp eax, 0x000816B5
        je LEqual_enum
    // 0073 else
        cmp eax, 0x00081449
        je LEqual_else
        jmp LDefault
L001D:
    // 001D case
        cmp eax, 0x0007E17C
        jb L003B
        je LEqual_case
    // 003A char
        cmp eax, 0x0007E83E
        je LEqual_char
        jmp LDefault
L003B:
    // 003B use
        cmp eax, 0x00008C1D
        je LEqual_use
    // 0077 var
        cmp eax, 0x00008C19
        je LEqual_var
        jmp LDefault
L000F:
    // 000F get
        cmp eax, 0x00007B70
        jb L001F
        je LEqual_get
    // 001E new
        cmp eax, 0x0000835A
        jb L003D
        je LEqual_new
    // 003C set
        cmp eax, 0x000088FC
        je LEqual_set
        jmp LDefault
L003D:
    // 003D let
        cmp eax, 0x00008115
        je LEqual_let
    // 007B int
        cmp eax, 0x00007E4B
        je LEqual_int
        jmp LDefault
L001F:
    // 001F in
        cmp eax, 0x00000767
        jb L003F
        je LEqual_in
    // 003E for
        cmp eax, 0x00007AF7
        je LEqual_for
        jmp LDefault
L003F:
    // 003F if
        cmp eax, 0x0000075F
        je LEqual_if
    // 007F do
        cmp eax, 0x00000713
        je LEqual_do
        jmp LDefault
        }
#else
    // 0001 while
    if (luHash < 0x009FF239) goto L0003;
    if (luHash == 0x009FF239) goto LEqual_while;
    // 0002 function
    if (luHash < 0x70E47CD6) goto L0005;
    if (luHash == 0x70E47CD6) goto LEqual_function;
    // 0004 package
    if (luHash < 0xA9DEE87C) goto L0009;
    if (luHash == 0xA9DEE87C) goto LEqual_package;
    // 0008 invariant
    if (luHash < 0xBEA8C42C) goto L0011;
    if (luHash == 0xBEA8C42C) goto LEqual_invariant;
    // 0010 transient
    if (luHash < 0xE10C0168) goto L0021;
    if (luHash == 0xE10C0168) goto LEqual_transient;
    // 0020 volatile
    if (luHash == 0xEF691640) goto LEqual_volatile;
    goto LDefault;
L0021:
    // 0021 abstract
    if (luHash == 0xDBC60B24) goto LEqual_abstract;
    goto LDefault;
L0011:
    // 0011 internal
    if (luHash < 0xB0B68A3D) goto L0023;
    if (luHash == 0xB0B68A3D) goto LEqual_internal;
    // 0022 interface
    if (luHash == 0xBC1E9451) goto LEqual_interface;
    goto LDefault;
L0023:
    // 0023 require
    if (luHash == 0xAD28D1BD) goto LEqual_require;
    // 0047 private
    if (luHash == 0xAB57B15B) goto LEqual_private;
    goto LDefault;
L0009:
    // 0009 boolean
    if (luHash < 0x96F94400) goto L0013;
    if (luHash == 0x96F94400) goto LEqual_boolean;
    // 0012 extends
    if (luHash < 0x9C13124B) goto L0025;
    if (luHash == 0x9C13124B) goto LEqual_extends;
    // 0024 finally
    if (luHash == 0x9C3672EF) goto LEqual_finally;
    goto LDefault;
L0025:
    // 0025 default
    if (luHash == 0x98F50305) goto LEqual_default;
    // 004B decimal
    if (luHash == 0x98F1BFFF) goto LEqual_decimal;
    goto LDefault;
L0013:
    // 0013 implements
    if (luHash < 0x856AF30E) goto L0027;
    if (luHash == 0x856AF30E) goto LEqual_implements;
    // 0026 namespace
    if (luHash == 0x9309F69D) goto LEqual_namespace;
    goto LDefault;
L0027:
    // 0027 instanceof
    if (luHash == 0x79C146FA) goto LEqual_instanceof;
    // 004F protected
    if (luHash == 0x7679AA2A) goto LEqual_protected;
    goto LDefault;
L0005:
    // 0005 return
    if (luHash < 0x0A2FCE00) goto L000B;
    if (luHash == 0x0A2FCE00) goto LEqual_return;
    // 000A ushort
    if (luHash < 0x0A81B775) goto L0015;
    if (luHash == 0x0A81B775) goto LEqual_ushort;
    // 0014 debugger
    if (luHash < 0x2806F445) goto L0029;
    if (luHash == 0x2806F445) goto LEqual_debugger;
    // 0028 synchronized
    if (luHash == 0x3189C9E0) goto LEqual_synchronized;
    goto LDefault;
L0029:
    // 0029 continue
    if (luHash == 0x1EF7AD75) goto LEqual_continue;
    goto LDefault;
L0015:
    // 0015 throws
    if (luHash < 0x0A5EC897) goto L002B;
    if (luHash == 0x0A5EC897) goto LEqual_throws;
    // 002A typeof
    if (luHash == 0x0A7440A7) goto LEqual_typeof;
    goto LDefault;
L002B:
    // 002B switch
    if (luHash == 0x0A5B93A2) goto LEqual_switch;
    // 0057 static
    if (luHash == 0x0A5727B8) goto LEqual_static;
    goto LDefault;
L000B:
    // 000B ensure
    if (luHash < 0x09218CF2) goto L0017;
    if (luHash == 0x09218CF2) goto LEqual_ensure;
    // 0016 native
    if (luHash < 0x09D3FE67) goto L002D;
    if (luHash == 0x09D3FE67) goto LEqual_native;
    // 002C public
    if (luHash == 0x0A17792F) goto LEqual_public;
    goto LDefault;
L002D:
    // 002D import
    if (luHash == 0x0976AFAB) goto LEqual_import;
    // 005B export
    if (luHash == 0x092E0B32) goto LEqual_export;
    goto LDefault;
L0017:
    // 0017 delete
    if (luHash < 0x08FFD213) goto L002F;
    if (luHash == 0x08FFD213) goto LEqual_delete;
    // 002E double
    if (luHash == 0x090D396B) goto LEqual_double;
    goto LDefault;
L002F:
    // 002F assert
    if (luHash == 0x08D130F2) goto LEqual_assert;
    // 005F yield
    if (luHash == 0x00A28D67) goto LEqual_yield;
    goto LDefault;
L0003:
    // 0003 null
    if (luHash < 0x0008CABB) goto L0007;
    if (luHash == 0x0008CABB) goto LEqual_null;
    // 0006 false
    if (luHash < 0x0089C56B) goto L000D;
    if (luHash == 0x0089C56B) goto LEqual_false;
    // 000C short
    if (luHash < 0x009AE070) goto L0019;
    if (luHash == 0x009AE070) goto LEqual_short;
    // 0018 throw
    if (luHash < 0x009C29E4) goto L0031;
    if (luHash == 0x009C29E4) goto LEqual_throw;
    // 0030 ulong
    if (luHash == 0x009DB965) goto LEqual_ulong;
    goto LDefault;
L0031:
    // 0031 super
    if (luHash == 0x009BDA2F) goto LEqual_super;
    goto LDefault;
L0019:
    // 0019 float
    if (luHash < 0x008A9AC6) goto L0033;
    if (luHash == 0x008A9AC6) goto LEqual_float;
    // 0032 sbyte
    if (luHash == 0x009A78A7) goto LEqual_sbyte;
    goto LDefault;
L0033:
    // 0033 final
    if (luHash == 0x008A600A) goto LEqual_final;
    // 0067 event
    if (luHash == 0x008A0A02) goto LEqual_event;
    goto LDefault;
L000D:
    // 000D with
    if (luHash < 0x00096A6C) goto L001B;
    if (luHash == 0x00096A6C) goto LEqual_with;
    // 001A class
    if (luHash < 0x0086B966) goto L0035;
    if (luHash == 0x0086B966) goto LEqual_class;
    // 0034 const
    if (luHash == 0x008701A7) goto LEqual_const;
    goto LDefault;
L0035:
    // 0035 catch
    if (luHash == 0x0085FAA3) goto LEqual_catch;
    // 006B break
    if (luHash == 0x0085E995) goto LEqual_break;
    // await
    if (luHash == 0x0084FF56) goto LEqual_await;
    goto LDefault;
L001B:
    // 001B uint
    if (luHash < 0x000943B0) goto L0037;
    if (luHash == 0x000943B0) goto LEqual_uint;
    // 0036 void
    if (luHash == 0x00095D42) goto LEqual_void;
    goto LDefault;
L0037:
    // 0037 true
    if (luHash == 0x00093B10) goto LEqual_true;
    // 006F this
    if (luHash == 0x00092F08) goto LEqual_this;
    goto LDefault;
L0007:
    // 0007 try
    if (luHash < 0x00008AFF) goto L000F;
    if (luHash == 0x00008AFF) goto LEqual_try;
    // 000E byte
    if (luHash < 0x0007E974) goto L001D;
    if (luHash == 0x0007E974) goto LEqual_byte;
    // 001C goto
    if (luHash < 0x00083E29) goto L0039;
    if (luHash == 0x00083E29) goto LEqual_goto;
    // 0038 long
    if (luHash == 0x00089DB0) goto LEqual_long;
    goto LDefault;
L0039:
    // 0039 enum
    if (luHash == 0x000816B5) goto LEqual_enum;
    // 0073 else
    if (luHash == 0x00081449) goto LEqual_else;
    goto LDefault;
L001D:
    // 001D case
    if (luHash < 0x0007E17C) goto L003B;
    if (luHash == 0x0007E17C) goto LEqual_case;
    // 003A char
    if (luHash == 0x0007E83E) goto LEqual_char;
    goto LDefault;
L003B:
    // 003B use
    if (luHash == 0x00008C1D) goto LEqual_use;
    // 0077 var
    if (luHash == 0x00008C19) goto LEqual_var;
    goto LDefault;
L000F:
    // 000F get
    if (luHash < 0x00007B70) goto L001F;
    if (luHash == 0x00007B70) goto LEqual_get;
    // 001E new
    if (luHash < 0x0000835A) goto L003D;
    if (luHash == 0x0000835A) goto LEqual_new;
    // 003C set
    if (luHash == 0x000088FC) goto LEqual_set;
    goto LDefault;
L003D:
    // 003D let
    if (luHash == 0x00008115) goto LEqual_let;
    // 007B int
    if (luHash == 0x00007E4B) goto LEqual_int;
    goto LDefault;
L001F:
    // 001F in
    if (luHash < 0x00000767) goto L003F;
    if (luHash == 0x00000767) goto LEqual_in;
    // 003E for
    if (luHash == 0x00007AF7) goto LEqual_for;
    goto LDefault;
L003F:
    // 003F if
    if (luHash == 0x0000075F) goto LEqual_if;
    // 007F do
    if (luHash == 0x00000713) goto LEqual_do;
    goto LDefault;
#endif // _WIN32 && _M_IX86