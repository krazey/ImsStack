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

#include "IIpcan.h"
#include "ServiceNetworkPolicy.h"

#include "handle/AosInfo.h"
#include "handle/AosHandle.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosApplication.h"
#include "interface/IAosBlock.h"
#include "interface/IAosConnection.h"
#include "interface/IAosRegistration.h"

#include "interface/MockIAosAppContext.h"
#include "interface/MockIAosApplication.h"
#include "interface/MockIAosBlock.h"
#include "interface/MockIAosConnection.h"
#include "interface/MockIAosRegistration.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::ReturnRef;

class TestAosHandle : public AosHandle
{
    inline TestAosHandle(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_UINT32 nServiceType) :
            AosHandle(piAppContext, strAppId, strServiceId, nServiceType)
    {
    }

    friend class AosInfoTest;
    FRIEND_TEST(AosInfoTest, GetImsFeatures_Test1);
};

class AosInfoTest : public ::testing::Test
{
public:
    TestAosHandle* m_pTestAosHandle;
    AosInfo* m_pAosInfo;
    MockIAosAppContext m_objMockIAosAppContext;
    MockIAosApplication m_objMockIAosApplication;

    const AString m_strAppId = AString("ims.app.test");
    const AString m_strServiceId = AString("ims.service.test");
    const IMS_UINT32 m_nServiceType = -1;

protected:
    void SetUp() override
    {
        EXPECT_CALL(m_objMockIAosAppContext, GetSlotId())
                .Times(AnyNumber())
                .WillRepeatedly(Return(0));

        const AString strValue = AString("test");
        EXPECT_CALL(m_objMockIAosAppContext, GetProfileId())
                .Times(AnyNumber())
                .WillRepeatedly(ReturnRef(strValue));

        ON_CALL(m_objMockIAosAppContext, GetApp()).WillByDefault(Return(&m_objMockIAosApplication));

        m_pTestAosHandle = new TestAosHandle(static_cast<IAosAppContext*>(&m_objMockIAosAppContext),
                m_strAppId, m_strServiceId, m_nServiceType);

        m_pAosInfo = new AosInfo(static_cast<IAosAppContext*>(&m_objMockIAosAppContext));
        ASSERT_TRUE(m_pAosInfo != nullptr);
    }

    void TearDown() override
    {
        if (m_pAosInfo != nullptr)
        {
            delete m_pAosInfo;
            m_pAosInfo = nullptr;
        }
    }

    AString GetAssociatedUri() { return m_pAosInfo->GetAssociatedUri(); }
    IMS_SINT32 GetConnectionType() { return m_pAosInfo->GetConnectionType(); }
    IMS_UINT32 GetImsFeatures() { return m_pAosInfo->GetImsFeatures(); }
    IMS_UINT32 GetImsState() { return m_pAosInfo->GetImsState(); }
    IMS_SINT32 GetIpcanType() { return m_pAosInfo->GetIpcanType(); }
    AString GetLastPathHeaderValue() { return m_pAosInfo->GetLastPathHeaderValue(); }
    AString GetLocalAddress() { return m_pAosInfo->GetLocalAddress(); }
    IMS_UINT32 GetLocalPort() { return m_pAosInfo->GetLocalPort(); }
    IMS_UINT32 GetRegisteredNetworkType() { return m_pAosInfo->GetRegisteredNetworkType(); }
    AString GetPathHeaderValue() { return m_pAosInfo->GetPathHeaderValue(); }
    AString GetPcscfAddress() { return m_pAosInfo->GetPcscfAddress(); }
    IMS_UINT32 GetPcscfPort() { return m_pAosInfo->GetPcscfPort(); }
    IMS_UINT32 GetRegistrationMode() { return m_pAosInfo->GetRegistrationMode(); }
    AString GetSupportedHeaderValue() { return m_pAosInfo->GetSupportedHeaderValue(); }
    AString GetServiceRouteHeaderValue() { return m_pAosInfo->GetServiceRouteHeaderValue(); }
    void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized)
    {
        m_pAosInfo->NotifyEmergencyCallState(bIsInitialized);
    }
    void NotifyPublishState(IN IMS_BOOL bIsStarted) { m_pAosInfo->NotifyPublishState(bIsStarted); }
    void NotifyEmergencySmsState(IN IMS_BOOL bIsInitialized)
    {
        m_pAosInfo->NotifyEmergencySmsState(bIsInitialized);
    }
    void NotifyEpsfbCallState(IN IMS_UINT32 nState) { m_pAosInfo->NotifyEpsfbCallState(nState); }
    IMS_BOOL IsForbiddenBlock() { return m_pAosInfo->IsForbiddenBlock(); }
};

