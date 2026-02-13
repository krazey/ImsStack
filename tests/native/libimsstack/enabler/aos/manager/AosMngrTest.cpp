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

#include "manager/AosMngr.h"
#include "provider/AosStaticConfig.h"
#include "provider/AosStaticProfile.h"
#include "provider/AosProvider.h"

#include "PlatformContext.h"
#include "TestTimerService.h"
#include "TestThreadService.h"
#include "TestUtilService.h"
#include "TestPhoneInfoService.h"
#include "TestConfigService.h"
#include "MockICarrierConfig.h"
#include "MockIImsPrivateProperty.h"

#include "interface/MockIAosBuilder.h"
#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosRegistration.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosPcscf.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosSubscriber.h"
#include "interface/MockIAosCallTracker.h"
#include "interface/MockIAosRegStateManager.h"
#include "../../interface/aos/MockIAosService.h"
#include "interface/MockIAosSubscriberManager.h"
#include "interface/MockIAosRetryRepository.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosTracer.h"
#include "interface/MockIAosTransaction.h"

using ::testing::_;
using ::testing::An;
using ::testing::AnyNumber;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;
const AString TEST_APP_ID = "ims.app.test";
const AString TEST_SRV_ID = "ims.service.test";
const AString TEST_PROFILE_ID = "profile_id_1";

#define DECLARE_USING(Base) \
    using Base::CreateAos;  \
    using Base::DestroyAos; \
    using Base::DestroyStaticConfig;

/**
 * @class TestAosMngr
 * @brief Subclass of AosMngr to expose protected members and inject Mocks.
 *        It handles the destruction of real objects created by the base constructor
 *        and re-initializes with Mock objects.
 */
class TestAosMngr : public AosMngr
{
public:
    DECLARE_USING(AosMngr)

    TestAosMngr(IN IMS_SINT32 nSlotId, IN MockIAosBuilder* pMockBuilder) :
            AosMngr(nSlotId),
            m_pMockBuilder(pMockBuilder)
    {
        // Vital: Destroy real objects created by the base AosMngr constructor. This prevents
        //        the test from interacting with uninitialized real system components.
        FullDestroyAos();
        DestroyStaticConfig();

        // Re-initialize using the Mock Builder.
        CreateAos();
    }

    void SetStaticConfig(IN AosStaticConfig* pConfig)
    {
        if (m_pStaticConfig != IMS_NULL)
        {
            DestroyStaticConfig();
        }
        m_pStaticConfig = pConfig;
    }

    void InjectAppContext(IN const AString& strProfileId, IN IAosAppContext* pContext)
    {
        m_objAppContext.Add(strProfileId, pContext);
    }

    void FullDestroyAos()
    {
        DestroyAos();
        m_objAppContext.Clear();
    }

    inline IMS_UINT32 GetAppContextCount() { return m_objAppContext.GetSize(); }

    inline IAosAppContext* GetAppContext(IN const AString& strProfileId)
    {
        return m_objAppContext.GetValue(strProfileId);
    }

protected:
    IAosBuilder* AosBuilderFactory() override { return m_pMockBuilder; }

private:
    MockIAosBuilder* m_pMockBuilder;
};

/**
 * @class FakeAosStaticConfig
 * @brief AosStaticConfig subclass for injecting test profiles.
 */
class FakeAosStaticConfig : public AosStaticConfig
{
public:
    void AddTestProfile(IN AosStaticProfile* pProfile)
    {
        const_cast<ImsList<AosStaticProfile*>&>(GetProfiles()).Append(pProfile);
    }
};

class MockAosStaticConfig : public AosStaticConfig
{
public:
    MOCK_METHOD(void, Die, ());
    virtual ~MockAosStaticConfig() { Die(); }
};

/**
 * @class AosMngrTest
 * @brief Test Suite for AosMngr class.
 */
class AosMngrTest : public ::testing::Test
{
public:
    TestAosMngr* m_pAosMngr = IMS_NULL;
    NiceMock<MockIAosBuilder>* m_pMockBuilder = IMS_NULL;

