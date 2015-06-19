//---------------------------------------------------------------------------
//
// File: arm.h
//
// Copyright (C) by Microsoft Corporation.  All rights reserved.
//
//----------------------------------------------------------------------------

// ARM-specific macro definitions

#pragma once

#ifndef _M_ARM
#error Include arm.h in builds of ARM targets only.
#endif

extern "C" LPVOID arm_GET_CURRENT_FRAME(void);
extern "C" VOID arm_SAVE_REGISTERS(void*);

/*
 * The relevant part of the frame looks like this (high addresses at the top, low ones at the bottom):
 *
 * ----------------------
 * r3     <=== Homed input parameters
 * r2     <
 * r1     <
 * r0     <===
 * lr     <=== return address
 * r11    <=== current r11 (frame pointer)
 * ...
 */

const DWORD ReturnAddrOffsetFromFramePtr = 1;
const DWORD ArgOffsetFromFramePtr = 2;
