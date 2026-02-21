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

using ::testing::TestWithParam;
using ::testing::ValuesIn;

struct AosLogParams
{
    IMS_UINT32 nValue;
    const IMS_CHAR* pszString;
};

using AosLogTestForAppMessage = TestWithParam<AosLogParams>;

TEST_P(AosLogTestForAppMessage, ReturnsValidStringForAppMessage)
{
    // GIVEN
    const AosLogParams& objAosLogParams = GetParam();

    // WHEN
    IMS_UINT32 nValue = objAosLogParams.nValue;
    const IMS_CHAR* pszString = objAosLogParams.pszString;

    // THEN
    EXPECT_STREQ(AosProvider::GetLog()->AppMessageToString(nValue), pszString);
}

INSTANTIATE_TEST_SUITE_P(AosLogInstantiation, AosLogTestForAppMessage,
        ValuesIn<AosLogParams>({
                {ApplicationLog::MSG_CONDITION,             "MSG_CONDITION"            },
                {ApplicationLog::MSG_CONNECTION,            "MSG_CONNECTION"           },
                {ApplicationLog::MSG_REGISTRATION,          "MSG_REGISTRATION"         },
                {ApplicationLog::MSG_INIT,                  "MSG_INIT"                 },
                {ApplicationLog::MSG_REG_START,             "MSG_REG_START"            },
                {ApplicationLog::MSG_REG_UPDATE,            "MSG_REG_UPDATE"           },
                {ApplicationLog::MSG_REG_STOP,              "MSG_REG_STOP"             },
                {ApplicationLog::MSG_REG_RECONFIG,          "MSG_REG_RECONFIG"         },
                {ApplicationLog::MSG_REG_RECOVER,           "MSG_REG_RECOVER"          },
                {ApplicationLog::MSG_IPCAN_CHANGED,         "MSG_IPCAN_CHANGED"        },
                {ApplicationLog::MSG_PUB_TERMINATED,        "MSG_PUB_TERMINATED"       },
                {ApplicationLog::MSG_DESTROY,               "MSG_DESTROY"              },
                {ApplicationLog::MSG_IMS_EST_TIMER_CONTROL, "MSG_IMS_EST_TIMER_CONTROL"},
                {ApplicationLog::MSG_REG_EXCHANGE,          "MSG_REG_EXCHANGE"         },
                {ApplicationLog::MSG_AC_CONFIGURED,         "MSG_AC_CONFIGURED"        },
                {ApplicationLog::MSG_PCSCF_RECOVER,         "MSG_PCSCF_RECOVER"        },
                {ApplicationLog::MSG_SCSCF_RESTORATION,     "MSG_SCSCF_RESTORATION"    },
                {ApplicationLog::MSG_OTHERS,                "MSG_OTHERS"               },
                {ApplicationLog::MSG_OTHERS + 99,           "__INVALID__"              }
}));

using AosLogTestForAppPending = TestWithParam<AosLogParams>;

TEST_P(AosLogTestForAppPending, ReturnsValidStringForAppPending)
{
    // GIVEN
    const AosLogParams& objAosLogParams = GetParam();

    // WHEN
    IMS_UINT32 nValue = objAosLogParams.nValue;
    const IMS_CHAR* pszString = objAosLogParams.pszString;

    // THEN
    EXPECT_STREQ(AosProvider::GetLog()->AppPendingToString(nValue), pszString);
}

INSTANTIATE_TEST_SUITE_P(AosLogInstantiation, AosLogTestForAppPending,
        ValuesIn<AosLogParams>({
                {ApplicationLog::PENDING_REG_RECOVERY_HELD,       "PENDING_REG_RECOVERY_HELD"},
                {ApplicationLog::PENDING_REG_STOP_HELD,           "PENDING_REG_STOP_HELD"    },
                {ApplicationLog::PENDING_APP_DESTROY_HELD,        "PENDING_APP_DESTROY_HELD" },
                {ApplicationLog::PENDING_REG_RECONFIG_HELD,       "PENDING_REG_RECONFIG_HELD"},
                {ApplicationLog::PENDING_REG_AFTER_CSFB_COMPLETE,
                 "PENDING_REG_AFTER_CSFB_COMPLETE"                                           },
                {ApplicationLog::PENDING_IPCAN_HELD,              "PENDING_IPCAN_HELD"       },
                {ApplicationLog::PENDING_REG_UPDATE_HELD,         "PENDING_REG_UPDATE_HELD"  },
                {ApplicationLog::PENDING_NONE,                    "__INVALID__"              }
}));