TEST_F(AosInfoTest, GetAssociatedUri_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_ASSOCIATED_URI)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(
            objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_ASSOCIATED_URI, _, _))
            .Times(1);

    GetAssociatedUri();
}

TEST_F(AosInfoTest, GetConnectionType_Test)
{
    // Expectation: Call AosConnection::GetConnectionType()

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
            .Times(1)
            .WillOnce(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));
    EXPECT_CALL(objMockIAosConnection, GetConnectionType())
            .Times(1)
            .WillOnce(Return(NetworkPolicy::APN_IMS));

    EXPECT_EQ(GetConnectionType(), NetworkPolicy::APN_IMS);
}

TEST_F(AosInfoTest, GetImsFeatures_Test1)
{
    // Test1: Ims connected
    // Expectation: Return binded features

    ImsMap<AString, IAosHandle*> objHandles;
    objHandles.Add(m_pTestAosHandle->GetServiceId(), static_cast<IAosHandle*>(m_pTestAosHandle));

    m_pTestAosHandle->SetHandleState(AosHandle::STATE_CONNECTED);
    m_pTestAosHandle->GetBindedFeatureTagList().AddFeature(ImsAosFeature::MMTEL);
    m_pTestAosHandle->GetBindedFeatureTagList().AddFeature(ImsAosFeature::VIDEO);

    EXPECT_CALL(m_objMockIAosAppContext, GetHandles()).Times(1).WillOnce(ReturnRef(objHandles));
    EXPECT_EQ(GetImsFeatures(), (ImsAosFeature::MMTEL | ImsAosFeature::VIDEO));
}

TEST_F(AosInfoTest, GetImsFeatures_Test2)
{
    // Test2: Ims disconnected
    // Expectation: Return None

    ImsMap<AString, IAosHandle*> objHandles;
    objHandles.Add(m_pTestAosHandle->GetServiceId(), static_cast<IAosHandle*>(m_pTestAosHandle));

    m_pTestAosHandle->GetBindedFeatureTagList().AddFeature(ImsAosFeature::MMTEL);
    m_pTestAosHandle->GetBindedFeatureTagList().AddFeature(ImsAosFeature::VIDEO);

    EXPECT_CALL(m_objMockIAosAppContext, GetHandles()).Times(1).WillOnce(ReturnRef(objHandles));
    EXPECT_EQ(GetImsFeatures(), ImsAosFeature::NONE);
}

TEST_F(AosInfoTest, GetImsState_Test1)
{
    // Test1: AppState: STATE_NOTREADY, IsForbiddenBlock: true
    // Expectation: return IMS_STATE_FORBIDDEN

    MockIAosApplication objMockIAosApplication;
    MockIAosBlock objMockIAosBlock;

    EXPECT_CALL(m_objMockIAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosApplication*>(&objMockIAosApplication)));
    EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosBlock*>(&objMockIAosBlock)));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .Times(1)
            .WillOnce(Return(IAosApplication::STATE_NOTREADY));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_IMS_DISABLED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_FORBIDDEN);
}

