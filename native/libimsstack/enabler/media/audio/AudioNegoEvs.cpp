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

PUBLIC AString AudioNegoEvs::SetSdpFmtpFromEvsFmtp(IN AudioProfile::EvsFmtp* evsFmtp)
{
    IMS_TRACE_I("SetSdpFmtpFromEvsFmtp()", 0, 0, 0);

    AString strFmtp = AString::ConstNull();

    if (evsFmtp == IMS_NULL)
    {
        return strFmtp;
    }

    AddPtimeToFmtp(evsFmtp, strFmtp);
    AddMaxPtimeToFmtp(evsFmtp, strFmtp);
    AddDtxToFmtp(evsFmtp, strFmtp);
    AddHfOnlyToFmtp(evsFmtp, strFmtp);
    AddEvsModeSwitchToFmtp(evsFmtp, strFmtp);
    AddMaxRedToFmtp(evsFmtp, strFmtp);
    AddBwToFmtp(evsFmtp, strFmtp);
    AddBrToFmtp(evsFmtp, strFmtp);
    AddCmrToFmtp(evsFmtp, strFmtp);
    AddChannelAwModeToFmtp(evsFmtp, strFmtp);
    AddModeSetListToFmtp(evsFmtp, strFmtp);
    AddModeChangeCapabilityToFmtp(evsFmtp, strFmtp);
    AddModeChangePeriodToFmtp(evsFmtp, strFmtp);
    AddModeChangeNeighborToFmtp(evsFmtp, strFmtp);
    AddBwSendToFmtp(evsFmtp, strFmtp);
    AddBwRecvToFmtp(evsFmtp, strFmtp);
    AddBrSendToFmtp(evsFmtp, strFmtp);
    AddBrRecvToFmtp(evsFmtp, strFmtp);

    return strFmtp;
}

PUBLIC void AudioNegoEvs::AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator)
{
    if (str.GetLength() > 0)
    {
        str.Append(separator);
    }
}

