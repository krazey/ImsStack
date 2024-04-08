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
#include "video/VideoNegoHevc.h"

const AString SEMICOLON = ";";
const AString COMMA = ",";

__IMS_TRACE_TAG_USER_DECL__("MED.VNH");

PUBLIC AString VideoNegoHevc::SetSdpFmtpFromHevcFmtp(IN VideoProfile::HevcFmtp* hevcFmtp)
{
    IMS_TRACE_I("SetSdpFmtpFromHevcFmtp()", 0, 0, 0);

    AString strFmtp = AString::ConstNull();

    if (hevcFmtp == IMS_NULL)
    {
        return strFmtp;
    }

    AddProfileIdToFmtp(hevcFmtp, strFmtp);
    AddLevelIdToFmtp(hevcFmtp, strFmtp);
    AddSpropParamsToFmtp(hevcFmtp, strFmtp);

    return strFmtp;
}

PUBLIC void VideoNegoHevc::AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator)
{
    if (str.GetLength() > 0)
    {
        str.Append(separator);
    }
}

PUBLIC void VideoNegoHevc::AddProfileIdToFmtp(IN VideoProfile::HevcFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddProfileIdToFmtp() profile-id=%d, show=%d", profile->nProfile,
            profile->bShow_Profile, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShow_Profile == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("profile-id=%d", profile->nProfile);
        fmtp.Append(strTemp);
    }
}

PUBLIC void VideoNegoHevc::AddLevelIdToFmtp(IN VideoProfile::HevcFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I(
            "AddLevelIdToFmtp() level-id=%d, show=%d", profile->nLevel, profile->bShow_Level, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShow_Level == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("level-id=%d", profile->nLevel);
        fmtp.Append(strTemp);
    }
}

PUBLIC void VideoNegoHevc::AddPacketizationModeToFmtp(
        IN VideoProfile::HevcFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddPacketizationModeToFmtp() packetization-mode=%d, show=%d",
            profile->nPacketizationMode, profile->bShow_PacketizationMode, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShow_PacketizationMode == IMS_TRUE)
    {
        AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

        AString strTemp;
        strTemp.Sprintf("packetization-mode=%d", profile->nPacketizationMode);
        fmtp.Append(strTemp);
    }
}

PUBLIC void VideoNegoHevc::AddSpropParamsToFmtp(
        IN VideoProfile::HevcFmtp* profile, OUT AString& fmtp)
{
    IMS_TRACE_I("AddSpropParamsToFmtp() sprop parameter=%s, show=%d",
            profile->strSpropParam.GetStr(), profile->bShow_SpropParam, 0);

    if (profile == IMS_NULL)
    {
        return;
    }

    if (profile->bShow_SpropParam == IMS_TRUE)
    {
        ImsList<AString> objSplitComma = profile->strSpropParam.Split(',');

        if (objSplitComma.GetSize() == 3)
        {
            profile->strVps = objSplitComma.GetAt(0);
            profile->strSps = objSplitComma.GetAt(1);
            profile->strPps = objSplitComma.GetAt(2);

            if (profile->strVps.GetLength() > 0 || profile->strSps.GetLength() > 0 ||
                    profile->strPps.GetLength() > 0)
            {
                AString strTemp;

                AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

                strTemp.Sprintf("sprop-vps=%s", profile->strVps.GetStr());
                fmtp.Append(strTemp);

                AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

                strTemp.Sprintf("sprop-sps=%s", profile->strSps.GetStr());
                fmtp.Append(strTemp);

                AppendSeparatorIfNotEmpty(fmtp, SEMICOLON);

                strTemp.Sprintf("sprop-pps=%s", profile->strPps.GetStr());
                fmtp.Append(strTemp);
            }
        }
    }
}
