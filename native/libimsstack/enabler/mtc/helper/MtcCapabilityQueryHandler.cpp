#include "IMSTypeDef.h"
#include "helper/MtcCapabilityQueryHandler.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "IMessage.h"
#include "ICoreService.h"
#include "IMessageBodyPart.h"
#include "ICapabilities.h"
#include "Sip.h"
#include "SipHeaderName.h"
#include "SdpMediaDescription.h"
#include "SdpSessionDescription.h"
#include "offeranswer/SdpAvCodec.h"
#include "utility/MessageUtil.h"
#include "AString.h"
#include "AStringBuffer.h"
#include "Configuration.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL void MtcCapabilityQueryHandler::HandleIncomingCapabilityQuery(
        IN ICoreService* piService, IN ICapabilities* piCapabilities, IN const AString& strAppId,
        IN const AString& strServiceId, IN IMS_SINT32 nSlotId)
{
    if (piCapabilities == IMS_NULL)
    {
        return;
    }

    IMessage* piMessage = piCapabilities->GetNextResponse();
    if (piMessage == IMS_NULL)
    {
        return;
    }

    SetHeaderForCapabilityQuery(piService, piMessage);
    SetBodyForCapabilityQuery(piService, piMessage, strAppId, strServiceId, nSlotId);

    IMS_TRACE_D("HandleIncomingCapabilityQuery success", 0, 0, 0);
    piCapabilities->AcceptEx();
    piCapabilities->Destroy();
}

PRIVATE GLOBAL void MtcCapabilityQueryHandler::SetHeaderForCapabilityQuery(
        IN ICoreService* piService, IN IMessage* piMessage)
{
    piMessage->AddHeader(SipHeaderName::SUPPORTED, MessageUtil::STR_TIMER);
    piMessage->AddHeader(SipHeaderName::SUPPORTED, Sip::STR_100REL);
    // TODO: supportability check.
    piMessage->AddHeader(SipHeaderName::SUPPORTED, MessageUtil::STR_PRECONDITION);

    ISipHeader* piContactHeader = piService->GetContactHeader();
    if (piContactHeader != IMS_NULL)
    {
        piMessage->GetMessage()->SetHeader(
                ISipHeader::CONTACT_NORMAL, piContactHeader->ToStringWithoutName());
        piContactHeader->Destroy();
    }
}

PRIVATE GLOBAL void MtcCapabilityQueryHandler::SetBodyForCapabilityQuery(IN ICoreService* piService,
        IN IMessage* piMessage, IN const AString& strAppId, IN const AString& strServiceId,
        IN IMS_SINT32 nSlotId)
{
    Configuration* pConfig = Configuration::GetInstance();

    const IAppConfig* piAppConfig = pConfig->GetAppConfig(strAppId, nSlotId);
    if (piAppConfig == IMS_NULL)
    {
        return;
    }

    const ICoreServiceConfig* piCoreServiceConfig = piAppConfig->GetCoreServiceConfig(strServiceId);
    if (piCoreServiceConfig == IMS_NULL)
    {
        return;
    }

    const AString strMprofName = piCoreServiceConfig->GetMediaProfile();
    AString strSDP;
    const IMediaConfig* pIMediaConfig = Configuration::GetInstance()->GetMediaConfig(nSlotId);

    // session-level description
    SetSessionLevelDescription(piService, strSDP);

    // audio media description
    const AStringArray& objAudioCap =
            pIMediaConfig->GetMediaProfile(strMprofName, IMediaConfig::STREAM_AUDIO);

    strSDP.Append(GetAdjustedCodecList(objAudioCap));

    // TODO: check vt is supported.
    if (IMS_TRUE)
    {
        // video media description
        SdpMediaDescription objStreamVideoDesc;
        const AStringArray& objVideoCap =
                pIMediaConfig->GetMediaProfile(strMprofName, IMediaConfig::STREAM_VIDEO);

        if (!objStreamVideoDesc.Decode(objVideoCap))
        {
            IMS_TRACE_E(0, "Decoding a StreamMedia(Video) description failed", 0, 0, 0);
            return;
        }

        strSDP.Append(objStreamVideoDesc.Encode());
    }

    IMessageBodyPart* pIBodyPart = piMessage->CreateBodyPart();
    pIBodyPart->SetHeader(SipHeaderName::CONTENT_TYPE, Sip::STR_APPLICATION_SDP);
    pIBodyPart->SetContent(ByteArray(strSDP));
}

