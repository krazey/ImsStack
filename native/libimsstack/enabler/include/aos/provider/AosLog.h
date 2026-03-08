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
#include "provider/AosStaticProfile.h"

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
        TIMER_IMS_ESTABLISHMENT,
        TIMER_RAT_BLOCK
    };

    // INTMSG CONNECTION wParam
    enum
    {
        CONNECTION_ACTIVATED = 10,
        CONNECTION_DEACTIVATED,
        CONNECTION_UPDATED
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
        MSG_REG_FORBIDDEN_IN_WIFI,
        MSG_REG_PROCESS_GIBA,
        MSG_REG_RESTART,

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

        PENDING_TRAFFIC = 0x100,
        PENDING_STOP = 0x200
    };

    enum
    {
        TIMER_OFFLINE_RECOVER = 100,
        TIMER_STOP_RETRY,
        TIMER_REFRESH,
        TIMER_DEREG_TRAFFIC,
        TIMER_MODE,
        TIMER_TRANSACTION,
        TIMER_INTERNAL_ERROR,
        TIMER_WAIT_EMERGENCY_NETWORK,
        TIMER_EXIT_EMERGENCY_MODE
    };

    enum
    {
        RESULT_NONE = 0,

        RESULT_SUCCESS,
        RESULT_TRYING,
        RESULT_FAILURE
    };

    enum
    {
        REASON_NONE = 0,

        /// RESULT_SUCCESS

        /// RESULT_TRYING
        REASON_TRYING_START,
        REASON_TRYING_UPDATE,
        REASON_TRYING_STOP,

        /// RESULT_FAILURE
        REASON_FAILURE_GENERAL,
        REASON_FAILURE_SPECIAL,

        REASON_FAILURE_FORBIDDEN,
        REASON_FAILURE_FORBIDDEN_IN_WIFI,
        REASON_FAILURE_AUTHENTICATION,
        REASON_FAILURE_USIM_AUTHENTICATION,
        REASON_FAILURE_TERMINATED,
        REASON_FAILURE_INTERNAL,
        REASON_FAILURE_BANNDED,
        REASON_FAILURE_INVALID_REGINFO,
        REASON_FAILURE_PDN_RECONNECT,
        REASON_FAILURE_PDN_RECONNECT_WITH_AWT,
        REASON_FAILURE_NEXT_PCSCF_REQUIRED,
        REASON_FAILURE_NO_PCSCF_AVAILABLE,
        REASON_FAILURE_REG_TERMINATING,
        REASON_FAILURE_PCO_LIMITED_SERVICE,
        REASON_FAILURE_PLMN_BLOCK_WITH_TIMEOUT
    };
};

class ConnectorLog
{
public:
    enum
    {
        REASON_NONE = 0,

        // Connection_Deactivated
        REASON_DISCONNECTED,
        REASON_FAILED,
        REASON_PCSCF_DISCOVERY_FAILED,
        REASON_PERMANENTLY_FAILED,
        REASON_LIMITED_SERVICE_PCO,

        // Connection_Updated
        REASON_IP_CHANGED,
        REASON_PCSCF_CHANGED,
        REASON_IPCAN_CAT_CHANGED,

        REASON_OTHERS
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
    static const IMS_CHAR* AppConnectionStateToString(IN IMS_UINT32 nState);

    // Registration Log
    static const IMS_CHAR* RegMessageToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegModeToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegPendingToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegReasonToString(IN IMS_SINT32 nType);
    static const IMS_CHAR* RegStateToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegTimerToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegResultToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* RegReasonForResultToString(IN IMS_UINT32 nType);

    // AosConnector Log
    static const IMS_CHAR* ConnectorReasonToString(IN IMS_UINT32 eType);

    // AosReason
    static const IMS_CHAR* AosReasonToString(IN IMS_UINT32 nReason);

    // AosStaticProfile
    static const IMS_CHAR* AosRegistrationTypeToString(IN AosRegistrationType eType);

    // Event Log
    static const IMS_CHAR* EventToString(IN IMS_SINT32 nEvent);
};

#endif  // AOS_LOG_H_