using AosLogTestForAppRequest = TestWithParam<AosLogParams>;

TEST_P(AosLogTestForAppRequest, ReturnsValidStringForAppRequest)
{
    // GIVEN
    const AosLogParams& objAosLogParams = GetParam();

    // WHEN
    IMS_UINT32 nValue = objAosLogParams.nValue;
    const IMS_CHAR* pszString = objAosLogParams.pszString;

    // THEN
    EXPECT_STREQ(AosProvider::GetLog()->AppRequestToString(nValue), pszString);
}

INSTANTIATE_TEST_SUITE_P(AosLogInstantiation, AosLogTestForAppRequest,
        ValuesIn<AosLogParams>({
                {ImsAosControl::REGISTER_START,                         "REGISTER_START"                 },
                {ImsAosControl::REGISTER_START_WITH_WLAN,               "REGISTER_START_WITH_WLAN"       },
                {ImsAosControl::REGISTER_REFRESH,                       "REGISTER_REFRESH"               },
                {ImsAosControl::REGISTER_STOP,                          "REGISTER_STOP"                  },
                {ImsAosControl::REGISTER_REINITIATE,                    "REGISTER_REINITIATE"            },
                {ImsAosControl::E_REGISTER_FAKE_WITH_NEXT_PCSCF,        "E_REGISTER_FAKE_WITH_NEXT_PCSCF"},
                {ImsAosControl::PCSCF_NEXT,                             "PCSCF_NEXT"                     },
                {ImsAosControl::PCSCF_NEXT_WITH_DISCOVERY,              "PCSCF_NEXT_WITH_DISCOVERY"      },
                {ImsAosControl::IPSEC_DISABLED,                         "IPSEC_DISABLED"                 },
                {ImsAosControl::RETRY_COUNT_INCREASE,                   "RETRY_COUNT_INCREASE"           },
                {ImsAosControl::UPDATE_SIP_DELEGATE_REGISTRATION,
                 "UPDATE_SIP_DELEGATE_REGISTRATION"                                                      },
                {ImsAosControl::TRIGGER_SIP_DELEGATE_DEREGISTRATION,
                 "TRIGGER_SIP_DELEGATE_DEREGISTRATION"                                                   },
                {ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION,
                 "TRIGGER_FULL_NETWORK_REGISTRATION"                                                     },
                {ImsAosControl::TRIGGER_FULL_NETWORK_REGISTRATION + 99, "__INVALID__"                    }
}));

using AosLogTestForAppState = TestWithParam<AosLogParams>;

TEST_P(AosLogTestForAppState, ReturnsValidStringForAppState)
{
    // GIVEN
    const AosLogParams& objAosLogParams = GetParam();

    // WHEN
    IMS_UINT32 nValue = objAosLogParams.nValue;
    const IMS_CHAR* pszString = objAosLogParams.pszString;

    // THEN
    EXPECT_STREQ(AosProvider::GetLog()->AppStateToString(nValue), pszString);
}

INSTANTIATE_TEST_SUITE_P(AosLogInstantiation, AosLogTestForAppState,
        ValuesIn<AosLogParams>({
                {ApplicationLog::STATE_NOTREADY,           "STATE_NOTREADY"     },
                {ApplicationLog::STATE_READY,              "STATE_READY"        },
                {ApplicationLog::STATE_CONNECTING,         "STATE_CONNECTING"   },
                {ApplicationLog::STATE_CONNECTED,          "STATE_CONNECTED"    },
                {ApplicationLog::STATE_UPDATING,           "STATE_UPDATING"     },
                {ApplicationLog::STATE_DISCONNECTING,      "STATE_DISCONNECTING"},
                {ApplicationLog::STATE_DISCONNECTING + 99, "__INVALID__"        }
}));

