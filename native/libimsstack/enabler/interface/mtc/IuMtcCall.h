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

#include "CallReasonInfo.h"
#include "ImsMessageDef.h"
#include "ImsMessage.h"
#include "MtcDef.h"
#include "ImsMap.h"
#include "call/IMtcCall.h"

class IuMtcService;
class IMtcService;
class MtcCall;

class IuMtcCall
{
public:
    static const IMS_SINT32 EVENT_U2I = IMS_MSG_BASE_SESSION;
    static const IMS_SINT32 EVENT_I2U = IMS_MSG_BASE_SESSION + 100;

    static const IMS_SINT32 EVENT_CONF_U2I = EVENT_U2I + 30;
    static const IMS_SINT32 EVENT_CONF_I2U = EVENT_I2U + 30;

    static const IMS_SINT32 EVENT_ECT_U2I = EVENT_U2I + 60;
    static const IMS_SINT32 EVENT_ECT_I2U = EVENT_I2U + 60;

    static const IMS_SINT32 EVENT_MEDIA_I2U = EVENT_I2U + 90;

    static const IMS_SINT32 MAXIMUM = (EVENT_I2U + 99);

    // UI to IMS events
    static const IMS_SINT32 START = (EVENT_U2I + 1);
    static const IMS_SINT32 STARTCONF = (EVENT_U2I + 2);
    static const IMS_SINT32 USER_ALERT = (EVENT_U2I + 3);
    static const IMS_SINT32 ACCEPT = (EVENT_U2I + 4);
    static const IMS_SINT32 REJECT = (EVENT_U2I + 5);
    static const IMS_SINT32 HOLD = (EVENT_U2I + 6);
    static const IMS_SINT32 RESUME = (EVENT_U2I + 7);
    static const IMS_SINT32 SEND_DTMF = (EVENT_U2I + 8);
    static const IMS_SINT32 TERMINATE = (EVENT_U2I + 9);
    static const IMS_SINT32 UPDATE = (EVENT_U2I + 10);
    static const IMS_SINT32 ACCEPT_UPDATE = (EVENT_U2I + 11);
    static const IMS_SINT32 REJECT_UPDATE = (EVENT_U2I + 12);
    static const IMS_SINT32 CANCEL_UPDATE = (EVENT_U2I + 13);
    static const IMS_SINT32 ACCEPT_RESUME = (EVENT_U2I + 14);
    static const IMS_SINT32 REJECT_RESUME = (EVENT_U2I + 15);

    static const IMS_SINT32 SEND_USSD = (EVENT_U2I + 16);

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

    static const IMS_SINT32 CODEC_INFO_UPDATED = (EVENT_MEDIA_I2U + 1);

    inline static IMS_BOOL IsMsg(IN IMS_SINT32 nMsg)
    {
        return ((nMsg > EVENT_U2I) && (nMsg < MAXIMUM));
    }
};

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* UCStrSessionEvtU2I(IN IMS_SINT32 eEvent)
{
    switch (eEvent)
    {
        case IuMtcCall::START:
            return "START";
        case IuMtcCall::STARTCONF:
            return "STARTCONF";
        case IuMtcCall::USER_ALERT:
            return "USER_ALERT";
        case IuMtcCall::ACCEPT:
            return "ACCEPT";
        case IuMtcCall::REJECT:
            return "REJECT";
        case IuMtcCall::HOLD:
            return "HOLD";
        case IuMtcCall::RESUME:
            return "RESUME";
        case IuMtcCall::SEND_DTMF:
            return "SEND_DTMF";
        case IuMtcCall::TERMINATE:
            return "TERMINATE";
        case IuMtcCall::UPDATE:
            return "UPDATE";
        case IuMtcCall::ACCEPT_UPDATE:
            return "ACCEPT_UPDATE";
        case IuMtcCall::REJECT_UPDATE:
            return "REJECT_UPDATE";
        case IuMtcCall::CANCEL_UPDATE:
            return "CANCEL_UPDATE";
        case IuMtcCall::ACCEPT_RESUME:
            return "ACCEPT_RESUME";
        case IuMtcCall::REJECT_RESUME:
            return "REJECT_RESUME";
        case IuMtcCall::SEND_USSD:
            return "SEND_USSD";
        case IuMtcCall::CONF_EXPAND:
            return "CONF_EXPAND";
        case IuMtcCall::CONF_MERGE:
            return "CONF_MERGE";
        case IuMtcCall::CONF_JOIN:
            return "CONF_JOIN";
        case IuMtcCall::CONF_DROP:
            return "CONF_DROP";
        case IuMtcCall::CONF_DELETE:
            return "CONF_DELETE";
        case IuMtcCall::ATTACH:
            return "ATTACH";
        case IuMtcCall::PUSH_CALL:
            return "PUSH_CALL";
        case IuMtcCall::CANCEL_CALL_PUSH:
            return "CANCEL_CALL_PUSH";
        case IuMtcCall::ECT_START:
            return "ECT_START";
        case IuMtcCall::ECT_START_BLIND:
            return "ECT_START_BLIND";

        default:
            return "__INVALID__";
    }
}

