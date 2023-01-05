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

#include "registration/MockAosIpsec.h"
#include "../../../platform/interface/MockIIpSecPolicy.h"

#include "ImsIpSecType.h"
#include "registration/AosIpsec.h"

const IMS_SINT32 SLOT_ID = 0;

class AosIpsecTest : public ::testing::Test
{
public:
    AosIpsec* pAosIpsec;

    MockIAosIpsecListener objIAosIpsecListener;

protected:
    virtual void SetUp() override
    {
        pAosIpsec = new AosIpsec(static_cast<IAosIpsecListener*>(&objIAosIpsecListener), SLOT_ID);
        ASSERT_TRUE(pAosIpsec != nullptr);
    }

    virtual void TearDown() override
    {
        if (pAosIpsec)
        {
            delete pAosIpsec;
        }
    }

    void SetIIpsecPolicy(IN IIpSecPolicy* piPolicy) { pAosIpsec->m_piPolicy = piPolicy; }

    IIpSecPolicy* GetIpsecPolicy() { return pAosIpsec->m_piPolicy; }

    void SetIgnorePolicyExpired(IN IMS_BOOL bIgnorePolicyExpired)
    {
        pAosIpsec->m_bIgnorePolicyExpired = bIgnorePolicyExpired;
    }

    IMS_UINT32 GetSecurityProtocol() { return pAosIpsec->m_nSecuProto; }

    IMS_UINT32 GetEncryptionlgorithm() { return pAosIpsec->m_nEncrAlgo; }

    void CreateSpforTcp(IN IMS_UINT32 nType) { pAosIpsec->CreateSpforTcp(nType); }

    void CreateSa(IN IMS_UINT32 nType) { pAosIpsec->CreateSa(nType); }
};

TEST_F(AosIpsecTest, IpSecPolicy_OnSecurityAssociationExpired)
{
    IIpSecPolicy* origpiPolicy = GetIpsecPolicy();
    MockIIpSecPolicy objMockIIpsecPolicy;
    SetIIpsecPolicy(static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));

    pAosIpsec->IpSecPolicy_OnSecurityAssociationExpired(IMS_NULL);

    SetIgnorePolicyExpired(IMS_TRUE);
    pAosIpsec->IpSecPolicy_OnSecurityAssociationExpired(
            static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));

    SetIIpsecPolicy(origpiPolicy);

    // Hold the below due to " delete this."
    // SetIgnorePolicyExpired(IMS_FALSE);
    // EXPECT_CALL(objIAosIpsecListener, IPSecPolicyExpired())
    // pAosIpsec->IpSecPolicy_OnSecurityAssociationExpired(
    //        static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
}

TEST_F(AosIpsecTest, CreateUePort)
{
    EXPECT_GE(39000, pAosIpsec->CreateUePort());
    EXPECT_LE(38001, pAosIpsec->CreateUePort());
}

TEST_F(AosIpsecTest, CreateUeSpi)
{
    EXPECT_LE(1000000000, pAosIpsec->CreateUeSpi());
}

TEST_F(AosIpsecTest, AddPolicy)
{
    // m_piPolicy == IMS_NULL
    IIpSecPolicy* origpiPolicy = GetIpsecPolicy();
    SetIIpsecPolicy(IMS_NULL);

    EXPECT_FALSE(pAosIpsec->AddPolicy());
    SetIIpsecPolicy(origpiPolicy);
}

TEST_F(AosIpsecTest, SetSecurityAlgorithm)
{
    pAosIpsec->SetSecurityAlgorithm(IpSecType::SECURITY_PROTOCOL_AH,
            SipSecurityHeader::ALG_HMAC_MD5_96, SipSecurityHeader::EALG_DES_EDE3_CBC);
    EXPECT_EQ(GetSecurityProtocol(), IpSecType::SECURITY_PROTOCOL_AH);
    EXPECT_EQ(pAosIpsec->GetIntegrityAlgorithm(), SipSecurityHeader::ALG_HMAC_MD5_96);
    EXPECT_EQ(GetEncryptionlgorithm(), SipSecurityHeader::EALG_DES_EDE3_CBC);

    pAosIpsec->SetSecurityAlgorithm(IpSecType::SECURITY_PROTOCOL_ESP,
            SipSecurityHeader::ALG_HMAC_SHA_1_96, SipSecurityHeader::EALG_AES_CBC);
    EXPECT_EQ(GetSecurityProtocol(), IpSecType::SECURITY_PROTOCOL_ESP);
    EXPECT_EQ(pAosIpsec->GetIntegrityAlgorithm(), SipSecurityHeader::ALG_HMAC_SHA_1_96);
    EXPECT_EQ(GetEncryptionlgorithm(), SipSecurityHeader::EALG_AES_CBC);

    pAosIpsec->SetSecurityAlgorithm(IpSecType::SECURITY_PROTOCOL_ESP,
            SipSecurityHeader::ALG_HMAC_SHA_1_96, SipSecurityHeader::EALG_NULL);
    EXPECT_EQ(GetSecurityProtocol(), IpSecType::SECURITY_PROTOCOL_ESP);
    EXPECT_EQ(pAosIpsec->GetIntegrityAlgorithm(), SipSecurityHeader::ALG_HMAC_SHA_1_96);
    EXPECT_EQ(GetEncryptionlgorithm(), SipSecurityHeader::EALG_NULL);

    pAosIpsec->SetSecurityAlgorithm(IpSecType::SECURITY_PROTOCOL_ESP,
            SipSecurityHeader::ALG_HMAC_SHA_1_96, SipSecurityHeader::EALG_UNSPECIFIED);
    EXPECT_EQ(GetSecurityProtocol(), IpSecType::SECURITY_PROTOCOL_ESP);
    EXPECT_EQ(pAosIpsec->GetIntegrityAlgorithm(), SipSecurityHeader::ALG_HMAC_SHA_1_96);
    EXPECT_EQ(GetEncryptionlgorithm(), IpSecType::ENCRYPTION_ALGORITHM_NO);
}

TEST_F(AosIpsecTest, SetPcscfPortsAndSpis)
{
    pAosIpsec->SetPcscfPortsAndSpis(38002, 39002, 12345678, 87654321);
    EXPECT_EQ(pAosIpsec->GetPcscfPort(AosIpsec::TYPE_CLIENT), 38002);
    EXPECT_EQ(pAosIpsec->GetPcscfPort(AosIpsec::TYPE_SERVER), 39002);
    EXPECT_EQ(pAosIpsec->GetPcscfSpi(AosIpsec::TYPE_CLIENT), 12345678);
    EXPECT_EQ(pAosIpsec->GetPcscfSpi(AosIpsec::TYPE_SERVER), 87654321);
}

TEST_F(AosIpsecTest, GetPolicy)
{
    IIpSecPolicy* origpiPolicy = GetIpsecPolicy();
    MockIIpSecPolicy objMockIIpsecPolicy;
    SetIIpsecPolicy(IMS_NULL);

    EXPECT_EQ(IMS_NULL, pAosIpsec->GetPolicy());

    SetIIpsecPolicy(static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy));
    EXPECT_EQ(static_cast<IIpSecPolicy*>(&objMockIIpsecPolicy), pAosIpsec->GetPolicy());

    SetIIpsecPolicy(origpiPolicy);
}

TEST_F(AosIpsecTest, CreateSpSa)
{
    // for else (0 ~ 3)
    CreateSpforTcp(4);
    CreateSa(4);
}