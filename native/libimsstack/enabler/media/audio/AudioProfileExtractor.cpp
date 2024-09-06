// Copyright 2024 Google LLC

#include "ServiceTrace.h"

#include "audio/AudioProfileExtractor.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AudioProfileExtractor::AudioProfileExtractor() :
        ProfileExtractor(MEDIA_TYPE_AUDIO)
{
    IMS_TRACE_I("+AudioProfileExtractor()", 0, 0, 0);
}

PUBLIC VIRTUAL AudioProfileExtractor::~AudioProfileExtractor()
{
    IMS_TRACE_I("~AudioProfileExtractor()", 0, 0, 0);
}

PUBLIC
IMS_BOOL AudioProfileExtractor::Extract(IN ISessionDescriptor* pSessionDescriptor,
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pSessionDescriptor == IMS_NULL || pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_TRACE_I("Extract()", 0, 0, 0);

    ProfileExtractor::Extract(pSessionDescriptor, pDescriptor, pProfile);

    ExtractCapaNego(pDescriptor, &(pProfile->GetCapaNego()));
    ExtractPayloads(pDescriptor, pProfile);
    ExtractPtime(pDescriptor, pProfile);
    ExtractMaxPtime(pDescriptor, pProfile);
    ExtractRtcpXr(pDescriptor, pProfile);
    ExtractAnbr(pDescriptor, pProfile);

    return IMS_TRUE;
}

PRIVATE
void AudioProfileExtractor::ExtractPayloads(
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));
        AudioProfile::Payload* pPayload = new AudioProfile::Payload();

        if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
        {
            return;
        }

        IMS_TRACE_I("ExtractPayload() - At[%d]", i, 0, 0);

        AString strCodecName = AString::ConstNull();
        IMS_SINT32 nPayloadTypeNumber = -1;
        ExtractRtpMap(pSdpCodec, pPayload, nPayloadTypeNumber, strCodecName);
        IMS_BOOL bIsPcm =
                (nPayloadTypeNumber == 0 || nPayloadTypeNumber == 8) ? IMS_TRUE : IMS_FALSE;

        if (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR") ||
                strCodecName.EqualsIgnoreCase("EVS"))
        {
            ExtractFmtp(pSdpCodec->GetFormatSpecificParameter(), pPayload, strCodecName);
        }
        else if (strCodecName.EqualsIgnoreCase("telephone-event"))
        {
            ExtractTelephoneEventFmtp(pSdpCodec->GetFormatSpecificParameter(), pPayload);
        }
        else if (bIsPcm)
        {
            IMS_TRACE_D("ExtractPayloads() - do nothing PCM case, codec[%s]", strCodecName.GetStr(),
                    0, 0);
        }
        else
        {
            IMS_TRACE_E(0, "ExtractPayloads() - NOT SUPPORTED audio codec[%s]",
                    strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        if (pPayload != IMS_NULL)
        {
            pProfile->GetPayloadList().Append(pPayload);
        }
    }
}

