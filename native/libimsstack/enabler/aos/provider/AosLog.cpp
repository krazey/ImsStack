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
#include "ImsEventDef.h"
#include "IRegistration.h"
#include "AoSAppRequestType.h"
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
        PENDING_APP_DESTROY_HELD = 0x4
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

        PENDING_SUBSCRIPTION = 0x10
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

/*

Remarks

*/
PUBLIC
AosLog::AosLog()
{
    IMS_TRACE_D("AosLog()", 0, 0, 0);
}

/*

Remarks

*/
PUBLIC VIRTUAL AosLog::~AosLog()
{
    IMS_TRACE_D("~AosLog()", 0, 0, 0);
}

/*

Remarks

*/
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

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
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

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PUBLIC GLOBAL const IMS_CHAR* AosLog::AppRequestToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case AoSAppRequest::COMMAND_REGISTER:
            return "COMMAND_REGISTER";

        case AoSAppRequest::COMMAND_REGISTER_RECOVERY:
            return "COMMAND_REGISTER_RECOVERY";

        case AoSAppRequest::COMMAND_REGISTER_STOP:
            return "COMMAND_REGISTER_STOP";

        case AoSAppRequest::COMMAND_REGISTER_NEXT_PCSCF:
            return "COMMAND_REGISTER_NEXT_PCSCF";

        case AoSAppRequest::COMMAND_REGISTER_REFRESH:
            return "COMMAND_REGISTER_REFRESH";

        case AoSAppRequest::COMMAND_SET_CALL_FALLBACK:
            return "COMMAND_SET_CALL_FALLBACK";

        case AoSAppRequest::COMMAND_SET_PUBLISH_STARTED:
            return "COMMAND_SET_PUBLISH_STARTED";

        case AoSAppRequest::COMMAND_SET_PUBLISH_TERMINATED:
            return "COMMAND_SET_PUBLISH_TERMINATED";

        case AoSAppRequest::COMMAND_SET_UNBLOCK_AC_INCOMPLTED:
            return "COMMAND_SET_UNBLOCK_AC_INCOMPLTED";

        case AoSAppRequest::COMMAND_SET_BLOCK_AC_INCOMPLTED:
            return "COMMAND_SET_BLOCK_AC_INCOMPLTED";

        case AoSAppRequest::COMMAND_SET_AC_CONFIGURED:
            return "COMMAND_SET_AC_CONFIGURED";

        case AoSAppRequest::COMMAND_FAKE_E_REGISTER:
            return "COMMAND_FAKE_E_REGISTER";

        case AoSAppRequest::COMMAND_HANDLE_INSTANTANEOUS_OFFLINE:
            return "COMMAND_NO_RTP_PING_CHECK";

        case AoSAppRequest::COMMAND_NO_RTP_PING_CHECK:
            return "COMMAND_NO_RTP_PING_CHECK";

        case AoSAppRequest::COMMAND_REGINFO_UPDATE:
            return "COMMAND_REGINFO_UPDATE";

        case AoSAppRequest::COMMAND_SERVICE_CONTROL:
            return "COMMAND_SERVICE_CONTROL";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
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

/*

Remarks

*/
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

/*

Remarks

*/
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

/*

Remarks

*/
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

/*

Remarks

*/
PUBLIC GLOBAL const IMS_CHAR* AosLog::RegPendingToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case Registration::PENDING_NONE:
            return "PENDING_NONE";

        case Registration::PENDING_RECONFIG:
            return "PENDING_RECONFIG";

        case Registration::PENDING_TRANSACTION:
            return "PENDING_TRANSACTION";

        case Registration::PENDING_UPDATE_HELD_BY_CALL:
            return "PENDING_UPDATE_HELD_BY_CALL";

        case Registration::PENDING_SUBSCRIPTION:
            return "PENDING_SUBSCRIPTION";

        default:
            return "__INVALID__";
    }
}

/*

Remarks

*/
PUBLIC GLOBAL const IMS_CHAR* AosLog::RegReasonToString(IN IMS_SINT32 nType)
{
    switch (nType)
    {
        case IRegistration::REASON_NONE:
            return "RESULT_NONE";

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

/*

Remarks

*/
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

/*

Remarks

*/
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

/*

Remarks

*/
PUBLIC GLOBAL const IMS_CHAR* AosLog::EventToString(IN IMS_SINT32 nEvent)
{
    switch (nEvent)
    {
        case IMS_EVENT_ROAMING_STATE:
            return "IMS_EVENT_ROAMING_STATE";

        case IMS_EVENT_SERVICE_SETTING:
            return "IMS_EVENT_SERVICE_SETTING";

        case IMS_EVENT_IMS_VOICE_OVER_PS_STATE:
            return "IMS_EVENT_IMS_VOICE_OVER_PS_STATE";

        case IMS_EVENT_UEINITIATED_IMSPDN_DISCONNECTION:
            return "IMS_EVENT_UEINITIATED_IMSPDN_DISCONNECTION";

        case IMS_EVENT_DATA_MODE:
            return "IMS_EVENT_DATA_MODE";

        case IMS_EVENT_CSCALL_STATE:
            return "IMS_EVENT_CSCALL_STATE";

        case IMS_EVENT_WPS_CALL_STATE:
            return "IMS_EVENT_WPS_CALL_STATE";

        case IMS_EVENT_RADIO_OFF:
            return "IMS_EVENT_RADIO_OFF";

        case IMS_EVENT_LTE_STATE:
            return "IMS_EVENT_LTE_STATE";

        case IMS_EVENT_VOIP_SETTING:
            return "IMS_EVENT_VOIP_SETTING";

        case IMS_EVENT_WFC_SETTING_CHANGED:
            return "IMS_EVENT_WFC_SETTING_CHANGED";

        case IMS_EVENT_MOBILE_DATA_SETTING:
            return "IMS_EVENT_MOBILE_DATA_SETTING";

        case IMS_EVENT_VIDEO_SETTING:
            return "IMS_EVENT_VIDEO_SETTING";

        case IMS_EVENT_VOIP_NETWORK_CAPAVILITY:
            return "IMS_EVENT_VOIP_NETWORK_CAPAVILITY";

        case IMS_EVENT_REG_PREF_STATE:
            return "IMS_EVENT_REG_PREF_STATE";

        case IMS_EVENT_OMADM_UPDATED:
            return "IMS_EVENT_OMADM_UPDATED";

        case IMS_EVENT_NETWORK_CAPABILITY:
            return "IMS_EVENT_NETWORK_CAPABILITY";

        case IMS_EVENT_ROAMING_PREFERRED_VOICE_CALL_NETWORK:
            return "IMS_EVENT_ROAMING_PREFERRED_VOICE_CALL_NETWORK";

        case IMS_EVENT_VOLTE_SETTING:
            return "IMS_EVENT_VOLTE_SETTING";

        case IMS_EVENT_DATA_ROAMING_SETTING:
            return "IMS_EVENT_DATA_ROAMING_SETTING";

        case IMS_EVENT_MOBILE_DATA_LIMIT_CHANGED:
            return "IMS_EVENT_MOBILE_DATA_LIMIT_CHANGED";

        case IMS_EVENT_SYNC_TO_NATIVE:
            return "IMS_EVENT_SYNC_TO_NATIVE";

        case IMS_EVENT_AVAIL_RAT_INFO_CHANGED:
            return "IMS_EVENT_AVAIL_RAT_INFO_CHANGED";

        default:
            return "__INVALID__";
    }
}
