/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "CarrierConfig.h"
#include "IMediaManager.h"
#include "IMessage.h"
#include "INetworkWatcher.h"
#include "IReference.h"
#include "ISession.h"
#include "ISipClientConnection.h"
#include "ISipKeepAliveHelper.h"
#include "ImsTypeDef.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SipFactory.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "call/EpsFallbackTrigger.h"
#include "call/IMtcSession.h"
#include "call/MtcCall.h"
#include "call/MtcSession.h"
#include "call/block/MtcBlockChecker.h"
#include "call/message/MessageSender.h"
#include "configuration/MtcConfigurationProxy.h"
#include "emergency/CurrentLocationDiscoveryController.h"
#include "helper/ICallStateProxy.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/ISrvccStateListener.h"
#include "helper/UdpKeepAliveSender.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "ussi/UssiController.h"
#include "ussi/UssiData.h"
#include <functional>

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE GLOBAL IMutex* MtcCall::s_pKeyCreationLock = MutexService::GetMutexService()->CreateMutex();

PUBLIC
MtcCall::MtcCall(IN IMtcContext& objContext, IN IMtcService& objService,
        IN const CallInfo& objCallInfo, IN std::unique_ptr<IMtcCallStateFactory> pStateFactory) :
        m_objContext(objContext),
        m_objService(objService),
        m_nKey(CreateCallKey()),
        m_bHeldByMe(IMS_FALSE),
        m_objCallInfo(objCallInfo),
        m_objParticipantInfo(ParticipantInfo(*this)),
        m_pUpdatingInfo(IMS_NULL),
        m_lstSessions(ImsList<IMtcSession*>()),
        m_objStateMachine(
                MtcCallStateMachine(*this, CallStateName::IDLE, std::move(pStateFactory), this)),
        m_objPendingOperationHolder(),
        m_pTimer(objContext.CreateTimer()),
        m_objUiNotifier(MtcUiNotifier(*this)),
        m_objMediaManager(MtcMediaManager(*this, *IMediaManager::GetInstance(GetSlotId()))),
        m_objPreconditionManager(MtcPreconditionManager(*this)),
        m_objSupplementaryService(
                MtcSupplementaryService(*this, objContext.GetConfigurationProxy())),
        m_objMessageMediator(MtcMessageMediator(*this)),
        m_pUssiController(IMS_NULL),
        m_pEpsFallbackTrigger(IMS_NULL),
        m_pCurrentLocationDiscoveryController(IMS_NULL)
{
    IMS_TRACE_D("+MtcCall key[%d]", m_nKey, 0, 0);

    m_pTimer->SetListener(this);
    m_objPreconditionManager.SetListener(this);
    m_objMediaManager.SetMediaReportEventListener(this);
    m_objService.AddAosStateListener(this);
    m_objService.AddSrvccStateListener(this);
    m_objService.AddNetworkWatcherListener(this);
}

PUBLIC VIRTUAL MtcCall::~MtcCall()
{
    IMS_TRACE_D("~MtcCall key[%d]", m_nKey, 0, 0);

    m_objService.RemoveAosStateListener(this);
    m_objService.RemoveSrvccStateListener(this);
    m_objService.RemoveNetworkWatcherListener(this);

    for (IMS_UINT32 nIndex = 0; nIndex < m_lstSessions.GetSize(); nIndex++)
    {
        IMtcSession* pSession = m_lstSessions.GetAt(nIndex);
        delete pSession;
    }
    m_lstSessions.Clear();

    delete m_pUssiController;
    delete m_pEpsFallbackTrigger;
    delete m_pCurrentLocationDiscoveryController;
}

PUBLIC VIRTUAL void MtcCall::HandleIncoming(IN ISession* piSession)
{
    IMS_TRACE_I("HandleIncoming : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    if (UssiController::IsNetworkInitiatedUssi(m_objContext.GetMessageUtils(),
                piSession->GetPreviousRequest(IMessage::SESSION_START)))
    {
        m_pUssiController = new UssiController(*this, new UssiDataParser());
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->HandleIncomingUssi(piSession);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->HandleIncoming(piSession);
                });
    }
}

