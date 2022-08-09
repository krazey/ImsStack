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

#include "IMessage.h"
#include "ImsAosParameter.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallManager.h"
#include "call/MtcSession.h"
#include "call/extension/EarlyDialogTerminatedExtension.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/PreconditionExtension.h"
#include "call/extension/RprExtension.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcSession::MtcSession(
        IN IMtcCallContext& objContext, IN ISession& objSession, IN CallType eCallType) :
        m_objContext(objContext),
        m_objSession(objSession),
        m_objMessageSender(MessageSender(*this, objSession)),
        m_objExtensionSet(GetSupportedExtensions()),
        m_eCallType(eCallType),
        m_bVideoCapable(IMS_FALSE),
        m_bRttCapable(IMS_FALSE),
        m_bTerminated(IMS_FALSE),
        m_strSessionIdHeader(AString::ConstNull())
{
    IMS_TRACE_I("+MtcSession", 0, 0, 0);

    if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
    {
        GetSipInterfaceFactory().GetISessionHolder()->AddISession(&m_objSession);
    }

    UpdateSessionProperty();

    m_bVideoCapable =
            IsRegisteredFeature(ImsAosFeature::MMTEL) && IsRegisteredFeature(ImsAosFeature::VIDEO);
    m_bRttCapable =
            IsRegisteredFeature(ImsAosFeature::MMTEL) && IsRegisteredFeature(ImsAosFeature::TEXT);

    if (m_objContext.GetCallInfo().ePeerType == PeerType::MO &&
            GetConfigurationProxy().Is(Feature::SUPPORT_SIP_SESSION_ID_HEADER))
    {
        m_strSessionIdHeader = GenerateSessionId();
    }
}

