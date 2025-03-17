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
    EXPECT_EQ(MtcCallStringUtils::ConvertCallType(eCallType), "unknown");

    eCallType = CallType::VOIP;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallType(eCallType), "voip");

    eCallType = CallType::VT;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallType(eCallType), "vt");

    eCallType = CallType::RTT;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallType(eCallType), "rtt");

    eCallType = CallType::VIDEO_RTT;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallType(eCallType), "video_rtt");
}

TEST_F(MtcCallStringUtilsTest, ConvertMediaType)
{
    IMS_UINT32 eMediaType = MEDIATYPE_NONE;
    EXPECT_EQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "none");

    eMediaType = MEDIATYPE_AUDIO;
    EXPECT_EQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "audio");

    eMediaType = MEDIATYPE_VIDEO;
    EXPECT_EQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "video");

    eMediaType = MEDIATYPE_TEXT;
    EXPECT_EQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "text");

    eMediaType = MEDIATYPE_NONE - 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "OUT_OF_RANGE");

    eMediaType = MEDIATYPE_TEXT + 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertMediaType(eMediaType), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertUpdateType)
{
    UpdateType eUpdateType = UpdateType::NONE;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "none");

    eUpdateType = UpdateType::NORMAL;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "normal");

    eUpdateType = UpdateType::HOLD;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "hold");

    eUpdateType = UpdateType::RESUME;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "resume");

    eUpdateType = UpdateType::SESSION;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "session");

    eUpdateType = UpdateType::CONF;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "conf");

    eUpdateType = UpdateType::REFRESH;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "refresh");

    eUpdateType = UpdateType::SRVCC_RECOVERED_CANCEL;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "srvcc recovered cancel");

    eUpdateType = UpdateType::SRVCC_RECOVERED_FAILURE;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "srvcc recovered failure");

    eUpdateType = UpdateType::LOCATION;
    EXPECT_EQ(MtcCallStringUtils::ConvertUpdateType(eUpdateType), "location");
}

TEST_F(MtcCallStringUtilsTest, ConvertTimerType)
{
    IMS_SINT32 nTimerType = MtcCallState::TIMER_MO_CALL_INITIATION_TO_18X_WAIT;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType),
            "TIMER_MO_CALL_INITIATION_TO_18X_WAIT");

    nTimerType = MtcCallState::TIMER_MO_18X_WAIT;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MO_18X_WAIT");

    nTimerType = MtcCallState::TIMER_MO_NOANSWER;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MO_NOANSWER");

    nTimerType = MtcCallState::TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType),
            "TIMER_MO_RESPONSE_TIMEOUT_FOR_REASON");

    nTimerType = MtcCallState::TIMER_MT_ALERTING;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MT_ALERTING");

    nTimerType = MtcCallState::TIMER_MT_PRACK_WAIT;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_MT_PRACK_WAIT");

    nTimerType = MtcCallState::TIMER_RETRY_UPDATE;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_RETRY_UPDATE");

    nTimerType = MtcCallState::TIMER_CONVERT_USER_RESPONSE;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_CONVERT_USER_RESPONSE");

    nTimerType = MtcCallState::TIMER_CONVERT_REMOTE_RESPONSE;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_CONVERT_REMOTE_RESPONSE");

    nTimerType = MtcCallState::TIMER_E911_WAIT_SESSION_RELEASED;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_E911_WAIT_SESSION_RELEASED");

    nTimerType = MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED;
    EXPECT_EQ(
            MtcCallStringUtils::ConvertTimerType(nTimerType), "TIMER_DELAY_UPDATE_AFTER_CONNECTED");

    nTimerType = MtcCallState::TIMER_MO_CALL_INITIATION_TO_18X_WAIT - 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "OUT_OF_RANGE");

    nTimerType = MtcCallState::TIMER_DELAY_UPDATE_AFTER_CONNECTED + 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertTimerType(nTimerType), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertBlockStatus)
{
    IMtcBlockChecker::Result::Status eStatus = IMtcBlockRule::Result::Status::UNBLOCKED;
    EXPECT_EQ(MtcCallStringUtils::ConvertBlockStatus(eStatus), "unblocked");

    eStatus = IMtcBlockRule::Result::Status::BLOCKED;
    EXPECT_EQ(MtcCallStringUtils::ConvertBlockStatus(eStatus), "blocked");

    eStatus = IMtcBlockRule::Result::Status::PENDING;
    EXPECT_EQ(MtcCallStringUtils::ConvertBlockStatus(eStatus), "pending");
}

TEST_F(MtcCallStringUtilsTest, ConvertIpcanType)
{
    IMS_UINT32 eIpcan = IIpcan::CATEGORY_MOBILE;
    EXPECT_EQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "mobile");

    eIpcan = IIpcan::CATEGORY_WLAN;
    EXPECT_EQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "wlan");

    eIpcan = IIpcan::CATEGORY_ANY;
    EXPECT_EQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "any");

    eIpcan = IIpcan::CATEGORY_ANY + 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertIpcanType(eIpcan), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertAosState)
{
    MtcAosState eState = MtcAosState::CONNECTED;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosState(eState), "connected");

    eState = MtcAosState::SUSPENDED;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosState(eState), "suspended");

    eState = MtcAosState::DISCONNECTING;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosState(eState), "disconnecting");

    eState = MtcAosState::DISCONNECTED;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosState(eState), "disconnected");
}

