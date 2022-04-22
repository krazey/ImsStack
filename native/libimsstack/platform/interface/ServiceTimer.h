/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090305  lovil@                    Created
    20090831  yhrhee@                   Modified
    20131212  yongnam.cha@              Featuring for RCS BB
    </table>

    Description
     MS Timer Service
*/

#ifndef _SERVICE_IMS_TIMER_H_
#define _SERVICE_IMS_TIMER_H_

#include "IMSList.h"
#include "ImsMessage.h"
#include "ITimer.h"

class IMutex;



class TimerService
{
private:
    TimerService();
    ~TimerService();

    TimerService(IN const TimerService& objRHS);
    TimerService& operator=(IN const TimerService& objRHS);

public:
    ITimer* CreateTimer();
    ITimer* CreateTimer(IN IMS_BOOL bAlarmTimer);
    void DestroyTimer(IN ITimer *&piTimer, IN IMS_BOOL bOnOwnerThread = IMS_TRUE);

    void DispatchServiceMessage(IN IMSMSG &objMSG);

    static TimerService* GetTimerService();

private:
    IMutex *piMutex;
    IMSList<ITimer*> objTimers;
};

#endif // _SERVICE_IMS_TIMER_H_