PUBLIC VIRTUAL void MtcCall::Attach()
{
    IMS_TRACE_I("Attach : key[%d]", m_nKey, 0, 0);

    if (m_objCallInfo.ePeerType == PeerType::MT)
    {
        OnAttached();
    }
}

PUBLIC VIRTUAL void MtcCall::Start(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices)
{
    IMS_TRACE_I("Start : key[%d]", m_nKey, 0, 0);

    if (IsUssi())
    {
        m_pUssiController = new UssiController(*this, new UssiDataParser());
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Start(eCallType, strTarget, objMediaInfo, objSuppServices);
            });
}

PUBLIC VIRTUAL void MtcCall::StartConference(IN CallType eCallType, IN const AString& strTarget,
        IN MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
        IN const ImsList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("StartConference : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->StartConference(
                        eCallType, strTarget, objMediaInfo, objSuppServices, objUsers);
            });
}

PUBLIC VIRTUAL void MtcCall::StartConference(
        IN CallType eCallType, IN const AString& strTarget, IN const ImsList<ConfUser*>& objUsers)
{
    IMS_TRACE_I("StartConference : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->StartConference(eCallType, strTarget, objUsers);
            });
}

PUBLIC VIRTUAL void MtcCall::HandleUserAlert()
{
    IMS_TRACE_I("HandleUserAlert : key[%d]", m_nKey, 0, 0);

    if (IsUssi())
    {
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->HandleUserAlert();
            });
}

PUBLIC VIRTUAL void MtcCall::Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_I("Accept : key[%d]", m_nKey, 0, 0);

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->AcceptUssi(eCallType, objMediaInfo);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->Accept(eCallType, objMediaInfo);
                });
    }
}

