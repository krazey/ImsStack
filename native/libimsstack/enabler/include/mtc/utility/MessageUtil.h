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

#ifndef MESSAGE_UTIL_H_
#define MESSAGE_UTIL_H_

#include "ImsTypeDef.h"

class MessageUtil
{
public:
    static const IMS_CHAR STR_199[];
    static const IMS_CHAR STR_ALERT_URN_CALL_WAITING[];
    static const IMS_CHAR STR_ANONYMOUS[];
    static const IMS_CHAR STR_UNAVAILABLE[];
    static const IMS_CHAR STR_CONTENT_DISPOSITION_RECIPIENT_LIST[];
    static const IMS_CHAR STR_CONTENT_ID[];
    static const IMS_CHAR STR_CONTENT_TYPE_3GPP_IMS_XML[];
    static const IMS_CHAR STR_CONTENT_TYPE_APPLICATION[];
    static const IMS_CHAR STR_CONTENT_TYPE_RESOURCE_LISTS_XML[];
    static const IMS_CHAR STR_CONTENT_TYPE_SIP_FRAG[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_SDP[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_3GPP_CURRENT_LOCATION_DISCOVERY_XML[];
    static const IMS_CHAR STR_ACCEPT_TYPE_APPLICATION_3GPP_IMS_XML[];
    static const IMS_CHAR STR_HEADER[];
    static const IMS_CHAR STR_HISTINFO[];
    static const IMS_CHAR STR_ICSI[];
    static const IMS_CHAR STR_ID[];
    static const IMS_CHAR STR_NONE[];
    static const IMS_CHAR STR_PACKAGE_CURRENT_LOCATION_DISCOVERY[];
    static const IMS_CHAR STR_PARAMETER_IS_FOCUS[];
    static const IMS_CHAR STR_PRECONDITION[];
    static const IMS_CHAR STR_REASON_FAILURE_TO_TRANSITION[];
    static const IMS_CHAR STR_REASON_HANDOVER_CANCELLED[];
    static const IMS_CHAR STR_RELEASE_CAUSE_1[];
    static const IMS_CHAR STR_RELEASE_CAUSE_2[];
    static const IMS_CHAR STR_RELEASE_CAUSE_3[];
    static const IMS_CHAR STR_RELEASE_CAUSE_4[];
    static const IMS_CHAR STR_RELEASE_CAUSE_5[];
    static const IMS_CHAR STR_RELEASE_CAUSE_6[];
    static const IMS_CHAR STR_REPLACES[];
    static const IMS_CHAR STR_SERVICE[];
    static const IMS_CHAR STR_SOS_AMBULANCE[];
    static const IMS_CHAR STR_SOS_ANIMAL_CONTROL[];
    static const IMS_CHAR STR_SOS_COUNTRY_SPECIFIC[];
    static const IMS_CHAR STR_SOS_FIRE[];
    static const IMS_CHAR STR_SOS_GAS[];
    static const IMS_CHAR STR_SOS_MARINE[];
    static const IMS_CHAR STR_SOS_MOUNTAIN[];
    static const IMS_CHAR STR_SOS_PHYSICIAN[];
    static const IMS_CHAR STR_SOS_POISON[];
    static const IMS_CHAR STR_SOS_POLICE[];
    static const IMS_CHAR STR_SOS[];
    static const IMS_CHAR STR_SRVCC_FEATURE_A[];
    static const IMS_CHAR STR_SRVCC_FEATURE_B[];
    static const IMS_CHAR STR_SRVCC_FEATURE_M[];
    static const IMS_CHAR STR_SUPPORTED[];
    static const IMS_CHAR STR_TIMER[];
    static const IMS_CHAR STR_URN[];
    static const IMS_CHAR STR_VIDEO[];
    static const IMS_CHAR STR_TEXT[];

    // carrier specific
    static const IMS_CHAR STR_P_SKT_BYE_CAUSE[];
    static const IMS_CHAR STR_P_TTA_VOLTE_INFO[];
    static const IMS_CHAR STR_AVCHANGE[];
    static const IMS_CHAR STR_REASON_USER_SESSIONEXPIRED[];
    static const IMS_CHAR STR_P_COM_ENABLETRANSCODING[];
};

#endif
