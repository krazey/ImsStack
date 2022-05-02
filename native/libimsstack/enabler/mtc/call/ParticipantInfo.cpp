#include "CallInfo.h"
#include "ICoreService.h"
#include "ImsIdentity.h"
#include "ISipHeader.h"
#include "ServicePhoneInfo.h"
#include "SipAddress.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "dialingplan/MtcDialingPlan.h"
#include "helper/MtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

const AString ParticipantInfo::URI_SET_BY_IMS_ENGINE = AString::ConstNull();
const AString ParticipantInfo::ANONYMOUS_ADDRESS = "sip:anonymous@anonymous.invalid";
const AString ParticipantInfo::ANONYMOUS_DISPLAY_NAME = "Anonymous";

PUBLIC
ParticipantInfo::ParticipantInfo(IN IMtcCallContext& objContext) :
        m_strRemoteUri(AString::ConstNull()),
        m_strRemoteDisplayName(AString::ConstNull()),
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
            .Get(SuppType::TARGET_URI);
    if (pSuppService != IMS_NULL)
    {
        IMS_TRACE_D("GetRemoteUri : From supplementary service[%s]",
                pSuppService->strValue.GetStr(), 0, 0);
        return pSuppService->strValue;
    }

    IMS_TRACE_D("GetRemoteUri : URI[%s]", m_strRemoteUri.GetStr(), 0, 0);
    return m_strRemoteUri;
}

PUBLIC
AString ParticipantInfo::GetRemoteDisplayName() const
{
    const SuppService* pSuppService = m_objContext.GetSupplementaryService()
            .Get(SuppType::CNAP);
    if (pSuppService != IMS_NULL)
    {
        return pSuppService->strValue;
    }

    return m_strRemoteUri;
}

PUBLIC
OipType ParticipantInfo::GetOipType() const
{
    const SuppService* pSuppService = m_objContext.GetSupplementaryService()
            .Get(SuppType::CALLER_ID);
    if (pSuppService != IMS_NULL)
    {
        return static_cast<OipType>(pSuppService->nValue);
    }
    return OipType::NONE;
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
    // TODO: this will be moved to DialingPlan.
    AString strUri = m_objContext.GetConfigurationProxy().GetStr(
            Feature::CONFERENCE_FACTORY_URI, 0);

    IMS_TRACE_D("GetRemoteUriForConferenceCall uri from config[%s]", strUri.GetStr(), 0, 0);

    if (strUri.GetLength() <= 0)
    {
        strUri = "sip:mmtel@conf-factory.ims.mnc[MNC].mcc[MCC].3gppnetwork.org";
    }

    IMS_TRACE_I("GetRemoteUriForConferenceCall [%s]", strUri.GetStr(), 0, 0);

    // TODO: exception handling: mcc/mnc is empty
    return strUri
            .Replace("[MCC]", GetMcc())
            .Replace("[MNC]", GetMnc(3))
            .Replace("[MNC2]", GetMnc(2));
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
