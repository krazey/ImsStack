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

#include "ImsEventDef.h"
#include "MockIMtcContext.h"
#include "MockIMtcImsEventReceiver.h"
#include "MockIMtcCallController.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "call/MockIMtcSession.h"
#include "call/MtcCallController.h"
#include "call/RttAutoUpgrader.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIPassiveTimerHolder.h"
#include "media/MockIMtcMediaManager.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

LOCAL CallKey CALL_KEY = 1;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

class RttAutoUpgraderTest : public ::testing::Test
{
public:
    CallInfo objCallInfo;
    RttAutoUpgrader* pRttAutoUpgrader;
    MediaInfo objMediaInfo;
    MockICallStateProxy objCallStateProxy;
    MockIMtcCall objMtcCall;
    MockIMtcCallContext objCallContext;
    MockIMtcCallController objController;
    MockIMtcCallManager objCallManager;
    MockIMtcContext objContext;
    MockIMtcImsEventReceiver objEventReceiver;
    MockIMtcMediaManager objMediaManager;
    // cppcheck-suppress unusedStructMember
    MockIMtcSession objMtcCallSession;
    MockIPassiveTimerHolder objPassiveTimer;
    MockMtcConfigurationProxy objConfigurationProxy;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objConfigurationProxy,
                GetInt(ConfigEmergency::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT))
                .WillByDefault(Return(180000));

        ON_CALL(objMediaManager, GetMediaInfo).WillByDefault(ReturnRef(objMediaInfo));
        ON_CALL(objCallContext, GetMediaManager).WillByDefault(ReturnRef(objMediaManager));
        ON_CALL(objMtcCall, GetCallContext).WillByDefault(ReturnRef(objCallContext));
        ON_CALL(objCallManager, GetCallByCallKey(CALL_KEY)).WillByDefault(Return(&objMtcCall));
        ON_CALL(objContext, GetCallController).WillByDefault(ReturnRef(objController));
        ON_CALL(objContext, GetCallManager).WillByDefault(ReturnRef(objCallManager));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));
        ON_CALL(objContext, GetConfigurationProxy).WillByDefault(ReturnRef(objConfigurationProxy));
        ON_CALL(objContext, GetImsEventReceiver).WillByDefault(ReturnRef(objEventReceiver));
        ON_CALL(objContext, GetPassiveTimerHolder).WillByDefault(ReturnRef(objPassiveTimer));

        pRttAutoUpgrader = new RttAutoUpgrader(objContext);
    }

    virtual void TearDown() override { delete pRttAutoUpgrader; }
};

TEST_F(RttAutoUpgraderTest, IsRequiredByConfigurationTimer)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    ON_CALL(objMtcCallSession, GetCallType()).WillByDefault(Return(CallType::RTT));
    EXPECT_CALL(objConfigurationProxy,
            GetInt(ConfigEmergency::KEY_EMERGENCY_RTT_GUARD_TIMER_MILLIS_INT))
            .WillOnce(Return(0))
            .WillOnce(Return(180000));

    EXPECT_FALSE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
    EXPECT_TRUE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
}

TEST_F(RttAutoUpgraderTest, IsRequiredByIsEmergency)
{
    ON_CALL(objMtcCallSession, GetCallType()).WillByDefault(Return(CallType::RTT));

    objCallInfo.eEmergencyType = EmergencyType::NONE;
    EXPECT_FALSE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));

    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    EXPECT_TRUE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));

    objCallInfo.eEmergencyType = EmergencyType::NORMAL_ROUTING;
    EXPECT_FALSE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
}

TEST_F(RttAutoUpgraderTest, IsRequiredByCallType)
{
    objCallInfo.eEmergencyType = EmergencyType::EMERGENCY_ROUTING;
    EXPECT_CALL(objMtcCallSession, GetCallType())
            .WillOnce(Return(CallType::UNKNOWN))
            .WillOnce(Return(CallType::VOIP))
            .WillOnce(Return(CallType::VT))
            .WillOnce(Return(CallType::RTT))
            .WillOnce(Return(CallType::VIDEO_RTT));

    EXPECT_FALSE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
    EXPECT_FALSE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
    EXPECT_FALSE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
    EXPECT_TRUE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
    EXPECT_TRUE(
            pRttAutoUpgrader->IsRequired(objConfigurationProxy, objCallInfo, &objMtcCallSession));
}

TEST_F(RttAutoUpgraderTest,
        StartRttGuardTimerAfterRttEmergencyCallIsTerminatedWithoutBeingNoticedEstablished)
{
    EXPECT_CALL(objPassiveTimer, AddTimer(_, _, _, _)).Times(1);
    EXPECT_CALL(objPassiveTimer, AddListener(_, _)).Times(1);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);
}

TEST_F(RttAutoUpgraderTest, StartRttGuardTimerAfterRttEmergencyCallIsTerminated)
{
    EXPECT_CALL(objPassiveTimer, AddTimer(_, _, _, _)).Times(1);
    EXPECT_CALL(objPassiveTimer, AddListener(_, _)).Times(1);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::RTT, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);
}

TEST_F(RttAutoUpgraderTest, StartRttGuardTimerAgainAtSecondRttEmergencyCallIsTerminated)
{
    EXPECT_CALL(objPassiveTimer, AddTimer(_, _, _, _)).Times(2);
    EXPECT_CALL(objPassiveTimer, AddListener(_, _)).Times(2);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::RTT, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);
}

