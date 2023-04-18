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
#include "Const3GPP.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "IMessage.h"
#include "ISession.h"
#include "ISipConfig.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsIdentity.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "call/message/MessageFormatter.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "core/Replaces.h"
#include "dialogevent/IMultiEndpointManager.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include "sipcore/Sip.h"
#include "sipcore/SipAddress.h"
#include "sipcore/SipHeaderName.h"
#include "sipcore/SipProfile.h"
#include "sipcore/SipStatusCode.h"
#include "utility/CallComposerUtil.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC
MessageFormatter::MessageFormatter(IN IMtcCallContext& objContext, IN ISession& objSession) :
        m_objContext(objContext),
        m_objSession(objSession),
        m_piNextMessage(IMS_NULL),
        m_eFormType(FormType::NONE)
{
    IMS_TRACE_I("+MessageFormatter", 0, 0, 0);
    m_objSession.SetReasonHeaderSetter(this);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL MessageFormatter::~MessageFormatter()
{
    IMS_TRACE_I("~MessageFormatter", 0, 0, 0);
    m_objSession.SetReasonHeaderSetter(IMS_NULL);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL void MessageFormatter::ReasonHeaderSetter_SetHeader(
        IN ISipMessage* piSipMsg, IN IMS_SINT32 nTerminationReason)
{
    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
    if (objConfig.Is(
                Feature::CARRIER_SPECIFIC_SIP_HEADER, MessageUtil::STR_REASON_USER_SESSIONEXPIRED))
    {
        IMS_TRACE_D("ReasonHeaderSetter_SetHeader [%d]", nTerminationReason, 0, 0);
        if ((nTerminationReason == ISession::TERMINATION_REASON_REFRESH_TIMEOUT) ||
                (nTerminationReason == ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT))
        {
            piSipMsg->AddHeader(
                    ISipHeader::UNKNOWN, "USER;text=\"Session Expired\"", SipHeaderName::REASON);
        }
    }

    if (objConfig.Is(Feature::CARRIER_SPECIFIC_SIP_HEADER, MessageUtil::STR_P_SKT_BYE_CAUSE))
    {
        IMS_TRACE_D("ReasonHeaderSetter_SetHeader [%d]", nTerminationReason, 0, 0);
        if ((nTerminationReason == ISession::TERMINATION_REASON_REFRESH_TIMEOUT) ||
                (nTerminationReason == ISession::TERMINATION_REASON_REFRESH_TXN_TIMEOUT))
        {
            piSipMsg->AddHeader(ISipHeader::UNKNOWN,
                    "SIP; cause=103; text=\"Session-Expire\"; fc=9602", SipHeaderName::REASON);
            piSipMsg->AddHeader(ISipHeader::UNKNOWN, "no_upd", MessageUtil::STR_P_SKT_BYE_CAUSE);
        }
        else
        {
            if (nTerminationReason == ISession::TERMINATION_REASON_USER_ACTION)
            {
                piSipMsg->AddHeader(ISipHeader::UNKNOWN,
                        "USER; cause=101;text=\"USER triggered\"; fc=9501", SipHeaderName::REASON);
            }
            else
            {
                piSipMsg->AddHeader(ISipHeader::UNKNOWN,
                        "ETC; cause=104; text=\"Unknown\"; fc=9999", SipHeaderName::REASON);
            }

            piSipMsg->AddHeader(ISipHeader::UNKNOWN, "normal", MessageUtil::STR_P_SKT_BYE_CAUSE);
        }
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL void MessageFormatter::ReasonHeaderSetter_SetPrivateHeader(
        IN ISipMessage* piOldSipMsg, IN ISipMessage* piNewSipMsg)
{
    MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
    if (objConfig.Is(Feature::CARRIER_SPECIFIC_SIP_HEADER, MessageUtil::STR_P_SKT_BYE_CAUSE))
    {
        const AString strPSktByeCause(MessageUtil::STR_P_SKT_BYE_CAUSE);
        AString strByeCause = piOldSipMsg->GetHeader(ISipHeader::UNKNOWN, 0, strPSktByeCause);
        IMS_TRACE_D("ReasonHeaderSetter_SetPrivateHeader [%s]", strByeCause.GetStr(), 0, 0);

        if (strByeCause.GetLength() > 0)
        {
            piNewSipMsg->SetHeader(ISipHeader::UNKNOWN, strByeCause, strPSktByeCause);
        }
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormStartMessage(IN CallType eCallType)
{
    if (InitVariables(FormType::START) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    SetAcceptContactHeader(eCallType);
    SetAcceptHeader();
    SetPPreferredServiceHeader();
    AddSrvccFeature();
    SetCallerIdHeader();
    // SetTipHeader();
    SetPEarlyMediaHeader();
    SetReplacesHeader();
    SetCarrierSpecificHeaders();
    SetCallComposerElements();

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

    if (objReason.nCode == CODE_REJECT_UNSUPPORTED_SIP_HEADERS &&
            objReason.strExtraMessage.GetLength() > 0)
    {
        m_objContext.GetMessageUtils().AddValueIfNotExists(
                m_piNextMessage, objReason.strExtraMessage, ISipHeader::UNSUPPORTED);
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
    SetCarrierSpecificHeaders();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PROTECTED VIRTUAL
void MessageFormatter::SetAcceptHeader()
{
    m_objContext.GetMessageUtils().AddValueIfNotExists(
            m_piNextMessage, MessageUtil::STR_ACCEPT_TYPE_APPLICATION_SDP, ISipHeader::ACCEPT);
    m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage,
            MessageUtil::STR_ACCEPT_TYPE_APPLICATION_3GPP_IMS_XML, ISipHeader::ACCEPT);
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
    m_objContext.GetMessageUtils().SetHeader(m_piNextMessage, Const3GPP::ICSI_MMTEL,
            ISipHeader::UNKNOWN, SipHeaderName::P_PREFERRED_SERVICE);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetAcceptContactHeader(IN CallType eCallType)
{
    AString strAcceptContact;
    strAcceptContact.Append(TextParser::CHAR_ASTERISK);
    strAcceptContact.Append(TextParser::CHAR_SEMICOLON);

    strAcceptContact.Append(MessageUtil::STR_ICSI);
    strAcceptContact.Append(TextParser::CHAR_DQUOT);
    strAcceptContact.Append(AString(Const3GPP::ICSI_MMTEL).Replace(":", "%3A"));
    strAcceptContact.Append(TextParser::CHAR_DQUOT);

    if (eCallType == CallType::VT || eCallType == CallType::VIDEO_RTT)
    {
        strAcceptContact.Append(TextParser::CHAR_SEMICOLON);
        strAcceptContact.Append(MessageUtil::STR_VIDEO);
    }

    m_objContext.GetMessageUtils().SetHeader(
            m_piNextMessage, strAcceptContact, ISipHeader::ACCEPT_CONTACT);
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

    if (m_objContext.GetMessageUtils().ContainsValue(piPreviousMessage,
                MessageUtil::STR_SRVCC_FEATURE_A, ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_A, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_RESPONSE);
    }

    if (m_objContext.GetMessageUtils().ContainsValue(piPreviousMessage,
                MessageUtil::STR_SRVCC_FEATURE_B, ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_B, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_RESPONSE);
    }

    if (m_objContext.GetMessageUtils().ContainsValue(piPreviousMessage,
                MessageUtil::STR_SRVCC_FEATURE_M, ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        piFeatureCaps->AddFeature(MessageUtil::STR_SRVCC_FEATURE_M, AString::ConstEmpty(),
                SipMethod::INVITE, ISipMessage::TYPE_RESPONSE);
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

    if (m_objContext.GetMessageUtils().ContainsValue(piPreviousMessage,
                MessageUtil::STR_SRVCC_FEATURE_A, ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        m_objSession.SetContactParameter(MessageUtil::STR_SRVCC_FEATURE_A, 0);
    }

    if (m_objContext.GetMessageUtils().ContainsValue(piPreviousMessage,
                MessageUtil::STR_SRVCC_FEATURE_B, ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        m_objSession.SetContactParameter(MessageUtil::STR_SRVCC_FEATURE_B, 0);
    }

    if (m_objContext.GetMessageUtils().ContainsValue(piPreviousMessage,
                MessageUtil::STR_SRVCC_FEATURE_M, ISipHeader::UNKNOWN, SipHeaderName::FEATURE_CAPS))
    {
        m_objSession.SetContactParameter(MessageUtil::STR_SRVCC_FEATURE_M, 0);
    }
}

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
        const MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
        if (objConfig.GetInt(Feature::SESSION_PRIVACY_TYPE) ==
                CarrierConfig::ImsVoice::SESSION_PRIVACY_TYPE_HEADER)
        {
            m_objContext.GetMessageUtils().SetHeader(
                    m_piNextMessage, MessageUtil::STR_HEADER, ISipHeader::PRIVACY);
        }
        else
        {
            m_objContext.GetMessageUtils().SetHeader(
                    m_piNextMessage, MessageUtil::STR_ID, ISipHeader::PRIVACY);
        }

        SipAddress objSIPAddress(ImsIdentity::GetAnonymousUserId());
        objSIPAddress.SetDisplayName(MessageUtil::STR_ANONYMOUS);
        m_objContext.GetMessageUtils().SetHeader(
                m_piNextMessage, objSIPAddress.ToString(), ISipHeader::FROM);
    }
    else if (pSuppService->nValue == CALLERID_IDENTITY)
    {
        m_objContext.GetMessageUtils().SetHeader(
                m_piNextMessage, MessageUtil::STR_NONE, ISipHeader::PRIVACY);
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
//         if (m_objContext.GetMessageUtils().HasValue(piPreviousMessage, Sip::STR_FROM_CHANGE,
//                 ISipHeader::SUPPORTED))
//         {
//             bAddTagToSupported = IMS_TRUE;
//         }
//     }

//     if (bAddTagToSupported)
//     {
//         m_objContext.GetMessageUtils().AddValueIfNotExists(
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
//         m_objContext.GetMessageUtils().SetHeader(m_piNextMessage, MessageUtil::STR_ID,
//         ISipHeader::PRIVACY);
//     }
//     else if (eTipType == TIP_TYPE_IDENTITY)
//     {
//         m_objContext.GetMessageUtils().SetHeader(m_piNextMessage, MessageUtil::STR_NONE,
//         ISipHeader::PRIVACY);
//     }
// }

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetPEarlyMediaHeader()
{
    if (m_objContext.GetCallInfo().bUssi)
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(
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

    m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage,
            MessageUtil::STR_ALERT_URN_CALL_WAITING, ISipHeader::UNKNOWN,
            SipHeaderName::ALERT_INFO);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetReasonHeader(IN const AString& strReason)
{
    if (strReason.GetLength() < 1)
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(
            m_piNextMessage, strReason, ISipHeader::UNKNOWN, SipHeaderName::REASON);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetCarrierSpecificHeaders()
{
    MtcConfigurationProxy& objConfig = m_objContext.GetConfigurationProxy();
    if (objConfig.Is(Feature::CARRIER_SPECIFIC_SIP_HEADER, MessageUtil::STR_P_TTA_VOLTE_INFO))
    {
        if (m_eFormType == FormType::START || m_eFormType == FormType::ACCEPT ||
                m_eFormType == FormType::UPDATE || m_eFormType == FormType::ACCEPT_UPDATE)
        {
            IMS_TRACE_D("SetCarrierSpecificHeaders : avchange", 0, 0, 0);
            m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage,
                    MessageUtil::STR_AVCHANGE, ISipHeader::UNKNOWN,
                    MessageUtil::STR_P_TTA_VOLTE_INFO);
        }
    }

    if (objConfig.Is(Feature::CARRIER_SPECIFIC_SIP_HEADER, MessageUtil::STR_P_SKT_BYE_CAUSE))
    {
        if (m_eFormType == FormType::TERMINATE)
        {
            // TODO: add bye cause by the CallReasonInfo.
            // TODO: update KT carrier's config for termination reason
            m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage, "normal",
                    ISipHeader::UNKNOWN, MessageUtil::STR_P_SKT_BYE_CAUSE);
        }
    }

    // Assumes only VZW supports CALL_PULL and it's a carrier specific feature.
    if (m_objContext.GetSupplementaryService().Get(SuppType::CALL_PULL))
    {
        if (m_eFormType == FormType::START)
        {
            m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage, "true",
                    ISipHeader::UNKNOWN, MessageUtil::STR_P_COM_ENABLETRANSCODING);
        }
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetCallComposerElements()
{
    MtcSupplementaryService& objSupplementaryServices = m_objContext.GetSupplementaryService();

    const SuppService* pPriority = objSupplementaryServices.Get(SuppType::CALL_COMPOSER_PRIORITY);
    if (pPriority != IMS_NULL)
    {
        CallComposerUtil::SetPriority(pPriority->nValue, *m_piNextMessage);
    }

    const SuppService* pSubject = objSupplementaryServices.Get(SuppType::CALL_COMPOSER_SUBJECT);
    if (pSubject != IMS_NULL)
    {
        CallComposerUtil::SetSubject(pSubject->strValue, *m_piNextMessage);
    }

    const SuppService* pPicture = objSupplementaryServices.Get(SuppType::CALL_COMPOSER_PICTURE_URL);
    if (pPicture != IMS_NULL)
    {
        CallComposerUtil::SetPicture(pPicture->strValue, *m_piNextMessage);
    }

    const SuppService* pLatitude =
            objSupplementaryServices.Get(SuppType::CALL_COMPOSER_LOCATION_LAT);
    const SuppService* pLongitude =
            objSupplementaryServices.Get(SuppType::CALL_COMPOSER_LOCATION_LONG);
    if (pLatitude != IMS_NULL && pLongitude != IMS_NULL)
    {
        CallComposerUtil::SetLocation(
                pLatitude->strValue, pLongitude->strValue, m_objContext, *m_piNextMessage);
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetReplacesHeader()
{
    if (!m_objContext.GetSupplementaryService().Get(SuppType::CALL_PULL))
    {
        return;
    }

    IMultiEndpointManager* piMultiEndpointManager = m_objContext.GetMultiEndpointManager();
    if (!piMultiEndpointManager)
    {
        return;
    }

    IMultiEndpointManager::PullingDialogInfo objDialogInfo = piMultiEndpointManager->GetDialogInfo(
            m_objContext.GetParticipantInfo().GetRemoteNumber());
    Replaces objReplaces(
            objDialogInfo.strCallId, objDialogInfo.strLocalTag, objDialogInfo.strRemoteTag);
    m_objContext.GetMessageUtils().AddValueIfNotExists(
            m_piNextMessage, objReplaces.ToString(IMS_FALSE), ISipHeader::REPLACES);
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
IMS_SINT32 MessageFormatter::GetRejectStatusCode(IN const CallReasonInfo& objReason)
{
    IMS_SINT32 eStatusCode = SipStatusCode::SC_INVALID;

    switch (objReason.nCode)
    {
        case CODE_USER_DECLINE:
            eStatusCode = m_objContext.GetConfigurationProxy().GetInt(
                    Feature::INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE);
            break;
        case CODE_USER_NOANSWER:
        case CODE_LOW_BATTERY:
        case CODE_REJECT_ONGOING_CALL_WAITING_DISABLED:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_LOCAL_SERVICE_UNAVAILABLE:
        case CODE_REJECT_VT_TTY_NOT_ALLOWED:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case CODE_REJECT_ONGOING_CS_CALL:
        case CODE_REJECT_ONGOING_E911_CALL:
        case CODE_REJECT_CALL_ON_OTHER_SUB:
        case CODE_REJECT_ONGOING_CALL_SETUP:
        case CODE_REJECT_MAX_CALL_LIMIT_REACHED:
        case CODE_LOCAL_CALL_EXCEEDED:
        case CODE_LOCAL_CALL_BUSY:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_LOCAL_CALL_DECLINE:
            eStatusCode = SipStatusCode::SC_603;
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
            else if (objReason.nExtraCode == EXTRA_CODE_NOT_ACCEPTABLE_BY_CALL_TYPE)
            {
                eStatusCode = m_objContext.GetConfigurationProxy().GetInt(
                        Feature::CALL_REJECT_CODE_FOR_NOT_ACCEPTABLE_CALL_TYPE);
            }
            else
            {
                eStatusCode = SipStatusCode::SC_406;
            }
            break;
        case CODE_REJECT_ONGOING_CALL_UPGRADE:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case CODE_REJECT_INTERNAL_ERROR:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case CODE_LOCAL_CALL_RESOURCE_RESERVATION_FAILED:  // TODO: can be removed?
        case CODE_REJECT_QOS_FAILURE:
            eStatusCode = SipStatusCode::SC_580;
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
        case CODE_BLACKLISTED_CALL_ID:
        case CODE_USER_REJECTED_SESSION_MODIFICATION:
            eStatusCode = SipStatusCode::SC_603;
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
        case CODE_REJECT_ONGOING_CALL_UPGRADE:
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
            if (objReason.nExtraCode == EXTRA_USER_TERMINATED_AND_SIP_TIMEOUT)
            {
                strReason = GetTerminateReason(TerminateType::USER_ENDS_AND_SIP_RESPONSE_TIMEOUT);
            }
            else if (objReason.nExtraCode == EXTRA_USER_TERMINATED_AND_RTP_TIMEOUT)
            {
                strReason = GetTerminateReason(TerminateType::USER_ENDS_CALL_AND_RTP_TIMEOUT);
            }

            if (strReason.GetLength() <= 0)
            {
                strReason = GetTerminateReason(TerminateType::USER_ENDS_CALL);
            }
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
        case CODE_INTERNAL_EARLYDIALOG_FORKED_TERMINATED:
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
    }

    return IMS_SUCCESS;
}
