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

#include "IpSecSaParameter.h"
#include "network/OsIpSecSa.h"

namespace android
{

class OsIpSecSaTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(OsIpSecSaTest, Constructor)
{
    OsIpSecSa objOsIpSecSa;

    EXPECT_EQ(0, objOsIpSecSa.GetSpi());
}

TEST_F(OsIpSecSaTest, SetSa)
{
    OsIpSecSa objOsIpSecSa;

    IpAddress objSrcIp(AString("192.168.5.29"));
    IMS_UINT32 nSrcPort = 5060;
    IpAddress objDstIp(AString("192.168.4.49"));
    IMS_UINT32 nDstPort = 7090;
    IMS_UINT32 nSecurityProtocol = IpSecSaParameter::SECURITY_PROTOCOL_ESP;
    IMS_UINT32 nSpi = 21;
    IMS_UINT32 nMode = IpSecSaParameter::Policy::MODE_TUNNEL;
    IMS_UINT32 nAuthAlgorithm = IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96;
    IMS_UINT32 nEncryptionAlgorithm = IpSecSaParameter::ENCRYPTION_ALG_AES_CBC;
    ByteArray objAuthKey(AString("12345"));
    ByteArray objEncryptionKey(AString("67890"));

    objOsIpSecSa.SetSa(objSrcIp, nSrcPort, objDstIp, nDstPort, nSecurityProtocol, nSpi, nMode,
            nAuthAlgorithm, nEncryptionAlgorithm, objAuthKey, objEncryptionKey);

    const IpSecSaParameter& objIpSecSaParameter = objOsIpSecSa.CreateSaParameter(90);

    EXPECT_EQ(90, objIpSecSaParameter.GetIpSecId());
    EXPECT_EQ(IpSecSaParameter::SECURITY_PROTOCOL_ESP, objIpSecSaParameter.GetSecurityProtocol());
    EXPECT_EQ(IpSecSaParameter::INTEGRITY_ALG_HMAC_SHA_1_96,
            objIpSecSaParameter.GetIntegrityAlgorithm());
    EXPECT_EQ(
            IpSecSaParameter::ENCRYPTION_ALG_AES_CBC, objIpSecSaParameter.GetEncryptionAlgorithm());

    const IMS_BYTE byExtra[4] = {0, 0, 0, 0};
    objAuthKey.Append(byExtra, 4);

    EXPECT_EQ(objIpSecSaParameter.GetIk().ToString(), objAuthKey.ToString());
    EXPECT_EQ(objIpSecSaParameter.GetCk().ToString(), objEncryptionKey.ToString());
    EXPECT_EQ(0, objIpSecSaParameter.GetPolicys().GetSize());
}

}  // namespace android
