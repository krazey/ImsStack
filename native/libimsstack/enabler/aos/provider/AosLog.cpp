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

__IMS_TRACE_TAG_AOS__;

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
        case ApplicationLog::MSG_CONDITION:
            return "MSG_CONDITION";

        case ApplicationLog::MSG_CONNECTION:
            return "MSG_CONNECTION";

        case ApplicationLog::MSG_REGISTRATION:
            return "MSG_REGISTRATION";

        case ApplicationLog::MSG_INIT:
            return "MSG_INIT";

        case ApplicationLog::MSG_REG_START:
            return "MSG_REG_START";

        case ApplicationLog::MSG_REG_UPDATE:
            return "MSG_REG_UPDATE";

        case ApplicationLog::MSG_REG_STOP:
            return "MSG_REG_STOP";

        case ApplicationLog::MSG_REG_RECONFIG:
            return "MSG_REG_RECONFIG";

        case ApplicationLog::MSG_REG_RECOVER:
            return "MSG_REG_RECOVER";

        case ApplicationLog::MSG_IPCAN_CHANGED:
            return "MSG_IPCAN_CHANGED";

        case ApplicationLog::MSG_PUB_TERMINATED:
            return "MSG_PUB_TERMINATED";

        case ApplicationLog::MSG_DESTROY:
            return "MSG_DESTROY";

        case ApplicationLog::MSG_IMS_EST_TIMER_CONTROL:
            return "MSG_IMS_EST_TIMER_CONTROL";

        case ApplicationLog::MSG_REG_EXCHANGE:
            return "MSG_REG_EXCHANGE";

        case ApplicationLog::MSG_AC_CONFIGURED:
            return "MSG_AC_CONFIGURED";

        case ApplicationLog::MSG_PCSCF_RECOVER:
            return "MSG_PCSCF_RECOVER";

        case ApplicationLog::MSG_SCSCF_RESTORATION:
            return "MSG_SCSCF_RESTORATION";

        case ApplicationLog::MSG_OTHERS:
            return "MSG_OTHERS";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppPendingToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case ApplicationLog::PENDING_REG_RECOVERY_HELD:
            return "PENDING_REG_RECOVERY_HELD";

        case ApplicationLog::PENDING_REG_STOP_HELD:
            return "PENDING_REG_STOP_HELD";

        case ApplicationLog::PENDING_APP_DESTROY_HELD:
            return "PENDING_APP_DESTROY_HELD";

        case ApplicationLog::PENDING_REG_RECONFIG_HELD:
            return "PENDING_REG_RECONFIG_HELD";

        case ApplicationLog::PENDING_REG_AFTER_CSFB_COMPLETE:
            return "PENDING_REG_AFTER_CSFB_COMPLETE";

        case ApplicationLog::PENDING_IPCAN_HELD:
            return "PENDING_IPCAN_HELD";

        case ApplicationLog::PENDING_REG_UPDATE_HELD:
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

        case ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF:
            return "E_REGISTER_FAKE_WITH_NEXT_PCSCF";

        case ImsAosControl::PCSCF_NEXT:
            return "PCSCF_NEXT";

        case ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY:
            return "PCSCF_NEXT_WITH_DISCOVERY";

        case ImsAosControl::IPSEC_DISABLED:
            return "IPSEC_DISABLED";

        case ImsAosControl::RETRY_COUNT_INCREASE:
            return "RETRY_COUNT_INCREASE";

        case ImsAosControl::RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION:
            return "RETRY_COUNT_INCREASE_WITH_INITIAL_REGISTRATION";

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
        case ApplicationLog::STATE_NOTREADY:
            return "STATE_NOTREADY";

        case ApplicationLog::STATE_READY:
            return "STATE_READY";

        case ApplicationLog::STATE_CONNECTING:
            return "STATE_CONNECTING";

        case ApplicationLog::STATE_CONNECTED:
            return "STATE_CONNECTED";

        case ApplicationLog::STATE_UPDATING:
            return "STATE_UPDATING";

        case ApplicationLog::STATE_DISCONNECTING:
            return "STATE_DISCONNECTING";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppTimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case ApplicationLog::TIMER_RECONFIG_GUARD:
            return "TIMER_RECONFIG_GUARD";

        case ApplicationLog::TIMER_MSG_CONDITION:
            return "TIMER_MSG_CONDITION";

        case ApplicationLog::TIMER_REG_STOP:
            return "TIMER_REG_STOP";

        case ApplicationLog::TIMER_REG_BLOCKED:
            return "TIMER_REG_BLOCKED";

        case ApplicationLog::TIMER_APP_ACTIVATED:
            return "TIMER_APP_ACTIVATED";

        case ApplicationLog::TIMER_APP_CONNECTED:
            return "TIMER_APP_CONNECTED";

        case ApplicationLog::TIMER_APP_TERMINATED:
            return "TIMER_APP_TERMINATED";

        case ApplicationLog::TIMER_PDN_BLOCKED:
            return "TIMER_PDN_BLOCKED";

        case ApplicationLog::TIMER_IMS_ESTABLISHMENT:
            return "TIMER_IMS_ESTABLISHMENT";

        case ApplicationLog::TIMER_RAT_BLOCK:
            return "TIMER_RAT_BLOCK";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegMessageToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case RegistrationLog::MSG_REG_START:
            return "MSG_REG_START";

        case RegistrationLog::MSG_REG_REINITIATE:
            return "MSG_REG_REINITIATE";

        case RegistrationLog::MSG_REG_UPDATE:
            return "MSG_REG_UPDATE";

        case RegistrationLog::MSG_REG_RECONFIG:
            return "MSG_REG_RECONFIG";

        case RegistrationLog::MSG_REG_REQUIRED_WITH_WAIT_TIME:
            return "MSG_REG_REQUIRED_WITH_WAIT_TIME";

        case RegistrationLog::MSG_REG_REQUIRED_WITH_NEXT_PCSCF:
            return "MSG_REG_REQUIRED_WITH_NEXT_PCSCF";

        case RegistrationLog::MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION:
            return "MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION";

        case RegistrationLog::MSG_REG_REINITIATE_WITH_REG_STATE:
            return "MSG_REG_REINITIATE_WITH_REG_STATE";

        case RegistrationLog::MSG_REG_TERMINATED_BY_NOTIFY:
            return "MSG_REG_TERMINATED_BY_NOTIFY";

        case RegistrationLog::MSG_SUB_REINITIATE:
            return "MSG_SUB_REINITIATE";

        case RegistrationLog::MSG_SUB_TERMINATED:
            return "MSG_SUB_TERMINATED";

        case RegistrationLog::MSG_REG_EVENT_REGISTERED:
            return "MSG_REG_EVENT_REGISTERED";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegModeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case RegistrationLog::MODE_NORMAL:
            return "MODE_NORMAL";

        case RegistrationLog::MODE_LIMITED:
            return "MODE_LIMITED";

        case RegistrationLog::MODE_FAKE:
            return "MODE_FAKE";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegPendingToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case RegistrationLog::PENDING_NONE:
            return "PENDING_NONE";

        case RegistrationLog::PENDING_START:
            return "PENDING_START";

        case RegistrationLog::PENDING_TRANSACTION:
            return "PENDING_TRANSACTION";

        case RegistrationLog::PENDING_UPDATE:
            return "PENDING_UPDATE";

        case RegistrationLog::PENDING_RECONFIG:
            return "PENDING_RECONFIG";

        case RegistrationLog::PENDING_UPDATE_HELD_BY_CALL:
            return "PENDING_UPDATE_HELD_BY_CALL";

        case RegistrationLog::PENDING_PLMN_BLOCK_HELD_BY_CALL:
            return "PENDING_PLMN_BLOCK_HELD_BY_CALL";

        case RegistrationLog::PENDING_SUBSCRIPTION:
            return "PENDING_SUBSCRIPTION";

        case RegistrationLog::PENDING_TERMINATED:
            return "PENDING_TERMINATED";

        case RegistrationLog::PENDING_TRAFFIC:
            return "PENDING_TRAFFIC";

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
        case RegistrationLog::STATE_OFFLINE:
            return "STATE_OFFLINE";

        case RegistrationLog::STATE_REGISTERING:
            return "STATE_REGISTERING";

        case RegistrationLog::STATE_REGSTOP:
            return "STATE_REGSTOP";

        case RegistrationLog::STATE_REGISTERED:
            return "STATE_REGISTERED";

        case RegistrationLog::STATE_REFRESHING:
            return "STATE_REFRESHING";

        case RegistrationLog::STATE_REFRESHSTOP:
            return "STATE_REFRESHSTOP";

        case RegistrationLog::STATE_DEREGISTERING:
            return "STATE_DEREGISTERING";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegTimerToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case RegistrationLog::TIMER_OFFLINE_RECOVER:
            return "TIMER_OFFLINE_RECOVER";

        case RegistrationLog::TIMER_STOP_RETRY:
            return "TIMER_STOP_RETRY";

        case RegistrationLog::TIMER_REFRESH:
            return "TIMER_REFRESH";

        case RegistrationLog::TIMER_EXPIRED:
            return "TIMER_EXPIRED";

        case RegistrationLog::TIMER_MODE:
            return "TIMER_MODE";

        case RegistrationLog::TIMER_TRANSACTION:
            return "TIMER_TRANSACTION";

        case RegistrationLog::TIMER_INTERNAL_ERROR:
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

        case IMS_EVENT_LTE_INFO:
            return "IMS_EVENT_LTE_INFO";

        case IMS_EVENT_WFC_SETTING_CHANGED:
            return "IMS_EVENT_WFC_SETTING_CHANGED";

        case IMS_EVENT_VOLTE_SETTING:
            return "IMS_EVENT_VOLTE_SETTING";

        default:
            return "__INVALID__";
    }
}
