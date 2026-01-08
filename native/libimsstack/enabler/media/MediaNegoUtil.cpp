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

#include "SdpMedia.h"
#include "MediaManager.h"
#include "MediaNegoUtil.h"
#include "MediaResourceManager.h"

PUBLIC
MediaNegoUtil::MediaNegoUtil() {}

PUBLIC
MediaNegoUtil::~MediaNegoUtil() {}

PUBLIC void MediaNegoUtil::ReleaseRtpPort(IN IMS_SINT32 slotId, IN IMS_UINT32 port)
{
    if (port != 0)
    {
        MediaManager* pMediaManager = MediaManager::GetInstance(slotId);

        if (pMediaManager != IMS_NULL)
        {
            MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

            if (pResourceMngr != IMS_NULL)
            {
                pResourceMngr->ReleaseRtpPort(port);
            }
        }
    }
}

PUBLIC IMS_UINT32 MediaNegoUtil::AcquireRtpPort(IN IMS_SINT32 slotId, IN IMS_UINT32 port)
{
    if (port != 0)
    {
        MediaManager* pMediaManager = MediaManager::GetInstance(slotId);

        if (pMediaManager != IMS_NULL)
        {
            MediaResourceManager* pResourceMngr = pMediaManager->GetResourceManager();

            if (pResourceMngr != IMS_NULL)
            {
                return pResourceMngr->AcquireRtpPort(port, port);
            }
        }
    }

    return 0;
}

PUBLIC IMS_SINT32 MediaNegoUtil::ConvertMediaTypeToSdpMediaType(IN const MEDIA_CONTENT_TYPE eType)
{
    switch (eType)
    {
        case MEDIA_TYPE_AUDIO:
            return SdpMedia::TYPE_AUDIO;
        case MEDIA_TYPE_VIDEO:
            return SdpMedia::TYPE_VIDEO;
        case MEDIA_TYPE_TEXT:
            return SdpMedia::TYPE_TEXT;
        default:
            return SdpMedia::TYPE_INVALID;
    }
}
