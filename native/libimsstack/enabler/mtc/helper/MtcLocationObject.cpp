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

#include "ByteArray.h"
#include "CarrierConfig.h"
#include "DocumentBuilder.h"
#include "DomDocumentBuilderFactory.h"
#include "GeolocationHelper.h"
#include "GeolocationPidfCreator.h"
#include "GeolocationPidfWriter.h"
#include "IDocument.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "ISubscriberConfig.h"
#include "MtcDef.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "TextParser.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcAosConnector.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include "helper/XmlElementWrapper.h"
#include "utility/IMessageUtils.h"

using namespace enabler;

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

PUBLIC
MtcLocationObject::MtcLocationObject(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC MtcLocationObject::~MtcLocationObject() {}

PUBLIC GLOBAL IMS_BOOL MtcLocationObject::IsGeolocationInfoRequired(IN IMtcCallContext& objContext)
{
    IMS_SINT32 nType = GetGeolocationPidfAllowedType(
            objContext.GetCallInfo().IsEmergency(), objContext.GetService().IsWlanIpCanType());

    const SuppService* pSuppService =
            objContext.GetSupplementaryService().Get(SuppType::GEOLOCATION);

    return objContext.GetConfigurationProxy().Is(
                   Feature::SUPPORT_GEOLOCATION_PIDF_IN_SIP_INVITE, nType) &&
            (pSuppService == IMS_NULL || pSuppService->bValue);
}

PUBLIC GLOBAL MtcLocationProperties* MtcLocationObject::GetLocationFromMessage(
        IN const IMessage& objMessage)
{
    AString strXmlBody = GetLocationBodyFrom(objMessage);
    if (strXmlBody.GetLength() <= 0)
    {
        IMS_TRACE_D("GetLocationFromMessage : No PIDF-LO content", 0, 0, 0);
        return IMS_NULL;
    }

    DocumentBuilder* pDocumentBuilder =
            DomDocumentBuilderFactory::GetInstance()->NewDocumentBuilder();
    IDocument* pDocument = pDocumentBuilder->Parse(strXmlBody);
    DomDocumentBuilderFactory::DestroyDocumentBuilder(pDocumentBuilder);
    if (pDocument == IMS_NULL)
    {
        return IMS_NULL;
    }

    XmlElementWrapper objShapeElement = XmlElementWrapper(pDocument->GetDocumentElement())
                                                .GetFirstChild("person")
                                                .GetFirstChild("geopriv")
                                                .GetFirstChild("location-info")
                                                .GetFirstChild();
    while (objShapeElement.IsValid())
    {
        if (!objShapeElement.GetTagName().EqualsIgnoreCase("Circle") &&
                !objShapeElement.GetTagName().EqualsIgnoreCase("Point"))
        {
            objShapeElement = objShapeElement.GetNextSibling();
            continue;
        }

        MtcLocationProperties* pLocation = new MtcLocationProperties();

        AString strPos = objShapeElement.GetFirstChild("pos").GetValue().GetStr();
        ImsList<AString> objPosTokens = strPos.Split(TextParser::CHAR_SP);
        if (objPosTokens.GetSize() >= 2)
        {
            pLocation->SetLatitude(objPosTokens.GetAt(0));
            pLocation->SetLongitude(objPosTokens.GetAt(1));
        }

        pLocation->SetRadius(objShapeElement.GetFirstChild("radius").GetValue());

        return pLocation;
    }

    pDocument->DestroyDocument();
    return IMS_NULL;
}

PUBLIC
void MtcLocationObject::SetLocationToMessage(IN_OUT IMessage& objMessage,
        IN IMS_BOOL bGeolocationRouting, IN const ByteArray& objContent)
{
    if (objContent.GetLength() <= 0)
    {
        IMS_TRACE_I("SetLocationToMessage : Content is empty", 0, 0, 0);
        return;
    }

    const AString strCid = CreateCid(*m_objContext.GetSubscriberConfig());

    objMessage.AddHeader(SipHeaderName::GEOLOCATION, GetGeolocationHeader(strCid));
    objMessage.AddHeader(SipHeaderName::GEOLOCATION_ROUTING,
            bGeolocationRouting ? GEOLOCATION_ROUTING_YES : GEOLOCATION_ROUTING_NO);

    IMessageBodyPart* pBodyPart = objMessage.CreateBodyPart();
    pBodyPart->SetContent(objContent);
    pBodyPart->SetHeader(SipHeaderName::CONTENT_LENGTH, GetContentLengthHeader(objContent));
    pBodyPart->SetHeader(SipHeaderName::CONTENT_ID, GetContentIdHeader(strCid));
    pBodyPart->SetHeader(SipHeaderName::CONTENT_TYPE, CONTENT_TYPE_PIDF_XML);
    pBodyPart->SetHeader(SipHeaderName::CONTENT_DISPOSITION, GetContentDispositionHeader());
}

PUBLIC
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
    else if (nInformationLevel == CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY)
    {
        pPidfCreator->CreateWithoutPosition(AString::ConstNull(), IMS_FALSE, IMS_FALSE, objContent);
    }
    else  // GEOLOCATION_PIDF_INFO_COUNTRY_CODE_AND_STATE
    {
        pPidfCreator->CreateWithoutPosition(AString::ConstNull(), IMS_FALSE, IMS_TRUE, objContent);
    }

    return objContent;
}

