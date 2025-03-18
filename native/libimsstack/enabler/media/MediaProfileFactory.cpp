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
#include "MediaProfileFactory.h"
#include "MediaProfileUtil.h"
#include "audio/AudioProfile.h"

static const IMS_SINT32 NOT_PRESENT = -1;

static MediaProfileFactory* g_pMediaProfileFactory = IMS_NULL;

__IMS_TRACE_TAG_MEDIA__;

PRIVATE
MediaProfileFactory::MediaProfileFactory() {}

PUBLIC VIRTUAL MediaProfileFactory::~MediaProfileFactory() {}

PUBLIC
MediaBaseProfile* MediaProfileFactory::CreateProfile(
        IN MEDIA_CONTENT_TYPE eType, IN MediaBaseProfile* pProfile)
{
    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
        {
            return pProfile != IMS_NULL ? new AudioProfile(*static_cast<AudioProfile*>(pProfile))
                                        : new AudioProfile();
        }
        case MEDIA_TYPE_VIDEO:
        {
            return pProfile != IMS_NULL ? new VideoProfile(*static_cast<VideoProfile*>(pProfile))
                                        : new VideoProfile();
        }
        case MEDIA_TYPE_TEXT:
        {
            return pProfile != IMS_NULL ? new TextProfile(*static_cast<TextProfile*>(pProfile))
                                        : new TextProfile();
        }
        default:
            IMS_TRACE_I("CreateProfile(): invalid type[%d]", eType, 0, 0);
            break;
    }
    return IMS_NULL;
}

PUBLIC
void MediaProfileFactory::DeleteProfile(IN MediaBaseProfile* pProfile)
{
    delete pProfile;
    pProfile = IMS_NULL;
}

PUBLIC
MediaBaseProfile::BasePayload* MediaProfileFactory::CreatePayload(IN MEDIA_CONTENT_TYPE eType)
{
    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
            return CreateAudioPayload();
        case MEDIA_TYPE_TEXT:
            return CreateTextPayload();
        case MEDIA_TYPE_VIDEO:
            return CreateVideoPayload();
        default:
            IMS_TRACE_I("CreatePayload(): invalid type[%d]", eType, 0, 0);
            return IMS_NULL;
    }
}

PUBLIC
MediaBaseProfile::BasePayload* MediaProfileFactory::CreatePayload(
        IN MediaBaseProfile::BasePayload* payload)
{
    switch (MediaProfileUtil::GetMediaType(payload->GetRtpMap().GetPayloadType()))
    {
        case MEDIA_TYPE_AUDIO:
            return CreateAudioPayload(static_cast<AudioProfile::Payload*>(payload));
        case MEDIA_TYPE_TEXT:
            return CreateTextPayload(static_cast<TextProfile::Payload*>(payload));
        case MEDIA_TYPE_VIDEO:
            return CreateVideoPayload(static_cast<VideoProfile::Payload*>(payload));
        default:
            return IMS_NULL;
    }
}

PUBLIC
void MediaProfileFactory::DeletePayload(IN MediaBaseProfile::BasePayload* pPayload)
{
    delete pPayload;
    pPayload = IMS_NULL;
}

PUBLIC
MediaProfileFactory* MediaProfileFactory::GetInstance()
{
    if (g_pMediaProfileFactory == IMS_NULL)
    {
        g_pMediaProfileFactory = new MediaProfileFactory();
    }

    return g_pMediaProfileFactory;
}

PUBLIC
void MediaProfileFactory::ReleaseInstance(MediaProfileFactory* pMediaProfileFactory)
{
    if (pMediaProfileFactory != IMS_NULL && pMediaProfileFactory == g_pMediaProfileFactory)
    {
        delete pMediaProfileFactory;
        g_pMediaProfileFactory = IMS_NULL;
    }
}

PRIVATE AudioProfile::Payload* MediaProfileFactory::CreateAudioPayload(
        IN AudioProfile::Payload* payload)
{
    return (payload != IMS_NULL) ? new AudioProfile::Payload(*payload)
                                 : new AudioProfile::Payload();
}

PRIVATE TextProfile::Payload* MediaProfileFactory::CreateTextPayload(
        IN TextProfile::Payload* payload)
{
    return (payload != IMS_NULL) ? new TextProfile::Payload(*payload) : new TextProfile::Payload();
}

PRIVATE VideoProfile::Payload* MediaProfileFactory::CreateVideoPayload(
        IN VideoProfile::Payload* payload)
{
    return (payload != IMS_NULL) ? new VideoProfile::Payload(*payload)
                                 : new VideoProfile::Payload();
}