PUBLIC VIRTUAL void MtcCall::Reject(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("Reject : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Reject(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Hold(IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_I("Hold : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Hold(objMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::Resume(IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_I("Resume : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Resume(objMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::AcceptResume(IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_I("AcceptResume : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->AcceptResume(eCallType, objMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::RejectResume(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("RejectResume : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->RejectResume(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Update(IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_I("Update : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Update(eCallType, objMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo)
{
    IMS_TRACE_I("AcceptUpdate : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->AcceptUpdate(eCallType, objMediaInfo);
            });
}

PUBLIC VIRTUAL void MtcCall::RejectUpdate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("RejectUpdate : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->RejectUpdate(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::CancelUpdate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("CancelUpdate : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->CancelUpdate(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::Terminate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("Terminate : key[%d]", m_nKey, 0, 0);

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->TerminateUssi(objReason);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->Terminate(objReason);
                });
    }
}

PUBLIC VIRTUAL void MtcCall::SendUssd(IN const AString& strUssd)
{
    IMS_TRACE_I("SendUssd : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->SendUssd(strUssd);
            });
}

PUBLIC VIRTUAL CallType MtcCall::GetCallType() const
{
    IMtcSession* pSession = GetSession();
    if (!pSession)
    {
        return m_objCallInfo.eInitialCallType;
    }
    return pSession->GetCallType();
}

PUBLIC VIRTUAL IMtcSession* MtcCall::GetSession(IN const ISession* piSession) const
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_lstSessions.GetSize(); nIndex++)
    {
        IMtcSession* pSession = m_lstSessions.GetAt(nIndex);
        if (&pSession->GetISession() == piSession)
        {
            return pSession;
        }
    }

    IMS_TRACE_D("GetSession : Not exists", 0, 0, 0);
    return IMS_NULL;
}

PUBLIC VIRTUAL IMtcSession* MtcCall::GetSession() const
{
    if (m_lstSessions.IsEmpty())
    {
        IMS_TRACE_D("GetSession : Empty", 0, 0, 0);
        return IMS_NULL;
    }

    const IMS_UINT32 nLastIndex = m_lstSessions.GetSize() - 1;
    return m_lstSessions.GetAt(nLastIndex);
}

PUBLIC VIRTUAL UpdatingInfo& MtcCall::GetUpdatingInfo()
{
    if (m_pUpdatingInfo == IMS_NULL)
    {
        m_pUpdatingInfo = new UpdatingInfo(*this);
    }

    return *m_pUpdatingInfo;
}

PUBLIC VIRTUAL EpsFallbackTrigger& MtcCall::GetEpsFallbackTrigger()
{
    if (m_pEpsFallbackTrigger == IMS_NULL)
    {
        m_pEpsFallbackTrigger = new EpsFallbackTrigger(*this);
    }

    return *m_pEpsFallbackTrigger;
}

PUBLIC VIRTUAL CurrentLocationDiscoveryController& MtcCall::GetCurrentLocationDiscoveryController()
{
    if (m_pCurrentLocationDiscoveryController == IMS_NULL)
    {
        m_pCurrentLocationDiscoveryController = new CurrentLocationDiscoveryController(*this);
    }

    return *m_pCurrentLocationDiscoveryController;
}

PUBLIC VIRTUAL IMtcSession* MtcCall::CreateSession(IN ISession* piSession)
{
    if (piSession == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession : ISession is null", 0, 0, 0);
        return IMS_NULL;
    }
    if (GetSession(piSession) != IMS_NULL)
    {
        IMS_TRACE_E(0, "CreateSession : Already exists", 0, 0, 0);
        return IMS_NULL;
    }

    piSession->SetListener(this);
    piSession->SetMessageMediator(&m_objMessageMediator);
    piSession->SetRefreshListener(this);

    IMtcSession* pSession = new MtcSession(*this, *piSession, m_objCallInfo.eInitialCallType,
            new MessageSender(*this, *piSession));
    m_lstSessions.Append(pSession);

    IMS_TRACE_D("CreateSession : Session count[%d]", m_lstSessions.GetSize(), 0, 0);

    return pSession;
}

PUBLIC VIRTUAL IMtcSession* MtcCall::CreateSession()
{
    ISession* piSession = GetSipInterfaceFactory().GetISessionHolder().GetISession(m_nKey,
            GetService().GetICoreService(), GetParticipantInfo().GetLocalUri(),
            GetParticipantInfo().GetRemoteUri());

    return CreateSession(piSession);
}

PUBLIC VIRTUAL IMtcBlockChecker* MtcCall::CreateBlockChecker(
        IN const ImsList<IMtcBlockRule*>& lstRules)
{
    return new MtcBlockChecker(lstRules, this);
}

PUBLIC VIRTUAL JniCallInfo MtcCall::CreateJniCallInfo()
{
    JniCallInfo objJniCallInfo;
    objJniCallInfo.eServiceType = GetService().GetServiceType();
    objJniCallInfo.eCallType = GetCallType();
    objJniCallInfo.eEmergencyType = m_objCallInfo.eEmergencyType;
    objJniCallInfo.bOffline = m_objCallInfo.bOffline;
    objJniCallInfo.bUssi = m_objCallInfo.bUssi;
    objJniCallInfo.bConference = m_objCallInfo.bConference;
    // TODO: conference extension for SKT
    objJniCallInfo.bConferenceEnabled = IMS_FALSE;
    // TODO: check host or participant
    objJniCallInfo.bConferenceSubscriptionRequired =
            m_objContext.GetConfigurationProxy().GetInt(
                    ConfigVoice::KEY_CONFERENCE_SUBSCRIBE_TYPE_INT) !=
            ConfigVoice::CONFERENCE_SUBSCRIBE_NOT_SUPPORT;
    objJniCallInfo.bRttCapable = GetSession() ? GetSession()->IsRttCapable() : IMS_FALSE;
    objJniCallInfo.bVideoCapable = GetSession() ? GetSession()->IsVideoCapable() : IMS_FALSE;

    return objJniCallInfo;
}

PUBLIC VIRTUAL ISipClientConnection* MtcCall::CreateClientConnection(IN SipMethod eMethod)
{
    IMtcSession* pSession = GetSession();
    if (!pSession)
    {
        return IMS_NULL;
    }

    ISipClientConnection* piSipClientConnection =
            pSession->GetISession().CreateTransaction(eMethod);

    if (piSipClientConnection)
    {
        piSipClientConnection->SetListener(this);
        piSipClientConnection->SetErrorListener(this);
    }

    return piSipClientConnection;
}

PUBLIC VIRTUAL UdpKeepAliveSender* MtcCall::CreateUdpKeepAliveSender()
{
    ISipKeepAliveHelper* pKeepAliveHelper = SipFactory::CreateKeepAliveHelper(GetSlotId());

    // UdpKeepAliveSender deletes pKeepAliveHelper.
    return new UdpKeepAliveSender(pKeepAliveHelper, *this);
}

PUBLIC VIRTUAL void MtcCall::RemoveSession(IN const ISession* piSession)
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_lstSessions.GetSize(); nIndex++)
    {
        IMtcSession* pSession = m_lstSessions.GetAt(nIndex);
        if (&pSession->GetISession() == piSession)
        {
            m_lstSessions.RemoveAt(nIndex);
            delete pSession;

            IMS_TRACE_D("RemoveSession : Session count[%d]", m_lstSessions.GetSize(), 0, 0);
            return;
        }
    }

    IMS_TRACE_E(0, "RemoveSession : Not exists", 0, 0, 0);
}

