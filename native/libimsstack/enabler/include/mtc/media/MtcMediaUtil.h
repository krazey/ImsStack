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

#ifndef MTC_MEDIA_UTIL_H_
#define MTC_MEDIA_UTIL_H_

#include "IMtcService.h"
#include "INetworkWatcher.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "call/IMtcCall.h"
#include <vector>

class MtcMediaUtil
{
public:
    static CallType GetCallTypeFromMediaTypes(IN IMS_UINT32 eMediaTypes);
    static CallType GetCallTypeFromMediaContents(IN MEDIA_CONTENT_TYPE eMediaContents);

    static IMS_UINT32 GetMediaTypesFromCallType(IN CallType eCallType);
    static IMS_UINT32 GetMediaTypesFromMediaContents(IN MEDIA_CONTENT_TYPE eMediaContents);
    static std::vector<IMS_UINT32> GetMediaTypeListFromCallType(IN CallType eCallType);
    static std::vector<IMS_UINT32> GetUnusedMediaTypeListFromCallType(IN CallType eCallType);

    static MEDIA_CONTENT_TYPE GetMediaContentsFromMediaTypes(IN IMS_UINT32 eMediaTypes);
    static MEDIA_CONTENT_TYPE GetMediaContentsFromCallType(IN CallType eCallType);

    static MEDIA_SERVICE_TYPE GetMediaServiceType(IN ServiceType eServiceType);
    static MEDIA_NETWORK_TYPE GetMediaNetworkType(IN const IMtcService* piMtcService,
            IN IMS_SINT32 eRadioType = INetworkWatcher::RADIOTECH_TYPE_INVALID);
    static IMS_SINT32 GetGttModeFromTextQuality(IN IMS_UINT32 eTextQuality);

    static AString MediaTypesToString(IN IMS_UINT32 eMediaTypes);
    static IMS_UINT32 StringToMediaTypes(IN const AString& strMediaTypes);

    static void RefineMediaInfoByCallType(IN CallType eCallType, IN_OUT MediaInfo& objMediaInfo);
    static PemType GetPemType(IN const AString& strPemHeader);
};

#endif
