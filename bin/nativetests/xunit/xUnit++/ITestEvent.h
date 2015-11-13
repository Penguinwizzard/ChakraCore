#ifndef ITESTEVENT_H_
#define ITESTEVENT_H_

#include "CompilerWorkarounds.h"

namespace xUnitpp
{

enum class EventLevel;

struct ITestAssert
{
protected:
    virtual ~ITestAssert() XU_DEFAULT

public:
    virtual const char * __stdcall GetCall() const = 0;
    virtual const char * __stdcall GetUserMessage() const = 0;
    virtual const char * __stdcall GetCustomMessage() const = 0;
    virtual const char * __stdcall GetExpected() const = 0;
    virtual const char * __stdcall GetActual() const = 0;
};

struct ITestEvent
{
protected:
    virtual ~ITestEvent() XU_DEFAULT

public:
    virtual bool __stdcall GetIsAssertType() const = 0;
    virtual bool __stdcall GetIsFailure() const = 0;
    virtual EventLevel __stdcall GetLevel() const = 0;
    virtual const char * __stdcall GetMessage() const = 0;
    virtual const char * __stdcall GetToString() const = 0;
    virtual const char * __stdcall GetFile() const = 0;
    virtual int __stdcall GetLine() const = 0;

    virtual const ITestAssert & __stdcall GetAssertInterface() const = 0;
};

}

#endif
