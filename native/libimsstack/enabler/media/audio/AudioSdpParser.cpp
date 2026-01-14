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

#include "audio/AudioSdpParser.h"

#include "SdpAttribute.h"
#include "ServiceTrace.h"
#include "audio/AudioProfileUtil.h"
#include "offeranswer/SdpAvCodec.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AudioSdpParser::AudioSdpParser() :
        MediaSdpParser(MEDIA_TYPE_AUDIO)
{
}

PUBLIC VIRTUAL AudioSdpParser::~AudioSdpParser() {}

PUBLIC
IMS_BOOL AudioSdpParser::Parse(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("Parse()", 0, 0, 0);

    MediaSdpParser::Parse(pSessionDescriptor, pDescriptor, pProfile);

    ParseCapaNego(pDescriptor, &(pProfile->GetCapaNego()));
    ParsePayloads(pDescriptor, pProfile);
    ParsePtime(pDescriptor, pProfile);
    ParseMaxPtime(pDescriptor, pProfile);
    ParseRtcpXr(pDescriptor, pProfile);
    ParseAnbr(pDescriptor, pProfile);

    return IMS_TRUE;
}

PROTECTED
void AudioSdpParser::ParsePayloads(
        IN const IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParsePayloads(): invalid arguments", 0, 0, 0);
        return;
    }

    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    IMS_TRACE_I("ParsePayloads(): payload size[%d]", lstMediaFormat.GetSize(), 0, 0);

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        const SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));
        ParsePayload(pSdpCodec, pProfile);
    }
}

PROTECTED
void AudioSdpParser::ParsePayload(IN const SdpAvCodec* pSdpCodec, OUT AudioProfile* pProfile)
{
    if (pSdpCodec == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParsePayload(): invalid arguments", 0, 0, 0);
        return;
    }

    AudioProfile::Payload* pPayload = new AudioProfile::Payload();

    if (pPayload == IMS_NULL)
    {
        return;
    }

    AString strCodecName = AString::ConstNull();
    IMS_SINT32 nPayloadTypeNumber = -1;
    ParseRtpMap(pSdpCodec, pPayload, nPayloadTypeNumber, strCodecName);
    IMS_BOOL bIsPcm = (nPayloadTypeNumber == 0 || nPayloadTypeNumber == 8) ? IMS_TRUE : IMS_FALSE;

    if (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR") ||
            strCodecName.EqualsIgnoreCase("EVS"))
    {
        ParseFmtp(pSdpCodec, pPayload, strCodecName);
    }
    else if (strCodecName.EqualsIgnoreCase("telephone-event"))
    {
        ParseTelephoneEventFmtp(pSdpCodec, pPayload);
    }
    else if (bIsPcm)
    {
        IMS_TRACE_I("ParsePayload(): do nothing PCM case, codec[%s]", strCodecName.GetStr(), 0, 0);
    }
    else
    {
        IMS_TRACE_E(
                0, "ParsePayload(): NOT SUPPORTED audio codec[%s]", strCodecName.GetStr(), 0, 0);
        delete pPayload;
        return;
    }

    if (pPayload != IMS_NULL)
    {
        pProfile->AddPayload(pPayload);
    }
}

