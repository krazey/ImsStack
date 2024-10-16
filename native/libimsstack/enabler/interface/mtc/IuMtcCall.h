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

#ifndef INTERFACE_UI_MTC_CALL_H_
#define INTERFACE_UI_MTC_CALL_H_

#include "ImsMessageDef.h"
#include "ImsTypeDef.h"

class IuMtcCall
{
public:
    static const IMS_SINT32 EVENT_U2I = IMS_MSG_BASE_SESSION;
    static const IMS_SINT32 EVENT_I2U = IMS_MSG_BASE_SESSION + 100;

    static const IMS_SINT32 EVENT_CONF_U2I = EVENT_U2I + 30;
    static const IMS_SINT32 EVENT_CONF_I2U = EVENT_I2U + 30;

    static const IMS_SINT32 EVENT_ECT_U2I = EVENT_U2I + 60;
    static const IMS_SINT32 EVENT_ECT_I2U = EVENT_I2U + 60;

    static const IMS_SINT32 MAXIMUM = (EVENT_I2U + 99);

    // UI to IMS events
    static const IMS_SINT32 START = (EVENT_U2I + 1);
    static const IMS_SINT32 STARTCONF = (EVENT_U2I + 2);
    static const IMS_SINT32 USER_ALERT = (EVENT_U2I + 3);
    static const IMS_SINT32 ACCEPT = (EVENT_U2I + 4);
    static const IMS_SINT32 REJECT = (EVENT_U2I + 5);
    static const IMS_SINT32 HOLD = (EVENT_U2I + 6);
    static const IMS_SINT32 RESUME = (EVENT_U2I + 7);
    static const IMS_SINT32 TERMINATE = (EVENT_U2I + 8);
    static const IMS_SINT32 UPDATE = (EVENT_U2I + 9);
    static const IMS_SINT32 ACCEPT_UPDATE = (EVENT_U2I + 10);
    static const IMS_SINT32 REJECT_UPDATE = (EVENT_U2I + 11);
    static const IMS_SINT32 CANCEL_UPDATE = (EVENT_U2I + 12);
    static const IMS_SINT32 ACCEPT_RESUME = (EVENT_U2I + 13);
    static const IMS_SINT32 REJECT_RESUME = (EVENT_U2I + 14);

    static const IMS_SINT32 SEND_USSD = (EVENT_U2I + 15);

    static const IMS_SINT32 CONF_EXPAND = (EVENT_CONF_U2I + 1);
    static const IMS_SINT32 CONF_MERGE = (EVENT_CONF_U2I + 2);
    static const IMS_SINT32 CONF_JOIN = (EVENT_CONF_U2I + 3);
    static const IMS_SINT32 CONF_DROP = (EVENT_CONF_U2I + 4);
    static const IMS_SINT32 CONF_DELETE = (EVENT_CONF_U2I + 5);

    static const IMS_SINT32 ECT_START = (EVENT_ECT_U2I + 1);
    static const IMS_SINT32 PUSH_CALL = (EVENT_ECT_U2I + 2);
    static const IMS_SINT32 CANCEL_CALL_PUSH = (EVENT_ECT_U2I + 3);
    static const IMS_SINT32 ECT_START_BLIND = (EVENT_ECT_U2I + 4);

    static const IMS_SINT32 ATTACH = (EVENT_U2I + 98);
    static const IMS_SINT32 OPEN = (EVENT_U2I + 99);

    // IMS to UI events
    static const IMS_SINT32 STARTED = (EVENT_I2U + 1);
    static const IMS_SINT32 START_FAILED = (EVENT_I2U + 2);
    static const IMS_SINT32 PROGRESSING = (EVENT_I2U + 3);
    static const IMS_SINT32 HELD = (EVENT_I2U + 4);
    static const IMS_SINT32 HOLD_FAILED = (EVENT_I2U + 5);
    static const IMS_SINT32 HELD_BY = (EVENT_I2U + 6);
    static const IMS_SINT32 RESUMED = (EVENT_I2U + 7);
    static const IMS_SINT32 RESUME_FAILED = (EVENT_I2U + 8);
    static const IMS_SINT32 RESUMED_BY = (EVENT_I2U + 9);
    static const IMS_SINT32 TERMINATED = (EVENT_I2U + 10);
    static const IMS_SINT32 INCOMING_UPDATE = (EVENT_I2U + 11);
    static const IMS_SINT32 UPDATED = (EVENT_I2U + 12);
    static const IMS_SINT32 UPDATE_FAILED = (EVENT_I2U + 13);
    static const IMS_SINT32 UPDATED_BY = (EVENT_I2U + 14);
    static const IMS_SINT32 NOTIFY_INFO = (EVENT_I2U + 15);
    static const IMS_SINT32 INCOMING_RESUME = (EVENT_I2U + 16);
    static const IMS_SINT32 SET_PROPERTY = (EVENT_I2U + 17);
    static const IMS_SINT32 INCOMING_CALL_RECEIVED = (EVENT_I2U + 18);

    static const IMS_SINT32 CONF_EXPANDED = (EVENT_CONF_I2U + 1);
    static const IMS_SINT32 CONF_EXPANDFAILED = (EVENT_CONF_I2U + 2);
    static const IMS_SINT32 CONF_EXPANDED_BY = (EVENT_CONF_I2U + 3);
    static const IMS_SINT32 CONF_MERGED = (EVENT_CONF_I2U + 4);
    static const IMS_SINT32 CONF_MERGEFAILED = (EVENT_CONF_I2U + 5);
    static const IMS_SINT32 CONF_JOINED = (EVENT_CONF_I2U + 6);
    static const IMS_SINT32 CONF_DROPPED = (EVENT_CONF_I2U + 7);
    static const IMS_SINT32 CONF_DELETED = (EVENT_CONF_I2U + 8);
    static const IMS_SINT32 CONF_NOTIFY_USERS_INFO = (EVENT_CONF_I2U + 9);
    static const IMS_SINT32 CONF_NOTIFY_CONF_INFO = (EVENT_CONF_I2U + 10);

    static const IMS_SINT32 ECT_COMPLETED = (EVENT_ECT_I2U + 1);
    static const IMS_SINT32 REPLACED_BY = (EVENT_ECT_I2U + 2);
    static const IMS_SINT32 CALL_PUSH_COMPLETED = (EVENT_ECT_I2U + 3);
};

#endif
