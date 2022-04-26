#ifndef QOS_TIMER_H_
#define QOS_TIMER_H_

#include "IMSTypeDef.h"
#include "IMSMap.h"
#include "ServiceTimer.h"
#include "precondition/IQosTimerListener.h"
#include "precondition/QosDef.h"

class QosTimer :
        public ITimerListener
{
public:
    QosTimer(IN IQosTimerListener* pListener);
    virtual ~QosTimer();

private:
    QosTimer(IN CONST QosTimer &objRHS);
    QosTimer& operator=(IN CONST QosTimer &objRHS);

public:
    virtual void Timer_TimerExpired(IN ITimer* piExpiredTimer);

    void StartQosTimer(IN QosTimerType eType, IN IMS_SINT32 nDuration);
    void StopQosTimer(IN QosTimerType eType);

    IMS_BOOL IsQosTimerActivated(IN QosTimerType eType);

private:
    ITimer* GetTimer(IN QosTimerType eType);

protected:
    IMSMap<QosTimerType, ITimer*> m_objTimers;
    IQosTimerListener* m_pQosTimerListener;
};
#endif
