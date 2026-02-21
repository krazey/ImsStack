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
#include "AosReason.h"
#include "provider/AosLog.h"
#include "provider/AosStaticProfile.h"

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

        case ImsAosControl::REGISTER_REINITIATE:
            return "REGISTER_REINITIATE";

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

PUBLIC GLOBAL const IMS_CHAR* AosLog::AppConnectionStateToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case ApplicationLog::CONNECTION_ACTIVATED:
            return "ACTIVATED";

        case ApplicationLog::CONNECTION_DEACTIVATED:
            return "DEACTIVATED";

        case ApplicationLog::CONNECTION_UPDATED:
            return "UPDATED";

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

        case RegistrationLog::MSG_REG_FORBIDDEN_IN_WIFI:
            return "MSG_REG_FORBIDDEN_IN_WIFI";

        case RegistrationLog::MSG_REG_PROCESS_GIBA:
            return "MSG_REG_PROCESS_GIBA";

        case RegistrationLog::MSG_REG_RESTART:
            return "MSG_REG_RESTART";

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

        case RegistrationLog::PENDING_STOP:
            return "PENDING_STOP";

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

        case RegistrationLog::TIMER_DEREG_TRAFFIC:
            return "TIMER_DEREG_TRAFFIC";

        case RegistrationLog::TIMER_MODE:
            return "TIMER_MODE";

        case RegistrationLog::TIMER_TRANSACTION:
            return "TIMER_TRANSACTION";

        case RegistrationLog::TIMER_INTERNAL_ERROR:
            return "TIMER_INTERNAL_ERROR";

        case RegistrationLog::TIMER_WAIT_EMERGENCY_NETWORK:
            return "TIMER_WAIT_EMERGENCY_NETWORK";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegResultToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case RegistrationLog::RESULT_NONE:
            return "NONE";

        case RegistrationLog::RESULT_SUCCESS:
            return "SUCCESS";

        case RegistrationLog::RESULT_TRYING:
            return "TRYING";

        case RegistrationLog::RESULT_FAILURE:
            return "FAILURE";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::RegReasonForResultToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case RegistrationLog::REASON_NONE:
            return "NONE";

            // --- RESULT_TRYING ---
        case RegistrationLog::REASON_TRYING_START:
            return "TRYING_START";
        case RegistrationLog::REASON_TRYING_UPDATE:
            return "TRYING_UPDATE";
        case RegistrationLog::REASON_TRYING_STOP:
            return "TRYING_STOP";

            // --- RESULT_FAILURE ---
        case RegistrationLog::REASON_FAILURE_GENERAL:
            return "FAILURE_GENERAL";
        case RegistrationLog::REASON_FAILURE_SPECIAL:
            return "FAILURE_SPECIAL";
        case RegistrationLog::REASON_FAILURE_FORBIDDEN:
            return "FAILURE_FORBIDDEN";
        case RegistrationLog::REASON_FAILURE_FORBIDDEN_IN_WIFI:
            return "FAILURE_FORBIDDEN_IN_WIFI";
        case RegistrationLog::REASON_FAILURE_AUTHENTICATION:
            return "FAILURE_AUTHENTICATION";
        case RegistrationLog::REASON_FAILURE_USIM_AUTHENTICATION:
            return "FAILURE_USIM_AUTHENTICATION";
        case RegistrationLog::REASON_FAILURE_TERMINATED:
            return "FAILURE_TERMINATED";
        case RegistrationLog::REASON_FAILURE_INTERNAL:
            return "FAILURE_INTERNAL";
        case RegistrationLog::REASON_FAILURE_BANNDED:
            return "FAILURE_BANNDED";
        case RegistrationLog::REASON_FAILURE_INVALID_REGINFO:
            return "FAILURE_INVALID_REGINFO";
        case RegistrationLog::REASON_FAILURE_PDN_RECONNECT:
            return "FAILURE_PDN_RECONNECT";
        case RegistrationLog::REASON_FAILURE_PDN_RECONNECT_WITH_AWT:
            return "FAILURE_PDN_RECONNECT_WITH_AWT";
        case RegistrationLog::REASON_FAILURE_NEXT_PCSCF_REQUIRED:
            return "FAILURE_NEXT_PCSCF_REQUIRED";
        case RegistrationLog::REASON_FAILURE_NO_PCSCF_AVAILABLE:
            return "FAILURE_NO_PCSCF_AVAILABLE";
        case RegistrationLog::REASON_FAILURE_REG_TERMINATING:
            return "FAILURE_REG_TERMINATING";
        case RegistrationLog::REASON_FAILURE_PCO_LIMITED_SERVICE:
            return "FAILURE_PCO_LIMITED_SERVICE";
        case RegistrationLog::REASON_FAILURE_PLMN_BLOCK_WITH_TIMEOUT:
            return "FAILURE_PLMN_BLOCK_WITH_TIMEOUT";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::ConnectorReasonToString(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case ConnectorLog::REASON_NONE:
            return "REASON_NONE";

            // Connection_Deactivated reasons
        case ConnectorLog::REASON_DISCONNECTED:
            return "REASON_DISCONNECTED";
        case ConnectorLog::REASON_FAILED:
            return "REASON_FAILED";
        case ConnectorLog::REASON_PCSCF_DISCOVERY_FAILED:
            return "REASON_PCSCF_DISCOVERY_FAILED";
        case ConnectorLog::REASON_PERMANENTLY_FAILED:
            return "REASON_PERMANENTLY_FAILED";
        case ConnectorLog::REASON_LIMITED_SERVICE_PCO:
            return "REASON_LIMITED_SERVICE_PCO";

            // Connection_Updated reasons
        case ConnectorLog::REASON_IP_CHANGED:
            return "REASON_IP_CHANGED";
        case ConnectorLog::REASON_PCSCF_CHANGED:
            return "REASON_PCSCF_CHANGED";
        case ConnectorLog::REASON_IPCAN_CAT_CHANGED:
            return "REASON_IPCAN_CAT_CHANGED";

        case ConnectorLog::REASON_OTHERS:
            return "REASON_OTHERS";

        default:
            return "__INVALID__";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AosReasonToString(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case AosReason::NONE:
            return "NONE";
        case AosReason::POWER_OFF:
            return "POWER_OFF";
        case AosReason::AIRPLANE_MODE:
            return "AIRPLANE_MODE";
        case AosReason::SERVICE_POLICY:
            return "SERVICE_POLICY";
        case AosReason::IMS_DISABLED:
            return "IMS_DISABLED";
        case AosReason::TTYMODEON:
            return "TTY_MODE_ON";
        case AosReason::WIFI_OFF:
            return "WIFI_OFF";
        case AosReason::VOPS_NOT_SUPPORTED:
            return "VOPS_NOT_SUPPORTED";
        case AosReason::SSAC_BARRED:
            return "SSAC_BARRED";
        case AosReason::NOT_SPECIFIED:
            return "NOT_SPECIFIED";
        case AosReason::IP_CHANGED:
            return "IP_CHANGED";
        case AosReason::DATA_DISCONNECTED:
            return "DATA_DISCONNECTED";
        case AosReason::DATA_CONNECTION_MAINTAIN:
            return "DATA_CONNECTION_MAINTAIN";
        case AosReason::DATA_PERMANENTLY_FAILED:
            return "DATA_PERMANENTLY_FAILED";
        case AosReason::NETWORK_ATTACH_REJECTED:
            return "NETWORK_ATTACH_REJECTED";
        case AosReason::REG_FAILURE:
            return "REG_FAILURE";
        case AosReason::REG_TERMINATED:
            return "REG_TERMINATED";
        case AosReason::INITIAL_REG_REQUESTED:
            return "INITIAL_REG_REQUESTED";
        case AosReason::REG_TERMINATING:
            return "REG_TERMINATING";
        case AosReason::REG_ALL_PCSCF_FAILED:
            return "REG_ALL_PCSCF_FAILED";
        default:
            return "UNKNOWN";
    }
}

PUBLIC GLOBAL const IMS_CHAR* AosLog::AosRegistrationTypeToString(IN AosRegistrationType eType)
{
    switch (static_cast<IMS_UINT32>(eType))
    {
        case static_cast<IMS_UINT32>(AosRegistrationType::NORMAL):
            return "NORMAL";

        case static_cast<IMS_UINT32>(AosRegistrationType::EMERGENCY):
            return "EMERGENCY";

        case static_cast<IMS_UINT32>(AosRegistrationType::FAKE):
            return "FAKE";

        case static_cast<IMS_UINT32>(AosRegistrationType::RCS):
            return "RCS";

        default:
            return "UNKNOWN";
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
