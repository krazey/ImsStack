#include "call/extension/MtcExtensionSet.h"
#include "call/IMtcCall.h"
#include "call/IMtcSessionContext.h"
#include "call/message/MessageFormatter.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "Const3GPP.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include "FailReason.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "IMessage.h"
#include "ImsIdentity.h"
#include "MtcDef.h"
#include "ISession.h"
#include "ISipConfig.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
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
MessageFormatter::MessageFormatter(IN IMtcSessionContext& objContext) :
        m_objContext(objContext),
        m_objSession(objContext.GetISession()),
        m_piNextMessage(IMS_NULL),
        m_eFormType(FormType::NONE)
{
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL MessageFormatter::~MessageFormatter() {}

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
    SetKeepAliveProfile();
    SetCallerIdHeader();
    SetTipHeader();
    SetSupportedHeader();
    SetPreconditionHeader();
    SetPEarlyMediaHeader();
    SetLocation();

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
    SetTipHeader();

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
    SetTipHeader();

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormRejectMessage(
        IN const FailReason& objReason, OUT IMS_SINT32& eStatusCode, OUT AString& strPhrase)
{
    if (InitVariables(FormType::REJECT) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    eStatusCode = GetRejectStatusCode(objReason);
    GetRejectPhrase(objReason, strPhrase);

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

    return IMS_SUCCESS;
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormCancelUpdateMessage(IN const FailReason& objReason)
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
PUBLIC VIRTUAL IMS_RESULT MessageFormatter::FormTerminateMessage(IN const FailReason& objReason)
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
PRIVATE
void MessageFormatter::SetKeepAliveProfile()
{
    IMS_BOOL bKeepAlive = IMS_FALSE;  // TODO, SESSION_SP_KEEP_ALIVE

    if (!bKeepAlive)
    {
        return;
    }

    ICoreService* piCoreService = GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        return;
    }

    IMS_UINT32 eSipFeatures = ISipConfig::SIP_FEATURE_CAPS_KEEP;
    RCPtr<SipProfile> pProfile = new SipProfile();

    pProfile->SetSipFeatures(eSipFeatures);
    piCoreService->SetSIPProfile(pProfile.Get());
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
        MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_ID, ISipHeader::PRIVACY);

        SipAddress objSIPAddress(ImsIdentity::GetAnonymousUserId());
        objSIPAddress.SetDisplayName(MessageUtil::STR_ANONYMOUS);

        AString strSipAddress = objSIPAddress.ToString();
        if (strSipAddress.GetLength() < 1)
        {
            return;
        }

        MessageUtil::SetHeader(m_piNextMessage, strSipAddress, ISipHeader::FROM);
    }
    else if (pSuppService->nValue == CALLERID_IDENTITY)
    {
        MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_NONE, ISipHeader::PRIVACY);
    }
}

