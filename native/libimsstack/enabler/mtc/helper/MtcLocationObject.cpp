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
#include "Configuration.h"
#include "GeolocationHelper.h"
#include "GeolocationPidfCreator.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "INetworkWatcher.h"
#include "ISipMessage.h"
#include "ISubscriberConfig.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "call/IMtcCallContext.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcAosConnector.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR CONTENT_TYPE_PIDF_XML[] = "application/pidf+xml";
LOCAL const IMS_CHAR GEOLOCATION_ROUTING_NO[] = "no";
LOCAL const IMS_CHAR GEOLOCATION_ROUTING_YES[] = "yes";
LOCAL const IMS_CHAR CONTENT_DISPOSITION_RENDER[] = "render";
LOCAL const IMS_CHAR CONTENT_DISPOSITION_HANDLING_OPTIONAL[] = "handling=optional";

LOCAL IMS_SINT32 GetGeolocationPidfAllowedType(IN IMS_BOOL bEmergency, IN IMS_BOOL bWifi)
{
    if (bWifi)
    {
        if (bEmergency)
        {
            return CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI;
        }
        else
        {
            return CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI;
        }
    }
    else
    {
        if (bEmergency)
        {
            return CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR;
        }
        else
        {
            return CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR;
        }
    }
}

LOCAL
IMS_BOOL IsWifiRegistered(IN IMtcAosConnector* pAosConnector)
{
    IMS_UINT32 nAosRegisteredNetworkType =
            pAosConnector ? pAosConnector->GetRegisteredNetworkType() : NW_REPORT_RADIO_INVALID;

    return nAosRegisteredNetworkType == NW_REPORT_RADIO_WLAN;
}

PUBLIC
MtcLocationObject::MtcLocationObject(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC MtcLocationObject::~MtcLocationObject() {}

PUBLIC GLOBAL IMS_BOOL MtcLocationObject::IsGeolocationInfoRequired(IN IMtcCallContext& objContext)
{
    IMS_SINT32 nType = GetGeolocationPidfAllowedType(objContext.GetCallInfo().bEmergency,
            IsWifiRegistered(objContext.GetService().GetAosConnector()));

    return objContext.GetConfigurationProxy().Is(
            Feature::SUPPORT_GEOLOCATION_PIDF_IN_SIP_INVITE, nType);

    // TODO: Check if we can remove this SuppType
    // return objContext.GetSupplementaryService().Get(SuppType::GEOLOCATION)->bValue;
}

PUBLIC
void MtcLocationObject::SetLocationToMessage(
        IN_OUT IMessage& objMessage, IN IMS_BOOL bGeolocationRouting)
{
    ByteArray objContent = CreateLocationBody();
    if (objContent.GetLength() <= 0)
    {
        IMS_TRACE_I("SetLocationToMessage : Creating a location information failed", 0, 0, 0);
        return;
    }

    const AString strCid =
            CreateCid(*Configuration::GetInstance()->GetSubscriberConfig(m_objContext.GetSlotId()));

    objMessage.AddHeader(SipHeaderName::GEOLOCATION, GetGeolocationHeader(strCid));
    objMessage.AddHeader(SipHeaderName::GEOLOCATION_ROUTING,
            bGeolocationRouting ? GEOLOCATION_ROUTING_YES : GEOLOCATION_ROUTING_NO);

    ISipMessageBodyPart* piBodyPart = objMessage.GetMessage()->CreateSdpBodyPart();
    piBodyPart->SetContent(objContent);
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_UNKNOWN, GetContentLengthHeader(objContent),
            SipHeaderName::CONTENT_LENGTH);
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_ID, GetContentIdHeader(strCid));
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_TYPE, CONTENT_TYPE_PIDF_XML);
    piBodyPart->SetHeader(ISipMessageBodyPart::CONTENT_DISPOSITION, GetContentDispositionHeader());
}

PRIVATE
AString MtcLocationObject::CreateCid(IN const ISubscriberConfig& objSubscriberConfig) const
{
    AString strCid;
    MessageUtil::GenerateContentId(objSubscriberConfig.GetHomeDomainName(), strCid);
    return strCid;
}

PRIVATE
ByteArray MtcLocationObject::CreateLocationBody() const
{
    ByteArray objContent;

    GeolocationPidfCreator* pPidfCreator =
            GeolocationHelper::GetInstance()->GetPidfCreator(m_objContext.GetSlotId());
    if (pPidfCreator == IMS_NULL)
    {
        return objContent;
    }

    IMS_SINT32 nInformationLevel = GetInformationLevel();
    if (nInformationLevel == CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG)
    {
        pPidfCreator->CreateWithoutCivic(AString::ConstNull(), objContent);
    }
    else if (nInformationLevel ==
            CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC)
    {
        pPidfCreator->CreateWithPosition(AString::ConstNull(), objContent);
    }
    else  // GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY
    {
        pPidfCreator->CreateWithoutPosition(
                AString::ConstNull(), IMS_FALSE, IMS_FALSE, objContent);
    }

    return objContent;
}

PRIVATE
IMS_SINT32 MtcLocationObject::GetInformationLevel() const
{
    return m_objContext.GetConfigurationProxy().GetInt(
            Feature::INFORMATION_LEVEL_OF_GEOLOCATION_PIDF, m_objContext.GetCallInfo().bEmergency,
            IsWifiRegistered(m_objContext.GetService().GetAosConnector()));
}

PRIVATE
AString MtcLocationObject::GetGeolocationHeader(IN const AString& strCid) const
{
    AString strGeolocationHeader;
    strGeolocationHeader.Sprintf("<cid:%s>", strCid.GetStr());
    return strGeolocationHeader;
}

PRIVATE
AString MtcLocationObject::GetContentLengthHeader(IN const ByteArray& objContent) const
{
    AString strContentLength;
    strContentLength.SetNumber(objContent.GetLength());
    return strContentLength;
}

PRIVATE
AString MtcLocationObject::GetContentIdHeader(IN const AString& strCid) const
{
    AString strContentIdHeader;
    strContentIdHeader.Sprintf("<%s>", strCid.GetStr());
    return strContentIdHeader;
}

PRIVATE
AString MtcLocationObject::GetContentDispositionHeader() const
{
    AString strHeader;
    strHeader.Sprintf("%s;%s", CONTENT_DISPOSITION_RENDER, CONTENT_DISPOSITION_HANDLING_OPTIONAL);
    return strHeader;
}
