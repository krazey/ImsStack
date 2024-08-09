// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "audio/AudioNegoAmr.h"
#include "audio/AudioNegoEvs.h"
#include "audio/AudioProfileUtil.h"
#include "audio/AudioSdpGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AudioSdpGenerator::AudioSdpGenerator() :
        SdpGenerator(MEDIA_TYPE_AUDIO)
{
    IMS_TRACE_I("+AudioSdpGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL AudioSdpGenerator::~AudioSdpGenerator()
{
    IMS_TRACE_I("~AudioSdpGenerator()", 0, 0, 0);
}

PUBLIC
IMS_BOOL AudioSdpGenerator::Generate(OUT ISessionDescriptor* pSessionDescriptor,
        OUT IMediaDescriptor* pDescriptor, IN MediaBaseProfile* pBaseProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pBaseProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioProfile* pProfile = static_cast<AudioProfile*>(pBaseProfile);

    IMS_TRACE_I("Generate() - PayloadSize[%d], AS[%d], port[%d]",
            pProfile->GetPayloadList().GetSize(), pProfile->GetBandwidthAs(),
            pProfile->GetDataPort());

    // clean attr & bandwidth line
    pDescriptor->RemoveAttribute(SdpAttribute::ATTRIBUTE_ALL);
    ImsList<AString> strEmptyList;
    pDescriptor->SetBandwidthInfo(strEmptyList);

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

    // make each payload
    // ------"a=rtpmap:104 AMR-WB/16000/1"
    // ------"a=fmtp:110 mode-set=2; octet-align=1"
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpmap, strFmtp, strPayloadNum;

        AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        // set "rtpmap"
        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        strRtpmap.Sprintf("%s/%d", pPayload->GetRtpMap().GetPayloadType().GetStr(),
                pPayload->GetRtpMap().GetSamplingRate());

        if (pPayload->GetRtpMap().GetChannel() > 0)
        {
            AString strChannel;
            strChannel.Sprintf("/%d", pPayload->GetRtpMap().GetChannel());
            strRtpmap.Append(strChannel);
        }

        // set "fmtp"
        if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR"))
        {
            AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pPayload->GetFmtp();
            if (pAmrFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = AudioNegoAmr::SetSdpFmtpFromAmrFmtp(pAmrFmtp);
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
        {
            AudioProfile::TelephoneEventFmtp* pTEFmtp =
                    (AudioProfile::TelephoneEventFmtp*)pPayload->GetFmtp();
            if (pTEFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = pTEFmtp->GetEvents();
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
        {
            AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pPayload->GetFmtp();
            if (pEvsFmtp == IMS_NULL)
            {
                continue;
            }

            strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(pEvsFmtp);
        }
        else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcmu") ||
                pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcma"))
        {
            // set rtpmap, not fmtp
            strFmtp = AString::ConstNull();
            pDescriptor->SetMediaFormat(
                    SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpmap, strFmtp);
            continue;
        }
        else
        {
            continue;
        }

        if (strFmtp.GetLength() == 0)
        {
            // strFmtp = (m_pConfig != IMS_NULL) ? AString::ConstNull() : AString::ConstEmpty();
            strFmtp = AString::ConstNull();
        }

        pDescriptor->SetMediaFormat(SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpmap, strFmtp);
    }

    // set make direction
    pDescriptor->SetDirection(pProfile->GetDirection());

    if (pProfile->GetDirection() > MEDIA_DIRECTION_INVALID &&
            pProfile->GetDirection() <= MEDIA_DIRECTION_SEND_RECEIVE)
    {
        // Set Session Level Direction Attribute according to the media direction
        // (avoid conflict between media and audio)
        pSessionDescriptor->SetDirection(pProfile->GetDirection());
    }

    // set make ptime & maxptime
    if (pProfile->GetPtime() != AudioProfile::AmrFmtp::DEFAULT_PTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::PTIME, pProfile->GetPtime());
    }

    if (pProfile->GetMaxPtime() != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::MAXPTIME, pProfile->GetMaxPtime());
    }

    // set candidate
    for (IMS_UINT32 nIndex = 0; nIndex < pProfile->GetCandidateAttr().GetSize(); nIndex++)
    {
        AString strCandidateAttr = pProfile->GetCandidateAttr().GetAt(nIndex);
        if (strCandidateAttr.GetLength() != 0)
        {
            strCandidateAttr.Sprintf("%d, %s", nIndex + 1, strCandidateAttr.GetStr());
            pDescriptor->AddAttribute(SdpAttribute::CANDIDATE, strCandidateAttr);
        }
    }

    // set RTCP-XR -- RTCP-XR is for VZW, not a negotiation target by VZW requirement
    if (pProfile->IsRtcpXrSupported() == IMS_TRUE &&
            pProfile->GetDirection() == MEDIA_DIRECTION_SEND_RECEIVE)
    {
        if (pProfile->GetRtcpXrAttr().IsStatisticMetricsSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "stat-summary=loss,dup,jitt,HL");
        }
        if (pProfile->GetRtcpXrAttr().IsVoipMetricsSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "voip-metrics");
        }
        if (pProfile->GetRtcpXrAttr().IsPacketLossRleSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "pkt-loss-rle");
        }
        if (pProfile->GetRtcpXrAttr().IsPacketDuplicatedRleSupported())
        {
            pDescriptor->AddAttribute(SdpAttribute::RTCP_XR, "pkt-dup-rle");
        }

        IMS_TRACE_I("Generate() - SupportRtcpXr[%d]", pProfile->IsRtcpXrSupported(), 0, 0);
    }

    if (pProfile->IsAnbrSupported())
    {
        pDescriptor->AddAttribute(SdpAttribute::ANBR, AString::ConstNull());
    }
    else
    {
        IMS_TRACE_D("Generate() - anbr feature is not supported", 0, 0, 0);
    }

    return IMS_TRUE;
}

