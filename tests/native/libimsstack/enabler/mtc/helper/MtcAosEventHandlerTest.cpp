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

#include "IIpcan.h"
#include "IMtcContext.h"
#include "ImsAosParameter.h"
#include "MockIJniMtcServiceThread.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "configuration/MockMtcConfigurationProxy.h"
#include "helper/IMtcAosStateListener.h"
#include "helper/MockIMtcAosStateListener.h"
#include "helper/MtcAosEventHandler.h"
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;

namespace android
{

MATCHER_P(IsEqualMtcService, serviceAddress, "")
{
    return &arg == serviceAddress;
}

class MtcAosEventHandlerTest : public ::testing::Test
{
public:
    MtcAosEventHandler* pEventHandler;

    MockIMtcContext objContext;
    MockIMtcService objMtcService;
    MockMtcConfigurationProxy* pConfigProxy;
    MockIJniMtcServiceThread objJniThread;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMtcService, GetJniServiceThread).WillByDefault(Return(&objJniThread));

        pConfigProxy = new MockMtcConfigurationProxy();
        pEventHandler = new MtcAosEventHandler(objMtcService, *pConfigProxy);
    }

    virtual void TearDown() override
    {
        delete pConfigProxy;
        delete pEventHandler;
    }
};

TEST_F(MtcAosEventHandlerTest, OnConnectedNotifiesJni)
{
    EXPECT_CALL(objJniThread, OnServiceChanged(IuMtcService::ServiceState::SERVICE_UC, 0));
    EXPECT_CALL(*pConfigProxy, OnRegistrationRefreshed).Times(3);
    pEventHandler->OnConnected(ImsAosFeature::MMTEL + ImsAosFeature::VIDEO + ImsAosFeature::TEXT);

    EXPECT_CALL(objJniThread, OnServiceChanged(IuMtcService::ServiceState::SERVICE_VOIP, 0));
    pEventHandler->OnConnected(ImsAosFeature::MMTEL);

    EXPECT_CALL(objJniThread, OnServiceChanged(IuMtcService::ServiceState::SERVICE_VT, 0));
    pEventHandler->OnConnected(ImsAosFeature::VIDEO);
}

TEST_F(MtcAosEventHandlerTest, OnConnectedNotifiesListenersAndNotNotifyAfterRemoveListener)
{
    MockIMtcAosStateListener objListener1;
    MockIMtcAosStateListener objListener2;
    pEventHandler->AddListener(&objListener1);
    pEventHandler->AddListener(&objListener1);  // to check duplicated case
    pEventHandler->AddListener(&objListener2);

    IMS_UINT32 nFeatures = ImsAosFeature::MMTEL + ImsAosFeature::VIDEO;
    EXPECT_CALL(objListener1,
            OnAosStateChanged(
                    IsEqualMtcService(&objMtcService), MtcAosState::CONNECTED, ImsAosReason::NONE))
            .Times(1);
    EXPECT_CALL(objListener2,
            OnAosStateChanged(
                    IsEqualMtcService(&objMtcService), MtcAosState::CONNECTED, ImsAosReason::NONE))
            .Times(1);
    pEventHandler->OnConnected(nFeatures);

    pEventHandler->RemoveListener(&objListener1);
    pEventHandler->RemoveListener(&objListener2);
    pEventHandler->RemoveListener(&objListener2);  // to check not matching case

    EXPECT_CALL(objListener1, OnAosStateChanged(_, _, _)).Times(0);
    EXPECT_CALL(objListener2, OnAosStateChanged(_, _, _)).Times(0);
    pEventHandler->OnConnected(nFeatures);
}

TEST_F(MtcAosEventHandlerTest, OnDisconnectedNotifiesJni)
{
    MockIMtcAosStateListener objListener;
    pEventHandler->AddListener(&objListener);

    IMS_UINT32 nAnyReason = 1;
    EXPECT_CALL(objJniThread, OnServiceChanged(IuMtcService::ServiceState::SERVICE_NONE, 0));
    EXPECT_CALL(objListener,
            OnAosStateChanged(
                    IsEqualMtcService(&objMtcService), MtcAosState::DISCONNECTED, nAnyReason));

    pEventHandler->OnDisconnected(nAnyReason);
}

TEST_F(MtcAosEventHandlerTest, OnDisconnectingNotifiesListenerOnly)
{
    MockIMtcAosStateListener objListener;
    pEventHandler->AddListener(&objListener);

    IMS_UINT32 nAnyReason = 1;
    EXPECT_CALL(objJniThread, OnServiceChanged(_, _)).Times(0);
    EXPECT_CALL(objListener,
            OnAosStateChanged(
                    IsEqualMtcService(&objMtcService), MtcAosState::DISCONNECTING, nAnyReason));

    pEventHandler->OnDisconnecting(nAnyReason);
}

TEST_F(MtcAosEventHandlerTest, OnSuspendedNotifiesListenerOnly)
{
    MockIMtcAosStateListener objListener;
    pEventHandler->AddListener(&objListener);

    IMS_UINT32 nAnyReason = 1;
    EXPECT_CALL(objJniThread, OnServiceChanged(_, _)).Times(0);
    EXPECT_CALL(objListener,
            OnAosStateChanged(
                    IsEqualMtcService(&objMtcService), MtcAosState::SUSPENDED, nAnyReason));

    pEventHandler->OnSuspended(nAnyReason);
}

TEST_F(MtcAosEventHandlerTest, OnResumedNotifiesListenerOnly)
{
    MockIMtcAosStateListener objListener;
    pEventHandler->AddListener(&objListener);

    EXPECT_CALL(objJniThread, OnServiceChanged(_, _)).Times(0);
    EXPECT_CALL(objListener,
            OnAosStateChanged(
                    IsEqualMtcService(&objMtcService), MtcAosState::CONNECTED, ImsAosReason::NONE));

    pEventHandler->OnResumed();
}

TEST_F(MtcAosEventHandlerTest, OnServiceConnectedDoesNothing)
{
    MockIMtcAosStateListener objListener;
    pEventHandler->AddListener(&objListener);

    EXPECT_CALL(objJniThread, OnServiceChanged(_, _)).Times(0);
    EXPECT_CALL(objListener, OnAosStateChanged(_, _, _)).Times(0);

    pEventHandler->OnServiceConnected(1, 1);
}

TEST_F(MtcAosEventHandlerTest, OnEventNotifyDoesNothing)
{
    MockIMtcAosStateListener objListener;
    pEventHandler->AddListener(&objListener);

    EXPECT_CALL(objJniThread, OnServiceChanged(_, _)).Times(0);
    EXPECT_CALL(objListener, OnAosStateChanged(_, _, _)).Times(0);

    pEventHandler->OnEventNotify(1, 1);
}

}  // namespace android
