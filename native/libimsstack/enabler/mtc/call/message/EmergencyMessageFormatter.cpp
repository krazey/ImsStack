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
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipParameter.h"
#include "TextParser.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "call/message/EmergencyMessageFormatter.h"
#include "call/message/TemplateFormatter.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
#include "helper/IMtcAosConnector.h"
#include "utility/IMessageUtils.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const AString HEADER_P_EMERGENCY_INFO = "P-Emergency-Info";

EmergencyMessageFormatter::EmergencyMessageFormatter(
        IN IMtcCallContext& objContext, IN ISession& objSession) :
        MessageFormatter(objContext, objSession)
{
    IMS_TRACE_I("+EmergencyMessageFormatter", 0, 0, 0);
}

PUBLIC VIRTUAL EmergencyMessageFormatter::~EmergencyMessageFormatter()
{
    IMS_TRACE_I("~EmergencyMessageFormatter", 0, 0, 0);
}

PUBLIC VIRTUAL IMS_RESULT EmergencyMessageFormatter::FormStartMessage(IN CallType eCallType)
{
    IMS_TRACE_I("FormStartMessage : AosRegMode - Normal[%d] Emergency[%d]",
            GetAosRegMode(ServiceType::NORMAL), GetAosRegMode(ServiceType::EMERGENCY), 0);
    if (GetAosRegMode(ServiceType::NORMAL) == IImsAosInfo::REG_MODE_UNKNOWN ||
            GetAosRegMode(ServiceType::EMERGENCY) == IImsAosInfo::REG_MODE_UNKNOWN)
    {
        return IMS_FAILURE;
    }

    if (MessageFormatter::FormStartMessage(eCallType) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

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
    const ImsVector<AString> objOipString = m_objContext.GetConfigurationProxy().GetStringArray(
            ConfigEmergency::KEY_NUMBER_NEED_OIP_STRING_ARRAY);
    for (IMS_UINT32 i = 0; i < objOipString.GetSize(); i++)
    {
        if (m_objContext.GetParticipantInfo().GetRemoteNumber().Contains(objOipString.GetAt(i)))
        {
            IMS_TRACE_I("SetCallerIdHeader - OIP is required for emergency call", 0, 0, 0);
            m_objContext.GetMessageUtils().SetHeader(
                    m_piNextMessage, MessageUtil::STR_NONE, ISipHeader::PRIVACY);
            return;
        }
    }

    const ImsVector<AString> objOirString = m_objContext.GetConfigurationProxy().GetStringArray(
            ConfigEmergency::KEY_NUMBER_NEED_OIR_STRING_ARRAY);
    for (IMS_UINT32 i = 0; i < objOirString.GetSize(); i++)
    {
        if (m_objContext.GetParticipantInfo().GetRemoteNumber().Contains(objOirString.GetAt(i)))
        {
            IMS_TRACE_I("SetCallerIdHeader - OIR is required for emergency call", 0, 0, 0);
            SetOirHeaders();
            return;
        }
    }
}

PROTECTED VIRTUAL void EmergencyMessageFormatter::SetPPreferredIdentityHeader()
{
    if (m_objContext.GetMessageUtils().IsHeaderPresent(
                m_piNextMessage, ISipHeader::P_PREFERRED_IDENTITY))
    {
        IMS_TRACE_I("SetPPreferredIdentityHeader : Header already present", 0, 0, 0);
        return;
    }

    AString strFormat = MtcConfigurationResolver::GetPPreferredIdentityHeaderInInviteForEmergency(
            m_objContext.GetConfigurationProxy(), GetAosRegMode(ServiceType::EMERGENCY));
    IMS_TRACE_D("SetPPreferredIdentityHeader : Format[%s]", strFormat.GetStr(), 0, 0);
    if (strFormat.GetLength() > 0)
    {
        return SetPPreferredIdentityHeaderByFormat(strFormat);
    }
    return SetPPreferredIdentityHeaderByUserId();
}

PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeaderByFormat(IN const AString& strFormat)
{
    AString strPpi = TemplateFormatter::Format(strFormat, m_objContext);
    m_objContext.GetMessageUtils().SetHeader(
            m_piNextMessage, strPpi, ISipHeader::P_PREFERRED_IDENTITY);
}

PRIVATE
void EmergencyMessageFormatter::SetPPreferredIdentityHeaderByUserId()
{
    const ICoreService* piCoreService = GetICoreService();
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
void EmergencyMessageFormatter::SetSipInstanceFeature()
{
    IMS_UINT32 eNormalAosRegMode = GetAosRegMode(ServiceType::NORMAL);
    if (eNormalAosRegMode == IImsAosInfo::REG_MODE_NORMAL ||
            eNormalAosRegMode == IImsAosInfo::REG_MODE_ADMIN)
    {
        return;
    }

    const ICoreService* piCoreService = GetICoreService();
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

    strPei = TemplateFormatter::Format(strPei, m_objContext);
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
    const IMtcAosConnector* pAosConnector = m_objContext.GetAosConnector(eServiceType);
    if (pAosConnector == IMS_NULL)
    {
        return IImsAosInfo::REG_MODE_UNKNOWN;
    }

    return pAosConnector->GetRegistrationMode();
}
