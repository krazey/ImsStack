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

#include "../../interface/aos/MockIAosService.h"

#include "app/MockAosAppContext.h"

#include "ImsServiceConfig.h"
#include "app/AosApplication.h"
#include "app/AosEApplication.h"
#include "handle/AosHandle.h"
#include "handle/AosHandleMtc.h"
#include "handle/AosHandleMts.h"
#include "handle/AosHandleEmergencyMtc.h"
#include "handle/AosHandleEmergencyMts.h"
#include "handle/AosHandleUce.h"
#include "handle/AosHandleSipController.h"
#include "interface/IAosApplication.h"
#include "interface/IAosAppContext.h"
#include "registration/AosRegistration.h"
#include "registration/AosERegistration.h"
#include "condition/AosSubscriber.h"
#include "connection/AosPcscf.h"
#include "condition/AosBlock.h"
#include "connection/AosConnection.h"
#include "network/AosNetTracker.h"
#include "provider/AosCallTracker.h"
#include "provider/AosProvider.h"
#include "provider/AosRegStateManager.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosNConfiguration.h"
#include "provider/AosSubscriberManager.h"
#include "provider/AosRetryRepository.h"
#include "external/AosService.h"
#include "manager/AosBuilder.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosBuilderTest : public ::testing::Test
{
public:
    AosBuilder* pAosBuilder;

    AosStaticProfile* pAosStaticProfile;
    MockAosAppContext* pMockAosAppContext;

    const AString strProfile = "aos_normal";

protected:
    virtual void SetUp() override
    {
        pAosStaticProfile = new AosStaticProfile();
        pMockAosAppContext = new MockAosAppContext(pAosStaticProfile);
        EXPECT_CALL(*pMockAosAppContext, GetStaticProfile())
                .WillRepeatedly(Return(pAosStaticProfile));
        EXPECT_CALL(*pMockAosAppContext, GetSlotId()).WillRepeatedly(Return(SLOT_ID));
        EXPECT_CALL(*pMockAosAppContext, GetProfileId()).WillRepeatedly(ReturnRef(strProfile));

        pAosBuilder = new AosBuilder();
        ASSERT_TRUE(pAosBuilder != nullptr);
    }

    virtual void TearDown() override
    {
        if (pMockAosAppContext)
        {
            delete pMockAosAppContext;
        }

        if (pAosStaticProfile)
        {
            delete pAosStaticProfile;
        }

        if (pAosBuilder)
        {
            delete pAosBuilder;
        }
    }
};

TEST_F(AosBuilderTest, BuildAppContext)
{
    AosAppContext* pAosAppContext =
            static_cast<AosAppContext*>(pAosBuilder->BuildAppContext(pAosStaticProfile));
    ASSERT_TRUE(pAosAppContext != nullptr);
    delete pAosAppContext;
}

TEST_F(AosBuilderTest, BuildApplication)
{
    pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);
    AosApplication* pAosApplication = static_cast<AosApplication*>(
            pAosBuilder->BuildApp(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosApplication != nullptr);
    delete pAosApplication;

    pAosStaticProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);
    AosEApplication* pAosEApplication = static_cast<AosEApplication*>(
            pAosBuilder->BuildApp(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosEApplication != nullptr);
    delete pAosEApplication;

    pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);
}

TEST_F(AosBuilderTest, BuildHandle)
{
    AosHandleMtc* pAosHandleMtc = static_cast<AosHandleMtc*>(pAosBuilder->BuildHandle(
            static_cast<IAosAppContext*>(pMockAosAppContext), "ims.app.mtc", "ims.service.mtc"));
    ASSERT_TRUE(pAosHandleMtc != nullptr);
    delete pAosHandleMtc;

    AosHandleMts* pAosHandleMts = static_cast<AosHandleMts*>(pAosBuilder->BuildHandle(
            static_cast<IAosAppContext*>(pMockAosAppContext), "ims.app.mts", "ims.service.mts"));
    ASSERT_TRUE(pAosHandleMts != nullptr);
    delete pAosHandleMts;

    AosHandleEmergencyMtc* pAosHandleEmergencyMtc = static_cast<AosHandleEmergencyMtc*>(
            pAosBuilder->BuildHandle(static_cast<IAosAppContext*>(pMockAosAppContext),
                    "ims.app.mtc", "ims.service.mtc.emergency"));
    ASSERT_TRUE(pAosHandleEmergencyMtc != nullptr);
    delete pAosHandleEmergencyMtc;

    AosHandleEmergencyMts* pAosHandleEmergencyMts = static_cast<AosHandleEmergencyMts*>(
            pAosBuilder->BuildHandle(static_cast<IAosAppContext*>(pMockAosAppContext),
                    "ims.app.mts", "ims.service.mts.emergency"));
    ASSERT_TRUE(pAosHandleEmergencyMts != nullptr);
    delete pAosHandleEmergencyMts;

    AosHandleUce* pAosHandleUce = static_cast<AosHandleUce*>(pAosBuilder->BuildHandle(
            static_cast<IAosAppContext*>(pMockAosAppContext), "ims.app.uce", "ims.service.uce"));
    ASSERT_TRUE(pAosHandleUce != nullptr);
    delete pAosHandleUce;

    AosHandleSipController* pAosHandleSipController = static_cast<AosHandleSipController*>(
            pAosBuilder->BuildHandle(static_cast<IAosAppContext*>(pMockAosAppContext),
                    "ims.app.sip_delegate", "ims.service.sip_delegate"));
    ASSERT_TRUE(pAosHandleSipController != nullptr);
    delete pAosHandleSipController;

    AosHandle* pAosHandle = static_cast<AosHandle*>(pAosBuilder->BuildHandle(
            static_cast<IAosAppContext*>(pMockAosAppContext), "ims.app.test", "ims.service.test"));
    ASSERT_TRUE(pAosHandle != nullptr);
    delete pAosHandle;
}

