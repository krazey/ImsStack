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
    if (pDescriptor == IMS_NULL || pProfile == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // IP
    pProfile->SetIpAddress(pDescriptor->GetRemoteAddress());

    // data & control port
    pProfile->SetDataPort(pDescriptor->GetRemotePort());

    if (pDescriptor->GetAttributeInt(SdpAttribute::RTCP) == IMediaDescriptor::INVALID_VALUE)
    {
        pProfile->SetControlPort(pProfile->GetDataPort() + 1);
    }
    else
    {
        pProfile->SetControlPort(pDescriptor->GetAttributeInt(SdpAttribute::RTCP));
    }

    // bandwidth
    pProfile->SetBandwidthAs(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_AS));
    pProfile->SetBandwidthRs(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RS));
    pProfile->SetBandwidthRr(pDescriptor->GetBandwidth(SdpBandwidth::TYPE_RR));

    IMS_TRACE_I("Extract() - AS[%d], RS[%d], RR[%d]", pProfile->GetBandwidthAs(),
            pProfile->GetBandwidthRs(), pProfile->GetBandwidthRr());

    // read CapaNego profile From SDP
    MakeCapaNegoProfileFromSdp(pDescriptor, &(pProfile->GetCapaNego()));

    // payload
    ImsList<SdpMediaFormat*> lstMediaFormat = pDescriptor->GetMediaFormats();

    for (IMS_UINT32 i = 0; i < lstMediaFormat.GetSize(); i++)
    {
        SdpAvCodec* pSdpCodec = DYNAMIC_CAST(SdpAvCodec*, lstMediaFormat.GetAt(i));

        if (pSdpCodec == IMS_NULL)
        {
            return IMS_FALSE;
        }

        AString strCodecName = pSdpCodec->GetName();
        AString strChannel = pSdpCodec->GetEncodingParameters();
        IMS_UINT32 nChannel;
        IMS_SINT32 nPayloadTypeNumber = pSdpCodec->GetPayloadType();

        if ((strChannel != IMS_NULL && strChannel.EqualsIgnoreCase("1")) &&
                (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR") ||
                        strCodecName.EqualsIgnoreCase("EVS")))
        {
            nChannel = 1;
        }
        else
        {
            nChannel = 0;
        }

        IMS_TRACE_D("Extract() - At[%d], PayloadType[%d], ClockRate[%d]", i,
                pSdpCodec->GetPayloadType(), pSdpCodec->GetClockRate());

        AudioProfile::Payload* pPayload = new AudioProfile::Payload();
        pPayload->SetRtpMap(
                pSdpCodec->GetPayloadType(), strCodecName, pSdpCodec->GetClockRate(), nChannel);

        if (strCodecName.EqualsIgnoreCase("AMR-WB") || strCodecName.EqualsIgnoreCase("AMR"))
        {
            // Create AMR fmtp
            AudioProfile::AmrFmtp* pAmrFmtp = new AudioProfile::AmrFmtp();
            GetFmtpFromString(pSdpCodec->GetFormatSpecificParameter(), pAmrFmtp);
            pPayload->SetFmtp(pAmrFmtp);
        }
        else if (strCodecName.EqualsIgnoreCase("telephone-event"))
        {
            // Create Telephone event fmtp
            AudioProfile::TelephoneEventFmtp* pTeFmtp = new AudioProfile::TelephoneEventFmtp();

            //[RFC4733] For backward compatibility, if no"events" parameter is received,
            // the sender SHOULD assume support for the DTMF events 0-15 but for no other events.
            if (pSdpCodec->GetFormatSpecificParameter() != IMS_NULL &&
                    pSdpCodec->GetFormatSpecificParameter().GetLength() > 0)
            {
                pTeFmtp->SetEvents(pSdpCodec->GetFormatSpecificParameter());
            }
            else
            {
                pTeFmtp->SetEvents("0-15");  // default value
            }

            pPayload->SetFmtp(pTeFmtp);
        }
        else if (strCodecName.EqualsIgnoreCase("EVS"))
        {
            // Create EVS fmtp
            AudioProfile::EvsFmtp* pEvsFmtp = new AudioProfile::EvsFmtp();
            // check and get EVS fmtp
            GetFmtpFromString(pSdpCodec->GetFormatSpecificParameter(), pEvsFmtp);
            // set Fmpt to payload
            pPayload->SetFmtp(pEvsFmtp);
        }
        else if (nPayloadTypeNumber == 0 || nPayloadTypeNumber == 8)
        {  // PCMU or PCMA case
            // do nothing.
            IMS_TRACE_D("Extract() - do nothing codec[%s]", strCodecName.GetStr(), 0, 0);
        }
        else
        {
            IMS_TRACE_E(
                    0, "Extract() - NOT SUPPORTED audio codec[%s]", strCodecName.GetStr(), 0, 0);
            delete pPayload;
            continue;
        }

        if (pPayload != IMS_NULL)
        {
            pProfile->GetPayloadList().Append(pPayload);
        }
    }

    // direction
    pProfile->SetDirection((MEDIA_DIRECTION)pDescriptor->GetDirection());

    if (pProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
    {
        IMS_TRACE_D("Extract() - Audio Direction does not exist", 0, 0, 0);
        // check session level attribute Direction
        pProfile->SetDirection((MEDIA_DIRECTION)pSessionDescriptor->GetDirection());

        if (pProfile->GetDirection() == MEDIA_DIRECTION_INVALID)
        {
            pProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);
        }
    }

    // ptime & max_ptime
    pProfile->SetPtime(pDescriptor->GetAttributeInt(SdpAttribute::PTIME));
    pProfile->SetMaxPtime(pDescriptor->GetAttributeInt(SdpAttribute::MAXPTIME));

    // RTCP-XR
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

    // ANBR
    pProfile->SetAnbr(IMS_FALSE);
    if (pDescriptor->GetAttribute(SdpAttribute::ANBR) == IMS_SUCCESS)
    {
        pProfile->SetAnbr(IMS_TRUE);
    }

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::GetFmtpFromString(
        IN const AString& strFmtp, OUT AudioProfile::EvsFmtp* pFmtp)
{
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
            IMS_TRACE_D("GetFmtpFromString() - Invalid fmtp parameter(%s) at index[%d]",
                    strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (objSplitEqual.GetAt(0).Equals("ptime") == IMS_TRUE)
        {
            pFmtp->SetPtime((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowPtime(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("maxptime") == IMS_TRUE)
        {
            pFmtp->SetMaxPtime((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowMaxPtime(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("dtx") == IMS_TRUE)
        {
            pFmtp->SetDtx((IMS_BOOL)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowDtx(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("hf-only") == IMS_TRUE)
        {
            pFmtp->SetHfOnly((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowHfOnly(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("evs-mode-switch") == IMS_TRUE)
        {
            pFmtp->SetEvsModeSwitch((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowEvsModeSwitch(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("max-red") == IMS_TRUE)
        {
            pFmtp->SetMaxRed((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowMaxRed(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("br") == IMS_TRUE)
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
        }
        else if (objSplitEqual.GetAt(0).Equals("bw") == IMS_TRUE)
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
        }
        else if (objSplitEqual.GetAt(0).Equals("cmr") == IMS_TRUE)
        {
            pFmtp->SetCmr((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowCmr(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("ch-aw-recv") == IMS_TRUE)
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
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-set") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');
            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(j).ToInt32();
                pFmtp->SetModeSetList((pFmtp->GetModeSetList() | (1 << nModeSet)));
            }
            pFmtp->SetShowModeSet(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-capability") == IMS_TRUE)
        {
            pFmtp->SetModeChangeCapability((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowModeChangeCapability(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-period") == IMS_TRUE)
        {
            pFmtp->SetModeChangePeriod((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowModeChangePeriod(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-neighbor") == IMS_TRUE)
        {
            pFmtp->SetModeChangeNeighbor((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowModeChangeNeighbor(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("br-send") == IMS_TRUE)
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
        }
        else if (objSplitEqual.GetAt(0).Equals("br-recv") == IMS_TRUE)
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
        }
        else if (objSplitEqual.GetAt(0).Equals("bw-send") == IMS_TRUE)
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
        }
        else if (objSplitEqual.GetAt(0).Equals("bw-recv") == IMS_TRUE)
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
        }
    }

    // check br/bw uni direction check
    if ((pFmtp->GetBrRecv() != 0) && (pFmtp->GetBrSend() != 0))
    {
        pFmtp->SetShowBrList(IMS_FALSE);
    }

    if ((pFmtp->GetBwRecv() != 0) && (pFmtp->GetBwSend() != 0))
    {
        pFmtp->SetShowBrList(IMS_FALSE);
    }

    IMS_TRACE_D("GetFmtpFromString() - EVS : ModeSet[0x%04x], Bitrate[0x%04x], Bandwidth[0x%04x]",
            pFmtp->GetModeSetList(), pFmtp->GetBrList(), pFmtp->GetBwList());
    IMS_TRACE_D("GetFmtpFromString() - EVS : MaxRed[%d], ReceivedChAwRecv[%d]", pFmtp->GetMaxRed(),
            pFmtp->GetReceivedChAwRecv(), 0);

    return IMS_TRUE;
}

PRIVATE
IMS_BOOL AudioProfileExtractor::GetFmtpFromString(
        IN const AString& strFmtp, OUT AudioProfile::AmrFmtp* pFmtp)
{
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
            IMS_TRACE_D("GetFmtpFromString() - Invalid fmtp parameter(%s) at index[%d]",
                    strTmp.GetStr(), i, 0);
            continue;
        }

        if ((objSplitEqual.GetAt(0).GetLength() == 0) || (objSplitEqual.GetAt(1).GetLength() == 0))
        {
            continue;
        }

        if (objSplitEqual.GetAt(0).Equals("mode-set") == IMS_TRUE)
        {
            ImsList<AString> objSplitComma = objSplitEqual.GetAt(1).Split(',');

            for (IMS_UINT32 j = 0; j < objSplitComma.GetSize(); j++)
            {
                IMS_UINT32 nModeSet = (IMS_UINT32)objSplitComma.GetAt(j).ToInt32();
                pFmtp->SetModeSetList((pFmtp->GetModeSetList() | (1 << nModeSet)));
                pFmtp->SetShowModeSet(IMS_TRUE);
            }
        }
        else if (objSplitEqual.GetAt(0).Equals("octet-align") == IMS_TRUE)
        {
            pFmtp->SetOctetAlign((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowOctetAlign(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-capability") == IMS_TRUE)
        {
            pFmtp->SetModeChangeCapability((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowModeChangeCapability(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-period") == IMS_TRUE)
        {
            pFmtp->SetModeChangePeriod((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowModeChangePeriod(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("mode-change-neighbor") == IMS_TRUE)
        {
            pFmtp->SetModeChangeNeighbor((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowModeChangeNeighbor(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("max-red") == IMS_TRUE)
        {
            pFmtp->SetMaxRed((IMS_UINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowMaxRed(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("ptime") == IMS_TRUE)
        {
            pFmtp->SetPtime((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowPtime(IMS_TRUE);
        }
        else if (objSplitEqual.GetAt(0).Equals("maxptime") == IMS_TRUE)
        {
            pFmtp->SetMaxPtime((IMS_SINT32)objSplitEqual.GetAt(1).ToInt32());
            pFmtp->SetShowMaxPtime(IMS_TRUE);
        }
    }

    IMS_TRACE_D("GetFmtpFromString() Ended. ModeSet[0x%04x], OctetAlign[%d], ModeChangeCapa[%d]",
            pFmtp->GetModeSetList(), pFmtp->GetOctetAlign(), pFmtp->GetModeChangeCapability());
    IMS_TRACE_D("GetFmtpFromString() Ended. MaxRed[%d]", pFmtp->GetMaxRed(), 0, 0);

    return IMS_TRUE;
}
