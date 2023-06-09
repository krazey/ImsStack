/*
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

#include "MtcDef.h"
#include "media/MtcMediaUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC GLOBAL CallType MtcMediaUtil::GetCallTypeFromMediaTypes(IN IMS_UINT32 eMediaTypes)
{
    CallType eCallType = CallType::UNKNOWN;

    if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO) &&
            (eMediaTypes & MEDIATYPE_TEXT))
    {
        eCallType = CallType::VIDEO_RTT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO))
    {
        eCallType = CallType::VT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_TEXT))
    {
        eCallType = CallType::RTT;
    }
    else if (eMediaTypes & MEDIATYPE_AUDIO)
    {
        eCallType = CallType::VOIP;
    }

    return eCallType;
}

PUBLIC GLOBAL CallType MtcMediaUtil::GetCallTypeFromMediaContents(
        IN MEDIA_CONTENT_TYPE eMediaContents)
{
    CallType eCallType = CallType::UNKNOWN;

    switch (eMediaContents)
    {
        case MEDIA_TYPE_AUDIO:
            eCallType = CallType::VOIP;
            break;
        case MEDIA_TYPE_AUDIOVIDEO:
            eCallType = CallType::VT;
            break;
        case MEDIA_TYPE_AUDIOTEXT:
            eCallType = CallType::RTT;
            break;
        case MEDIA_TYPE_AUDIOVIDEOTEXT:
            eCallType = CallType::VIDEO_RTT;
            break;

        default:
            break;
    }

    return eCallType;
}

PUBLIC GLOBAL IMS_UINT32 MtcMediaUtil::GetMediaTypesFromCallType(IN CallType eCallType)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;

    switch (eCallType)
    {
        case CallType::VOIP:
            eMediaTypes = MEDIATYPE_AUDIO;
            break;
        case CallType::VT:
            eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_VIDEO;
            break;
        case CallType::RTT:
            eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_TEXT;
            break;
        case CallType::VIDEO_RTT:
            eMediaTypes = MEDIATYPE_AUDIO | MEDIATYPE_VIDEO | MEDIATYPE_TEXT;
            break;

        default:
            break;
    }

    return eMediaTypes;
}

PUBLIC GLOBAL IMS_UINT32 MtcMediaUtil::GetMediaTypesFromMediaContents(
        IN MEDIA_CONTENT_TYPE eMediaContents)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;

    if (eMediaContents & MEDIA_TYPE_AUDIO)
    {
        eMediaTypes |= MEDIATYPE_AUDIO;
    }

    if (eMediaContents & MEDIA_TYPE_VIDEO)
    {
        eMediaTypes |= MEDIATYPE_VIDEO;
    }

    if (eMediaContents & MEDIA_TYPE_TEXT)
    {
        eMediaTypes |= MEDIATYPE_TEXT;
    }

    return eMediaTypes;
}

PUBLIC GLOBAL MEDIA_CONTENT_TYPE MtcMediaUtil::GetMediaContentsFromMediaTypes(
        IN IMS_UINT32 eMediaTypes)
{
    MEDIA_CONTENT_TYPE eContents = MEDIA_TYPE_INVALID;

    if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO) &&
            (eMediaTypes & MEDIATYPE_TEXT))
    {
        eContents = MEDIA_TYPE_AUDIOVIDEOTEXT;
    }
    else if ((eMediaTypes & MEDIATYPE_VIDEO) && (eMediaTypes & MEDIATYPE_TEXT))
    {
        eContents = MEDIA_TYPE_VIDEOTEXT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_TEXT))
    {
        eContents = MEDIA_TYPE_AUDIOTEXT;
    }
    else if ((eMediaTypes & MEDIATYPE_AUDIO) && (eMediaTypes & MEDIATYPE_VIDEO))
    {
        eContents = MEDIA_TYPE_AUDIOVIDEO;
    }
    else if (eMediaTypes & MEDIATYPE_TEXT)
    {
        eContents = MEDIA_TYPE_TEXT;
    }
    else if (eMediaTypes & MEDIATYPE_VIDEO)
    {
        eContents = MEDIA_TYPE_VIDEO;
    }
    else if (eMediaTypes & MEDIATYPE_AUDIO)
    {
        eContents = MEDIA_TYPE_AUDIO;
    }

    return eContents;
}

PUBLIC GLOBAL MEDIA_CONTENT_TYPE MtcMediaUtil::GetMediaContentsFromCallType(IN CallType eCallType)
{
    MEDIA_CONTENT_TYPE eMediaContentTypes = MEDIA_TYPE_INVALID;

    switch (eCallType)
    {
        case CallType::VOIP:
            eMediaContentTypes = MEDIA_TYPE_AUDIO;
            break;
        case CallType::VT:
            eMediaContentTypes = MEDIA_TYPE_AUDIOVIDEO;
            break;
        case CallType::RTT:
            eMediaContentTypes = MEDIA_TYPE_AUDIOTEXT;
            break;
        case CallType::VIDEO_RTT:
            eMediaContentTypes = MEDIA_TYPE_AUDIOVIDEOTEXT;
            break;

        default:
            break;
    }

    return eMediaContentTypes;
}

PUBLIC GLOBAL MEDIA_SERVICE_TYPE MtcMediaUtil::GetMediaServiceType(IN ServiceType eServiceType)
{
    if (eServiceType == ServiceType::EMERGENCY)
    {
        return MEDIA_SERVICE_EMERGENCY;
    }

    return MEDIA_SERVICE_DEFAULT;
}

PUBLIC GLOBAL MEDIA_NETWORK_TYPE MtcMediaUtil::GetMediaNetworkType(IN IMtcService* piMtcService,
        IN IMS_SINT32 eRadioType /* = INetworkWatcher::RADIOTECH_TYPE_INVALID */)
{
    if (piMtcService->IsWlanIpCanType())
    {
        return MEDIA_NETWORK_WIFI;
    }

    MEDIA_NETWORK_TYPE eMediaNetworkType = MEDIA_NETWORK_NONE;
    switch (eRadioType)
    {
        case INetworkWatcher::RADIOTECH_TYPE_LTE:
            eMediaNetworkType = MEDIA_NETWORK_LTE;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_HSPAP:
            eMediaNetworkType = MEDIA_NETWORK_HSPA_PLUS;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_UMTS:
        case INetworkWatcher::RADIOTECH_TYPE_HSPA:
        case INetworkWatcher::RADIOTECH_TYPE_HSDPA:
        case INetworkWatcher::RADIOTECH_TYPE_HSUPA:
        case INetworkWatcher::RADIOTECH_TYPE_CDMA:
            eMediaNetworkType = MEDIA_NETWORK_HSPA;
            break;
        case INetworkWatcher::RADIOTECH_TYPE_EHRPD:
            eMediaNetworkType = MEDIA_NETWORK_EHRPD;
            break;
        default:
            eMediaNetworkType = MEDIA_NETWORK_LTE;
            break;
    }

    return eMediaNetworkType;
}