PUBLIC VIRTUAL void MtcCall::RemoveInactiveSessions(IN const ISession* piActiveSession)
{
    IMtcSession* pActiveSession = GetSession(piActiveSession);
    if (pActiveSession == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < m_lstSessions.GetSize(); nIndex++)
    {
        IMtcSession* pSession = m_lstSessions.GetAt(nIndex);
        if (pSession != pActiveSession)
        {
            delete pSession;
        }
    }
    m_lstSessions.Clear();

    m_lstSessions.Append(pActiveSession);
    IMS_TRACE_D("RemoveInactiveSessions : Session count[%d]", m_lstSessions.GetSize(), 0, 0);
}

PUBLIC VIRTUAL void MtcCall::DeleteUpdatingInfo()
{
    delete m_pUpdatingInfo;
    m_pUpdatingInfo = IMS_NULL;
}

PUBLIC VIRTUAL void MtcCall::RunPendingOperationIfPossible()
{
    while (m_objPendingOperationHolder.HasPendingOperation() &&
            GetState() == CallStateName::ESTABLISHED)
    {
        IMS_TRACE_I("RunPendingOperationIfPossible : key[%d]", m_nKey, 0, 0);
        m_objStateMachine.RunStateOperation(m_objPendingOperationHolder.PopPendingOperation());
    }
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
            [&](IMtcCallState* pState)
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
            GetSipInterfaceFactory().GetISessionHolder().AddISession(m_nKey, piSession);
            GetSipInterfaceFactory().GetISessionHolder().ReleaseISession(piSession);
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
            [&](IMtcCallState* pState)
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

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->UssiStarted(piSession);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->SessionStarted(piSession);
                });
    }
}

