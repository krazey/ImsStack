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
#include "AccessNetworkInfoFormatter.h"
#include "ServiceMemory.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"

#include "private/SipConfigV.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipRtConfigHelper.h"
#include "PAccessNetworkInfoHeader.h"
#include "SipConfigProxy.h"
#include "SipFactory.h"
#include "SipFeatures.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC GLOBAL IMS_BOOL PAccessNetworkInfoHeader::FormHeader(
        IN IMS_SINT32 nSlotId, IN const AccessNetworkInfo& objAnInfo, OUT AString& strHeader)
{
    if (!AccessNetworkInfoFormatter::Encode(objAnInfo, strHeader))
    {
        return IMS_FALSE;
    }

    const SipConfigV* pSipConfigV =
            DYNAMIC_CAST(const SipConfigV*, SipConfigProxy::GetSipConfigV(nSlotId));

    if (pSipConfigV != IMS_NULL)
    {
        switch (objAnInfo.nType)
        {
            case AccessNetworkInfo::TYPE_3GPP_UTRAN_FDD:  // FALL-THROUGH
            case AccessNetworkInfo::TYPE_3GPP_UTRAN_TDD:
                if (pSipConfigV->GetPredefinedPaniForUtran().GetLength() > 0)
                {
                    strHeader = pSipConfigV->GetPredefinedPaniForUtran();
                    IMS_TRACE_D("PANI(utran) is overwritten by the pre-defined value", 0, 0, 0);
                }
                break;
            case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_FDD:  // FALL-THROUGH
            case AccessNetworkInfo::TYPE_3GPP_E_UTRAN_TDD:
                if (pSipConfigV->GetPredefinedPaniForEutran().GetLength() > 0)
                {
                    strHeader = pSipConfigV->GetPredefinedPaniForEutran();
                    IMS_TRACE_D("PANI(eutran) is overwritten by the pre-defined value", 0, 0, 0);
                }
                break;

                // WIFI
            case AccessNetworkInfo::TYPE_IEEE_802_11:   // FALL-THROUGH
            case AccessNetworkInfo::TYPE_IEEE_802_11A:  // FALL-THROUGH
            case AccessNetworkInfo::TYPE_IEEE_802_11B:  // FALL-THROUGH
            case AccessNetworkInfo::TYPE_IEEE_802_11G:  // FALL-THROUGH
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

PUBLIC GLOBAL IMS_BOOL PAccessNetworkInfoHeader::FormHeaderForOperatorSpecific(
        IN IMS_SINT32 nSlotId, IN INetworkConnection* piConnection,
        IN const SipMethod& /*objMethod*/, IN const SipProfile* pSipProfile, OUT AString& strHeader)
{
    if (piConnection == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AccessNetworkInfo objAnInfo;

    piConnection->GetAccessNetworkInfo(objAnInfo);

    if (!FormHeader(nSlotId, objAnInfo, strHeader))
    {
        IMS_TRACE_D("Forming PANI header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (SipConfigProxy::IsInvalidMacAddressRequiredInPaniHeader(nSlotId, pSipProfile))
    {
        ReformPaniHeaderForInvalidMacAddress(objAnInfo, strHeader);
    }

    if (strHeader.GetLength() == 0)
    {
        IMS_TRACE_D("PANInfo - length 0", 0, 0, 0);
        return IMS_FALSE;
    }

    if (SipConfigProxy::IsLocalTimeZoneRequiredInPaniHeader(nSlotId, pSipProfile))
    {
        ReformPaniHeaderForLocalTimeZone(strHeader);
    }

    if (IsAccessNetworkTypeWiFi(objAnInfo))
    {
        if (IsCountryInfoRequiredForVoWiFi(nSlotId, pSipProfile))
        {
            ReformPaniHeaderForCountryCode(nSlotId, strHeader, IMS_FALSE);
        }
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL PAccessNetworkInfoHeader::FormHeaderForOperatorSpecific(
        IN IMS_SINT32 nSlotId, IN const IPAddress& objIpAddr, IN const SipMethod& objMethod,
        IN const SipProfile* pSipProfile, OUT AString& strHeader)
{
    INetworkConnection* piConnection =
            NetworkService::GetNetworkService()->FindConnection(objIpAddr);

    if (piConnection != IMS_NULL)
    {
        return FormHeaderForOperatorSpecific(
                nSlotId, piConnection, objMethod, pSipProfile, strHeader);
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL void PAccessNetworkInfoHeader::SetHeader(IN IMS_SINT32 nSlotId,
        IN const IPAddress& objIpAddr, IN const SipProfile* pSipProfile,
        IN_OUT ISipMessage*& piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    INetworkConnection* piConnection =
            NetworkService::GetNetworkService()->FindConnection(objIpAddr);

    if (piConnection == IMS_NULL)
    {
        return;
    }

    AString strHeader;

    if (!FormHeaderForOperatorSpecific(
                nSlotId, piConnection, piSipMsg->GetMethod(), pSipProfile, strHeader))
    {
        return;
    }

    if (strHeader.GetLength() > 0)
    {
        if (piSipMsg->SetHeader(ISipHeader::P_ACCESS_NETWORK_INFO, strHeader) != IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting P-Access-Network-Info header failed", 0, 0, 0);
        }
    }

    SetPrivateHeaderForPlani(nSlotId, piConnection, piSipMsg);

    if (piConnection->IsePDGEnabled())
    {
        SetPrivateHeaderForPlci(nSlotId, piConnection, piSipMsg);
        SetCniHeader(nSlotId, piConnection, pSipProfile, piSipMsg);
    }
}

PRIVATE GLOBAL void PAccessNetworkInfoHeader::ReformPaniHeaderForInvalidMacAddress(
        IN const AccessNetworkInfo& objAnInfo, IN_OUT AString& strPaniHeader)
{
    switch (objAnInfo.nType)
    {
        case AccessNetworkInfo::TYPE_IEEE_802_11:   // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11A:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11B:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11G:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11N:
            strPaniHeader = "IEEE-802.11;i-wlan-node-id=000000000000";
            break;
        default:
            break;
    }
}

PRIVATE GLOBAL void PAccessNetworkInfoHeader::ReformPaniHeaderForLocalTimeZone(
        IN_OUT AString& strHeader)
{
    if (strHeader.GetLength() == 0)
    {
        return;
    }

    ISystemTime* piSysTime = SystemTimeService::GetSystemTimeService()->GetSystemTime();

    if (piSysTime != IMS_NULL)
    {
        // "YYYY-MM-DDTHH:MM:SS+09:00"
        AString strUtcFormat = piSysTime->GetUtcFormat(IMS_TRUE);

        if (strUtcFormat.GetLength() > 0)
        {
            strHeader.Append(";local-time-zone=");
            strHeader.Append('\"');
            strHeader.Append(strUtcFormat);
            strHeader.Append('\"');
        }
    }
}

PRIVATE GLOBAL void PAccessNetworkInfoHeader::ReformPaniHeaderForCountryCode(
        IN IMS_SINT32 nSlotId, IN_OUT AString& strHeader, IN IMS_BOOL bUseUicc)
{
    AString strCountry(AString::ConstEmpty());

    ILocationInfo* piLocationInfo =
            PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(nSlotId);
    ILocationProperties* piLocation = (piLocationInfo != IMS_NULL)
            ? piLocationInfo->GetLocationProperties(ILocationInfo::LOCATION_POSITION_N_COUNTRY)
            : IMS_NULL;

    if (piLocation != IMS_NULL)
    {
        strCountry = piLocation->GetCountry();
    }

    if ((strCountry.GetLength() == 0) || (strCountry.Equals("ZZ")))
    {
        if (bUseUicc)
        {
            ISubscriberInfo* piSubsInfo =
                    PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(nSlotId);

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

PRIVATE GLOBAL void PAccessNetworkInfoHeader::SetPrivateHeaderForPlani(
        IN IMS_SINT32 nSlotId, IN INetworkConnection* piConnection, IN_OUT ISipMessage*& piSipMsg)
{
    if (piConnection == IMS_NULL)
    {
        return;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (objMethod.Equals(SipMethod::ACK) || objMethod.Equals(SipMethod::CANCEL))
    {
        return;
    }

    IMS_BOOL bPlaniRequired = IMS_TRUE;
    AString strPolicyName;
    const AString EXTRA_POLICY_NAME("policy_name");

    if (piConnection->GetExtraInfo(EXTRA_POLICY_NAME, strPolicyName))
    {
        NetworkServicePolicy* pNsp = NetworkServicePolicy::GetInstance();
        const NetworkPolicy* pPolicy = pNsp->GetPolicy(NetworkPolicy::APN_EMERGENCY, nSlotId);

        if ((pPolicy != IMS_NULL) && strPolicyName.Equals(pPolicy->GetName()))
        {
            // Don't set P-Last-Access-Network-Info if the message is sent by emergency PDN.
            bPlaniRequired = IMS_FALSE;
        }
    }

    if (!bPlaniRequired)
    {
        return;
    }

    ////////
    // P-Last-Access-Network-Info header -- starts
    ////////
    const AString strPlaniHeaderName("P-Last-Access-Network-Info");
    ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(nSlotId);
    const SipRtConfig::Header* pHeader = piRtConfigHelper->GetHeader(strPlaniHeaderName);

    do
    {
        if (pHeader == IMS_NULL)
        {
            // no-op
            break;
        }

        AString strLastPanInfo;
        AString strTimestamp;
        AString strCellInfoAge;
        AccessNetworkInfo objAnInfo;

        piConnection->GetLastAccessNetworkInfo(objAnInfo, strTimestamp, strCellInfoAge);

        if (!FormHeader(nSlotId, objAnInfo, strLastPanInfo))
        {
            IMS_TRACE_D("PLANI: FormHeader fails", 0, 0, 0);
            break;
        }

        if (strLastPanInfo.GetLength() == 0)
        {
            IMS_TRACE_D("PLANI: length 0", 0, 0, 0);
            break;
        }

        INetworkWatcher* piNetworkWatcher =
                PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(nSlotId);

        IMS_SINT32 nNetworkType = piNetworkWatcher->GetNetworkType();

        if (nNetworkType == INetworkWatcher::RADIOTECH_TYPE_UNKNOWN)
        {
            // Timestamp for last known cell identity
            strTimestamp.Replace(':', "%3A");

            strLastPanInfo.Append(';');
            strLastPanInfo.Append('\"');
            strLastPanInfo.Append(strTimestamp);
            strLastPanInfo.Append('\"');

            // Timestamp for IMS-REG over WiFi (ePDG)
            if (pHeader->strParameter.GetLength() > 0)
            {
                strLastPanInfo.Append(';');
                strLastPanInfo.Append(pHeader->strParameter);
            }
        }

        if (piSipMsg->SetHeader(ISipHeader::UNKNOWN, strLastPanInfo, strPlaniHeaderName) !=
                IMS_SUCCESS)
        {
            IMS_TRACE_E(0, "Setting %s header failed", strPlaniHeaderName.GetStr(), 0, 0);
        }
    } while (0);
    ////////
    // P-Last-Access-Network-Info header -- ends
    ////////
}

PRIVATE GLOBAL void PAccessNetworkInfoHeader::SetPrivateHeaderForPlci(
        IN IMS_SINT32 nSlotId, IN INetworkConnection* piConnection, IN_OUT ISipMessage*& piSipMsg)
{
    if (piConnection == IMS_NULL)
    {
        return;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (objMethod.Equals(SipMethod::ACK) || objMethod.Equals(SipMethod::CANCEL))
    {
        return;
    }

    const AString strPlciHeaderName("P-Last-Cell-ID");
    ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(nSlotId);
    const SipRtConfig::Header* pHeader = piRtConfigHelper->GetHeader(strPlciHeaderName);

    if (pHeader == IMS_NULL)
    {
        return;
    }

    AString strLastKnownPanInfo;
    AString strTimestamp;
    AString strCellInfoAge;
    AccessNetworkInfo objAnInfo;

    piConnection->GetLastAccessNetworkInfo(objAnInfo, strTimestamp, strCellInfoAge);

    if (!FormHeader(nSlotId, objAnInfo, strLastKnownPanInfo))
    {
        IMS_TRACE_D("PLCI: FormHeader fails", 0, 0, 0);
        return;
    }

    if (strLastKnownPanInfo.GetLength() == 0)
    {
        IMS_TRACE_D("PLCI: length 0", 0, 0, 0);
        return;
    }

    // Timestamp for last known cell identity
    strLastKnownPanInfo.Append(';');
    strLastKnownPanInfo.Append('\"');
    strLastKnownPanInfo.Append(strTimestamp);
    strLastKnownPanInfo.Append('\"');

    // Timestamp for IMS-REG over WiFi (ePDG)
    if (pHeader->strParameter.GetLength() > 0)
    {
        strLastKnownPanInfo.Append(';');
        strLastKnownPanInfo.Append(pHeader->strParameter);
    }

    if (piSipMsg->SetHeader(ISipHeader::UNKNOWN, strLastKnownPanInfo, strPlciHeaderName) !=
            IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting %s header failed", strPlciHeaderName.GetStr(), 0, 0);
    }
}

PRIVATE GLOBAL void PAccessNetworkInfoHeader::SetCniHeader(IN IMS_SINT32 nSlotId,
        IN INetworkConnection* piConnection, IN const SipProfile* pSipProfile,
        IN_OUT ISipMessage*& piSipMsg)
{
    (void)pSipProfile;

    if (piConnection == IMS_NULL)
    {
        return;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (objMethod.Equals(SipMethod::CANCEL))
    {
        return;
    }
    else if (objMethod.Equals(SipMethod::ACK) && !SipFeatures::IsPaniHeaderForAckRequired(nSlotId))
    {
        return;
    }

    // If runtime condition is required, then use SipProfile.
    IMS_BOOL bCniHeaderRequired =
            SipConfigProxy::IsCellularNetworkInfoHeaderRequired(nSlotId, IMS_NULL /*pSipProfile*/);
    const AString strHeaderName("Cellular-Network-Info");

    if (!bCniHeaderRequired)
    {
        ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(nSlotId);
        const SipRtConfig::Header* pHeader = piRtConfigHelper->GetHeader(strHeaderName);

        if (pHeader == IMS_NULL)
        {
            return;
        }
    }

    AString strHeader;
    AString strTimestamp;
    AString strCellInfoAge;
    AccessNetworkInfo objAnInfo;

    piConnection->GetLastAccessNetworkInfo(objAnInfo, strTimestamp, strCellInfoAge);

    if (!FormHeader(nSlotId, objAnInfo, strHeader))
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

    if (piSipMsg->SetHeader(ISipHeader::UNKNOWN, strHeader, strHeaderName) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "SetCniHeader :: Setting %s header failed", strHeaderName.GetStr(), 0, 0);
    }
}

PRIVATE GLOBAL IMS_BOOL PAccessNetworkInfoHeader::IsAccessNetworkTypeWiFi(
        IN const AccessNetworkInfo& objAnInfo)
{
    switch (objAnInfo.nType)
    {
        case AccessNetworkInfo::TYPE_IEEE_802_11:   // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11A:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11B:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11G:  // FALL-THROUGH
        case AccessNetworkInfo::TYPE_IEEE_802_11N:
            return IMS_TRUE;
        default:
            break;
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL PAccessNetworkInfoHeader::IsCountryInfoRequiredForVoWiFi(
        IN IMS_SINT32 nSlotId, IN const SipProfile* pSipProfile)
{
    (void)pSipProfile;

    // If runtime condition is required, then use SipProfile.
    IMS_BOOL bCountryInfoRequired =
            SipConfigProxy::IsCountryInfoRequiredInPaniHeader(nSlotId, IMS_NULL /*pSipProfile*/);

    return bCountryInfoRequired;
}
