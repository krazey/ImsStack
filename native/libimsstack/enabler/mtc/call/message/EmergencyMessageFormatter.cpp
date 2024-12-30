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

#include "CarrierConfig.h"
#include "ICoreService.h"
#include "IFeatureCaps.h"
#include "IImsAosInfo.h"
#include "IMtcApp.h"
#include "IMtcService.h"
#include "INetworkConnection.h"
#include "ISipHeader.h"
#include "ISubscriberConfig.h"
#include "ServiceNetwork.h"
#include "ServiceNetworkPolicy.h"
#include "ServicePhoneInfo.h"
#include "Sip.h"
#include "SipParameter.h"
#include "TextParser.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "call/message/EmergencyMessageFormatter.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString HEADER_P_EMERGENCY_INFO = "P-Emergency-Info";

EmergencyMessageFormatter::EmergencyMessageFormatter(
        IN IMtcCallContext& objContext, IN ISession& objSession) :
        MessageFormatter(objContext, objSession),
        m_eNormalAosRegMode(IImsAosInfo::REG_MODE_UNKNOWN),
        m_eEmergencyAosRegMode(IImsAosInfo::REG_MODE_UNKNOWN)
{
    IMS_TRACE_I("+EmergencyMessageFormatter", 0, 0, 0);
}

