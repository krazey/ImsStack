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

#include "interface/MockIAosAppContext.h"

#include "ImsServiceConfig.h"
#include "app/AosAppContext.h"
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
#include "provider/AosTracer.h"
#include "external/AosService.h"
#include "manager/AosBuilder.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class AosBuilderTest : public ::testing::Test
{
public:
    AosBuilder* m_pAosBuilder;

    AosStaticProfile* m_pAosStaticProfile;
    MockIAosAppContext m_objMockIAosAppContext;

    const AString m_strProfile = "aos_normal";

protected:
    void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        ON_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .WillByDefault(Return(m_pAosStaticProfile));
        ON_CALL(m_objMockIAosAppContext, GetSlotId()).WillByDefault(Return(SLOT_ID));
        ON_CALL(m_objMockIAosAppContext, GetProfileId()).WillByDefault(ReturnRef(m_strProfile));

        m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::NORMAL);

        m_pAosBuilder = new AosBuilder();
        ASSERT_TRUE(m_pAosBuilder != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }

        if (m_pAosBuilder)
        {
            delete m_pAosBuilder;
        }
    }
};

TEST_F(AosBuilderTest, SucceedBuildingAppContext)
{
    AosAppContext* pAosAppContext =
            static_cast<AosAppContext*>(m_pAosBuilder->BuildAppContext(m_pAosStaticProfile));
    ASSERT_TRUE(pAosAppContext != nullptr);
    delete pAosAppContext;
}

