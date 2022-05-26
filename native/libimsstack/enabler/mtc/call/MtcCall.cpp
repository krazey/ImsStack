#include "IMessage.h"
#include "IMSTypeDef.h"
#include "IReference.h"
#include "ISession.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "call/MtcCall.h"
#include "call/state/AlertingState.h"
#include "call/state/EstablishedState.h"
#include "call/state/IdleState.h"
#include "call/state/IncomingState.h"
#include "call/state/OutgoingState.h"
#include "call/state/TerminatingState.h"
#include "call/state/UpdatingState.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/CallStateProxy.h"
#include "helper/MtcAosConnector.h"
#include "helper/block/MtcBlockChecker.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE GLOBAL IMutex* MtcCall::s_pKeyCreationLock = MutexService::GetMutexService()->CreateMutex();

PUBLIC
MtcCall::MtcCall(
        IN IMtcContext& objContext, IN IMtcService& objService, IN const CallInfo& objCallInfo) :
        m_objContext(objContext),
        m_objService(objService),
        m_nKey(CreateCallKey()),
        m_bHeldByMe(IMS_FALSE),
        m_objCallInfo(objCallInfo),
        m_objParticipantInfo(ParticipantInfo(*this)),
        m_pUpdatingInfo(IMS_NULL),
        m_pSession(IMS_NULL),
        m_objStateMachine(
                MtcCallStateMachine<MtcCallState, CallStateName>(CallStateName::IDLE, *this, this)),
        m_objTimer(MtcTimerWrapper()),
        m_objUiNotifier(MtcUiNotifier(*this)),
        m_objMediaManager(MtcMediaManager(*this)),
        m_objPreconditionManager(MtcPreconditionManager(*this)),
        m_objSupplementaryService(MtcSupplementaryService(objContext.GetConfigurationProxy()))
{
    IMS_TRACE_D("+MtcCall key[%d]", m_nKey, 0, 0);

    m_objTimer.SetListener(this);
    m_objPreconditionManager.SetListener(this);
}

PUBLIC VIRTUAL MtcCall::~MtcCall()
{
    IMS_TRACE_D("~MtcCall key[%d]", m_nKey, 0, 0);
    delete m_pSession;
}

PUBLIC VIRTUAL void MtcCall::HandleIncoming(
        IN ISession* piSession, IN JniMtcServiceThread* pServiceThread)
{
    IMS_TRACE_I("HandleIncoming : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL || pServiceThread == IMS_NULL)
    {
        if (piSession != IMS_NULL)
        {
            piSession->Reject();
            GetSipInterfaceFactory().GetISessionHolder()->AddISession(piSession);
            GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(piSession);
        }
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->HandleIncoming(piSession, pServiceThread);
            });
}

PUBLIC VIRTUAL void MtcCall::Attach(
        IN JniMtcCallThread* pJniMtcCallThread, IN JniMediaSessionThread* pJniMediaThread)
{
    IMS_TRACE_I("Attach : key[%d]", m_nKey, 0, 0);

    if (pJniMtcCallThread == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    // TODO: will be removed and JniConnector will provide the getters.
    m_objUiNotifier.SetJniCallThread(pJniMtcCallThread);
    m_objUiNotifier.SetJniMediaThread(pJniMediaThread);

    if (m_objCallInfo.ePeerType == PeerType::MT)
    {
        OnAttached();
    }
}

PUBLIC VIRTUAL void MtcCall::Detach()
{
    IMS_TRACE_I("Detach : key[%d]", m_nKey, 0, 0);
    m_objUiNotifier.SetJniCallThread(IMS_NULL);
}

PUBLIC VIRTUAL void MtcCall::Start(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("Start : key[%d]", m_nKey, 0, 0);

    if (pMediaInfo == IMS_NULL)
    {
        delete pMediaInfo;
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Start(eCallType, strTarget, pMediaInfo, objSuppServices);
            });
}

PUBLIC VIRTUAL void MtcCall::HandleUserAlert()
{
    IMS_TRACE_I("HandleUserAlert : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->HandleUserAlert();
            });
}

