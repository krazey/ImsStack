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
#include "IImsAosInfo.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "IMtcImsEventReceiver.h"
#include "INetworkWatcher.h"
#include "IPhoneInfoSubscriber.h"
#include "ISubscriberConfig.h"
#include "ImsEventDef.h"
#include "MtcDef.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "TextParser.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/ParticipantInfo.h"
#include "call/message/TemplateFormatter.h"
#include "configuration/MtcConfigurationProxy.h"
#include "configuration/MtcConfigurationResolver.h"
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
LOCAL const IMS_CHAR ANONYMOUS_DOMAIN[] = "anonymous.invalid";

PUBLIC
MtcLocationObject::MtcLocationObject(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC MtcLocationObject::~MtcLocationObject() {}

PUBLIC GLOBAL IMS_BOOL MtcLocationObject::IsGeolocationInfoRequired(IN IMtcCallContext& objContext)
{
    if (IsNoUicc(objContext))
    {
        // Since there's no UICC inserted, this is for non-WFC emergency call cases and
        // default carrier config could have been loaded.
        // Information level will follow ims.information_level_of_geolocation_pidf_int_array.
        // If we need different information levels for each PLMN later, we can extend
        // imsemergency.plmn_allowing_geolocation_pidf_in_sip_invite_no_uicc_string_array.
        return !IsGeolocationBlockedByPlmn(objContext);
    }

    return !IsGeolocationBlockedByConfig(objContext) &&
            !IsGeolocationBlockedBySuppService(objContext);
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
void MtcLocationObject::SetLocationToMessage(
        IN_OUT IMessage& objMessage, IN const ByteArray& objContent)
{
    if (objContent.GetLength() <= 0)
    {
        IMS_TRACE_I("SetLocationToMessage : Content is empty", 0, 0, 0);
        return;
    }

    const AString strCid = CreateCid();

    objMessage.AddHeader(SipHeaderName::GEOLOCATION, GetGeolocationHeader(strCid));

    IMS_SINT32 eGeoRoutingHeaderMode = m_objContext.GetConfigurationProxy().GetInt(
            ConfigIms::KEY_GEOLOCATION_ROUTING_HEADER_MODE_INT);
    if (eGeoRoutingHeaderMode == ConfigIms::GEOLOCATION_HEADER_MODE_INCLUDE_YES_ALWAYS ||
            (eGeoRoutingHeaderMode == ConfigIms::GEOLOCATION_HEADER_MODE_INCLUDE_YES_ON_IWLAN &&
                    m_objContext.GetService().IsWlanIpCanType()))
    {
        objMessage.AddHeader(SipHeaderName::GEOLOCATION_ROUTING, GEOLOCATION_ROUTING_YES);
    }
    else if (eGeoRoutingHeaderMode == ConfigIms::GEOLOCATION_HEADER_MODE_INCLUDE_NO_ALWAYS)
    {
        objMessage.AddHeader(SipHeaderName::GEOLOCATION_ROUTING, GEOLOCATION_ROUTING_NO);
    }

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

    const GeolocationPidfCreator* pPidfCreator =
            GeolocationHelper::GetInstance()->GetPidfCreator(m_objContext.GetSlotId());
    if (pPidfCreator == IMS_NULL)
    {
        return objContent;
    }

    IMS_SINT32 nInformationLevel = GetInformationLevel();
    if (nInformationLevel == ConfigIms::GEOLOCATION_PIDF_INFO_LAT_AND_LONG)
    {
        pPidfCreator->CreateWithoutCivic(AString::ConstNull(), objContent);
    }
    else if (nInformationLevel == ConfigIms::GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC)
    {
        pPidfCreator->CreateWithPosition(AString::ConstNull(), objContent);
    }
    else if (nInformationLevel == ConfigIms::GEOLOCATION_PIDF_INFO_COUNTRY_CODE_ONLY)
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
AString MtcLocationObject::CreateCid() const
{
    AString strCid =
            TemplateFormatter::Format(m_objContext.GetConfigurationProxy().GetString(
                                              ConfigVoice::KEY_CONTENT_ID_FOR_GEOLOCATION_STRING),
                    m_objContext);
    IMS_SINT32 nIndexAt = strCid.GetIndexOf(TextParser::CHAR_AT);
    if (nIndexAt < 1 || ((nIndexAt + 1) == strCid.GetLength()))
    {
        return m_objContext.GetMessageUtils().GenerateContentId(ANONYMOUS_DOMAIN);
    }

    return strCid;
}

PRIVATE
AString MtcLocationObject::CreatePersonId() const
{
    const ISubscriberInfo* pSubscriberInfo =
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
    return MtcConfigurationResolver::GetGeolocationLevel(m_objContext.GetConfigurationProxy(),
            GetGeolocationPidfAllowedType(m_objContext.GetCallInfo().eEmergencyType,
                    m_objContext.GetService().IsWlanIpCanType()),
            m_objContext.GetParticipantInfo().GetRemoteNumber());
}

PRIVATE
AString MtcLocationObject::GetLocationBodyFrom(IN const IMessage& objMessage)
{
    ImsList<IMessageBodyPart*> lstMessageBodies = objMessage.GetBodyParts();
    for (IMS_UINT32 nIndex = 0; nIndex < lstMessageBodies.GetSize(); nIndex++)
    {
        const IMessageBodyPart* pBody = lstMessageBodies.GetAt(nIndex);

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

PRIVATE
IMS_BOOL MtcLocationObject::IsNoUicc(IN const IMtcCallContext& objContext)
{
    const IMtcAosConnector* pAosConnector = objContext.GetService().GetAosConnector();
    const IMS_UINT32 nAosRegistrationMode =
            pAosConnector ? pAosConnector->GetRegistrationMode() : IImsAosInfo::REG_MODE_UNKNOWN;
    IMS_TRACE_D("IsNoUicc : Registration mode [%d]", nAosRegistrationMode, 0, 0);
    return nAosRegistrationMode == IImsAosInfo::REG_MODE_NOUICC;
}

PRIVATE
IMS_BOOL MtcLocationObject::IsGeolocationBlockedByConfig(IN IMtcCallContext& objContext)
{
    const IMS_SINT32 nType = GetGeolocationPidfAllowedType(
            objContext.GetCallInfo().eEmergencyType, objContext.GetService().IsWlanIpCanType());
    if (!objContext.GetConfigurationProxy().Contains(
                ConfigIms::KEY_GEOLOCATION_PIDF_IN_SIP_INVITE_SUPPORT_INT_ARRAY, nType))
    {
        IMS_TRACE_D("IsGeolocationBlockedByConfig : Not allowed for type %d", nType, 0, 0);
        return IMS_TRUE;
    }

    if (objContext.GetConfigurationProxy().Contains(
                ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                ConfigVoice::GEOLOCATION_BLOCK_CONDITION_IN_ROAMING) &&
            objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_ROAMING_STATE) ==
                    IMS_ROAMING_STATE_ON)
    {
        IMS_TRACE_D("IsGeolocationBlockedByConfig : In roaming", 0, 0, 0);
        return IMS_TRUE;
    }

    if (objContext.GetCallInfo().eEmergencyType == EmergencyType::NORMAL_ROUTING &&
            objContext.GetConfigurationProxy().Contains(
                    ConfigVoice::KEY_GEOLOCATION_BLOCK_CONDITION_INT_ARRAY,
                    ConfigVoice::GEOLOCATION_BLOCK_CONDITION_FOR_NORMAL_ROUTING_EMERGENCY_CALL))
    {
        IMS_TRACE_D("IsGeolocationBlockedByConfig : Normal routing emergency call", 0, 0, 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL MtcLocationObject::IsGeolocationBlockedByPlmn(IN IMtcCallContext& objContext)
{
    const INetworkWatcher* pNetworkWatcher =
            PhoneInfoService::GetPhoneInfoService()->GetNetworkWatcher(objContext.GetSlotId());
    const AString strPlmn = pNetworkWatcher->GetNetworkOperator();

    if (strPlmn.IsEmpty() ||
            objContext.GetConfigurationProxy().Contains(
                    ConfigEmergency::
                            KEY_PLMN_ALLOWING_GEOLOCATION_PIDF_IN_SIP_INVITE_NO_UICC_STRING_ARRAY,
                    strPlmn.GetStr()))
    {
        return IMS_FALSE;
    }
    IMS_TRACE_D("IsGeolocationBlockedByPlmn : Blocked by PLMN [%s]", strPlmn.GetStr(), 0, 0);
    return IMS_TRUE;
}

PRIVATE
IMS_BOOL MtcLocationObject::IsGeolocationBlockedBySuppService(IN IMtcCallContext& objContext)
{
    const SuppService* pSuppService =
            objContext.GetSupplementaryService().Get(SuppType::GEOLOCATION);
    return pSuppService != IMS_NULL && !pSuppService->bValue;
}

PRIVATE
IMS_SINT32 MtcLocationObject::GetGeolocationPidfAllowedType(
        IN EmergencyType eEmergencyType, IN IMS_BOOL bWifi)
{
    switch (eEmergencyType)
    {
        case EmergencyType::EMERGENCY_ROUTING:
            return bWifi ? ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI
                         : ConfigIms::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR;
        case EmergencyType::NORMAL_ROUTING:
            return bWifi ? ConfigIms::GEOLOCATION_PIDF_FOR_NORMAL_ROUTING_EMERGENCY_ON_WIFI
                         : ConfigIms::GEOLOCATION_PIDF_FOR_NORMAL_ROUTING_EMERGENCY_ON_CELLULAR;
        default:
            return bWifi ? ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI
                         : ConfigIms::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR;
    }
}