PUBLIC GLOBAL IMS_SINT32 MtcMediaUtil::GetGttModeFromTextQuality(IN IMS_UINT32 eTextQuality)
{
    IMS_SINT32 eGttMode = GTT_MODE_INVALID;

    switch (eTextQuality)  // This Text Quality is matched with the values defined in TextDef.h.
    {
        case TEXT_QUALITY_NONE:
            eGttMode = GTT_MODE_INVALID;
            break;
        case TEXT_QUALITY_T140:
        case TEXT_QUALITY_T140_RED:
            eGttMode = GTT_MODE_FULL;
            break;
        case TEXT_QUALITY_NOTUSED:
            eGttMode = GTT_MODE_INVALID;
            break;
        default:
            break;
    }

    return eGttMode;
}

PUBLIC GLOBAL AString MtcMediaUtil::MediaTypesToString(IN IMS_UINT32 eMediaTypes)
{
    AString strMediaTypes;
    if (eMediaTypes & MEDIATYPE_AUDIO)
    {
        strMediaTypes.Append("audio");
    }

    if (eMediaTypes & MEDIATYPE_VIDEO)
    {
        strMediaTypes.Append("video");
    }

    if (eMediaTypes & MEDIATYPE_TEXT)
    {
        strMediaTypes.Append("text");
    }

    return strMediaTypes;
}

PUBLIC GLOBAL IMS_UINT32 MtcMediaUtil::StringToMediaTypes(IN const AString& strMediaTypes)
{
    IMS_UINT32 eMediaTypes = MEDIATYPE_NONE;

    if (strMediaTypes.Contains("audio"))
    {
        eMediaTypes |= MEDIATYPE_AUDIO;
    }

    if (strMediaTypes.Contains("video"))
    {
        eMediaTypes |= MEDIATYPE_VIDEO;
    }

    if (strMediaTypes.Contains("text"))
    {
        eMediaTypes |= MEDIATYPE_TEXT;
    }

    return eMediaTypes;
}
