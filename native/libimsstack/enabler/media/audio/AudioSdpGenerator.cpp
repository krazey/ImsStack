/*
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

#include "audio/AudioProfileUtil.h"
#include "audio/AudioSdpGenerator.h"

#define MODESET_MAX_AMR   7
#define MODESET_MAX_AMRWB 8

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AudioSdpGenerator::AudioSdpGenerator() :
        MediaSdpGenerator(MEDIA_TYPE_AUDIO)
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
        IMS_TRACE_E(0, "Generate(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_I("Generate(): PayloadSize[%d]", pBaseProfile->GetPayloadList().GetSize(), 0, 0);

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

PROTECTED
void AudioSdpGenerator::GeneratePayload(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GeneratePayload(): invalid arguments", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 i = 0; i < pProfile->GetPayloadList().GetSize(); i++)
    {
        AString strRtpMap = AString::ConstNull();
        AString strPayloadNum = AString::ConstNull();
        AString strFmtp = AString::ConstNull();

        AudioProfile::Payload* pPayload = pProfile->GetPayloadAt(i);

        if (pPayload != IMS_NULL &&
                GenerateRtpMap(strRtpMap, strPayloadNum, pPayload->GetRtpMap()) &&
                GenerateFmtp(strFmtp, pPayload))
        {
            pDescriptor->SetMediaFormat(
                    SdpMediaFormat::TYPE_RTP, strPayloadNum, strRtpMap, strFmtp);
        }
    }
}

PROTECTED
IMS_BOOL AudioSdpGenerator::GenerateFmtp(OUT AString& strFmtp, IN AudioProfile::Payload* pPayload)
{
    if (pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateFmtp(): invalid payload", 0, 0, 0);
        return IMS_FALSE;
    }

    // set "fmtp"
    if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB") ||
            pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR"))
    {
        auto pAmrFmtp = std::static_pointer_cast<AudioProfile::AmrFmtp>(pPayload->GetFmtp());
        if (pAmrFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        strFmtp = GenerateAmrFmtp(pAmrFmtp);
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("telephone-event"))
    {
        auto pTeFmtp =
                std::static_pointer_cast<AudioProfile::TelephoneEventFmtp>(pPayload->GetFmtp());
        if (pTeFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        strFmtp = pTeFmtp->GetEvents();
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS"))
    {
        auto pEvsFmtp = std::static_pointer_cast<AudioProfile::EvsFmtp>(pPayload->GetFmtp());
        if (pEvsFmtp == IMS_NULL)
        {
            return IMS_FALSE;
        }

        strFmtp = GenerateEvsFmtp(pEvsFmtp);
    }
    else if (pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcmu") ||
            pPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("pcma"))
    {
        // Generate rtpmap, not fmtp
        return IMS_TRUE;
    }
    else
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PROTECTED
void AudioSdpGenerator::GeneratePtime(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GeneratePtime(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPtime = pProfile->GetPtime();

    if (nPtime != AudioProfile::DEFAULT_PTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::PTIME, nPtime);
        IMS_TRACE_D("GeneratePtime(): ptime[%d]", nPtime, 0, 0);
    }
}

PROTECTED
void AudioSdpGenerator::GenerateMaxPtime(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateMaxPtime(): invalid arguments", 0, 0, 0);
        return;
    }

    IMS_SINT32 nMaxPtime = pProfile->GetMaxPtime();

    if (nMaxPtime != AudioProfile::DEFAULT_MAXPTIME)
    {
        pDescriptor->AddAttributeInt(SdpAttribute::MAXPTIME, nMaxPtime);
        IMS_TRACE_D("GenerateMaxPtime(): MaxPtime[%d]", nMaxPtime, 0, 0);
    }
}

PROTECTED
void AudioSdpGenerator::GenerateCandidateAttribute(
        OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateCandidateAttribute(): invalid arguments", 0, 0, 0);
        return;
    }

    for (IMS_UINT32 nIndex = 0; nIndex < pProfile->GetCandidateAttr().GetSize(); nIndex++)
    {
        AString strCandidateAttr = pProfile->GetCandidateAttr().GetAt(nIndex);
        if (strCandidateAttr.GetLength() != 0)
        {
            strCandidateAttr.Sprintf("%d, %s", nIndex + 1, strCandidateAttr.GetStr());
            pDescriptor->AddAttribute(SdpAttribute::CANDIDATE, strCandidateAttr);
            IMS_TRACE_D("GenerateCandidateAttribute(): [%s]", strCandidateAttr.GetStr(), 0, 0);
        }
    }
}

void AudioSdpGenerator::GenerateRtcpXr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateRtcpXr(): invalid arguments", 0, 0, 0);
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

        IMS_TRACE_D("GenerateRtcpXr(): Support RtcpXr", 0, 0, 0);
    }
}

PROTECTED
void AudioSdpGenerator::GenerateAnbr(OUT IMediaDescriptor* pDescriptor, IN AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateAnbr(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pProfile->IsAnbrSupported())
    {
        pDescriptor->AddAttribute(SdpAttribute::ANBR, AString::ConstNull());
        IMS_TRACE_D("GenerateAnbr(): Support Anbr", 0, 0, 0);
    }
}

PROTECTED AString AudioSdpGenerator::GenerateAmrFmtp(
        IN std::shared_ptr<AudioProfile::AmrFmtp> pAmrFmtp)
{
    AString strFmtp = AString::ConstNull();

    if (pAmrFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateAmrFmtp(): invalid fmtp", 0, 0, 0);
        return strFmtp;
    }

    AddModeSetListToFmtp(pAmrFmtp, strFmtp);
    AddOctetAlignToFmtp(pAmrFmtp, strFmtp);

    if (strFmtp.IsNull())
    {
        ForceToAddModeSetList(pAmrFmtp, strFmtp);

        if (strFmtp.IsNull())
        {
            ForceToAddOctetAlign(pAmrFmtp, strFmtp);
        }
    }

    AddModeChangeCapabilityToFmtp(pAmrFmtp, strFmtp);
    AddModeChangePeriodToFmtp(pAmrFmtp, strFmtp);
    AddModeChangeNeighborToFmtp(pAmrFmtp, strFmtp);
    AddMaxRedToFmtp(pAmrFmtp, strFmtp);

    return strFmtp;
}

PROTECTED AString AudioSdpGenerator::GenerateEvsFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pEvsFmtp)
{
    AString strFmtp = AString::ConstNull();

    if (pEvsFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "GenerateEvsFmtp(): invalid fmtp", 0, 0, 0);
        return strFmtp;
    }

    AddDtxToFmtp(pEvsFmtp, strFmtp);
    AddHfOnlyToFmtp(pEvsFmtp, strFmtp);
    AddEvsModeSwitchToFmtp(pEvsFmtp, strFmtp);
    AddMaxRedToFmtp(pEvsFmtp, strFmtp);
    AddBrToFmtp(pEvsFmtp, strFmtp);
    AddBwToFmtp(pEvsFmtp, strFmtp);
    AddCmrToFmtp(pEvsFmtp, strFmtp);
    AddChannelAwModeToFmtp(pEvsFmtp, strFmtp);
    AddModeSetListToFmtp(pEvsFmtp, strFmtp);

    if (pEvsFmtp->GetEvsModeSwitch() == 1)
    {
        AddModeChangeCapabilityToFmtp(pEvsFmtp, strFmtp);
        AddModeChangePeriodToFmtp(pEvsFmtp, strFmtp);
        AddModeChangeNeighborToFmtp(pEvsFmtp, strFmtp);
    }

    AddBrSendToFmtp(pEvsFmtp, strFmtp);
    AddBrRecvToFmtp(pEvsFmtp, strFmtp);
    AddBwSendToFmtp(pEvsFmtp, strFmtp);
    AddBwRecvToFmtp(pEvsFmtp, strFmtp);

    return strFmtp;
}

PROTECTED void AudioSdpGenerator::AddModeSetListToFmtp(
        IN std::shared_ptr<AudioProfile::AudioFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddModeSetListToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddModeSetListToFmtp(): modeSetList[%d], visible modeSet[%d]",
            pFmtp->GetModeSetList(), pFmtp->IsModeSetVisible(), 0);

    if (pFmtp->GetModeSetList() != 0 && pFmtp->IsModeSetVisible() == IMS_TRUE)
    {
        AString strMode, strTemp;
        IMS_UINT32 nModeSet;

        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        for (nModeSet = 0; nModeSet <= MODESET_MAX_AMRWB; nModeSet++)
        {
            if ((pFmtp->GetModeSetList()) & (1 << nModeSet))
            {
                AppendSeparatorIfNotEmpty(strTemp, COMMA);
                strMode.Sprintf("%d", nModeSet);
                strTemp.Append(strMode);
            }
        }

        strFmtp.Append("mode-set=");
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddModeChangeCapabilityToFmtp(
        IN std::shared_ptr<AudioProfile::AudioFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddModeChangeCapabilityToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddModeChangeCapabilityToFmtp(): mode-change-capability[%d], visible[%d]",
            pFmtp->GetModeChangeCapability(), pFmtp->IsModeChangeCapabilityVisible(), 0);

    if (pFmtp->IsModeChangeCapabilityVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-capability=%d", pFmtp->GetModeChangeCapability());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddModeChangePeriodToFmtp(
        IN std::shared_ptr<AudioProfile::AudioFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeChangePeriodToFmtp(): mode-change-period[%d], visible[%d]",
            pFmtp->GetModeChangePeriod(), pFmtp->IsModeChangePeriodVisible(), 0);

    if (pFmtp->IsModeChangePeriodVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-period=%d", pFmtp->GetModeChangePeriod());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddModeChangeNeighborToFmtp(
        IN std::shared_ptr<AudioProfile::AudioFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddModeChangeNeighborToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddModeChangeNeighborToFmtp(): mode-change-neighbor[%d], visible[%d]",
            pFmtp->GetModeChangeNeighbor(), pFmtp->IsModeChangeNeighborVisible(), 0);

    if (pFmtp->IsModeChangeNeighborVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-neighbor=%d", pFmtp->GetModeChangeNeighbor());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddMaxRedToFmtp(
        IN std::shared_ptr<AudioProfile::AudioFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddMaxRedToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddMaxRedToFmtp(): max-red[%d], visible[%d]", pFmtp->GetMaxRed(),
            pFmtp->IsMaxRedVisible(), 0);

    if (pFmtp->IsMaxRedVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("max-red=%d", pFmtp->GetMaxRed());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddOctetAlignToFmtp(
        IN std::shared_ptr<AudioProfile::AmrFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddOctetAlignToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddOctetAlignToFmtp(): octet-align[%d], visible[%d]", pFmtp->GetOctetAlign(),
            pFmtp->IsOctetAlignVisible(), 0);

    if (pFmtp->IsOctetAlignVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("octet-align=%d", pFmtp->GetOctetAlign());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddDtxToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddDtxToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddDtxToFmtp(): dtx[%d], visible[%d]", pFmtp->IsDtxEnabled(),
            pFmtp->IsDtxVisible(), 0);

    if (pFmtp->IsDtxVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("dtx=%d", pFmtp->IsDtxEnabled());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddHfOnlyToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddHfOnlyToFmtp(): hf-only[%d], visible[%d]", pFmtp->GetHfOnly(),
            pFmtp->IsHfOnlyVisible(), 0);

    if (pFmtp->IsHfOnlyVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("hf-only=%d", pFmtp->GetHfOnly());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddEvsModeSwitchToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddEvsModeSwitchToFmtp(): evs-mode-switch[%d], visible[%d]",
            pFmtp->GetEvsModeSwitch(), pFmtp->IsEvsModeSwitchVisible(), 0);

    if (pFmtp->IsEvsModeSwitchVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("evs-mode-switch=%d", pFmtp->GetEvsModeSwitch());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddBwToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddBwToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddBwToFmtp(): bw-list[%d], visible[%d]", pFmtp->GetBwList(),
            pFmtp->IsBwListVisible(), 0);

    if (pFmtp->GetBwList() != 0 && pFmtp->IsBwListVisible())
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        /** TODO: Need to check that '11' is proper later */
        for (nBandwidthList = 0; nBandwidthList <= 11; nBandwidthList++)
        {
            if ((pFmtp->GetBwList()) & (1 << nBandwidthList))
            {
                if (strTemp.GetLength() > 0)
                {
                    strTemp.Append(",");
                }
                AppendSeparatorIfNotEmpty(strTemp, COMMA);

                strMode.Sprintf("%s", AudioProfileUtil::EVS_BW[nBandwidthList].GetStr());
                strTemp.Append(strMode);
                nBandwidthListTotalCnt++;

                if (nBandwidthListTotalCnt == 1)
                {
                    strFirstBandwidth = AudioProfileUtil::EVS_BW[nBandwidthList];
                }

                strLastBandwidth = AudioProfileUtil::EVS_BW[nBandwidthList];
            }
        }

        if (nBandwidthListTotalCnt > 1)
        {
            strTemp = "";
            strTemp.Append(strFirstBandwidth);
            strTemp.Append("-");
            strTemp.Append(strLastBandwidth);
        }

        strFmtp.Append("bw=");
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddBrToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddBrToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddBrToFmtp(): br-list[%d], visible[%d]", pFmtp->GetBrList(),
            pFmtp->IsBrListVisible(), 0);

    if (pFmtp->IsBrListVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (pFmtp->GetBrList()) & (1 << nBitrateList);
            if (nMatch)
            {
                if (strTemp.GetLength() > 0)
                    strTemp.Append(",");

                strMode.Sprintf("%s", AudioProfileUtil::EVS_BR[nBitrateList].GetStr());
                strTemp.Append(strMode);
                nBitrateListTotalCnt++;

                if (nBitrateListTotalCnt == 1)
                    strFirstBitrate = AudioProfileUtil::EVS_BR[nBitrateList];

                strLastBitrate = AudioProfileUtil::EVS_BR[nBitrateList];
            }
        }

        if (nBitrateListTotalCnt > 1)
        {
            strTemp = "";
            strTemp.Append(strFirstBitrate);
            strTemp.Append("-");
            strTemp.Append(strLastBitrate);
        }

        strFmtp.Append("br=");
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddCmrToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddCmrToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddCmrToFmtp(): cmr[%d], visible[%d]", pFmtp->GetCmr(), pFmtp->IsCmrVisible(), 0);

    if (pFmtp->IsCmrVisible() == IMS_TRUE && pFmtp->GetEvsModeSwitch() != 1)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("cmr=%d", pFmtp->GetCmr());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddChannelAwModeToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddChannelAwModeToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddChannelAwModeToFmtp(): ch-aw-recv[%d], visible[%d]", pFmtp->GetChAwRecv(),
            pFmtp->IsChannelAwModeVisible(), 0);

    if (pFmtp->IsChannelAwModeVisible() == IMS_TRUE && pFmtp->GetEvsModeSwitch() != 1)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("ch-aw-recv=%d", pFmtp->GetChAwRecv());
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddBwSendToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddBwSendToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddBwSendToFmtp(): bw-send[%d]", pFmtp->GetBwSend(), 0, 0);

    if (pFmtp->GetBwSend() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        for (nBandwidthList = 0; nBandwidthList < AudioProfileUtil::EVS_BW_CNT; nBandwidthList++)
        {
            IMS_UINT32 nMatch = (pFmtp->GetBwSend()) & (1 << nBandwidthList);
            if (nMatch)
            {
                AppendSeparatorIfNotEmpty(strTemp, COMMA);
                strMode.Sprintf("%s", AudioProfileUtil::EVS_BW[nBandwidthList].GetStr());
                strTemp.Append(strMode);
                nBandwidthListTotalCnt++;

                if (nBandwidthListTotalCnt == 1)
                {
                    strFirstBandwidth = AudioProfileUtil::EVS_BW[nBandwidthList];
                }

                strLastBandwidth = AudioProfileUtil::EVS_BW[nBandwidthList];
            }
        }

        if (nBandwidthListTotalCnt > 1)
        {
            strTemp = "";
            strTemp.Append(strFirstBandwidth);
            strTemp.Append("-");
            strTemp.Append(strLastBandwidth);
        }

        strFmtp.Append("bw-send=");
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddBwRecvToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddBwRecvToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddBwRecvToFmtp(): bw-recv[%d]", pFmtp->GetBwRecv(), 0, 0);

    if (pFmtp->GetBwRecv() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        for (nBandwidthList = 0; nBandwidthList < AudioProfileUtil::EVS_BW_CNT; nBandwidthList++)
        {
            IMS_UINT32 nMatch = (pFmtp->GetBwRecv()) & (1 << nBandwidthList);
            if (nMatch)
            {
                AppendSeparatorIfNotEmpty(strTemp, COMMA);
                strMode.Sprintf("%s", AudioProfileUtil::EVS_BW[nBandwidthList].GetStr());
                strTemp.Append(strMode);
                nBandwidthListTotalCnt++;

                if (nBandwidthListTotalCnt == 1)
                {
                    strFirstBandwidth = AudioProfileUtil::EVS_BW[nBandwidthList];
                }

                strLastBandwidth = AudioProfileUtil::EVS_BW[nBandwidthList];
            }
        }

        if (nBandwidthListTotalCnt > 1)
        {
            strTemp = "";
            strTemp.Append(strFirstBandwidth);
            strTemp.Append("-");
            strTemp.Append(strLastBandwidth);
        }

        strFmtp.Append("bw-recv=");
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddBrSendToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddBrSendToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddBrSendToFmtp(): br-send[%d]", pFmtp->GetBwSend(), 0, 0);

    if (pFmtp->GetBrSend() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (pFmtp->GetBrSend()) & (1 << nBitrateList);
            if (nMatch)
            {
                AppendSeparatorIfNotEmpty(strTemp, COMMA);

                strMode.Sprintf("%s", AudioProfileUtil::EVS_BR[nBitrateList].GetStr());
                strTemp.Append(strMode);
                nBitrateListTotalCnt++;

                if (nBitrateListTotalCnt == 1)
                {
                    strFirstBitrate = AudioProfileUtil::EVS_BR[nBitrateList];
                }

                strLastBitrate = AudioProfileUtil::EVS_BR[nBitrateList];
            }
        }

        if (nBitrateListTotalCnt > 1)
        {
            strTemp = "";
            strTemp.Append(strFirstBitrate);
            strTemp.Append("-");
            strTemp.Append(strLastBitrate);
        }

        strFmtp.Append("br-send=");
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::AddBrRecvToFmtp(
        IN std::shared_ptr<AudioProfile::EvsFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "AddBrRecvToFmtp(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("AddBrRecvToFmtp(): br-recv[%d]", pFmtp->GetBwRecv(), 0, 0);

    if (pFmtp->GetBrRecv() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (pFmtp->GetBrRecv()) & (1 << nBitrateList);
            if (nMatch)
            {
                AppendSeparatorIfNotEmpty(strTemp, COMMA);

                strMode.Sprintf("%s", AudioProfileUtil::EVS_BR[nBitrateList].GetStr());
                strTemp.Append(strMode);
                nBitrateListTotalCnt++;

                if (nBitrateListTotalCnt == 1)
                {
                    strFirstBitrate = AudioProfileUtil::EVS_BR[nBitrateList];
                }

                strLastBitrate = AudioProfileUtil::EVS_BR[nBitrateList];
            }
        }

        if (nBitrateListTotalCnt > 1)
        {
            strTemp = "";
            strTemp.Append(strFirstBitrate);
            strTemp.Append("-");
            strTemp.Append(strLastBitrate);
        }

        strFmtp.Append("br-recv=");
        strFmtp.Append(strTemp);
    }
}

PROTECTED void AudioSdpGenerator::ForceToAddModeSetList(
        IN std::shared_ptr<AudioProfile::AudioFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ForceToAddModeSetList(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("ForceToAddModeSetList()", 0, 0, 0);

    IMS_BOOL bVisibleModeSet = pFmtp->IsModeSetVisible();
    pFmtp->SetVisibleModeSet(IMS_TRUE);
    AddModeSetListToFmtp(pFmtp, strFmtp);
    pFmtp->SetVisibleModeSet(bVisibleModeSet);
    IMS_TRACE_I("ForceToAddModeSetList(): mode-set visible[%d]", bVisibleModeSet, 0, 0);
}

PROTECTED void AudioSdpGenerator::ForceToAddOctetAlign(
        IN std::shared_ptr<AudioProfile::AmrFmtp> pFmtp, OUT AString& strFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ForceToAddOctetAlign(): invalid fmtp", 0, 0, 0);
        return;
    }

    IMS_BOOL bVisibleOctetAlign = pFmtp->IsOctetAlignVisible();
    pFmtp->SetVisibleOctetAlign(IMS_TRUE);
    AddOctetAlignToFmtp(pFmtp, strFmtp);
    pFmtp->SetVisibleOctetAlign(bVisibleOctetAlign);

    IMS_TRACE_I("ForceToAddOctetAlign(): octet-aligned visible[%d]", bVisibleOctetAlign, 0, 0);
}
