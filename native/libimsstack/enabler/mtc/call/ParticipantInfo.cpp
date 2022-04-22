#include "ICoreService.h"
#include "IMessage.h"
#include "ImsIdentity.h"
#include "ISIPHeader.h"
#include "ServicePhoneInfo.h"
#include "SIPAddress.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "dialingplan/MtcDialingPlan.h"
#include "helper/MtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

const AString ParticipantInfo::URI_SET_BY_IMS_ENGINE = AString::ConstNull();
const AString ParticipantInfo::ANONYMOUS_ADDRESS = "sip:anonymous@anonymous.invalid";
const AString ParticipantInfo::ANONYMOUS_DISPLAY_NAME = "Anonymous";

PUBLIC
ParticipantInfo::ParticipantInfo(IN IMtcCallContext& objContext) :
        m_strRemoteUri(AString::ConstNull()),
        m_strRemoteDisplayName(AString::ConstNull()),
        m_eOipType(OipType::NONE),
        m_objContext(objContext)
{
}

PUBLIC
ParticipantInfo::~ParticipantInfo()
{
}

/*
PUBLIC
AString ParticipantInfo::GetLocalNumber()
{
    const ICoreService* piCoreService = m_objContext
            .GetService()
            .GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        return AString::ConstNull();
    }

    return piCoreService->GetAuthorizedUserId().GetUser();
}
*/

PUBLIC
AString ParticipantInfo::GetLocalUri() const
{
    if (m_objContext.GetService().IsEmergency())
    {
        return GetLocalUriForEmergencyCall();
    }
    return URI_SET_BY_IMS_ENGINE;
}

PUBLIC
AString ParticipantInfo::GetRemoteUri() const
{
    /*
    TODO: E911 redial I/F will be changed
    AStringBuffer objServiceUrn;
    UCRedial::GetInstance()->GetServiceURNForRedial(
            m_objContext.GetSlotId(), strNumber, objServiceUrn);
    UCRedial::GetInstance()->ClearServiceURNForRedial(m_objContext.GetSlotId());
    if (objServiceUrn.GetLength() > 0 && m_objContext.GetCallInfo().bEmergency)
    {
        return objServiceUrn.GetString();
    }
    */

    if (m_objContext.GetCallInfo().bConference)
    {
        return GetRemoteUriForConferenceCall();
    }

    const SuppService* pSuppService = m_objContext.GetSupplementaryService()
            .Get(SUPP_TYPE_TARGET_URI);
    if (pSuppService != IMS_NULL)
    {
        IMS_TRACE_D("GetRemoteUri : From supplementary service[%s]",
                pSuppService->aStrValue.GetStr(), 0, 0);
        return pSuppService->aStrValue;
    }

    IMS_TRACE_D("GetRemoteUri : URI[%s]", m_strRemoteUri.GetStr(), 0, 0);
    return m_strRemoteUri;
}

PUBLIC
AString ParticipantInfo::GetRemoteDisplayName() const
{
    const SuppService* pSuppService = m_objContext.GetSupplementaryService()
            .Get(SUPP_TYPE_CNAP);
    if (pSuppService != IMS_NULL)
    {
        return pSuppService->aStrValue;
    }

    return m_strRemoteUri;
}

PUBLIC
OipType ParticipantInfo::GetOipType() const
{
    return m_eOipType;
}

PUBLIC
void ParticipantInfo::SetRemoteNumber(IN const AString& strRemoteNumber)
{
    m_strRemoteUri = m_objContext.GetDialingPlan().GetToUri(strRemoteNumber,
            m_objContext.GetCallInfo());
}

PUBLIC
void ParticipantInfo::SetRemoteUri(IN const AString& strRemoteUri)
{
    m_strRemoteUri = strRemoteUri;
}

PUBLIC
void ParticipantInfo::FormatRequest(IN IMS_UINT32 /* eMethod */, IN_OUT IMessage& /* objRequest */)
{
}

PUBLIC
void ParticipantInfo::FormatResponse(
        IN IMS_UINT32 /* eMethod */, IN_OUT IMessage& /* objResponse */)
{
}

PUBLIC
void ParticipantInfo::HandleRequest(IN IMS_UINT32 eMethod, IN const IMessage& objRequest)
{
    if (eMethod != IMessage::SESSION_START)
    {
        return;
    }

    m_eOipType = GetOipTypeFrom(objRequest);

    IMS_TRACE_D("HandleRequest : OIP type[%d]", static_cast<IMS_SINT32>(m_eOipType), 0, 0);
}

