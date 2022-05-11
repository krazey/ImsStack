/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "connection/AosConnector.h"

#include "app/MockAosAppContext.h"
#include "app/MockAosApplication.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosPcscf.h"

using ::testing::AnyNumber;
using ::testing::Assign;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_UINT32 TIMER_DURATION_FIVE_SEC = 5;

class AosConnectorTest : public ::testing::Test
{
public:
    AosConnector* pAosConnector;
    MockAosAppContext* pMockAosAppContext;
    MockIAosConnection* pMockAosConnection;

protected:
    virtual void SetUp() override
    {
        AosStaticProfile* pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);
        pMockAosConnection = new MockIAosConnection();
        MockIAosPcscf* pMockAosPcscf = new MockIAosPcscf();

        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).Times(AnyNumber()).WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(*pMockAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        EXPECT_CALL(*pMockAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(pMockAosConnection));

        EXPECT_CALL(*pMockAosAppContext, GetPcscf())
                .Times(AnyNumber())
                .WillRepeatedly(Return(pMockAosPcscf));

        pAosConnector = new AosConnector(static_cast<IAosAppContext*>(pMockAosAppContext));
        ASSERT_TRUE(pAosConnector != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosConnector)
        {
            delete pAosConnector;
        }
    }

    IAosConnection* GetConnection() { return pAosConnector->m_piConnection; }

    IAosPcscf* GetPcscf() { return pAosConnector->m_piPcscf; }

    void SetState(IMS_UINT32 nState) { pAosConnector->SetState(nState); }

    IMS_UINT32 GetState() { return pAosConnector->m_nState; }

    void SetFeature(IMS_UINT32 nFeature) { pAosConnector->m_nPendingFeature |= nFeature; }

    IMS_UINT32 GetFeature() { return pAosConnector->m_nPendingFeature; }

    void ClearFeature() { pAosConnector->ClearPending(); }

    void StartTimer(IMS_UINT32 nType, IMS_UINT32 nDuration)
    {
        pAosConnector->StartTimer(nType, nDuration);
    }

    void StopTimer(IN IMS_UINT32 nType) { pAosConnector->StopTimer(nType); }

    IMS_BOOL IsTimerRunning(IMS_UINT32 nType) { return pAosConnector->IsTimerRunning(nType); }

    IMS_BOOL IsListenerExist() { return (pAosConnector->m_piListener != IMS_NULL) ? true : false; }
};

TEST_F(AosConnectorTest, Constructor)
{
    EXPECT_NE(GetConnection(), nullptr);
    EXPECT_NE(GetPcscf(), nullptr);
}

TEST_F(AosConnectorTest, Start)
{
    bool isInvokeActivate = false;
    EXPECT_CALL(*pMockAosConnection, Activate())
            .Times(1)
            .WillOnce(DoAll(Assign(&isInvokeActivate, true), Return(true)));

    SetState(AosConnector::STATE_READY);
    EXPECT_FALSE(pAosConnector->Start());
    SetState(AosConnector::STATE_IDLE);

    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    EXPECT_FALSE(pAosConnector->Start());
    ClearFeature();

    EXPECT_TRUE(pAosConnector->Start());
    EXPECT_TRUE(isInvokeActivate);
}

TEST_F(AosConnectorTest, Stop)
{
    bool isInvokeDeactivate = false;
    EXPECT_CALL(*pMockAosConnection, Deactivate())
            .Times(1)
            .WillOnce(Assign(&isInvokeDeactivate, true));

    StartTimer(AosConnector::TIMER_IPV6, TIMER_DURATION_FIVE_SEC);
    StartTimer(AosConnector::TIMER_READY_RECOVERY, TIMER_DURATION_FIVE_SEC);
    SetFeature(AosConnector::PENDING_IPV6_DELAY);
    SetState(AosConnector::STATE_READY);

    pAosConnector->Stop();

    EXPECT_TRUE(isInvokeDeactivate);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_IPV6));
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_READY_RECOVERY));
    EXPECT_TRUE(GetFeature() == AosConnector::PENDING_NONE);
    EXPECT_TRUE(GetState() == AosConnector::STATE_IDLE);
}

TEST_F(AosConnectorTest, StopWithDelay)
{
    pAosConnector->Stop(0);
    EXPECT_FALSE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));

    pAosConnector->Stop(TIMER_DURATION_FIVE_SEC);
    EXPECT_TRUE(IsTimerRunning(AosConnector::TIMER_STOP_DELAY));
}

TEST_F(AosConnectorTest, SetListener)
{
    IAosAppContext* piAppContext = static_cast<IAosAppContext*>(pMockAosAppContext);
    AString strProfileId = AString("test");
    MockAosApplication* pMockAosApplication = new MockAosApplication(piAppContext, strProfileId);

    EXPECT_FALSE(IsListenerExist());
    pAosConnector->SetListener(pMockAosApplication);
    EXPECT_TRUE(IsListenerExist());
}

TEST_F(AosConnectorTest, IsReady)
{
    StartTimer(AosConnector::TIMER_STOP_DELAY, TIMER_DURATION_FIVE_SEC);
    EXPECT_FALSE(pAosConnector->IsReady());
    StopTimer(AosConnector::TIMER_STOP_DELAY);

    SetState(AosConnector::STATE_IDLE);
    EXPECT_FALSE(pAosConnector->IsReady());

    SetState(AosConnector::STATE_READY);
    EXPECT_TRUE(pAosConnector->IsReady());
}