PUBLIC VIRTUAL void MtcCall::SessionStartFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionStartFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    IMtcSession* piMtcSession = GetSession(piSession);
    if (piMtcSession)
    {
        piMtcSession->SetSessionTerminatedOrStartFailed();
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
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

    IMtcSession* piMtcSession = GetSession(piSession);
    if (piMtcSession)
    {
        piMtcSession->SetSessionTerminatedOrStartFailed();
    }

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->UssiTerminated(piSession);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->SessionTerminated(piSession);
                });
    }
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            GetSipInterfaceFactory().GetISessionHolder().AddISession(m_nKey, piSession);
            GetSipInterfaceFactory().GetISessionHolder().ReleaseISession(piSession);
        }
        if (piForkedSession != IMS_NULL)
        {
            piForkedSession->Terminate();
            GetSipInterfaceFactory().GetISessionHolder().AddISession(m_nKey, piForkedSession);
            GetSipInterfaceFactory().GetISessionHolder().ReleaseISession(piForkedSession);
        }
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->SessionForkedResponseReceived(piSession, piForkedSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionPrackDelivered(IN ISession* piSession)
{
    IMS_TRACE_I("SessionPrackDelivered : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->SessionPrackDelivered(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionPrackDeliveryFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionPrackDeliveryFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->SessionPrackDeliveryFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionPrackReceived(IN ISession* piSession)
{
    IMS_TRACE_I("SessionPrackReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->SessionPrackReceived(piSession);
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
            [&](IMtcCallState* pState)
            {
                return pState->SessionProvisionalResponseReceived(piSession, nIndex);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionRprDeliveryFailed(IN ISession* piSession)
{
    IMS_TRACE_I("SessionRprDeliveryFailed : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->SessionRprDeliveryFailed(piSession);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionRprReceived(IN ISession* piSession, IN IMS_UINT32 nIndex)
{
    IMS_TRACE_I("SessionRprReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->SessionRprReceived(piSession, nIndex);
            });
}

PUBLIC VIRTUAL void MtcCall::SessionTransactionReceived(
        IN ISession* piSession, IN ISipServerConnection* piSipServerConnection)
{
    IMS_TRACE_I("SessionTransactionReceived : key[%d]", m_nKey, 0, 0);

    if (piSession == IMS_NULL || piSipServerConnection == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->UssiInfoReceived(piSession, piSipServerConnection);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->SessionTransactionReceived(piSession, piSipServerConnection);
                });
    }
}

PUBLIC VIRTUAL void MtcCall::Refresh_NotifyCompleted(IN ISipClientConnection* piScc)
{
    IMS_TRACE_D("Refresh_NotifyCompleted key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Refresh_NotifyCompleted(piScc);
            });
}

PUBLIC VIRTUAL void MtcCall::Refresh_NotifyTerminated()
{
    IMS_TRACE_D("Refresh_NotifyTerminated : key[%d]", m_nKey, 0, 0);
    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Refresh_NotifyTerminated();
            });
}

PUBLIC VIRTUAL void MtcCall::Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh)
{
    IMS_TRACE_D("Refresh_NotifyTimerExpired : key[%d]", m_nKey, 0, 0);
    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->Refresh_NotifyTimerExpired(bDoImplicitRefresh);
            });
}

PUBLIC VIRTUAL void MtcCall::OnTimerExpired(IN IMS_SINT32 nType)
{
    IMS_TRACE_I("OnTimerExpired : key[%d] type[%d]", m_nKey, nType, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnTimerExpired(nType);
            });
}

PUBLIC VIRTUAL void MtcCall::OnBlockChecked(IN IMtcBlockChecker::Result objResult)
{
    IMS_TRACE_I("OnBlockChecked : key[%d] result[%d]", m_nKey,
            static_cast<IMS_SINT32>(objResult.eStatus), 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
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
            [&](IMtcCallState* pState)
            {
                return pState->QosReserveFailed(piSession, eNextAction);
            });
}

PUBLIC VIRTUAL void MtcCall::OnStateTransition(IN CallStateName eState)
{
    IMS_TRACE_I(
            "OnStateTransition : key[%d] state[%d]", m_nKey, static_cast<IMS_SINT32>(eState), 0);

    GetCallStateProxy().UpdateCallState(m_nKey, eState, GetCallType(), m_objCallInfo.IsEmergency());
}

PUBLIC VIRTUAL void MtcCall::ClientConnection_NotifyResponse(
        IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc /*= IMS_NULL*/)
{
    IMS_TRACE_I("ClientConnection_NotifyResponse : key[%d]", m_nKey, 0, 0);
    if (piScc == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->NotifyResponseToUssiInfo(piScc, piForkedScc);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->ClientConnection_NotifyResponse(piScc, piForkedScc);
                });
    }
}