PUBLIC VIRTUAL EmergencyMessageFormatter::~EmergencyMessageFormatter()
{
    IMS_TRACE_I("~EmergencyMessageFormatter", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_RESULT EmergencyMessageFormatter::FormStartMessage(IN CallType eCallType)
{
    if (MessageFormatter::FormStartMessage(eCallType) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    m_eNormalAosRegMode = GetAosRegMode(ServiceType::NORMAL);
    m_eEmergencyAosRegMode = GetAosRegMode(ServiceType::EMERGENCY);
    if ((m_eNormalAosRegMode == IImsAosInfo::REG_MODE_UNKNOWN) ||
            (m_eEmergencyAosRegMode == IImsAosInfo::REG_MODE_UNKNOWN))
    {
        return IMS_FAILURE;
    }

    SetPPreferredIdentityHeader();
    SetRecvInfoHeader();
    SetPEmergencyInfoHeader();
    SetSipInstanceFeature();
    SetPComServiceTypeHeader();

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT EmergencyMessageFormatter::FormUpdateMessage(
        IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo)
{
    if (MessageFormatter::FormUpdateMessage(eUpdateType, bIncludeAlertInfo) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    if (eUpdateType == UpdateType::LOCATION)
    {
        SetLocation();
    }

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL void EmergencyMessageFormatter::SetAcceptHeader()
{
    MessageFormatter::SetAcceptHeader();

    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL))
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage,
            MessageUtil::STR_ACCEPT_TYPE_APPLICATION_3GPP_CURRENT_LOCATION_DISCOVERY_XML,
            ISipHeader::ACCEPT);
}

PROTECTED VIRTUAL void EmergencyMessageFormatter::SetCallerIdHeader()
{
    const ImsVector<AString> objConfigNumbers = m_objContext.GetConfigurationProxy().GetStringArray(
            ConfigEmergency::KEY_NUMBER_NEED_OIR_STRING_ARRAY);
    for (IMS_UINT32 i = 0; i < objConfigNumbers.GetSize(); i++)
    {
        if (m_objContext.GetParticipantInfo().GetRemoteNumber().Contains(objConfigNumbers.GetAt(i)))
        {
            IMS_TRACE_I("SetCallerIdHeader - OIR is required for emergency call", 0, 0, 0);
            SetOirHeaders();
            break;
        }
    }
}

PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeader()
{
    if (m_objContext.GetMessageUtils().IsHeaderPresent(
                m_piNextMessage, ISipHeader::P_PREFERRED_IDENTITY))
    {
        return;
    }

    AString strFormat = MtcConfigurationResolver::GetPPreferredIdentityHeaderInInviteForEmergency(
            m_objContext.GetConfigurationProxy(), m_eEmergencyAosRegMode);
    if (strFormat.GetLength() > 0)
    {
        return SetPPreferredIdentityHeaderByFormat(strFormat);
    }

    if ((m_eNormalAosRegMode != IImsAosInfo::REG_MODE_NORMAL) &&
            (m_eNormalAosRegMode != IImsAosInfo::REG_MODE_ADMIN) &&
            (m_eEmergencyAosRegMode != IImsAosInfo::REG_MODE_NORMAL) &&
            (m_eEmergencyAosRegMode != IImsAosInfo::REG_MODE_ADMIN))
    {
        return SetPPreferredIdentityHeaderByDeviceId();
    }

    SetPPreferredIdentityHeaderByUserId();
}

PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeaderByFormat(IN const AString& strFormat)
{
    AString strPpi = strFormat;
    strPpi.Replace("#PUID#", GetPublicUserId());
    strPpi.Replace("#IMEI#", GetDeviceId());
    strPpi.Replace("#IP#", GetLocalAddress());
    strPpi.Replace("#PORT#", GetLocalPort());

    m_objContext.GetMessageUtils().SetHeader(
            m_piNextMessage, strPpi, ISipHeader::P_PREFERRED_IDENTITY);
}

PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeaderByUserId()
{
    ICoreService* piCoreService = GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        return;
    }

    const AStringArray& objUserIds = piCoreService->GetUserIdentities();
    for (IMS_SINT32 i = 0; i < objUserIds.GetCount(); i++)
    {
        SipAddress objSipAddress;
        AString strAddress = objUserIds.GetElementAt(i);
        if (!objSipAddress.Create(strAddress))
        {
            continue;
        }

        m_objContext.GetMessageUtils().SetHeader(
                m_piNextMessage, strAddress, ISipHeader::P_PREFERRED_IDENTITY);
        return;
    }
}

PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeaderByDeviceId()
{
    AString strIpAddress = GetLocalAddress();
    AString strPort = GetLocalPort();
    if (strIpAddress.GetLength() <= 0 || strPort.GetLength() <= 0)
    {
        return;
    }

    AString strImei;
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceId(
            m_objContext.GetSlotId(), strImei);

    LOCAL const IMS_UINT32 LEN_IMEI_TAC = 8;
    LOCAL const IMS_UINT32 LEN_IMEI_SNR = 6;
    strImei = strImei.AlignLeft(LEN_IMEI_TAC + LEN_IMEI_SNR, '0');

    AString strValue;
    strValue.Append(Sip::STR_SIP);
    strValue.Append(TextParser::CHAR_COLON);
    strValue.Append(strImei.GetSubStr(0, LEN_IMEI_TAC));
    strValue.Append(strImei.GetSubStr(LEN_IMEI_TAC, LEN_IMEI_SNR));
    strValue.Append('0');
    strValue.Append(TextParser::STR_AT);

    IpAddress objIpAddress;
    if (!objIpAddress.Parse(strIpAddress) || objIpAddress.IsIPv4Address())
    {
        strValue.Append(strIpAddress);
    }
    else
    {
        strValue.Append(TextParser::STR_LSBRACKET);
        strValue.Append(strIpAddress);
        strValue.Append(TextParser::STR_RSBRACKET);
    }
    strValue.Append(TextParser::CHAR_COLON);
    strValue.Append(strPort);

    m_objContext.GetMessageUtils().SetHeader(
            m_piNextMessage, strValue, ISipHeader::P_PREFERRED_IDENTITY);
}

PRIVATE
void EmergencyMessageFormatter::SetSipInstanceFeature()
{
    if ((m_eNormalAosRegMode == IImsAosInfo::REG_MODE_NORMAL) ||
            (m_eNormalAosRegMode == IImsAosInfo::REG_MODE_ADMIN))
    {
        return;
    }

    ICoreService* piCoreService = GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        return;
    }

    const SipParameter* pParameter = piCoreService->GetInstanceParameter();
    if (pParameter == IMS_NULL)
    {
        return;
    }

    IFeatureCaps* piFeatureCaps = GetIFeatureCaps();
    if (piFeatureCaps == IMS_NULL)
    {
        return;
    }

    piFeatureCaps->AddFeature(pParameter->GetName(), pParameter->GetValue());
}

PRIVATE
void EmergencyMessageFormatter::SetRecvInfoHeader()
{
    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigEmergency::KEY_EMERGENCY_CALL_CURRENT_LOCATION_DISCOVERY_SUPPORTED_BOOL))
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage,
            MessageUtil::STR_PACKAGE_CURRENT_LOCATION_DISCOVERY, ISipHeader::RECV_INFO);
}