PUBLIC void AudioNegoEvs::AddPtimeToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddPtimeToFmtp() ptime=%d, show=%d", profile->nPtime, profile->bShowMaxPtime, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nPtime != AudioProfile::EvsFmtp::DEFAULT_PTIME && profile->bShowPtime == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("ptime=%d", profile->nPtime);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddMaxPtimeToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I(
            "AddMaxPtimeToFmtp() ptime=%d, show=%d", profile->nMaxPtime, profile->bShowMaxPtime, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nMaxPtime != AudioProfile::EvsFmtp::DEFAULT_MAXPTIME &&
            profile->bShowMaxPtime == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("maxptime=%d", profile->nMaxPtime);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddDtxToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddDtxToFmtp() dtx=%d, show=%d", profile->bDtx, profile->bShowDtx, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowDtx == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("dtx=%d", profile->bDtx);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddHfOnlyToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddHfOnlyToFmtp() hf-only=%d, show=%d", profile->nHfOnly, profile->bShowHfOnly, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowHfOnly == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("hf-only=%d", profile->nHfOnly);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddEvsModeSwitchToFmtp(
        IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddEvsModeSwitchToFmtp() evs-mode-switch=%d, show=%d", profile->nEvsModeSwitch,
            profile->bShowEvsModeSwitch, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowEvsModeSwitch == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("evs-mode-switch=%d", profile->nEvsModeSwitch);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddMaxRedToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddMaxRedToFmtp() max-red=%d, show=%d", profile->nMaxRed, profile->bShowMaxRed, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowMaxRed == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("max-red=%d", profile->nMaxRed);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBwToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddBwToFmtp() bw-list=%d, show=%d", profile->nBwList, profile->bShowBwList, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nBwList != 0 && profile->bShowBwList)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        /** TODO: Need to check that '11' is proper later */
        for (nBandwidthList = 0; nBandwidthList <= 11; nBandwidthList++)
        {
            if ((profile->nBwList) & (1 << nBandwidthList))
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

        fmtp.Append("bw=");
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBrToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddBrToFmtp() br-list=%d, show=%d", profile->nBrList, profile->bShowBrList, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowBrList == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (profile->nBrList) & (1 << nBitrateList);
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

        fmtp.Append("br=");
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddCmrToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddCmrToFmtp() cmr=%d, show=%d", profile->nCmr, profile->bShowCmr, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowCmr == IMS_TRUE && profile->nEvsModeSwitch != 1)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("cmr=%d", profile->nCmr);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddChannelAwModeToFmtp(
        IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddChannelAwModeToFmtp() ch-aw-recv=%d, show=%d", profile->nChAwRecv,
            profile->bShowChannelAwMode, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowChannelAwMode == IMS_TRUE && profile->nEvsModeSwitch != 1)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("ch-aw-recv=%d", profile->nChAwRecv);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddModeSetListToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeSetListToFmtp() mode-set=%d, show=%d", profile->nModeSetList,
            profile->bShowModeSet, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nModeSetList != 0 && profile->bShowModeSet)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp, strMode;
        IMS_UINT32 nModeSet;

        for (nModeSet = 0; nModeSet <= MODESET_MAX_AMRWB; nModeSet++)
        {
            IMS_UINT32 nMatch = (profile->nModeSetList) & (1 << nModeSet);
            if (nMatch)
            {
                AppendSeparatorIfNotEmpty(strTemp, COMMA);

                strMode.Sprintf("%d", nModeSet);
                strTemp.Append(strMode);
            }
        }
        fmtp.Append("mode-set=");
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddModeChangeCapabilityToFmtp(
        IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeChangeCapabilityToFmtp() mode-change-capability=%d, show=%d",
            profile->nModeChangeCapability, profile->bShowModeChangeCapability, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowModeChangeCapability == IMS_TRUE && profile->nEvsModeSwitch == 1)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-capability=%d", profile->nModeChangeCapability);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddModeChangePeriodToFmtp(
        IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeChangePeriodToFmtp() mode-change-period=%d, show=%d",
            profile->nModeChangePeriod, profile->bShowModeChangePeriod, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowModeChangePeriod == IMS_TRUE && profile->nEvsModeSwitch == 1)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-period=%d", profile->nModeChangePeriod);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddModeChangeNeighborToFmtp(
        IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeChangeNeighborToFmtp() mode-change-neighbor=%d, show=%d",
            profile->nModeChangeNeighbor, profile->bShowModeChangeNeighbor, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowModeChangeNeighbor == IMS_TRUE && profile->nEvsModeSwitch == 1)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-neighbor=%d", profile->nModeChangeNeighbor);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBwSendToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddBwSendToFmtp() bw-send=%d", profile->nBwSend, 0, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nBwSend != 0)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        for (nBandwidthList = 0; nBandwidthList < AudioProfileUtil::EVS_BW_CNT; nBandwidthList++)
        {
            IMS_UINT32 nMatch = (profile->nBwSend) & (1 << nBandwidthList);
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

        fmtp.Append("bw-send=");
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBwRecvToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddBwRecvToFmtp() bw-recv=%d", profile->nBwRecv, 0, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nBwRecv != 0)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBandwidth, strLastBandwidth;
        IMS_UINT32 nBandwidthList;
        IMS_UINT32 nBandwidthListTotalCnt = 0;

        for (nBandwidthList = 0; nBandwidthList < AudioProfileUtil::EVS_BW_CNT; nBandwidthList++)
        {
            IMS_UINT32 nMatch = (profile->nBwRecv) & (1 << nBandwidthList);
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

        fmtp.Append("bw-recv=");
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBrSendToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddBrSendToFmtp() br-send=%d", profile->nBwSend, 0, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nBrSend != 0)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);
        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (profile->nBrSend) & (1 << nBitrateList);
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

        fmtp.Append("br-send=");
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoEvs::AddBrRecvToFmtp(IN AudioProfile::EvsFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddBrRecvToFmtp() br-recv=%d", profile->nBwRecv, 0, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->nBrRecv != 0)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp, strMode, strFirstBitrate, strLastBitrate;
        IMS_UINT32 nBitrateList;
        IMS_UINT32 nBitrateListTotalCnt = 0;

        for (nBitrateList = 0; nBitrateList < AudioProfileUtil::EVS_BR_CNT; nBitrateList++)
        {
            IMS_UINT32 nMatch = (profile->nBrRecv) & (1 << nBitrateList);
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

        fmtp.Append("br-recv=");
        fmtp.Append(strTemp);
    }
}
