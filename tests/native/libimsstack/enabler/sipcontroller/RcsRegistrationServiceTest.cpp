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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "IURcsMessageService.h"
#include "JniEnablerConnector.h"
#include "MockIJniEnabler.h"
#include "MockIImsAos.h"
#include "MockISipControllerServiceThread.h"
#include "RcsRegistrationService.h"
#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

LOCAL IMS_SINT32 SLOT_ID = 0;

class TestRcsRegistrationService : public RcsRegistrationService
{
public:
    inline explicit TestRcsRegistrationService(IImsAos* piImsAos) :
            RcsRegistrationService(piImsAos, SLOT_ID)
    {
    }
    virtual ~TestRcsRegistrationService() {}

    void Connecting() { ImsAos_Connecting(); }
    void Connected() { ImsAos_Connected(0, 0); }
    void DisConnecting() { ImsAos_Disconnecting(0); }
    void DisConnected() { ImsAos_Disconnected(0); }
};

class RcsRegistrationServiceTest : public ::testing::Test
{
public:
    TestRcsRegistrationService* pService;
    MockIJniEnabler objMockEnabler;
    MockISipControllerServiceThread objMockThread;
    MockIImsAos objMockIImsAos;

protected:
    virtual void SetUp() override
    {
        pService = new TestRcsRegistrationService(&objMockIImsAos);
        ASSERT_TRUE(pService != nullptr);
        ON_CALL(objMockEnabler, GetJniThread()).WillByDefault(Return(&objMockThread));
        JniEnablerConnector::GetInstance().SetJniEnabler(
                0, EnablerType::SIP_DELEGATE, &objMockEnabler);
    }

    virtual void TearDown() override
    {
        if (pService)
        {
            delete pService;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::SIP_DELEGATE, IMS_NULL);
    }
};

TEST_F(RcsRegistrationServiceTest, updateDelegateRegistration)
{
    IMS_TRACE_D("UpdateDelegateRegistration", 0, 0, 0);
    IUSncFeatureTagsParam* pParam = new IUSncFeatureTagsParam();
    pParam->m_nFeatureCount = 1;
    pParam->m_objFeatureTags.AddElement("FeatureTag");
    EXPECT_EQ(pService->UpdateDelegateRegistration(reinterpret_cast<IMS_UINTP>(pParam)), IMS_TRUE);
}

TEST_F(RcsRegistrationServiceTest, triggerDelegateDeregistration)
{
    IMS_TRACE_D("TriggerDelegateDeregistration", 0, 0, 0);
    EXPECT_EQ(pService->TriggerDelegateDeregistration(), IMS_TRUE);
}

TEST_F(RcsRegistrationServiceTest, aosConnected)
{
    IMS_TRACE_D("aosConnected", 0, 0, 0);
    EXPECT_CALL(objMockThread, OnRegistrationUpdated(_)).Times(1);
    pService->Connected();
}

TEST_F(RcsRegistrationServiceTest, aosConnecting)
{
    IMS_TRACE_D("aosConnecting", 0, 0, 0);
    EXPECT_CALL(objMockThread, OnRegistrationUpdated(_)).Times(1);
    pService->Connecting();
}

TEST_F(RcsRegistrationServiceTest, aosDisConnected)
{
    IMS_TRACE_D("aosDisConnected", 0, 0, 0);
    EXPECT_CALL(objMockThread, OnRegistrationUpdated(_)).Times(1);
    pService->DisConnected();
}

TEST_F(RcsRegistrationServiceTest, aosDisConnecting)
{
    IMS_TRACE_D("aosDisConnecting", 0, 0, 0);
    EXPECT_CALL(objMockThread, OnRegistrationUpdated(_)).Times(1);
    pService->DisConnecting();
}
