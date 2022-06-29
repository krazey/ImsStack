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

#include "ImsHmac.h"
#include "ImsStrLib.h"

namespace android
{

class ImsHmacTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}

protected:
    static const IMS_CHAR HMAC_TEXT[];
    static const IMS_UCHAR HMAC_KEY[];
    static const IMS_SINT32 HMAC_KEY_LEN = 7;
};

const IMS_CHAR ImsHmacTest::HMAC_TEXT[] = "This is an IMS world.";
const IMS_UCHAR ImsHmacTest::HMAC_KEY[] = {0x63, 0x72, 0x79, 0x70, 0x74, 0x69, 0x69};

TEST_F(ImsHmacTest, Md5)
{
    // clang-format off
    const IMS_UCHAR aucMd5Data[IMS_HMAC_MD5_SIZE] = {
            0x17, 0x97, 0xd9, 0x3e, 0x06, 0x0b, 0xce, 0x0b,
            0x0a, 0x6a, 0x97, 0x78, 0x81, 0x8f, 0xa6, 0x5b
    };
    // clang-format on
    IMS_UCHAR aucDigest[IMS_HMAC_MD5_SIZE] = {
            0,
    };

    ImsHmac_Md5(reinterpret_cast<const IMS_UCHAR*>(HMAC_TEXT), IMS_StrLen(HMAC_TEXT), HMAC_KEY,
            HMAC_KEY_LEN, aucDigest);

    for (IMS_SINT32 i = 0; i < IMS_HMAC_MD5_SIZE; ++i)
    {
        EXPECT_EQ(aucMd5Data[i], aucDigest[i]);
    }
}

TEST_F(ImsHmacTest, Sha1)
{
    // clang-format off
    const IMS_UCHAR aucSha1Data[IMS_HMAC_SHA1_SIZE] = {
            0xb6, 0xfb, 0xc3, 0x52, 0x98, 0xbe, 0x1c, 0xf3, 0xc5, 0x7e,
            0xa2, 0x87, 0xe1, 0xc2, 0xf6, 0xf3, 0x44, 0x73, 0x2a, 0x44
    };
    // clang-format on
    IMS_UCHAR aucHash[IMS_HMAC_SHA1_SIZE] = {
            0,
    };

    ImsHmac_Sha1(reinterpret_cast<const IMS_UCHAR*>(HMAC_TEXT), IMS_StrLen(HMAC_TEXT), HMAC_KEY,
            HMAC_KEY_LEN, aucHash);

    for (IMS_SINT32 i = 0; i < IMS_HMAC_SHA1_SIZE; ++i)
    {
        EXPECT_EQ(aucSha1Data[i], aucHash[i]);
    }
}

}  // namespace android
