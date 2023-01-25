/*
 * Copyright (C) 2023 The Android Open Source Project
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

#include "IMtcCallStateListener.h"
#include "IMtcService.h"
#include "ImsAosParameter.h"
#include "ImsAosReason.h"
#include "ImsTypeDef.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "MtcEmergencyServiceManager.h"
#include "configuration/MockIMtcConfigurationManager.h"
#include "configuration/MtcConfigurationProxy.h"
#include "helper/EmergencyNormalRoutingHelper.h"
#include "helper/MockICallStateProxy.h"
#include "helper/MockIEmergencyNormalRoutingHelperListener.h"
#include <gtest/gtest.h>
#include <vector>

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class EmergencyNormalRoutingHelperTest : public ::testing::Test
{
public:
    inline explicit EmergencyNormalRoutingHelperTest() :
            objContext(),
            objService(),
            objCallStateProxy(),
            objServiceThread(),
            objListener(),
            pRoutingHelper(IMS_NULL)
    {
    }

    inline virtual ~EmergencyNormalRoutingHelperTest() {}

protected:
    MockIMtcContext objContext;
    MockIMtcService objService;
    MockICallStateProxy objCallStateProxy;
    MockIJniMtcServiceThread objServiceThread;
    MockIEmergencyNormalRoutingHelperListener objListener;
    EmergencyNormalRoutingHelper* pRoutingHelper;

    inline void SetUp() override
    {
        ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL))
                .WillByDefault(Return(&objService));
        ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(&objServiceThread));
        ON_CALL(objContext, GetCallStateProxy).WillByDefault(ReturnRef(objCallStateProxy));

        pRoutingHelper = new EmergencyNormalRoutingHelper(objContext, objListener);
    }

    inline void TearDown() override { delete pRoutingHelper; }
};

TEST_F(EmergencyNormalRoutingHelperTest, DestructorInvokesRemoveListener)
{
    EXPECT_CALL(objCallStateProxy, RemoveListener(_));
}

TEST_F(EmergencyNormalRoutingHelperTest, OnCallStateTerminatingForNormalCallIsIgnored)
{
    EXPECT_CALL(objServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_FALSE, 0);
}

TEST_F(EmergencyNormalRoutingHelperTest, OnCallStateNonTerminatingIsIgnored)
{
    EXPECT_CALL(objServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);

    pRoutingHelper->OnCallStateChanged(
            0, IMtcCallStateListener::State::IDLE, IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::OUTGOING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::INCOMING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::ALERTING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::ESTABLISHED,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::UPDATING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyNormalRoutingHelperTest, OnCallStateTerminatingNotifiesIdleStateAndCloseHelper)
{
    EXPECT_CALL(objListener, OnNormalRoutingClosed);
    EXPECT_CALL(objServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::IDLE), -1,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)));
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyNormalRoutingHelperTest, OnCallStateTerminatingDoesNotNotifyIfNoService)
{
    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(nullptr));

    EXPECT_CALL(objServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);

    ON_CALL(objContext, GetServiceByType(_)).WillByDefault(Return(&objService));
    ON_CALL(objService, GetJniServiceThread).WillByDefault(Return(nullptr));

    EXPECT_CALL(objServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);
    pRoutingHelper->OnCallStateChanged(0, IMtcCallStateListener::State::TERMINATING,
            IMtcCallStateListener::Type::VOIP, IMS_TRUE, 0);
}

TEST_F(EmergencyNormalRoutingHelperTest, OnTotalCallStateTerminatingNotifiesIdleStateAndCloseHelper)
{
    EXPECT_CALL(objListener, OnNormalRoutingClosed);
    EXPECT_CALL(objServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::IDLE), -1,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)));
    pRoutingHelper->OnTotalCallStateChanged(IMtcCallStateListener::State::TERMINATING);
}

TEST_F(EmergencyNormalRoutingHelperTest, HandleEmergencyCallNotifiesOpenedStateIfServiceActive)
{
    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_ACTIVE));
    EXPECT_CALL(objServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::OPENED), -1,
                    static_cast<IMS_SINT32>(ServiceType::NORMAL)));
    pRoutingHelper->HandleEmergencyCall();
}

TEST_F(EmergencyNormalRoutingHelperTest, HandleEmergencyCallNotifiesUnavailableIfServiceNotActive)
{
    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_IDLE));
    EXPECT_CALL(objServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::UNAVAILABLE),
                    -1, static_cast<IMS_SINT32>(ServiceType::NORMAL)));
    pRoutingHelper->HandleEmergencyCall();

    ON_CALL(objService, GetStatus).WillByDefault(Return(ServiceStatus::SERVICE_SUSPENDED));
    EXPECT_CALL(objServiceThread,
            OnEmergencyServiceChanged(static_cast<IMS_SINT32>(EmergencyServiceState::UNAVAILABLE),
                    -1, static_cast<IMS_SINT32>(ServiceType::NORMAL)));
    pRoutingHelper->HandleEmergencyCall();
}

TEST_F(EmergencyNormalRoutingHelperTest, HandleEmergencyCallDoesNotNotifyIfNoNormalService)
{
    ON_CALL(objContext, GetServiceByType(ServiceType::NORMAL)).WillByDefault(Return(nullptr));
    EXPECT_CALL(objServiceThread, OnEmergencyServiceChanged(_, _, _)).Times(0);
    pRoutingHelper->HandleEmergencyCall();
}

}  // namespace android
