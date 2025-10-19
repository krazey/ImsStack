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

#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "IMessage.h"
#include "ISession.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsAosParameter.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "SipStatusCode.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcCallManager.h"
#include "call/MtcSession.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/PreconditionExtension.h"
#include "call/message/IMessageSender.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/IMtcAosConnector.h"
#include "helper/sipinterfaceholder/IMtcSipInterfaceFactory.h"
#include "helper/sipinterfaceholder/SessionInterfaceHolder.h"
#include "media/IMtcMediaManager.h"
#include "precondition/IMtcPreconditionManager.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"
#include <algorithm>
#include <optional>
#include <vector>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcSession::MtcSession(IN IMtcCallContext& objContext, IN ISession& objSession,
        IN CallType eCallType, IN IMessageSender* pMessageSender) :
        m_objContext(objContext),
        m_objSession(objSession),
        m_pMessageSender(pMessageSender),
        m_objExtensionSet(objContext, GetSupportedExtensions()),
        m_eCallType(eCallType),
        m_ePreviousCallType(CallType::UNKNOWN),
        m_bVideoCapable(IMS_FALSE),
        m_bRttCapable(IMS_FALSE),
        m_bTerminated(IMS_FALSE),
        m_bSessionTerminatedOrStartFailed(IMS_FALSE),
        m_bPrackPending(IMS_FALSE),
        m_eOngoingUpdateType(UpdateType::NONE),
        m_objCallTypeHistory({})
{
    IMS_TRACE_I("+MtcSession", 0, 0, 0);
    IMS_ASSERT(m_pMessageSender != IMS_NULL);

    if (m_eCallType != CallType::UNKNOWN)
    {
        SaveCallTypeHistory(m_eCallType);
    }

    if (m_objContext.GetCallInfo().ePeerType == PeerType::MT)
    {
        m_objContext.GetSipInterfaceFactory().GetISessionHolder().AddISession(
                m_objContext.GetCallKey(), &m_objSession);
    }

    UpdateSessionProperty();

    m_bVideoCapable = IsRegisteredFeature(ImsAosFeature::VIDEO);
    m_bRttCapable = IsRegisteredFeature(ImsAosFeature::TEXT);
}

