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

#ifndef CONFERENCE_CONST_H_
#define CONFERENCE_CONST_H_

#include "IMSTypeDef.h"

class ConferenceConst
{
public:
    static const IMS_CHAR LEG_ID[];

    static const IMS_CHAR FACTORY_URI_FORMAT[];

    static const IMS_CHAR ANONYMOUS[];
    static const IMS_CHAR ANONYMOUS_URI[];

    static const IMS_CHAR STR_STATUS_CONNECTED[];
    static const IMS_CHAR STR_STATUS_DISCONNECTED[];
    static const IMS_CHAR STR_STATUS_ON_HOLD[];
    static const IMS_CHAR STR_STATUS_MUTED_VIA_FOCUS[];
    static const IMS_CHAR STR_STATUS_PENDING[];
    static const IMS_CHAR STR_STATUS_ALERTING[];
    static const IMS_CHAR STR_STATUS_DIALING_IN[];
    static const IMS_CHAR STR_STATUS_DIALING_OUT[];
    static const IMS_CHAR STR_STATUS_DISCONNECTING[];
    static const IMS_CHAR STR_STATUS_CONNECT_FAIL[];

    static const IMS_CHAR APPLICATION_CONFERENCEINFO[];
};

#endif
