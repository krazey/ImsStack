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
#include "video/VideoNegoAvc.h"

const AString SEMICOLON = ";";
const AString COMMA = ",";

__IMS_TRACE_TAG_MEDIA__;

PUBLIC AString VideoNegoAvc::SetSdpFmtpFromAvcFmtp(IN VideoProfile::AvcFmtp* avcFmtp)
{
    IMS_TRACE_I("SetSdpFmtpFromAvcFmtp()", 0, 0, 0);

    AString strFmtp = AString::ConstNull();

    if (avcFmtp == IMS_NULL)
    {
        return strFmtp;
    }

    AddProfileLevelIdToFmtp(avcFmtp, strFmtp);
    AddPacketizationModeToFmtp(avcFmtp, strFmtp);
    AddSpropParameterSetsToFmtp(avcFmtp, strFmtp);

    return strFmtp;
}

PUBLIC void VideoNegoAvc::AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator)
{
    if (str.GetLength() > 0)
    {
        str.Append(separator);
    }
}

PUBLIC void VideoNegoAvc::AddProfileLevelIdToFmtp(
        IN VideoProfile::AvcFmtp* avcFmtp, OUT AString& fmtp)
{
    if (avcFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddProfileLevelIdToFmtp() profile-level-id=%s, show=%d",
                avcFmtp->GetProfileLevelId().GetStr(), avcFmtp->IsShowProfileLevelIdEnabled(), 0);

        if (avcFmtp->IsShowProfileLevelIdEnabled() == IMS_TRUE)
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("profile-level-id=%s", avcFmtp->GetProfileLevelId().GetStr());
            fmtp.Append(strTemp);
        }
    }
}

PUBLIC void VideoNegoAvc::AddPacketizationModeToFmtp(
        IN VideoProfile::AvcFmtp* avcFmtp, OUT AString& fmtp)
{
    if (avcFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddPacketizationModeToFmtp() packetization-mode=%d, show=%d",
                avcFmtp->GetPacketizationMode(), avcFmtp->IsShowPacketizationModeEnabled(), 0);

        if (avcFmtp->IsShowPacketizationModeEnabled() == IMS_TRUE)
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("packetization-mode=%d", avcFmtp->GetPacketizationMode());
            fmtp.Append(strTemp);
        }
    }
}

PUBLIC void VideoNegoAvc::AddSpropParameterSetsToFmtp(
        IN VideoProfile::AvcFmtp* avcFmtp, OUT AString& fmtp)
{
    if (avcFmtp != IMS_NULL)
    {
        IMS_TRACE_I("AddProfileLevelIdToFmtp() sprop-parameter-sets=%s, show=%d",
                avcFmtp->GetSpropParam().GetStr(), avcFmtp->IsShowSpropParamEnabled(), 0);

        if (avcFmtp->IsShowSpropParamEnabled() == IMS_TRUE)
        {
            AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

            AString strTemp;
            strTemp.Sprintf("sprop-parameter-sets=%s", avcFmtp->GetSpropParam().GetStr());
            fmtp.Append(strTemp);
        }
    }
}
