/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090305  lovil@                    Created
    20090831  yhrhee@                   Modified
    20131212  yongnam.cha@              Modify CreateTimer()
    </table>

    Description
    IMS Timer Service
*/

#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ImsTimer.h"
#include "PlatformFactory.h"
#include "ServiceTimer.h"

PRIVATE
TimerService::TimerService()
{
    piMutex = MutexService::GetMutexService()->CreateMutex();
}

PRIVATE
TimerService::~TimerService()
{
    MutexService::GetMutexService()->DestroyMutex(piMutex);
}

PUBLIC
ITimer* TimerService::CreateTimer()
{
    ImsTimer *pTimer = PlatformFactory::CreateTimer();

    IMS_ASSERT(pTimer != IMS_NULL);

    piMutex->Lock();
    objTimers.Append(pTimer);
    piMutex->Unlock();

    return pTimer;
}

PUBLIC
ITimer* TimerService::CreateTimer(IN IMS_BOOL /* bAlarmTimer = IMS_FALSE */)
{
    return CreateTimer();
}

PUBLIC
void TimerService::DestroyTimer(IN ITimer *&piTimer, IN IMS_BOOL bOnOwnerThread/* = IMS_TRUE*/)
{
    ImsTimer *pTimer = DYNAMIC_CAST(ImsTimer*, piTimer);

    if (pTimer == IMS_NULL)
    {
        return;
    }

    piMutex->Lock();

    for (IMS_UINT32 i = 0; i < objTimers.GetSize(); ++i)
    {
        ITimer *piExTimer = objTimers.GetAt(i);

        if (piExTimer == IMS_NULL)
        {
            continue;
        }

        if (piExTimer->Equals(piTimer))
        {
            objTimers.RemoveAt(i);
            break;
        }
    }

    piMutex->Unlock();

    if (bOnOwnerThread)
    {
        pTimer->Destroy();
    }
    else
    {
        delete pTimer;
    }

    piTimer = IMS_NULL;
}

PUBLIC
void TimerService::DispatchServiceMessage(IN IMSMSG &objMSG)
{
    // FIX_TIMING_ISSUE: same timer id issue
    // If the internal timer id is MSG_PARAM_DESTROY,
    // then it indicates that the timer is killed and needs to be destroyed.
    if (objMSG.nLparam == ImsTimer::MSG_PARAM_DESTROY)
    {
        ImsTimer *pTimer = reinterpret_cast<ImsTimer*>(objMSG.nWparam);

        if (pTimer != IMS_NULL)
        {
            delete pTimer;
        }
        return;
    }

    // Check if the expired timer exists in the timer aggregate.
    piMutex->Lock();

    for (IMS_UINT32 i = 0; i < objTimers.GetSize(); ++i)
    {
        ITimer *pTimer = objTimers.GetAt(i);

        if (pTimer != IMS_NULL)
        {
            ImsTimer *pIMSTimer = DYNAMIC_CAST(ImsTimer*, pTimer);
            // Compare Timer ID
            if (pIMSTimer->GetTimerId() == objMSG.nWparam)
            {
                piMutex->Unlock();

                pIMSTimer->DispatchServiceMessage(objMSG.nWparam, objMSG.nLparam);
                return;
            }
        }
    }

    piMutex->Unlock();
}

// Creates the singleton class and return it
PUBLIC GLOBAL
TimerService* TimerService::GetTimerService()
{
    static TimerService *pTimerService = IMS_NULL;

    if (pTimerService == IMS_NULL)
    {
        pTimerService = new TimerService();
    }

    return pTimerService;
}
