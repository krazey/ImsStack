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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "ImsAosParameter.h"
#include "ImsEventDef.h"
#include "ImsTypeDef.h"
#include "IRegistration.h"
#include "app/AosApplication.h"
#include "provider/AosLog.h"
#include "provider/AosProvider.h"

class ApplicationLogTest
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
        TIMER_MSG_CONITION,
        TIMER_REG_STOP,
        TIMER_REG_BLOCKED,
        TIMER_APP_ACTIVATED,
        TIMER_APP_CONNECTED,
        TIMER_APP_TERMINATED,
        TIMER_PDN_BLOCKED
    };
};

class RegistrationLogTest
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
        MSG_REG_REQUIRED_WITH_AVAILABLE_NEXT_PCSCF,
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

TEST(AosLogTest, AppMessageToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_CONDITION),
            "MSG_CONDITION");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_CONNECTION),
            "MSG_CONNECTION");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_REGISTRATION),
            "MSG_REGISTRATION");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_INIT), "MSG_INIT");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_REG_START),
            "MSG_REG_START");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_REG_UPDATE),
            "MSG_REG_UPDATE");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_REG_STOP),
            "MSG_REG_STOP");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_REG_RECONFIG),
            "MSG_REG_RECONFIG");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_REG_RECOVER),
            "MSG_REG_RECOVER");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_IPCAN_CHANGED),
            "MSG_IPCAN_CHANGED");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_PUB_TERMINATED),
            "MSG_PUB_TERMINATED");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_DESTROY),
            "MSG_DESTROY");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(
                         ApplicationLogTest::MSG_IMS_EST_TIMER_CONTROL),
            "MSG_IMS_EST_TIMER_CONTROL");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_REG_EXCHANGE),
            "MSG_REG_EXCHANGE");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_AC_CONFIGURED),
            "MSG_AC_CONFIGURED");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_PCSCF_RECOVER),
            "MSG_PCSCF_RECOVER");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_SCSCF_RESTORATION),
            "MSG_SCSCF_RESTORATION");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_OTHERS),
            "MSG_OTHERS");
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(ApplicationLogTest::MSG_OTHERS + 99),
            "__INVALID__");
}

TEST(AosLogTest, AppPendingToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AppPendingToString(
                         ApplicationLogTest::PENDING_REG_RECOVERY_HELD),
            "PENDING_REG_RECOVERY_HELD");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppPendingToString(ApplicationLogTest::PENDING_REG_STOP_HELD),
            "PENDING_REG_STOP_HELD");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppPendingToString(ApplicationLogTest::PENDING_APP_DESTROY_HELD),
            "PENDING_APP_DESTROY_HELD");
    EXPECT_STREQ(AosProvider::GetLog()->AppPendingToString(
                         ApplicationLogTest::PENDING_REG_RECONFIG_HELD),
            "PENDING_REG_RECONFIG_HELD");
    EXPECT_STREQ(AosProvider::GetLog()->AppPendingToString(
                         ApplicationLogTest::PENDING_REG_AFTER_CSFB_COMPLETE),
            "PENDING_REG_AFTER_CSFB_COMPLETE");
    EXPECT_STREQ(AosProvider::GetLog()->AppPendingToString(ApplicationLogTest::PENDING_IPCAN_HELD),
            "PENDING_IPCAN_HELD");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppPendingToString(ApplicationLogTest::PENDING_REG_UPDATE_HELD),
            "PENDING_REG_UPDATE_HELD");
    EXPECT_STREQ(AosProvider::GetLog()->AppPendingToString(
                         ApplicationLogTest::PENDING_REG_UPDATE_HELD + 99),
            "__INVALID__");
}

TEST(AosLogTest, AppRequestToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::REGISTER_START),
            "REGISTER_START");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::REGISTER_START_WITH_WLAN),
            "REGISTER_START_WITH_WLAN");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::REGISTER_REFRESH),
            "REGISTER_REFRESH");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::REGISTER_STOP),
            "REGISTER_STOP");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::REGISTER_STOP_BY_ROAMING),
            "REGISTER_STOP_BY_ROAMING");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::REGISTER_REINITIATE),
            "REGISTER_REINITIATE");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppRequestToString(ImsAosControl::REGISTER_REINITIATE_BY_CSFB),
            "REGISTER_REINITIATE_BY_CSFB");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(
                         ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF),
            "E_REGISTER_FAKE_WITH_NEXT_PCSCF");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppRequestToString(ImsAosControl::PCSCF_NEXT), "PCSCF_NEXT");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppRequestToString(ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY),
            "PCSCF_NEXT_WITH_DISCOVERY");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::IPSEC_DISABLED),
            "IPSEC_DISABLED");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(ImsAosControl::RETRY_COUNT_INCREASE),
            "RETRY_COUNT_INCREASE");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(
                         ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION),
            "UPDATE_SIP_DELEGATE_REGISTRATION");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(
                         ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION),
            "TRIGGER_SIP_DELEGATE_DEREGISTRATION");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(
                         ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION),
            "TRIGGER_FULL_NETWORK_REGISTRATION");
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(
                         ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION + 99),
            "__INVALID__");
}

