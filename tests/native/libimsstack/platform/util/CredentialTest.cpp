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

#include "Credential.h"
#include "ImsStrLib.h"
#include "ImsHmac.h"

namespace android
{

class CredentialTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(CredentialTest, SetCredentials)
{
    Credential objCredential;
    AString strUserName("405861079851317@ims.mnc861.mcc405.3gppnetwork.org");
    AString strPassword("xxxx");
    AString strRealm("ims.mnc861.mcc405.3gppnetwork.org");

    objCredential.SetCredentials(strUserName, strPassword, strRealm);
    EXPECT_EQ(objCredential.GetPassword(), strPassword);
    EXPECT_EQ(objCredential.GetRealm(), strRealm);
    EXPECT_EQ(objCredential.GetUsername(), strUserName);

    ByteArray objRes("response");
    objCredential.SetAkaResponse(ImsAkaParam::RESULT_OK, objRes);
    strPassword = "response";
    EXPECT_EQ(objCredential.GetPassword(), strPassword);
    ImsAkaParam objAkaParam = objCredential.GetAkaResponse();
    EXPECT_EQ(objAkaParam.m_nStatus, ImsAkaParam::RESULT_OK);
    EXPECT_EQ(objAkaParam.m_strAuts, AString::ConstNull());

    Credential objCopyCredential(objCredential);
    objCopyCredential.IsSameRealm(strRealm);
    EXPECT_EQ(objCopyCredential.GetType(), Credential::TYPE_MD5);

    ByteArray objAuts("AuthenticationFailed");
    objCopyCredential.SetAkaResponse(
            ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED, ByteArray::ConstNull(), objAuts);
    strPassword = AString::ConstEmpty();
    EXPECT_EQ(objCopyCredential.GetPassword(), strPassword);

    objAkaParam = objCopyCredential.GetAkaResponse();
    EXPECT_EQ(objAkaParam.m_nStatus, ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED);
    EXPECT_EQ(objAkaParam.m_strAuts, objAuts.ToString().ToBase64());
}

TEST_F(CredentialTest, SetAkaResponse)
{
    AString strUserName("1111@vzw.com");
    AString strPassword("0000");
    AString strRealm("vzw.com");
    Credential objCredential(strUserName, strPassword, strRealm);

    ByteArray objRes("response");
    ByteArray objIk("integrity0001");
    ByteArray objCk("cipher0001");
    objCredential.SetAkaResponse(ImsAkaParam::RESULT_OK, objRes, objIk, objCk);
    strPassword = "TdYfuQBYToQ3tpgdiEMkXQ==";
    EXPECT_STREQ(objCredential.GetPassword().GetStr(), strPassword.GetStr());

    Credential objCopyCredential(objCredential);
    objCopyCredential.IsSameRealm(strRealm);
    EXPECT_EQ(objCopyCredential.GetType(), Credential::TYPE_MD5);

    ByteArray objAuts("AuthFailed");
    objCopyCredential.SetAkaResponse(
            ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED, objRes, objIk, objCk, objAuts);
    strPassword = AString::ConstEmpty();
    EXPECT_EQ(objCopyCredential.GetPassword(), strPassword);

    ImsAkaParam objAkaParam = objCopyCredential.GetAkaResponse();
    EXPECT_EQ(objAkaParam.m_nStatus, ImsAkaParam::RESULT_NOK_SQN_SYNC_FAILED);
    EXPECT_EQ(objAkaParam.m_strAuts, objAuts.ToString().ToBase64());
}

TEST_F(CredentialTest, DeriveCkForAkav2)
{
    ByteArray objEmpty(ByteArray::ConstNull());
    EXPECT_EQ(Credential::DeriveCkForAkav2(objEmpty).ToString(), objEmpty.ToString());
    ByteArray objCk("1?\x3\x13\x8D\x99t\x87\xD9\x89\xCB\xDD\xA8\xE2\x84\xF3");
    ByteArray objKey("temp");
    EXPECT_EQ(Credential::DeriveCkForAkav2(objKey).ToString(), objCk.ToString());
}

TEST_F(CredentialTest, DeriveIkForAkav2)
{
    ByteArray objEmpty(ByteArray::ConstNull());
    EXPECT_EQ(Credential::DeriveIkForAkav2(objEmpty).ToString(), objEmpty.ToString());
    ByteArray objIk("\xF5\xF7mB/o8^DU\x99\x1F\x8F\a\xBC\xE0");
    ByteArray objKey("integrity22");
    EXPECT_EQ(Credential::DeriveIkForAkav2(objKey).ToString(), objIk.ToString());
}

TEST_F(CredentialTest, TranslateAlgorithm)
{
    EXPECT_EQ(Credential::TranslateAlgorithm("AKAv1-MD5"), Credential::TYPE_AKAv1_MD5);
    EXPECT_EQ(Credential::TranslateAlgorithm("AKAv2-MD5"), Credential::TYPE_AKAv2_MD5);
    EXPECT_EQ(Credential::TranslateAlgorithm("ssl"), Credential::TYPE_MD5);
}

}  // namespace android
