/**
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

#include "MediaNegoUtil.h"

PUBLIC
MediaNegoUtil::MediaNegoUtil() {}

PUBLIC
MediaNegoUtil::~MediaNegoUtil() {}

PUBLIC
IMS_BOOL MediaNegoUtil::GetMediaNegoInfo(IN IMSMap<IMS_UINTP, MediaNego*>* pMediaNegoMap,
        IN AString strIpAddr, IN IMS_SINT32 nPort, OUT IMS_UINTP& nNegoId,
        OUT MEDIA_CONTENT_TYPE& eMediaType)
{
    if (pMediaNegoMap == IMS_NULL || pMediaNegoMap->IsEmpty())
    {
        return IMS_FALSE;
    }

    IMS_UINT32 nIndex = 0;

    MediaNego* pMediaNego = IMS_NULL;
    AudioNego* pAudioNego = IMS_NULL;
    VideoNego* pVideoNego = IMS_NULL;
    TextNego* pTextNego = IMS_NULL;

    for (nIndex = 0; nIndex < pMediaNegoMap->GetSize(); nIndex++)
    {
        pMediaNego = pMediaNegoMap->GetValueAt(nIndex);

        if (pMediaNego != IMS_NULL)
        {
            pAudioNego = pMediaNego->GetAudioNego();

            if (pAudioNego != IMS_NULL)
            {
                if (pAudioNego->GetNegotiatedRemoteAddress().ToString().Equals(strIpAddr) &&
                        pAudioNego->GetNegotiatedRemotePort() == nPort)
                {
                    nNegoId = pMediaNegoMap->GetKeyAt(nIndex);
                    eMediaType = MEDIA_TYPE_AUDIO;
                    return IMS_TRUE;
                }
            }

            pVideoNego = pMediaNego->GetVideoNego();

            if (pVideoNego != IMS_NULL)
            {
                if (pVideoNego->GetNegotiatedRemoteAddress().ToString().Equals(strIpAddr) &&
                        pVideoNego->GetNegotiatedRemotePort() == nPort)
                {
                    nNegoId = pMediaNegoMap->GetKeyAt(nIndex);
                    eMediaType = MEDIA_TYPE_VIDEO;
                    return IMS_TRUE;
                }
            }

            pTextNego = pMediaNego->GetTextNego();

            if (pTextNego != IMS_NULL)
            {
                if (pTextNego->GetNegotiatedRemoteAddress().ToString().Equals(strIpAddr) &&
                        pTextNego->GetNegotiatedRemotePort() == nPort)
                {
                    nNegoId = pMediaNegoMap->GetKeyAt(nIndex);
                    eMediaType = MEDIA_TYPE_TEXT;
                    return IMS_TRUE;
                }
            }
        }
    }

    return IMS_FALSE;
}
