/**
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

#include "ServiceTrace.h"
#include "config/ImsCodec.h"
#include "config/CodecAmrConfig.h"
#include "config/CodecTelephoneEventConfig.h"
#include "config/CodecEvsConfig.h"
#include "config/CodecPcmConfig.h"
#include "config/AudioConfiguration.h"
#include "audio/AudioDef.h"
#include "MediaEnvironment.h"
#include "IService.h"
#include "audio/AudioProfileUtil.h"
#include "ServicePhoneInfo.h"
#include "ServiceSystemTime.h"
#include "MediaManager.h"
#include "MediaProfileUtil.h"
#include "MediaResourceManager.h"
#include "config/MediaSessionConfig.h"
#include "config/MediaSessionConfigFactory.h"

__IMS_TRACE_TAG_MEDIA__;

const IMS_SINT32 AudioProfileUtil::AMR_AS[8][9] = {
        {22, 22, 23, 24, 24, 25, 27, 29, 0 }, // nb, ipv4, be
        {22, 22, 23, 24, 25, 25, 28, 30, 0 }, // nb, ipv4, oa
        {30, 30, 31, 32, 32, 33, 35, 37, 0 }, // nb, ipv6, be
        {30, 30, 31, 32, 33, 33, 36, 38, 0 }, // nb, ipv6, oa
        {24, 26, 30, 31, 33, 35, 37, 40, 41}, // wb, ipv4, be
        {24, 26, 30, 32, 33, 36, 37, 40, 41}, // wb, ipv4, oa
        {32, 34, 38, 39, 41, 43, 45, 48, 49}, // wb, ipv6, be
        {32, 34, 38, 40, 41, 44, 45, 48, 49}  // wb, ipv6, oa
};

const IMS_SINT32 AudioProfileUtil::EVS_AS[4][12] = {
        {23, 24, 25, 27, 30, 34, 42, 49, 65, 81, 113, 145}, // Primary, ipv4
        {31, 32, 33, 35, 38, 42, 50, 57, 73, 89, 121, 153}, // Primary, ipv6
        {23, 26, 29, 31, 32, 35, 36, 40, 40, 0,  0,   0  }, // AMR IO, ipv4
        {31, 34, 37, 39, 40, 43, 44, 48, 48, 0,  0,   0  }  // AMR IO, ipv6
};

const AString AudioProfileUtil::EVS_BR[EVS_BR_CNT] = {
        "5.9", "7.2", "8", "9.6", "13.2", "16.4", "24.4", "32", "48", "64", "96", "128"};
const AString AudioProfileUtil::EVS_BW[EVS_BW_CNT] = {"nb", "wb", "swb", "fb"};
const AString AudioProfileUtil::EVS_BW_LIST[EVS_BW_LIST_CNT] = {
        "nb", "wb", "swb", "fb", "nb-wb", "nb-swb", "nb-fb", "wb-swb", "wb-fb"};
const AString AudioProfileUtil::AUDIO_CODEC_BANDWIDTH_STRING[EVS_BW_CNT] = {
        "NB", "WB", "SWB", "FB"};
const AString AudioProfileUtil::AUDIO_CODEC_BITRATE_STRING[3][9] = {
  // AMR NB
        {"4.75", "5.15", "5.90",  "6.70",  "7.40",  "7.95",  "10.20", "12.20", "0"    },
 // AMR WB/EVS AMR IO
        {"6.60", "8.85", "12.65", "14.25", "15.85", "18.25", "19.85", "23.05", "23.85"},
 // EVS
        {"5.90", "7.20", "8.00",  "9.60",  "13.20", "16.40", "24.40", "0",     "0"    }
};

PUBLIC GLOBAL IMS_BOOL AudioProfileUtil::SetRtcpXr(
        OUT AudioProfile* pAudioProfile, IN AudioConfiguration* pConfig)
{
    if (pAudioProfile == IMS_NULL || pConfig == IMS_NULL)
    {
        return IMS_FALSE;
    }

    pAudioProfile->SetSupportRtcpXr(pConfig->IsRtcpXrEnabled());

    IMS_TRACE_D("SetRtcpXr() Support Rtcp-Xr[%d]", pAudioProfile->IsRtcpXrSupported(), 0, 0);

    if (pAudioProfile->IsRtcpXrSupported() == IMS_TRUE)
    {
        if (pConfig->IsRtcpXrVoipEnabled() == IMS_TRUE)
        {
            pAudioProfile->GetRtcpXrAttr().SetSupportVoipMetrics(IMS_TRUE);
        }
        if (pConfig->IsRtcpXrStatisticsEnabled() == IMS_TRUE)
        {
            pAudioProfile->GetRtcpXrAttr().SetSupportStatisticMetrics(IMS_TRUE);
        }
        if (pConfig->IsRtcpXrPlrEnabled() == IMS_TRUE)
        {
            pAudioProfile->GetRtcpXrAttr().SetSupportPacketLossRle(IMS_TRUE);
        }
        if (pConfig->IsRtcpXrPdrEnabled() == IMS_TRUE)
        {
            pAudioProfile->GetRtcpXrAttr().SetSupportPacketDuplicatedRle(IMS_TRUE);
        }

        IMS_TRACE_D("SetRtcpXr() VoipMetrics[%d], StatisticMetrics[%d], PacketLossRle[%d]",
                pAudioProfile->GetRtcpXrAttr().IsVoipMetricsSupported(),
                pAudioProfile->GetRtcpXrAttr().IsStatisticMetricsSupported(),
                pAudioProfile->GetRtcpXrAttr().IsPacketLossRleSupported());
        IMS_TRACE_D("SetRtcpXr() PacketDuplicatedRl[%d]",
                pAudioProfile->GetRtcpXrAttr().IsPacketDuplicatedRleSupported(), 0, 0);
    }

    return IMS_TRUE;
}

PUBLIC GLOBAL const IMS_SINT32* AudioProfileUtil::GetAmrAsArray(
        IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6)
{
    const IMS_SINT32* pArrAmr;
    IMS_SINT32 AsArrIndex = 0;
    if (eCodec == AUDIO_CODEC_AMRWB)
    {
        AsArrIndex += 4;
    }
    if (bIpV6 == IMS_TRUE)
    {
        AsArrIndex += 2;
    }
    if (nOctet == 1)
    {
        AsArrIndex += 1;
    }
    pArrAmr = AMR_AS[AsArrIndex];
    return pArrAmr;
}

PUBLIC GLOBAL const IMS_SINT32* AudioProfileUtil::GetEvsAsArray(
        IN IMS_SINT32 nEVSFormat, IN IMS_BOOL bIpV6)
{
    const IMS_SINT32* pArrEvs;
    IMS_SINT32 AsArrIndex = 0;
    if (nEVSFormat == 1)
    {
        AsArrIndex += 2;  // AMR IO mode == 1
    }
    if (bIpV6 == IMS_TRUE)
    {
        AsArrIndex += 1;
    }
    pArrEvs = EVS_AS[AsArrIndex];
    return pArrEvs;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::ConvertToBandwidthAS(IN IMS_SINT32 eCodec,
        IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6, IN IMS_SINT32 nModeSet,
        IN IMS_BOOL bGetMaxValue /*= IMS_FALSE*/)
{
    IMS_TRACE_D("ConvertToBandwidthAS() - Entered. eCodec[%d] nOctet[%d] nModeSet[%d]", eCodec,
            nOctet, nModeSet);

    IMS_SINT32 nResultAs = -1;

    const IMS_SINT32* pArrAmr;
    IMS_SINT32 nModeCount;

    pArrAmr = GetAmrAsArray(eCodec, nOctet, bIpV6);
    // Bandwidth for bandwidth-efficient mode
    // Bandwidth for octet-align mode

    if (eCodec == AUDIO_CODEC_AMRWB)
    {
        nModeCount = 9;
    }
    else
    {
        nModeCount = 8;
    }

    if (bGetMaxValue || (nModeSet >= nModeCount))
    {
        // Bandwidth for bandwidth-efficient mode
        // Bandwidth for octet-align mode
        nModeSet = nModeCount - 1;
    }

    nResultAs = pArrAmr[nModeSet];

    IMS_TRACE_D("ConvertToBandwidthAS() - Ended : IPv6[%d], nAs[%d]", bIpV6, nResultAs, 0);
    return nResultAs;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::ConvertToBandwidthAS(IN IMS_SINT32 eCodec,
        IN IMS_BOOL bIpV6, IN IMS_SINT32 nCodecFormat, IN IMS_SINT32 nCodecMode,
        IN IMS_BOOL bGetMaxValue)
{
    IMS_TRACE_D("ConvertToBandwidthAS() - Entered. eCodec[%d] bIpV6[%d] nCodecFormat[%d]", eCodec,
            bIpV6, nCodecFormat);

    IMS_SINT32 nResultAs = -1;

    const IMS_SINT32* pArrEvs;
    pArrEvs = GetEvsAsArray(nCodecFormat, bIpV6);

    IMS_SINT32 nModeCount = 0;

    if (nCodecFormat == 1)
    {
        nModeCount = 9;  // AMR IO Mode
    }
    else
    {
        nModeCount = 12;
    }

    if (bGetMaxValue || (nCodecMode >= nModeCount))
    {
        nCodecMode = nModeCount - 1;
    }

    nResultAs = pArrEvs[nCodecMode];
    IMS_TRACE_D("ConvertToBandwidthAS() - Ended : nAs[%d]", nResultAs, 0, 0);

    return nResultAs;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::ConvertToModeSet(
        IN IMS_SINT32 eCodec, IN IMS_SINT32 nOctet, IN IMS_BOOL bIpV6, IN IMS_SINT32 nAs)
{
    IMS_TRACE_D("ConvertToModeSet() eCodec[%d], nOctet[%d], nAs[%d]", eCodec, nOctet, nAs);

    IMS_SINT32 nModeSet = -1;
    const IMS_SINT32* pArrAmr;
    IMS_SINT32 nModeCount;
    pArrAmr = GetAmrAsArray(eCodec, nOctet, bIpV6);

    if (pArrAmr == IMS_NULL)
    {
        return -1;
    }

    if (eCodec == AUDIO_CODEC_AMRWB)
    {
        nModeCount = 9;
    }
    else
    {
        nModeCount = 8;
    }

    for (IMS_SINT32 i = 0; i < nModeCount; ++i)
    {
        if (nAs >= pArrAmr[i])
        {
            nModeSet = i;
        }
        else
        {
            break;
        }
    }

    IMS_TRACE_D("ConvertToModeSet() - Result : bIpV6[%d]m nModeSet[%d]", bIpV6, nModeSet, 0);
    return nModeSet;
}

/*!
 *  @brief      UpdateAudioProfileBandwidth
 *  @details    UpdateAudioProfileBandwidth for update bandwidth(AS) at audio profile
 */
PUBLIC GLOBAL IMS_BOOL AudioProfileUtil::UpdateAudioProfileBandwidth(
        OUT AudioProfile* pAudioProfile, IN AudioConfiguration* pConfig)
{
    if ((pAudioProfile == IMS_NULL) || (pConfig == IMS_NULL))
    {
        return IMS_FALSE;
    }

    IMS_SINT32 nAsOptimal = -1;
    IMS_SINT32 nAsMax = -1;
    IMS_SINT32 nCurrAs = 0;
    AUDIO_CODEC nCurrCodec = AUDIO_CODEC_NONE;

    for (IMS_UINT32 i = 0; i < pAudioProfile->GetPayloadList().GetSize(); i++)
    {
        AudioProfile::Payload* pAudioPayload = pAudioProfile->GetPayloadAt(i);
        if (pAudioPayload == IMS_NULL)
        {
            continue;
        }

        if ((pAudioPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR-WB") == IMS_TRUE) ||
                (pAudioPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("AMR") == IMS_TRUE))
        {
            AudioProfile::AmrFmtp* pAmrFmtp =
                    reinterpret_cast<AudioProfile::AmrFmtp*>(pAudioPayload->GetFmtp());
            if (pAmrFmtp == IMS_NULL)
            {
                continue;
            }

            if (pAudioPayload->GetRtpMap().GetSamplingRate() == 16000)
            {
                nCurrCodec = AUDIO_CODEC_AMRWB;
            }
            else
            {
                nCurrCodec = AUDIO_CODEC_AMR;
            }

            // find max modeset
            IMS_SINT32 nMaxModeset = 0;
            if (pAmrFmtp->GetModeSetList() == 0)
            {
                if (nCurrCodec == AUDIO_CODEC_AMRWB)
                {
                    nMaxModeset = 8;  // amr wb case
                }
                else
                {
                    nMaxModeset = 7;  // amr nb case
                }
            }
            else
            {
                for (nMaxModeset = 8; nMaxModeset >= 0; nMaxModeset--)
                {
                    if (pAmrFmtp->GetModeSetList() & (1 << nMaxModeset))
                    {
                        break;
                    }
                }
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->GetOctetAlign(),
                    pAudioProfile->GetIpAddress().IsIPv6Address(), nMaxModeset);
            if (nCurrAs > nAsOptimal)
            {
                nAsOptimal = nCurrAs;
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec, pAmrFmtp->GetOctetAlign(),
                    pAudioProfile->GetIpAddress().IsIPv6Address(), nMaxModeset, IMS_TRUE);
            if (nCurrAs > nAsMax)
            {
                nAsMax = nCurrAs;
            }
        }
        else if (pAudioPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("EVS") == IMS_TRUE)
        {
            nCurrCodec = AUDIO_CODEC_EVS;
            AudioProfile::EvsFmtp* pEvsFmtp =
                    reinterpret_cast<AudioProfile::EvsFmtp*>(pAudioPayload->GetFmtp());
            if (pEvsFmtp == IMS_NULL)
            {
                continue;
            }

            // find max br
            IMS_SINT32 nMaxBr = 0;
            {
                if (pEvsFmtp->GetBrList() == 0)
                {
                    nMaxBr = 6;
                }
                else
                {
                    for (nMaxBr = 6; nMaxBr >= 0; nMaxBr--)
                    {
                        if (pEvsFmtp->GetBrList() & (1 << nMaxBr))
                        {
                            break;
                        }
                    }
                }
            }

            nCurrAs = ConvertToBandwidthAS(nCurrCodec,
                    pAudioProfile->GetIpAddress().IsIPv6Address(), 0, nMaxBr);  // primary mode
            if (nCurrAs > nAsOptimal)
            {
                nAsOptimal = nCurrAs;
            }

            nCurrAs = ConvertToBandwidthAS(
                    nCurrCodec, pAudioProfile->GetIpAddress().IsIPv6Address(), 0, nMaxBr, IMS_TRUE);
            if (nCurrAs > nAsMax)
            {
                nAsMax = nCurrAs;
            }
        }
        else if ((pAudioPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMU") ==
                         IMS_TRUE) ||
                (pAudioPayload->GetRtpMap().GetPayloadType().EqualsIgnoreCase("PCMA") == IMS_TRUE))
        {
            if (72 > nAsMax)  // 72 is PCMU/PCMA AS value at IPv6
            {
                nAsMax = 72;
                nAsOptimal = 72;
            }
        }
    }

    pAudioProfile->SetBandwidthAs(pConfig->GetAsBandwidthKbps());

    MediaProfileUtil::SetRtcpRsRr(pAudioProfile, pConfig);
    IMS_TRACE_D("UpdateAudioProfileBandwidth() update AS[%d]", nAsMax, 0, 0);

    return IMS_TRUE;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::GetLargestModesetInFmtp(
        IN const AString& strCodec, IN AudioProfile::Payload* pPayload)
{
    const IMS_SINT32 NO_MODESET = -1;
    const IMS_SINT32 AMR_MAX_MODESET = 7;
    const IMS_SINT32 AMRWB_MAX_MODESET = 8;
    const IMS_SINT32 EVS_PRIMARY_MODE_MAX_MODESET = 11;
    const IMS_SINT32 EVS_IO_MODE_MAX_MODESET = 8;

    if (pPayload == IMS_NULL)
    {
        return NO_MODESET;
    }
    if (!strCodec.EqualsIgnoreCase("AMR") && !strCodec.EqualsIgnoreCase("AMR-WB") &&
            !strCodec.EqualsIgnoreCase("EVS"))
    {
        return NO_MODESET;
    }

    if (strCodec.EqualsIgnoreCase("AMR") || strCodec.EqualsIgnoreCase("AMR-WB"))
    {
        AudioProfile::AmrFmtp* pAmrFmtp =
                reinterpret_cast<AudioProfile::AmrFmtp*>(pPayload->GetFmtp());
        if (pAmrFmtp == NULL)
        {
            return NO_MODESET;
        }

        if (pAmrFmtp->GetModeSetList() == 0)
        {
            if (strCodec.EqualsIgnoreCase("AMR"))
            {
                return AMR_MAX_MODESET;
            }
            else
            {
                return AMRWB_MAX_MODESET;
            }
        }
        else
        {
            IMS_SINT32 nModeSet;
            for (nModeSet = 8; nModeSet >= 0; nModeSet--)
            {
                IMS_UINT32 nMatch = pAmrFmtp->GetModeSetList() & (1 << nModeSet);
                if (nMatch)
                {
                    return nModeSet;
                }
            }

            return nModeSet;
        }
    }
    else if (strCodec.EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp =
                reinterpret_cast<AudioProfile::EvsFmtp*>(pPayload->GetFmtp());
        if (pEvsFmtp == NULL)
        {
            return NO_MODESET;
        }

        // Primary mode
        if (pEvsFmtp->GetEvsModeSwitch() != 1)
        {
            // check bitrate...
            if (pEvsFmtp->GetBrList() == 0)
            {
                return EVS_PRIMARY_MODE_MAX_MODESET;
            }
            else
            {
                IMS_SINT32 nBitrate = 0;
                for (nBitrate = 11; nBitrate >= 0; nBitrate--)
                {
                    IMS_UINT32 nMatch = pEvsFmtp->GetBrList() & (1 << nBitrate);
                    if (nMatch)
                    {
                        return nBitrate;
                    }
                }
            }
        }
        else  // AMR IO mode
        {
            if (pEvsFmtp->GetModeSetList() == 0)
            {
                return EVS_IO_MODE_MAX_MODESET;
            }
            else
            {
                IMS_SINT32 nModeSet = 0;
                for (nModeSet = 8; nModeSet >= 0; nModeSet--)
                {
                    IMS_UINT32 nMatch = pEvsFmtp->GetModeSetList() & (1 << nModeSet);
                    if (nMatch)
                    {
                        return nModeSet;
                    }
                }
            }
        }
    }

    return NO_MODESET;
}

PUBLIC GLOBAL IMS_SINT32 AudioProfileUtil::GetModesetList(
        IN const AString& strCodec, IN AudioProfile::Payload* pPayload)
{
    const IMS_SINT32 NO_MODESET = 0;
    const IMS_SINT32 EVS_PRIMARY_MODE_MAX_MODESET = 11;

    if (pPayload == IMS_NULL)
    {
        return NO_MODESET;
    }
    if (!strCodec.EqualsIgnoreCase("AMR") && !strCodec.EqualsIgnoreCase("AMR-WB") &&
            !strCodec.EqualsIgnoreCase("EVS"))
    {
        return NO_MODESET;
    }

    if (strCodec.EqualsIgnoreCase("AMR") || strCodec.EqualsIgnoreCase("AMR-WB"))
    {
        AudioProfile::AmrFmtp* pAmrFmtp =
                reinterpret_cast<AudioProfile::AmrFmtp*>(pPayload->GetFmtp());
        if (pAmrFmtp == NULL)
        {
            return NO_MODESET;
        }

        if (pAmrFmtp->GetModeSetList() > 0)
        {
            IMS_TRACE_D("GetModesetList is %d", pAmrFmtp->GetModeSetList(), 0, 0);
            return pAmrFmtp->GetModeSetList();
        }
        else
        {
            IMS_TRACE_D("GetModesetList is %d", pAmrFmtp->GetDefaultRtpModeSet(), 0, 0);
            return pAmrFmtp->GetDefaultRtpModeSet();
        }
    }
    else if (strCodec.EqualsIgnoreCase("EVS"))
    {
        AudioProfile::EvsFmtp* pEvsFmtp =
                reinterpret_cast<AudioProfile::EvsFmtp*>(pPayload->GetFmtp());
        if (pEvsFmtp == NULL)
        {
            return NO_MODESET;
        }

        // Primary mode
        if (pEvsFmtp->GetEvsModeSwitch() != 1)
        {
            if (pEvsFmtp->GetBrList() == 0)
            {
                IMS_TRACE_D(
                        "BrList is 0 -> GetModesetList is %d", EVS_PRIMARY_MODE_MAX_MODESET, 0, 0);
                return EVS_PRIMARY_MODE_MAX_MODESET;
            }
            else
            {
                IMS_TRACE_D("GetModesetList is %d", pEvsFmtp->GetBrList(), 0, 0);
                return pEvsFmtp->GetBrList();
            }
        }
        else  // AMR IO mode
        {
            if (pEvsFmtp->GetModeSetList() > 0)
            {
                IMS_TRACE_D("GetModesetList is %d", pEvsFmtp->GetModeSetList(), 0, 0);
                return pEvsFmtp->GetModeSetList();
            }
            else
            {
                IMS_TRACE_D("GetModesetList is %d", pEvsFmtp->GetDefaultRtpModeSet(), 0, 0);
                return pEvsFmtp->GetDefaultRtpModeSet();
            }
        }
    }

    return NO_MODESET;
}

PUBLIC GLOBAL void AudioProfileUtil::SetAnbr(
        OUT AudioProfile* pProfile, IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId)
{
    if (pProfile != IMS_NULL)
    {
        MediaSessionConfig* pMediaSessionConfig =
                MediaSessionConfigFactory::GetInstance()->FindMediaSessionConfig(
                        nSlotId, pEnvironment->eServiceType);

        if (pMediaSessionConfig != IMS_NULL)
        {
            pProfile->SetAnbr(pMediaSessionConfig->IsAnbrSupported());
            IMS_TRACE_D("SetAnbr anbr : %d", pProfile->IsAnbrSupported(), 0, 0);
        }
    }
}