PUBLIC
void ParticipantInfo::HandleResponse(
        IN IMS_UINT32 /* eMethod */, IN const IMessage& /* objResponse */)
{
}

PRIVATE
AString ParticipantInfo::GetLocalUriForEmergencyCall() const
{
    MtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    IMS_UINT32 nAosRegistrationMode =
            pAosConnector ? pAosConnector->GetRegistrationMode() : IImsAosInfo::REG_MODE_UNKNOWN;

    if (nAosRegistrationMode == IImsAosInfo::REG_MODE_NOUICC ||
            nAosRegistrationMode == IImsAosInfo::REG_MODE_ADMIN)
    {
        SIPAddress objSipAddress;
        if (objSipAddress.Create(ANONYMOUS_ADDRESS) == IMS_FALSE)
        {
            IMS_TRACE_E(0, "GetLocalUriForEmergencyCall : Failed to create SIP address", 0, 0, 0);
            return URI_SET_BY_IMS_ENGINE;
        }

        objSipAddress.SetDisplayName(ANONYMOUS_DISPLAY_NAME);
        return objSipAddress.ToString();
    }

    return URI_SET_BY_IMS_ENGINE;
}

PRIVATE
AString ParticipantInfo::GetRemoteUriForConferenceCall() const
{
    AString strUri = "";    // TODO: from config

    if (strUri.GetLength() <= 0 || strUri.EqualsIgnoreCase("standard"))
    {
        strUri = "sip:mmtel@conf-factory.ims.mnc[MNC].mcc[MCC].3gppnetwork.org";
    }
    else if (strUri.EqualsIgnoreCase("standard_MNC2"))
    {
        strUri = "sip:mmtel@conf-factory.ims.mnc[MNC2].mcc[MCC].3gppnetwork.org";
    }

    IMS_TRACE_I("GetRemoteUriForConferenceCall [%s]", strUri.GetStr(), 0, 0);

    // TODO: exception handling: mcc/mnc is empty
    return strUri
            .Replace("[MCC]", GetMcc())
            .Replace("[MNC]", GetMnc(3))
            .Replace("[MNC2]", GetMnc(2));
}

PRIVATE
OipType ParticipantInfo::GetOipTypeFrom(IN const IMessage& objMessage) const
{
    AString strPrivacy;
    MessageUtil::GetHeader(&objMessage, ISIPHeader::PRIVACY, strPrivacy);
    if (strPrivacy.EqualsIgnoreCase("id"))
    {
        return OipType::RESTRICTED;
    }
    else if (strPrivacy.EqualsIgnoreCase("none"))
    {
        return OipType::IDENTITY;
    }

    IMS_SINT32 nCnapType = CNAP_SCHEME_PAID_FROM;   // TODO: From config
    switch (nCnapType)
    {
        case CNAP_SCHEME_PAID_FROM:
            return (MessageUtil::IsHeaderPresent(&objMessage, ISIPHeader::P_ASSERTED_IDENTITY) ||
                    MessageUtil::IsHeaderPresent(&objMessage, ISIPHeader::FROM)) ?
                    OipType::IDENTITY :
                    OipType::NONE;

        case CNAP_SCHEME_FROM:
            return MessageUtil::IsHeaderPresent(&objMessage, ISIPHeader::FROM) ?
                    OipType::IDENTITY :
                    OipType::NONE;

        case CNAP_SCHEME_PAID:
            return MessageUtil::IsHeaderPresent(&objMessage, ISIPHeader::P_ASSERTED_IDENTITY) ?
                    OipType::IDENTITY :
                    OipType::NONE;

        default:
            IMS_TRACE_E(0, "Unhandled CNAP type[%d]", nCnapType, 0, 0);
    }
    return OipType::NONE;
}

PRIVATE
AString ParticipantInfo::GetMcc() const
{
    AString strMcc;
    PhoneInfoService::GetPhoneInfoService()
            ->GetSubscriberInfo(m_objContext.GetSlotId())
            ->GetMcc(strMcc);
    return strMcc;
}

PRIVATE
AString ParticipantInfo::GetMnc(IN IMS_UINT32 nLength) const
{
    AString strMnc;
    PhoneInfoService::GetPhoneInfoService()
            ->GetSubscriberInfo(m_objContext.GetSlotId())
            ->GetMnc(strMnc);
    if (nLength == 3 && strMnc.GetLength() == 2)
    {
        strMnc.Prepend("0");
    }
    return strMnc;
}
