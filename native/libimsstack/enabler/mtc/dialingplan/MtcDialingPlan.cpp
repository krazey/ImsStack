#include "ImsIdentity.h"
#include "ImsAccessNetworkInfoType.h"
#include "IMSLib.h"
#include "ServiceNetwork.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "AString.h"
#include "Sip.h"
#include "SipAddress.h"
#include "IMtcContext.h"
#include "dialingplan/MtcDialingPlan.h"
#include "dialingplan/EmergencyDialingPlan.h"
#include "util/TextParser.h"
#include "helper/MtcAosConnector.h"
#include "CallInfo.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcDialingPlan::MtcDialingPlan(IN IMtcContext& objContext) :
        m_objContext(objContext),
        m_pTemporaryServiceUrn(nullptr)
{
    IMS_TRACE_I("+MtcDialingPlan", 0, 0, 0);
}

PUBLIC
MtcDialingPlan::~MtcDialingPlan()
{
    IMS_TRACE_I("~MtcDialingPlan", 0, 0, 0);
}

PUBLIC
AString MtcDialingPlan::GetToUri(IN const AString& strNumber, IN const CallInfo& objCallInfo,
        IN Scheme eScheme/* = Scheme::Unknown*/)
{
    AString strUri = strNumber;

    // this is for the case STK generates URI.
    if (IsUriForm(strUri))
    {
        return strUri;
    }

    if (m_pTemporaryServiceUrn)
    {
        if (m_pTemporaryServiceUrn->GetNumber().Equals(strNumber))
        {
            AString strUrn = m_pTemporaryServiceUrn->GetUrn();
            m_pTemporaryServiceUrn = nullptr;
            return strUrn;
        }
        m_pTemporaryServiceUrn = nullptr;
    }

    if (objCallInfo.bUssi)
    {
        return NormalDialingPlan::GetTranslatedUriForDialString(m_objContext, strUri);
    }

    if (objCallInfo.bConference)
    {
        // TODO: creating confrence factory uri also needs to be moved
    }

    if (objCallInfo.bEmergency)
    {
        return EmergencyDialingPlan::GetTranslatedUri(m_objContext, strUri);
    }

    return NormalDialingPlan::GetTranslatedUri(m_objContext, strUri, eScheme);
}

PUBLIC
void MtcDialingPlan::OnCountrySpecificServiceUrnReceived(IN const AString& strNumber,
        IN const AString& strServiceUrn)
{
    // if already exists, overwrite.
    m_pTemporaryServiceUrn = std::make_unique<TemporaryServiceUrn>(strNumber, strServiceUrn);
}

PRIVATE
IMS_BOOL MtcDialingPlan::IsUriForm(IN const AString& strNumber)
{
    SIPAddress objSipAddress;
    objSipAddress.Create(strNumber);

    if (objSipAddress.GetScheme().EqualsIgnoreCase(SIP::STR_SIP) ||
            objSipAddress.GetScheme().EqualsIgnoreCase(SIP::STR_SIPS) ||
            objSipAddress.GetScheme().EqualsIgnoreCase(SIP::STR_TEL))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