TEST_F(AosInfoTest, GetImsState_Test2)
{
    // Test2: AppState: STATE_NOTREADY, IsForbiddenBlock: false, BLOCK_SUBSCRIBER_INCOMPLETED
    // Expectation: return IMS_STATE_UNSUBSCRIBED

    MockIAosApplication objMockIAosApplication;
    MockIAosBlock objMockIAosBlock;

    EXPECT_CALL(m_objMockIAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosApplication*>(&objMockIAosApplication)));
    EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosBlock*>(&objMockIAosBlock)));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .Times(1)
            .WillOnce(Return(IAosApplication::STATE_NOTREADY));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_IMS_DISABLED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_USIM_AUTHENTICATION_FAILED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_TRUE));

    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_UNSUBSCRIBED);
}

TEST_F(AosInfoTest, GetImsState_Test3)
{
    // Test3: AppState: STATE_NOTREADY, IsForbiddenBlock: false, no BLOCK_SUBSCRIBER_INCOMPLETED
    // Expectation: return IMS_STATE_UNAVAILABLE

    MockIAosApplication objMockIAosApplication;
    MockIAosBlock objMockIAosBlock;

    EXPECT_CALL(m_objMockIAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosApplication*>(&objMockIAosApplication)));
    EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosBlock*>(&objMockIAosBlock)));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .Times(1)
            .WillOnce(Return(IAosApplication::STATE_NOTREADY));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_IMS_DISABLED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_USIM_AUTHENTICATION_FAILED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .Times(1)
            .WillOnce(Return(IMS_FALSE));

    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_UNAVAILABLE);
}

TEST_F(AosInfoTest, GetImsState_Test4)
{
    // Test4: AppState: Other than STATE_NOTREADY
    // Expectation: return IMS_STATE_PENDING if STATE_READY or STATE_CONNECTING
    //              return IMS_STATE_AVAILABLE if STATE_CONNECTED, STATE_UPDATING
    //              return IMS_STATE_UNAVAILABLE if STATE_DISCONNECTING

    MockIAosApplication objMockIAosApplication;

    EXPECT_CALL(m_objMockIAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosApplication*>(&objMockIAosApplication)));
    EXPECT_CALL(objMockIAosApplication, GetAppState())
            .Times(5)
            .WillOnce(Return(IAosApplication::STATE_READY))
            .WillOnce(Return(IAosApplication::STATE_CONNECTING))
            .WillOnce(Return(IAosApplication::STATE_CONNECTED))
            .WillOnce(Return(IAosApplication::STATE_UPDATING))
            .WillOnce(Return(IAosApplication::STATE_DISCONNECTING));

    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_PENDING);
    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_PENDING);
    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_AVAILABLE);
    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_AVAILABLE);
    EXPECT_EQ(GetImsState(), IImsAosInfo::IMS_STATE_UNAVAILABLE);
}

TEST_F(AosInfoTest, GetIpcanType_Test)
{
    // Expectation: Call AosConnection::GetIpcanCategory()

    MockIAosConnection objMockIAosConnection;
    EXPECT_CALL(m_objMockIAosAppContext, GetConnection())
            .Times(1)
            .WillOnce(Return(static_cast<IAosConnection*>(&objMockIAosConnection)));
    EXPECT_CALL(objMockIAosConnection, GetIpcanCategory())
            .Times(1)
            .WillOnce(Return(IIpcan::CATEGORY_MOBILE));

    EXPECT_EQ(GetIpcanType(), IIpcan::CATEGORY_MOBILE);
}

TEST_F(AosInfoTest, GetLastPathHeaderValue_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_LAST_PATH)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_LAST_PATH, _, _))
            .Times(1);

    GetLastPathHeaderValue();
}

TEST_F(AosInfoTest, GetLocalAddress_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_LOCAL_ADDRESS)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(
            objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_LOCAL_ADDRESS, _, _))
            .Times(1);

    GetLocalAddress();
}

