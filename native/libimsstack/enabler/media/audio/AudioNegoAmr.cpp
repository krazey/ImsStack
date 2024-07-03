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

PUBLIC AString AudioNegoAmr::SetSdpFmtpFromAmrFmtp(IN AudioProfile::AmrFmtp* amrFmtp)
{
    IMS_TRACE_I("SetSdpFmtpFromAmrFmtp()", 0, 0, 0);

    AString strFmtp = AString::ConstNull();

    if (amrFmtp == IMS_NULL)
    {
        return strFmtp;
    }

    AddModeSetListToFmtp(amrFmtp, strFmtp);
    AddOctetAlignToFmtp(amrFmtp, strFmtp);
    AddModeChangeCapabilityToFmtp(amrFmtp, strFmtp);
    AddModeChangePeriodToFmtp(amrFmtp, strFmtp);
    AddModeChangeNeighborToFmtp(amrFmtp, strFmtp);
    AddMaxRedToFmtp(amrFmtp, strFmtp);
    AddPtimeToFmtp(amrFmtp, strFmtp);
    AddMaxPtimeToFmtp(amrFmtp, strFmtp);

    return strFmtp;
}

PUBLIC void AudioNegoAmr::AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator)
{
    if (str.GetLength() > 0)
    {
        str.Append(separator);
    }
}

PUBLIC void AudioNegoAmr::AddModeSetListToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeSetListToFmtp() mode-set list=%d, show mode-set=%d",
            profile->GetModeSetList(), profile->IsShowModeSetEnabled(), 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->GetModeSetList() != 0 && profile->IsShowModeSetEnabled() == IMS_TRUE)
    {
        AString strMode, strTemp;
        IMS_UINT32 nModeSet;

        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        for (nModeSet = 0; nModeSet <= MODESET_MAX_AMRWB; nModeSet++)
        {
            if ((profile->GetModeSetList()) & (1 << nModeSet))
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

PUBLIC void AudioNegoAmr::AddOctetAlignToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddOctetAlignToFmtp() octet-align=%d, show=%d", profile->nOctetAlign,
            profile->bShowOctetAlign, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShowOctetAlign == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("octet-align=%d", profile->nOctetAlign);
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddModeChangeCapabilityToFmtp(
        IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeChangeCapabilityToFmtp() mode-change-capability=%d, show=%d",
            profile->GetModeChangeCapability(), profile->IsShowModeChangeCapabilityEnabled(), 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->IsShowModeChangeCapabilityEnabled() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-capability=%d", profile->GetModeChangeCapability());
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddModeChangePeriodToFmtp(
        IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeChangePeriodToFmtp() mode-change-period=%d, show=%d",
            profile->GetModeChangePeriod(), profile->IsShowModeChangePeriodEnabled(), 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->IsShowModeChangePeriodEnabled() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-period=%d", profile->GetModeChangePeriod());
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddModeChangeNeighborToFmtp(
        IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddModeChangeNeighborToFmtp() mode-change-neighbor=%d, show=%d",
            profile->GetModeChangeNeighbor(), profile->IsShowModeChangeNeighborEnabled(), 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->IsShowModeChangeNeighborEnabled() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("mode-change-neighbor=%d", profile->GetModeChangeNeighbor());
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddMaxRedToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddMaxRedToFmtp() max-red=%d, show=%d", profile->GetMaxRed(),
            profile->IsShowMaxRedEnabled(), 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->IsShowMaxRedEnabled() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("max-red=%d", profile->GetMaxRed());
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddPtimeToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddPtimeToFmtp() ptime=%d, show=%d", profile->GetPtime(),
            profile->IsShowPtimeEnabled(), 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->GetPtime() != AudioProfile::AmrFmtp::DEFAULT_PTIME &&
            profile->IsShowPtimeEnabled() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("ptime=%d", profile->GetPtime());
        fmtp.Append(strTemp);
    }
}

PUBLIC void AudioNegoAmr::AddMaxPtimeToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddMaxPtimeToFmtp() maxptime=%d, show=%d", profile->GetMaxPtime(),
            profile->IsShowMaxPtimeEnabled(), 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->GetMaxPtime() != AudioProfile::AmrFmtp::DEFAULT_MAXPTIME &&
            profile->IsShowMaxPtimeEnabled() == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("maxptime=%d", profile->GetMaxPtime());
        fmtp.Append(strTemp);
    }
}