PROTECTED void AudioSdpParser::ParseRtpMap(IN const SdpAvCodec* pSdpCodec,
        OUT AudioProfile::Payload* pPayload, OUT IMS_SINT32& nPayloadTypeNumber,
        OUT AString& strCodecName)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseRtpMap(): invalid arguments", 0, 0, 0);
        return;
    }

    nPayloadTypeNumber = pSdpCodec->GetPayloadType();
    strCodecName = pSdpCodec->GetName();
    IMS_UINT32 nSamplingRate = pSdpCodec->GetClockRate();
    AString strChannel = pSdpCodec->GetEncodingParameters();
    IMS_UINT32 nChannel = ((strChannel != IMS_NULL && strChannel.EqualsIgnoreCase("1")) &&
                                  (strCodecName.EqualsIgnoreCase("AMR-WB") ||
                                          strCodecName.EqualsIgnoreCase("AMR") ||
                                          strCodecName.EqualsIgnoreCase("EVS")))
            ? 1
            : 0;

    IMS_TRACE_D(
            "ParseRtpMap(): Payload[%d], Codec[%s]", nPayloadTypeNumber, strCodecName.GetStr(), 0);
    IMS_TRACE_D("ParseRtpMap(): Sampling rate[%d], Channel[%d]", nSamplingRate, nChannel, 0);

    pPayload->SetRtpMap(nPayloadTypeNumber, strCodecName, nSamplingRate, nChannel);
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseFmtp(IN const SdpAvCodec* pSdpCodec,
        OUT AudioProfile::Payload* pPayload, IN const AString& strCodecName)
{
    if (pPayload == IMS_NULL || pSdpCodec == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseFmtp(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strFmtp = pSdpCodec->GetFormatSpecificParameter();
    std::shared_ptr<AudioProfile::AudioFmtp> pFmtp = IMS_NULL;
    IMS_BOOL bIsEvs = IMS_FALSE;
    IMS_BOOL bFmtpParsed = IMS_FALSE;

    if (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR"))
    {
        pFmtp = std::make_shared<AudioProfile::AmrFmtp>();
    }
    else if (strCodecName.EqualsIgnoreCase("EVS"))
    {
        pFmtp = std::make_shared<AudioProfile::EvsFmtp>();
        bIsEvs = IMS_TRUE;
    }
    else
    {
        IMS_TRACE_E(0, "ParseFmtp(): not supported audio codec[%s]", strCodecName.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseFmtp(): invalid fmtp", 0, 0, 0);
        return IMS_FALSE;
    }

    ImsList<AString> objSplitColon = strFmtp.Split(';');

    for (IMS_UINT32 i = 0; i < objSplitColon.GetSize(); i++)
    {
        if (objSplitColon.GetAt(i).GetLength() == 0)
        {
            continue;
        }

        ImsList<AString> objSplitEqual = objSplitColon.GetAt(i).Split('=');

        if (objSplitEqual.GetSize() < 2)
        {
            const AString& strTmp = objSplitColon.GetAt(i);
            IMS_TRACE_D(
                    "ParseFmtp(): Invalid fmtp parameter[%s] at index[%d]", strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        bool bAudioFmtpParsed = ParseAudioFmtp(objSplitEqual, pFmtp);

        if (!bAudioFmtpParsed)
        {
            if (bIsEvs)
            {
                bFmtpParsed = ParseEvsFmtp(objSplitEqual,
                                      std::static_pointer_cast<AudioProfile::EvsFmtp>(pFmtp)) ||
                        bFmtpParsed;
            }
            else  // AMR
            {
                bFmtpParsed = ParseAmrFmtp(objSplitEqual,
                                      std::static_pointer_cast<AudioProfile::AmrFmtp>(pFmtp)) ||
                        bFmtpParsed;
            }
        }
        else
        {
            bFmtpParsed = bFmtpParsed || bAudioFmtpParsed;
        }
    }

    if (bIsEvs)
    {
        SetEvsFullBr(std::static_pointer_cast<AudioProfile::EvsFmtp>(pFmtp));
        SetEvsFullBw(std::static_pointer_cast<AudioProfile::EvsFmtp>(pFmtp));
        SetEvsBrVisible(std::static_pointer_cast<AudioProfile::EvsFmtp>(pFmtp));
        SetEvsBwVisible(std::static_pointer_cast<AudioProfile::EvsFmtp>(pFmtp));
    }

    if (!bIsEvs || bFmtpParsed)
    {
        pPayload->SetFmtp(pFmtp);
    }

    return bFmtpParsed;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseAudioFmtp(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<AudioProfile::AudioFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (ParseModeSet(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ParseModeChangeCapability(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ParseModeChangePeriod(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ParseModeChangeNeighbor(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ParseMaxRed(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseAmrFmtp(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::AmrFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return ParseOctetAlign(objSplitEqual, pFmtp);
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseEvsFmtp(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    return ParseDtx(objSplitEqual, pFmtp) || ParseHfOnly(objSplitEqual, pFmtp) ||
            ParseEvsSwitchMode(objSplitEqual, pFmtp) || ParseBr(objSplitEqual, pFmtp) ||
            ParseBw(objSplitEqual, pFmtp) || ParseCmr(objSplitEqual, pFmtp) ||
            ParseChAwMode(objSplitEqual, pFmtp) || ParseBrSend(objSplitEqual, pFmtp) ||
            ParseBrRecv(objSplitEqual, pFmtp) || ParseBwSend(objSplitEqual, pFmtp) ||
            ParseBwRecv(objSplitEqual, pFmtp);
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseModeSet(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<AudioProfile::AudioFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseModeSet(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-set"))
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');

        for (IMS_UINT32 i = 0; i < objSplitComma.GetSize(); i++)
        {
            IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(i).ToInt32();
            pFmtp->SetModeSetList((pFmtp->GetModeSetList() | (1 << nModeSet)));
        }
        pFmtp->SetVisibleModeSet(IMS_TRUE);

        IMS_TRACE_D("ParseModeSet(): modeSetList[%d], visible[%d]", pFmtp->GetModeSetList(),
                pFmtp->IsModeSetVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseModeChangeCapability(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<AudioProfile::AudioFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseModeChangeCapability(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-change-capability"))
    {
        pFmtp->SetModeChangeCapability((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleModeChangeCapability(IMS_TRUE);

        IMS_TRACE_D("ParseModeChangeCapability(): mode-change-capability[%d], visible[%d]",
                pFmtp->GetModeChangeCapability(), pFmtp->IsModeChangeCapabilityVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseModeChangePeriod(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<AudioProfile::AudioFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseModeChangePeriod(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-change-period"))
    {
        pFmtp->SetModeChangePeriod((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleModeChangePeriod(IMS_TRUE);

        IMS_TRACE_D("ParseModeChangePeriod(): mode-change-period[%d], visible[%d]",
                pFmtp->GetModeChangePeriod(), pFmtp->IsModeChangePeriodVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseModeChangeNeighbor(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<AudioProfile::AudioFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseModeChangeNeighbor(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-change-neighbor"))
    {
        pFmtp->SetModeChangeNeighbor((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleModeChangeNeighbor(IMS_TRUE);

        IMS_TRACE_D("ParseModeChangeNeighbor(): mode-change-neighbor[%d], visible[%d]",
                pFmtp->GetModeChangeNeighbor(), pFmtp->IsModeChangeNeighborVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseMaxRed(IN const ImsList<AString>& objSplitEqual,
        OUT std::shared_ptr<AudioProfile::AudioFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseMaxRed(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("max-red"))
    {
        pFmtp->SetMaxRed((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleMaxRed(IMS_TRUE);

        IMS_TRACE_D("ParseMaxRed(): max-red[%d], visible[%d]", pFmtp->GetMaxRed(),
                pFmtp->IsMaxRedVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseOctetAlign(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::AmrFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseOctetAlign(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("octet-align"))
    {
        pFmtp->SetOctetAlign((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleOctetAlign(IMS_TRUE);

        IMS_TRACE_D("ParseOctetAlign(): octet-align[%d], visible[%d]", pFmtp->GetOctetAlign(),
                pFmtp->IsOctetAlignVisible(), 0);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseDtx(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseDtx(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("dtx"))
    {
        pFmtp->SetDtx((IMS_BOOL)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetVisibleDtx(IMS_TRUE);

        IMS_TRACE_D("ParseDtx(): dtx[%d], visible[%d]", pFmtp->IsDtxEnabled(),
                pFmtp->IsDtxVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseHfOnly(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseHfOnly(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("hf-only"))
    {
        pFmtp->SetHfOnly((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowHfOnly(IMS_TRUE);

        IMS_TRACE_D("ParseHfOnly(): hf-only[%d], visible[%d]", pFmtp->GetHfOnly(),
                pFmtp->IsHfOnlyVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseEvsSwitchMode(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseEvsSwitchMode(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("evs-mode-switch"))
    {
        pFmtp->SetEvsModeSwitch((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowEvsModeSwitch(IMS_TRUE);

        IMS_TRACE_D("ParseEvsSwitchMode(): evs-mode-switch[%d], visible[%d]",
                pFmtp->GetEvsModeSwitch(), pFmtp->IsEvsModeSwitchVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseBr(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseBr(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("br"))
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
        ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
        if (objSplitHyphen.GetSize() == 2)
        {
            IMS_UINT32 nFirstBr = 0;
            IMS_UINT32 nLastBr = 0;
            for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BR_CNT; j++)
            {
                if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                        IMS_TRUE)
                {
                    nFirstBr = j;
                }
                if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                        IMS_TRUE)
                {
                    nLastBr = j;
                    break;
                }
            }

            for (IMS_UINT32 k = nFirstBr; k <= nLastBr; k++)
            {
                pFmtp->SetBrList((pFmtp->GetBrList() | (1 << k)));
            }
        }
        else  // comma case
        {
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BR_CNT; k++)
                {
                    if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[k]) ==
                            IMS_TRUE)
                    {
                        pFmtp->SetBrList((pFmtp->GetBrList() | (1 << k)));
                    }
                }
            }
        }

        IMS_TRACE_D("ParseBr(): brList[%d], visible[%d]", pFmtp->GetBrList(),
                pFmtp->IsBrListVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseBw(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseBw(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("bw"))
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
        ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
        if (objSplitHyphen.GetSize() == 2)
        {
            IMS_UINT32 nFirstBw = 0;
            IMS_UINT32 nLastBw = 0;

            for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BW_CNT; j++)
            {
                if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                        IMS_TRUE)
                {
                    nFirstBw = j;
                }

                if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                        IMS_TRUE)
                {
                    nLastBw = j;
                    break;
                }
            }

            for (IMS_UINT32 k = nFirstBw; k <= nLastBw; k++)
            {
                pFmtp->SetBwList((pFmtp->GetBwList() | (1 << k)));
            }
        }
        else  // comma case
        {
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BW_CNT; k++)
                {
                    if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[k]) ==
                            IMS_TRUE)
                    {
                        pFmtp->SetBwList((pFmtp->GetBwList() | (1 << k)));
                    }
                }
            }
        }

        IMS_TRACE_D("ParseBw(): bwList[%d], visible[%d]", pFmtp->GetBwList(),
                pFmtp->IsBwListVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseCmr(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseCmr(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("cmr"))
    {
        pFmtp->SetCmr((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowCmr(IMS_TRUE);

        IMS_TRACE_D("ParseCmr(): cmr[%d], visible[%d]", pFmtp->GetCmr(), pFmtp->IsCmrVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseChAwMode(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseChAwMode(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("ch-aw-recv"))
    {
        pFmtp->SetReceivedChAwRecv((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetChAwRecv(pFmtp->GetReceivedChAwRecv());
        pFmtp->SetShowChannelAwMode(IMS_TRUE);

        IMS_TRACE_D("ParseChAwMode(): ch-aw-recv[%d], visible[%d]", pFmtp->GetChAwRecv(),
                pFmtp->IsChannelAwModeVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseBrSend(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseBrSend(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("br-send"))
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
        ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
        if (objSplitHyphen.GetSize() == 2)
        {
            IMS_UINT32 nFirstBr = 0;
            IMS_UINT32 nLastBr = 0;
            for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BR_CNT; j++)
            {
                if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                        IMS_TRUE)
                {
                    nFirstBr = j;
                }

                if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                        IMS_TRUE)
                {
                    nLastBr = j;
                    break;
                }
            }

            for (IMS_UINT32 k = nFirstBr; k <= nLastBr; k++)
            {
                pFmtp->SetBrRecv((pFmtp->GetBrRecv() | (1 << k)));
            }
        }
        else  // comma case
        {
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BR_CNT; k++)
                {
                    if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[k]) ==
                            IMS_TRUE)
                    {
                        pFmtp->SetBrRecv((pFmtp->GetBrRecv() | (1 << k)));
                    }
                }
            }
        }

        IMS_TRACE_D("ParseBrSend(): br-send[%d]", pFmtp->GetBrRecv(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseBrRecv(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseBrRecv(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("br-recv"))
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
        ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
        if (objSplitHyphen.GetSize() == 2)
        {
            IMS_UINT32 nFirstBr = 0;
            IMS_UINT32 nLastBr = 0;

            for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BR_CNT; j++)
            {
                if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                        IMS_TRUE)
                {
                    nFirstBr = j;
                }
                if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[j]) ==
                        IMS_TRUE)
                {
                    nLastBr = j;
                    break;
                }
            }
            for (IMS_UINT32 k = nFirstBr; k <= nLastBr; k++)
            {
                pFmtp->SetBrSend((pFmtp->GetBrSend() | (1 << k)));
            }
        }
        else  // comma case
        {
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BR_CNT; k++)
                {
                    if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BR[k]) ==
                            IMS_TRUE)
                    {
                        pFmtp->SetBrSend((pFmtp->GetBrSend() | (1 << k)));
                    }
                }
            }
        }

        IMS_TRACE_D("ParseBrRecv(): br-recv[%d]", pFmtp->GetBrSend(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseBwSend(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseBwSend(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("bw-send"))
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
        ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
        if (objSplitHyphen.GetSize() == 2)
        {
            IMS_UINT32 nFirstBw = 0;
            IMS_UINT32 nLastBw = 0;

            for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BW_CNT; j++)
            {
                if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                        IMS_TRUE)
                {
                    nFirstBw = j;
                }

                if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                        IMS_TRUE)
                {
                    nLastBw = j;
                    break;
                }
            }

            for (IMS_UINT32 k = nFirstBw; k <= nLastBw; k++)
            {
                pFmtp->SetBwRecv((pFmtp->GetBwRecv() | (1 << k)));
            }
        }
        else  // comma case
        {
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BW_CNT; k++)
                {
                    if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[k]) ==
                            IMS_TRUE)
                    {
                        pFmtp->SetBwRecv((pFmtp->GetBwRecv() | (1 << k)));
                    }
                }
            }
        }

        IMS_TRACE_D("ParseBwSend(): bw-send[%d]", pFmtp->GetBwRecv(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseBwRecv(
        IN const ImsList<AString>& objSplitEqual, OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseBwRecv(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("bw-recv"))
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
        ImsList<AString> objSplitHyphen = objSplitEqual.GetAt(1).Split('-');
        if (objSplitHyphen.GetSize() == 2)
        {
            IMS_UINT32 nFirstBw = 0;
            IMS_UINT32 nLastBw = 0;

            for (IMS_UINT32 j = 0; j < AudioProfileUtil::EVS_BW_CNT; j++)
            {
                if (objSplitHyphen.GetAt(0).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                        IMS_TRUE)
                {
                    nFirstBw = j;
                }

                if (objSplitHyphen.GetAt(1).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[j]) ==
                        IMS_TRUE)
                {
                    nLastBw = j;
                    break;
                }
            }
            for (IMS_UINT32 k = nFirstBw; k <= nLastBw; k++)
            {
                pFmtp->SetBwSend((pFmtp->GetBwSend() | (1 << k)));
            }
        }
        else  // comma case
        {
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                for (IMS_UINT32 k = 0; k < AudioProfileUtil::EVS_BW_CNT; k++)
                {
                    if (objSplitComma.GetAt(j).EqualsIgnoreCase(AudioProfileUtil::EVS_BW[k]) ==
                            IMS_TRUE)
                    {
                        pFmtp->SetBwSend((pFmtp->GetBwSend() | (1 << k)));
                    }
                }
            }
        }

        IMS_TRACE_D("ParseBwRecv(): bw-recv[%d]", pFmtp->GetBwSend(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PROTECTED
void AudioSdpParser::SetEvsFullBr(OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetEvsFullBr(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pFmtp->GetBrList() == 0 && (pFmtp->GetBrRecv() == 0 && pFmtp->GetBrSend() == 0))
    {
        pFmtp->SetBrList(0x0FFF);  // 5.9-128 case
    }

    IMS_TRACE_D("SetEvsFullBr(): brList[%d], visible[%d]", pFmtp->GetBrList(),
            pFmtp->IsBrListVisible(), 0);
}

PROTECTED
void AudioSdpParser::SetEvsFullBw(OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetEvsFullBw(): invalid arguments", 0, 0, 0);
        return;
    }

    if (pFmtp->GetBwList() == 0 && (pFmtp->GetBwRecv() == 0 && pFmtp->GetBwSend() == 0))
    {
        pFmtp->SetBwList(0x0F);  // NB-FB case
    }

    IMS_TRACE_D("SetEvsFullBw(): bwList[%d], visible[%d]", pFmtp->GetBwList(),
            pFmtp->IsBwListVisible(), 0);
}

PROTECTED
void AudioSdpParser::SetEvsBrVisible(OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetEvsBrVisible(): invalid arguments", 0, 0, 0);
        return;
    }

    if ((pFmtp->GetBrRecv() != 0) && (pFmtp->GetBrSend() != 0))
    {
        pFmtp->SetShowBrList(IMS_FALSE);
    }

    IMS_TRACE_D("SetEvsBrVisible(): br-recv[%d], br-send[%d], br visible[%d]", pFmtp->GetBrRecv(),
            pFmtp->GetBrSend(), pFmtp->IsBrListVisible());
}

PROTECTED
void AudioSdpParser::SetEvsBwVisible(OUT std::shared_ptr<AudioProfile::EvsFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "SetEvsBwVisible(): invalid arguments", 0, 0, 0);
        return;
    }

    if ((pFmtp->GetBwRecv() != 0) && (pFmtp->GetBwSend() != 0))
    {
        pFmtp->SetShowBwList(IMS_FALSE);
    }

    IMS_TRACE_D("SetEvsBwVisible(): bw-recv[%d], bw-send[%d], bw visible[%d]", pFmtp->GetBwRecv(),
            pFmtp->GetBwSend(), pFmtp->IsBwListVisible());
}

PROTECTED
IMS_BOOL AudioSdpParser::ParseTelephoneEventFmtp(
        IN const SdpAvCodec* pSdpCodec, OUT AudioProfile::Payload* pPayload)
{
    if (pPayload == IMS_NULL || pSdpCodec == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseTelephoneEventFmtp(): invalid arguments", 0, 0, 0);
        return IMS_FALSE;
    }

    AString strFmtp = pSdpCodec->GetFormatSpecificParameter();
    std::shared_ptr<AudioProfile::TelephoneEventFmtp> pFmtp =
            std::make_shared<AudioProfile::TelephoneEventFmtp>();

    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseTelephoneEventFmtp(): invalid fmtp", 0, 0, 0);
        return IMS_FALSE;
    }

    IMS_TRACE_D("ParseTelephoneEventFmtp(): strFmtp[%s]", strFmtp.GetStr(), 0, 0);

    ParseEvents(strFmtp, pFmtp);

    pPayload->SetFmtp(pFmtp);

    return IMS_TRUE;
}

PROTECTED
void AudioSdpParser::ParseEvents(
        IN const AString& strFmtp, OUT std::shared_ptr<AudioProfile::TelephoneEventFmtp> pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseEvents(): invalid arguments", 0, 0, 0);
        return;
    }

    //[RFC4733] For backward compatibility, if no "events" parameter is received,
    // the sender SHOULD assume support for the DTMF events 0-15 but for no other events.
    pFmtp->SetEvents((strFmtp != IMS_NULL && strFmtp.GetLength() > 0) ? strFmtp : "0-15");

    IMS_TRACE_D("ParseEvents(): events[%s]", pFmtp->GetEvents().GetStr(), 0, 0);
}

PROTECTED
void AudioSdpParser::ParsePtime(IN const IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParsePtime(): invalid arguments", 0, 0, 0);
        return;
    }

    pProfile->SetPtime(pDescriptor->GetAttributeInt(SdpAttribute::PTIME));

    IMS_TRACE_D("ParsePtime(): ptime[%d]", pProfile->GetPtime(), 0, 0);
}

PROTECTED
void AudioSdpParser::ParseMaxPtime(
        IN const IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseMaxPtime(): invalid arguments", 0, 0, 0);
        return;
    }

    pProfile->SetMaxPtime(pDescriptor->GetAttributeInt(SdpAttribute::MAXPTIME));

    IMS_TRACE_D("ParseMaxPtime(): maxPtime[%d]", pProfile->GetMaxPtime(), 0, 0);
}

PROTECTED
void AudioSdpParser::ParseRtcpXr(IN const IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseRtcpXr(): invalid arguments", 0, 0, 0);
        return;
    }

    ImsList<AString> lstRtcpXrAttr = pDescriptor->GetAttributes(SdpAttribute::RTCP_XR);

    if (lstRtcpXrAttr.GetSize() == 0)
    {
        IMS_TRACE_D("ParseRtcpXr(): No Rtcp-Xr Attributes", 0, 0, 0);
        return;
    }

    pProfile->SetSupportRtcpXr(IMS_TRUE);

    for (IMS_UINT32 i = 0; i < lstRtcpXrAttr.GetSize(); i++)
    {
        AString xrAttr = lstRtcpXrAttr.GetAt(i);

        if (xrAttr.Contains("stat-summary"))
        {
            pProfile->GetRtcpXrAttr().SetSupportStatisticMetrics(IMS_TRUE);
        }
        if (xrAttr.Contains("voip-metrics"))
        {
            pProfile->GetRtcpXrAttr().SetSupportVoipMetrics(IMS_TRUE);
        }
        if (xrAttr.Contains("pkt-loss-rle"))
        {
            pProfile->GetRtcpXrAttr().SetSupportPacketLossRle(IMS_TRUE);
        }
        if (xrAttr.Contains("pkt-dup-rle"))
        {
            pProfile->GetRtcpXrAttr().SetSupportPacketDuplicatedRle(IMS_TRUE);
        }
    }

    IMS_TRACE_D("ParseRtcpXr(): support[%d], stat-summary[%d], voip-metrics[%d]",
            pProfile->IsRtcpXrSupported(), pProfile->GetRtcpXrAttr().IsStatisticMetricsSupported(),
            pProfile->GetRtcpXrAttr().IsVoipMetricsSupported());
    IMS_TRACE_D("ParseRtcpXr(): pkt-loss-rle[%d], pkt-dup-rle[%d]",
            pProfile->GetRtcpXrAttr().IsPacketLossRleSupported(),
            pProfile->GetRtcpXrAttr().IsPacketDuplicatedRleSupported(), 0);
}

PROTECTED
void AudioSdpParser::ParseAnbr(IN const IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        IMS_TRACE_E(0, "ParseAnbr(): invalid arguments", 0, 0, 0);
        return;
    }

    pProfile->SetAnbr(
            (pDescriptor->GetAttribute(SdpAttribute::ANBR).IsEmpty()) ? IMS_TRUE : IMS_FALSE);

    IMS_TRACE_D("ParseAnbr(): anbr[%d]", pProfile->IsAnbrSupported(), 0, 0);
}
