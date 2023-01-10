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

#ifndef MEDIA_NEGO_UTIL_H_
#define MEDIA_NEGO_UTIL_H_

#include "MediaDef.h"

class MediaNego;

class MediaNegoUtil
{
public:
    MediaNegoUtil();
    ~MediaNegoUtil();

    /**
     * @brief Get the nego id and media type from remote address and port
     *
     * @param pMediaNegoMap MediaNego object container
     * @param strIpAddr remote address
     * @param nPort remote port
     * @param nNegoId identifier of SDP Negotiation Profile(dialog)
     * @param eMediaType media type
     * @return IMS_BOOL IMS_TRUE when there is a valid object existed with respects of remote
     * address and port
     */
    static IMS_BOOL GetMediaNegoInfo(IN IMSMap<IMS_UINTP, MediaNego*>* pMediaNegoMap,
            IN const AString& strIpAddr, IN IMS_SINT32 nPort, OUT IMS_UINTP& nNegoId,
            OUT MEDIA_CONTENT_TYPE& eMediaType);
};

#endif