PUBLIC
ByteArray MtcLocationObject::CreateCallComposerLocationBody(
        IN const AString& strLatitude, IN const AString& strLongitude) const
{
    const IMS_SINT32 eNamespaces = Presence::Namespace::DM | Presence::Namespace::GP |
            Presence::Namespace::GML | Presence::Namespace::GS;

    AString strEntityUri = GetEntityUri(*m_objContext.GetSubscriberConfig());

    // clang-format off
    return PidfLoXml{
        new Presence{eNamespaces, strEntityUri, {
            new Person{CreatePersonId(), {
                new Geopriv{
                    new LocationInfo{
                        new Circle{strLatitude, strLongitude, ""},
                    },
                    new UsageRules{},
                },
            }},
        }},
    }.Write();
    // clang-format on
}

PRIVATE
AString MtcLocationObject::CreateCid(IN const ISubscriberConfig& objSubscriberConfig) const
{
    return m_objContext.GetMessageUtils().GenerateContentId(
            objSubscriberConfig.GetHomeDomainName());
}

PRIVATE
AString MtcLocationObject::CreatePersonId() const
{
    ISubscriberInfo* pSubscriberInfo =
            PhoneInfoService::GetPhoneInfoService()->GetSubscriberInfo(m_objContext.GetSlotId());
    if (pSubscriberInfo == IMS_NULL)
    {
        IMS_TRACE_E(0, "CreatePersonId : SubscriberInfo is null", 0, 0, 0);
        return AString::ConstNull();
    }

    // No specific requirement for the person ID value
    AString strPersonId;
    pSubscriberInfo->GetPhoneNumber(strPersonId);
    return strPersonId;
}

PRIVATE
IMS_SINT32 MtcLocationObject::GetInformationLevel() const
{
    return m_objContext.GetConfigurationProxy().GetInt(
            Feature::INFORMATION_LEVEL_OF_GEOLOCATION_PIDF,
            m_objContext.GetCallInfo().IsEmergency(), m_objContext.GetService().IsWlanIpCanType(),
            m_objContext.GetConfigurationProxy().Is(
                    Feature::PIDF_SHORT_CODE, m_objContext.GetParticipantInfo().GetRemoteNumber()));
}

PRIVATE
AString MtcLocationObject::GetLocationBodyFrom(IN const IMessage& objMessage)
{
    ImsList<IMessageBodyPart*> lstMessageBodies = objMessage.GetBodyParts();
    for (IMS_UINT32 nIndex = 0; nIndex < lstMessageBodies.GetSize(); nIndex++)
    {
        IMessageBodyPart* pBody = lstMessageBodies.GetAt(nIndex);

        if (pBody->GetHeader(SipHeaderName::CONTENT_TYPE).EqualsIgnoreCase(CONTENT_TYPE_PIDF_XML))
        {
            return pBody->GetContent().ToString();
        }
    }

    return AString::ConstNull();
}

PRIVATE
AString MtcLocationObject::GetGeolocationHeader(IN const AString& strCid)
{
    AString strGeolocationHeader;
    strGeolocationHeader.Sprintf("<cid:%s>", strCid.GetStr());
    return strGeolocationHeader;
}

PRIVATE
AString MtcLocationObject::GetContentLengthHeader(IN const ByteArray& objContent)
{
    AString strContentLength;
    strContentLength.SetNumber(objContent.GetLength());
    return strContentLength;
}

PRIVATE
AString MtcLocationObject::GetContentIdHeader(IN const AString& strCid)
{
    AString strContentIdHeader;
    strContentIdHeader.Sprintf("<%s>", strCid.GetStr());
    return strContentIdHeader;
}

PRIVATE
AString MtcLocationObject::GetContentDispositionHeader()
{
    AString strHeader;
    strHeader.Sprintf("%s;%s", CONTENT_DISPOSITION_RENDER, CONTENT_DISPOSITION_HANDLING_OPTIONAL);
    return strHeader;
}

PRIVATE
AString MtcLocationObject::GetEntityUri(IN const ISubscriberConfig& objSubscriberConfig)
{
    AString strEntityUri;
    strEntityUri.Sprintf("pres:%s", objSubscriberConfig.GetPrivateUserId().GetStr());
    return strEntityUri;
}
