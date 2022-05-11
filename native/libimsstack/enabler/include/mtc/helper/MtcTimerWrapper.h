#ifndef MTC_TIMER_WRAPPER_H_
#define MTC_TIMER_WRAPPER_H_

#include "ServiceTimer.h"
#include "IMSList.h"
#include "ITimer.h"

class IMtcTimerListener;

class MtcTimerWrapper : public ITimerListener
{
public:
    MtcTimerWrapper();
    virtual ~MtcTimerWrapper();
    MtcTimerWrapper(IN const MtcTimerWrapper&) = delete;
    MtcTimerWrapper& operator=(IN const MtcTimerWrapper&) = delete;

    // ITimerListener implementation
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    void SetListener(IN IMtcTimerListener* piListener);
    void Start(IN IMS_UINT32 eType, IN IMS_SINT32 nDuration);
    void Stop(IN IMS_UINT32 eType);
    void StopAll();
    IMS_BOOL IsActive(IN IMS_UINT32 eType);

    struct MtcTimer
    {
        MtcTimer(IN IMS_UINT32 eType) :
                eType(eType),
                piTimer(TimerService::GetTimerService()->CreateTimer())
        {
        }
        ~MtcTimer()
        {
            if (piTimer)
            {
                piTimer->KillTimer();
                TimerService::GetTimerService()->DestroyTimer(piTimer);
            }
        }
        MtcTimer(IN const MtcTimer&) = delete;
        MtcTimer& operator=(IN const MtcTimer&) = delete;

        IMS_UINT32 eType;
        ITimer* piTimer;
    };

private:
    // TODO: IMSMap<type, timer> would be better.
    IMSList<MtcTimer*> m_lstTimers;
    IMtcTimerListener* m_piListener;
};

#endif