PRIVATE GLOBAL void MtcCapabilityQueryHandler::SetSessionLevelDescription(
        IN ICoreService* piService, OUT AString& strDesc)
{
    SdpSessionDescription objSessionDesc;

    // Create a session-level mandatory descriptions
    if (!objSessionDesc.CreateMandatoryLines(
                piService->GetAuthorizedUserId().GetUri(), piService->GetIPAddress()))
    {
        IMS_TRACE_E(0, "Creating a session descriptor failed", 0, 0, 0);
        return;
    }

    strDesc.Append(objSessionDesc.Encode());
}

PRIVATE GLOBAL AString MtcCapabilityQueryHandler::GetAdjustedCodecList(
        IN const AStringArray& objAudioCaps)
{
    SdpMediaDescription objMediaDesc;
    objMediaDesc.Decode(objAudioCaps);

    /* TODO: use carrier-config
    if (UtilService::GetUtilService()->GetFeatureUtil()->IsFeatureSupported(
            IFeatureUtil::FEATURE_MEDIA_EVS) == IMS_TRUE)
    {
        return objMediaDesc.Encode();
    }*/

    //// Look up "rtpmap" attribute and parse all the attributes
    IMSList<SdpAttribute> objRtpMaps = objMediaDesc.GetAttributes(SdpAttribute::RTPMAP);
    IMSList<SdpAvCodec> objAVCodecs;

    for (IMS_UINT32 i = 0; i < objRtpMaps.GetSize(); i++)
    {
        SdpAvCodec objAVCodec;
        objAVCodec.SetParameters(objRtpMaps.GetAt(i).GetAttributeValue(), AString::ConstNull());

        objAVCodecs.Append(objAVCodec);
    }

    //// Remove "EVS" codec
    for (IMS_UINT32 i = 0; i < objAVCodecs.GetSize();)
    {
        const SdpAvCodec& objAVCodec = objAVCodecs.GetAt(i);

        if (objAVCodec.GetName().EqualsIgnoreCase("EVS"))
        {
            IMS_TRACE_D("remove : [%s]", objAVCodecs.GetAt(i).ToSdp().GetStr(), 0, 0);
            objAVCodecs.RemoveAt(i);
        }
        else
        {
            i++;
        }
    }

    if (objRtpMaps.GetSize() != objAVCodecs.GetSize())
    {
        //// Remove the existing "rtpmap" attributes
        for (IMS_UINT32 i = 0; i < objRtpMaps.GetSize(); i++)
        {
            objMediaDesc.RemoveAttribute(objRtpMaps.GetAt(i));
        }

        AStringArray objFormats;

        //// Add "rtpmap" attributes
        for (IMS_UINT32 i = 0; i < objAVCodecs.GetSize(); i++)
        {
            const SdpAvCodec& objAVCodec = objAVCodecs.GetAt(i);
            AString strALine = objAVCodec.ToSdp();  // including '/r/n'
            IMS_TRACE_D("attribute=[%s]", strALine.GetStr(), 0, 0);

            SdpAttribute objRtpMap;
            // remove a= and CRLF
            objRtpMap.Decode(strALine.GetSubStr(2, strALine.GetLength() - 4));

            objMediaDesc.AddAttribute(objRtpMap);

            AString strPayloadType;
            strPayloadType.SetNumber(objAVCodec.GetPayloadType());
            objFormats.AddElement(strPayloadType);  // payload number
        }

        SdpMedia& objMedia = const_cast<SdpMedia&>(objMediaDesc.GetMedia());
        objMedia.SetFormats(objFormats);
    }

    return objMediaDesc.Encode();
}