PUBLIC VIRTUAL MtcSession::~MtcSession()
{
    IMS_TRACE_I("~MtcSession", 0, 0, 0);

    m_objContext.GetMediaManager().DestroyMediaForSession(&m_objSession);
    m_objContext.GetPreconditionManager().DestroyQos(&m_objSession);
    m_objSession.SetMessageMediator(IMS_NULL);
    m_objSession.SetRefreshListener(IMS_NULL);
    delete m_pMessageSender;

    m_objContext.GetSipInterfaceFactory().GetISessionHolder().ReleaseISession(
            &m_objSession, IMS_FALSE, m_bSessionTerminatedOrStartFailed);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Start()
{
    IMS_TRACE_D("Start", 0, 0, 0);

    if (SetSdpToSend(IMS_FALSE, IMS_FALSE, IMS_TRUE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatRequest(RequestType::START, *m_objSession.GetNextRequest());
    return m_pMessageSender->Start(GetCallType());
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendProvisionalResponse(
        IN IMS_BOOL bUserAlert, IN IMS_BOOL bReliable)
{
    IMS_TRACE_D("SendProvisionalResponse", 0, 0, 0);

    IMS_BOOL bIncludeSdp = bReliable;
    if (bReliable)
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
    return m_pMessageSender->SendProvisionalResponse(
            nStatusCode, bReliable, bIncludeSdp, IsCallWaiting());
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendPrack(IN IMS_BOOL bSdpOfferRequired)
{
    IMS_TRACE_D("SendPrack offer required[%s]", _TRACE_B_(bSdpOfferRequired), 0, 0);

    if (SetSdpToSend(bSdpOfferRequired) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatRequest(RequestType::PRACK, *m_objSession.GetNextRequest());
    return m_pMessageSender->SendPrack();
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::RespondToPrack(IN IMS_SINT32 eStatusCode)
{
    IMS_TRACE_D("RespondToPrack", 0, 0, 0);

    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatResponse(ResponseType::PRACK_RESPONSE, *m_objSession.GetNextResponse());
    return m_pMessageSender->RespondToPrack(eStatusCode);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendEarlyUpdate(IN UpdateType eUpdateType)
{
    IMS_TRACE_D("SendEarlyUpdate", 0, 0, 0);

    if (SetSdpToSend(IMS_TRUE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_eOngoingUpdateType = eUpdateType;
    m_objExtensionSet.FormatRequest(RequestType::EARLY_UPDATE, *m_objSession.GetNextRequest());
    return m_pMessageSender->SendEarlyUpdate(eUpdateType);
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
    return m_pMessageSender->RespondToEarlyUpdate(eStatusCode);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::SendAck()
{
    IMS_TRACE_D("SendAck", 0, 0, 0);

    if (SetSdpToSend(IMS_FALSE) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatRequest(RequestType::ACK, *m_objSession.GetNextRequest());
    return m_pMessageSender->SendAck();
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
    return m_pMessageSender->Accept();
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Reject(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("Reject", 0, 0, 0);

    m_objExtensionSet.FormatResponse(ResponseType::REJECT, *m_objSession.GetNextResponse());
    return m_pMessageSender->Reject(objReason);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::Update(
        IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo, IN IMS_SINT32 eMethod)
{
    IMS_TRACE_D("Update", 0, 0, 0);

    if (SetSdpToSend(UpdateType::LOCATION != eUpdateType) == ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatRequest(RequestType::UPDATE, *m_objSession.GetNextRequest());
    return m_pMessageSender->Update(
            eUpdateType, bIncludeAlertInfo, eMethod, eUpdateType == UpdateType::REFRESH);
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::AcceptUpdate()
{
    const IMessage* piMessage = m_objSession.GetPreviousRequest(IMessage::SESSION_UPDATE);
    IMS_BOOL bAnswerForOfferlessReInvite = !m_objContext.GetMessageUtils().HasSdp(piMessage) &&
            piMessage->GetMethod().Equals(SipMethod::INVITE);
    IMS_TRACE_D("AcceptUpdate Offerless case[%s]", _TRACE_B_(bAnswerForOfferlessReInvite), 0, 0);

    // bAnswerForOfferlessReInvite should allow re-offer.
    if (SetSdpToSend(bAnswerForOfferlessReInvite, bAnswerForOfferlessReInvite) ==
            ResultSetSdp::FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objExtensionSet.FormatResponse(ResponseType::ACCEPT_UPDATE, *m_objSession.GetNextResponse());
    return m_pMessageSender->AcceptUpdate();
}

PUBLIC VIRTUAL IMS_RESULT MtcSession::CancelUpdate(IN const CallReasonInfo& objReason)
{
    IMS_TRACE_D("CancelUpdate", 0, 0, 0);

    m_objExtensionSet.FormatRequest(RequestType::CANCEL_UPDATE, *m_objSession.GetNextRequest());
    return m_pMessageSender->CancelUpdate(objReason);
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

    if (CallReasonInfo::IsTerminateRequired(objReason.nCode))
    {
        m_objExtensionSet.FormatRequest(RequestType::TERMINATE, *m_objSession.GetNextRequest());
        return m_pMessageSender->Terminate(bUseBye, objReason);
    }
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL void MtcSession::HandleRequest(IN RequestType eType, IN const IMessage& objRequest)
{
    m_objExtensionSet.HandleRequest(eType, objRequest);

    switch (eType)
    {
        case RequestType::START:
            if (m_objContext.GetMessageUtils().HasSdp(&objRequest))
            {
                UpdateCallTypeFromMessage(objRequest, IMS_FALSE);
            }
            else
            {
                SetCallType(GetCallTypeForOfferlessInvite());
            }

            SetInConference(objRequest);
            break;

        case RequestType::PRACK:
        case RequestType::EARLY_UPDATE:
        case RequestType::ACK:
            UpdateCallTypeFromMessage(objRequest, IMS_TRUE);
            break;

        case RequestType::UPDATE:
            if (m_objContext.GetMessageUtils().HasSdp(&objRequest))
            {
                UpdateCallTypeFromMessage(objRequest, IMS_FALSE);
            }
            else
            {
                SetCallType(GetCallTypeForOfferlessReInvite());
            }

            SetInConference(objRequest);
            break;

        default:
            break;
    }

    UpdateCapabilityFromMessage(objRequest);
}

PUBLIC VIRTUAL void MtcSession::HandleResponse(
        IN ResponseType eType, IN const IMessage& objResponse)
{
    m_objExtensionSet.HandleResponse(eType, objResponse);

    if (eType == ResponseType::REJECT)
    {
        return;
    }

    UpdateCallTypeFromMessage(objResponse, IMS_TRUE);
    UpdateCapabilityFromMessage(objResponse);

    if (objResponse.GetStatusCode() == SipStatusCode::SC_183 &&
            objResponse.GetMessage()->IsMessageRpr())
    {
        m_bPrackPending = IMS_TRUE;
    }

    if (eType == ResponseType::PRACK_RESPONSE)
    {
        m_bPrackPending = IMS_FALSE;
    }

    if (eType == ResponseType::EARLY_UPDATE_RESPONSE &&
            objResponse.GetStatusCode() == SipStatusCode::SC_200)
    {
        m_eOngoingUpdateType = UpdateType::NONE;
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

PUBLIC VIRTUAL void MtcSession::SetCallType(IN CallType eNewCallType)
{
    IMS_TRACE_D("SetCallType [%d] -> [%d]", m_eCallType, eNewCallType, 0);
    m_ePreviousCallType = m_eCallType;
    m_eCallType = eNewCallType;
    SaveCallTypeHistory(m_eCallType);
}

PRIVATE
ImsList<IMtcExtension*> MtcSession::GetSupportedExtensions() const
{
    ImsList<IMtcExtension*> lstExtensions;

    lstExtensions.Append(new MtcExtension(m_objContext, MtcExtensionSet::OPTION_TAG_FROM_CHANGE,
            {RequestType::START}, {ResponseType::PROVISIONAL_RESPONSE, ResponseType::ACCEPT}));
    lstExtensions.Append(new MtcExtension(
            m_objContext, MtcExtensionSet::OPTION_TAG_HISTORY_INFO, {RequestType::START}, {}));
    lstExtensions.Append(
            new MtcExtension(m_objContext, MtcExtensionSet::OPTION_TAG_REPLACES,
            {RequestType::START}, {ResponseType::PROVISIONAL_RESPONSE}));
    lstExtensions.Append(new MtcExtension(m_objContext, MtcExtensionSet::OPTION_TAG_TARGET_DIALOG,
            {RequestType::START}, {ResponseType::ACCEPT}));
    lstExtensions.Append(new MtcExtension(m_objContext,
            MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED, {RequestType::START}, {}));
    lstExtensions.Append(new MtcExtension(m_objContext, MtcExtensionSet::OPTION_TAG_RPR,
            {RequestType::START, RequestType::UPDATE}, {ResponseType::PROVISIONAL_RESPONSE}));
    lstExtensions.Append(
            new MtcExtension(m_objContext, MtcExtensionSet::OPTION_TAG_SESSION_TIMER, {}, {}));

    // TODO: check CallType.
    if (!m_objContext.GetCallInfo().bUssi &&
            m_objContext.GetConfigurationProxy().GetBoolean(
                    ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
    {
        lstExtensions.Append(new PreconditionExtension(m_objContext));
    }

    return lstExtensions;
}

PRIVATE
void MtcSession::UpdateSessionProperty()
{
    IMS_SINT32 nInterval = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_SESSION_REFRESH_TRIGGER_INTERVAL_SEC_INT);
    if (nInterval > 0)
    {
        m_objSession.SetRefreshPolicy(ISession::REFRESH_POLICY_REMAIN_TIME, 0, 0, nInterval);
    }

    m_objSession.SetConfiguration(
            m_objSession.GetConfiguration() | ISession::CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED);

    m_objSession.SetImplicitRoutingRequired(IMS_TRUE);
}

PRIVATE
IMS_RESULT MtcSession::UpdateCallTypeFromMessage(
        IN const IMessage& objMessage, IN IMS_BOOL bSkipSameType)
{
    CallType eCallTypeOfMessage =
            m_objContext.GetMessageUtils().GetCallType(&objMessage, &m_objSession, IMS_TRUE);

    if (bSkipSameType && eCallTypeOfMessage == m_eCallType)
    {
        return IMS_SUCCESS;
    }

    if (eCallTypeOfMessage != CallType::UNKNOWN)
    {
        SetCallType(RestrictCallTypeByRegisteredFeature(eCallTypeOfMessage));
        return IMS_SUCCESS;
    }

    return IMS_FAILURE;
}

PRIVATE
void MtcSession::UpdateCapabilityFromMessage(IN const IMessage& objMessage)
{
    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVt::KEY_SUPPORT_VIDEO_CALL_UPGRADE_REGARDLESS_OF_FEATURE_TAGS_BOOL))
    {
        m_bVideoCapable = IMS_TRUE;
    }
    else if (m_objContext.GetConfigurationProxy().Contains(
                     ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                     MessageUtil::STR_P_TTA_VOLTE_INFO))
    {
        AString strAvchange = m_objContext.GetMessageUtils().GetHeader(
                &objMessage, ISipHeader::UNKNOWN, MessageUtil::STR_P_TTA_VOLTE_INFO);
        m_bVideoCapable = strAvchange.Equals(MessageUtil::STR_AVCHANGE);
    }
    else
    {
        if (auto bFeature = m_objContext.GetMessageUtils().IsVideoFeatureIncluded(&objMessage))
        {
            m_bVideoCapable = *bFeature && IsRegisteredFeature(ImsAosFeature::VIDEO);
        }
    }

    if (auto bFeature = m_objContext.GetMessageUtils().IsTextFeatureIncluded(&objMessage))
    {
        m_bRttCapable = *bFeature && IsRegisteredFeature(ImsAosFeature::TEXT);
    }

    IMS_TRACE_D("UpdateCapabilityFromMessage : Video[%s] RTT[%s]", _TRACE_B_(m_bVideoCapable),
            _TRACE_B_(m_bRttCapable), 0);
}

PRIVATE
void MtcSession::SetInConference(IN const IMessage& objMessage)
{
    if (m_objContext.GetCallInfo().bConference == IMS_TRUE)
    {
        return;
    }
    m_objContext.GetCallInfo().bConference =
            m_objContext.GetMessageUtils().IsFocusConf(&objMessage);
}

PRIVATE
CallType MtcSession::RestrictCallTypeByRegisteredFeature(IN CallType& eCallType)
{
    IMS_BOOL bVideoFeature = IsRegisteredFeature(ImsAosFeature::VIDEO);
    IMS_BOOL bTextFeature = IsRegisteredFeature(ImsAosFeature::TEXT);

    if ((eCallType == CallType::VT && !bVideoFeature) ||
            (eCallType == CallType::RTT && !bTextFeature))
    {
        return CallType::VOIP;
    }

    return eCallType;
}

PRIVATE
CallType MtcSession::GetCallTypeForOfferlessInvite()
{
    IMS_SINT32 eMediaType = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_INVITE_INT);
    if (eMediaType == ConfigVoice::OFFERLESS_INVITE_MEDIA_TYPE_FULL_CAPABILITY)
    {
        return GetCallTypeByRegisteredFeature();
    }
    else  // ConfigVoice::OFFERLESS_INVITE_MEDIA_TYPE_AUDIO
    {
        return CallType::VOIP;
    }
}

PRIVATE
CallType MtcSession::GetCallTypeForOfferlessReInvite()
{
    IMS_SINT32 eMediaType = m_objContext.GetConfigurationProxy().GetInt(
            ConfigVoice::KEY_MEDIA_TYPE_FOR_OFFERLESS_REINVITE_INT);
    switch (eMediaType)
    {
        case ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_FULL:
            return GetCallTypeByRegisteredFeature();
        case ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_AUDIO:
            return CallType::VOIP;
        case ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_CURRENT:
            return m_eCallType;
        case ConfigVoice::OFFERLESS_REINVITE_MEDIA_TYPE_BY_HISTORY:
            return GetCallTypeByHistory();
        default:  // OFFERLESS_REINVITE_MEDIA_TYPE_INITIALLY_OFFERED
            return MayGetFirstCallType();
    }
}

PRIVATE
CallType MtcSession::GetCallTypeByRegisteredFeature()
{
    IMS_BOOL bVideoFeature = IsRegisteredFeature(ImsAosFeature::VIDEO);
    IMS_BOOL bTextFeature = IsRegisteredFeature(ImsAosFeature::TEXT);

    if (bVideoFeature && !bTextFeature)
    {
        return CallType::VT;
    }
    else if (!bVideoFeature && bTextFeature)
    {
        return CallType::RTT;
    }
    else if (bVideoFeature && bTextFeature)
    {
        // Video && RTT
        IMS_SINT32 nPolicyForTextAndVideo = m_objContext.GetConfigurationProxy().GetInt(
                ConfigVt::KEY_POLICY_FOR_TEXT_WITH_VIDEO_INT);
        if (nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_NOT_ALLOWED ||
                nPolicyForTextAndVideo == ConfigVt::TEXT_VIDEO_NOT_ALLOWED_IF_ACTIVE)
        {
            return CallType::VT;
        }
        else
        {
            // TEXT_VIDEO_ALLOWED
            return CallType::VIDEO_RTT;
        }
    }

    return CallType::VOIP;
}

PRIVATE
CallType MtcSession::MayGetFirstCallType()
{
    auto pCallType = std::find_if(m_objCallTypeHistory.begin(), m_objCallTypeHistory.end(),
            [](CallType callType)
            {
                return callType != CallType::UNKNOWN;
            });

    return (pCallType != m_objCallTypeHistory.end()) ? *pCallType : CallType::VOIP;
}

PRIVATE
CallType MtcSession::GetCallTypeByHistory()
{
    std::vector<CallType> objCallTypesInPriority{
            CallType::VIDEO_RTT, CallType::VT, CallType::RTT, CallType::VOIP};
    auto pCallType = std::find_if(objCallTypesInPriority.begin(), objCallTypesInPriority.end(),
            [this](CallType eType)
            {
                return IsInHistory(eType);
            });

    return (pCallType != objCallTypesInPriority.end()) ? *pCallType : CallType::UNKNOWN;
}

PRIVATE
MtcSession::ResultSetSdp MtcSession::SetSdpToSend(IN IMS_BOOL bAllowReOffer,
        IN IMS_BOOL bAnswerForOfferlessReInvite /* = IMS_FALSE*/,
        IN IMS_BOOL bInitialInvite /* = IMS_FALSE */)
{
    // Answering for offerless re-INVITE case must not come into this.
    IMtcMediaManager& objMediaManager = m_objContext.GetMediaManager();
    NegotiationState eState = objMediaManager.GetNegotiationState(&m_objSession);

    if (eState == NegotiationState::STATE_OFFER_SENT)
    {
        IMS_TRACE_D("SetSdpToSend - already STATE_OFFER_SENT", 0, 0, 0);
        return ResultSetSdp::NO_SDP;
    }

    if (!bAllowReOffer && eState == NegotiationState::STATE_NEGOTIATED)
    {
        IMS_TRACE_D("SetSdpToSend - nothing to update", 0, 0, 0);
        return ResultSetSdp::NO_SDP;
    }

    if (objMediaManager.FormSdp(&m_objSession, GetCallType(), bAnswerForOfferlessReInvite) ==
            IMS_FAILURE)
    {
        IMS_TRACE_D("SetSdpToSend - Form SDP Failed", 0, 0, 0);
        return ResultSetSdp::FAILURE;
    }

    IMS_TRACE_D("SetSdpToSend - Set Done", 0, 0, 0);

    IMtcPreconditionManager& objPreconditionManager = m_objContext.GetPreconditionManager();
    // TODO: bFailure to true for failure cases is not in this api?
    objPreconditionManager.FormPreconditionSdp(&m_objSession, IMS_FALSE);
    objPreconditionManager.OnSdpSent(&m_objSession, bInitialInvite);

    return ResultSetSdp::SUCCESS;
}

PRIVATE
IMS_BOOL MtcSession::IsRegisteredFeature(IMS_UINT32 nFeature) const
{
    const IMtcAosConnector* pAosConnector =
            m_objContext.GetAosConnector(m_objContext.GetService().GetServiceType());
    if (pAosConnector == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return pAosConnector->GetFeatures() & nFeature;
}

PRIVATE
IMS_BOOL MtcSession::IsCallWaiting() const
{
    if (m_objContext.GetCall().GetState() != IMtcCall::State::IDLE &&
            m_objContext.GetCall().GetState() != IMtcCall::State::INCOMING &&
            m_objContext.GetCall().GetState() != IMtcCall::State::ALERTING)
    {
        return IMS_FALSE;
    }

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
IMS_BOOL MtcSession::IsInHistory(IN CallType eCallType) const
{
    return std::find(m_objCallTypeHistory.begin(), m_objCallTypeHistory.end(), eCallType) !=
            m_objCallTypeHistory.end();
}

PRIVATE
void MtcSession::SaveCallTypeHistory(IN CallType eCallType)
{
    if (!IsInHistory(eCallType))
    {
        IMS_TRACE_D("SaveCallTypeHistory [%d]", eCallType, 0, 0);
        m_objCallTypeHistory.push_back(eCallType);
    }
}
