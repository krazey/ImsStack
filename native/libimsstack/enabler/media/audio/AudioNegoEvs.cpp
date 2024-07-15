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

#include "audio/AudioNegoEvs.h"
#include "audio/AudioProfileUtil.h"

#define MODESET_MAX_AMRWB 8

const AString SEMICOLON = ";";
const AString COMMA = ",";

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AString AudioNegoEvs::SetSdpFmtpFromEvsFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp)
{
    IMS_TRACE_I("SetSdpFmtpFromEvsFmtp()", 0, 0, 0);

    AString strFmtp = AString::ConstNull();

    if (pEvsFmtp == IMS_NULL)
    {
        return strFmtp;
    }

    AddPtimeToFmtp(pEvsFmtp, strFmtp);
    AddMaxPtimeToFmtp(pEvsFmtp, strFmtp);
    AddDtxToFmtp(pEvsFmtp, strFmtp);
    AddHfOnlyToFmtp(pEvsFmtp, strFmtp);
    AddEvsModeSwitchToFmtp(pEvsFmtp, strFmtp);
    AddMaxRedToFmtp(pEvsFmtp, strFmtp);
    AddBwToFmtp(pEvsFmtp, strFmtp);
    AddBrToFmtp(pEvsFmtp, strFmtp);
    AddCmrToFmtp(pEvsFmtp, strFmtp);
    AddChannelAwModeToFmtp(pEvsFmtp, strFmtp);
    AddModeSetListToFmtp(pEvsFmtp, strFmtp);
    AddModeChangeCapabilityToFmtp(pEvsFmtp, strFmtp);
    AddModeChangePeriodToFmtp(pEvsFmtp, strFmtp);
    AddModeChangeNeighborToFmtp(pEvsFmtp, strFmtp);
    AddBwSendToFmtp(pEvsFmtp, strFmtp);
    AddBwRecvToFmtp(pEvsFmtp, strFmtp);
    AddBrSendToFmtp(pEvsFmtp, strFmtp);
    AddBrRecvToFmtp(pEvsFmtp, strFmtp);

    return strFmtp;
}

PUBLIC void AudioNegoEvs::AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator)
{
    if (str.GetLength() > 0)
    {
        str.Append(separator);
    }
}

