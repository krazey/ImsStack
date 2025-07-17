/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "AString.h"
#include "INetworkConnection.h"
#include "ISubscriberConfig.h"
#include "IpAddress.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "call/IMtcCallContext.h"
#include "call/message/TemplateFormatter.h"
#include "device/OsLocationInfo.h"
#include "helper/IMtcAosConnector.h"
#include <functional>

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString ALL_ZERO_ADDRESS("0000:0000:0000:0000");
LOCAL const IMS_UINT8 IMEI_INSERT_INDEX_1 = 8;
LOCAL const IMS_UINT8 IMEI_INSERT_INDEX_2 = 15;

PUBLIC GLOBAL AString TemplateFormatter::Format(
        IN const AString& strFormatString, IN IMtcCallContext& objContext)
{
    AString strResult = strFormatString;
    // clang-format off
    Replace(strResult, "#IMEI#", [&]() { return GetImei(objContext); });
    Replace(strResult, "#IMEIWITHHYPHEN#", [&]() { return GetImeiWithHyphen(objContext); });
    Replace(strResult, "#IMEIASADDRREFID#", [&]() { return GetImeiAsAddressRefId(objContext); });
    Replace(strResult, "#IMSI#", [&]() { return GetImsi(objContext); });
    Replace(strResult, "#MAC#", [&]() { return GetMacAddress(objContext); });
    Replace(strResult, "#IP#", [&]() { return GetIpAddress(objContext); });
    Replace(strResult, "#PORT#", [&]() { return GetPort(objContext); });
    Replace(strResult, "#PUID#", [&]() { return GetPublicUserId(objContext); });
    Replace(strResult, "#AID#", [&]() { return GetWifiCallingAddressId(objContext); });
    Replace(strResult, "#PUID#", [&]() { return GetPublicUserId(objContext); });
    Replace(strResult, "#MSISDN#", [&]() { return GetMsisdn(objContext); });
    Replace(strResult, "#HOME_DOMAIN#", [&]() { return GetHomeDomain(objContext); });
    Replace(strResult, "#UNIQUE_ID#", [&]() { return GetUniqueId(); });
    Replace(strResult, "#MNC#", [&]() { return GetMnc(objContext, 3); });
    Replace(strResult, "#MNC2#", [&]() { return GetMnc(objContext, 2); });
    Replace(strResult, "#MCC#", [&]() { return GetMcc(objContext); });
    // clang-format on
    return strResult;
}

PRIVATE GLOBAL AString TemplateFormatter::GetImei(IN const IMtcCallContext& objContext)
{
    AString strDeviceId;
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceId(
            objContext.GetSlotId(), strDeviceId);

    AString strDeviceIdSpareDigit;
    strDeviceIdSpareDigit.Sprintf("%-14.14s0", strDeviceId.GetStr()).Replace(' ', '0');
    return strDeviceIdSpareDigit;
}

/**
 * The IMEI based identity included in P-Preferred-Identity header shall be encoded according to
 * ABNF of imeival as defined in IETF RFC 7254 below:
 *     imeival  =  tac "-" snr "-" spare
 */
PRIVATE GLOBAL AString TemplateFormatter::GetImeiWithHyphen(IN const IMtcCallContext& objContext)
{
    AString strImei = GetImei(objContext);
    return strImei.Insert(IMEI_INSERT_INDEX_1, '-').Insert(IMEI_INSERT_INDEX_2, '-');
}

/**
* This function formats value of IMEI adding a padding of “0”
* to a 16 digit MAC address.
* For example:
* IMEI: 123456789012345
* address-ref-id: 1234:5678:9012:3450
*/
PRIVATE GLOBAL AString TemplateFormatter::GetImeiAsAddressRefId(IN const IMtcCallContext& objContext)
{
    AString strDeviceId;
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceId(
            objContext.GetSlotId(), strDeviceId);

    AString strImeiAsAddrRefId;
    strImeiAsAddrRefId.Sprintf("%-15.15s0", strDeviceId.GetStr()).Replace(' ', '0');

    for (IMS_UINT8 i = 4; i < strImeiAsAddrRefId.GetLength(); i += 5)
    {
        strImeiAsAddrRefId.Insert(i, TextParser::CHAR_COLON);
    }

    return strImeiAsAddrRefId;
}

PRIVATE GLOBAL AString TemplateFormatter::GetImsi(IN const IMtcCallContext& objContext)
{
    const ISubscriberInfo* pSubscriberInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(objContext.GetSlotId());

    AString strImsi;
    pSubscriberInfo->GetSubscriberId(strImsi);
    return strImsi;
}

PRIVATE GLOBAL AString TemplateFormatter::GetMacAddress(IN const IMtcCallContext& objContext)
{
    AString strMacAddress;

    INetworkConnection* pNetworkConnection = NetworkService::GetNetworkService()->FindConnection(
            NetworkPolicy::APN_WIFI, objContext.GetSlotId());
    if (pNetworkConnection != IMS_NULL)
    {
        pNetworkConnection->GetExtraInfo("mac_address", strMacAddress);
    }

    return strMacAddress;
}

PRIVATE GLOBAL AString TemplateFormatter::GetIpAddress(IN IMtcCallContext& objContext)
{
    ServiceType eType = objContext.GetService().GetServiceType();
    const IMtcAosConnector* pAosConnector = objContext.GetAosConnector(eType);
    if (pAosConnector == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtcAosConnector is null", 0, 0, 0);
        return AString::ConstEmpty();
    }

    AString strAddress = pAosConnector->GetLocalAddress();

    IpAddress objIpAddress;
    if (objIpAddress.Parse(strAddress) && objIpAddress.IsIPv6Address())
    {
        AString strEnclosedAddress;
        strEnclosedAddress += TextParser::CHAR_LSBRACKET;
        strEnclosedAddress += strAddress;
        strEnclosedAddress += TextParser::CHAR_RSBRACKET;
        return strEnclosedAddress;
    }
    return strAddress;
}