PUBLIC VIRTUAL MtcSession::~MtcSession()
{
    IMS_TRACE_I("~MtcSession", 0, 0, 0);

    m_objContext.GetPreconditionManager().DestroyQos(&m_objSession);
    m_objSession.SetMessageMediator(IMS_NULL);
    m_objSession.SetRefreshListener(IMS_NULL);
    GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(&m_objSession);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Start()
{
    IMS_TRACE_D("Start", 0, 0, 0);

    if (m_objContext.GetMediaManager().FormSdp(&m_objSession, m_eCallType) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(&m_objSession, IMS_FALSE);

    m_objExtensionSet.FormatRequest(RequestType::START, *m_objSession.GetNextRequest());
    return m_objMessageSender.Start(GetCallType());
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendProvisionalResponse(IN IMS_BOOL bUserAlert)
{
    IMS_TRACE_D("SendProvisionalResponse", 0, 0, 0);

    IMS_BOOL bIncludeSdp = !IsNeedToRemoveSdpInPr();

    if (bIncludeSdp)
    {
        switch (SetSdpToSend(IMS_FALSE))
        {
            case ResultSetSdp::NO_SDP:
                bIncludeSdp = IMS_FALSE;
                break;
            case ResultSetSdp::FAILURE:
                return IMS_FAILURE;
            case ResultSetSdp::SUCCESS:
                break;
        }
    }

    // TODO: determine the response code based on the configuration for KR carriers?
    IMS_SINT32 nStatusCode = bUserAlert ? SipStatusCode::SC_180 : SipStatusCode::SC_183;

    m_objExtensionSet.FormatResponse(
            ResponseType::PROVISIONAL_RESPONSE, *m_objSession.GetNextResponse());
    return m_objMessageSender.SendProvisionalResponse(
            nStatusCode, IsNeedToReliable(bIncludeSdp), bIncludeSdp, IsCallWaiting());
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendPrack()
{
    IMS_TRACE_D("SendPrack", 0, 0, 0);

    // Not supporting to send Offer in PRACK.
    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatRequest(RequestType::PRACK, *m_objSession.GetNextRequest());
    return m_objMessageSender.SendPrack();
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::RespondToPrack(IN IMS_SINT32 eStatusCode)
{
    IMS_TRACE_D("RespondToPrack", 0, 0, 0);

    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatResponse(ResponseType::PRACK_RESPONSE, *m_objSession.GetNextResponse());
    return m_objMessageSender.RespondToPrack(eStatusCode);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendEarlyUpdate(IN UpdateType eUpdateType)
{
    IMS_TRACE_D("SendEarlyUpdate", 0, 0, 0);

    if (SetSdpToSend(IMS_TRUE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatRequest(RequestType::EARLY_UPDATE, *m_objSession.GetNextRequest());
    return m_objMessageSender.SendEarlyUpdate(eUpdateType);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode)
{
    IMS_TRACE_D("RespondToEarlyUpdate", 0, 0, 0);

    // TODO: check status code in SetSdpToSend()?
    if (SipStatusCode::IsFinalSuccess(eStatusCode) &&
            SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatResponse(
            ResponseType::EARLY_UPDATE_RESPONSE, *m_objSession.GetNextResponse());
    return m_objMessageSender.RespondToEarlyUpdate(eStatusCode);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendAck()
{
    IMS_TRACE_D("SendAck", 0, 0, 0);

    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatRequest(RequestType::ACK, *m_objSession.GetNextRequest());
    return m_objMessageSender.SendAck();
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Accept()
{
    IMS_TRACE_D("Accept", 0, 0, 0);

    // TODO: "REJECT_REASON_MEDIA_FORMFAIL" is required?
    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatResponse(ResponseType::ACCEPT, *m_objSession.GetNextResponse());
    return m_objMessageSender.Accept();
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Reject(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Reject", 0, 0, 0);

    m_objExtensionSet.FormatResponse(ResponseType::REJECT, *m_objSession.GetNextResponse());
    return m_objMessageSender.Reject(objReason);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Update(
        IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo, IN IMS_SINT32 eMethod)
{
    IMS_TRACE_D("Update", 0, 0, 0);

    m_objExtensionSet.FormatRequest(RequestType::UPDATE, *m_objSession.GetNextRequest());
    return m_objMessageSender.Update(
            eUpdateType, bIncludeAlertInfo, eMethod, eUpdateType == UpdateType::REFRESH);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::AcceptUpdate()
{
    IMS_TRACE_D("AcceptUpdate", 0, 0, 0);

    m_objExtensionSet.FormatResponse(ResponseType::ACCEPT_UPDATE, *m_objSession.GetNextResponse());
    return m_objMessageSender.AcceptUpdate();
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::CancelUpdate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("CancelUpdate", 0, 0, 0);

    m_objExtensionSet.FormatRequest(RequestType::CANCEL_UPDATE, *m_objSession.GetNextRequest());
    return m_objMessageSender.CancelUpdate(objReason);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Terminate(
        IMS_BOOL bUseBye, IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Terminate", 0, 0, 0);

    if (m_bTerminated)
    {
        return IMS_FAILURE;
    }

    m_bTerminated = IMS_TRUE;

    m_objExtensionSet.FormatRequest(RequestType::TERMINATE, *m_objSession.GetNextRequest());
    return m_objMessageSender.Terminate(bUseBye, objReason);
}

PUBLIC VIRTUAL void MtcSession::HandleRequest(IN RequestType eType, IN const IMessage& objRequest)
{
    m_objExtensionSet.HandleRequest(eType, objRequest);

    if (eType == RequestType::START)
    {
        if (GetConfigurationProxy().Is(Feature::SUPPORT_SIP_SESSION_ID_HEADER))
        {
            UpdateSessionIdFromMessage(objRequest);
        }
    }

    if (eType == RequestType::START || eType == RequestType::EARLY_UPDATE)
    {
        UpdateCallTypeFromMessage(objRequest);
        if (m_eCallType == CallType::UNKNOWN)
        {
            // UE must send full media list for the incoming INVITE w/o SDP
            // TODO: but, let us optimize.
            m_eCallType = CallType::VOIP;
            m_objContext.GetMediaManager().UpdateMediaDirection(
                    MEDIATYPE_AUDIO, DIRECTION_SEND_RECEIVE);
        }

        UpdateCapabilityFromMessage(objRequest);
    }

    if (eType == RequestType::START || eType == RequestType::UPDATE)
    {
        SetInConference(objRequest);
    }
}

PUBLIC VIRTUAL void MtcSession::HandleResponse(
        IN ResponseType eType, IN const IMessage& objResponse)
{
    m_objExtensionSet.HandleResponse(eType, objResponse);

    if (eType == ResponseType::PROVISIONAL_RESPONSE || eType == ResponseType::EARLY_UPDATE_RESPONSE)
    {
        UpdateCallTypeFromMessage(objResponse);
        UpdateCapabilityFromMessage(objResponse);
    }

    if (objResponse.GetStatusCode() == SipStatusCode::SC_199)
    {
        m_bTerminated = IMS_TRUE;
    }

    if (eType == ResponseType::PROVISIONAL_RESPONSE || eType == ResponseType::ACCEPT_UPDATE)
    {
        SetInConference(objResponse);
    }
}

PRIVATE
ImsList<IMtcExtension*> MtcSession::GetSupportedExtensions() const
{
    ImsList<IMtcExtension*> lstExtensions;

    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_FROM_CHANGE));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_HISTORY_INFO));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_REPLACES));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
    lstExtensions.Append(new EarlyDialogTerminatedExtension());
    lstExtensions.Append(new RprExtension());

    // TODO: check CallType.
    if (!m_objContext.GetCallInfo().bUssi &&
            m_objContext.GetConfigurationProxy().Is(Feature::VOICE_QOS_PRECONDITION_SUPPORTED))
    {
        lstExtensions.Append(new PreconditionExtension());
    }

    return lstExtensions;
}

PRIVATE
void MtcSession::UpdateSessionProperty()
{
    IMS_SINT32 nInterval =
            m_objContext.GetConfigurationProxy().GetInt(Feature::SESSION_REFRESH_TRIGGER_INTERVAL);
    if (nInterval > 0)
    {
        m_objSession.SetRefreshPolicy(ISession::REFRESH_POLICY_REMAIN_TIME, 0, 0, nInterval);
    }

    m_objSession.SetConfiguration(
            m_objSession.GetConfiguration() | ISession::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED);

    m_objSession.SetImplicitRoutingRequired(IMS_TRUE);
}

PRIVATE
void MtcSession::UpdateCallTypeFromMessage(IN const IMessage& objMessage)
{
    CallType eNewCallType = MessageUtil::GetCallType(&objMessage, &m_objSession, IMS_TRUE);
    if (eNewCallType != CallType::UNKNOWN)
    {
        m_eCallType = eNewCallType;
    }

    CheckCallTypeWithRegisteredFeature();
    IMS_TRACE_D("UpdateCallTypeFromMessage : CallType[%d]", m_eCallType, 0, 0);
}

PRIVATE
void MtcSession::UpdateCapabilityFromMessage(IN const IMessage& objMessage)
{
    if (GetConfigurationProxy().Is(Feature::SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS))
    {
        m_bVideoCapable = IMS_TRUE;
    }
    else if (GetConfigurationProxy().Is(
                     Feature::CARRIER_SPECIFIC_SIP_HEADER, MessageUtil::STR_P_TTA_VOLTE_INFO))
    {
        AString strAvchange;
        MessageUtil::GetHeader(
                &objMessage, ISipHeader::UNKNOWN, strAvchange, MessageUtil::STR_P_TTA_VOLTE_INFO);
        m_bVideoCapable = strAvchange.Equals(MessageUtil::STR_AVCHANGE);
    }
    else
    {
        m_bVideoCapable = IsRegisteredFeature(ImsAosFeature::MMTEL) &&
                IsRegisteredFeature(ImsAosFeature::VIDEO) &&
                MessageUtil::IsVideoFeatureIncluded(&objMessage);
    }
    m_bRttCapable = IsRegisteredFeature(ImsAosFeature::MMTEL) &&
            IsRegisteredFeature(ImsAosFeature::TEXT) &&
            MessageUtil::IsTextFeatureIncluded(&objMessage);

    IMS_TRACE_D("UpdateCapabilityFromMessage : Video[%s] Rtt[%s]", _TRACE_B_(m_bVideoCapable),
            _TRACE_B_(m_bRttCapable), 0);
}

PRIVATE
void MtcSession::UpdateSessionIdFromMessage(IN const IMessage& objMessage)
{
    AString strSessionIdHeader;
    MessageUtil::GetHeader(
            &objMessage, ISipHeader::UNKNOWN, strSessionIdHeader, SipHeaderName::SESSION_ID);

    if (strSessionIdHeader.GetLength() <= 0)
    {
        return;
    }

    IMS_TRACE_D("UpdateSessionIdFromMessage : [%s]", m_strSessionIdHeader.GetStr(), 0, 0);
    m_strSessionIdHeader = strSessionIdHeader;
}

PRIVATE
void MtcSession::SetInConference(IN const IMessage& objMessage)
{
    if (m_objContext.GetCallInfo().bConference == IMS_TRUE)
    {
        return;
    }
    m_objContext.GetCallInfo().bConference = MessageUtil::IsFocusConf(&objMessage);
}

PRIVATE
void MtcSession::CheckCallTypeWithRegisteredFeature()
{
    IMS_BOOL bVideoFeature =
            IsRegisteredFeature(ImsAosFeature::MMTEL) && IsRegisteredFeature(ImsAosFeature::VIDEO);
    IMS_BOOL bTextFeature =
            IsRegisteredFeature(ImsAosFeature::MMTEL) && IsRegisteredFeature(ImsAosFeature::TEXT);

    if ((m_eCallType == CallType::VT && !bVideoFeature) ||
            (m_eCallType == CallType::RTT && !bTextFeature))
    {
        m_eCallType = CallType::VOIP;
    }
}

PRIVATE
MtcSession::ResultSetSdp MtcSession::SetSdpToSend(IN IMS_BOOL bAllowReOffer)
{
    // TODO: RFC 6337 instead of bAllowReOffer?
    // Need 'Method'/'Request or Response' information of the message to be sent

    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    NegotiationState eState = objMediaManager.GetNegotiationState(&m_objSession);

    if (eState == NegotiationState::STATE_OFFER_SENT)
    {
        return ResultSetSdp::NO_SDP;
    }

    if (!bAllowReOffer && eState == NegotiationState::STATE_NEGOTIATED)
    {
        IMS_TRACE_D("SetSdpToSend - nothing to update", 0, 0, 0);
        return ResultSetSdp::NO_SDP;
    }

    if (objMediaManager.FormSdp(&m_objSession, GetCallType()) == IMS_FAILURE)
    {
        IMS_TRACE_D("SetSdpToSend - Form SDP Failed", 0, 0, 0);
        return ResultSetSdp::FAILURE;
    }

    IMS_TRACE_D("SetSdpToSend - Set Done", 0, 0, 0);

    // TODO: bFailure to true for failure cases is not in this api?
    m_objContext.GetPreconditionManager().FormPreconditionSdp(&m_objSession, IMS_FALSE);

    return ResultSetSdp::SUCCESS;
}

PRIVATE
AString MtcSession::GenerateSessionId() const
{
    // Pseudo-random 128-bit system secret key
    AString strSessionId;
    strSessionId.Sprintf("%08x%08x%08x%08x", IMS_SYS_GetTimeInMicroSeconds(), IMS_SYS_GetRandom0(),
            IMS_SYS_GetRandom0(), IMS_SYS_GetRandom0());

    return strSessionId;
}

PRIVATE
IMS_BOOL MtcSession::IsRegisteredFeature(IMS_UINT32 nFeature)
{
    IMtcAosConnector* pAosConnector = GetAosConnector(GetService().GetServiceType());
    if (pAosConnector == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pAosConnector->GetFeatures() & nFeature;
}

PRIVATE
IMS_BOOL MtcSession::IsCallWaiting() const
{
    ImsList<IMtcCall*> lstCalls = m_objContext.GetCallManager().GetCalls();

    for (IMS_UINT32 nIndex = 0; nIndex < lstCalls.GetSize(); nIndex++)
    {
        IMtcCall::State eState = lstCalls.GetAt(nIndex)->GetState();
        if (eState == IMtcCall::State::ESTABLISHED || eState == IMtcCall::State::UPDATING)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcSession::IsNeedToReliable(IN IMS_BOOL bIncludeSdp) const
{
    if (!m_objExtensionSet.IsAvailableOnBoth(MtcExtensionSet::OPTION_TAG_RPR))
    {
        return IMS_FALSE;
    }

    if (m_objExtensionSet.IsRequiredOnRemote(MtcExtensionSet::OPTION_TAG_RPR))
    {
        return IMS_TRUE;
    }

    if (bIncludeSdp)
    {
        return IMS_TRUE;
    }

    if (m_objContext.GetConfigurationProxy().Is(Feature::PRACK_SUPPORTED_FOR_18X))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcSession::IsNeedToRemoveSdpInPr() const
{
    // This is only for the case of VZW equipment TC.
    if (m_objContext.GetConfigurationProxy().Is(Feature::SEND_180_FOR_INITIAL_INVITE))
    {
        IMS_TRACE_D("IsNeedToRemoveSdpInPr - VZW Test Config On", 0, 0, 0);
        if (m_objContext.GetMediaManager().GetNegotiationState(&m_objSession) ==
                STATE_OFFER_RECEIVED)
        {
            return IMS_TRUE;
        }
    }
    return IMS_FALSE;
}