TEST(AosLogTest, AppTimerToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_RECONFIG_GUARD),
            "TIMER_RECONFIG_GUARD");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_MSG_CONDITION),
            "TIMER_MSG_CONDITION");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_REG_STOP),
            "TIMER_REG_STOP");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_REG_BLOCKED),
            "TIMER_REG_BLOCKED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_APP_ACTIVATED),
            "TIMER_APP_ACTIVATED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_APP_CONNECTED),
            "TIMER_APP_CONNECTED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_APP_TERMINATED),
            "TIMER_APP_TERMINATED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_PDN_BLOCKED),
            "TIMER_PDN_BLOCKED");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_IMS_ESTABLISHMENT),
            "TIMER_IMS_ESTABLISHMENT");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_RAT_BLOCK),
            "TIMER_RAT_BLOCK");
    EXPECT_STREQ(AosProvider::GetLog()->AppTimerToString(ApplicationLog::TIMER_RAT_BLOCK + 99),
            "__INVALID__");
}

TEST(AosLogTest, AppConnectionStateToString)
{
    EXPECT_STREQ(
            AosProvider::GetLog()->AppConnectionStateToString(ApplicationLog::CONNECTION_ACTIVATED),
            "ACTIVATED");

    EXPECT_STREQ(AosProvider::GetLog()->AppConnectionStateToString(
                         ApplicationLog::CONNECTION_DEACTIVATED),
            "DEACTIVATED");

    EXPECT_STREQ(
            AosProvider::GetLog()->AppConnectionStateToString(ApplicationLog::CONNECTION_UPDATED),
            "UPDATED");

    EXPECT_STREQ(AosProvider::GetLog()->AppConnectionStateToString(
                         ApplicationLog::CONNECTION_UPDATED + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegMessageToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_START),
            "MSG_REG_START");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_REINITIATE),
            "MSG_REG_REINITIATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_UPDATE),
            "MSG_REG_UPDATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_RECONFIG),
            "MSG_REG_RECONFIG");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLog::MSG_REG_REQUIRED_WITH_WAIT_TIME),
            "MSG_REG_REQUIRED_WITH_WAIT_TIME");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLog::MSG_REG_REQUIRED_WITH_NEXT_PCSCF),
            "MSG_REG_REQUIRED_WITH_NEXT_PCSCF");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLog::MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION),
            "MSG_REG_REQUIRED_WITH_SCSCF_RESTORATION");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLog::MSG_REG_REINITIATE_WITH_REG_STATE),
            "MSG_REG_REINITIATE_WITH_REG_STATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLog::MSG_REG_TERMINATED_BY_NOTIFY),
            "MSG_REG_TERMINATED_BY_NOTIFY");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_FORBIDDEN_IN_WIFI),
            "MSG_REG_FORBIDDEN_IN_WIFI");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_PROCESS_GIBA),
            "MSG_REG_PROCESS_GIBA");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_RESTART),
            "MSG_REG_RESTART");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_SUB_REINITIATE),
            "MSG_SUB_REINITIATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_SUB_TERMINATED),
            "MSG_SUB_TERMINATED");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegMessageToString(RegistrationLog::MSG_REG_EVENT_REGISTERED),
            "MSG_REG_EVENT_REGISTERED");
    EXPECT_STREQ(AosProvider::GetLog()->RegMessageToString(
                         RegistrationLog::MSG_REG_EVENT_REGISTERED + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegModeToString)
{
    EXPECT_STREQ(
            AosProvider::GetLog()->RegModeToString(RegistrationLog::MODE_NORMAL), "MODE_NORMAL");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegModeToString(RegistrationLog::MODE_LIMITED), "MODE_LIMITED");
    EXPECT_STREQ(AosProvider::GetLog()->RegModeToString(RegistrationLog::MODE_FAKE), "MODE_FAKE");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegModeToString(RegistrationLog::MODE_FAKE + 99), "__INVALID__");
}

