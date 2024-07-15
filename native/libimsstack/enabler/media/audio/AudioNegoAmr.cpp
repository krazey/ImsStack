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
#include "audio/AudioNegoAmr.h"

#define MODESET_MAX_AMR   7
#define MODESET_MAX_AMRWB 8

const AString SEMICOLON = ";";
const AString COMMA = ",";

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AString AudioNegoAmr::SetSdpFmtpFromAmrFmtp(IN AudioProfile::AmrFmtp* pAmrFmtp)
{
    IMS_TRACE_I("SetSdpFmtpFromAmrFmtp()", 0, 0, 0);

    AString strFmtp = AString::ConstNull();

    if (pAmrFmtp == IMS_NULL)
    {
        return strFmtp;
    }

    AddModeSetListToFmtp(pAmrFmtp, strFmtp);
    AddOctetAlignToFmtp(pAmrFmtp, strFmtp);
    AddModeChangeCapabilityToFmtp(pAmrFmtp, strFmtp);
    AddModeChangePeriodToFmtp(pAmrFmtp, strFmtp);
    AddModeChangeNeighborToFmtp(pAmrFmtp, strFmtp);
    AddMaxRedToFmtp(pAmrFmtp, strFmtp);
    AddPtimeToFmtp(pAmrFmtp, strFmtp);
    AddMaxPtimeToFmtp(pAmrFmtp, strFmtp);

    return strFmtp;
}

PUBLIC void AudioNegoAmr::AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator)
{
    if (str.GetLength() > 0)
    {
        str.Append(separator);
    }
}

PUBLIC void AudioNegoAmr::AddModeSetListToFmtp(
        IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeSetListToFmtp() mode-set list=%d, visible mode-set=%d",
            pAmrFmtp->GetModeSetList(), pAmrFmtp->IsModeSetVisible(), 0);

    if (pAmrFmtp->GetModeSetList() != 0 && pAmrFmtp->IsModeSetVisible() == IMS_TRUE)
    {
        AString strMode, strTemp;
        IMS_UINT32 nModeSet;

        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        for (nModeSet = 0; nModeSet <= MODESET_MAX_AMRWB; nModeSet++)
        {
            if ((pAmrFmtp->GetModeSetList()) & (1 << nModeSet))
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

PUBLIC void AudioNegoAmr::AddOctetAlignToFmtp(
        IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddOctetAlignToFmtp() octet-align=%d, visible=%d", pAmrFmtp->GetOctetAlign(),
            pAmrFmtp->IsOctetAlignVisible(), 0);

    if (pAmrFmtp->IsOctetAlignVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("octet-align=%d", pAmrFmtp->GetOctetAlign());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddModeChangeCapabilityToFmtp(
        IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeChangeCapabilityToFmtp() mode-change-capability=%d, visible=%d",
            pAmrFmtp->GetModeChangeCapability(), pAmrFmtp->IsModeChangeCapabilityVisible(), 0);

    if (pAmrFmtp->IsModeChangeCapabilityVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-capability=%d", pAmrFmtp->GetModeChangeCapability());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddModeChangePeriodToFmtp(
        IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeChangePeriodToFmtp() mode-change-period=%d, visible=%d",
            pAmrFmtp->GetModeChangePeriod(), pAmrFmtp->IsModeChangePeriodVisible(), 0);

    if (pAmrFmtp->IsModeChangePeriodVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-period=%d", pAmrFmtp->GetModeChangePeriod());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddModeChangeNeighborToFmtp(
        IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddModeChangeNeighborToFmtp() mode-change-neighbor=%d, visible=%d",
            pAmrFmtp->GetModeChangeNeighbor(), pAmrFmtp->IsModeChangeNeighborVisible(), 0);

    if (pAmrFmtp->IsModeChangeNeighborVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-neighbor=%d", pAmrFmtp->GetModeChangeNeighbor());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddMaxRedToFmtp(IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddMaxRedToFmtp() max-red=%d, visible=%d", pAmrFmtp->GetMaxRed(),
            pAmrFmtp->IsMaxRedVisible(), 0);

    if (pAmrFmtp->IsMaxRedVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("max-red=%d", pAmrFmtp->GetMaxRed());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddPtimeToFmtp(IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddPtimeToFmtp() ptime=%d, visible=%d", pAmrFmtp->GetPtime(),
            pAmrFmtp->IsPtimeVisible(), 0);

    if (pAmrFmtp->GetPtime() != AudioProfile::AmrFmtp::DEFAULT_PTIME &&
            pAmrFmtp->IsPtimeVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("ptime=%d", pAmrFmtp->GetPtime());
        strFmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddMaxPtimeToFmtp(
        IN AudioProfile::AmrFmtp* pAmrFmtp, OUT AString& strFmtp)
{
    if (pAmrFmtp == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_I("AddMaxPtimeToFmtp() maxptime=%d, visible=%d", pAmrFmtp->GetMaxPtime(),
            pAmrFmtp->IsMaxPtimeVisible(), 0);

    if (pAmrFmtp->GetMaxPtime() != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME &&
            pAmrFmtp->IsMaxPtimeVisible() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("maxptime=%d", pAmrFmtp->GetMaxPtime());
        strFmtp.Append(strTemp);
    }
}
