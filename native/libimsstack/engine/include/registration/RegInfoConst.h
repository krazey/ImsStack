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
#ifndef REG_INFO_CONST_H_
#define REG_INFO_CONST_H_

#include "ImsTypeDef.h"

class RegInfoConst
{
public:
    RegInfoConst() = delete;

public:
    // elements
    static const IMS_CHAR ELEMENT_CONTACT[];
    static const IMS_CHAR ELEMENT_REGINFO[];
    static const IMS_CHAR ELEMENT_REGISTRATION[];
    static const IMS_CHAR ELEMENT_URI[];
    static const IMS_CHAR ELEMENT_DISPLAY_NAME[];
    static const IMS_CHAR ELEMENT_PUB_GRUU[];
    static const IMS_CHAR ELEMENT_TEMP_GRUU[];
    static const IMS_CHAR ELEMENT_UNKNOWN_PARAM[];

    // attributes
    static const IMS_CHAR ATTR_AOR[];
    static const IMS_CHAR ATTR_CALLID[];
    static const IMS_CHAR ATTR_CSEQ[];
    static const IMS_CHAR ATTR_DURATION_REGISTERED[];
    static const IMS_CHAR ATTR_EVENT[];
    static const IMS_CHAR ATTR_EXPIRES[];
    static const IMS_CHAR ATTR_FIRST_CSEQ[];
    static const IMS_CHAR ATTR_ID[];
    static const IMS_CHAR ATTR_NAME[];
    static const IMS_CHAR ATTR_Q[];
    static const IMS_CHAR ATTR_RETRY_AFTER[];
    static const IMS_CHAR ATTR_STATE[];
    static const IMS_CHAR ATTR_URI[];
    static const IMS_CHAR ATTR_VERSION[];

    // values of "event" attribute
    static const IMS_CHAR ATTR_EVENT_REGISTERED[];
    static const IMS_CHAR ATTR_EVENT_CREATED[];
    static const IMS_CHAR ATTR_EVENT_REFRESHED[];
    static const IMS_CHAR ATTR_EVENT_SHORTENED[];
    static const IMS_CHAR ATTR_EVENT_EXPIRED[];
    static const IMS_CHAR ATTR_EVENT_DEACTIVATED[];
    static const IMS_CHAR ATTR_EVENT_PROBATION[];
    static const IMS_CHAR ATTR_EVENT_UNREGISTERED[];
    static const IMS_CHAR ATTR_EVENT_REJECTED[];

    // values of "state" attribute
    static const IMS_CHAR ATTR_STATE_FULL[];
    static const IMS_CHAR ATTR_STATE_PARTIAL[];

    static const IMS_CHAR ATTR_STATE_INIT[];
    static const IMS_CHAR ATTR_STATE_ACTIVE[];
    static const IMS_CHAR ATTR_STATE_TERMINATED[];
};

#endif