TEST_F(MtcCallStringUtilsTest, ConvertRatType)
{
    IMS_SINT32 eRatType = INetworkWatcher::RADIOTECH_TYPE_INVALID;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "invalid");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_UNKNOWN;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "unknown");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_GPRS;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "GPRS");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EDGE;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "EDGE");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_UMTS;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "UMTS");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_CDMA;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "CDMA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EVDO_0;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "EVDO_0");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EVDO_A;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "EVDO_A");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_1xRTT;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "1xRTT");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSDPA;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSDPA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSUPA;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSUPA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSPA;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSPA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_IDEN;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "IDEN");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EVDO_B;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "EVDO_B");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_LTE;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "LTE");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_EHRPD;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "EHRPD");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_HSPAP;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "HSPAP");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_TD_SCDMA;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "TD_SCDMA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_IWLAN;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "IWLAN");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_LTE_CA;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "LTE_CA");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_NR;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "NR");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_MAX;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "invalid max");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_INVALID - 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "OUT_OF_RANGE");

    eRatType = INetworkWatcher::RADIOTECH_TYPE_MAX + 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertRatType(eRatType), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertFailureReason)
{
    IMS_UINT32 nFailureReason = IImsRadio::REASON_ACCESS_DENIED;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "Access denied");

    nFailureReason = IImsRadio::REASON_NAS_FAILURE;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "NAS failure");

    nFailureReason = IImsRadio::REASON_RACH_FAILURE;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RACH failure");

    nFailureReason = IImsRadio::REASON_RLC_FAILURE;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RLC failure");

    nFailureReason = IImsRadio::REASON_RRC_REJECT;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RRC reject");

    nFailureReason = IImsRadio::REASON_RRC_TIMEOUT;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RRC timeout");

    nFailureReason = IImsRadio::REASON_NO_SERVICE;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "No service");

    nFailureReason = IImsRadio::REASON_PDN_NOT_AVAILABLE;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "PDN not available");

    nFailureReason = IImsRadio::REASON_RF_BUSY;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "RF busy");

    nFailureReason = IImsRadio::REASON_INTERNAL_ERROR;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "Internal error");

    nFailureReason = IImsRadio::REASON_ACCESS_DENIED - 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "OUT_OF_RANGE");

    nFailureReason = IImsRadio::REASON_INTERNAL_ERROR + 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertFailureReason(nFailureReason), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertSrvccState)
{
    SrvccState eState = SrvccState::IDLE;
    EXPECT_EQ(MtcCallStringUtils::ConvertSrvccState(eState), "IDLE");

    eState = SrvccState::STARTED;
    EXPECT_EQ(MtcCallStringUtils::ConvertSrvccState(eState), "STARTED");

    eState = SrvccState::SUCCEEDED;
    EXPECT_EQ(MtcCallStringUtils::ConvertSrvccState(eState), "SUCCEEDED");

    eState = SrvccState::FAILED;
    EXPECT_EQ(MtcCallStringUtils::ConvertSrvccState(eState), "FAILED");

    eState = SrvccState::CANCELED;
    EXPECT_EQ(MtcCallStringUtils::ConvertSrvccState(eState), "CANCELED");
}

TEST_F(MtcCallStringUtilsTest, ConvertAosReason)
{
    IMS_UINT32 eAosReason = ImsAosReason::NONE;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "none");

    eAosReason = ImsAosReason::POWER_OFF;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "power off");

    eAosReason = ImsAosReason::SERVICE_POLICY;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "service policy");

    eAosReason = ImsAosReason::DATA_DISCONNECTED;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "data disconnected");

    eAosReason = ImsAosReason::REG_TERMINATED;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "registration terminated");

    eAosReason = ImsAosReason::REG_NEW_REQUIRED;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "new registration required");

    eAosReason = ImsAosReason::REG_TERMINATING;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "registration terminating");

    eAosReason = ImsAosReason::NOT_SPECIFIED;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "not specified");

    eAosReason = ImsAosReason::POWER_OFF - 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "OUT_OF_RANGE");

    eAosReason = ImsAosReason::NOT_SPECIFIED + 1;
    EXPECT_EQ(MtcCallStringUtils::ConvertAosReason(eAosReason), "OUT_OF_RANGE");
}

TEST_F(MtcCallStringUtilsTest, ConvertCallState)
{
    CallStateName eState = CallStateName::IDLE;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallState(eState), "IDLE");

    eState = CallStateName::OUTGOING;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallState(eState), "OUTGOING");

    eState = CallStateName::INCOMING;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallState(eState), "INCOMING");

    eState = CallStateName::ALERTING;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallState(eState), "ALERTING");

    eState = CallStateName::ESTABLISHED;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallState(eState), "ESTABLISHED");

    eState = CallStateName::UPDATING;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallState(eState), "UPDATING");

    eState = CallStateName::TERMINATING;
    EXPECT_EQ(MtcCallStringUtils::ConvertCallState(eState), "TERMINATING");
}

}  // namespace android
