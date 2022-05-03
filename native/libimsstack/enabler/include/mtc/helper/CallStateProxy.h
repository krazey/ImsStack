#ifndef CALL_STATE_PROXY_H_
#define CALL_STATE_PROXY_H_

#include "FailReason.h"
#include "IMSActivity.h"
#include "IMSTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "IMtcCallStateListener.h"

class IMtcCallManager;

struct CallStateDetails
{
public:
    inline CallStateDetails(IN CallKey _nCallKey, IN IMtcCallStateListener::State _eState,
            IN IMtcCallStateListener::Type _eType, IN IMS_BOOL _bEmergency,
            IN IMS_SINT32 _nReason) :
            nCallKey(_nCallKey),
            eState(_eState),
            eType(_eType),  // for Aos, java.
            bEmergency(_bEmergency),
            nReason(_nReason)  // for CallInfoService.java
    {
    }
    inline virtual ~CallStateDetails() {}

private:
    CallStateDetails(IN const CallStateDetails&) = delete;
    CallStateDetails& operator=(IN const CallStateDetails&) = delete;

public:
    CallKey nCallKey;
    IMtcCallStateListener::State eState;
    IMtcCallStateListener::Type eType;
    IMS_BOOL bEmergency;
    IMS_SINT32 nReason;  // enum class????
};

class CallStateProxy final : public IMSActivity
{
public:
    CallStateProxy(IN IMtcCallManager& objCallManager);
    ~CallStateProxy();
    CallStateProxy(IN const CallStateProxy&) = delete;
    CallStateProxy& operator=(IN const CallStateProxy&) = delete;

public:
    virtual IIMSActivityControl* GetController() override { return IMS_NULL; }
    void AddListener(IN IMtcCallStateListener* pListener);
    void RemoveListener(IN IMtcCallStateListener* pListener);

    void UpdateCallState(IN CallKey nCallkey, IN CallInfo& objCallInfo, IN IMtcCall::State eState,
            IN IMS_SINT32 nReason = FAIL_REASON_NONE);

protected:
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMsg) override;

private:
    IMS_BOOL UpdateTotalCallState();
    IMtcCall::State CalculateTotalCallState();

    void NotifyToListeners(IN IMS_BOOL bSynchronous, IN CallStateDetails* pDetails,
            IN IMS_BOOL bTotalCallStateUpdated);
    void NotifyCallState(
            IN IMSList<IMtcCallStateListener*> objListeners, IN CallStateDetails* pDetails);
    void NotifyTotalCallState(IN IMSList<IMtcCallStateListener*> objListeners);

private:
    IMSList<IMtcCallStateListener*> m_objSynchronousListeners;
    IMSList<IMtcCallStateListener*> m_objAsynchronousListeners;
    IMtcCallManager& m_objCallManager;
    IMtcCall::State m_eTotalState;
    static const IMS_UINT32 MESSAGE_ASYNC_NOTIFY = 0;
};

#endif