PRIVATE void AudioProfileExtractor::ExtractRtpMap(IN const SdpAvCodec* pSdpCodec,
        OUT AudioProfile::Payload* pPayload, OUT IMS_SINT32& nPayloadTypeNumber,
        OUT AString& strCodecName)
{
    if (pSdpCodec == IMS_NULL || pPayload == IMS_NULL)
    {
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

    IMS_TRACE_D("ExtractRtpMap() - Payload[%d], Codec[%s]", nPayloadTypeNumber,
            strCodecName.GetStr(), 0);
    IMS_TRACE_D("ExtractRtpMap() - Sampling rate[%d], Channel[%d]", nSamplingRate, nChannel, 0);

    pPayload->SetRtpMap(nPayloadTypeNumber, strCodecName, nSamplingRate, nChannel);
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtp(IN const AString& strFmtp,
        OUT AudioProfile::Payload* pPayload, IN const AString& strCodecName)
{
    if (pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioProfile::AudioFmtp* pFmtp = IMS_NULL;
    IMS_BOOL bIsEvs = IMS_FALSE;

    if (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR"))
    {
        pFmtp = new AudioProfile::AmrFmtp();
    }
    else if (strCodecName.EqualsIgnoreCase("EVS"))
    {
        pFmtp = new AudioProfile::EvsFmtp();
        bIsEvs = IMS_TRUE;
    }
    else
    {
        IMS_TRACE_E(
                0, "ExtractFmtp() - NOT SUPPORTED audio codec[%s]", strCodecName.GetStr(), 0, 0);
        return IMS_FALSE;
    }

    if (pFmtp == IMS_NULL)
    {
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
            IMS_TRACE_D("ExtractFmtp() - Invalid fmtp parameter(%s) at index[%d]", strTmp.GetStr(),
                    i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (ExtractAudioBaseFmtp(objSplitEqual, pFmtp) == IMS_FALSE)
        {
            if (bIsEvs)
            {
                ExtractEvsFmtp(objSplitEqual, static_cast<AudioProfile::EvsFmtp*>(pFmtp));
            }
            else
            {
                ExtractAmrFmtp(objSplitEqual, static_cast<AudioProfile::AmrFmtp*>(pFmtp));
            }
        }
    }

    if (bIsEvs)
    {
        SetEvsBrVisible(static_cast<AudioProfile::EvsFmtp*>(pFmtp));
        SetEvsBwVisible(static_cast<AudioProfile::EvsFmtp*>(pFmtp));
    }

    pPayload->SetFmtp(pFmtp);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractAudioBaseFmtp(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (ExtractFmtpModeSet(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ExtractFmtpModeChangeCapability(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ExtractFmtpModeChangePeriod(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ExtractFmtpModeChangeNeighbor(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ExtractFmtpMaxRed(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ExtractFmtpPtime(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }
    if (ExtractFmtpMaxPtime(objSplitEqual, pFmtp))
    {
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void AudioProfileExtractor::ExtractAmrFmtp(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AmrFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    ExtractFmtpOctetAlign(objSplitEqual, pFmtp);
}

PRIVATE
void AudioProfileExtractor::ExtractEvsFmtp(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    if (ExtractFmtpDtx(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpHfOnly(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpEvsSwitchMode(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpBr(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpBw(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpCmr(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpChAwMode(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpBrSend(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpBrRecv(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpBwSend(objSplitEqual, pFmtp))
    {
        return;
    }
    if (ExtractFmtpBwRecv(objSplitEqual, pFmtp))
    {
        return;
    }
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpModeSet(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-set") == IMS_TRUE)
    {
        ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');

        for (IMS_UINT32 i = 0; i < objSplitComma.GetSize(); i++)
        {
            IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(i).ToInt32();
            pFmtp->SetModeSetList((pFmtp->GetModeSetList() | (1 << nModeSet)));
        }
        pFmtp->SetShowModeSet(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpModeSet() - modset list[%d], visible[%d]", pFmtp->GetModeSetList(),
                pFmtp->IsModeSetVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpModeChangeCapability(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-change-capability") == IMS_TRUE)
    {
        pFmtp->SetModeChangeCapability((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowModeChangeCapability(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpModeChangeCapability() - mode-change-capability[%d], visible[%d]",
                pFmtp->GetModeChangeCapability(), pFmtp->IsModeChangeCapabilityVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpModeChangePeriod(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-change-period") == IMS_TRUE)
    {
        pFmtp->SetModeChangePeriod((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowModeChangePeriod(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpModeChangePeriod() - mode-change-period[%d], visible[%d]",
                pFmtp->GetModeChangePeriod(), pFmtp->IsModeChangePeriodVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpModeChangeNeighbor(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("mode-change-neighbor") == IMS_TRUE)
    {
        pFmtp->SetModeChangeNeighbor((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowModeChangeNeighbor(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpModeChangeNeighbor() - mode-change-neighbor[%d], visible[%d]",
                pFmtp->GetModeChangeNeighbor(), pFmtp->IsModeChangeNeighborVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpMaxRed(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("max-red") == IMS_TRUE)
    {
        pFmtp->SetMaxRed((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowMaxRed(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpMaxRed() - max-red[%d], visible[%d]", pFmtp->GetMaxRed(),
                pFmtp->IsMaxRedVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpPtime(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("ptime") == IMS_TRUE)
    {
        pFmtp->SetPtime((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowPtime(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpPtime() - ptime[%d], visible[%d]", pFmtp->GetPtime(),
                pFmtp->IsPtimeVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpMaxPtime(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AudioFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("maxptime") == IMS_TRUE)
    {
        pFmtp->SetMaxPtime((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowMaxPtime(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpMaxPtime() - maxptime[%d], visible[%d]", pFmtp->GetMaxPtime(),
                pFmtp->IsMaxPtimeVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void AudioProfileExtractor::ExtractFmtpOctetAlign(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::AmrFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    if (objSplitEqual.GetAt(0).Equals("octet-align") == IMS_TRUE)
    {
        pFmtp->SetOctetAlign((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowOctetAlign(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpOctetAlign() - octet-align[%d], visible[%d]",
                pFmtp->GetOctetAlign(), pFmtp->IsOctetAlignVisible(), 0);
    }
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpDtx(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("ExtractFmtpDtx") == IMS_TRUE)
    {
        pFmtp->SetDtx((IMS_BOOL)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowDtx(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpDtx() - ExtractFmtpDtx[%d], visible[%d]", pFmtp->IsDtxEnabled(),
                pFmtp->IsDtxVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpHfOnly(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("hf-only") == IMS_TRUE)
    {
        pFmtp->SetHfOnly((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowHfOnly(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpHfOnly() - hf-only[%d], visible[%d]", pFmtp->GetHfOnly(),
                pFmtp->IsHfOnlyVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpEvsSwitchMode(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("evs-mode-switch") == IMS_TRUE)
    {
        pFmtp->SetEvsModeSwitch((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowEvsModeSwitch(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpEvsSwitchMode() - evs-mode-switch[%d], visible[%d]",
                pFmtp->GetEvsModeSwitch(), pFmtp->IsEvsModeSwitchVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpBr(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("br") == IMS_TRUE)
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

        IMS_TRACE_D("ExtractFmtpBr() - br[%d], visible[%d]", pFmtp->GetBrList(),
                pFmtp->IsBrListVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpBw(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("bw") == IMS_TRUE)
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

        IMS_TRACE_D("ExtractFmtpBw() - bw[%d], visible[%d]", pFmtp->GetBwList(),
                pFmtp->IsBwListVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpCmr(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("cmr") == IMS_TRUE)
    {
        pFmtp->SetCmr((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetShowCmr(IMS_TRUE);

        IMS_TRACE_D("ExtractFmtpCmr() - cmr[%d], visible[%d]", pFmtp->GetCmr(),
                pFmtp->IsCmrVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpChAwMode(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("ch-aw-recv") == IMS_TRUE)
    {
        // pFmtp->SetChAwRecv((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetReceivedChAwRecv((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
        pFmtp->SetChAwRecv(pFmtp->GetReceivedChAwRecv());
        pFmtp->SetShowChannelAwMode(IMS_TRUE);
        /*
                    if ((pFmtp->GetChAwRecv() == 0) || (pFmtp->GetChAwRecv() == 2) ||
            (pFmtp->GetChAwRecv()
            == 3)
                        || (pFmtp->GetChAwRecv() == 5) || (pFmtp->GetChAwRecv() == 7) ||
            (pFmtp->GetChAwRecv() == -1)) { pFmtp->SetShowChannelAwMode(IMS_TRUE); } else {
                        pFmtp->SetChAwRecv(0);
                        pFmtp->SetShowChannelAwMode(IMS_FALSE);
                    }
        */

        IMS_TRACE_D("ExtractFmtpChAwMode() - ch-aw-recv[%d], visible[%d]", pFmtp->GetChAwRecv(),
                pFmtp->IsChannelAwModeVisible(), 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpBrSend(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("br-send") == IMS_TRUE)
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

        IMS_TRACE_D("ExtractFmtpBrSend() - br-send[%d]", pFmtp->GetBrRecv(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpBrRecv(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("br-recv") == IMS_TRUE)
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

        IMS_TRACE_D("ExtractFmtpBrRecv() - br-recv[%d]", pFmtp->GetBrSend(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpBwSend(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("bw-send") == IMS_TRUE)
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

        IMS_TRACE_D("ExtractFmtpBwSend() - bw-send[%d]", pFmtp->GetBwRecv(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractFmtpBwRecv(
        IN const ImsList<AString>& objSplitEqual, OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (objSplitEqual.GetAt(0).Equals("bw-recv") == IMS_TRUE)
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

        IMS_TRACE_D("ExtractFmtpBwRecv() - bw-recv[%d]", pFmtp->GetBwSend(), 0, 0);

        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
void AudioProfileExtractor::SetEvsBrVisible(OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    if ((pFmtp->GetBrRecv() != 0) && (pFmtp->GetBrSend() != 0))
    {
        pFmtp->SetShowBrList(IMS_FALSE);
    }

    IMS_TRACE_D("SetEvsBrVisible() - br-recv[%d], br-send[%d], br visible[%d]", pFmtp->GetBrRecv(),
            pFmtp->GetBrSend(), pFmtp->IsBrListVisible());
}

PRIVATE
void AudioProfileExtractor::SetEvsBwVisible(OUT AudioProfile::EvsFmtp* pFmtp)
{
    if (pFmtp == IMS_NULL)
    {
        return;
    }

    if ((pFmtp->GetBwRecv() != 0) && (pFmtp->GetBwSend() != 0))
    {
        pFmtp->SetShowBwList(IMS_FALSE);
    }

    IMS_TRACE_D("SetEvsBwVisible() - bw-recv[%d], bw-send[%d], bw visible[%d]", pFmtp->GetBwRecv(),
            pFmtp->GetBwSend(), pFmtp->IsBwListVisible());
}

PRIVATE
IMS_BOOL AudioProfileExtractor::ExtractTelephoneEventFmtp(
        IN const AString& strFmtp, OUT AudioProfile::Payload* pPayload)
{
    if (pPayload == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AudioProfile::TelephoneEventFmtp* pFmtp = new AudioProfile::TelephoneEventFmtp();

    if (pFmtp == IMS_NULL)
    {
        return IMS_FALSE;
    }

    //[RFC4733] For backward compatibility, if no "events" parameter is received,
    // the sender SHOULD assume support for the DTMF events 0-15 but for no other events.
    pFmtp->SetEvents((strFmtp != IMS_NULL && strFmtp.GetLength() > 0) ? strFmtp : "0-15");

    pPayload->SetFmtp(pFmtp);

    return IMS_TRUE;
}

PRIVATE
void AudioProfileExtractor::ExtractPtime(
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    pProfile->SetPtime(pDescriptor->GetAttributeInt(SdpAttribute::PTIME));
}

PRIVATE
void AudioProfileExtractor::ExtractMaxPtime(
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    pProfile->SetMaxPtime(pDescriptor->GetAttributeInt(SdpAttribute::MAXPTIME));
}

PRIVATE
void AudioProfileExtractor::ExtractRtcpXr(
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    ImsList<AString> lstRtcpXrAttr = pDescriptor->GetAttributes(SdpAttribute::RTCP_XR);

    if (lstRtcpXrAttr.GetSize() > 0)
    {
        pProfile->SetSupportRtcpXr(IMS_TRUE);
    }

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
}

PRIVATE
void AudioProfileExtractor::ExtractAnbr(
        IN IMediaDescriptor* pDescriptor, OUT AudioProfile* pProfile)
{
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return;
    }

    pProfile->SetAnbr(
            (pDescriptor->GetAttribute(SdpAttribute::ANBR).IsEmpty()) ? IMS_TRUE : IMS_FALSE);

    IMS_TRACE_D("ExtractAnbr() - anbr[%d]", pProfile->IsAnbrSupported(), 0, 0);
}
