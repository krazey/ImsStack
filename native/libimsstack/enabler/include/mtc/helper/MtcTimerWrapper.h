#ifndef MTC_TIMER_WRAPPER_H_
#define MTC_TIMER_WRAPPER_H_

#include "IMSList.h"
#include "IMSMap.h"
#include "ITimer.h"

class IMtcTimerListener;

class MtcTimerWrapper final : public ITimerListener
{
public:
    explicit MtcTimerWrapper();
    ~MtcTimerWrapper();
    MtcTimerWrapper(IN const MtcTimerWrapper&) = delete;
    MtcTimerWrapper& operator=(IN const MtcTimerWrapper&) = delete;

    // ITimerListener implementation
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    void SetListener(IN IMtcTimerListener* piListener);
    void Start(IN IMS_UINT32 eType, IN IMS_SINT32 nDuration);
    void Stop(IN IMS_UINT32 eType);
    void StopAll();
    IMS_BOOL IsActive(IN IMS_UINT32 eType);

    typedef struct MtcTimer
    {
        MtcTimer() :
                eType(0),
                nDuration(-1),
                piTimer(IMS_NULL),
                nId(0)
        {
        }

        IMS_UINT32 eType;
        IMS_SINT32 nDuration;
        ITimer* piTimer;
        IMS_UINTP nId;
    } MtcTimer;

private:
    IMSList<MtcTimer*> m_lstTimers;
    IMtcTimerListener* m_piListener;
};
#endif /*  MTC_TIMER_WRAPPER_H_ */