TEST(AosLogTest, AppStateToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AppStateToString(ApplicationLogTest::STATE_NOTREADY),
            "STATE_NOTREADY");
    EXPECT_STREQ(AosProvider::GetLog()->AppStateToString(ApplicationLogTest::STATE_READY),
            "STATE_READY");
    EXPECT_STREQ(AosProvider::GetLog()->AppStateToString(ApplicationLogTest::STATE_CONNECTING),
            "STATE_CONNECTING");
    EXPECT_STREQ(AosProvider::GetLog()->AppStateToString(ApplicationLogTest::STATE_CONNECTED),
            "STATE_CONNECTED");
    EXPECT_STREQ(AosProvider::GetLog()->AppStateToString(ApplicationLogTest::STATE_UPDATING),
            "STATE_UPDATING");
    EXPECT_STREQ(AosProvider::GetLog()->AppStateToString(ApplicationLogTest::STATE_DISCONNECTING),
            "STATE_DISCONNECTING");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppStateToString(ApplicationLogTest::STATE_DISCONNECTING + 99),
            "__INVALID__");
}

TEST(AosLogTest, AppTimerToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_RECONFIG_GUARD),
            "TIMER_RECONFIG_GUARD");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_MSG_CONITION),
            "TIMER_MSG_CONITION");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_REG_STOP),
            "TIMER_REG_STOP");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_REG_BLOCKED),
            "TIMER_REG_BLOCKED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_APP_ACTIVATED),
            "TIMER_APP_ACTIVATED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_APP_CONNECTED),
            "TIMER_APP_CONNECTED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_APP_TERMINATED),
            "TIMER_APP_TERMINATED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_PDN_BLOCKED),
            "TIMER_PDN_BLOCKED");
    EXPECT_STREQ(
            AosProvider::GetLog()->AppTimerToString(ApplicationLogTest::TIMER_PDN_BLOCKED + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegMessageToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLogTest::MSG_REG_START),
            "MSG_REG_START");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLogTest::MSG_REG_REINITIATE),
            "MSG_REG_REINITIATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLogTest::MSG_REG_UPDATE),
            "MSG_REG_UPDATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLogTest::MSG_REG_RECONFIG),
            "MSG_REG_RECONFIG");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLogTest::MSG_REG_REQUIRED_WITH_WAIT_TIME),
            "MSG_REG_REQUIRED_WITH_WAIT_TIME");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLogTest::MSG_REG_REQUIRED_WITH_NEXT_PCSCF),
            "MSG_REG_REQUIRED_WITH_NEXT_PCSCF");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLogTest::MSG_REG_REQUIRED_WITH_AVAILABLE_NEXT_PCSCF),
            "MSG_REG_REQUIRED_WITH_AVAILABLE_NEXT_PCSCF");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLogTest::MSG_REG_REINITIATE_WITH_REG_STATE),
            "MSG_REG_REINITIATE_WITH_REG_STATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLogTest::MSG_REG_TERMINATED_BY_NOTIFY),
            "MSG_REG_TERMINATED_BY_NOTIFY");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLogTest::MSG_SUB_REINITIATE),
            "MSG_SUB_REINITIATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLogTest::MSG_SUB_TERMINATED),
            "MSG_SUB_TERMINATED");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLogTest::MSG_REG_EVENT_REGISTERED),
            "MSG_REG_EVENT_REGISTERED");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLogTest::MSG_REG_EVENT_REGISTERED + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegModeToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegModeToString(RegistrationLogTest::MODE_NORMAL),
            "MODE_NORMAL");
    EXPECT_STREQ(AosProvider::GetLog()->RegModeToString(RegistrationLogTest::MODE_LIMITED),
            "MODE_LIMITED");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegModeToString(RegistrationLogTest::MODE_FAKE), "MODE_FAKE");
    EXPECT_STREQ(AosProvider::GetLog()->RegModeToString(RegistrationLogTest::MODE_FAKE + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegPendingToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_NONE),
            "PENDING_NONE");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_START),
            "PENDING_START");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_TRANSACTION),
            "PENDING_TRANSACTION");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_UPDATE),
            "PENDING_UPDATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_RECONFIG),
            "PENDING_RECONFIG");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(
                         RegistrationLogTest::PENDING_UPDATE_HELD_BY_CALL),
            "PENDING_UPDATE_HELD_BY_CALL");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(
                         RegistrationLogTest::PENDING_PLMN_BLOCK_HELD_BY_CALL),
            "PENDING_PLMN_BLOCK_HELD_BY_CALL");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_SUBSCRIPTION),
            "PENDING_SUBSCRIPTION");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_TERMINATED),
            "PENDING_TERMINATED");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_TRAFFIC),
            "PENDING_TRAFFIC");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegPendingToString(RegistrationLogTest::PENDING_TERMINATED + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegReasonToString)
{
    EXPECT_STREQ(
            AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_NONE), "REASON_NONE");
    EXPECT_STREQ(AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_STATUS_CODE),
            "REASON_STATUS_CODE");
    EXPECT_STREQ(AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_NO_EXPIRES),
            "REASON_NO_EXPIRES");
    EXPECT_STREQ(AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_NO_ACTIVE_BINDINGS),
            "REASON_NO_ACTIVE_BINDINGS");
    EXPECT_STREQ(AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_INTERNAL_ERROR),
            "REASON_INTERNAL_ERROR");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_TRANSACTION_TIMEOUT),
            "REASON_TRANSACTION_TIMEOUT");
    EXPECT_STREQ(AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_REFRESH_TIMEOUT),
            "REASON_REFRESH_TIMEOUT");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_REFRESH_INTERNAL_ERROR),
            "REASON_REFRESH_INTERNAL_ERROR");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_CLIENT_SOCKET_ERROR),
            "REASON_CLIENT_SOCKET_ERROR");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegReasonToString(IRegistration::REASON_SERVER_SOCKET_ERROR),
            "REASON_SERVER_SOCKET_ERROR");
    EXPECT_STREQ(AosProvider::GetLog()->RegReasonToString(
                         IRegistration::REASON_SERVER_SOCKET_ERROR + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegStateToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_OFFLINE),
            "STATE_OFFLINE");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_REGISTERING),
            "STATE_REGISTERING");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_REGSTOP),
            "STATE_REGSTOP");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_REGISTERED),
            "STATE_REGISTERED");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_REFRESHING),
            "STATE_REFRESHING");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_REFRESHSTOP),
            "STATE_REFRESHSTOP");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_DEREGISTERING),
            "STATE_DEREGISTERING");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegStateToString(RegistrationLogTest::STATE_DEREGISTERING + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegTimerToString)
{
    EXPECT_STREQ(
            AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_OFFLINE_RECOVER),
            "TIMER_OFFLINE_RECOVER");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_STOP_RETRY),
            "TIMER_STOP_RETRY");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_REFRESH),
            "TIMER_REFRESH");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_EXPIRED),
            "TIMER_EXPIRED");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_MODE), "TIMER_MODE");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_TRANSACTION),
            "TIMER_TRANSACTION");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_INTERNAL_ERROR),
            "TIMER_INTERNAL_ERROR");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegTimerToString(RegistrationLogTest::TIMER_INTERNAL_ERROR + 99),
            "__INVALID__");
}

TEST(AosLogTest, EventToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_ROAMING_STATE),
            "IMS_EVENT_ROAMING_STATE");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_IMS_VOICE_OVER_PS_STATE),
            "IMS_EVENT_IMS_VOICE_OVER_PS_STATE");
    EXPECT_STREQ(
            AosProvider::GetLog()->EventToString(IMS_EVENT_CSCALL_STATE), "IMS_EVENT_CSCALL_STATE");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_LTE_STATE), "IMS_EVENT_LTE_STATE");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_WFC_SETTING_CHANGED),
            "IMS_EVENT_WFC_SETTING_CHANGED");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_VOLTE_SETTING),
            "IMS_EVENT_VOLTE_SETTING");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_VOLTE_SETTING + 99), "__INVALID__");
}