/* -------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------- */
PRIVATE
void MessageFormatter::SetTipHeader()
{
    IMS_BOOL bTip = IMS_FALSE;  // TODO, SESSION_SP_TIP
    if (!bTip)
    {
        return;
    }

    IMS_BOOL bAddTagToSupported = IMS_FALSE;
    if (m_eFormType == FormType::START)
    {
        bAddTagToSupported = IMS_TRUE;
    }
    else if ((m_eFormType == FormType::PROVISIONAL_RESPONSE) || (m_eFormType == FormType::ACCEPT))
    {
        IMessage* piPreviousMessage = m_objSession.GetPreviousRequest(IMessage::SESSION_START);
        if (MessageUtil::HasValue(piPreviousMessage, Sip::STR_FROM_CHANGE, ISipHeader::SUPPORTED))
        {
            bAddTagToSupported = IMS_TRUE;
        }
    }

    if (bAddTagToSupported)
    {
        MessageUtil::AddValueIfNotExists(
                m_piNextMessage, Sip::STR_FROM_CHANGE, ISipHeader::SUPPORTED);
    }

    if (m_eFormType != FormType::PROVISIONAL_RESPONSE)
    {
        return;
    }

    IMS_SINT32 eTipMode = TIP_MODE_TEMPORARY;  // TODO, SESSION_SP_TIP_MODE
    if (eTipMode != TIP_MODE_TEMPORARY)
    {
        return;
    }

    IMS_SINT32 eTipType = TIP_TYPE_IDENTITY;  // TODO, from user settings
    if (eTipType == TIP_TYPE_RESTRICTED)
    {
        MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_ID, ISipHeader::PRIVACY);
    }
    else if (eTipType == TIP_TYPE_IDENTITY)
    {
        MessageUtil::SetHeader(m_piNextMessage, MessageUtil::STR_NONE, ISipHeader::PRIVACY);
    }
}

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

    IMS_BOOL bHistoryInfo = IMS_FALSE;  // TODO
    if (bHistoryInfo)
    {
        MessageUtil::AddValueIfNotExists(
                m_piNextMessage, MessageUtil::STR_HISTINFO, ISipHeader::SUPPORTED);
    }
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
IMS_SINT32 MessageFormatter::GetRejectStatusCode(IN const FailReason& objReason)
{
    IMS_SINT32 eStatusCode = SipStatusCode::SC_INVALID;

    switch (objReason.nReason)
    {
        case REJECT_REASON_UNKNOWN:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_DECLINE_USER:
            eStatusCode = m_objContext.GetConfigurationProxy().GetInt(
                    Feature::INCOMING_CALL_REJECT_CODE_FOR_USER_DECLINE);
            break;
        case REJECT_REASON_DECLINE_NOANSWER:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case REJECT_REASON_DECLINE_NOBATTERY:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case REJECT_REASON_DECLINE_NORMAL:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case REJECT_REASON_DECLINE_CW:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_DECLINE_UPDATE:
            eStatusCode = SipStatusCode::SC_603;
            break;

        case REJECT_REASON_SERVICE_UNAVAILABLE:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_SERVICE_TTY:
            eStatusCode = SipStatusCode::SC_480;
            break;

        case REJECT_REASON_BUSY_ISCSCALL:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_ISEMERGENCY:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_ISWIFICALL:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_ISOTHERSCALL:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_ESTABLISHING:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_ALERTING:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_MAXCALL:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_NORMAL:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_IGNORE:
            eStatusCode = SipStatusCode::SC_486;
            break;
        case REJECT_REASON_BUSY_HIDE:
            eStatusCode = SipStatusCode::SC_486;
            break;

        case REJECT_REASON_SESSION_NOTSUPPORT:
            eStatusCode = SipStatusCode::SC_420;
            break;
        case REJECT_REASON_SESSION_NOTACCEPTABLE:
            eStatusCode = SipStatusCode::SC_406;
            break;
        case REJECT_REASON_SESSION_NOTACCEPTABLEHERE:
            eStatusCode = SipStatusCode::SC_488;
            break;
        case REJECT_REASON_SESSION_UPDATE:
            eStatusCode = SipStatusCode::SC_491;
            break;
        case REJECT_REASON_SESSION_BAD:
            eStatusCode = SipStatusCode::SC_400;
            break;
        case REJECT_REASON_SESSION_FAIL:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_SESSION_FAIL_PRECONDITION:
            eStatusCode = SipStatusCode::SC_580;
            break;
        case REJECT_REASON_SESSION_INVALID_REFERRER_IDENTITY:
            eStatusCode = SipStatusCode::SC_429;
            break;
        case REJECT_REASON_CONF_JOINED:
            eStatusCode = SipStatusCode::SC_480;
            break;

        case REJECT_REASON_MEDIA_INITFAIL:
            eStatusCode = SipStatusCode::SC_415;
            break;
        case REJECT_REASON_MEDIA_CODEC:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_MEDIA_LOWEST_BIT_RATE:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_MEDIA_CHECK_RADIO_CONNECTION:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_MEDIA_NEGOFAIL:
            eStatusCode = SipStatusCode::SC_415;
            break;
        case REJECT_REASON_MEDIA_FORMFAIL:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_MEDIA_NODATA:
            eStatusCode = SipStatusCode::SC_480;
            break;
        case REJECT_REASON_MEDIA_FAIL:
            eStatusCode = SipStatusCode::SC_480;
            break;

        case REJECT_REASON_TO_MO_PROGRESSING:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case REJECT_REASON_TO_MO_STARTED:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case REJECT_REASON_TO_MO_UPDATE:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case REJECT_REASON_TO_MT_NOANSWER:
            eStatusCode = m_objContext.GetConfigurationProxy().GetInt(
                    Feature::INCOMING_CALL_REJECT_CODE_FOR_NO_ANSWER);
            break;
        case REJECT_REASON_TO_MT_UPDATE:
            eStatusCode = SipStatusCode::SC_603;
            break;
        case REJECT_REASON_TO_MT_PRACK:
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
void MessageFormatter::GetRejectPhrase(IN const FailReason& objReason, OUT AString& strPhrase)
{
    switch (objReason.nReason)
    {
        case REJECT_REASON_DECLINE_USER:
            strPhrase = GetRejectPhrase(RejectType::USER_REJECT);
            break;
        case REJECT_REASON_BUSY_ISCSCALL:
            strPhrase = GetRejectPhrase(RejectType::ON_CS_CALL);
            break;
        case REJECT_REASON_BUSY_NORMAL:
        case REJECT_REASON_BUSY_ALERTING:
        case REJECT_REASON_BUSY_ESTABLISHING:
            strPhrase = GetRejectPhrase(RejectType::ON_CONNECTING_CALL);
            break;
        case REJECT_REASON_BUSY_MAXCALL:
            strPhrase = GetRejectPhrase(RejectType::EXCEEDS_MAX_CALL);
            break;
        case REJECT_REASON_TO_MT_NOANSWER:
            strPhrase = GetRejectPhrase(RejectType::NO_ANSWER_BY_USER);
            break;
        case REJECT_REASON_SESSION_UPDATE:
            strPhrase = GetRejectPhrase(RejectType::ON_CONVERTING);
            break;
        case REJECT_REASON_MEDIA_NEGOFAIL:
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
void MessageFormatter::GetTerminateReason(IN const FailReason& objReason, OUT AString& strReason)
{
    switch (objReason.nReason)
    {
        case FAIL_REASON_SESSION_USERTERMINATE:
            strReason = GetTerminateReason(TerminateType::USER_ENDS_CALL);
            break;
        case FAIL_REASON_MEDIA_NODATA:
            strReason = GetTerminateReason(TerminateType::RTP_TIMEOUT);
            break;
        case FAIL_REASON_SESSION_PRECONDITION:
            strReason = GetTerminateReason(TerminateType::MEDIA_BEARER_LOSS);
            break;
        case FAIL_REASON_SESSION_CANCELED:
            strReason = GetTerminateReason(TerminateType::SIP_TIMEOUT);
            break;
        case FAIL_REASON_SESSION_RES_TIMEOUT:  // FALL_THROUGH
        case FAIL_REASON_TO_MO_PROGRESSING:
            strReason = GetTerminateReason(TerminateType::SIP_RESPONSE_TIMEOUT);
            break;
        case FAIL_REASON_TO_MO_STARTED:
            strReason = GetTerminateReason(TerminateType::CALL_SETUP_TIMEOUT);
            break;

        case FAIL_REASON_SESSION_EARLYDIALOG:
            strReason = GetTerminateReason(TerminateType::TERMINATING_EARLY_DIALOG);
            break;
        case FAIL_REASON_SESSION_REFRESH_OUT:
            strReason = GetTerminateReason(TerminateType::SESSION_RREFRESH_FAILURE);
            break;
        case FAIL_REASON_CONF_JOINED:
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
