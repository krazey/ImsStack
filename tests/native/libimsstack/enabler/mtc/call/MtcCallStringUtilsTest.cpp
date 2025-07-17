/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "call/MtcCallStringUtils.h"

#include <gtest/gtest.h>

namespace android
{

class MtcCallStringUtilsTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(MtcCallStringUtilsTest, ConvertCallType)
{
    CallType eCallType = CallType::UNKNOWN;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallType(eCallType), "unknown");

    eCallType = CallType::VOIP;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallType(eCallType), "voip");

    eCallType = CallType::VT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallType(eCallType), "vt");

    eCallType = CallType::RTT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallType(eCallType), "rtt");

    eCallType = CallType::VIDEO_RTT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallType(eCallType), "video_rtt");
}

TEST_F(MtcCallStringUtilsTest, ConvertMediaType)
{
    IMS_UINT32 eMediaType = MEDIATYPE_NONE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "none");

    eMediaType = MEDIATYPE_AUDIO;
    EXPECT_STREQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "audio");

    eMediaType = MEDIATYPE_VIDEO;
    EXPECT_STREQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "video");

    eMediaType = MEDIATYPE_TEXT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "text");

    eMediaType = MEDIATYPE_NONE - 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "OUT_OF_RANGE");

    eMediaType = MEDIATYPE_TEXT + 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertUpdateType)
{
    UpdateType eUpdateType = UpdateType::NONE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "none");

    eUpdateType = UpdateType::NORMAL;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "normal");

    eUpdateType = UpdateType::HOLD;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "hold");

    eUpdateType = UpdateType::RESUME;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "resume");

    eUpdateType = UpdateType::SESSION;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "session");

    eUpdateType = UpdateType::CONF;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "conf");

    eUpdateType = UpdateType::REFRESH;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "refresh");

    eUpdateType = UpdateType::SRVCC_RECOVERED_CANCEL;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "srvcc recovered cancel");

    eUpdateType = UpdateType::SRVCC_RECOVERED_FAILURE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "srvcc recovered failure");

    eUpdateType = UpdateType::LOCATION;
    EXPECT_STREQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "location");
}

TEST_F(MtcCallStringUtilsTest, ConvertTimerType)
{
    IMS_SINT32 nTimerType = MtcCallState::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType),
            "TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL");

    nTimerType = MtcCallState::TIMER_MO_CALL_INITIATION_TO_18X_WAIT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType),
            "TIMER_MO_CALL_INITIATION_TO_18X_WAIT");

    nTimerType = MtcCallState::TIMER_MO_18X_WAIT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MO_18X_WAIT");

    nTimerType = MtcCallState::TIMER_MO_NOANSWER;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MO_NOANSWER");

    nTimerType = MtcCallState::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType),
            "TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON");

    nTimerType = MtcCallState::TIMER_MT_ALERTING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MT_ALERTING");

    nTimerType = MtcCallState::TIMER_MT_PRACK_WAIT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MT_PRACK_WAIT");

    nTimerType = MtcCallState::TIMER_RETRY_UPDATE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_RETRY_UPDATE");

    nTimerType = MtcCallState::TIMER_CONVERT_USER_RESPONSE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_CONVERT_USER_RESPONSE");

    nTimerType = MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_CONVERT_REMOTE_RESPONSE");

    nTimerType = MtcCallState::TIMER_E911_WAIT_SESSION_RELEASED;
    EXPECT_STREQ(
            MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_E911_WAIT_SESSION_RELEASED");

    nTimerType = MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED;
    EXPECT_STREQ(
            MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_DELAY_UPDATE_AFTER_CONNECTED");

    nTimerType = MtcCallState::TIMER_MO_REGISTRATION_FOR_SILENT_REDIAL - 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "OUT_OF_RANGE");

    nTimerType = MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED + 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertBlockStatus)
{
    IMtcBlockChecker::Result::Status eStatus = IMtcBlockRule::Result::Status::UNBLOCKED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertBlockStatus(eStatus), "unblocked");

    eStatus = IMtcBlockRule::Result::Status::BLOCKED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertBlockStatus(eStatus), "blocked");

    eStatus = IMtcBlockRule::Result::Status::PENDING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertBlockStatus(eStatus), "pending");
}

TEST_F(MtcCallStringUtilsTest, ConvertIpcanType)
{
    IMS_UINT32 eIpcan = IIpcan::CATEGORY_MOBILE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "mobile");

    eIpcan = IIpcan::CATEGORY_WLAN;
    EXPECT_STREQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "wlan");

    eIpcan = IIpcan::CATEGORY_ANY;
    EXPECT_STREQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "any");

    eIpcan = IIpcan::CATEGORY_ANY + 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertAosState)
{
    MtcAosState eState = MtcAosState::CONNECTED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosState(eState), "connected");

    eState = MtcAosState::SUSPENDED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosState(eState), "suspended");

    eState = MtcAosState::DISCONNECTING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosState(eState), "disconnecting");

    eState = MtcAosState::DISCONNECTED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosState(eState), "disconnected");
}

