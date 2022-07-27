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

#include "call/extension/MtcExtensionSet.h"
#include "call/IMtcCall.h"
#include "call/IMtcSessionContext.h"
#include "call/message/MessageFormatter.h"
#include "CallReasonInfo.h"
#include "CarrierConfig.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "Const3GPP.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "IMessage.h"
#include "ImsIdentity.h"
#include "ISession.h"
#include "ISipConfig.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipAddress.h"
#include "SipHeaderName.h"
#include "SipProfile.h"
#include "SipStatusCode.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
MessageFormatter::MessageFormatter(IN IMtcSessionContext& objContext, IN ISession& objSession) :
        m_objContext(objContext),
        m_objSession(objSession),
        m_piNextMessage(IMS_NULL),
        m_eFormType(FormType::NONE)
{
    IMS_TRACE_I("+MessageFormatter", 0, 0, 0);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL MessageFormatter::~MessageFormatter()
{
    IMS_TRACE_I("~MessageFormatter", 0, 0, 0);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormStartMessage()
{
    if (InitVariables(FormType::START) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetAcceptContactHeader();
    SetAcceptHeader();
    SetPPreferredServiceHeader();
    AddSrvccFeature();
    // SetKeepAliveProfile();
    SetCallerIdHeader();
    // SetTipHeader();
    SetSupportedHeader();
    SetPreconditionHeader();
    SetPEarlyMediaHeader();
    SetCarrierSpecificHeaders();

    if (m_objContext.GetConfigurationProxy().Is(Feature::MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF,
                static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::INVITE)))
    {
        SetLocation();
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormProvisionalResponseMessage(
        IN IMS_BOOL bIncludeAlertInfo)
{
    if (InitVariables(FormType::PROVISIONAL_RESPONSE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetAlertInfoHeader(bIncludeAlertInfo);
    SetPreconditionHeader();
    AddSrvccFeature();
    // SetTipHeader();

    if (m_objContext.GetConfigurationProxy().Is(Feature::MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF,
                static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::PROVISIONAL_RESPONSE)))
    {
        SetLocation();
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormPrackMessage()
{
    if (InitVariables(FormType::PRACK) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormPrackResponseMessage()
{
    if (InitVariables(FormType::PRACK_RESPONSE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetPreconditionHeader();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormEarlyUpdateMessage(IN UpdateType eUpdateType)
{
    if (InitVariables(FormType::EARLY_UPDATE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    AString strReason;
    GetUpdateReason(eUpdateType, strReason);
    SetReasonHeader(strReason);
    SetPreconditionHeader();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormEarlyUpdateResponseMessage()
{
    if (InitVariables(FormType::EARLY_UPDATE_RESPONSE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetPreconditionHeader();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormAcceptMessage()
{
    if (InitVariables(FormType::ACCEPT) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetPreconditionHeader();
    SetSrvccContactParameter();
    // SetTipHeader();
    SetCarrierSpecificHeaders();

    if (m_objContext.GetConfigurationProxy().Is(Feature::MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF,
                static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::FINAL_SUCCESS_RESPONSE)))
    {
        SetLocation();
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormRejectMessage(
        IN const CallReasonInfo& objReason, OUT IMS_SINT32& eStatusCode, OUT AString& strPhrase)
{
    if (InitVariables(FormType::REJECT) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    eStatusCode = GetRejectStatusCode(objReason);
    GetRejectPhrase(objReason, strPhrase);

    // If PIDF-LO shouldn't be added to reject messages for re-INVITE, need a fix
    if (m_objContext.GetConfigurationProxy().Is(Feature::MESSAGE_TYPE_SUPPORT_GEOLOCATION_PIDF,
                static_cast<IMS_SINT32>(MessageTypeForGeolocationPidf::FINAL_FAILURE_RESPONSE)))
    {
        SetLocation();
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormAckMessage()
{
    if (InitVariables(FormType::ACK) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormUpdateMessage(
        IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo)
{
    if (InitVariables(FormType::UPDATE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    AString strReason;
    GetUpdateReason(eUpdateType, strReason);
    SetReasonHeader(strReason);
    SetAlertInfoHeader(bIncludeAlertInfo);
    SetSupportedHeader();
    SetPreconditionHeader();
    SetCarrierSpecificHeaders();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormAcceptUpdateMessage()
{
    if (InitVariables(FormType::ACCEPT_UPDATE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetPreconditionHeader();
    SetCarrierSpecificHeaders();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormCancelUpdateMessage(
        IN const CallReasonInfo& objReason)
{
    if (InitVariables(FormType::CANCEL_UPDATE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    AString strReason;
    GetTerminateReason(objReason, strReason);
    SetReasonHeader(strReason);

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormTerminateMessage(IN const CallReasonInfo& objReason)
{
    if (InitVariables(FormType::TERMINATE) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    AString strReason;
    GetTerminateReason(objReason, strReason);
    SetReasonHeader(strReason);

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PROTECTED VIRTUAL void MessageFormatter::SetLocation()
{
    if (!MtcLocationObject::IsGeolocationInfoRequired(m_objContext))
    {
        IMS_TRACE_D("Geolocation Info is not required", 0, 0, 0);
        return;
    }

    MtcLocationObject(m_objContext).SetLocationToMessage(*m_piNextMessage);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PROTECTED
ICoreService* MessageFormatter::GetICoreService()
{
    return m_objContext.GetService().GetICoreService();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PROTECTED
IFeatureCaps* MessageFormatter::GetIFeatureCaps()
{
    ICoreService* piCoreService = GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        return IMS_NULL;
    }

    return piCoreService->GetFeatureCaps();
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetPPreferredServiceHeader()
{
    MessageUtil::SetHeader(m_piNextMessage, Const3GPP::ICSI_MMTEL, ISipHeader::UNKNOWN,
            SipHeaderName::P_PREFERRED_SERVICE);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetAcceptContactHeader()
{
    AString strAcceptContact;
    strAcceptContact.Append(TextParser::CHAR_ASTERISK);
    strAcceptContact.Append(TextParser::CHAR_SEMICOLON);

    strAcceptContact.Append(MessageUtil::STR_ICSI);
    strAcceptContact.Append(TextParser::CHAR_DQUOT);
    strAcceptContact.Append(AString(Const3GPP::ICSI_MMTEL).Replace(":", "%3A"));
    strAcceptContact.Append(TextParser::CHAR_DQUOT);

    if (m_objContext.GetCallType() == CallType::VT ||
            m_objContext.GetCallType() == CallType::VIDEO_RTT)
    {
        strAcceptContact.Append(TextParser::CHAR_SEMICOLON);
        strAcceptContact.Append(MessageUtil::STR_VIDEO);
    }

    MessageUtil::SetHeader(m_piNextMessage, strAcceptContact, ISipHeader::ACCEPT_CONTACT);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetAcceptHeader()
{
    MessageUtil::AddValueIfNotExists(
            m_piNextMessage, MessageUtil::STR_ACCEPT_TYPE_APPLICATION_SDP, ISipHeader::ACCEPT);
    MessageUtil::AddValueIfNotExists(m_piNextMessage,
            MessageUtil::STR_ACCEPT_TYPE_APPLICATION_3GPP_IMS_XML, ISipHeader::ACCEPT);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::AddSrvccFeature()
{
    IFeatureCaps* piFeatureCaps = GetIFeatureCaps();
    if (piFeatureCaps == IMS_NULL)
    {
        return;
    }

    if (m_eFormType == FormType::START)
    {
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_A, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_REQUEST);
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_B, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_REQUEST);
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_M, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_REQUEST);

        return;
    }

    IMessage* piPreviousMessage = m_objSession.GetPreviousRequest(IMessage::SESSION_START);
    if (piPreviousMessage == IMS_NULL)
    {
        return;
    }

    if (MessageUtil::ContainsValue(piPreviousMessage, MessageUtil::STR_SRVCC_FEATURE_A,
                ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_A, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_REQUEST);
    }

    if (MessageUtil::ContainsValue(piPreviousMessage, MessageUtil::STR_SRVCC_FEATURE_B,
                ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_B, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_REQUEST);
    }

    if (MessageUtil::ContainsValue(piPreviousMessage, MessageUtil::STR_SRVCC_FEATURE_M,
                ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_M, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_REQUEST);
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetSrvccContactParameter()
{
    IMessage* piPreviousMessage = m_objSession.GetPreviousRequest(IMessage::SESSION_START);
    if (piPreviousMessage == IMS_NULL)
    {
        return;
    }

    if (MessageUtil::ContainsValue(piPreviousMessage, MessageUtil::STR_SRVCC_FEATURE_A,
                ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        m_objSession.SetContactParameter(MessageUtil::STR_SRVCC_FEATURE_A, 0);
    }

    if (MessageUtil::ContainsValue(piPreviousMessage, MessageUtil::STR_SRVCC_FEATURE_B,
                ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        m_objSession.SetContactParameter(MessageUtil::STR_SRVCC_FEATURE_B, 0);
    }

    if (MessageUtil::ContainsValue(piPreviousMessage, MessageUtil::STR_SRVCC_FEATURE_M,
                ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        m_objSession.SetContactParameter(MessageUtil::STR_SRVCC_FEATURE_M, 0);
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
// PRIVATE
// void MessageFormatter::SetKeepAliveProfile()
// {
//     IMS_BOOL bKeepAlive = IMS_FALSE;  // TODO, SESSION_SP_KEEP_ALIVE

//     if (!bKeepAlive)
//     {
//         return;
//     }

//     ICoreService* piCoreService = GetICoreService();
//     if (piCoreService == IMS_NULL)
//     {
//         return;
//     }

//     IMS_UINT32 eSipFeatures = ISipConfig::SIP_FEATURE_CAPS_KEEP;
//     RcPtr<SipProfile> pProfile = new SipProfile();

//     pProfile->SetSipFeatures(eSipFeatures);
//     piCoreService->SetSipProfile(pProfile.Get());
// }

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetCallerIdHeader()
{
    const SuppService* pSuppService =
            m_objContext.GetSupplementaryService().Get(SuppType::CALLER_ID);

    if (pSuppService == IMS_NULL)
    {
        return;
    }

    if (pSuppService->nValue == CALLERID_RESTRICTED)
    {
        MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
        if (objConfig.GetInt(Feature::SESSION_PRIVACY_TYPE) ==
                CarrierConfig::ImsVoice::SESSION_PRIVACY_TYPE_HEADER)
        {
            MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_HEADER, ISipHeader::PRIVACY);
        }
        else
        {
            MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_ID, ISipHeader::PRIVACY);
        }

        SipAddress objSIPAddress(ImsIdentity::GetAnonymousUserId());
        objSIPAddress.SetDisplayName(MessageUtil::STR_ANONYMOUS);
        MessageUtil::SetHeader(m_piNextMessage, objSIPAddress.ToString(), ISipHeader::FROM);
    }
    else if (pSuppService->nValue == CALLERID_IDENTITY)
    {
        MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_NONE, ISipHeader::PRIVACY);
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
// PRIVATE
// void MessageFormatter::SetTipHeader()
// {
//     IMS_BOOL bTip = IMS_FALSE;  // TODO, SESSION_SP_TIP
//     if (!bTip)
//     {
//         return;
//     }

//     IMS_BOOL bAddTagToSupported = IMS_FALSE;
//     if (m_eFormType == FormType::START)
//     {
//         bAddTagToSupported = IMS_TRUE;
//     }
//     else if ((m_eFormType == FormType::PROVISIONAL_RESPONSE) ||
//             (m_eFormType == FormType::ACCEPT))
//     {
//         IMessage* piPreviousMessage = m_objSession.GetPreviousRequest(IMessage::SESSION_START);
//         if (MessageUtil::HasValue(piPreviousMessage, Sip::STR_FROM_CHANGE,
//                 ISipHeader::SUPPORTED))
//         {
//             bAddTagToSupported = IMS_TRUE;
//         }
//     }

//     if (bAddTagToSupported)
//     {
//         MessageUtil::AddValueIfNotExists(
//                 m_piNextMessage, Sip::STR_FROM_CHANGE, ISipHeader::SUPPORTED);
//     }

//     if (m_eFormType != FormType::PROVISIONAL_RESPONSE)
//     {
//         return;
//     }

//     IMS_SINT32 eTipMode = TIP_MODE_TEMPORARY;  // TODO, SESSION_SP_TIP_MODE
//     if (eTipMode != TIP_MODE_TEMPORARY)
//     {
//         return;
//     }

//     IMS_SINT32 eTipType = TIP_TYPE_IDENTITY;  // TODO, from user settings
//     if (eTipType == TIP_TYPE_RESTRICTED)
//     {
//         MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_ID, ISipHeader::PRIVACY);
//     }
//     else if (eTipType == TIP_TYPE_IDENTITY)
//     {
//         MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_NONE, ISipHeader::PRIVACY);
//     }
// }

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetSupportedHeader()
{
    MessageUtil::AddValueIfNotExists(m_piNextMessage, Sip::STR_100REL, ISipHeader::SUPPORTED);

    if (m_eFormType != FormType::START)
    {
        return;
    }

    MessageUtil::AddValueIfNotExists(m_piNextMessage, MessageUtil::STR_199, ISipHeader::SUPPORTED);

    // IMS_BOOL bHistoryInfo = IMS_FALSE;  // TODO
    // if (bHistoryInfo)
    // {
    //     MessageUtil::AddValueIfNotExists(
    //             m_piNextMessage, MessageUtil::STR_HISTINFO, ISipHeader::SUPPORTED);
    // }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetPreconditionHeader()
{
    if (m_eFormType != FormType::START &&
            !m_objContext.GetExtensionSet().IsAvailableOnBoth(
                    MtcExtensionSet::OPTION_TAG_PRECONDITION))
    {
        return;
    }

    IMS_BOOL bInclude = IMS_FALSE;
    IMS_SINT32 eHeaderType = ISipHeader::SUPPORTED;

    switch (m_eFormType)
    {
        case FormType::START:
        {
            bInclude = m_objContext.GetExtensionSet().IsAvailableOnLocal(
                    MtcExtensionSet::OPTION_TAG_PRECONDITION);
        }
        break;
        case FormType::EARLY_UPDATE:
        {
            bInclude = IMS_TRUE;
            eHeaderType = ISipHeader::REQUIRE;  // TODO, B_SEND_UPDATE_WITH_REQUIRE_PRECONDITION
        }
        break;
        case FormType::UPDATE:
        {
            bInclude = IMS_TRUE;  // TODO, B_PRECONDITION_SUPPORTED_IN_REINVITE
        }
        break;
        case FormType::PROVISIONAL_RESPONSE:   // FALL_THROUGH
        case FormType::PRACK_RESPONSE:         // FALL_THROUGH
        case FormType::EARLY_UPDATE_RESPONSE:  // FALL_THROUGH
        case FormType::ACCEPT:
        {
            bInclude = IMS_TRUE;
            eHeaderType = ISipHeader::REQUIRE;
        }
        break;
        case FormType::ACCEPT_UPDATE:
        {
            bInclude = IMS_TRUE;  // TODO, check condition
            eHeaderType = ISipHeader::REQUIRE;
        }
        break;
        default:
            break;
    }

    if (!bInclude)
    {
        return;
    }

    MessageUtil::AddValueIfNotExists(m_piNextMessage, MessageUtil::STR_PRECONDITION, eHeaderType);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetPEarlyMediaHeader()
{
    if (m_objContext.GetCallInfo().bUssi)
    {
        return;
    }

    MessageUtil::AddValueIfNotExists(
            m_piNextMessage, MessageUtil::STR_SUPPORTED, ISipHeader::P_EARLY_MEDIA);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetAlertInfoHeader(IN IMS_BOOL bIncludeAlertInfo)
{
    if (!bIncludeAlertInfo)
    {
        return;
    }

    MessageUtil::AddValueIfNotExists(m_piNextMessage, MessageUtil::STR_ALERT_URN_CALL_WAITING,
            ISipHeader::UNKNOWN, SipHeaderName::ALERT_INFO);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetReasonHeader(IN CONST AString& strReason)
{
    if (strReason.GetLength() < 1)
    {
        return;
    }

    MessageUtil::AddValueIfNotExists(
            m_piNextMessage, strReason, ISipHeader::UNKNOWN, SipHeaderName::REASON);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetCarrierSpecificHeaders()
{
    // TODO: add SetCarrierSpecificHeaders() to all message depends on the requirements
    MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
    if (objConfig.Is(Feature::CARRIER_SPECIFIC_SIP_HEADER, MessageUtil::STR_P_TTA_VOLTE_INFO))
    {
        if (m_eFormType == FormType::START || m_eFormType == FormType::ACCEPT ||
                m_eFormType == FormType::UPDATE || m_eFormType == FormType::ACCEPT_UPDATE)
        {
            IMS_TRACE_D("SetCarrierSpecificHeaders : avchange", 0, 0, 0);
            MessageUtil::AddValueIfNotExists(m_piNextMessage, MessageUtil::STR_AVCHANGE,
                    ISipHeader::UNKNOWN, MessageUtil::STR_P_TTA_VOLTE_INFO);
        }
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
IMS_SINT32 MessageFormatter::GetRejectStatusCode(IN const CallReasonInfo& objReason)
{
    IMS_SINT32 eStatusCode = SipStatusCode::SC_INVALID;

    switch (objReason.nCode)
    {
        case CODE_UNSPECIFIED:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case CODE_USER_DECLINE:
            eStatusCode = m_objContext.GetConfigurationProxy().GetInt(
                    Feature::INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE);
            break;
        case CODE_USER_NOANSWER:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case CODE_LOW_BATTERY:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case CODE_LOCAL_CALL_END_UNSPECIFIED:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case CODE_REJECT_ONGOING_CALL_WAITING_DISABLED:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_LOCAL_SERVICE_UNAVAILABLE:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case CODE_REJECT_VT_TTY_NOT_ALLOWED:
            eStatusCode = SipStatusCode::SC_480;
            break;

        case CODE_REJECT_ONGOING_CS_CALL:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_REJECT_ONGOING_E911_CALL:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_REJECT_CALL_ON_OTHER_SUB:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_REJECT_ONGOING_CALL_SETUP:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_REJECT_MAX_CALL_LIMIT_REACHED:
        case CODE_LOCAL_CALL_EXCEEDED:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_LOCAL_CALL_BUSY:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_USER_IGNORE:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_REJECT_UNSUPPORTED_SIP_HEADERS:
            eStatusCode = SipStatusCode::SC_420;
            break;
        case CODE_SIP_NOT_ACCEPTABLE:
            if (objReason.nExtraCode == EXTRA_CODE_NOT_ACCEPTABLE_SIP_488)
            {
                eStatusCode = SipStatusCode::SC_488;
            }
            else if (objReason.nExtraCode == EXTRA_CODE_NOT_ACCEPTABLE_SIP_606)
            {
                eStatusCode = SipStatusCode::SC_606;
            }
            else
            {
                eStatusCode = SipStatusCode::SC_406;
            }
            break;
        case CODE_REJECT_ONGOING_CALL_UPDATE:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_SESSION_INTERNAL_ERROR:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED:
            eStatusCode = SipStatusCode::SC_580;
            break;
        case CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case CODE_MEDIA_INIT_FAILED:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case CODE_MEDIA_NOT_ACCEPTABLE:
            eStatusCode = SipStatusCode::SC_488;
            break;
        case CODE_TIMEOUT_NO_ANSWER:
            eStatusCode = m_objContext.GetConfigurationProxy().GetInt(
                    Feature::INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER);
            break;
        case CODE_TIMEOUT_NO_ANSWER_CALL_UPDATE:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case CODE_NETWORK_RESP_TIMEOUT:
            eStatusCode = SipStatusCode::SC_500;
            break;

        default:
            eStatusCode = SipStatusCode::SC_480;
            break;
    }

    return eStatusCode;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::GetRejectPhrase(IN const CallReasonInfo& objReason, OUT AString& strPhrase)
{
    switch (objReason.nCode)
    {
        case CODE_USER_DECLINE:
            strPhrase = GetRejectPhrase(RejectType::USER_REJECT);
            break;
        case CODE_REJECT_ONGOING_CS_CALL:
            strPhrase = GetRejectPhrase(RejectType::ON_CS_CALL);
            break;
        case CODE_LOCAL_CALL_BUSY:
        case CODE_REJECT_ONGOING_CALL_SETUP:
            strPhrase = GetRejectPhrase(RejectType::ON_CONNECTING_CALL);
            break;
        case CODE_REJECT_MAX_CALL_LIMIT_REACHED:
            strPhrase = GetRejectPhrase(RejectType::EXCEEDS_MAX_CALL);
            break;
        case CODE_TIMEOUT_NO_ANSWER:
            strPhrase = GetRejectPhrase(RejectType::NO_ANSWER_BY_USER);
            break;
        case CODE_REJECT_ONGOING_CALL_UPDATE:
            strPhrase = GetRejectPhrase(RejectType::ON_CONVERTING);
            break;
        case CODE_MEDIA_NOT_ACCEPTABLE:
            strPhrase = GetRejectPhrase(RejectType::NEGOTIATION_FAILURE);
            break;
        default:
            strPhrase = AString::ConstNull();
            break;
    }

    if (strPhrase.GetLength() <= 0)
    {
        strPhrase = AString::ConstNull();
    }

    IMS_TRACE_D("GetRejectPhrase [%s]", strPhrase.GetStr(), 0, 0);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::GetUpdateReason(IN UpdateType eUpdateType, OUT AString& strReason)
{
    switch (eUpdateType)
    {
        case UpdateType::SRVCC_RECOVERED_CANCEL:
            strReason = MessageUtil::STR_REASON_HANDOVER_CANCELLED;
            break;
        case UpdateType::SRVCC_RECOVERED_FAILURE:
            strReason = MessageUtil::STR_REASON_FAILURE_TO_TRANSITION;
            break;
        default:
            strReason = AString::ConstNull();
            break;
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::GetTerminateReason(
        IN const CallReasonInfo& objReason, OUT AString& strReason)
{
    switch (objReason.nCode)
    {
        case CODE_USER_TERMINATED:
            strReason = GetTerminateReason(TerminateType::USER_ENDS_CALL);
            break;
        case CODE_MEDIA_NO_DATA:
            strReason = GetTerminateReason(TerminateType::RTP_TIMEOUT);
            break;
        case CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED:
            strReason = GetTerminateReason(TerminateType::MEDIA_BEARER_LOSS);
            break;
        case CODE_SIP_REQUEST_CANCELLED:
            strReason = GetTerminateReason(TerminateType::SIP_TIMEOUT);
            break;
        case CODE_NETWORK_RESP_TIMEOUT:  // FALL_THROUGH
        case CODE_TIMEOUT_1XX_WAITING:
            strReason = GetTerminateReason(TerminateType::SIP_RESPONSE_TIMEOUT);
            break;
        case CODE_TIMEOUT_NO_ANSWER:
            strReason = GetTerminateReason(TerminateType::CALL_SETUP_TIMEOUT);
            break;

        case CODE_EARLYDIALOG_FORKED_TERMINATED_INTERNALONLY:
            strReason = GetTerminateReason(TerminateType::TERMINATING_EARLY_DIALOG);
            break;
        case CODE_LOCAL_ENDED_BY_CONFERENCE_MERGE:
            strReason = GetTerminateReason(TerminateType::CONFERENCE_CALL_JOINED);
            break;

        default:
            strReason = AString::ConstNull();
            break;
    }

    IMS_TRACE_D("GetTerminateReason [%s]", strReason.GetStr(), 0, 0);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
AString MessageFormatter::GetTerminateReason(IN TerminateType eType)
{
    return m_objContext.GetConfigurationProxy().GetStr(
            Feature::CALL_TERMINATE_REASON_HEADER, static_cast<IMS_SINT32>(eType));
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
AString MessageFormatter::GetRejectPhrase(IN RejectType eType)
{
    return m_objContext.GetConfigurationProxy().GetStr(
            Feature::CALL_REJECT_REASON_PHRASE, static_cast<IMS_SINT32>(eType));
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
IMS_RESULT MessageFormatter::InitVariables(IN FormType eFormType)
{
    m_eFormType = eFormType;

    if (SetNextMessage() != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
IMS_RESULT MessageFormatter::SetNextMessage()
{
    switch (m_eFormType)
    {
        case FormType::START:          // FALL_THROUGH
        case FormType::PRACK:          // FALL_THROUGH
        case FormType::EARLY_UPDATE:   // FALL_THROUGH
        case FormType::ACK:            // FALL_THROUGH
        case FormType::UPDATE:         // FALL_THROUGH
        case FormType::CANCEL_UPDATE:  // FALL_THROUGH
        case FormType::TERMINATE:
        {
            m_piNextMessage = m_objSession.GetNextRequest();
            if (m_piNextMessage == IMS_NULL)
            {
                return IMS_FAILURE;
            }
        }
        break;
        case FormType::PROVISIONAL_RESPONSE:   // FALL_THROUGH
        case FormType::PRACK_RESPONSE:         // FALL_THROUGH
        case FormType::EARLY_UPDATE_RESPONSE:  // FALL_THROUGH
        case FormType::ACCEPT:                 // FALL_THROUGH
        case FormType::REJECT:                 // FALL_THROUGH
        case FormType::ACCEPT_UPDATE:
        {
            m_piNextMessage = m_objSession.GetNextResponse();
            if (m_piNextMessage == IMS_NULL)
            {
                return IMS_FAILURE;
            }
        }
        break;
        default:
            return IMS_FAILURE;
            break;
    }

    return IMS_SUCCESS;
}
