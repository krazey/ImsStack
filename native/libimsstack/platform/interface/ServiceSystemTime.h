/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090305  lovil@                    Created
    20090831  yhrhee@                   Modified
    </table>

    Description
     This file defines the system utility functions.
*/

#ifndef _SERVICE_IMS_SYSTEM_H_
#define _SERVICE_IMS_SYSTEM_H_

#include "ISystemTime.h"

class SystemTimeServicePrivate;

class SystemTimeService
{
private:
    SystemTimeService();
    ~SystemTimeService();

    SystemTimeService(IN const SystemTimeService& objRHS);
    SystemTimeService& operator=(IN const SystemTimeService& objRHS);

public:
    ISystemTime* GetSystemTime();

    static SystemTimeService* GetSystemTimeService();

private:
    SystemTimeServicePrivate *pPrivate;
};

//-------------------------------------------------------------------------------------------------
#define IMS_SYS_GetDate() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetDate()

#define IMS_SYS_GetLocalTime() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetLocalTime()

#define IMS_SYS_GetRandom0() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom(IMS_FALSE)

#define IMS_SYS_GetRandom(R) \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom(IMS_FALSE, R)

#define IMS_SYS_GetSRandom0() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom(IMS_TRUE)

#define IMS_SYS_GetSRandom(R) \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetRandom(IMS_TRUE, R)

#define IMS_SYS_GetTickCount() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTickCount()

#define IMS_SYS_GetTimeInSeconds() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeInSeconds()

#define IMS_SYS_GetTimeInMicroSeconds() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeInMicroSeconds()

#define IMS_SYS_GetTimeInMilliSeconds() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeInMilliSeconds()

#define IMS_SYS_GetTimeString() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeString()

//LGU+ Knight
#define IMS_SYS_GetTimeStringEx() \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetTimeStringEx()

#define IMS_SYS_Sleep(MS) \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->Sleep(MS)

#define IMS_SYS_GetDiffGMTime(BEGIN,END) \
        SystemTimeService::GetSystemTimeService()->GetSystemTime()->GetDiffGmTime(BEGIN,END);

#endif // _SERVICE_IMS_SYSTEM_H_