TEST_F(RttAutoUpgraderTest,
        NotStartRttGuardTimerAfterSecondRttEmergencyCallTerminatingWithoutEstablishing)
{
    // Set m_bRttEmergencyCallEstablished to false
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);

    EXPECT_CALL(objPassiveTimer, AddTimer(_, _, _, _)).Times(0);
    EXPECT_CALL(objPassiveTimer, AddListener(_, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::OUTGOING, CallType::RTT, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::RTT, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ALERTING, CallType::RTT, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::UPDATING, CallType::RTT, IMS_TRUE, 0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);
}

TEST_F(RttAutoUpgraderTest, NotStartRttGuardTimerAfterNonEmergencyCallTerminating)
{
    // Set m_bRttEmergencyCallEstablished to false
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);

    EXPECT_CALL(objPassiveTimer, AddTimer(_, _, _, _)).Times(0);
    EXPECT_CALL(objPassiveTimer, AddListener(_, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::RTT, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest, NotStartRttGuardTimerAfterNonRttCallTerminating)
{
    // Set m_bRttEmergencyCallEstablished to false
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);

    EXPECT_CALL(objPassiveTimer, AddTimer(_, _, _, _)).Times(0);
    EXPECT_CALL(objPassiveTimer, AddListener(_, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::UNKNOWN, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_TRUE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VT, IMS_TRUE, 0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::TERMINATING, CallType::RTT, IMS_TRUE, 0);
}

TEST_F(RttAutoUpgraderTest, UpgradeToRttWhenRttGuardTimerIsActiveAndRttSettingIsAlwaysVisible)
{
    ON_CALL(objPassiveTimer, IsActive).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_RTT_SETTING))
            .WillByDefault(Return(IMS_RTT_ALWAYS_VISIBLE));

    EXPECT_CALL(objController, Update(_, _, _)).Times(1);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::VOIP, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest, UpgradeToRttWhenRttGuardTimerIsActiveAndRttSettingIsVisibleDuringCall)
{
    ON_CALL(objPassiveTimer, IsActive).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_RTT_SETTING))
            .WillByDefault(Return(IMS_RTT_VISIBLE_DURING_CALL));

    EXPECT_CALL(objController, Update(_, _, _)).Times(1);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::VOIP, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest, NotUpgradeToRttWhenRttGuardTimerIsNotActiveAndRttSettingIsAlwaysVisible)
{
    ON_CALL(objPassiveTimer, IsActive).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_RTT_SETTING))
            .WillByDefault(Return(IMS_RTT_ALWAYS_VISIBLE));

    EXPECT_CALL(objController, Update(_, _, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::VOIP, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest,
        NotUpgradeToRttWhenRttGuardTimerIsNotActiveAndRttSettingIsVisibleDuringCall)
{
    ON_CALL(objPassiveTimer, IsActive).WillByDefault(Return(IMS_FALSE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_RTT_SETTING))
            .WillByDefault(Return(IMS_RTT_VISIBLE_DURING_CALL));

    EXPECT_CALL(objController, Update(_, _, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::VOIP, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest, NotUpgradeToRttWhenRttSettingIsWrong)
{
    ON_CALL(objPassiveTimer, IsActive).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_RTT_SETTING))
            .WillByDefault(Return(IMS_RTT_NO_VISIBLE));

    EXPECT_CALL(objController, Update(_, _, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::VOIP, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest, NotUpgradeToRttWhenCallKeyIsNotMathedAndRttSettingIsAlwaysVisible)
{
    ON_CALL(objPassiveTimer, IsActive).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_RTT_SETTING))
            .WillByDefault(Return(IMS_RTT_ALWAYS_VISIBLE));

    EXPECT_CALL(objController, Update(_, _, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::VOIP, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY + 1, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest, NotUpgradeToRttWhenCallKeyIsNotMathedAndRttSettingIsVisibleDuringCall)
{
    ON_CALL(objPassiveTimer, IsActive).WillByDefault(Return(IMS_TRUE));
    ON_CALL(objEventReceiver, GetWParam(IMS_EVENT_RTT_SETTING))
            .WillByDefault(Return(IMS_RTT_VISIBLE_DURING_CALL));

    EXPECT_CALL(objController, Update(_, _, _)).Times(0);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::INCOMING, CallType::VOIP, IMS_FALSE, 0);
    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY + 1, IMtcCall::State::ESTABLISHED, CallType::VOIP, IMS_FALSE, 0);
}

TEST_F(RttAutoUpgraderTest, DestroyRttAutoUpgraderWhenOnPassiveTimerExpired)
{
    EXPECT_CALL(objContext, DestroyRttAutoUpgrader).Times(1);
    pRttAutoUpgrader->OnPassiveTimerExpired(IPassiveTimerHolder::Type::RTT_AUTO_UPGRADE_GUARD);
}

TEST_F(RttAutoUpgraderTest, RemoveRttGuardTimerWhenRttEmergencyCallIsEstablished)
{
    EXPECT_CALL(objPassiveTimer, RemoveListener(_, _)).Times(1);
    EXPECT_CALL(objPassiveTimer, RemoveTimer(_)).Times(1);

    pRttAutoUpgrader->OnCallStateChanged(
            CALL_KEY, IMtcCall::State::ESTABLISHED, CallType::RTT, IMS_TRUE, 0);

    EXPECT_CALL(objPassiveTimer, RemoveListener(_, _)).Times(1);  // destructor RttAutoUpgrader
}
