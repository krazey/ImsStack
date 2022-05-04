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

#include "interface/IAosAppContext.h"
#include "interface/IAosCallTracker.h"
#include "interface/IAosConnection.h"
#include "interface/IAosRegistration.h"
#include "condition/AosServiceAvailableWifi.h"

#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosRegistration.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;

class AosServiceAvailableWifiTest : public ::testing::Test {
public:
    AosServiceAvailableWifi* pAosServiceAvailableWifi;

protected:
    virtual void SetUp() override {
        pAosServiceAvailableWifi = new AosServiceAvailableWifi();
        ASSERT_TRUE(pAosServiceAvailableWifi != nullptr);
    }

    virtual void TearDown() override {
        if (pAosServiceAvailableWifi) {
            delete pAosServiceAvailableWifi;
        }
    }

    void SetCallTracker(IN IAosCallTracker* piCallTracker) {
        pAosServiceAvailableWifi->m_piCallTracker = piCallTracker;
    }

    IAosCallTracker* GetCallTracker() {
        return pAosServiceAvailableWifi->m_piCallTracker;
    }

    void SetRegistration(IN IAosRegistration* piRegistration) {
        pAosServiceAvailableWifi->m_piRegistration = piRegistration;
    }

    IAosRegistration* GetRegistration() {
        return pAosServiceAvailableWifi->m_piRegistration;
    }

    void SetConnection(IN IAosConnection* piConnection) {
        pAosServiceAvailableWifi->m_piConnection = piConnection;
    }

    IAosConnection* GetConnection() {
        return pAosServiceAvailableWifi->m_piConnection;
    }

    void SetBadNetworkState(IN IMS_UINT32 nState) {
        pAosServiceAvailableWifi->m_nBadNetworkState = nState;
    }

};

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_CallTrackerNull) {
    SetCallTracker(IMS_NULL);
    EXPECT_EQ(GetCallTracker(), nullptr);

    EXPECT_FALSE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_EmergencyActive) {
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_TRUE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    EXPECT_FALSE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationNull) {
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_FALSE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    SetRegistration(IMS_NULL);
    EXPECT_EQ(GetRegistration(), nullptr);

    EXPECT_FALSE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_RegistrationRegistered) {
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_FALSE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_FALSE));

    SetRegistration(static_cast<IAosRegistration*>(&objMockIAosRegistration));
    EXPECT_NE(GetRegistration(), nullptr);

    EXPECT_FALSE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_BadNetworkState) {
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_FALSE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_TRUE));

    SetRegistration(static_cast<IAosRegistration*>(&objMockIAosRegistration));
    EXPECT_NE(GetRegistration(), nullptr);

    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    EXPECT_FALSE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());

    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
    EXPECT_FALSE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StartToCheckNetworkConnection_NormalCallActive) {
    MockIAosCallTracker objMockIAosCallTracker;
    EXPECT_CALL(objMockIAosCallTracker, IsEmergencyCallActive())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_FALSE));

    EXPECT_CALL(objMockIAosCallTracker, IsNormalCallActive())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_TRUE));

    SetCallTracker(static_cast<IAosCallTracker*>(&objMockIAosCallTracker));
    EXPECT_NE(GetCallTracker(), nullptr);

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(objMockIAosRegistration, IsRegistered())
        .Times(AnyNumber())
        .WillRepeatedly(Return(IMS_TRUE));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(_, _, _))
        .Times(AnyNumber());

    SetRegistration(static_cast<IAosRegistration*>(&objMockIAosRegistration));
    EXPECT_NE(GetRegistration(), nullptr);

    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(objMockIAosConnection, IsEpdgEnabled())
        .Times(2)
        .WillOnce(Return(IMS_FALSE))
        .WillOnce(Return(IMS_TRUE));

    SetConnection(static_cast<IAosConnection*>(&objMockIAosConnection));

    EXPECT_FALSE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());
    EXPECT_TRUE(pAosServiceAvailableWifi->StartToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkNone) {
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_NONE);
    EXPECT_FALSE(pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkDetected) {
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_DETECTED);
    EXPECT_TRUE(pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}

TEST_F(AosServiceAvailableWifiTest, StopToCheckNetworkConnection_BadNetworkChecking) {
    SetBadNetworkState(AosServiceAvailableWifi::STATE_BAD_NETWORK_CHECKING);
    EXPECT_FALSE(pAosServiceAvailableWifi->StopToCheckNetworkConnection());
}