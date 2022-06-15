#ifndef MTC_CALL_H_
#define MTC_CALL_H_

#include "IMSList.h"
#include "IMSTypeDef.h"
#include "IMtcContext.h"
#include "ISessionListener.h"
#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "MtcDef.h"
#include "base/IMessageMediator.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/MtcSession.h"
#include "call/MtcUiNotifier.h"
#include "call/ParticipantInfo.h"
#include "call/state/MtcCallState.h"
#include "call/state/MtcCallStateMachine.h"
#include "call/UpdatingInfo.h"
#include "helper/IMtcTimerListener.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/MtcTimerWrapper.h"
#include "helper/block/IMtcBlockChecker.h"
#include "media/IMediaReportEventListener.h"
#include "media/MtcMediaManager.h"
#include "precondition/MtcPreconditionManager.h"
#include "ussi/UssiController.h"

class IMtcContext;
class IMtcMediaManager;
class IMtcPreconditionManager;
class IMtcService;
class IMutex;
class IReference;
class ISession;
class JniMediaSessionThread;
class JniMtcCallThread;
class MessageSender;
class MtcConfigurationProxy;
class MtcDialingPlan;
class MtcSipInterfaceFactory;
class MtcVonrManager;
class UssiController;
struct FailReason;

class MtcCall final :
        public IMtcCall,
        public IMtcCallContext,
        public ISessionListener,
        public IMessageMediator,
        public IMtcTimerListener,
        public IMtcBlockCheckListener,
        public IMtcPreconditionListener,
        public IMtcCallStateFactory<MtcCallState, CallStateName>,
        public IMtcCallStateWatcher<CallStateName>,
        public ISipClientConnectionListener,
        public ISipErrorListener,
        public IMediaReportEventListener
{
public:
    MtcCall(IN IMtcContext& objContext, IN IMtcService& objService, IN const CallInfo& objCallInfo);
    virtual ~MtcCall();
    MtcCall(IN const MtcCall&) = delete;
    MtcCall& operator=(IN const MtcCall&) = delete;

    void Attach(IN JniMtcCallThread* pJniMtcCallThread,
            IN JniMediaSessionThread* pJniMediaThread) override;
    void Detach() override;

    void Start(IN CallType eCallType, IN const AString& strTarget, IN MediaInfo* pMediaInfo,
            IN const IMSMap<SuppType, SuppService*>& objSuppServices) override;
    void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
            IN IMSList<ConfUser*> lstUsers) override;
    void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const IMSList<ConfUser*> objUsers) override;
    void HandleIncoming(IN ISession* piSession, IN JniMtcServiceThread* pServiceThread) override;
    void HandleUserAlert() override;
    void Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void Reject(IN const FailReason& objReason) override;
    void Hold(IN MediaInfo* pMediaInfo) override;
    void Resume(IN MediaInfo* pMediaInfo) override;
    void AcceptResume(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void RejectResume(IN const FailReason& objReason) override;
    void Convert(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void AcceptConvert(IN CallType eCallType, IN MediaInfo* pMediaInfo) override;
    void RejectConvert(IN const FailReason& objReason) override;
    void CancelConvert(IN const FailReason& objReason) override;
    void Terminate(IN const FailReason& objReason) override;
    void SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration) override;
    void SendUssd(IN const AString& strUssd) override;
    void HandleSrvccSuccess() override;
    void HandleSrvccFailure(IN UpdateType eUpdateType) override;

    inline CallKey GetKey() const override { return m_nKey; }
    inline CallStateName GetState() const override { return m_objStateMachine.GetState(); }
    // TODO: TEMP for conference.
    inline IMtcCallContext& GetCallContext() const override { return *(IMtcCallContext*)this; }

    inline IMS_UINTP GetCallKey() const override { return m_nKey; }
    inline IMS_BOOL IsHeldByMe() const override { return m_bHeldByMe; }
    inline IMS_BOOL IsUssi() const override { return m_objCallInfo.bUssi; }
    inline CallInfo& GetCallInfo() override { return m_objCallInfo; }
    inline ParticipantInfo& GetParticipantInfo() override { return m_objParticipantInfo; }
    MtcSession* GetSession(IN const ISession* piSession) override;
    MtcSession* GetSession() override;
    inline IMtcService& GetService() override { return m_objService; }
    inline MtcUiNotifier& GetUiNotifier() override { return m_objUiNotifier; }
    inline IMtcMediaManager& GetMediaManager() override { return m_objMediaManager; }
    inline IMtcPreconditionManager& GetPreconditionManager() override
    {
        return m_objPreconditionManager;
    }
    inline UssiController* GetUssiController() override { return m_pUssiController; }
    UpdatingInfo& GetUpdatingInfo() override;
    MtcSession* CreateSession(IN ISession* piSession) override;
    MtcSession* CreateSession() override;
    IMtcBlockChecker* CreateBlockChecker(IN const IMSList<IMtcBlockRule*>& lstRules) override;
    JniCallInfo CreateJniCallInfo() override;
    ISipClientConnection* CreateClientConnection(IN IMS_SINT32 nMethod) override;
    void RemoveSession(IN const ISession* piSession) override;
    void RemoveInactiveSessions(IN const ISession* piActiveSession) override;
    void DeleteUpdatingInfo() override;
    inline MtcTimerWrapper& GetTimer() override { return m_objTimer; }
    inline MtcSupplementaryService& GetSupplementaryService() override
    {
        return m_objSupplementaryService;
    }
    inline IMS_SINT32 GetSlotId() override { return m_objContext.GetSlotId(); }
    inline MtcDialingPlan& GetDialingPlan() override { return m_objContext.GetDialingPlan(); }
    inline IMtcService* GetServiceByType(IN ServiceType eServiceType) override
    {
        return m_objContext.GetServiceByType(eServiceType);
    }
    inline IMtcCallManager& GetCallManager() override { return m_objContext.GetCallManager(); }
    inline MtcCallController& GetCallController() override
    {
        return m_objContext.GetCallController();
    }
    inline MtcVonrManager& GetVonrManager() override { return m_objContext.GetVonrManager(); }
    inline MtcConfigurationProxy& GetConfigurationProxy() override
    {
        return m_objContext.GetConfigurationProxy();
    }
    inline CallStateProxy& GetCallStateProxy() override { return m_objContext.GetCallStateProxy(); }
    inline MtcImsEventReceiver& GetImsEventReceiver() override
    {
        return m_objContext.GetImsEventReceiver();
    }
    inline MtcAosConnector* GetAosConnector(IN ServiceType eServiceType) override
    {
        return m_objContext.GetAosConnector(eServiceType);
    }
    inline MtcSipInterfaceFactory& GetSipInterfaceFactory() override
    {
        return m_objContext.GetSipInterfaceFactory();
    }
    inline ConferenceManager& GetConferenceManager() override
    {
        return m_objContext.GetConferenceManager();
    }
    inline EctManager* GetEctManager() override { return m_objContext.GetEctManager(); }

    inline void SetHeldByMe(IN IMS_BOOL bHeldByMe) override { m_bHeldByMe = bHeldByMe; }

    void SessionAlerting(IN ISession* piSession) override;
    void SessionReferenceReceived(IN ISession* piSession, IN IReference* piReference) override;
    void SessionStarted(IN ISession* piSession) override;
    void SessionStartFailed(IN ISession* piSession) override;
    void SessionTerminated(IN ISession* piSession) override;
    void SessionUpdated(IN ISession* piSession) override;
    void SessionUpdateFailed(IN ISession* piSession) override;
    void SessionUpdateReceived(IN ISession* piSession) override;
    void SessionCancelDelivered(IN ISession* piSession) override;
    void SessionCancelDeliveryFailed(IN ISession* piSession) override;
    void SessionEarlyMediaUpdated(IN ISession* piSession) override;
    void SessionEarlyMediaUpdateFailed(IN ISession* piSession) override;
    void SessionEarlyMediaUpdateReceived(IN ISession* piSession) override;
    void SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) override;
    void SessionPrackDelivered(IN ISession* piSession) override;
    void SessionPrackDeliveryFailed(IN ISession* piSession) override;
    void SessionPrackReceived(IN ISession* piSession) override;
    void SessionProvisionalResponseReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    void SessionRprDeliveryFailed(IN ISession* piSession) override;
    void SessionRprReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) override;
    void SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) override;

    IMS_RESULT MessageMediator_AdjustMessage(
            IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 nMessage) override;

    void OnTimerExpired(IN IMS_SINT32 nType) override;

    void OnBlockChecked(IN IMtcBlockChecker::Result objResult) override;

    void QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) override;
    void QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction) override;

    MtcCallState* CreateState(IN CallStateName eState) override;
    void OnStateTransition(IN CallStateName eState) override;

    virtual void ClientConnection_NotifyResponse(IN ISipClientConnection* piScc,
            IN ISipClientConnection* piForkedScc = IMS_NULL) override;
    virtual void Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    virtual void OnReceivingMediaDataFailed(IN IMS_UINT32 eMediaType) override;
    virtual void OnVideoLowestBitRate() override;
    virtual void OnReceivingNetworkToneStarted() override;
    virtual void OnReceivingNetworkToneFailed() override;
    virtual void OnMediaFailed(IN FailReason objReason) override;

private:
    static IMutex* s_pKeyCreationLock;

    CallKey CreateCallKey();
    void OnInternalFailure();
    void OnAttached();

    IMtcContext& m_objContext;
    IMtcService& m_objService;

    CallKey m_nKey;

    IMS_BOOL m_bHeldByMe;

    CallInfo m_objCallInfo;
    ParticipantInfo m_objParticipantInfo;
    UpdatingInfo* m_pUpdatingInfo;
    IMSList<MtcSession*> m_lstSessions;

    MtcCallStateMachine<MtcCallState, CallStateName> m_objStateMachine;
    MtcTimerWrapper m_objTimer;
    MtcUiNotifier m_objUiNotifier;
    MtcMediaManager m_objMediaManager;
    MtcPreconditionManager m_objPreconditionManager;
    MtcSupplementaryService m_objSupplementaryService;
    UssiController* m_pUssiController;
};

#endif