TEST_F(AosInfoTest, GetLocalPort_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_LOCAL_PORT)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_LOCAL_PORT, _, _))
            .Times(1);

    GetLocalPort();
}

TEST_F(AosInfoTest, GetRegisteredNetworkType_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosApplication::PROPERTY_REGISTERED_RAT)

    MockIAosApplication objMockIAosApplication;

    EXPECT_CALL(m_objMockIAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosApplication*>(&objMockIAosApplication)));

    EXPECT_CALL(objMockIAosApplication, GetProperty(IAosApplication::PROPERTY_REGISTERED_RAT, _, _))
            .Times(1);

    GetRegisteredNetworkType();
}

TEST_F(AosInfoTest, GetPathHeaderValue_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_PATH)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_PATH, _, _))
            .Times(1);

    GetPathHeaderValue();
}

TEST_F(AosInfoTest, GetPcscfAddress_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_PCSCF_ADDRESS)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(
            objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_PCSCF_ADDRESS, _, _))
            .Times(1);

    GetPcscfAddress();
}

TEST_F(AosInfoTest, GetPcscfPort_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_PCSCF_PORT)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_PCSCF_PORT, _, _))
            .Times(1);

    GetPcscfPort();
}

TEST_F(AosInfoTest, GetRegistrationMode_Test1)
{
    // Test1: Other than IAosRegistration::MODE_FAKE
    // Expectation: return REG_MODE_NORMAL if IAosRegistration::MODE_NORMAL
    //              return REG_MODE_ADMIN if IAosRegistration::MODE_LIMITED
    //              return REG_MODE_UNKNOWN if invalid

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, GetMode())
            .Times(3)
            .WillOnce(Return(IAosRegistration::MODE_NORMAL))
            .WillOnce(Return(IAosRegistration::MODE_LIMITED))
            .WillOnce(Return(3));

    EXPECT_EQ(GetRegistrationMode(), IImsAosInfo::REG_MODE_NORMAL);
    EXPECT_EQ(GetRegistrationMode(), IImsAosInfo::REG_MODE_ADMIN);
    EXPECT_EQ(GetRegistrationMode(), IImsAosInfo::REG_MODE_UNKNOWN);
}

TEST_F(AosInfoTest, GetRegistrationMode_Test2)
{
    // Test2: IAosRegistration::MODE_FAKE,
    // Expectation: return REG_MODE_NOUICC if BLOCK_SUBSCRIBER_INCOMPLETED blocked
    //              return REG_MODE_INTERNAL if BLOCK_SUBSCRIBER_INCOMPLETED not blocked

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, GetMode())
            .Times(AnyNumber())
            .WillRepeatedly(Return(IAosRegistration::MODE_FAKE));

    MockIAosBlock objMockIAosBlock;
    EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosBlock*>(&objMockIAosBlock)));

    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_SUBSCRIBER_INCOMPLETED, _, _))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_EQ(GetRegistrationMode(), IImsAosInfo::REG_MODE_NOUICC);
    EXPECT_EQ(GetRegistrationMode(), IImsAosInfo::REG_MODE_INTERNAL);
}

TEST_F(AosInfoTest, GetSupportedHeaderValue_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_SUPPORTED)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_SUPPORTED, _, _))
            .Times(1);

    GetSupportedHeaderValue();
}

TEST_F(AosInfoTest, GetServiceRouteHeaderValue_Test)
{
    // Expectation: Call AosRegistration::GetProperty(IAosRegistration::PROPERTY_SERVICE_ROUTE)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(1)
            .WillOnce(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(
            objMockIAosRegistration, GetProperty(IAosRegistration::PROPERTY_SERVICE_ROUTE, _, _))
            .Times(1);

    GetServiceRouteHeaderValue();
}

TEST_F(AosInfoTest, NotifyEmergencyCallState_Test)
{
    // Expectation: Call AosRegistration::RequestCmd(IAosRegistration::CMD_ECALL_INIT)
    //              if param is true
    //              else Call AosRegistration::RequestCmd(IAosRegistration::CMD_ECALL_DONE)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_ECALL_INIT, _)).Times(1);
    EXPECT_CALL(objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_ECALL_DONE, _)).Times(1);
    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(IAosApplication::CMD_ECALL_INIT, _));

    NotifyEmergencyCallState(IMS_TRUE);
    NotifyEmergencyCallState(IMS_FALSE);
}