/* ------------------------------------------------------------------------------------------------
------------------------------------------------------------------------------------------------ */
inline const IMS_CHAR* UCStrSessionEvtI2U(IN IMS_SINT32 eEvent)
{
    switch (eEvent)
    {
        case IuMtcCall::STARTED:
            return "STARTED";
        case IuMtcCall::START_FAILED:
            return "START_FAILED";
        case IuMtcCall::PROGRESSING:
            return "PROGRESSING";
        case IuMtcCall::HELD:
            return "HELD";
        case IuMtcCall::HOLD_FAILED:
            return "HOLD_FAILED";
        case IuMtcCall::HELD_BY:
            return "HELD_BY";
        case IuMtcCall::RESUMED:
            return "RESUMED";
        case IuMtcCall::RESUME_FAILED:
            return "RESUME_FAILED";
        case IuMtcCall::RESUMED_BY:
            return "RESUMED_BY";
        case IuMtcCall::TERMINATED:
            return "TERMINATED";
        case IuMtcCall::INCOMING_UPDATE:
            return "INCOMING_UPDATE";
        case IuMtcCall::UPDATED:
            return "UPDATED";
        case IuMtcCall::UPDATE_FAILED:
            return "UPDATE_FAILED";
        case IuMtcCall::UPDATED_BY:
            return "UPDATED_BY";
        case IuMtcCall::NOTIFY_INFO:
            return "NOTIFY_INFO";
        case IuMtcCall::INCOMING_RESUME:
            return "INCOMING_RESUME";
        case IuMtcCall::SET_PROPERTY:
            return "SET_PROPERTY";
        case IuMtcCall::CONF_EXPANDED:
            return "CONF_EXPANDED";
        case IuMtcCall::CONF_EXPANDFAILED:
            return "CONF_EXPANDFAILED";
        case IuMtcCall::CONF_EXPANDED_BY:
            return "CONF_EXPANDED_BY";
        case IuMtcCall::CONF_MERGED:
            return "CONF_MERGED";
        case IuMtcCall::CONF_MERGEFAILED:
            return "CONF_MERGEFAILED";
        case IuMtcCall::CONF_JOINED:
            return "CONF_JOINED";
        case IuMtcCall::CONF_DROPPED:
            return "CONF_DROPPED";
        case IuMtcCall::CONF_DELETED:
            return "CONF_DELETED";
        case IuMtcCall::CONF_NOTIFY_USERS_INFO:
            return "CONF_NOTIFY_USERS_INFO";
        case IuMtcCall::CONF_NOTIFY_CONF_INFO:
            return "CONF_NOTIFY_CONF_INFO";
        case IuMtcCall::CALL_PUSH_COMPLETED:
            return "CALL_PUSH_COMPLETED";
        case IuMtcCall::ECT_COMPLETED:
            return "ECT_COMPLETED";
        case IuMtcCall::REPLACED_BY:
            return "REPLACED_BY";
        case IuMtcCall::CODEC_INFO_UPDATED:
            return "CODEC_INFO_UPDATED";

        default:
            return "__INVALID__";
    }
}
#endif  // INTERFACE_UI_MTC_CALL_H_
