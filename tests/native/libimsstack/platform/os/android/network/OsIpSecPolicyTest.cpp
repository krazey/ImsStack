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
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ImsIpSecType.h"
#include "IpSecSaParameter.h"
#include "MockIIpSecPolicyListener.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "network/OsIpSecPolicy.h"
#include "network/OsIpSecSa.h"
#include "network/OsIpSecSp.h"

using ::testing::_;
using ::testing::Invoke;

namespace android
{

const IMS_SINT32 POLICY_ID = 25;

class OsIpSecPolicyTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pObjOsIpSecPolicy = new OsIpSecPolicy(POLICY_ID);
        m_pObjOsIpSecPolicy->SetListener(&objMockIIpSecPolicyListener);

        m_pObjTimerService = new TestTimerService();
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, m_pObjTimerService);
    }
    virtual void TearDown() override
    {
        delete m_pObjOsIpSecPolicy;
        delete m_pObjTimerService;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }

protected:
    MockIIpSecPolicyListener objMockIIpSecPolicyListener;
    OsIpSecPolicy* m_pObjOsIpSecPolicy;
    TestTimerService* m_pObjTimerService;
};

TEST_F(OsIpSecPolicyTest, GetId)
{
    EXPECT_EQ(POLICY_ID, m_pObjOsIpSecPolicy->GetId());
}

TEST_F(OsIpSecPolicyTest, CreateSp)
{
    IIpSecSp* pIpSecSp = m_pObjOsIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);
}

TEST_F(OsIpSecPolicyTest, DestroySp)
{
    IIpSecSp* pIpSecSp = m_pObjOsIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    const ImsList<OsIpSecSp*>& objPolicyList = m_pObjOsIpSecPolicy->GetSPs();
    EXPECT_EQ(1, objPolicyList.GetSize());

    m_pObjOsIpSecPolicy->DestroySp(pIpSecSp);

    EXPECT_EQ(0, objPolicyList.GetSize());
}

TEST_F(OsIpSecPolicyTest, CreateSa)
{
    IIpSecSa* pIpSecSa = m_pObjOsIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);
}

TEST_F(OsIpSecPolicyTest, DestroySa)
{
    IIpSecSa* pIpSecSa = m_pObjOsIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    const ImsList<OsIpSecSa*>& objAssociationList = m_pObjOsIpSecPolicy->GetSAs();
    EXPECT_EQ(1, objAssociationList.GetSize());

    m_pObjOsIpSecPolicy->DestroySa(pIpSecSa);

    EXPECT_EQ(0, objAssociationList.GetSize());
}

TEST_F(OsIpSecPolicyTest, ManageLifetime)
{
    MockITimer& objMockTimer = m_pObjTimerService->GetMockTimer();
    ITimerListener* pPolicyTimerListener = IMS_NULL;
    IMS_UINT32 nTimerDuration = 0;

    ON_CALL(objMockTimer, SetTimer)
            .WillByDefault(Invoke(
                    [&](IMS_UINT32 nDuration, ITimerListener* pTimerListener)
                    {
                        nTimerDuration = nDuration;
                        pPolicyTimerListener = pTimerListener;
                        return 1;
                    }));

    IMS_UINT32 nLifeTime = 1000;
    EXPECT_CALL(objMockTimer, SetTimer(nLifeTime, m_pObjOsIpSecPolicy));
    m_pObjOsIpSecPolicy->ManageLifetime(nLifeTime);

    ASSERT_TRUE(pPolicyTimerListener != nullptr);
    EXPECT_EQ(nLifeTime, nTimerDuration);

    EXPECT_CALL(objMockIIpSecPolicyListener, IpSecPolicy_OnSecurityAssociationExpired(_)).Times(0);
    pPolicyTimerListener->Timer_TimerExpired(IMS_NULL);

    MockITimer objDifferentMockTimer;
    pPolicyTimerListener->Timer_TimerExpired(&objDifferentMockTimer);

    EXPECT_CALL(objMockIIpSecPolicyListener, IpSecPolicy_OnSecurityAssociationExpired(_)).Times(1);
    pPolicyTimerListener->Timer_TimerExpired(&objMockTimer);
}

