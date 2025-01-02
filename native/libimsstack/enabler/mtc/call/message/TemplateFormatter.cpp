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
#include "call/IMtcCallContext.h"
#include "call/message/TemplateFormatter.h"
#include "helper/IMtcAosConnector.h"
#include <functional>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL AString TemplateFormatter::Format(
        IN const AString& strFormatString, IN IMtcCallContext& objContext)
{
    AString strResult = strFormatString;
    // clang-format off
    Replace(strResult, "#IMEI#", [&]() { return GetImei(objContext); });
    Replace(strResult, "#IMSI#", [&]() { return GetImsi(objContext); });
    Replace(strResult, "#MAC#", [&]() { return GetMacAddress(objContext); });
    Replace(strResult, "#IP#", [&]() { return GetIpAddress(objContext); });
    Replace(strResult, "#PORT#", [&]() { return GetPort(objContext); });
    Replace(strResult, "#PUID#", [&]() { return GetPublicUserId(objContext); });
    Replace(strResult, "#AID#", [&]() { return GetWifiCallingAddressId(objContext); });
    // clang-format on
    return strResult;
}

PRIVATE GLOBAL AString TemplateFormatter::GetImei(IN IMtcCallContext& objContext)
{
    AString strDeviceId;
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceId(
            objContext.GetSlotId(), strDeviceId);
    return strDeviceId;
}

PRIVATE GLOBAL AString TemplateFormatter::GetImsi(IN IMtcCallContext& objContext)
{
    ISubscriberInfo* pSubscriberInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(objContext.GetSlotId());

    AString strImsi;
    pSubscriberInfo->GetSubscriberId(strImsi);
    return strImsi;
}

PRIVATE GLOBAL AString TemplateFormatter::GetMacAddress(IN IMtcCallContext& objContext)
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
    IMtcAosConnector* pAosConnector = objContext.GetAosConnector(eType);
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
    IMtcAosConnector* pAosConnector = objContext.GetAosConnector(eType);
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

PRIVATE GLOBAL const AString& TemplateFormatter::GetPublicUserId(IN IMtcCallContext& objContext)
{
    const ISubscriberConfig* pConfig = objContext.GetSubscriberConfig();
    if (pConfig == IMS_NULL)
    {
        return AString::ConstEmpty();
    }

    return pConfig->GetPublicUserId();
}

PRIVATE GLOBAL AString TemplateFormatter::GetWifiCallingAddressId(IN IMtcCallContext& objContext)
{
    return PhoneInfoService::GetPhoneInfoService()
            ->GetCallInfo(objContext.GetSlotId())
            ->GetWifiCallingAddressId();
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