TEST_F(AosBuilderTest, BuildRegistration)
{
    pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);
    AosRegistration* pAosRegistration = static_cast<AosRegistration*>(
            pAosBuilder->BuildRegistration(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosRegistration != nullptr);
    delete pAosRegistration;

    pAosStaticProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);
    AosERegistration* pAosERegistration = static_cast<AosERegistration*>(
            pAosBuilder->BuildRegistration(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosERegistration != nullptr);
    delete pAosERegistration;

    pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);
}

TEST_F(AosBuilderTest, BuildSubscriber)
{
    AosSubscriber* pAosSubscriber = static_cast<AosSubscriber*>(
            pAosBuilder->BuildSubscriber(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosSubscriber != nullptr);
    delete pAosSubscriber;
}

TEST_F(AosBuilderTest, BuildPcscf)
{
    AosPcscf* pAosPcscf = static_cast<AosPcscf*>(
            pAosBuilder->BuildPcscf(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosPcscf != nullptr);
    delete pAosPcscf;
}

TEST_F(AosBuilderTest, BuildBlock)
{
    AosBlock* pAosBlock = static_cast<AosBlock*>(
            pAosBuilder->BuildBlock(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosBlock != nullptr);
    delete pAosBlock;
}

TEST_F(AosBuilderTest, BuildConnection)
{
    AosConnection* pAosConnection = static_cast<AosConnection*>(
            pAosBuilder->BuildConnection(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosConnection != nullptr);
    delete pAosConnection;
}

TEST_F(AosBuilderTest, BuildNetTracker)
{
    AosNetTracker* pAosNetTracker = static_cast<AosNetTracker*>(
            pAosBuilder->BuildNetTracker(static_cast<IAosAppContext*>(pMockAosAppContext)));
    ASSERT_TRUE(pAosNetTracker != nullptr);
    delete pAosNetTracker;
}

TEST_F(AosBuilderTest, BuildCallTracker)
{
    IAosService* pOriginAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
    MockIAosService objMockAosService;
    AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&objMockAosService), SLOT_ID);

    AosCallTracker* pAosCallTracker =
            static_cast<AosCallTracker*>(pAosBuilder->BuildCallTracker(SLOT_ID));
    ASSERT_TRUE(pAosCallTracker != nullptr);
    delete pAosCallTracker;

    AosProvider::GetInstance()->SetService(pOriginAosService, SLOT_ID);
}

TEST_F(AosBuilderTest, BuildRegStateManager)
{
    AosRegStateManager* pAosRegStateManager =
            static_cast<AosRegStateManager*>(pAosBuilder->BuildRegStateManager());
    ASSERT_TRUE(pAosRegStateManager != nullptr);
    delete pAosRegStateManager;
}

TEST_F(AosBuilderTest, BuildService)
{
    AosService* pAosService = static_cast<AosService*>(pAosBuilder->BuildService(SLOT_ID));
    ASSERT_TRUE(pAosService != nullptr);
    delete pAosService;
}

TEST_F(AosBuilderTest, BuildSubscriberManager)
{
    AosSubscriberManager* pAosSubscriberManager =
            static_cast<AosSubscriberManager*>(pAosBuilder->BuildSubscriberManager(SLOT_ID));
    ASSERT_TRUE(pAosSubscriberManager != nullptr);
    delete pAosSubscriberManager;
}

TEST_F(AosBuilderTest, BuildRetryRepository)
{
    AosRetryRepository* pAosRetryRepository =
            static_cast<AosRetryRepository*>(pAosBuilder->BuildRetryRepository(SLOT_ID));
    ASSERT_TRUE(pAosRetryRepository != nullptr);
    delete pAosRetryRepository;
}

TEST_F(AosBuilderTest, BuildNConfiguration)
{
    AosNConfiguration* pAosNConfiguration =
            static_cast<AosNConfiguration*>(pAosBuilder->BuildNConfiguration());
    ASSERT_TRUE(pAosNConfiguration != nullptr);
    delete pAosNConfiguration;
}