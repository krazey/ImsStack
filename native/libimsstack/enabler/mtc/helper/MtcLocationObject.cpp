#include "CarrierConfig.h"
#include "Configuration.h"
#include "GeolocationHelper.h"
#include "GeolocationPidfCreator.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "ISipMessage.h"
#include "ISubscriberConfig.h"
#include "ServiceTrace.h"
#include "SipHeaderName.h"
#include "call/IMtcCallContext.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcLocationObject.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PRIVATE GLOBAL const IMS_CHAR MtcLocationObject::CONTENT_TYPE_PIDF_XML[] = "application/pidf+xml";
const IMS_CHAR MtcLocationObject::HEADER_GEOLOCATION[] = "Geolocation";
const IMS_CHAR MtcLocationObject::HEADER_GEOLOCATION_ROUTING[] = "Geolocation-Routing";
const IMS_CHAR MtcLocationObject::GEOLOCATION_ROUTING_NO[] = "no";
const IMS_CHAR MtcLocationObject::GEOLOCATION_ROUTING_YES[] = "yes";
const IMS_CHAR MtcLocationObject::CONTENT_DISPOSITION_RENDER[] = "render";
const IMS_CHAR MtcLocationObject::CONTENT_DISPOSITION_HANDLING_OPTIONAL[] = "handling=optional";

PUBLIC
MtcLocationObject::MtcLocationObject(IN IMtcCallContext& objContext) :
        m_objContext(objContext)
{
}

PUBLIC MtcLocationObject::~MtcLocationObject() {}

PUBLIC IMS_BOOL MtcLocationObject::IsGeolocationInfoRequired()
{
    IMS_SINT32 nType = GetGeolocationPidfAllowedType(m_objContext.GetCallInfo());
    if (!m_objContext.GetConfigurationProxy().Is(
            Feature::SUPPORT_GEOLOCATION_PIDF_IN_SIP_INVITE, nType))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
    // TODO: Check if we can remove this SuppType
    // return m_objContext.GetSupplementaryService().Get(SuppType::GEOLOCATION)->bValue;
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

    GeolocationPidfCreator& objPidfCreator =
            *GeolocationHelper::GetInstance()->GetPidfCreator(m_objContext.GetSlotId());

    if (GetInformationLevel() == CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG)
    {
        objPidfCreator.CreateWithoutCivic(AString::ConstNull(), objContent);
    }
    else  // CarrierConfig::ImsVoice::GEOLOCATION_PIDF_INFO_LAT_AND_LONG_AND_CIVIC)
    {
        objPidfCreator.CreateWithPosition(AString::ConstNull(), objContent);
    }

    return objContent;
}

PRIVATE
IMS_SINT32 MtcLocationObject::GetGeolocationPidfAllowedType(IN const CallInfo& objCallInfo) const
{
    if (objCallInfo.bWifi)
    {
        if (objCallInfo.bEmergency)
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
        if (objCallInfo.bEmergency)
        {
            return CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR;
        }
        else
        {
            return CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR;
        }
    }
}

PRIVATE
IMS_SINT32 MtcLocationObject::GetInformationLevel() const
{
    const CallInfo& objCallInfo = m_objContext.GetCallInfo();
    return m_objContext.GetConfigurationProxy().GetInt(
            Feature::INFORMATION_LEVEL_OF_GEOLOCATION_PIDF, objCallInfo.bEmergency,
            objCallInfo.bWifi);
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
