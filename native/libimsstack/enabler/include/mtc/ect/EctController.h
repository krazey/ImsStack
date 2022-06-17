#ifndef ECT_CONTROLLER_H_
#define ECT_CONTROLLER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCallManager.h"
#include "ect/IEctReferenceListener.h"
#include "CallReasonInfo.h"
#include "ITimer.h"

class IMtcContext;
class IMtcCall;
class IEctControllerListener;
class EctReference;

class EctController : public IEctReferenceListener, public ITimerListener
{
public:
    explicit EctController(IN IMtcContext& objContext, IN CallKey nCallKey,
            IN IEctControllerListener& objListener);
    virtual ~EctController();
    EctController(IN const EctController&) = delete;
    EctController& operator=(IN const EctController&) = delete;

    // IEctReferenceListener implementation
    void OnReferenceStarted() override;
    void OnReferenceStartFailed() override;
    void OnReferenceUpdated(IN SipStatusCode nSipFragCode) override;

    // ITimerListener implementation
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    inline virtual void Transfer(IN const AString& strNumber) { (void)strNumber; }
    inline virtual void Transfer() {}

protected:
    static const IMS_UINT32 TIME_WAIT_OPERATION_COMPLETE = 3000;

    IMtcCall* GetTransferee() const;
    inline virtual IMS_BOOL IsValid() const { return IMS_FALSE; }
    virtual void OnCompleted();  // TODO: OnSucceeded?
    virtual void OnFailed();

    void NotifyResult(IN IMS_RESULT nResult, IN IMS_SINT32 nReason = CODE_NONE) const;
    void CreateReference();
    void TerminateTransfereeCall();

    void StartTimer();
    void StopTimer();

    IMtcContext& m_objContext;
    CallKey m_nTransfereeKey;
    IEctControllerListener& m_objListener;
    std::unique_ptr<EctReference> m_pReference;
    ITimer* m_piTimer;
};

#endif
