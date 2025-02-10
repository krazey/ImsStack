/**
 * Copyright (C) 2024 The Android Open Source Project
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

#include "ServiceTrace.h"

#include "MediaNegoUtil.h"
#include "MediaSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC MediaSdpGenerator::MediaSdpGenerator(IN const MEDIA_CONTENT_TYPE eType) :
        m_eType(eType)
{
}

PUBLIC VIRTUAL MediaSdpGenerator::~MediaSdpGenerator()
{
    IMS_TRACE_I("~MediaSdpGenerator()", 0, 0, 0);
}

PROTECTED
void MediaSdpGenerator::GenerateCommonAttributes(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    // clean attr & bandwidth line
    ClearAttributeAndBandwidth(pDescriptor);

    // make"c" &"o" line of session level if IP does not matched
    SetSdpSessionIpAddress(pSessionDescriptor, pProfile);

    // make"m" line
    // ------"m=audio xxxx RTP/AVP 104 110 105 102 108 100"
    SetSdpMediaDescription(pDescriptor, pProfile);

    // make bandwidth
    // ------"b=AS:xx"
    // ------"b=AS:xx"
    // ------"b=AS:xx"
    SetSdpMediaBandwidth(pDescriptor, pProfile);
}

PROTECTED
void MediaSdpGenerator::GenerateRtpMap(
        OUT AString& strRtpMap, OUT AString& strPayloadNum, IN MediaBaseProfile::RtpMap& objRtpMap)
{
    IMS_UINT32 nPayloadNumber = objRtpMap.GetPayloadNumber();
    AString strPayloadType = objRtpMap.GetPayloadType();
    IMS_UINT32 nSamplingRate = objRtpMap.GetSamplingRate();
    IMS_SINT32 nChannel = objRtpMap.GetChannel();

    strPayloadNum.Sprintf("%d", nPayloadNumber);

    strRtpMap.Sprintf("%s/%d", strPayloadType.GetStr(), nSamplingRate);

    if (nChannel > 0)
    {
        AString strChannel;
        strChannel.Sprintf("/%d", nChannel);
        strRtpMap.Append(strChannel);
    }

    IMS_TRACE_D("GenerateRtpMap() - media[%d], RtpMap[%d %s]", m_eType, nPayloadNumber,
            strRtpMap.GetStr());
}

PROTECTED
IMS_SINT32 MediaSdpGenerator::GenerateDirection(
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return MEDIA_DIRECTION_INVALID;
    }

    IMS_SINT32 nDirection = (IMS_SINT32)pProfile->GetDirection();

    pDescriptor->SetDirection(nDirection);
    IMS_TRACE_D("GenerateDirection() - media[%d], direction[%d]", m_eType, nDirection, 0);

    return nDirection;
}

PROTECTED
void MediaSdpGenerator::GenerateSessionLevelDirection(
        OUT ISessionDescriptor* pSessionDescriptor, IN IMS_SINT32 nDirection)
{
    if (pSessionDescriptor == IMS_NULL)
    {
        return;
    }

    if (IS_VALID_MEDIA_DIRECTION(nDirection))
    {
        pSessionDescriptor->SetDirection(nDirection);
    }
}

PROTECTED void MediaSdpGenerator::AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator)
{
    if (str.GetLength() > 0)
    {
        str.Append(separator);
    }
}

PRIVATE void MediaSdpGenerator::ClearAttributeAndBandwidth(OUT IMediaDescriptor* pDescriptor)
{
    if (pDescriptor == IMS_NULL)
    {
        return;
    }

    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);

    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);
}

PRIVATE void MediaSdpGenerator::SetSdpSessionIpAddress(
        OUT ISessionDescriptor* pSessionDescriptor, IN MediaBaseProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->GetIpAddress()))
    {
        IMS_TRACE_D("SetSdpSessionIpAddress() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->GetIpAddress().ToCharString(), 0);

        pSessionDescriptor->SetConnectionAddress(pProfile->GetIpAddress().ToString());
        pSessionDescriptor->SetOriginAddress(pProfile->GetIpAddress().ToString());
    }
}

PRIVATE void MediaSdpGenerator::SetSdpMediaDescription(
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    AStringArray objMediaFormat;
    AString strPayloadNum;

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        MediaBaseProfile::BasePayload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        objMediaFormat.AddElement(strPayloadNum);
    }

    IMS_SINT32 nSdpMediaType = MediaNegoUtil::ConvertMediaTypeToSdpMediaType(m_eType);
    IMS_SINT32 nTransPortType = pProfile->GetTransportType().EqualsIgnoreCase("RTP/AVPF")
            ? SdpMedia::TRANSPORT_RTP_AVPF
            : SdpMedia::TRANSPORT_RTP_AVP;

    // Set transport type and port number
    pDescriptor->SetMediaDescription(
            nSdpMediaType, pProfile->GetDataPort(), nTransPortType, objMediaFormat);

    IMS_TRACE_I("SetSdpMediaDescription() Sdp MediaType[%d], DataPort[%d] TransPortType[%s]",
            nSdpMediaType, pProfile->GetDataPort(), pProfile->GetTransportType().GetStr());
}

PRIVATE void MediaSdpGenerator::SetSdpMediaBandwidth(
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nAs = pProfile->GetBandwidthAs();
    IMS_SINT32 nRs = pProfile->GetBandwidthRs();
    IMS_SINT32 nRr = pProfile->GetBandwidthRr();

    IMS_TRACE_I("SetSdpMediaBandwidth() AS[%d], RS[%d] RR[%d]", nAs, nRs, nRr);

    if (nAs > 0)
    {
        pDescriptor->AddBandwidth(SdpBandwidth::TYPE_AS, nAs);

        if (nRs >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RS, nRs);
        }

        if (nRr >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RR, nRr);
        }
    }
}
