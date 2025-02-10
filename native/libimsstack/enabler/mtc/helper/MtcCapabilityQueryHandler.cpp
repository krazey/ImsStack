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

#include "AString.h"
#include "AStringBuffer.h"
#include "CarrierConfig.h"
#include "ICapabilities.h"
#include "ICoreService.h"
#include "IMessage.h"
#include "IMessageBodyPart.h"
#include "IMtcContext.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ImsAosParameter.h"
#include "ImsList.h"
#include "ImsTypeDef.h"
#include "SdpMediaDescription.h"
#include "SdpOrigin.h"
#include "SdpSessionDescription.h"
#include "ServiceTrace.h"
#include "Sip.h"
#include "SipHeaderName.h"
#include "SipParsingHelper.h"
#include "common/ICoreServiceConfig.h"
#include "common/IMediaConfig.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/MtcCapabilityQueryHandler.h"
#include "offeranswer/SdpAvCodec.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcCapabilityQueryHandler::MtcCapabilityQueryHandler(IN IMtcContext& objContext,
        IN const ICoreServiceConfig* piCoreServiceConfig, IN const IMediaConfig* piMediaConfig) :
        m_objContext(objContext),
        m_piCoreServiceConfig(piCoreServiceConfig),
        m_piMediaConfig(piMediaConfig)
{
}

PUBLIC VIRTUAL MtcCapabilityQueryHandler::~MtcCapabilityQueryHandler() {}

PUBLIC VIRTUAL IMS_RESULT MtcCapabilityQueryHandler::MessageMediator_AdjustMessage(
        IN_OUT ISipMessage* piSipMessage, IN IMS_SINT32 /*nMessage*/)
{
    // invoked only case VZW - USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE is on

    ISipHeader* piContactHeader = SipParsingHelper::CreateHeader(
            ISipHeader::CONTACT_NORMAL, piSipMessage->GetHeader(ISipHeader::CONTACT_NORMAL));
    piContactHeader->RemoveParameter("text");
    piSipMessage->SetHeader(ISipHeader::CONTACT_NORMAL, piContactHeader->GetHeaderValue());

    IMS_TRACE_D("HandleIncomingCapabilityQuery remove text", 0, 0, 0);
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT MtcCapabilityQueryHandler::HandleIncomingCapabilityQuery(
        IN ICoreService* piService, IN ICapabilities* piCapabilities, IN IMS_UINT32 nFeatures)
{
    IMS_TRACE_D("HandleIncomingCapabilityQuery", 0, 0, 0);
    if (piCapabilities == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    IMessage* piMessage = piCapabilities->GetNextResponse();
    if (piMessage == IMS_NULL)
    {
        piCapabilities->Destroy();
        return IMS_FAILURE;
    }

    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_USE_CARRIER_SPECIFIC_CONTACT_HEADER_FOR_OPTIONS_RESPONSE_BOOL) ==
            IMS_TRUE)
    {
        piCapabilities->SetMessageMediator(this);
    }

    SetHeaderForCapabilityQuery(piMessage);
    IMS_SINT32 nFlags = ICapabilities::FLAG_RESPONSE_DEFAULT;
    if (SetBodyForCapabilityQuery(piService, piMessage, nFeatures) == IMS_FAILURE)
    {
        nFlags |= ICapabilities::FLAG_ADD_SDP_BODY_PART;
    }

    piCapabilities->Accept(nFlags);
    piCapabilities->SetMessageMediator(IMS_NULL);
    piCapabilities->Destroy();
    return IMS_SUCCESS;
}

PRIVATE
void MtcCapabilityQueryHandler::SetHeaderForCapabilityQuery(IN IMessage* piMessage)
{
    piMessage->AddHeader(SipHeaderName::SUPPORTED, MessageUtil::STR_TIMER);
    piMessage->AddHeader(SipHeaderName::SUPPORTED, Sip::STR_100REL);

    if (m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL))
    {
        // checking only voice for precondition is enough.
        piMessage->AddHeader(SipHeaderName::SUPPORTED, MessageUtil::STR_PRECONDITION);
    }
}

PRIVATE
IMS_RESULT MtcCapabilityQueryHandler::SetBodyForCapabilityQuery(
        IN ICoreService* piService, IN IMessage* piMessage, IN IMS_UINT32 nFeatures)
{
    // session-level description
    AString strSDP;
    if (SetSessionLevelDescription(piService, strSDP) == IMS_FAILURE)
    {
        return IMS_FAILURE;
    }

    // audio media description
    const AStringArray& objAudioCap = GetMediaCapability(IMediaConfig::STREAM_AUDIO);

    if (objAudioCap.IsEmpty())
    {
        return IMS_FAILURE;
    }
    strSDP.Append(GetAdjustedCodecList(objAudioCap));

    if (nFeatures & ImsAosFeature::VIDEO)
    {
        // video media description
        SdpMediaDescription objStreamVideoDesc;
        const AStringArray& objVideoCap = GetMediaCapability(IMediaConfig::STREAM_VIDEO);

        if (!objStreamVideoDesc.Decode(objVideoCap))
        {
            IMS_TRACE_E(0, "Decoding a StreamMedia(Video) description failed", 0, 0, 0);
            return IMS_FAILURE;
        }

        strSDP.Append(objStreamVideoDesc.Encode());
    }

    // text is not a default service.

    IMessageBodyPart* pIBodyPart = piMessage->CreateBodyPart();
    pIBodyPart->SetHeader(SipHeaderName::CONTENT_TYPE, Sip::STR_APPLICATION_SDP);
    pIBodyPart->SetContent(ByteArray(strSDP));
    return IMS_SUCCESS;
}

PRIVATE VIRTUAL const AStringArray& MtcCapabilityQueryHandler::GetMediaCapability(
        IN IMS_SINT32 nMediaType) const
{
    if (!m_piCoreServiceConfig || !m_piMediaConfig)
    {
        IMS_TRACE_E(0, "config is null", 0, 0, 0);
        return AStringArray::ConstNull();
    }

    return m_piMediaConfig->GetMediaProfile(m_piCoreServiceConfig->GetMediaProfile(), nMediaType);
}

PRIVATE
IMS_RESULT MtcCapabilityQueryHandler::SetSessionLevelDescription(
        IN ICoreService* piService, OUT AString& strDesc)
{
    SdpSessionDescription objSessionDesc;

    // Create a session-level mandatory descriptions
    if (!objSessionDesc.CreateMandatoryLines(
                SdpOrigin::DEFAULT_USERNAME, piService->GetIpAddress()))
    {
        IMS_TRACE_E(0, "Creating a session descriptor failed", 0, 0, 0);
        return IMS_FAILURE;
    }

    strDesc.Append(objSessionDesc.Encode());
    return IMS_SUCCESS;
}

PRIVATE GLOBAL AString MtcCapabilityQueryHandler::GetAdjustedCodecList(
        IN const AStringArray& objAudioCaps)
{
    SdpMediaDescription objMediaDesc;
    objMediaDesc.Decode(objAudioCaps);

    // TODO: use carrier-config
    // Currently, media always add EVS. They will define how to judge the supportability of EVS
    if (IMS_TRUE)
    {
        return objMediaDesc.Encode();
    }

    //// Look up "rtpmap" attribute and parse all the attributes
    ImsList<SdpAttribute> objRtpMaps = objMediaDesc.GetAttributes(SdpAttribute::RTPMAP);
    ImsList<SdpAvCodec> objAVCodecs;

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
