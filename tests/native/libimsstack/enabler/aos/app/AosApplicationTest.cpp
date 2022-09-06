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

#include "AString.h"
#include "ServiceNetworkPolicy.h"

#include "../../../config/interface/ImsServiceConfig.h"

#include "app/MockAosAppContext.h"
#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosHandle.h"
#include "interface/MockIAosNConfiguration.h"
#include "interface/MockIAosNetTracker.h"
#include "interface/MockIAosService.h"

#include "app/AosApplication.h"
#include "condition/AosCondition.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosHandle.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"
#include "provider/AosStaticProfile.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnNull;
using ::testing::ReturnRef;

const IMS_SINT32 SLOT_ID = 0;

class TestAosCondition : public AosCondition
{
    inline TestAosCondition(IN IAosAppContext* piAppContext) :
            AosCondition(piAppContext)
    {
    }

    friend class AosApplicationTest;

public:
private:
};

class TestAosApplication : public AosApplication
{
    inline TestAosApplication(IN IAosAppContext* piAppContext, IN AString& strAppId) :
            AosApplication(piAppContext, strAppId),
            m_piOrigAosCondition(IMS_NULL)
    {
    }

    friend class AosApplicationTest;
    FRIEND_TEST(AosApplicationTest, IsPdnDisconnectRequired);

public:
    void SetAosCondition(IN AosCondition* piAosCondition)
    {
        if (piAosCondition != IMS_NULL)
        {
            m_piOrigAosCondition = m_pCondition;
            m_pCondition = piAosCondition;
        }
        else
        {
            m_pCondition = m_piOrigAosCondition;
        }
    }

private:
    AosCondition* m_piOrigAosCondition;
};

class AosApplicationTest : public ::testing::Test
{
public:
    TestAosApplication* m_pTestAosApplication;
    TestAosCondition* m_pTestAosCondition;
    AosStaticProfile* m_pAosStaticProfile;

    IAosNConfiguration* m_piAosNConfiguration;
    IAosService* m_piAosService;

    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosBlock m_objMockIAosBlock;
    MockIAosConnection m_objMockIAosConnection;
    MockIAosHandle m_objMockIAosHandle;
    MockIAosNConfiguration m_objMockAosIAosNConfiguration;
    MockIAosNetTracker m_objMockAosINetTracker;
    MockIAosService m_objMockIAosService;

protected:
    virtual void SetUp() override
    {
        m_pAosStaticProfile = new AosStaticProfile();
        m_pAosStaticProfile->SetProflieType(AosStaticProfile::Type::NORMAL);

        IMSList<ImsServiceName> objServiceName =
                ImsServiceConfig::GetServiceNames(ImsServiceConfig::GetServiceProfile());

        for (IMS_UINT32 i = 0; i < objServiceName.GetSize(); i++)
        {
            ImsServiceName objService = objServiceName.GetAt(i);
            m_pAosStaticProfile->AddService(objService.GetAppId(), objService.GetServiceId());
        }

        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(SLOT_ID));
        EXPECT_CALL(m_objMockIAosAppContext, GetStaticProfile())
                .Times(AnyNumber())
                .WillRepeatedly(Return(m_pAosStaticProfile));

        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(m_pAosStaticProfile->GetId()));

        EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosBlock));

        EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
                .Times(AnyNumber())
                .WillRepeatedly(Return(&m_objMockIAosConnection));

        EXPECT_CALL(m_objMockIAosConnection, GetConnectionType())
                .Times(AnyNumber())
                .WillRepeatedly(Return(NetworkPolicy::APN_IMS));

        m_piAosNConfiguration = AosProvider::GetInstance()->GetNConfiguration();
        AosProvider::GetInstance()->SetNConfiguration(
                static_cast<IAosNConfiguration*>(&m_objMockAosIAosNConfiguration), SLOT_ID);

        EXPECT_CALL(m_objMockAosIAosNConfiguration, IsVoLteAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        EXPECT_CALL(m_objMockAosIAosNConfiguration, IsWfcImsAvailable())
                .Times(AnyNumber())
                .WillRepeatedly(Return(IMS_TRUE));

        m_piAosService = AosProvider::GetInstance()->GetService();
        AosProvider::GetInstance()->SetService(
                static_cast<IAosService*>(&m_objMockIAosService), SLOT_ID);

        m_pTestAosApplication =
                new TestAosApplication(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                        m_pAosStaticProfile->GetId());

        m_pTestAosCondition =
                new TestAosCondition(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
    }

    virtual void TearDown() override
    {
        AosProvider::GetInstance()->SetNConfiguration(m_piAosNConfiguration, SLOT_ID);
        AosProvider::GetInstance()->SetService(m_piAosService, SLOT_ID);

        if (m_pTestAosCondition)
        {
            delete m_pTestAosCondition;
        }

        if (m_pTestAosApplication)
        {
            delete m_pTestAosApplication;
        }

        if (m_pAosStaticProfile)
        {
            delete m_pAosStaticProfile;
        }
    }
};

TEST_F(AosApplicationTest, IsPdnDisconnectRequired)
{
    m_pTestAosApplication->SetAosCondition(m_pTestAosCondition);

    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _)).Times(1).WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(2)
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    EXPECT_CALL(m_objMockIAosBlock, IsReasonBlocked(_, _, _))
            .Times(AnyNumber())
            .WillRepeatedly(Return(IMS_FALSE));

    m_pTestAosApplication->SetOffReason(AoSReason::IMS_DISABLED);
    EXPECT_TRUE(m_pTestAosApplication->IsPdnDisconnectRequired());

    m_pTestAosApplication->SetOffReason(AoSReason::NONE);
    EXPECT_FALSE(m_pTestAosApplication->IsPdnDisconnectRequired());
}