TEST_F(AosBuilderTest, SucceedBuildingAosApplication)
{
    AosApplication* pAosApplication =
            static_cast<AosApplication*>(m_pAosBuilder->BuildApp(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosApplication != nullptr);
    delete pAosApplication;
}

TEST_F(AosBuilderTest, SucceedBuildingAosEApplication)
{
    m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    AosEApplication* pAosEApplication =
            static_cast<AosEApplication*>(m_pAosBuilder->BuildApp(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosEApplication != nullptr);
    delete pAosEApplication;
}

TEST_F(AosBuilderTest, SucceedBuildingAosHandleForMtc)
{
    AosHandleMtc* pAosHandleMtc = static_cast<AosHandleMtc*>(
            m_pAosBuilder->BuildHandle(&m_objMockIAosAppContext, "ims.app.mtc", "ims.service.mtc"));
    ASSERT_TRUE(pAosHandleMtc != nullptr);
    delete pAosHandleMtc;
}

TEST_F(AosBuilderTest, SucceedBuildingAosHandleForMts)
{
    AosHandleMts* pAosHandleMts = static_cast<AosHandleMts*>(
            m_pAosBuilder->BuildHandle(&m_objMockIAosAppContext, "ims.app.mts", "ims.service.mts"));
    ASSERT_TRUE(pAosHandleMts != nullptr);
    delete pAosHandleMts;
}

TEST_F(AosBuilderTest, SucceedBuildingAosHandleForEmergencyMtc)
{
    AosHandleEmergencyMtc* pAosHandleEmergencyMtc =
            static_cast<AosHandleEmergencyMtc*>(m_pAosBuilder->BuildHandle(
                    &m_objMockIAosAppContext, "ims.app.mtc", "ims.service.mtc.emergency"));
    ASSERT_TRUE(pAosHandleEmergencyMtc != nullptr);
    delete pAosHandleEmergencyMtc;
}

TEST_F(AosBuilderTest, SucceedBuildingAosHandleForEmergencyMts)
{
    AosHandleEmergencyMts* pAosHandleEmergencyMts =
            static_cast<AosHandleEmergencyMts*>(m_pAosBuilder->BuildHandle(
                    &m_objMockIAosAppContext, "ims.app.mts", "ims.service.mts.emergency"));
    ASSERT_TRUE(pAosHandleEmergencyMts != nullptr);
    delete pAosHandleEmergencyMts;
}

TEST_F(AosBuilderTest, SucceedBuildingAosHandleForUce)
{
    AosHandleUce* pAosHandleUce = static_cast<AosHandleUce*>(
            m_pAosBuilder->BuildHandle(&m_objMockIAosAppContext, "ims.app.uce", "ims.service.uce"));
    ASSERT_TRUE(pAosHandleUce != nullptr);
    delete pAosHandleUce;
}

TEST_F(AosBuilderTest, SucceedBuildingAosHandleForSipDelegate)
{
    AosHandleSipController* pAosHandleSipController =
            static_cast<AosHandleSipController*>(m_pAosBuilder->BuildHandle(
                    &m_objMockIAosAppContext, "ims.app.sip_delegate", "ims.service.sip_delegate"));
    ASSERT_TRUE(pAosHandleSipController != nullptr);
    delete pAosHandleSipController;
}

TEST_F(AosBuilderTest, SucceedBuildingAosHandle)
{
    AosHandle* pAosHandle = static_cast<AosHandle*>(m_pAosBuilder->BuildHandle(
            &m_objMockIAosAppContext, "ims.app.test", "ims.service.test"));
    ASSERT_TRUE(pAosHandle != nullptr);
    delete pAosHandle;
}

TEST_F(AosBuilderTest, SucceedBuildingAosRegistration)
{
    AosRegistration* pAosRegistration = static_cast<AosRegistration*>(
            m_pAosBuilder->BuildRegistration(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosRegistration != nullptr);
    delete pAosRegistration;
}

TEST_F(AosBuilderTest, SucceedBuildingAosERegistration)
{
    m_pAosStaticProfile->SetProfileType(AosStaticProfile::Type::EMERGENCY);

    AosERegistration* pAosERegistration = static_cast<AosERegistration*>(
            m_pAosBuilder->BuildRegistration(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosERegistration != nullptr);
    delete pAosERegistration;
}

TEST_F(AosBuilderTest, SucceedBuildingAosSubscriber)
{
    AosSubscriber* pAosSubscriber =
            static_cast<AosSubscriber*>(m_pAosBuilder->BuildSubscriber(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosSubscriber != nullptr);
    delete pAosSubscriber;
}

TEST_F(AosBuilderTest, SucceedBuildingAosPcscf)
{
    AosPcscf* pAosPcscf =
            static_cast<AosPcscf*>(m_pAosBuilder->BuildPcscf(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosPcscf != nullptr);
    delete pAosPcscf;
}

TEST_F(AosBuilderTest, SucceedBuildingAosBlock)
{
    AosBlock* pAosBlock =
            static_cast<AosBlock*>(m_pAosBuilder->BuildBlock(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosBlock != nullptr);
    delete pAosBlock;
}

TEST_F(AosBuilderTest, SucceedBuildingAosConnection)
{
    AosConnection* pAosConnection =
            static_cast<AosConnection*>(m_pAosBuilder->BuildConnection(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosConnection != nullptr);
    delete pAosConnection;
}

TEST_F(AosBuilderTest, SucceedBuildingAosNetTracker)
{
    AosNetTracker* pAosNetTracker =
            static_cast<AosNetTracker*>(m_pAosBuilder->BuildNetTracker(&m_objMockIAosAppContext));
    ASSERT_TRUE(pAosNetTracker != nullptr);
    delete pAosNetTracker;
}

TEST_F(AosBuilderTest, SucceedBuildingAosCallTracker)
{
    IAosService* pOriginAosService = AosProvider::GetInstance()->GetService(SLOT_ID);
    MockIAosService objMockAosService;
    AosProvider::GetInstance()->SetService(static_cast<IAosService*>(&objMockAosService), SLOT_ID);

    AosCallTracker* pAosCallTracker =
            static_cast<AosCallTracker*>(m_pAosBuilder->BuildCallTracker(SLOT_ID));
    ASSERT_TRUE(pAosCallTracker != nullptr);
    delete pAosCallTracker;

    AosProvider::GetInstance()->SetService(pOriginAosService, SLOT_ID);
}

TEST_F(AosBuilderTest, SucceedBuildingAosRegStateManager)
{
    AosRegStateManager* pAosRegStateManager =
            static_cast<AosRegStateManager*>(m_pAosBuilder->BuildRegStateManager());
    ASSERT_TRUE(pAosRegStateManager != nullptr);
    delete pAosRegStateManager;
}

TEST_F(AosBuilderTest, SucceedBuildingAosService)
{
    AosService* pAosService = static_cast<AosService*>(m_pAosBuilder->BuildService(SLOT_ID));
    ASSERT_TRUE(pAosService != nullptr);
    delete pAosService;
}

TEST_F(AosBuilderTest, SucceedBuildingAosSubscriberManager)
{
    AosSubscriberManager* pAosSubscriberManager =
            static_cast<AosSubscriberManager*>(m_pAosBuilder->BuildSubscriberManager(SLOT_ID));
    ASSERT_TRUE(pAosSubscriberManager != nullptr);
    delete pAosSubscriberManager;
}

TEST_F(AosBuilderTest, SucceedBuildingAosRetryRepository)
{
    AosRetryRepository* pAosRetryRepository =
            static_cast<AosRetryRepository*>(m_pAosBuilder->BuildRetryRepository(SLOT_ID));
    ASSERT_TRUE(pAosRetryRepository != nullptr);
    delete pAosRetryRepository;
}

TEST_F(AosBuilderTest, SucceedBuildingAosNConfiguration)
{
    AosNConfiguration* pAosNConfiguration =
            static_cast<AosNConfiguration*>(m_pAosBuilder->BuildNConfiguration());
    ASSERT_TRUE(pAosNConfiguration != nullptr);
    delete pAosNConfiguration;
}

TEST_F(AosBuilderTest, SucceedBuildingAosTracer)
{
    AosTracer* pAosTracer = static_cast<AosTracer*>(m_pAosBuilder->BuildTracer(SLOT_ID));
    ASSERT_TRUE(pAosTracer != nullptr);
    delete pAosTracer;
}