PUBLIC void AudioNegoEvs::AddPtimeToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddPtimeToFmtp() ptime=%d, visible=%d", pEvsFmtp->GetPtime(),
            pEvsFmtp->IsMaxPtimeVisible(), 0);

    if (pEvsFmtp->GetPtime() != AudioProfile::EvsFmtp::DEFAULT_PTIME &&
            pEvsFmtp->IsPtimeVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("ptime=%d", pEvsFmtp->GetPtime());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddMaxPtimeToFmtp(
        IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddMaxPtimeToFmtp() ptime=%d, visible=%d", pEvsFmtp->GetMaxPtime(),
            pEvsFmtp->IsMaxPtimeVisible(), 0);

    if (pEvsFmtp->GetMaxPtime() != AudioProfile::EvsFmtp::DEFAULT_MAXPTIME &&
            pEvsFmtp->IsMaxPtimeVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("maxptime=%d", pEvsFmtp->GetMaxPtime());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddDtxToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddDtxToFmtp() dtx=%d, visible=%d", pEvsFmtp->IsDtxEnabled(),
            pEvsFmtp->IsDtxVisible(), 0);

    if (pEvsFmtp->IsDtxVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("dtx=%d", pEvsFmtp->IsDtxEnabled());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddHfOnlyToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddHfOnlyToFmtp() hf-only=%d, visible=%d", pEvsFmtp->GetHfOnly(),
            pEvsFmtp->IsHfOnlyVisible(), 0);

    if (pEvsFmtp->IsHfOnlyVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("hf-only=%d", pEvsFmtp->GetHfOnly());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddEvsModeSwitchToFmtp(
        IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddEvsModeSwitchToFmtp() evs-mode-switch=%d, visible=%d",
            pEvsFmtp->GetEvsModeSwitch(), pEvsFmtp->IsEvsModeSwitchVisible(), 0);

    if (pEvsFmtp->IsEvsModeSwitchVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("evs-mode-switch=%d", pEvsFmtp->GetEvsModeSwitch());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddMaxRedToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddMaxRedToFmtp() max-red=%d, visible=%d", pEvsFmtp->GetMaxRed(),
            pEvsFmtp->IsMaxRedVisible(), 0);

    if (pEvsFmtp->IsMaxRedVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("max-red=%d", pEvsFmtp->GetMaxRed());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBwToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddBwToFmtp() bw-list=%d, visible=%d", pEvsFmtp->GetBwList(),
            pEvsFmtp->IsBwListVisible(), 0);

    if (pEvsFmtp->GetBwList() != 0 && pEvsFmtp->IsBwListVisible())
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        /** TODO: Need to check that '11' is proper later */
        for (nBandwidthList = 0; nBandwidthList <= 11; nBandwidthList++)
        {
            if ((pEvsFmtp->GetBwList()) & (1 << nBandwidthList))
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

PUBLIC void AudioNegoEvs::AddBrToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddBrToFmtp() br-list=%d, visible=%d", pEvsFmtp->GetBrList(),
            pEvsFmtp->IsBrListVisible(), 0);

    if (pEvsFmtp->IsBrListVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (pEvsFmtp->GetBrList()) & (1 << nBitrateList);
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

PUBLIC void AudioNegoEvs::AddCmrToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I(
            "AddCmrToFmtp() cmr=%d, visible=%d", pEvsFmtp->GetCmr(), pEvsFmtp->IsCmrVisible(), 0);

    if (pEvsFmtp->IsCmrVisible() == IMS_TRUE && pEvsFmtp->GetEvsModeSwitch() != 1)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("cmr=%d", pEvsFmtp->GetCmr());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddChannelAwModeToFmtp(
        IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddChannelAwModeToFmtp() ch-aw-recv=%d, visible=%d", pEvsFmtp->GetChAwRecv(),
            pEvsFmtp->IsChannelAwModeVisible(), 0);

    if (pEvsFmtp->IsChannelAwModeVisible() == IMS_TRUE && pEvsFmtp->GetEvsModeSwitch() != 1)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("ch-aw-recv=%d", pEvsFmtp->GetChAwRecv());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddModeSetListToFmtp(
        IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeSetListToFmtp() mode-set=%d, visible=%d", pEvsFmtp->GetModeSetList(),
            pEvsFmtp->IsModeSetVisible(), 0);

    if (pEvsFmtp->GetModeSetList() != 0 && pEvsFmtp->IsModeSetVisible())
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode;
        IMS_UINT32 nModeSet;

        for (nModeSet = 0; nModeSet <= MODESET_MAX_AMRWB; nModeSet++)
        {
            IMS_UINT32 nMatch = (pEvsFmtp->GetModeSetList()) & (1 << nModeSet);
            if (nMatch)
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

PUBLIC void AudioNegoEvs::AddModeChangeCapabilityToFmtp(
        IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeChangeCapabilityToFmtp() mode-change-capability=%d, visible=%d",
            pEvsFmtp->GetModeChangeCapability(), pEvsFmtp->IsModeChangeCapabilityVisible(), 0);

    if (pEvsFmtp->IsModeChangeCapabilityVisible() == IMS_TRUE && pEvsFmtp->GetEvsModeSwitch() == 1)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-capability=%d", pEvsFmtp->GetModeChangeCapability());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddModeChangePeriodToFmtp(
        IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeChangePeriodToFmtp() mode-change-period=%d, visible=%d",
            pEvsFmtp->GetModeChangePeriod(), pEvsFmtp->IsModeChangePeriodVisible(), 0);

    if (pEvsFmtp->IsModeChangePeriodVisible() == IMS_TRUE && pEvsFmtp->GetEvsModeSwitch() == 1)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-period=%d", pEvsFmtp->GetModeChangePeriod());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddModeChangeNeighborToFmtp(
        IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeChangeNeighborToFmtp() mode-change-neighbor=%d, visible=%d",
            pEvsFmtp->GetModeChangeNeighbor(), pEvsFmtp->IsModeChangeNeighborVisible(), 0);

    if (pEvsFmtp->IsModeChangeNeighborVisible() == IMS_TRUE && pEvsFmtp->GetEvsModeSwitch() == 1)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-neighbor=%d", pEvsFmtp->GetModeChangeNeighbor());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBwSendToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddBwSendToFmtp() bw-send=%d", pEvsFmtp->GetBwSend(), 0, 0);

    if (pEvsFmtp->GetBwSend() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        for (nBandwidthList = 0; nBandwidthList < AudioProfileUtil::EVS_BW_CNT; nBandwidthList++)
        {
            IMS_UINT32 nMatch = (pEvsFmtp->GetBwSend()) & (1 << nBandwidthList);
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

PUBLIC void AudioNegoEvs::AddBwRecvToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddBwRecvToFmtp() bw-recv=%d", pEvsFmtp->GetBwRecv(), 0, 0);

    if (pEvsFmtp->GetBwRecv() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        for (nBandwidthList = 0; nBandwidthList < AudioProfileUtil::EVS_BW_CNT; nBandwidthList++)
        {
            IMS_UINT32 nMatch = (pEvsFmtp->GetBwRecv()) & (1 << nBandwidthList);
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

PUBLIC void AudioNegoEvs::AddBrSendToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddBrSendToFmtp() br-send=%d", pEvsFmtp->GetBwSend(), 0, 0);

    if (pEvsFmtp->GetBrSend() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);
        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (pEvsFmtp->GetBrSend()) & (1 << nBitrateList);
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

PUBLIC void AudioNegoEvs::AddBrRecvToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp)
{
    if (pEvsFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddBrRecvToFmtp() br-recv=%d", pEvsFmtp->GetBwRecv(), 0, 0);

    if (pEvsFmtp->GetBrRecv() != 0)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (pEvsFmtp->GetBrRecv()) & (1 << nBitrateList);
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