TEST(AosLogTest, RegPendingToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_NONE),
            "PENDING_NONE");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_START),
            "PENDING_START");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_TRANSACTION),
            "PENDING_TRANSACTION");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_UPDATE),
            "PENDING_UPDATE");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_RECONFIG),
            "PENDING_RECONFIG");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_UPDATE_HELD_BY_CALL),
            "PENDING_UPDATE_HELD_BY_CALL");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(
                         RegistrationLog::PENDING_PLMN_BLOCK_HELD_BY_CALL),
            "PENDING_PLMN_BLOCK_HELD_BY_CALL");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_SUBSCRIPTION),
            "PENDING_SUBSCRIPTION");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_TERMINATED),
            "PENDING_TERMINATED");
    EXPECT_STREQ(AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_TRAFFIC),
            "PENDING_TRAFFIC");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegPendingToString(RegistrationLog::PENDING_TERMINATED + 99),
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
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_OFFLINE),
            "STATE_OFFLINE");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_REGISTERING),
            "STATE_REGISTERING");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_REGSTOP),
            "STATE_REGSTOP");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_REGISTERED),
            "STATE_REGISTERED");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_REFRESHING),
            "STATE_REFRESHING");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_REFRESHSTOP),
            "STATE_REFRESHSTOP");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_DEREGISTERING),
            "STATE_DEREGISTERING");
    EXPECT_STREQ(AosProvider::GetLog()->RegStateToString(RegistrationLog::STATE_DEREGISTERING + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegTimerToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_OFFLINE_RECOVER),
            "TIMER_OFFLINE_RECOVER");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_STOP_RETRY),
            "TIMER_STOP_RETRY");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_REFRESH),
            "TIMER_REFRESH");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_DEREG_TRAFFIC),
            "TIMER_DEREG_TRAFFIC");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_MODE), "TIMER_MODE");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_TRANSACTION),
            "TIMER_TRANSACTION");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_INTERNAL_ERROR),
            "TIMER_INTERNAL_ERROR");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegTimerToString(RegistrationLog::TIMER_WAIT_EMERGENCY_NETWORK),
            "TIMER_WAIT_EMERGENCY_NETWORK");
    EXPECT_STREQ(AosProvider::GetLog()->RegTimerToString(
                         RegistrationLog::TIMER_WAIT_EMERGENCY_NETWORK + 99),
            "__INVALID__");
}

TEST(AosLogTest, RegResultToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->RegResultToString(RegistrationLog::RESULT_NONE), "NONE");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegResultToString(RegistrationLog::RESULT_SUCCESS), "SUCCESS");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegResultToString(RegistrationLog::RESULT_TRYING), "TRYING");
    EXPECT_STREQ(
            AosProvider::GetLog()->RegResultToString(RegistrationLog::RESULT_FAILURE), "FAILURE");
    EXPECT_STREQ(AosProvider::GetLog()->RegResultToString(RegistrationLog::RESULT_FAILURE + 99),
            "__INVALID__");
}

TEST(AosLogTest, ConnectorReasonToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_NONE),
            "REASON_NONE");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_DISCONNECTED),
            "REASON_DISCONNECTED");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_FAILED),
            "REASON_FAILED");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(
                         ConnectorLog::REASON_PCSCF_DISCOVERY_FAILED),
            "REASON_PCSCF_DISCOVERY_FAILED");
    EXPECT_STREQ(
            AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_PERMANENTLY_FAILED),
            "REASON_PERMANENTLY_FAILED");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(
                         ConnectorLog::REASON_LIMITED_SERVICE_PCO),
            "REASON_LIMITED_SERVICE_PCO");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_IP_CHANGED),
            "REASON_IP_CHANGED");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_PCSCF_CHANGED),
            "REASON_PCSCF_CHANGED");
    EXPECT_STREQ(
            AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_IPCAN_CAT_CHANGED),
            "REASON_IPCAN_CAT_CHANGED");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_OTHERS),
            "REASON_OTHERS");
    EXPECT_STREQ(AosProvider::GetLog()->ConnectorReasonToString(ConnectorLog::REASON_OTHERS + 99),
            "__INVALID__");
}

