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
#ifndef _IM_CONSTANTS_H_
#define _IM_CONSTANTS_H_

#include "IMSTypeDef.h"

class IMConstants
{
public:
    static const IMS_CHAR TAG_OMA_IM[];
    static const IMS_CHAR TAG_FILETRANSFER[];
    static const IMS_CHAR TAG_HTTP_FILETRANSFER[];
    static const IMS_CHAR TAG_GEOLOCATIONPUSH[];

    static const IMS_CHAR TAG_CPM_SESSION[];
    static const IMS_CHAR TAG_CPM_MSG[];
    static const IMS_CHAR TAG_CPM_DEFERRED[];
    static const IMS_CHAR TAG_CPM_LARGE_MSG[];
    static const IMS_CHAR TAG_CPM_FILE_TRANSFER[];
    static const IMS_CHAR TAG_CPM_SYSTEM_MSG[];

    static const IMS_CHAR TAG_CPM_CFS[];
    static const IMS_CHAR TAG_CPM_NFS[];
};

#endif  // _IM_CONSTANTS_H_