PUBLIC VIRTUAL void MtcCall::Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("Accept : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Accept(eCallType, pMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::Reject(IN const FailReason& objReason)
{
    IMS_TRACE_I("Reject : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Reject(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Hold(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("Hold : key[%d]", m_nKey, 0, 0);

    if (pMediaInfo == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Hold(pMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::Resume(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("Resume : key[%d]", m_nKey, 0, 0);

    if (pMediaInfo == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Resume(pMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::AcceptResume(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("AcceptResume : key[%d]", m_nKey, 0, 0);

    if (pMediaInfo == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->AcceptResume(eCallType, pMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::RejectResume(IN const FailReason& objReason)
{
    IMS_TRACE_I("RejectResume : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->RejectResume(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Convert(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("Convert : key[%d]", m_nKey, 0, 0);

    if (pMediaInfo == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Convert(eCallType, pMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::AcceptConvert(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("AcceptConvert : key[%d]", m_nKey, 0, 0);

    if (pMediaInfo == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->AcceptConvert(eCallType, pMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::RejectConvert(IN const FailReason& objReason)
{
    IMS_TRACE_I("RejectConvert : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->RejectConvert(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::CancelConvert(IN const FailReason& objReason)
{
    IMS_TRACE_I("CancelConvert : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->CancelConvert(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Terminate(IN const FailReason& objReason)
{
    IMS_TRACE_I("Terminate : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Terminate(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration)
{
    IMS_TRACE_I("SendDtmf : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SendDtmf(strSignal, nDuration);
            });
}

PUBLIC VIRTUAL void MtcCall::SendUssi(IN const AString& strUssi)
{
    IMS_TRACE_I("SendUssi : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SendUssi(strUssi);
            });
}

PUBLIC VIRTUAL void MtcCall::StartConference(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices,
        IN IMSList<ConfUser*> lstUsers)
{
    IMS_TRACE_I("StartConference : key[%d]", m_nKey, 0, 0);

    if (pMediaInfo == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->StartConference(
                        eCallType, strTarget, pMediaInfo, objSuppServices, lstUsers);
            });
}

PUBLIC VIRTUAL void MtcCall::StartConference(
        IN CallType eCallType, IN const AString& strTarget, IN IMSList<ConfUser*> lstUsers)
{
    IMS_TRACE_I("StartConference : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->StartConference(eCallType, strTarget, lstUsers);
            });
}

PUBLIC VIRTUAL void MtcCall::HandleSrvccSuccess()
{
    IMS_TRACE_I("HandleSrvccSuccess : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->HandleSrvccSuccess();
            });
}

PUBLIC VIRTUAL void MtcCall::HandleSrvccFailure(IN UpdateType eUpdateType)
{
    IMS_TRACE_I("HandleSrvccFailure : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->HandleSrvccFailure(eUpdateType);
            });
}

PUBLIC VIRTUAL UpdatingInfo& MtcCall::GetUpdatingInfo()
{
    if (m_pUpdatingInfo == IMS_NULL)
    {
        m_pUpdatingInfo = new UpdatingInfo();
    }

    return *m_pUpdatingInfo;
}

PUBLIC VIRTUAL MtcSession* MtcCall::CreateSession(IN ISession& objSession, IN CallType eCallType)
{
    objSession.SetListener(this);
    return new MtcSession(*this, objSession, eCallType);
}

PUBLIC VIRTUAL IMtcBlockChecker* MtcCall::CreateBlockChecker(
        IN const IMSList<IMtcBlockRule*>& lstRules)
{
    return new MtcBlockChecker(lstRules, *this);
}

PUBLIC VIRTUAL JniCallInfo MtcCall::CreateJniCallInfo()
{
    JniCallInfo objJniCallInfo;
    objJniCallInfo.eServiceType = GetService().GetServiceType();
    objJniCallInfo.eCallType = GetSession() ? GetSession()->GetCallType() : CallType::UNKNOWN;
    objJniCallInfo.bWifi = m_objCallInfo.bWifi;
    objJniCallInfo.bEmergency = m_objCallInfo.bEmergency;
    objJniCallInfo.bOffline = m_objCallInfo.bOffline;
    objJniCallInfo.bUssi = m_objCallInfo.bUssi;
    objJniCallInfo.bConference = m_objCallInfo.bConference;
    objJniCallInfo.bConferenceEnabled = IMS_TRUE;  // TODO: Any meaning to use this?
    objJniCallInfo.bConferenceSubscriptionRequired =
            m_objContext.GetConfigurationProxy().GetInt(Feature::CONFERENCE_SUBSCRIBE_TYPE) > -1;
    objJniCallInfo.bRttCapable = GetSession() ? GetSession()->IsRttCapable() : IMS_FALSE;
    objJniCallInfo.bVideoCapable = GetSession() ? GetSession()->IsVideoCapable() : IMS_FALSE;

    return objJniCallInfo;
}

PUBLIC VIRTUAL void MtcCall::DeleteUpdatingInfo()
{
    delete m_pUpdatingInfo;
    m_pUpdatingInfo = IMS_NULL;
}

PUBLIC VIRTUAL void MtcCall::SessionAlerting(IN ISession* piSession)
{
    IMS_TRACE_I("SessionAlerting : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionAlerting(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionReferenceReceived(
        IN ISession* piSession, IN IReference* piReference)
{
    IMS_TRACE_I("SessionReferenceReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL || piReference == IMS_NULL)
    {
        if (piSession != IMS_NULL)
        {
            piSession->Terminate();
            GetSipInterfaceFactory().GetISessionHolder()->AddISession(piSession);
            GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(piSession);
        }
        if (piReference != IMS_NULL)
        {
            piReference->Reject();
            piReference->Destroy();  // TODO: Use ReferenceInterfaceHolder
        }
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionReferenceReceived(piSession, piReference);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionStarted(IN ISession* piSession)
{
    IMS_TRACE_I("SessionStarted : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionStarted(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionStartFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionStartFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionTerminated(IN ISession* piSession)
{
    IMS_TRACE_I("SessionTerminated : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionTerminated(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionUpdated(IN ISession* piSession)
{
    IMS_TRACE_I("SessionUpdated : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionUpdated(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionUpdateFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionUpdateFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_I("SessionUpdateReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionUpdateReceived(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionCancelDelivered(IN ISession* piSession)
{
    IMS_TRACE_I("SessionCancelDelivered : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionCancelDelivered(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionCancelDeliveryFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionCancelDeliveryFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionCancelDeliveryFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionEarlyMediaUpdated(IN ISession* piSession)
{
    IMS_TRACE_I("SessionEarlyMediaUpdated : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionEarlyMediaUpdated(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionEarlyMediaUpdateFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionEarlyMediaUpdateFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionEarlyMediaUpdateFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionEarlyMediaUpdateReceived(IN ISession* piSession)
{
    IMS_TRACE_I("SessionEarlyMediaUpdateReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionEarlyMediaUpdateReceived(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionForkedResponseReceived(
        IN ISession* piSession, IN ISession* piForkedSession)
{
    IMS_TRACE_I("SessionForkedResponseReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL || piForkedSession == IMS_NULL)
    {
        if (piSession != IMS_NULL)
        {
            piSession->Reject();
            GetSipInterfaceFactory().GetISessionHolder()->AddISession(piSession);
            GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(piSession);
        }
        if (piForkedSession != IMS_NULL)
        {
            piForkedSession->Terminate();
            GetSipInterfaceFactory().GetISessionHolder()->AddISession(piForkedSession);
            GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(piForkedSession);
        }
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionForkedResponseReceived(piSession, piForkedSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionPRAckDelivered(IN ISession* piSession)
{
    IMS_TRACE_I("SessionPRAckDelivered : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionPRAckDelivered(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionPRAckDeliveryFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionPRAckDeliveryFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionPRAckDeliveryFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionPRAckReceived(IN ISession* piSession)
{
    IMS_TRACE_I("SessionPRAckReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionPRAckReceived(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionProvisionalResponseReceived(
        IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_I("SessionProvisionalResponseReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionProvisionalResponseReceived(piSession, nIndex);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionRPRDeliveryFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionRPRDeliveryFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionRPRDeliveryFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionRPRReceived(IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_I("SessionRPRReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionRPRReceived(piSession, nIndex);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionTransactionReceived(
        IN ISession* piSession, IN ISipServerConnection* piSipServerConnection)
{
    IMS_TRACE_I("SessionTransactionReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionTransactionReceived(piSession, piSipServerConnection);
            });
}

PUBLIC VIRTUAL IMS_RESULT MtcCall::MessageMediator_AdjustMessage(
        IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 nMessage)
{
    IMS_TRACE_I("MessageMediator_AdjustMessage : key[%d]", m_nKey, 0, 0);

    if (piSipMessage == IMS_NULL)
    {
        OnInternalFailure();
        return IMS_FAILURE;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->MessageMediator_AdjustMessage(piSipMessage, nMessage);
            });

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL void MtcCall::OnTimerExpired(IN IMS_SINT32 nType)
{
    IMS_TRACE_I("OnTimerExpired : key[%d] type[%d]", m_nKey, nType, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->OnTimerExpired(nType);
            });
}

PUBLIC VIRTUAL void MtcCall::OnBlockChecked(IN IMtcBlockChecker::Result objResult)
{
    IMS_TRACE_I("OnBlockChecked : key[%d] result[%d]", m_nKey,
            static_cast<IMS_SINT32>(objResult.eStatus), 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->OnBlockChecked(objResult);
            });
}

PUBLIC VIRTUAL void MtcCall::QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_I("QosReserved : key[%d] MediaType[%d]", m_nKey, eMediaType, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->QosReserved(piSession, eMediaType);
            });
}

PUBLIC VIRTUAL void MtcCall::QosReserveFailed(IN ISession* piSession, IN QosLossPolicy eNextAction)
{
    IMS_TRACE_I("QosReserveFailed : key[%d] NextAction[%d]", m_nKey, eNextAction, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->QosReserveFailed(piSession, eNextAction);
            });
}

PUBLIC VIRTUAL MtcCallState* MtcCall::CreateState(IN CallStateName eState)
{
    switch (eState)
    {
        case CallStateName::IDLE:
            return new IdleState(*this);
        case CallStateName::OUTGOING:
            return new OutgoingState(*this);
        case CallStateName::INCOMING:
            return new IncomingState(*this);
        case CallStateName::ALERTING:
            return new AlertingState(*this);
        case CallStateName::ESTABLISHED:
            return new EstablishedState(*this);
        case CallStateName::UPDATING:
            return new UpdatingState(*this);
        case CallStateName::TERMINATING:
            return new TerminatingState(*this);
    }
}

PUBLIC VIRTUAL void MtcCall::OnStateTransition(IN CallStateName eState)
{
    IMS_TRACE_I(
            "OnStateTransition : key[%d] state[%d]", m_nKey, static_cast<IMS_SINT32>(eState), 0);

    GetCallStateProxy().UpdateCallState(m_nKey, eState,
            GetSession() ? GetSession()->GetCallType() : CallType::UNKNOWN,
            m_objCallInfo.bEmergency);
}

PRIVATE
CallKey MtcCall::CreateCallKey()
{
    LockGuard objLock(s_pKeyCreationLock);

    static CallKey nKey = 0;
    return nKey++;
}

PRIVATE
void MtcCall::OnInternalFailure()
{
    IMS_TRACE_I("OnInternalFailure : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->OnInternalFailure();
            });
}

PRIVATE
void MtcCall::OnAttached()
{
    IMS_TRACE_I("OnAttached : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->OnAttached();
            });
}