TEST(AosLogTest, AosReasonToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::NONE), "NONE");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::POWER_OFF), "POWER_OFF");
    EXPECT_STREQ(
            AosProvider::GetLog()->AosReasonToString(AosReason::AIRPLANE_MODE), "AIRPLANE_MODE");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::IMS_DISABLED), "IMS_DISABLED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::TTYMODEON), "TTY_MODE_ON");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::WIFI_OFF), "WIFI_OFF");
    EXPECT_STREQ(
            AosProvider::GetLog()->AosReasonToString(AosReason::SERVICE_POLICY), "SERVICE_POLICY");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::VOPS_NOT_SUPPORTED),
            "VOPS_NOT_SUPPORTED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::SSAC_BARRED), "SSAC_BARRED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::IP_CHANGED), "IP_CHANGED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::DATA_DISCONNECTED),
            "DATA_DISCONNECTED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::DATA_CONNECTION_MAINTAIN),
            "DATA_CONNECTION_MAINTAIN");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::DATA_PERMANENTLY_FAILED),
            "DATA_PERMANENTLY_FAILED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::NETWORK_ATTACH_REJECTED),
            "NETWORK_ATTACH_REJECTED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::REG_FAILURE), "REG_FAILURE");
    EXPECT_STREQ(
            AosProvider::GetLog()->AosReasonToString(AosReason::REG_TERMINATED), "REG_TERMINATED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::REG_TERMINATING),
            "REG_TERMINATING");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::INITIAL_REG_REQUESTED),
            "INITIAL_REG_REQUESTED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(AosReason::REG_ALL_PCSCF_FAILED),
            "REG_ALL_PCSCF_FAILED");
    EXPECT_STREQ(
            AosProvider::GetLog()->AosReasonToString(AosReason::NOT_SPECIFIED), "NOT_SPECIFIED");
    EXPECT_STREQ(AosProvider::GetLog()->AosReasonToString(99999), "UNKNOWN");
}

TEST(AosLogTest, AosRegistrationTypeToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->AosRegistrationTypeToString(AosRegistrationType::NORMAL),
            "NORMAL");

    EXPECT_STREQ(AosProvider::GetLog()->AosRegistrationTypeToString(AosRegistrationType::EMERGENCY),
            "EMERGENCY");

    EXPECT_STREQ(
            AosProvider::GetLog()->AosRegistrationTypeToString(AosRegistrationType::FAKE), "FAKE");

    EXPECT_STREQ(
            AosProvider::GetLog()->AosRegistrationTypeToString(AosRegistrationType::RCS), "RCS");

    EXPECT_STREQ(AosProvider::GetLog()->AosRegistrationTypeToString(
                         static_cast<AosRegistrationType>(999)),
            "UNKNOWN");
}

TEST(AosLogTest, EventToString)
{
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_ROAMING_STATE),
            "IMS_EVENT_ROAMING_STATE");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_IMS_VOICE_OVER_PS_STATE),
            "IMS_EVENT_IMS_VOICE_OVER_PS_STATE");
    EXPECT_STREQ(
            AosProvider::GetLog()->EventToString(IMS_EVENT_CSCALL_STATE), "IMS_EVENT_CSCALL_STATE");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_WFC_SETTING_CHANGED),
            "IMS_EVENT_WFC_SETTING_CHANGED");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_VOLTE_SETTING),
            "IMS_EVENT_VOLTE_SETTING");
    EXPECT_STREQ(AosProvider::GetLog()->EventToString(IMS_EVENT_VOLTE_SETTING + 99), "__INVALID__");
}