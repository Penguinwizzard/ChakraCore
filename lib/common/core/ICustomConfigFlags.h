//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once


interface ICmdLineArgsParser
{
    virtual BSTR GetCurrentString() = 0;
    virtual bool GetCurrentBoolean() = 0;
    virtual int GetCurrentInt() = 0;
};


interface ICustomConfigFlags
{
    virtual void PrintUsage() = 0;
    virtual bool ParseFlag(LPCWSTR flagsString, ICmdLineArgsParser * parser) = 0;
};

