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

#ifndef _UCE_DEF_H_
#define _UCE_DEF_H_

#include "ImsTypeDef.h"
#include "ImsMessageDef.h"

#define MSG_THREAD_BASE                 0   // for thread operation
#define MSG_THREAD_RECEIVED_NOTIFY_RLMI 50  // received notify msg include rlmi xml body.
#define MSG_THREAD_RECEIVED_NOTIFY_PIDF 51  // received notify msg include pidf xml body.
#define MSG_THREAD_PARSERED_XML_RLMI    52  // rlmi xml parser is done
#define MSG_THREAD_PARSERED_XML_PIDF    53  // pidf xml parser is done
#define MSG_THREAD_NOTIFY_COMPLETED     54

class UceNamespace
{
public:
    UceNamespace();
    static const IMS_CHAR UCE_APP_NAME_PREFIX[];
    static const IMS_CHAR PRESENCE[];

    // uce service namespace
    static const IMS_CHAR PUBMNGR_NAME[];
    static const IMS_CHAR SUBMNGR_NAME[];
    static const IMS_CHAR OPTMNGR_NAME[];
};

class UceTag
{
public:
    UceTag();

    static const IMS_CHAR TAG_IARI[];
    static const IMS_CHAR TAG_ICSI[];
    static const IMS_CHAR TAG_STANDALONE_PAGER_MESSAGING[];
    static const IMS_CHAR TAG_STANDALONE_LARGE_MESSAGING[];
    static const IMS_CHAR TAG_CHAT[];
    static const IMS_CHAR TAG_IM[];
    static const IMS_CHAR TAG_FULL_STORE_AND_FORWARD_GROUP_CHAT[];
    static const IMS_CHAR TAG_FILE_TRANSFER[];
    static const IMS_CHAR TAG_FILE_TRANSFER_THUMBNAIL[];
    static const IMS_CHAR TAG_FILE_TRANSFER_STORE_AND_FORWARD[];
    static const IMS_CHAR TAG_FILE_TRANSFER_HTTP[];
    static const IMS_CHAR TAG_GEOLOCATION_PUSH[];
    static const IMS_CHAR TAG_FT_SMS[];
    static const IMS_CHAR TAG_GEOLOCATIONPUSH_SMS[];
    static const IMS_CHAR TAG_PRESENCE[];
    static const IMS_CHAR TAG_IP_VOICE_CALL[];
    static const IMS_CHAR TAG_IP_VIDEO_CALL[];
    static const IMS_CHAR TAG_SHARED_MAP[];
    static const IMS_CHAR TAG_SHARED_SKETCH[];
    static const IMS_CHAR TAG_CALL_COMPOSER[];
    static const IMS_CHAR TAG_POST_CALL[];
    static const IMS_CHAR TAG_CPM_SYSTEM_MSG[];
    static const IMS_CHAR TAG_CHATBOT_SESSION[];
    static const IMS_CHAR TAG_CHATBOT_STANDALONE_MESSAGE[];
    static const IMS_CHAR TAG_CHATBOT_VERSION_V1[];
    static const IMS_CHAR TAG_CHATBOT_VERSION_V2[];
};

enum INTERNALMSG
{
    TIMER_EXPIRED = IMS_MSG_UCE + 0,  // 13600

    AOS_CONNECTED_IND,
    AOS_DISCONNECTING_IND,
    AOS_DISCONNECTED_IND,
    OPTIONS_QUERY_RECEIVED_IND,
    RAT_CHANGED_IND,
    AOS_HO_WIFI_TO_LTE_PREPARE_IND,
    AOS_HO_WIFI_TO_LTE_COMPLETED_IND,
    UCE_INTERNAL_MAX,
};

enum
{
    CONNECTED_SERVICE_VIDEO = (0x00000002),

    CONNECTED_SERVICE_CPM_MSG = (0x00000020),
    CONNECTED_SERVICE_CPM_LARGEMSG = (0x00000040),

    CONNECTED_SERVICE_CPM_SESSION = (0x00000100),
    CONNECTED_SERVICE_HTTPFT = (0x00000200),
    CONNECTED_SERVICE_FTSMS = (0x00000400),
    CONNECTED_SERVICE_CALL_COMPOSER = (0x00000800),

    CONNECTED_SERVICE_GEOPUSH = (0x00010000),
    CONNECTED_SERVICE_GEOSMS = (0x00020000),
    CONNECTED_SERVICE_CHATBOT = (0x00040000),
    CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG = (0x00080000),
    CONNECTED_SERVICE_CHATBOT_V1 = (0x00100000),
    CONNECTED_SERVICE_CHATBOT_V2 = (0x00200000),

    CONNECTED_SERVICE_PRESENCE = (0x00800000),
};
#endif  // _UCE_DEF_H_
