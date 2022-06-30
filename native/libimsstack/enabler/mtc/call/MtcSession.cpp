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
#include "call/MtcSession.h"
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
        m_objMessageSender(MessageSender(*this)),
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
    GetSipInterfaceFactory().GetISessionHolder()->ReleaseISession(&m_objSession);
}

PUBLIC IMS_RESULT MtcSession::Start()
{
    if (m_objContext.GetMediaManager().FormSdp(&m_objSession, m_eCallType) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    m_objContext.GetPreconditionManager().FormPreconditionSdp(&m_objSession, IMS_FALSE);

    return m_objMessageSender.Start();
}

PUBLIC IMS_RESULT MtcSession::Terminate(IMS_BOOL bUseBye, IN const CallReasonInfo& objReason)
{
    if (m_bTerminated)
    {
        return IMS_FAILURE;
    }

    m_bTerminated = IMS_TRUE;
    return m_objMessageSender.Terminate(bUseBye, objReason);
}

PUBLIC
void MtcSession::HandleRequest(IN IMS_UINT32 nMethod, IN const IMessage& objRequest)
{
    m_objExtensionSet.HandleRequest(nMethod, objRequest);

    if (nMethod == IMessage::SESSION_START)
    {
        if (GetConfigurationProxy().Is(Feature::SUPPORT_SIP_SESSION_ID_HEADER))
        {
            UpdateSessionIdFromMessage(objRequest);
        }
    }

    if (nMethod == IMessage::SESSION_START || nMethod == IMessage::SESSION_EARLY_UPDATE)
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

    if (nMethod == IMessage::SESSION_START || nMethod == IMessage::SESSION_UPDATE)
    {
        SetInConference(objRequest);
    }
}

PUBLIC
void MtcSession::HandleResponse(IN IMS_UINT32 nMethod, IN const IMessage& objResponse)
{
    m_objExtensionSet.HandleResponse(nMethod, objResponse);

    if (nMethod == IMessage::SESSION_START || nMethod == IMessage::SESSION_EARLY_UPDATE)
    {
        UpdateCallTypeFromMessage(objResponse);
        UpdateCapabilityFromMessage(objResponse);
    }

    if (objResponse.GetStatusCode() == SipStatusCode::SC_199)
    {
        m_bTerminated = IMS_TRUE;
    }

    if (nMethod == IMessage::SESSION_START || nMethod == IMessage::SESSION_UPDATE)
    {
        SetInConference(objResponse);
    }
}

PRIVATE
ImsList<IMtcExtension*> MtcSession::GetSupportedExtensions() const
{
    ImsList<IMtcExtension*> lstExtensions;

    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_FROM_CHANGE));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_HISTORY_INFO));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_REPLACES));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_TARGET_DIALOG));
    lstExtensions.Append(new MtcExtension(MtcExtensionSet::OPTION_TAG_TIMER));
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
