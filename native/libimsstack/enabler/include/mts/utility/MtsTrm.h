#ifndef MTS_TRM_H_
#define MTS_TRM_H_

#include "ITimer.h"
#include "ITrm.h"

class MtsTrmListener
{
public:
    virtual void Trm_PriorityChanged() = 0;
};

class MtsTrm final : public ITrmListener, public ITimerListener
{
public:
    MtsTrm(IN IMS_SINT32 nSlotId_);
    ~MtsTrm();
    static MtsTrm* GetInstance(IN IMS_SINT32 nSlotId);
    static void DestroyMtsTrm(IN IMS_SINT32 nSlotId);

    void AddListener(IN MtsTrmListener* piListener);
    void RemoveListener(IN MtsTrmListener* piListener);
    IMS_BOOL IsReady();
    IMS_BOOL IsTRMSupported();
    void Set(IN IMS_BOOL bStart);

private:
    void StartTimer(IN IMS_UINT32 nDuration);
    void StopTimer();

    void ProcessTimerExpired();

    // ITrmListener
    void Trm_NotifyServicePriorityChanged();

    // ITimerListener Interface
    void Timer_TimerExpired(IN ITimer* piTimer);

private:
    IMS_SINT32 m_nSlotId;
    IMS_BOOL m_bIsTrmSet;
    ITrm* m_piTrm;
    ITimer* m_piMtsTrmTimer;
    IMSList<MtsTrmListener*> m_objTrmListeners;

    static const IMS_UINT32 MTS_TRM_TIME_STOP_DELAY = 500;
};

#endif
