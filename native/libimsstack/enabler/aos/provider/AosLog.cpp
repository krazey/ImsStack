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
#include "ServiceTrace.h"
#include "ImsAosParameter.h"
#include "ImsEventDef.h"
#include "IRegistration.h"
#include "provider/AosLog.h"

__IMS_TRACE_TAG_USER_DECL__("AOS");

class Application
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
        MSG_SERVICE_CONTROL,
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
        TIMER_MSG_CONITION,
        TIMER_REG_STOP,
        TIMER_REG_BLOCKED,
        TIMER_APP_ACTIVATED,
        TIMER_APP_CONNECTED,
        TIMER_APP_TERMINATED,
        TIMER_PDN_BLOCKED
    };
};

class Registration
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
        PENDING_TRANSACTION = 0x1,
        PENDING_UPDATE = 0x2,
        PENDING_RECONFIG = 0x4,
        PENDING_UPDATE_HELD_BY_CALL = 0x8,

        PENDING_SUBSCRIPTION = 0x10,
        PENDING_TERMINATED = 0x20
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

PUBLIC
AosLog::AosLog()
{
    IMS_TRACE_D("AosLog()", 0, 0, 0);
}

PUBLIC VIRTUAL AosLog::~AosLog()
{
    IMS_TRACE_D("~AosLog()", 0, 0, 0);
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppMessageToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Application::MSG_CONDITION:
            return "MSG_CONDITION";

        case Application::MSG_CONNECTION:
            return "MSG_CONNECTION";

        case Application::MSG_REGISTRATION:
            return "MSG_REGISTRATION";

        case Application::MSG_INIT:
            return "MSG_INIT";

        case Application::MSG_REG_START:
            return "MSG_REG_START";

        case Application::MSG_REG_UPDATE:
            return "MSG_REG_UPDATE";

        case Application::MSG_REG_STOP:
            return "MSG_REG_STOP";

        case Application::MSG_REG_RECONFIG:
            return "MSG_REG_RECONFIG";

        case Application::MSG_REG_RECOVER:
            return "MSG_REG_RECOVER";

        case Application::MSG_IPCAN_CHANGED:
            return "MSG_IPCAN_CHANGED";

        case Application::MSG_PUB_TERMINATED:
            return "MSG_PUB_TERMINATED";

        case Application::MSG_DESTROY:
            return "MSG_DESTROY";

        case Application::MSG_SERVICE_CONTROL:
            return "MSG_SERVICE_CONTROL";

        case Application::MSG_REG_EXCHANGE:
            return "MSG_REG_EXCHANGE";

        case Application::MSG_AC_CONFIGURED:
            return "MSG_AC_CONFIGURED";

        case Application::MSG_PCSCF_RECOVER:
            return "MSG_PCSCF_RECOVER";

        case Application::MSG_SCSCF_RESTORATION:
            return "MSG_SCSCF_RESTORATION";

        case Application::MSG_OTHERS:
            return "MSG_OTHERS";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppPendingToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Application::PENDING_REG_RECOVERY_HELD:
            return "PENDING_REG_RECOVERY_HELD";

        case Application::PENDING_REG_STOP_HELD:
            return "PENDING_REG_STOP_HELD";

        case Application::PENDING_APP_DESTROY_HELD:
            return "PENDING_APP_DESTROY_HELD";

        case Application::PENDING_REG_RECONFIG_HELD:
            return "PENDING_REG_RECONFIG_HELD";

        case Application::PENDING_REG_AFTER_CSFB_COMPLETE:
            return "PENDING_REG_AFTER_CSFB_COMPLETE";

        case Application::PENDING_IPCAN_HELD:
            return "PENDING_IPCAN_HELD";

        case Application::PENDING_REG_UPDATE_HELD:
            return "PENDING_REG_UPDATE_HELD";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppRequestToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case ImsAosControl::REGISTER_START:
            return "REGISTER_START";

        case ImsAosControl::REGISTER_START_WITH_WLAN:
            return "REGISTER_START_WITH_WLAN";

        case ImsAosControl::REGISTER_REFRESH:
            return "REGISTER_REFRESH";

        case ImsAosControl::REGISTER_STOP:
            return "REGISTER_STOP";

        case ImsAosControl::REGISTER_STOP_BY_ROAMING:
            return "REGISTER_STOP_BY_ROAMING";

        case ImsAosControl::REGISTER_REINITIATE:
            return "REGISTER_REINITIATE";

        case ImsAosControl::REGISTER_REINITIATE_BY_CSFB:
            return "REGISTER_REINITIATE_BY_CSFB";

        case ImsAosControl::PCSCF_NEXT:
            return "PCSCF_NEXT";

        case ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY:
            return "PCSCF_NEXT_WITH_DISCOVERY";

        case ImsAosControl::RETRY_COUNT_INCREASE:
            return "RETRY_COUNT_INCREASE";

        case ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION:
            return "UPDATE_SIP_DELEGATE_REGISTRATION";

        case ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION:
            return "TRIGGER_SIP_DELEGATE_DEREGISTRATION";

        case ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION:
            return "TRIGGER_FULL_NETWORK_REGISTRATION";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppStateToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Application::STATE_NOTREADY:
            return "STATE_NOTREADY";

        case Application::STATE_READY:
            return "STATE_READY";

        case Application::STATE_CONNECTING:
            return "STATE_CONNECTING";

        case Application::STATE_CONNECTED:
            return "STATE_CONNECTED";

        case Application::STATE_UPDATING:
            return "STATE_UPDATING";

        case Application::STATE_DISCONNECTING:
            return "STATE_DISCONNECTING";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppTimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Application::TIMER_RECONFIG_GUARD:
            return "TIMER_RECONFIG_GUARD";

        case Application::TIMER_MSG_CONITION:
            return "TIMER_MSG_CONITION";

        case Application::TIMER_REG_STOP:
            return "TIMER_REG_STOP";

        case Application::TIMER_REG_BLOCKED:
            return "TIMER_REG_BLOCKED";

        case Application::TIMER_APP_ACTIVATED:
            return "TIMER_APP_ACTIVATED";

        case Application::TIMER_APP_CONNECTED:
            return "TIMER_APP_CONNECTED";

        case Application::TIMER_APP_TERMINATED:
            return "TIMER_APP_TERMINATED";

        case Application::TIMER_PDN_BLOCKED:
            return "TIMER_PDN_BLOCKED";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegMessageToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Registration::MSG_REG_START:
            return "MSG_REG_START";

        case Registration::MSG_REG_REINITIATE:
            return "MSG_REG_REINITIATE";

        case Registration::MSG_REG_UPDATE:
            return "MSG_REG_UPDATE";

        case Registration::MSG_REG_RECONFIG:
            return "MSG_REG_RECONFIG";

        case Registration::MSG_REG_REQUIRED_WITH_WAIT_TIME:
            return "MSG_REG_REQUIRED_WITH_WAIT_TIME";

        case Registration::MSG_REG_REQUIRED_WITH_NEXT_PCSCF:
            return "MSG_REG_REQUIRED_WITH_NEXT_PCSCF";

        case Registration::MSG_REG_REINITIATE_WITH_REG_STATE:
            return "MSG_REG_REINITIATE_WITH_REG_STATE";

        case Registration::MSG_REG_TERMINATED_BY_NOTIFY:
            return "MSG_REG_TERMINATED_BY_NOTIFY";

        case Registration::MSG_SUB_REINITIATE:
            return "MSG_SUB_REINITIATE";

        case Registration::MSG_SUB_TERMINATED:
            return "MSG_SUB_TERMINATED";

        case Registration::MSG_REG_EVENT_REGISTERED:
            return "MSG_REG_EVENT_REGISTERED";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegModeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Registration::MODE_NORMAL:
            return "MODE_NORMAL";

        case Registration::MODE_LIMITED:
            return "MODE_LIMITED";

        case Registration::MODE_FAKE:
            return "MODE_FAKE";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegPendingToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Registration::PENDING_NONE:
            return "PENDING_NONE";

        case Registration::PENDING_TRANSACTION:
            return "PENDING_TRANSACTION";

        case Registration::PENDING_UPDATE:
            return "PENDING_UPDATE";

        case Registration::PENDING_RECONFIG:
            return "PENDING_RECONFIG";

        case Registration::PENDING_UPDATE_HELD_BY_CALL:
            return "PENDING_UPDATE_HELD_BY_CALL";

        case Registration::PENDING_SUBSCRIPTION:
            return "PENDING_SUBSCRIPTION";

        case Registration::PENDING_TERMINATED:
            return "PENDING_TERMINATED";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegReasonToString(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case IRegistration::REASON_NONE:
            return "REASON_NONE";

        case IRegistration::REASON_STATUS_CODE:
            return "REASON_STATUS_CODE";

        case IRegistration::REASON_NO_EXPIRES:
            return "REASON_NO_EXPIRES";

        case IRegistration::REASON_NO_ACTIVE_BINDINGS:
            return "REASON_NO_ACTIVE_BINDINGS";

        case IRegistration::REASON_INTERNAL_ERROR:
            return "REASON_INTERNAL_ERROR";

        case IRegistration::REASON_TRANSACTION_TIMEOUT:
            return "REASON_TRANSACTION_TIMEOUT";

        case IRegistration::REASON_REFRESH_TIMEOUT:
            return "REASON_REFRESH_TIMEOUT";

        case IRegistration::REASON_REFRESH_INTERNAL_ERROR:
            return "REASON_REFRESH_INTERNAL_ERROR";

        case IRegistration::REASON_CLIENT_SOCKET_ERROR:
            return "REASON_CLIENT_SOCKET_ERROR";

        case IRegistration::REASON_SERVER_SOCKET_ERROR:
            return "REASON_SERVER_SOCKET_ERROR";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegStateToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Registration::STATE_OFFLINE:
            return "STATE_OFFLINE";

        case Registration::STATE_REGISTERING:
            return "STATE_REGISTERING";

        case Registration::STATE_REGSTOP:
            return "STATE_REGSTOP";

        case Registration::STATE_REGISTERED:
            return "STATE_REGISTERED";

        case Registration::STATE_REFRESHING:
            return "STATE_REFRESHING";

        case Registration::STATE_REFRESHSTOP:
            return "STATE_REFRESHSTOP";

        case Registration::STATE_DEREGISTERING:
            return "STATE_DEREGISTERING";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegTimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Registration::TIMER_OFFLINE_RECOVER:
            return "TIMER_OFFLINE_RECOVER";

        case Registration::TIMER_STOP_RETRY:
            return "TIMER_STOP_RETRY";

        case Registration::TIMER_REFRESH:
            return "TIMER_REFRESH";

        case Registration::TIMER_EXPIRED:
            return "TIMER_EXPIRED";

        case Registration::TIMER_MODE:
            return "TIMER_MODE";

        case Registration::TIMER_TRANSACTION:
            return "TIMER_TRANSACTION";

        case Registration::TIMER_INTERNAL_ERROR:
            return "TIMER_INTERNAL_ERROR";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::EventToString(IN IMS_SINT32 nEvent)
{
    switch (nEvent)
    {
        case IMS_EVENT_ROAMING_STATE:
            return "IMS_EVENT_ROAMING_STATE";

        case IMS_EVENT_IMS_VOICE_OVER_PS_STATE:
            return "IMS_EVENT_IMS_VOICE_OVER_PS_STATE";

        case IMS_EVENT_CSCALL_STATE:
            return "IMS_EVENT_CSCALL_STATE";

        case IMS_EVENT_LTE_STATE:
            return "IMS_EVENT_LTE_STATE";

        case IMS_EVENT_WFC_SETTING_CHANGED:
            return "IMS_EVENT_WFC_SETTING_CHANGED";

        case IMS_EVENT_VOLTE_SETTING:
            return "IMS_EVENT_VOLTE_SETTING";

        default:
            return "__INVALID__";
    }
}
