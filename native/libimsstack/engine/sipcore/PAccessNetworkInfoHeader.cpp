/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20101031  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceSystemTime.h"
#include "ServicePhoneInfo.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServiceUtil.h"
#include "AccessNetworkInfoFormatter.h"
#include "private/SipConfigV.h"
#include "SIPFeatures.h"
#include "SIPFactory.h"
#include "ISIPHeader.h"
#include "ISIPMessage.h"
#include "ISIPRTConfigHelper.h"
#include "SIPConfigProxy.h"
#include "PAccessNetworkInfoHeader.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL
IMS_BOOL PAccessNetworkInfoHeader::FormHeader(IN IMS_SINT32 nSlotId,
        IN const AccessNetworkInfo &objANInfo, OUT AString &strHeader)
{
    //---------------------------------------------------------------------------------------------

    if (!AccessNetworkInfoFormatter::Encode(objANInfo, strHeader))
    {
        return IMS_FALSE;
    }

    const SipConfigV *pSipConfigV
            = DYNAMIC_CAST(const SipConfigV*, SIPConfigProxy::GetSipConfigV(nSlotId));

    if (pSipConfigV != IMS_NULL)
    {
        switch (objANInfo.nType)
        {
            case AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD: // FALL-THROUGH
            case AccessNetworkInfo::TYPE_3GPP_UTRAN_TDD:
                if (pSipConfigV->GetPredefinedPaniForUtran().GetLength() > 0)
                {
                    strHeader = pSipConfigV->GetPredefinedPaniForUtran();
                    IMS_TRACE_D("PANI(utran) is overwritten by the pre-defined value", 0, 0, 0);
                }
                break;
            case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD: // FALL-THROUGH
            case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD:
                if (pSipConfigV->GetPredefinedPaniForEutran().GetLength() > 0)
                {
                    strHeader = pSipConfigV->GetPredefinedPaniForEutran();
                    IMS_TRACE_D("PANI(eutran) is overwritten by the pre-defined value", 0, 0, 0);
                }
                break;

                // WIFI
            case AccessNetworkInfo::TYPE_IEEE_802_11: // FALL-THROUGH
            case AccessNetworkInfo::TYPE_IEEE_802_11A: // FALL-THROUGH
            case AccessNetworkInfo::TYPE_IEEE_802_11B: // FALL-THROUGH
            case AccessNetworkInfo::TYPE_IEEE_802_11G: // FALL-THROUGH
            case AccessNetworkInfo::TYPE_IEEE_802_11N:
                if (pSipConfigV->GetPredefinedPaniForWlan().GetLength() > 0)
                {
                    strHeader = pSipConfigV->GetPredefinedPaniForWlan();
                    IMS_TRACE_D("PANI(wlan) is overwritten by the pre-defined value", 0, 0, 0);
                }
                break;
            default:
                break;
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL PAccessNetworkInfoHeader::FormHeaderForOperatorSpecific(IN IMS_SINT32 nSlotId,
        IN INetworkConnection* piConnection, IN const SIPMethod& /*objMethod*/,
        IN const SIPProfile* pSIPProfile, OUT AString& strHeader)
{
    if (piConnection == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AccessNetworkInfo objANInfo;

    piConnection->GetAccessNetworkInfo(objANInfo);

    if (!FormHeader(nSlotId, objANInfo, strHeader))
    {
        IMS_TRACE_D("Forming PANI header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (SIPConfigProxy::IsInvalidMacAddressRequiredInPaniHeader(nSlotId, pSIPProfile))
    {
        ReformPANIHeaderForATT(objANInfo, strHeader);
    }

    if (strHeader.GetLength() == 0)
    {
        IMS_TRACE_D("PANInfo - length 0", 0, 0, 0);
        return IMS_FALSE;
    }

    if (SIPConfigProxy::IsLocalTimeZoneRequiredInPaniHeader(nSlotId, pSIPProfile))
    {
        ReformPANIHeaderForTEL(strHeader);
    }

    if (IsAccessNetworkTypeWiFi(objANInfo))
    {
        if (IsCountryInfoRequiredForVoWiFi(nSlotId, pSIPProfile))
        {
            ReformPANIHeaderForCountryCode(nSlotId, strHeader, IMS_FALSE);
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL
IMS_BOOL PAccessNetworkInfoHeader::FormHeaderForOperatorSpecific(IN IMS_SINT32 nSlotId,
        IN const IPAddress &objIP, IN const SIPMethod& objMethod,
        IN const SIPProfile* pSIPProfile, OUT AString& strHeader)
{
    INetworkConnection* piConnection = NetworkService::GetNetworkService()->FindConnection(objIP);

    if (piConnection != IMS_NULL)
    {
        return FormHeaderForOperatorSpecific(nSlotId,
                piConnection, objMethod, pSIPProfile, strHeader);
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL
void PAccessNetworkInfoHeader::SetHeader(IN IMS_SINT32 nSlotId, IN const IPAddress &objIP,
        IN const SIPProfile* pSIPProfile, IN_OUT ISIPMessage *&piSIPMsg)
{
    if (piSIPMsg == IMS_NULL)
    {
        return;
    }

    INetworkConnection* piConnection = NetworkService::GetNetworkService()->FindConnection(objIP);

    if (piConnection == IMS_NULL)
    {
        return;
    }

    AString strHeader;

    if (!FormHeaderForOperatorSpecific(nSlotId,
            piConnection, piSIPMsg->GetMethod(), pSIPProfile, strHeader))
    {
        return;
    }

    if (strHeader.GetLength() > 0)
    {
        if (piSIPMsg->SetHeader(ISIPHeader::P_ACCESS_NETWORK_INFO,
                strHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting P-Access-Network-Info header failed", 0, 0, 0);
        }
    }

    SetPrivateHeaderForTMUS(nSlotId, piConnection, piSIPMsg);

    if (piConnection->IsePDGEnabled())
    {
        SetPrivateHeaderForMTS(nSlotId, piConnection, piSIPMsg);
        SetCNIHeader(nSlotId, piConnection, pSIPProfile, piSIPMsg);
    }
}

PRIVATE GLOBAL
void PAccessNetworkInfoHeader::ReformPANIHeaderForATT(
        IN CONST AccessNetworkInfo &objANInfo, IN_OUT AString &strPANIHeader)
{
    IMS_BOOL bExtraParamRequired = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    switch (objANInfo.nType)
    {
    case AccessNetworkInfo::TYPE_IEEE_802_11: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11A: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11B: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11G: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11N:
        bExtraParamRequired = IMS_TRUE;
        break;
    default:
        break;
    }

    if (bExtraParamRequired)
    {
        strPANIHeader = "IEEE-802.11;i-wlan-node-id=000000000000";
    }
}

PRIVATE GLOBAL
void PAccessNetworkInfoHeader::ReformPANIHeaderForTEL(IN_OUT AString &strHeader)
{
    //---------------------------------------------------------------------------------------------

    if (strHeader.GetLength() == 0)
    {
        return;
    }

    ISystemTime *piST = SystemTimeService::GetSystemTimeService()->GetSystemTime();

    if (piST != IMS_NULL)
    {
        // "YYYY-MM-DDTHH:MM:SS+09:00"
        AString strUTCFormat = piST->GetUtcFormat(IMS_TRUE);

        if (strUTCFormat.GetLength() > 0)
        {
            strHeader.Append(";local-time-zone=");
            strHeader.Append('\"');
            strHeader.Append(strUTCFormat);
            strHeader.Append('\"');
        }
    }
}

PRIVATE GLOBAL
void PAccessNetworkInfoHeader::ReformPANIHeaderForCountryCode(IN IMS_SINT32 nSlotId,
        IN_OUT AString &strHeader, IN IMS_BOOL bUseUICC)
{
    AString strCountry(AString::ConstEmpty());

    ILocationInfo* piLocationInfo
            = PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(nSlotId);
    ILocationProperties* piLocation = (piLocationInfo != IMS_NULL)
            ? piLocationInfo->GetLocationProperties(ILocationInfo::LOCATION_POSITION_N_COUNTRY)
            : IMS_NULL;

    if (piLocation != IMS_NULL)
    {
        strCountry = piLocation->GetCountry();
    }

    if ((strCountry.GetLength() == 0) || (strCountry.Equals("ZZ")))
    {
        if (bUseUICC)
        {
            ISubscriberInfo* piSubsInfo
            = PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);
            if (piSubsInfo != IMS_NULL)
            {
                // location information from mcc/mnc in uicc.
                // it could be different from user's location.
                piSubsInfo->GetCountry(strCountry);
            }
        }
    }

    if ((strCountry.GetLength() > 0) && (!strCountry.Equals("ZZ")))
    {
        strHeader.Append(";country=");
        strHeader.Append(strCountry);
    }
}

PRIVATE GLOBAL
void PAccessNetworkInfoHeader::SetPrivateHeaderForTMUS(IN IMS_SINT32 nSlotId,
        IN INetworkConnection *piConnection, IN_OUT ISIPMessage *&piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piConnection == IMS_NULL)
    {
        return;
    }

    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    if (objMethod.Equals(SIPMethod::ACK) || objMethod.Equals(SIPMethod::CANCEL))
    {
        return;
    }

    IMS_BOOL bPLANIRequired = IMS_TRUE;
    AString strPolicyName;
    const AString EXTRA_POLICY_NAME("policy_name");

    if (piConnection->GetExtraInfo(EXTRA_POLICY_NAME, strPolicyName))
    {
        NetworkServicePolicy *pNSP = NetworkServicePolicy::GetInstance();
        const NetworkPolicy *pPolicy = pNSP->GetPolicy(NetworkPolicy::APN_EMERGENCY);

        if ((pPolicy != IMS_NULL) && strPolicyName.Equals(pPolicy->GetName()))
        {
            // Don't set P-Last-Access-Network-Info if the message is sent by emergency PDN.
            bPLANIRequired = IMS_FALSE;
        }
    }

    ////////
    // P-Last-Access-Network-Info header -- starts
    ////////
    if (bPLANIRequired)
    {
        const AString P_LAST_ACCESS_NETWORK_INFO("P-Last-Access-Network-Info");
        ISIPRTConfigHelper *piRTConfigHelper = SIPFactory::GetRTConfigHelper();
        const SIPRTConfig::Header *pHeader
                = piRTConfigHelper->GetHeader(P_LAST_ACCESS_NETWORK_INFO);
        do
        {
            if (pHeader == IMS_NULL)
            {
                // no-op
                break;
            }

            AString strLastPANInfo;
            AString strTimeStamp;
            AString strCellInfoAge;
            AccessNetworkInfo objANInfo;

            piConnection->GetLastAccessNetworkInfo(objANInfo, strTimeStamp, strCellInfoAge);

            if (!FormHeader(nSlotId, objANInfo, strLastPANInfo))
            {
                IMS_TRACE_D("PLANInfo - FormHeader() returns FALSE", 0, 0, 0);
                break;
            }

            if (strLastPANInfo.GetLength() == 0)
            {
                IMS_TRACE_D("PLANInfo - length 0", 0, 0, 0);
                break;
            }

            INetworkWatcher* piNetworkWatcher =
                    PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(nSlotId);

            IMS_SINT32 nNetworkType = piNetworkWatcher->GetNetworkType();

            if (nNetworkType == INetworkWatcher::RADIOTECH_TYPE_UNKNOWN)
            {
                // Timestamp for last known cell identity
                strTimeStamp.Replace(':', "%3A");

                strLastPANInfo.Append(';');
                strLastPANInfo.Append('\"');
                strLastPANInfo.Append(strTimeStamp);
                strLastPANInfo.Append('\"');

                // Timestamp for IMS-REG over WiFi (ePDG)
                if (pHeader->strParameter.GetLength() > 0)
                {
                    strLastPANInfo.Append(';');
                    strLastPANInfo.Append(pHeader->strParameter);
                }
            }

            if (piSIPMsg->SetHeader(ISIPHeader::UNKNOWN,
                    strLastPANInfo, P_LAST_ACCESS_NETWORK_INFO) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Setting %s header failed",
                        P_LAST_ACCESS_NETWORK_INFO.GetStr(), 0, 0);
            }
        } while (0);
    }
    ////////
    // P-Last-Access-Network-Info header -- ends
    ////////
}

PRIVATE GLOBAL
void PAccessNetworkInfoHeader::SetPrivateHeaderForMTS(IN IMS_SINT32 nSlotId,
        IN INetworkConnection *piConnection, IN_OUT ISIPMessage *&piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piConnection == IMS_NULL)
    {
        return;
    }

    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    if (objMethod.Equals(SIPMethod::ACK)
            || objMethod.Equals(SIPMethod::CANCEL))
    {
        return;
    }

    const AString strHeaderName("P-Last-Cell-ID");
    ISIPRTConfigHelper *piRTConfigHelper = SIPFactory::GetRTConfigHelper(nSlotId);
    const SIPRTConfig::Header *pHeader
            = piRTConfigHelper->GetHeader(strHeaderName);

    if (pHeader == IMS_NULL)
    {
        return;
    }

    AString strLastKnownPANInfo;
    AString strTimeStamp;
    AString strCellInfoAge;
    AccessNetworkInfo objANInfo;

    piConnection->GetLastAccessNetworkInfo(objANInfo, strTimeStamp, strCellInfoAge);

    if (!FormHeader(nSlotId, objANInfo, strLastKnownPANInfo))
    {
        IMS_TRACE_D("SetPrivateHeaderForMTS :: FormHeader() returns FALSE", 0, 0, 0);
        return;
    }

    if (strLastKnownPANInfo.GetLength() == 0)
    {
        IMS_TRACE_D("SetPrivateHeaderForMTS :: length 0", 0, 0, 0);
        return;
    }

    // Timestamp for last known cell identity
    strLastKnownPANInfo.Append(';');
    strLastKnownPANInfo.Append('\"');
    strLastKnownPANInfo.Append(strTimeStamp);
    strLastKnownPANInfo.Append('\"');

    // Timestamp for IMS-REG over WiFi (ePDG)
    if (pHeader->strParameter.GetLength() > 0)
    {
        strLastKnownPANInfo.Append(';');
        strLastKnownPANInfo.Append(pHeader->strParameter);
    }

    if (piSIPMsg->SetHeader(ISIPHeader::UNKNOWN,
            strLastKnownPANInfo, strHeaderName) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "SetPrivateHeaderForMTS :: Setting %s header failed",
                strHeaderName.GetStr(), 0, 0);
    }
}

PRIVATE GLOBAL
void PAccessNetworkInfoHeader::SetCNIHeader(IN IMS_SINT32 nSlotId,
        IN INetworkConnection *piConnection,
        IN const SIPProfile* pSIPProfile, IN_OUT ISIPMessage *&piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    (void)pSIPProfile;

    if (piConnection == IMS_NULL)
    {
        return;
    }

    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    if (objMethod.Equals(SIPMethod::CANCEL))
    {
        return;
    }
    else if (objMethod.Equals(SIPMethod::ACK)
            && !SIPFeatures::IsPANIHeaderForAckRequired(nSlotId))
    {
        return;
    }

    // If runtime condition is required, then use SIPProfile.
    IMS_BOOL bCNIHeaderRequired = SIPConfigProxy::IsCellularNetworkInfoHeaderRequired(
            nSlotId, IMS_NULL/*pSIPProfile*/);
    const AString strHeaderName("Cellular-Network-Info");

    if (!bCNIHeaderRequired)
    {
        ISIPRTConfigHelper *piRTConfigHelper = SIPFactory::GetRTConfigHelper(nSlotId);
        const SIPRTConfig::Header* pHeader = piRTConfigHelper->GetHeader(strHeaderName);

        if (pHeader == IMS_NULL)
        {
            return;
        }
    }

    AString strHeader;
    AString strTimeStamp;
    AString strCellInfoAge;
    AccessNetworkInfo objANInfo;

    piConnection->GetLastAccessNetworkInfo(objANInfo, strTimeStamp, strCellInfoAge);

    if (!FormHeader(nSlotId, objANInfo, strHeader))
    {
        return;
    }

    if (strHeader.GetLength() == 0)
    {
        return;
    }

    // The relative time since the information about the cell identity was collected by the UE
    strHeader.Append(";cell-info-age=");
    strHeader.Append(strCellInfoAge);

    if (piSIPMsg->SetHeader(ISIPHeader::UNKNOWN,
            strHeader, strHeaderName) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "SetCNIHeader :: Setting %s header failed",
                strHeaderName.GetStr(), 0, 0);
    }
}

PRIVATE GLOBAL
IMS_BOOL PAccessNetworkInfoHeader::IsAccessNetworkTypeWiFi(IN CONST AccessNetworkInfo &objANInfo)
{
    switch (objANInfo.nType)
    {
    case AccessNetworkInfo::TYPE_IEEE_802_11: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11A: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11B: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11G: // FALL-THROUGH
    case AccessNetworkInfo::TYPE_IEEE_802_11N:
        return IMS_TRUE;
    default:
        break;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL
IMS_BOOL PAccessNetworkInfoHeader::IsCountryInfoRequiredForVoWiFi(IN IMS_SINT32 nSlotId,
        IN const SIPProfile* pSIPProfile)
{
    (void)pSIPProfile;

    // If runtime condition is required, then use SIPProfile.
    IMS_BOOL bCountryInfoRequired = SIPConfigProxy::IsCountryInfoRequiredInPANIHeader(
            nSlotId, IMS_NULL/*pSIPProfile*/);

    return bCountryInfoRequired;
}