TEST_F(OsIpSecPolicyTest, DestroyAllSas)
{
    IIpSecSa* pIpSecSa = m_pObjOsIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    pIpSecSa = nullptr;
    pIpSecSa = m_pObjOsIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    const ImsList<OsIpSecSa*>& objAssociationList = m_pObjOsIpSecPolicy->GetSAs();
    EXPECT_EQ(2, objAssociationList.GetSize());

    IIpSecSp* pIpSecSp = m_pObjOsIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    pIpSecSp = nullptr;
    pIpSecSp = m_pObjOsIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    const ImsList<OsIpSecSp*>& objPolicyList = m_pObjOsIpSecPolicy->GetSPs();
    EXPECT_EQ(2, objPolicyList.GetSize());

    m_pObjOsIpSecPolicy->DestroyAllSas();
    EXPECT_EQ(0, objAssociationList.GetSize());
    EXPECT_EQ(0, objPolicyList.GetSize());
}

TEST_F(OsIpSecPolicyTest, FindSp)
{
    EXPECT_TRUE(nullptr == m_pObjOsIpSecPolicy->FindSp(20));

    SocketAddress objLocalAddress(AString("192.168.2.3"), 5060);
    SocketAddress objRemoteAddress(AString("192.168.5.9"), 5080);
    IMS_UINT32 nTransportProtocol = IpSecSaParameter::Policy::TRANSPORT_PROTOCOL_UDP;
    IMS_UINT32 nAction = IpSecType::ACTION_APPLY;
    IMS_UINT32 nDirection = IpSecSaParameter::Policy::DIRECTION_OUT;
    IMS_UINT32 nSpi = 21;
    IMS_UINT32 nMode = IpSecSaParameter::Policy::MODE_TUNNEL;

    IIpSecSp* pIpSecSp = m_pObjOsIpSecPolicy->CreateSp();
    ASSERT_TRUE(pIpSecSp != nullptr);

    static_cast<OsIpSecSp*>(pIpSecSp)->SetTransportInfo(objLocalAddress.GetAddress(),
            objLocalAddress.GetPort(), objRemoteAddress.GetAddress(), objRemoteAddress.GetPort(),
            nTransportProtocol, nAction, nDirection, nSpi, nMode);

    EXPECT_TRUE(nullptr == m_pObjOsIpSecPolicy->FindSp(20));

    OsIpSecSp* pOsIpSecSp = m_pObjOsIpSecPolicy->FindSp(21);
    ASSERT_TRUE(pOsIpSecSp != nullptr);
    EXPECT_EQ(21, pOsIpSecSp->GetSpi());
}

TEST_F(OsIpSecPolicyTest, FindSa)
{
    EXPECT_TRUE(nullptr == m_pObjOsIpSecPolicy->FindSa(20));

    SocketAddress objLocalAddress(AString("192.168.2.3"), 5060);
    SocketAddress objRemoteAddress(AString("192.168.5.9"), 5080);
    IMS_UINT32 nSecurityProtocol = IpSecSaParameter::SECURITY_PROTOCOL_ESP;
    IMS_UINT32 nSpi = 21;
    IMS_UINT32 nMode = IpSecSaParameter::Policy::MODE_TUNNEL;
    IMS_UINT32 nAuthAlgorithm = IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96;
    IMS_UINT32 nEncryptionAlgorithm = IpSecSaParameter::ENCRYPTION_ALG_AES_CBC;
    ByteArray objAuthKey(AString("12345"));
    ByteArray objEncryptionKey(AString("67890"));

    IIpSecSa* pIpSecSa = m_pObjOsIpSecPolicy->CreateSa();
    ASSERT_TRUE(pIpSecSa != nullptr);

    static_cast<OsIpSecSa*>(pIpSecSa)->SetSa(objLocalAddress.GetAddress(),
            objLocalAddress.GetPort(), objRemoteAddress.GetAddress(), objRemoteAddress.GetPort(),
            nSecurityProtocol, nSpi, nMode, nAuthAlgorithm, nEncryptionAlgorithm, objAuthKey,
            objEncryptionKey);

    EXPECT_TRUE(nullptr == m_pObjOsIpSecPolicy->FindSa(20));

    OsIpSecSa* pOsIpSecSa = m_pObjOsIpSecPolicy->FindSa(21);
    ASSERT_TRUE(pOsIpSecSa != nullptr);
    EXPECT_EQ(21, pOsIpSecSa->GetSpi());
}

}  // namespace android