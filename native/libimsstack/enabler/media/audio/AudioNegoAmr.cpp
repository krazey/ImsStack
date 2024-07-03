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

    IMS_TRACE_I("AddModeSetListToFmtp() mode-set list=%d, show mode-set=%d",
            pAmrFmtp->GetModeSetList(), pAmrFmtp->IsShowModeSetEnabled(), 0);

    if (pAmrFmtp->GetModeSetList() != 0 && pAmrFmtp->IsShowModeSetEnabled() == IMS_TRUE)
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

    IMS_TRACE_I("AddOctetAlignToFmtp() octet-align=%d, show=%d", pAmrFmtp->nOctetAlign,
            pAmrFmtp->bShowOctetAlign, 0);

    if (pAmrFmtp->bShowOctetAlign == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("octet-align=%d", pAmrFmtp->nOctetAlign);
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

    IMS_TRACE_I("AddModeChangeCapabilityToFmtp() mode-change-capability=%d, show=%d",
            pAmrFmtp->GetModeChangeCapability(), pAmrFmtp->IsShowModeChangeCapabilityEnabled(), 0);

    if (pAmrFmtp->IsShowModeChangeCapabilityEnabled() == IMS_TRUE)
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

    IMS_TRACE_I("AddModeChangePeriodToFmtp() mode-change-period=%d, show=%d",
            pAmrFmtp->GetModeChangePeriod(), pAmrFmtp->IsShowModeChangePeriodEnabled(), 0);

    if (pAmrFmtp->IsShowModeChangePeriodEnabled() == IMS_TRUE)
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

    IMS_TRACE_I("AddModeChangeNeighborToFmtp() mode-change-neighbor=%d, show=%d",
            pAmrFmtp->GetModeChangeNeighbor(), pAmrFmtp->IsShowModeChangeNeighborEnabled(), 0);

    if (pAmrFmtp->IsShowModeChangeNeighborEnabled() == IMS_TRUE)
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

    IMS_TRACE_I("AddMaxRedToFmtp() max-red=%d, show=%d", pAmrFmtp->GetMaxRed(),
            pAmrFmtp->IsShowMaxRedEnabled(), 0);

    if (pAmrFmtp->IsShowMaxRedEnabled() == IMS_TRUE)
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

    IMS_TRACE_I("AddPtimeToFmtp() ptime=%d, show=%d", pAmrFmtp->GetPtime(),
            pAmrFmtp->IsShowPtimeEnabled(), 0);

    if (pAmrFmtp->GetPtime() != AudioProfile::AmrFmtp::DEFAULT_PTIME &&
            pAmrFmtp->IsShowPtimeEnabled() == IMS_TRUE)
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

    IMS_TRACE_I("AddMaxPtimeToFmtp() maxptime=%d, show=%d", pAmrFmtp->GetMaxPtime(),
            pAmrFmtp->IsShowMaxPtimeEnabled(), 0);

    if (pAmrFmtp->GetMaxPtime() != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME &&
            pAmrFmtp->IsShowMaxPtimeEnabled() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(strFmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("maxptime=%d", pAmrFmtp->GetMaxPtime());
        strFmtp.Append(strTemp);
    }
}