    // Platform Services
    TestTimerService m_objTimerService;
    TestThreadService m_objThreadService;
    TestUtilService m_objUtilService;
    TestPhoneInfoService m_objPhoneInfoService;
    TestConfigService m_objConfigService;

    // Safety Containers for Reference Returns (to prevent dangling pointers)
    ImsVector<IMS_SINT32> m_objDummyIntVector;
    ImsVector<AString> m_objDummyStringVector;

    // Recursive Mock Object for GetBundle calls
    NiceMock<MockICarrierConfig> m_objDummyBundle;

    // Mock Components
    NiceMock<MockIAosAppContext>* m_pMockAppContext;
    NiceMock<MockIAosHandle>* m_pMockHandle;
    NiceMock<MockIAosService>* m_pMockService;

protected:
    void SetUp() override
    {
        // 1. Initialize Platform Services with Mocks
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &m_objTimerService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_THREAD, &m_objThreadService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_UTIL, &m_objUtilService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_PHONE_INFO, &m_objPhoneInfoService);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_CONFIG, &m_objConfigService);

        // 2. Configure Mock CarrierConfig (Recursive Mocking Strategy)
        MockICarrierConfig& objMockCarrierConfig = m_objConfigService.GetMockCarrierConfig();

        EXPECT_CALL(objMockCarrierConfig, GetBundle(_))
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objDummyBundle));
        ON_CALL(m_objDummyBundle, GetBundle(_)).WillByDefault(Return(&m_objDummyBundle));

        EXPECT_CALL(objMockCarrierConfig, GetIntArray(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_objDummyIntVector));
        EXPECT_CALL(objMockCarrierConfig, GetStringArray(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_objDummyStringVector));
        EXPECT_CALL(objMockCarrierConfig, GetBoolean(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(false));
        EXPECT_CALL(objMockCarrierConfig, GetInt(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));
        EXPECT_CALL(objMockCarrierConfig, GetString(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(AString("")));

        // Silence Add/RemoveListener warnings
        EXPECT_CALL(objMockCarrierConfig, RemoveListener(_)).Times(AnyNumber());
        EXPECT_CALL(objMockCarrierConfig, AddListener(_)).Times(AnyNumber());

        // Dummy Bundle defaults
        ON_CALL(m_objDummyBundle, GetIntArray(_, _)).WillByDefault(Return(m_objDummyIntVector));
        ON_CALL(m_objDummyBundle, GetBoolean(_, _)).WillByDefault(Return(false));
        ON_CALL(m_objDummyBundle, GetInt(_, _)).WillByDefault(Return(0));

        // 3. Configure Mock PrivateProperty (Suppress warnings)
        MockIImsPrivateProperty& pMockPrivateProperty = m_objUtilService.GetMockPrivateProperty();
        EXPECT_CALL(pMockPrivateProperty, GetPersistent(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(AString("")));
        EXPECT_CALL(pMockPrivateProperty, GetPersistentInt(_, _))
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        // 4. Initialize Mock Builder and Components
        m_pMockBuilder = new NiceMock<MockIAosBuilder>();
        m_pMockAppContext = new NiceMock<MockIAosAppContext>();
        m_pMockHandle = new NiceMock<MockIAosHandle>();
        m_pMockService = new NiceMock<MockIAosService>();

        // 5. Set default AppId for MockHandle to avoid filtering in GetAllAosHandles
        ON_CALL(*m_pMockHandle, GetAppId()).WillByDefault(ReturnRef(TEST_APP_ID));
        ON_CALL(*m_pMockHandle, GetServiceId()).WillByDefault(ReturnRef(TEST_SRV_ID));

        // 6. Set Builder Expectations
        ON_CALL(*m_pMockBuilder, BuildService(SLOT_ID)).WillByDefault(Return(m_pMockService));
        ON_CALL(*m_pMockBuilder, BuildNConfiguration())
                .WillByDefault(Return(new NiceMock<MockIAosNConfiguration>()));
        ON_CALL(*m_pMockBuilder, BuildCallTracker(_))
                .WillByDefault(Return(new NiceMock<MockIAosCallTracker>()));
        ON_CALL(*m_pMockBuilder, BuildRegStateManager())
                .WillByDefault(Return(new NiceMock<MockIAosRegStateManager>()));
        ON_CALL(*m_pMockBuilder, BuildSubscriberManager(_))
                .WillByDefault(Return(new NiceMock<MockIAosSubscriberManager>()));
        ON_CALL(*m_pMockBuilder, BuildRetryRepository(_))
                .WillByDefault(Return(new NiceMock<MockIAosRetryRepository>()));
        ON_CALL(*m_pMockBuilder, BuildTracer(_))
                .WillByDefault(Return(new NiceMock<MockIAosTracer>()));
        ON_CALL(*m_pMockBuilder, BuildTransaction(_))
                .WillByDefault(Return(new NiceMock<MockIAosTransaction>()));
        ON_CALL(*m_pMockBuilder, BuildAppContext(_)).WillByDefault(Return(m_pMockAppContext));
        ON_CALL(*m_pMockBuilder, BuildApp(_))
                .WillByDefault(Return(new NiceMock<MockIAosApplication>()));
        ON_CALL(*m_pMockBuilder, BuildRegistration(_))
                .WillByDefault(Return(new NiceMock<MockIAosRegistration>()));
        ON_CALL(*m_pMockBuilder, BuildConnection(_))
                .WillByDefault(Return(new NiceMock<MockIAosConnection>()));
        ON_CALL(*m_pMockBuilder, BuildNetTracker(_))
                .WillByDefault(Return(new NiceMock<MockIAosNetTracker>()));
        ON_CALL(*m_pMockBuilder, BuildBlock(_))
                .WillByDefault(Return(new NiceMock<MockIAosBlock>()));
        ON_CALL(*m_pMockBuilder, BuildPcscf(_))
                .WillByDefault(Return(new NiceMock<MockIAosPcscf>()));
        ON_CALL(*m_pMockBuilder, BuildSubscriber(_))
                .WillByDefault(Return(new NiceMock<MockIAosSubscriber>()));
        ON_CALL(*m_pMockBuilder, BuildHandle(_, _, _)).WillByDefault(Return(m_pMockHandle));
    }

    void TearDown() override
    {
        if (m_pAosMngr != IMS_NULL)
        {
            m_pAosMngr->FullDestroyAos();
            delete m_pAosMngr;
            m_pAosMngr = IMS_NULL;
        }

        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_CONFIG, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_PHONE_INFO, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_UTIL, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

    AosStaticProfile* CreateTestProfile(
            IN const AString& strAppId, IN const AString& strSrvId, IN const AString& strProfileId)
    {
        AosStaticProfile* pProfile = new AosStaticProfile();
        pProfile->GetId() = strProfileId;
        pProfile->AddService(strAppId, strSrvId);
        return pProfile;
    }
};

TEST_F(AosMngrTest, ConstructionWithMockBuilderNoCrash)
{
    // GIVEN: Expectation that the Mock Builder is called during re-initialization (CreateAos).
    EXPECT_CALL(*m_pMockBuilder, BuildService(SLOT_ID)).Times(1);

    // WHEN
    m_pAosMngr = new TestAosMngr(SLOT_ID, m_pMockBuilder);

    // THEN
    EXPECT_NE(m_pAosMngr, nullptr);
}

TEST_F(AosMngrTest, GetAosHandleReturnsHandleWhenProfileAndContextExist)
{
    // GIVEN 1: AosMngr is created
    m_pAosMngr = new TestAosMngr(SLOT_ID, m_pMockBuilder);

    // GIVEN 2: Static Config with a profile is injected
    AosStaticProfile* pProfile = CreateTestProfile(TEST_APP_ID, TEST_SRV_ID, TEST_PROFILE_ID);
    FakeAosStaticConfig* pConfig = new FakeAosStaticConfig();
    pConfig->AddTestProfile(pProfile);
    m_pAosMngr->SetStaticConfig(pConfig);

    // GIVEN 3: Mock AppContext is injected into the manager
    m_pAosMngr->InjectAppContext(TEST_PROFILE_ID, m_pMockAppContext);

    // GIVEN 4: Expectation for GetHandle
    EXPECT_CALL(*m_pMockAppContext, GetHandle(TEST_SRV_ID)).WillOnce(Return(m_pMockHandle));

    // WHEN
    IAosHandle* piHandle = m_pAosMngr->GetAosHandle(TEST_APP_ID, TEST_SRV_ID);

    // THEN
    EXPECT_EQ(piHandle, m_pMockHandle);
}

TEST_F(AosMngrTest, GetAosHandleReturnsNullWhenProfileDoesNotExist)
{
    // GIVEN: Initialize AosMngr and set an empty configuration (no profiles).
    m_pAosMngr = new TestAosMngr(SLOT_ID, m_pMockBuilder);
    FakeAosStaticConfig* pConfig = new FakeAosStaticConfig();
    // Do not add any profiles
    m_pAosMngr->SetStaticConfig(pConfig);

    // WHEN: Request a handle for a non-existent service ID.
    IAosHandle* piHandle = m_pAosMngr->GetAosHandle(TEST_APP_ID, "invalid.service.id");

    // THEN
    EXPECT_EQ(piHandle, nullptr);
}

TEST_F(AosMngrTest, GetAllAosHandlesReturnsValidList)
{
    // GIVEN 1: Initialize AosMngr.
    m_pAosMngr = new TestAosMngr(SLOT_ID, m_pMockBuilder);

    // GIVEN 2: Configure profiles (Register 2 different services under the same AppId).
    FakeAosStaticConfig* pConfig = new FakeAosStaticConfig();
    AosStaticProfile* pProfile1 = CreateTestProfile(TEST_APP_ID, "service_1", "profile_1");
    pConfig->AddTestProfile(pProfile1);
    AosStaticProfile* pProfile2 = CreateTestProfile(TEST_APP_ID, "service_2", "profile_2");
    pConfig->AddTestProfile(pProfile2);
    m_pAosMngr->SetStaticConfig(pConfig);

    // GIVEN 3: Inject Mock AppContexts.
    m_pAosMngr->InjectAppContext("profile_1", m_pMockAppContext);
    m_pAosMngr->InjectAppContext("profile_2", m_pMockAppContext);

    ON_CALL(*m_pMockAppContext, GetHandle(An<const AString&>()))
            .WillByDefault(Return(m_pMockHandle));
    ON_CALL(*m_pMockHandle, GetAppId()).WillByDefault(ReturnRef(TEST_APP_ID));

    // WHEN: Retrieve all handles for the AppId.
    ImsList<IAosHandle*> objHandles = m_pAosMngr->GetAllAosHandles(TEST_APP_ID);

    // THEN: Two handles should be returned.
    EXPECT_EQ(objHandles.GetSize(), 2);
}

TEST_F(AosMngrTest, DestroyAosClearsAppContexts)
{
    // GIVEN: Initialize AosMngr.
    m_pAosMngr = new TestAosMngr(SLOT_ID, m_pMockBuilder);
    m_pAosMngr->FullDestroyAos();  // Ensure clean start

    // Inject a Mock Context
    m_pAosMngr->InjectAppContext(TEST_PROFILE_ID, m_pMockAppContext);

    // Verify Injection
    ASSERT_EQ(m_pAosMngr->GetAppContextCount(), 1);
    ASSERT_EQ(m_pAosMngr->GetAppContext(TEST_PROFILE_ID), m_pMockAppContext);

    // WHEN
    m_pAosMngr->FullDestroyAos();

    // THEN: The internal AppContext map should be empty.
    EXPECT_EQ(m_pAosMngr->GetAppContextCount(), 0);
}

TEST_F(AosMngrTest, DestroyStaticConfigDeletesObject)
{
    // GIVEN
    m_pAosMngr = new TestAosMngr(SLOT_ID, m_pMockBuilder);
    MockAosStaticConfig* pMockAosStaticConfig = new MockAosStaticConfig();
    m_pAosMngr->SetStaticConfig(pMockAosStaticConfig);

    EXPECT_CALL(*pMockAosStaticConfig, Die()).Times(1);

    // WHEN
    m_pAosMngr->DestroyStaticConfig();

    // THEN: The Die() expectation confirms the object was deleted.
}