PUBLIC VIRTUAL void MtcCall::Error_NotifyError(
        IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage)
{
    IMS_TRACE_I("Error_NotifyError : key[%d]", m_nKey, 0, 0);
    if (piSc == IMS_NULL)
    {
        OnInternalFailure();
        return;
    }

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->NotifyErrorToUssiInfo(piSc, nCode, strMessage);
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->Error_NotifyError(piSc, nCode, strMessage);
                });
    }
}

PUBLIC VIRTUAL void MtcCall::OnReceivingMediaDataStarted(
        IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType)
{
    IMS_TRACE_I("OnReceivingMediaDataStarted : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnReceivingMediaDataStarted(eMediaType, eProtocolType);
            });
}

PUBLIC VIRTUAL void MtcCall::OnReceivingMediaDataFailed(
        IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType)
{
    IMS_TRACE_I("OnReceivingMediaDataFailed : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnReceivingMediaDataFailed(eMediaType, eProtocolType);
            });
}

PUBLIC VIRTUAL void MtcCall::OnVideoLowestBitRate()
{
    IMS_TRACE_I("OnVideoLowestBitRate : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnVideoLowestBitRate();
            });
}

PUBLIC VIRTUAL void MtcCall::OnReceivingNetworkToneStarted()
{
    IMS_TRACE_I("OnReceivingNetworkToneStarted : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnReceivingNetworkToneStarted();
            });
}

PUBLIC VIRTUAL void MtcCall::OnReceivingNetworkToneFailed()
{
    IMS_TRACE_I("OnReceivingNetworkToneFailed : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnReceivingNetworkToneFailed();
            });
}

PUBLIC VIRTUAL void MtcCall::OnMediaFailed(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_I("OnMediaFailed : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnMediaFailed(objReason);
            });
}

PUBLIC VIRTUAL void MtcCall::OnSrvccStateUpdated(IN SrvccState eState)
{
    IMS_TRACE_I("OnSrvccStateUpdated : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnSrvccStateUpdated(eState);
            });
}

PUBLIC VIRTUAL void MtcCall::OnAosStateChanged(
        IN IMtcService& /*objMtcService*/, IN MtcAosState eState, IN IMS_UINT32 eAosReason)
{
    IMS_TRACE_I("OnAosStateChanged State[%d]", eState, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnAosStateChanged(eState, eAosReason);
            });
}

PUBLIC VIRTUAL void MtcCall::OnIpcanChanged(IN IMtcService& /*objMtcService*/, IN IMS_UINT32 eIpcan)
{
    IMS_TRACE_I("OnIpcanChanged IpCan[%d]", eIpcan, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnIpcanChanged(eIpcan);
            });
}

PUBLIC VIRTUAL void MtcCall::OnRatChanged(IMS_SINT32 eRatType)
{
    IMS_TRACE_I("OnRatChanged :  RAT=%d", eRatType, 0, 0);

    m_objUiNotifier.SendRatChanged(eRatType);
}

PRIVATE
CallKey MtcCall::CreateCallKey()
{
    LockGuard objLock(s_pKeyCreationLock);
    static CallKey s_nKey = 0;

    s_nKey += 1;
    if (s_nKey == CALL_KEY_INVALID)
    {
        s_nKey += 1;
    }
    return s_nKey;
}

PRIVATE
void MtcCall::OnInternalFailure()
{
    IMS_TRACE_I("OnInternalFailure : key[%d]", m_nKey, 0, 0);

    m_objStateMachine.RunStateOperation(
            [&](IMtcCallState* pState)
            {
                return pState->OnInternalFailure();
            });
}

PRIVATE
void MtcCall::OnAttached()
{
    IMS_TRACE_I("OnAttached : key[%" PFLS_x "]", m_nKey, 0, 0);

    if (IsUssi())
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->OnUssiAttached();
                });
    }
    else
    {
        m_objStateMachine.RunStateOperation(
                [&](IMtcCallState* pState)
                {
                    return pState->OnAttached();
                });
    }
}
