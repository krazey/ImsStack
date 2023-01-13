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
#ifndef IM_CONSTANTS_H_
#define IM_CONSTANTS_H_

#include "IMSTypeDef.h"

class IMConstants
{
public:
    static const IMS_CHAR TAG_OMA_IM[];
    static const IMS_CHAR TAG_CHAT_IM[];
    static const IMS_CHAR TAG_FILETRANSFER[];
    static const IMS_CHAR TAG_HTTP_FILETRANSFER[];
    static const IMS_CHAR TAG_SMS_FILETRANSFER[];
    static const IMS_CHAR TAG_FILE_TRANSFER[];
    static const IMS_CHAR TAG_GEOLOCATIONPUSH[];
    static const IMS_CHAR TAG_SMS_GEOLOCATIONPUSH[];
    static const IMS_CHAR TAG_SESSION[];
    static const IMS_CHAR TAG_STANDALONE_PAGER[];
    static const IMS_CHAR TAG_STANDALONE_DEFERRED[];
    static const IMS_CHAR TAG_STANDALONE_LARGE[];
    static const IMS_CHAR TAG_STANDALONE_PAGER_LARGE[];
    static const IMS_CHAR TAG_SYSTEM_MSG[];
    static const IMS_CHAR TAG_CPM_CFS[];
    static const IMS_CHAR TAG_CPM_NFS[];
    static const IMS_CHAR STR_UP_URN_REVOKE[];
    static const IMS_CHAR TAG_CALL_COMPOSER_ENRICHED_CALLING[];
    static const IMS_CHAR TAG_CALL_COMPOSER_VIA_TELEPHONY[];
    static const IMS_CHAR TAG_POST_CALL[];
    static const IMS_CHAR TAG_SHARED_MAP[];
    static const IMS_CHAR TAG_SHARED_SKETCH[];
    static const IMS_CHAR TAG_CHATBOT_COMMUNICATION_USING_SESSION[];
    static const IMS_CHAR TAG_CHATBOT_COMMUNICATION_USING_STANDALONE_MSG[];
    static const IMS_CHAR TAG_CHATBOT_VERSION_SUPPORTED[];
    static const IMS_CHAR TAG_CHATBOT_VERSION_V2_SUPPORTED[];
    static const IMS_CHAR TAG_CHATBOT_ROLE[];
};
#endif  // IM_CONSTANTS_H_