TEST_F(MtcCallStringUtilsTest, ConvertRatType)
{
    IMS_SINT32 eRatType = INetworkWatcher::RADIOTECH_TYPE_INVALID;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "invalid");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_UNKNOWN;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "unknown");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_GPRS;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "GPRS");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EDGE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "EDGE");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_UMTS;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "UMTS");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_CDMA;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "CDMA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EVDO_0;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "EVDO_0");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EVDO_A;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "EVDO_A");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_1xRTT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "1xRTT");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSDPA;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSDPA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSUPA;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSUPA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSPA;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSPA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_IDEN;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "IDEN");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EVDO_B;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "EVDO_B");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_LTE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "LTE");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EHRPD;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "EHRPD");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSPAP;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSPAP");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_TD_SCDMA;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "TD_SCDMA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_IWLAN;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "IWLAN");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_LTE_CA;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "LTE_CA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_NR;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "NR");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_MAX;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "invalid max");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_INVALID - 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "OUT_OF_RANGE");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_MAX + 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertRatType(eRatType), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertFailureReason)
{
    IMS_UINT32 nFailureReason = IImsRadio::REASON_ACCESS_DENIED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "Access denied");

    nFailureReason = IImsRadio::REASON_NAS_FAILURE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "NAS failure");

    nFailureReason = IImsRadio::REASON_RACH_FAILURE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RACH failure");

    nFailureReason = IImsRadio::REASON_RLC_FAILURE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RLC failure");

    nFailureReason = IImsRadio::REASON_RRC_REJECT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RRC reject");

    nFailureReason = IImsRadio::REASON_RRC_TIMEOUT;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RRC timeout");

    nFailureReason = IImsRadio::REASON_NO_SERVICE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "No service");

    nFailureReason = IImsRadio::REASON_PDN_NOT_AVAILABLE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "PDN not available");

    nFailureReason = IImsRadio::REASON_RF_BUSY;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RF busy");

    nFailureReason = IImsRadio::REASON_INTERNAL_ERROR;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "Internal error");

    // cppcheck-suppress duplicateExpression
    nFailureReason = IImsRadio::REASON_ACCESS_DENIED - 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "OUT_OF_RANGE");

    nFailureReason = IImsRadio::REASON_INTERNAL_ERROR + 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertSrvccState)
{
    SrvccState eState = SrvccState::IDLE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertSrvccState(eState), "IDLE");

    eState = SrvccState::STARTED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertSrvccState(eState), "STARTED");

    eState = SrvccState::SUCCEEDED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertSrvccState(eState), "SUCCEEDED");

    eState = SrvccState::FAILED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertSrvccState(eState), "FAILED");

    eState = SrvccState::CANCELED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertSrvccState(eState), "CANCELED");
}

TEST_F(MtcCallStringUtilsTest, ConvertAosReason)
{
    IMS_UINT32 eAosReason = ImsAosReason::NONE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "none");

    eAosReason = ImsAosReason::POWER_OFF;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "power off");

    eAosReason = ImsAosReason::SERVICE_POLICY;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "service policy");

    eAosReason = ImsAosReason::DATA_DISCONNECTED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "data disconnected");

    eAosReason = ImsAosReason::DATA_PERMANENTLY_FAILED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "data permanently failed");

    eAosReason = ImsAosReason::REG_TERMINATED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "registration terminated");

    eAosReason = ImsAosReason::REG_NEW_REQUIRED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "new registration required");

    eAosReason = ImsAosReason::REG_TERMINATING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "registration terminating");

    eAosReason = ImsAosReason::REG_ALL_PCSCF_FAILED;
    EXPECT_STREQ(
            MtcCallStringUtils::ConvertAosReason(eAosReason), "registration failed for all pcscfs");

    eAosReason = ImsAosReason::IP_CHANGED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "ip changed");

    eAosReason = ImsAosReason::NETWORK_ATTACH_REJECTED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "network attach rejected");

    eAosReason = ImsAosReason::NOT_SPECIFIED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "not specified");

    eAosReason = ImsAosReason::POWER_OFF - 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "OUT_OF_RANGE");

    eAosReason = ImsAosReason::NOT_SPECIFIED + 1;
    EXPECT_STREQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertCallState)
{
    CallStateName eState = CallStateName::IDLE;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallState(eState), "IDLE");

    eState = CallStateName::OUTGOING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallState(eState), "OUTGOING");

    eState = CallStateName::INCOMING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallState(eState), "INCOMING");

    eState = CallStateName::ALERTING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallState(eState), "ALERTING");

    eState = CallStateName::ESTABLISHED;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallState(eState), "ESTABLISHED");

    eState = CallStateName::UPDATING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallState(eState), "UPDATING");

    eState = CallStateName::TERMINATING;
    EXPECT_STREQ(MtcCallStringUtils::ConvertCallState(eState), "TERMINATING");
}

}  // namespace android
