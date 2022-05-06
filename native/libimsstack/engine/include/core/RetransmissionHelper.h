/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090722  toastops@                 Created
    </table>

    Description

*/

#ifndef _RETRANSMISSION_HELPER_H_
#define _RETRANSMISSION_HELPER_H_

#include "ITimer.h"

class IRetransmissionHelperListener;
class Service;

class RetransmissionHelper : public ITimerListener
{
public:
    explicit RetransmissionHelper(IN Service* pService_, IN IMS_BOOL bIntervalCap = IMS_TRUE);
    virtual ~RetransmissionHelper();

private:
    RetransmissionHelper(IN CONST RetransmissionHelper& objRHS);
    RetransmissionHelper& operator=(IN CONST RetransmissionHelper& objRHS);

public:
    void SetListener(IN IRetransmissionHelperListener* piListener);
    void SetMaxDuration(IN IMS_SINT32 nValue);
    IMS_RESULT Start();
    void Stop();

protected:
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

public:
    enum
    {
        NOTIFICATION_INTERNAL_ERROR = 0,
        NOTIFICATION_RETRANSMIT = 1,
        NOTIFICATION_TIMER_EXPIRED = 2
    };

private:
    enum
    {
        TIMER_T1 = 2000
    };
    enum
    {
        TIMER_T2 = (8 * TIMER_T1)
    };
    enum
    {
        TIMER_MAX = (64 * TIMER_T1)
    };

    IMS_SINT32 nDuration;
    IMS_SINT32 nCumulativeDuration;
    IMS_SINT32 nMaxDuration;
    IMS_SINT32 nIntervalCap;

    ITimer* piTimer;
    IRetransmissionHelperListener* piListener;
};

#endif  // _RETRANSMISSION_HELPER_H_
