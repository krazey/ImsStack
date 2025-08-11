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

#include "ImsDigest.h"

namespace android
{

class ImsDigestTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(ImsDigestTest, CalculateResponse)
{
    HASHHEX hEntity = {
            0,
    };
    IMS_CHAR acA1[HASHHEX_SIZE + 1] = {
            0,
    };
    IMS_CHAR acResponse[HASHHEX_SIZE + 1] = {
            0,
    };
    AString strUserName("310410123456789@private.att.net");
    AString strPassword("xxxx");
    AString strRealm("one.att.net");
    AString strNonce("33OwNiwLSXwCAowt1cRhYTUoDk97CIAA33KyNSgMz3s=");
    AString strCNonce("MzhkZTcxNjQ");

    ImsDigest_CalculateEntity("Volte only", hEntity);

    // Calculate the H(A1)
    ImsDigest_CalculateA1(
            "AKAv1-MD5", strUserName, strRealm, strPassword, strNonce, strCNonce, acA1);
    EXPECT_STREQ(acA1, "634a9b99a3db5d06f0fd1b8c4b4ab18e");

    // Calculate the response digest
    ImsDigest_CalculateResponse(acA1, strNonce, "00000002", strCNonce, "auth", "REGISTER",
            "sip:one.att.net", hEntity, acResponse);

    EXPECT_STREQ(acResponse, "90686613c9e29457f98d59fc84d33416");
}

}  // namespace android
