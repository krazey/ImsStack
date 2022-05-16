#include "IMessage.h"
#include "ImsAosParameter.h"
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
#include "helper/CallStateProxy.h"
#include "helper/MtcAosConnector.h"
#include "helper/block/MtcBlockChecker.h"
#include "helper/sipinterfaceholder/MtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE GLOBAL IMutex* MtcCall::s_pKeyCreationLock = MutexService::GetMutexService()->CreateMutex();

PUBLIC
MtcCall::MtcCall(IN IMtcContext& objContext, IN IMtcService& objService, IN CallInfo objCallInfo) :
        m_objContext(objContext),
        m_objService(objService),
        m_nKey(CreateCallKey()),
        m_bEct(IMS_FALSE),
        m_bHeldByMe(IMS_FALSE),
        m_objCallInfo(objCallInfo),
        m_objParticipantInfo(ParticipantInfo(*this)),
        m_pUpdatingInfo(IMS_NULL),
        m_objStateMachine(
                MtcCallStateMachine<MtcCallState, CallStateName>(CallStateName::IDLE, *this, this)),
        m_pSession(IMS_NULL),
        m_objTimer(MtcTimerWrapper()),
        m_objUiNotifier(MtcUiNotifier()),
        m_objMediaManager(MtcMediaManager(*this)),
        m_objPreconditionManager(MtcPreconditionManager(*this)),
        m_objSupplementaryService(MtcSupplementaryService(objContext.GetConfigurationProxy()))
{
    IMS_TRACE_D("+MtcCall key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objCallInfo.eServiceType = objService.GetServiceType();

    m_objTimer.SetListener(this);
    m_objPreconditionManager.SetListener(this);
}

PUBLIC VIRTUAL MtcCall::~MtcCall()
{
    IMS_TRACE_D("~MtcCall key[%" PFLS_x "]", m_nKey, 0, 0);
    delete m_pSession;
}

PUBLIC VIRTUAL void MtcCall::HandleIncoming(
        IN ISession* piSession, IN JniMtcServiceThread* pServiceThread)
{
    IMS_TRACE_I("HandleIncoming : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("Attach : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("Detach : key[%" PFLS_x "]", m_nKey, 0, 0);
    m_objUiNotifier.SetJniCallThread(IMS_NULL);
}

PUBLIC VIRTUAL void MtcCall::Start(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo* pMediaInfo, IN const IMSMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("Start : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("HandleUserAlert : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->HandleUserAlert();
            });
}

PUBLIC VIRTUAL void MtcCall::Accept(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("Accept : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Accept(eCallType, pMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::Reject(IN const FailReason& objReason)
{
    IMS_TRACE_I("Reject : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Reject(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Hold(IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("Hold : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("Resume : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("AcceptResume : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("RejectResume : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->RejectResume(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Convert(IN CallType eCallType, IN MediaInfo* pMediaInfo)
{
    IMS_TRACE_I("Convert : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("AcceptConvert : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("RejectConvert : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->RejectConvert(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::CancelConvert(IN const FailReason& objReason)
{
    IMS_TRACE_I("CancelConvert : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->CancelConvert(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Terminate(IN const FailReason& objReason)
{
    IMS_TRACE_I("Terminate : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->Terminate(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::SendDtmf(IN const AString& strSignal, IN IMS_SINT32 nDuration)
{
    IMS_TRACE_I("SendDtmf : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SendDtmf(strSignal, nDuration);
            });
}

PUBLIC VIRTUAL void MtcCall::SendUssi(IN const AString& strUssi)
{
    IMS_TRACE_I("SendUssi : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("StartConference : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("StartConference : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->StartConference(eCallType, strTarget, lstUsers);
            });
}

PUBLIC VIRTUAL void MtcCall::HandleSrvccSuccess()
{
    IMS_TRACE_I("HandleSrvccSuccess : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->HandleSrvccSuccess();
            });
}

PUBLIC VIRTUAL void MtcCall::HandleSrvccFailure(IN UpdateType eUpdateType)
{
    IMS_TRACE_I("HandleSrvccFailure : key[%" PFLS_x "]", m_nKey, 0, 0);

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

PUBLIC VIRTUAL MtcSession* MtcCall::CreateSession(IN ISession& objSession)
{
    objSession.SetListener(this);
    return new MtcSession(*this, objSession);
}

PUBLIC VIRTUAL IMtcBlockChecker* MtcCall::CreateBlockChecker(
        IN const IMSList<IMtcBlockRule*>& lstRules)
{
    return new MtcBlockChecker(lstRules, *this);
}

PUBLIC VIRTUAL void MtcCall::DeleteUpdatingInfo()
{
    delete m_pUpdatingInfo;
    m_pUpdatingInfo = IMS_NULL;
}

PUBLIC VIRTUAL void MtcCall::SessionAlerting(IN ISession* piSession)
{
    IMS_TRACE_I("SessionAlerting : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionReferenceReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionStarted : key[%" PFLS_x "]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    SetVideoCapable(piSession);  // TODO: move into CallInfo

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->SessionStarted(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionStartFailed : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionTerminated : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionUpdated : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionUpdateFailed : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionUpdateReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionCancelDelivered : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionCancelDeliveryFailed : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionEarlyMediaUpdated : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionEarlyMediaUpdateFailed : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionEarlyMediaUpdateReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionForkedResponseReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionPRAckDelivered : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionPRAckDeliveryFailed : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionPRAckReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionProvisionalResponseReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionRPRDeliveryFailed : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionRPRReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("SessionTransactionReceived : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("MessageMediator_AdjustMessage : key[%" PFLS_x "]", m_nKey, 0, 0);

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
    IMS_TRACE_I("OnTimerExpired : key[%" PFLS_x "] type[%d]", m_nKey, nType, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->OnTimerExpired(nType);
            });
}

PUBLIC VIRTUAL void MtcCall::OnBlockChecked(IN IMtcBlockChecker::Result objResult)
{
    IMS_TRACE_I("OnBlockChecked : key[%" PFLS_x "] result[%d]", m_nKey,
            static_cast<IMS_SINT32>(objResult.eStatus), 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->OnBlockChecked(objResult);
            });
}

PUBLIC VIRTUAL void MtcCall::QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType)
{
    IMS_TRACE_I("QosReserved : key[%" PFLS_x "] MediaType[%d]", m_nKey, eMediaType, 0);

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
    IMS_TRACE_I("QosReserveFailed : key[%" PFLS_x "] NextAction[%d]", m_nKey, eNextAction, 0);

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
    IMS_TRACE_I("OnStateTransition : key[%" PFLS_x "] state[%d]", m_nKey,
            static_cast<IMS_SINT32>(eState), 0);

    GetCallStateProxy().UpdateCallState(m_nKey, m_objCallInfo, eState);
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
    IMS_TRACE_I("OnInternalFailure : key[%" PFLS_x "]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](MtcCallState* pState)
            {
                return pState->OnInternalFailure();
            });
}

PRIVATE
void MtcCall::SetVideoCapable(IN ISession* piSession)
{
    MtcAosConnector* pAosConnector = GetAosConnector(m_objCallInfo.eServiceType);
    if (pAosConnector == IMS_NULL)
    {
        return;
    }

    IMS_UINT32 nFeatures = pAosConnector->GetFeatures();
    if ((nFeatures & ImsAosFeature::MMTEL) && (nFeatures & ImsAosFeature::VIDEO))
    {
        IMessage* piMessage;
        if (m_objCallInfo.ePeerType == PeerType::MO)
        {
            piMessage = piSession->GetPreviousResponse(IMessage::SESSION_START);
        }
        else
        {
            piMessage = piSession->GetPreviousRequest(IMessage::SESSION_EARLY_UPDATE);

            if (piMessage == IMS_NULL)
            {
                piMessage = MessageUtil::GetRemotePreviousMessage(
                        piSession, IMessage::SESSION_START, false);
            }
        }

        if (piMessage == IMS_NULL)
        {
            return;
        }

        GetCallInfo().bVideoCapable = MessageUtil::IsVideoFeatureIncluded(piMessage);
    }
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
