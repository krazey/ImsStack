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
#ifndef AOS_LOG_H_
#define AOS_LOG_H_

#include "interface/AosInternalMsgDef.h"

class ApplicationLog
{
public:
    // State
    enum
    {
        STATE_NOTREADY = 0,
        STATE_READY,
        STATE_CONNECTING,
        STATE_CONNECTED,
        STATE_UPDATING,
        STATE_DISCONNECTING
    };

    // Message
    enum
    {
        // State-Machine MSG
        MSG_CONDITION = AOSMSG_SERVICE_INTERNAL,
        MSG_CONNECTION,
        MSG_REGISTRATION,

        // NO State-Machine MSG
        MSG_INIT = AOSMSG_SERVICE_INTERNAL + 10,
        MSG_REG_START,
        MSG_REG_UPDATE,
        MSG_REG_STOP,
        MSG_REG_RECONFIG,
        MSG_REG_RECOVER,
        MSG_IPCAN_CHANGED,
        MSG_PUB_TERMINATED,
        MSG_DESTROY,
        MSG_IMS_EST_TIMER_CONTROL,
        MSG_REG_EXCHANGE,
        MSG_AC_CONFIGURED,
        MSG_PCSCF_RECOVER,
        MSG_SCSCF_RESTORATION,
        MSG_OTHERS
    };

    // Pending
    enum
    {
        PENDING_NONE = 0x0,

        // REG PENDING
        PENDING_REG_RECOVERY_HELD = 0x1,

        // REG STOP PENDING
        PENDING_REG_STOP_HELD = 0x2,

        // APP PENDING
        PENDING_APP_DESTROY_HELD = 0x4,

        // REG RECONFIG PENDING
        PENDING_REG_RECONFIG_HELD = 0x8,

        // After CSFB
        PENDING_REG_AFTER_CSFB_COMPLETE = 0x10,

        // IPCAN PENDING
        PENDING_IPCAN_HELD = 0x20,

        // REG UPDATE PENDING
        PENDING_REG_UPDATE_HELD = 0x40
    };

    // Timer
    enum
    {
        TIMER_RECONFIG_GUARD = 0,
        TIMER_MSG_CONDITION,
        TIMER_REG_STOP,
        TIMER_REG_BLOCKED,
        TIMER_APP_ACTIVATED,
        TIMER_APP_CONNECTED,
        TIMER_APP_TERMINATED,
        TIMER_PDN_BLOCKED,
        TIMER_IMS_ESTABLISHMENT
    };
};

class RegistrationLog
{
public:
    enum
    {
        STATE_OFFLINE = 0,
        STATE_REGISTERING,
        STATE_REGSTOP,
        STATE_REGISTERED,
        STATE_REFRESHING,
        STATE_REFRESHSTOP,
        STATE_DEREGISTERING
    };

    enum
    {
        MSG_REG_START = AOSMSG_SERVICE_INTERNAL,

        MSG_REG_REINITIATE,
        MSG_REG_UPDATE,
        MSG_REG_RECONFIG,

        MSG_REG_REQUIRED_WITH_WAIT_TIME,
        MSG_REG_REQUIRED_WITH_NEXT_PCSCF,
        MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION,
        MSG_REG_REINITIATE_WITH_REG_STATE,
        MSG_REG_TERMINATED_BY_NOTIFY,

        MSG_SUB_REINITIATE,
        MSG_SUB_TERMINATED,

        MSG_REG_EVENT_REGISTERED
    };

    enum
    {
        MODE_NORMAL = 0,
        MODE_LIMITED,
        MODE_FAKE
    };

    enum
    {
        PENDING_NONE = 0x0,
        PENDING_START = 0x1,
        PENDING_TRANSACTION = 0x2,
        PENDING_UPDATE = 0x4,
        PENDING_RECONFIG = 0x8,
        PENDING_UPDATE_HELD_BY_CALL = 0x10,
        PENDING_PLMN_BLOCK_HELD_BY_CALL = 0x20,

        PENDING_SUBSCRIPTION = 0x40,
        PENDING_TERMINATED = 0x80,

        PENDING_TRAFFIC = 0x100
    };

    enum
    {
        TIMER_OFFLINE_RECOVER = 100,
        TIMER_STOP_RETRY,
        TIMER_REFRESH,
        TIMER_EXPIRED,
        TIMER_MODE,
        TIMER_TRANSACTION,
        TIMER_INTERNAL_ERROR
    };
};

class AosLog
{
public:
    AosLog();
    virtual ~AosLog();

    // Application Log
    static const IMS_CHAR* AppMessageToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppPendingToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppRequestToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppStateToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* AppTimerToString(IN IMS_UINT32 nType);

    // Registration Log
    static const IMS_CHAR* RegMessageToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegModeToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegPendingToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegReasonToString(IN IMS_SINT32 nType);
    static const IMS_CHAR* RegStateToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegTimerToString(IN IMS_UINT32 nType);

    // Event Log
    static const IMS_CHAR* EventToString(IN IMS_SINT32 nEvent);
};

#endif  // AOS_LOG_H_
