#ifndef IOUTPUT_H_
#define IOUTPUT_H_


#include <string>
#include "CompilerWorkarounds.h"
#include "xUnitTime.h"

namespace xUnitpp
{

struct ITestDetails;
struct ITestEvent;

struct IOutput
{
    virtual ~IOutput() XU_DEFAULT

    virtual void __stdcall ReportStart(const ITestDetails &testDetails) = 0;
    virtual void __stdcall ReportEvent(const ITestDetails &testDetails, const ITestEvent &evt) = 0;
    virtual void __stdcall ReportSkip(const ITestDetails &testDetails, const char *reason) = 0;
    virtual void __stdcall ReportFinish(const ITestDetails &testDetails, long long ns) = 0;
    virtual void __stdcall ReportAllTestsComplete(size_t testCount, size_t skipped, size_t failed, long long nsTotal) = 0;
};

}

#endif
