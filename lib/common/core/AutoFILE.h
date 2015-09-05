//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once
class AutoFILE : public BasePtr<FILE>
{
public:
    AutoFILE(FILE * file = nullptr) : BasePtr<FILE>(file) {};
    ~AutoFILE()
    {
        Close();
    }
    AutoFILE& operator=(FILE * file)
    {
        Close();
        this->ptr = file;
        return *this;
    }
    void Close()
    {
        if (ptr != nullptr)
        {
            fclose(ptr);
        }
    }
};
