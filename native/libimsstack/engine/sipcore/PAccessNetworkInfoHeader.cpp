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

#include "private/SipConfigV.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipRtConfigHelper.h"
#include "PAccessNetworkInfoHeader.h"
#include "SipAddress.h"
#include "SipConfigProxy.h"
#include "SipFactory.h"
#include "SipFeatures.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

// clang-format off
static const IMS_CHAR* N11[] = {
        "211",
        "311",
        "411",
        "511",
        "611",
        "711",
        "811",
        IMS_NULL
};
// clang-format on

PUBLIC GLOBAL IMS_BOOL PAccessNetworkInfoHeader::FormHeader(IN IMS_SINT32 nSlotId,
        IN INetworkConnection* piConnection, IN const ISipMessage* piSipMsg,
        IN const SipProfile* pSipProfile, OUT AString& strHeader)
{
    if (piConnection == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AccessNetworkInfo objAni;

    piConnection->GetAccessNetworkInfo(objAni);

    if (IsAccessNetworkTypeWiFi(objAni))
    {
        IMS_SINT32 nMacAddressDisplayRule =
                SipConfigProxy::GetHideMacInPaniHeaderPolicy(nSlotId, pSipProfile);

        if (nMacAddressDisplayRule == ISipConfig::HIDE_MAC_IN_PANI ||
                (nMacAddressDisplayRule == ISipConfig::HIDE_MAC_IN_PANI_EXCEPT_N11_AND_ECALL &&
                        !IsMessageForN11OrEmergency(pSipProfile, piSipMsg)))
        {
            // Hide MAC address in PANI header
            IMS_MEM_Memset(&objAni.uniAI.i_wlan_node_id, 0x00, sizeof(I_WLAN_NODE_ID));
        }
    }

    if (!FormHeader(nSlotId, objAni, strHeader))
    {
        IMS_TRACE_D("Forming PANI header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    if (IsAccessNetworkTypeWiFi(objAni))
    {
        if (SipConfigProxy::IsCountryParameterSupportedInPaniHeader(nSlotId, pSipProfile))
        {
            AddCountryParameter(nSlotId, strHeader);
        }
    }

    if (SipConfigProxy::IsLocalTimezoneParameterSupportedInPaniHeader(nSlotId, pSipProfile))
    {
        AddLocalTimezone(strHeader);
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_BOOL PAccessNetworkInfoHeader::FormHeader(IN IMS_SINT32 nSlotId,
        IN const IpAddress& objIpAddr, IN const ISipMessage* piSipMsg,
        IN const SipProfile* pSipProfile, OUT AString& strHeader)
{
    INetworkConnection* piConnection =
            NetworkService::GetNetworkService()->FindConnection(objIpAddr);

    if (piConnection != IMS_NULL)
    {
        return FormHeader(nSlotId, piConnection, piSipMsg, pSipProfile, strHeader);
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL void PAccessNetworkInfoHeader::SetHeader(IN IMS_SINT32 nSlotId,
        IN const IpAddress& objIpAddr, IN const SipProfile* pSipProfile,
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

    if (!FormHeader(nSlotId, piConnection, piSipMsg, pSipProfile, strHeader))
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
        SetPrivateHeaderForPcni(nSlotId, piConnection, piSipMsg);
        SetCniHeader(nSlotId, piConnection, pSipProfile, piSipMsg);
    }
}

PRIVATE GLOBAL IMS_BOOL PAccessNetworkInfoHeader::FormHeader(
        IN IMS_SINT32 nSlotId, IN const AccessNetworkInfo& objAni, OUT AString& strHeader)
{
    if (!AccessNetworkInfoFormatter::Encode(objAni, strHeader))
    {
        return IMS_FALSE;
    }

    const SipConfigV* pSipConfigV =
            DYNAMIC_CAST(const SipConfigV*, SipConfigProxy::GetSipConfigV(nSlotId));

    if (pSipConfigV != IMS_NULL)
    {
        switch (objAni.nType)
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

PRIVATE GLOBAL void PAccessNetworkInfoHeader::AddLocalTimezone(IN_OUT AString& strHeader)
{
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

PRIVATE GLOBAL void PAccessNetworkInfoHeader::AddCountryParameter(
        IN IMS_SINT32 nSlotId, IN_OUT AString& strHeader)
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

    if ((strCountry.GetLength() > 0) && !strCountry.Equals("ZZ"))
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

PRIVATE GLOBAL void PAccessNetworkInfoHeader::SetPrivateHeaderForPcni(
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

    const AString strHeaderName("P-Cellular-Network-Info");
    ISipRtConfigHelper* piRtConfigHelper = SipFactory::GetRtConfigHelper(nSlotId);
    const SipRtConfig::Header* pHeader = piRtConfigHelper->GetHeader(strHeaderName);

    if (pHeader == IMS_NULL)
    {
        return;
    }

    AString strHeader;
    AString strTimestamp;
    AString strCellInfoAge;
    AccessNetworkInfo objAnInfo;

    piConnection->GetLastAccessNetworkInfo(objAnInfo, strTimestamp, strCellInfoAge);

    if (!FormHeader(nSlotId, objAnInfo, strHeader))
    {
        IMS_TRACE_D("P-CNI: FormHeader fails", 0, 0, 0);
        return;
    }

    if (strHeader.GetLength() == 0)
    {
        IMS_TRACE_D("P-CNI: length 0", 0, 0, 0);
        return;
    }

    // The relative time since the information about the cell identity was collected by the UE
    strHeader.Append(";cell-info-age=");
    strHeader.Append(strCellInfoAge);

    if (piSipMsg->SetHeader(ISipHeader::UNKNOWN, strHeader, strHeaderName) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting %s header failed", strHeaderName.GetStr(), 0, 0);
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

    if (objMethod.Equals(SipMethod::CANCEL) ||
            (objMethod.Equals(SipMethod::ACK) && !SipFeatures::IsPaniHeaderForAckRequired(nSlotId)))
    {
        return;
    }

    // If runtime condition is required, then use SipProfile.
    IMS_BOOL bCniHeaderRequired =
            SipConfigProxy::IsCellularNetworkInfoHeaderRequired(nSlotId, IMS_NULL /*pSipProfile*/);
    const AString strHeaderName(SipHeaderName::CELLULAR_NETWORK_INFO);

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
        IN const AccessNetworkInfo& objAni)
{
    switch (objAni.nType)
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

PRIVATE GLOBAL IMS_BOOL PAccessNetworkInfoHeader::IsMessageForN11OrEmergency(
        IN const SipProfile* pSipProfile, IN const ISipMessage* piSipMsg)
{
    if (pSipProfile != IMS_NULL && pSipProfile->IsForEmergency())
    {
        return IMS_TRUE;
    }

    AString strHeader;

    if (piSipMsg->GetType() == ISipMessage::TYPE_REQUEST)
    {
        strHeader = piSipMsg->GetHeader(ISipHeader::TO);
    }
    else
    {
        strHeader = piSipMsg->GetHeader(ISipHeader::FROM);
    }

    ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::TO, strHeader);

    if (piHeader != IMS_NULL)
    {
        SipAddress* pAddress = piHeader->GetSipAddress();
        const SipAddress::UserInfoPart* pUserInfoPart =
                pAddress != IMS_NULL ? pAddress->GetUserInfoPart() : IMS_NULL;
        const AString& strUser =
                pUserInfoPart != IMS_NULL ? pUserInfoPart->GetUser() : AString::ConstNull();

        IMS_SINT32 i = 0;

        while (N11[i] != IMS_NULL)
        {
            if (strUser.Equals(N11[i]))
            {
                piHeader->Destroy();
                return IMS_TRUE;
            }
            ++i;
        }

        piHeader->Destroy();
    }
    return IMS_FALSE;
}