PRIVATE
void EmergencyMessageFormatter::SetPEmergencyInfoHeader()
{
    if (!m_objContext.GetService().IsWlanIpCanType())
    {
        return;
    }

    AString strPei = m_objContext.GetConfigurationProxy().GetString(
            ConfigEmergency::KEY_P_EMERGENCY_INFO_HEADER_IN_INVITE_STRING);
    if (strPei.GetLength() <= 0)
    {
        return;
    }

    strPei.Replace("#AID#", GetWifiCallingAddressId());
    strPei.Replace("#IMEI#", GetDeviceId());
    strPei.Replace("#MAC#", GetMacAddress());

    m_objContext.GetMessageUtils().AddValueIfNotExists(
            m_piNextMessage, strPei, ISipHeader::UNKNOWN, HEADER_P_EMERGENCY_INFO);
}

PRIVATE
void EmergencyMessageFormatter::SetPComServiceTypeHeader()
{
    if (!m_objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_CARRIER_SPECIFIC_SIP_HEADERS_STRING_ARRAY,
                MessageUtil::STR_P_COM_SERVICETYPE))
    {
        return;
    }

    if (MtcConfigurationResolver::IsCallHandoverAllowed(m_objContext.GetConfigurationProxy(),
                "IWLAN", "EUTRAN", IMS_TRUE, m_objContext.GetService().IsRoaming()))
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(m_piNextMessage,
            MessageUtil::STR_STATIC_EMERGENCY, ISipHeader::UNKNOWN,
            MessageUtil::STR_P_COM_SERVICETYPE);
}

PRIVATE
IMS_UINT32 EmergencyMessageFormatter::GetAosRegMode(IN ServiceType eServiceType) const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(eServiceType);
    if (pAosConnector == IMS_NULL)
    {
        return IImsAosInfo::REG_MODE_UNKNOWN;
    }

    return pAosConnector->GetRegistrationMode();
}

PRIVATE
AString EmergencyMessageFormatter::GetLocalAddress() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return AString::ConstEmpty();
    }

    return pAosConnector->GetLocalAddress();
}

PRIVATE
AString EmergencyMessageFormatter::GetLocalPort() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(ServiceType::EMERGENCY);
    if (pAosConnector == IMS_NULL)
    {
        return AString::ConstEmpty();
    }

    IMS_UINT32 nPort = pAosConnector->GetLocalPort();
    if (nPort == 0)
    {
        return AString::ConstEmpty();
    }

    AString strPort;
    strPort.Sprintf("%d", nPort);
    return strPort;
}

PRIVATE
AString EmergencyMessageFormatter::GetWifiCallingAddressId() const
{
    return PhoneInfoService::GetPhoneInfoService()
            ->GetCallInfo(m_objContext.GetSlotId())
            ->GetWifiCallingAddressId();
}

PRIVATE
AString EmergencyMessageFormatter::GetDeviceId() const
{
    AString strDeviceId;
    PhoneInfoService::GetPhoneInfoService()->GetDeviceInfo()->GetDeviceId(
            m_objContext.GetSlotId(), strDeviceId);
    return strDeviceId;
}

PRIVATE
AString EmergencyMessageFormatter::GetMacAddress() const
{
    AString strMacAddress;

    INetworkConnection* pNetworkConnection = NetworkService::GetNetworkService()->FindConnection(
            NetworkPolicy::APN_WIFI, m_objContext.GetSlotId());
    if (pNetworkConnection != IMS_NULL)
    {
        pNetworkConnection->GetExtraInfo("mac_address", strMacAddress);
    }

    return strMacAddress;
}

PRIVATE
const AString& EmergencyMessageFormatter::GetPublicUserId() const
{
    const ISubscriberConfig* pConfig = m_objContext.GetSubscriberConfig();
    if (pConfig == IMS_NULL)
    {
        return AString::ConstEmpty();
    }

    return pConfig->GetPublicUserId();
}
