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
#include "MediaBaseProfile.h"
#include "MediaProfileUtil.h"
#include "config/MediaConfiguration.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC GLOBAL MEDIA_CONTENT_TYPE MediaProfileUtil::GetMediaType(IN const AString payloadType)
{
    if (payloadType.EqualsIgnoreCase("AMR-WB") || payloadType.EqualsIgnoreCase("AMR") ||
            payloadType.EqualsIgnoreCase("EVS") || payloadType.EqualsIgnoreCase("PCMU") ||
            payloadType.EqualsIgnoreCase("PCMA") || payloadType.EqualsIgnoreCase("telephone-event"))
    {
        return MEDIA_TYPE_AUDIO;
    }
    else if (payloadType.EqualsIgnoreCase("T140") || payloadType.EqualsIgnoreCase("RED"))
    {
        return MEDIA_TYPE_TEXT;
    }
    else if (payloadType.EqualsIgnoreCase("H264") || payloadType.EqualsIgnoreCase("H265"))
    {
        return MEDIA_TYPE_VIDEO;
    }
    else
    {
        return MEDIA_TYPE_INVALID;
    }
}

PUBLIC GLOBAL IMS_BOOL MediaProfileUtil::IsAudioType(IN const AString payloadType)
{
    return (GetMediaType(payloadType) == MEDIA_TYPE_AUDIO) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL MediaProfileUtil::IsTextType(IN const AString payloadType)
{
    return (GetMediaType(payloadType) == MEDIA_TYPE_TEXT) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC GLOBAL IMS_BOOL MediaProfileUtil::IsVideoType(IN const AString payloadType)
{
    return (GetMediaType(payloadType) == MEDIA_TYPE_VIDEO) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC GLOBAL void MediaProfileUtil::SetRtcpRsRr(
        OUT MediaBaseProfile* pProfile, IN MediaConfiguration* pConfig)
{
    if (pProfile != IMS_NULL && pConfig != IMS_NULL)
    {
        pProfile->nBandwidthRr = pConfig->GetRrBandwidthBps();
        pProfile->nBandwidthRs = pConfig->GetRsBandwidthBps();

        IMS_TRACE_D(
                "SetRtcpRsRr(), RS[%d], RR[%d]", pProfile->nBandwidthRs, pProfile->nBandwidthRr, 0);
    }
}