TEST_F(AosInfoTest, NotifyPublishState_Test)
{
    // Expectation:: Call IAosApplication::NotifyPublishState with input param.

    MockIAosApplication objMockIAosApplication;

    EXPECT_CALL(m_objMockIAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosApplication*>(&objMockIAosApplication)));

    EXPECT_CALL(objMockIAosApplication, NotifyPublishState(IMS_TRUE)).Times(1);
    EXPECT_CALL(objMockIAosApplication, NotifyPublishState(IMS_FALSE)).Times(1);

    NotifyPublishState(IMS_TRUE);
    NotifyPublishState(IMS_FALSE);
}

TEST_F(AosInfoTest, NotifyEmergencySmsState_Test)
{
    // Expectation: Call AosRegistration::RequestCmd(IAosRegistration::CMD_ESMS_INIT)
    //              if param is true
    //              else Call AosRegistration::RequestCmd(IAosRegistration::CMD_ESMS_DONE)

    MockIAosRegistration objMockIAosRegistration;
    EXPECT_CALL(m_objMockIAosAppContext, GetRegistration())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosRegistration*>(&objMockIAosRegistration)));
    EXPECT_CALL(objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_ESMS_INIT, _)).Times(1);
    EXPECT_CALL(objMockIAosRegistration, RequestCmd(IAosRegistration::CMD_ESMS_DONE, _)).Times(1);
    EXPECT_CALL(m_objMockIAosApplication, RequestCmd(IAosApplication::CMD_ESMS_INIT, _));

    NotifyEmergencySmsState(IMS_TRUE);
    NotifyEmergencySmsState(IMS_FALSE);
}

TEST_F(AosInfoTest, NotifyEpsfbCallState_Test)
{
    // Expectation: Call AosApplication::NotifyEpsFallbackCallState()

    MockIAosApplication objMockIAosApplication;

    EXPECT_CALL(m_objMockIAosAppContext, GetApp())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosApplication*>(&objMockIAosApplication)));

    EXPECT_CALL(objMockIAosApplication, NotifyEpsFallbackCallState(_)).Times(1);

    NotifyEpsfbCallState(1);
}

TEST_F(AosInfoTest, IsForbiddenBlock_Test)
{
    // Expectation: return true
    // if block is BLOCK_IMS_DISABLED or BLOCK_PERMANENT_REG_FAILED or BLOCK_AUTHENTICATION_FAILED
    // else return false

    MockIAosBlock objMockIAosBlock;

    EXPECT_CALL(m_objMockIAosAppContext, GetBlock())
            .Times(AnyNumber())
            .WillRepeatedly(Return(static_cast<IAosBlock*>(&objMockIAosBlock)));

    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_IMS_DISABLED, _, _))
            .Times(5)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_PERMANENT_REG_FAILED, _, _))
            .Times(4)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_AUTHENTICATION_FAILED, _, _))
            .Times(3)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));
    EXPECT_CALL(objMockIAosBlock, IsReasonBlocked(BLOCK_USIM_AUTHENTICATION_FAILED, _, _))
            .Times(2)
            .WillOnce(Return(IMS_TRUE))
            .WillOnce(Return(IMS_FALSE));

    EXPECT_TRUE(IsForbiddenBlock());
    EXPECT_TRUE(IsForbiddenBlock());
    EXPECT_TRUE(IsForbiddenBlock());
    EXPECT_TRUE(IsForbiddenBlock());
    EXPECT_FALSE(IsForbiddenBlock());
}