PRIVATE void AudioSdpGenerator::SetSdpSessionIpAddress(
        OUT ISessionDescriptor* pSessionDescriptor, IN AudioProfile* pProfile)
{
    if (!pSessionDescriptor->GetLocalAddress().Equals(pProfile->GetIpAddress()))
    {
        IMS_TRACE_D("SetSdpSessionIpAddress() - IP does not matched, SessionIP[%s], ProfileIP[%s]",
                pSessionDescriptor->GetLocalAddress().ToCharString(),
                pProfile->GetIpAddress().ToCharString(), 0);

        pSessionDescriptor->SetConnectionAddress(pProfile->GetIpAddress().ToString());
        pSessionDescriptor->SetOriginAddress(pProfile->GetIpAddress().ToString());
    }
}

PRIVATE void AudioSdpGenerator::SetSdpMediaDescription(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    AStringArray objAudioFormat;
    AString strPayloadNum;
    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        strPayloadNum.Sprintf("%d", pPayload->GetRtpMap().GetPayloadNumber());
        objAudioFormat.AddElement(strPayloadNum);
    }

    // Set transport type and port number
    pDescriptor->SetMediaDescription(SdpMedia::TYPE_AUDIO, pProfile->GetDataPort(),
            SdpMedia::TRANSPORT_RTP_AVP, objAudioFormat);
}

PRIVATE void AudioSdpGenerator::SetSdpMediaBandwidth(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pProfile->GetBandwidthAs() > 0)
    {
        pDescriptor->AddBandwidth(SdpBandwidth::TYPE_AS, pProfile->GetBandwidthAs());

        if (pProfile->GetBandwidthRs() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RS, pProfile->GetBandwidthRs());
        }

        if (pProfile->GetBandwidthRr() >= 0)
        {
            pDescriptor->AddBandwidth(SdpBandwidth::TYPE_RR, pProfile->GetBandwidthRr());
        }
    }
}