PRIVATE GLOBAL AString TemplateFormatter::GetPort(IN IMtcCallContext& objContext)
{
    ServiceType eType = objContext.GetService().GetServiceType();
    const IMtcAosConnector* pAosConnector = objContext.GetAosConnector(eType);
    if (pAosConnector == IMS_NULL)
    {
        IMS_TRACE_E(0, "IMtcAosConnector is null", 0, 0, 0);
        return AString::ConstEmpty();
    }

    IMS_UINT32 nPort = pAosConnector->GetLocalPort();
    if (nPort == 0)
    {
        IMS_TRACE_E(0, "Port is 0", 0, 0, 0);
        return AString::ConstEmpty();
    }

    AString strPort;
    strPort.Sprintf("%d", nPort);
    return strPort;
}

PRIVATE GLOBAL const AString& TemplateFormatter::GetPublicUserId(
        IN const IMtcCallContext& objContext)
{
    const ISubscriberConfig* pConfig = objContext.GetSubscriberConfig();
    if (pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "No subscriber", 0, 0, 0);
        return AString::ConstEmpty();
    }

    const AStringArray& lstPuids = pConfig->GetPublicUserIds();
    if (lstPuids.GetCount() <= 0)
    {
        IMS_TRACE_E(0, "No PUID", 0, 0, 0);
    }

    return lstPuids.GetElementAt(0);
}

PRIVATE GLOBAL AString TemplateFormatter::GetWifiCallingAddressId(
        IN const IMtcCallContext& objContext)
{
    if (IsInUnknownCountry(objContext.GetSlotId()))
    {
        IMS_TRACE_D("country is unknown", 0, 0, 0);
        return ALL_ZERO_ADDRESS;
    }

    AString strAid = PhoneInfoService::GetPhoneInfoService()
                             ->GetCallInfo(objContext.GetSlotId())
                             ->GetWifiCallingAddressId();

    if (strAid.GetLength() == 0)
    {
        IMS_TRACE_D("No AID", 0, 0, 0);
        return ALL_ZERO_ADDRESS;
    }

    return strAid;
}

PRIVATE GLOBAL AString TemplateFormatter::GetMsisdn(IN const IMtcCallContext& objContext)
{
    const ISubscriberInfo* pInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(objContext.GetSlotId());
    if (pInfo == IMS_NULL)
    {
        IMS_TRACE_E(0, "No subscriber", 0, 0, 0);
        return AString::ConstEmpty();
    }

    AString strMsisdn;
    pInfo->GetPhoneNumber(strMsisdn);
    return strMsisdn;
}

PRIVATE GLOBAL AString TemplateFormatter::GetHomeDomain(IN const IMtcCallContext& objContext)
{
    const ISubscriberConfig* pConfig = objContext.GetSubscriberConfig();
    if (pConfig == IMS_NULL)
    {
        IMS_TRACE_E(0, "No subscriber", 0, 0, 0);
        return AString::ConstEmpty();
    }

    return pConfig->GetHomeDomainName();
}

PRIVATE GLOBAL AString TemplateFormatter::GetUniqueId()
{
    const IMS_UINT32 nRandom = IMS_SYS_GetSRandom0();
    const IMS_UINT32 nMicroSeconds = IMS_SYS_GetTimeInMicroSeconds();
    AString strUniqueId;
    strUniqueId.Sprintf("%05x%05x", nMicroSeconds, nRandom);
    return strUniqueId;
}

PRIVATE GLOBAL AString TemplateFormatter::GetMcc(IN const IMtcCallContext& objContext)
{
    const ISubscriberInfo* pSubscriberInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(objContext.GetSlotId());

    AString strMcc;
    pSubscriberInfo->GetSimMcc(strMcc);
    return strMcc;
}

PRIVATE GLOBAL AString TemplateFormatter::GetMnc(
        IN const IMtcCallContext& objContext, IN IMS_UINT32 nLength)
{
    const ISubscriberInfo* pSubscriberInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(objContext.GetSlotId());

    AString strMnc;
    pSubscriberInfo->GetSimMnc(strMnc);
    if (nLength == 3 && strMnc.GetLength() == 2)
    {
        strMnc.Prepend("0");
    }
    return strMnc;
}

PRIVATE GLOBAL void TemplateFormatter::Replace(IN_OUT AString& strText,
        IN const AString& strTemplateLiteral, IN const std::function<AString()>& objSubstitution)
{
    if (!strText.Contains(strTemplateLiteral))
    {
        return;
    }

    strText.Replace(strTemplateLiteral, objSubstitution());
}

PRIVATE GLOBAL IMS_BOOL TemplateFormatter::IsInUnknownCountry(IN IMS_SINT32 nSlotId)
{
    AString strCountry;
    const ILocationInfo* piLocationInfo =
            PhoneInfoService::GetPhoneInfoService()->GetLocationInfo(nSlotId);
    if (piLocationInfo != IMS_NULL)
    {
        strCountry = piLocationInfo->GetLastKnownCountry();
    }

    if ((strCountry.GetLength() == 0) || strCountry.Equals(OsLocationInfo::COUNTRY_ISO_UNKNOWN))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}
