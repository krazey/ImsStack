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

    IMS_TRACE_I("Generate() - PayloadSize[%d]", pBaseProfile->GetPayloadList().GetSize(), 0, 0);

    GenerateCommonAttributes(pSessionDescriptor, pDescriptor, pBaseProfile);

    AudioProfile* pProfile = static_cast<AudioProfile*>(pBaseProfile);

    GeneratePayload(pDescriptor, pProfile);
    GenerateSessionLevelDirection(pSessionDescriptor, GenerateDirection(pDescriptor, pProfile));
    GeneratePtime(pDescriptor, pProfile);
    GenerateMaxPtime(pDescriptor, pProfile);
    GenerateCandidateAttribute(pDescriptor, pProfile);
    GenerateRtcpXr(pDescriptor, pProfile);
    GenerateAnbr(pDescriptor, pProfile);

    return IMS_TRUE;
}

PRIVATE
void AudioSdpGenerator::GeneratePayload(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpMap = AString::ConstNull();
        AString strPayloadNum = AString::ConstNull();
        AString strFmtp = AString::ConstNull();

        AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(i);
        if (pPayload == IMS_NULL)
        {
            continue;
        }

        GenerateRtpMap(strRtpMap, strPayloadNum, pPayload->GetRtpMap());

        if (GenerateFmtp(strFmtp, pPayload))
        {
            pDescriptor->SetMediaFormat(
                    SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpMap, strFmtp);
        }
    }
}

PRIVATE
IMS_BOOL AudioSdpGenerator::GenerateFmtp(OUT AString& strFmtp, IN AudioProfile::Payload* pPayload)
{
    if (pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // set "fmtp"
    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB") ||
            pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR"))
    {
        AudioProfile::AmrFmtp* pAmrFmtp = (AudioProfile::AmrFmtp*)pPayload->GetFmtp();
        if (pAmrFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        strFmtp = AudioNegoAmr::SetSdpFmtpFromAmrFmtp(pAmrFmtp);
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
    {
        AudioProfile::TelephoneEventFmtp* pTeFmtp =
                (AudioProfile::TelephoneEventFmtp*)pPayload->GetFmtp();
        if (pTeFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        strFmtp = pTeFmtp->GetEvents();
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp = (AudioProfile::EvsFmtp*)pPayload->GetFmtp();
        if (pEvsFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        strFmtp = AudioNegoEvs::SetSdpFmtpFromEvsFmtp(pEvsFmtp);
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcmu") ||
            pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcma"))
    {
        // Generatrtpmap, not fmtp
        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PRIVATE
void AudioSdpGenerator::GeneratePtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nPtime = pProfile->GetPtime();

    if (nPtime != AudioProfile::AmrFmtp::DEFAULT_PTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::PTIME, nPtime);
        IMS_TRACE_D("GeneratePtime() - ptime[%d]", nPtime, 0, 0);
    }
}

PRIVATE
void AudioSdpGenerator::GenerateMaxPtime(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nMaxPtime = pProfile->GetMaxPtime();

    if (nMaxPtime != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::MAXPTIME, nMaxPtime);
        IMS_TRACE_D("GenerateMaxPtime() - nMaxPtime[%d]", nMaxPtime, 0, 0);
    }
}

PRIVATE
void AudioSdpGenerator::GenerateCandidateAttribute(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < pProfile->GetCandidateAttr().GetSize(); nIndex++)
    {
        AString strCandidateAttr = pProfile->GetCandidateAttr().GetAt(nIndex);
        if (strCandidateAttr.GetLength() != 0)
        {
            strCandidateAttr.Sprintf("%d, %s", nIndex + 1, strCandidateAttr.GetStr());
            pDescriptor->AddAttribute(SdpAttribute::CANDIDATE, strCandidateAttr);
            IMS_TRACE_D("GenerateCandidateAttribute() - [%s]", strCandidateAttr.GetStr(), 0, 0);
        }
    }
}

void AudioSdpGenerator::GenerateRtcpXr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

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

        IMS_TRACE_D("GenerateRtcpXr() - Support RtcpXr", 0, 0, 0);
    }
}

PRIVATE
void AudioSdpGenerator::GenerateAnbr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    if (pProfile->IsAnbrSupported())
    {
        pDescriptor->AddAttribute(SdpAttribute::ANBR, AString::ConstNull());
        IMS_TRACE_D("GenerateAnbr() - Support Anbr", 0, 0, 0);
    }
}
