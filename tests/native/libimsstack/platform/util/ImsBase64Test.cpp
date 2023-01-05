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

#include "ImsBase64.h"
#include "ImsNew.h"
#include "ImsStrLib.h"

namespace android
{

class ImsBase64Test : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}

protected:
    static const IMS_SIZE_T DST_DATA_LEN = 256;

    static const IMS_CHAR DATA_WITH_TWO_PADDING[];
    static const IMS_CHAR DATA_WITH_ONE_PADDING[];
    static const IMS_CHAR DATA_WITH_NO_PADDING[];

    static const IMS_CHAR BASE64_WITH_TWO_PADDING[];
    static const IMS_CHAR BASE64_WITH_ONE_PADDING[];
    static const IMS_CHAR BASE64_WITH_NO_PADDING[];

    static const IMS_CHAR DATA_FOR_CRLF[];
    static const IMS_CHAR BASE64_FOR_CRLF[];
};

const IMS_CHAR ImsBase64Test::DATA_WITH_TWO_PADDING[] = "Hello~! This is an IMS world";
const IMS_CHAR ImsBase64Test::DATA_WITH_ONE_PADDING[] = "Hello~! This is an IMS world.";
const IMS_CHAR ImsBase64Test::DATA_WITH_NO_PADDING[] = "Hello~! This is an IMS world..";

const IMS_CHAR ImsBase64Test::BASE64_WITH_TWO_PADDING[] =
        "SGVsbG9+ISBUaGlzIGlzIGFuIElNUyB3b3JsZA==";
const IMS_CHAR ImsBase64Test::BASE64_WITH_ONE_PADDING[] =
        "SGVsbG9+ISBUaGlzIGlzIGFuIElNUyB3b3JsZC4=";
const IMS_CHAR ImsBase64Test::BASE64_WITH_NO_PADDING[] = "SGVsbG9+ISBUaGlzIGlzIGFuIElNUyB3b3JsZC4u";

// clang-format off
const IMS_CHAR ImsBase64Test::DATA_FOR_CRLF[] =
        "Hello~! This is an IMS world. Welcome to the IMS team and "
        "we will go forward to achieve our goal in this year.";
const IMS_CHAR ImsBase64Test::BASE64_FOR_CRLF[] =
        "SGVsbG9+ISBUaGlzIGlzIGFuIElNUyB3b3JsZC4gV2VsY29tZSB0byB0aGUgSU1TIHRlYW0gYW5k\r\n"
        "IHdlIHdpbGwgZ28gZm9yd2FyZCB0byBhY2hpZXZlIG91ciBnb2FsIGluIHRoaXMgeWVhci4=";
// clang-format on

TEST_F(ImsBase64Test, Encode)
{
    IMS_SIZE_T nSrcLen = IMS_StrLen(DATA_WITH_TWO_PADDING);
    IMS_CHAR pszEncodedData[DST_DATA_LEN] = {
            0,
    };
    IMS_SINT32 nEncodedLen;

    nEncodedLen = ImsBase64_Encode(reinterpret_cast<const IMS_BYTE*>(DATA_WITH_TWO_PADDING),
            nSrcLen, pszEncodedData, DST_DATA_LEN, IMS_FALSE);

    EXPECT_EQ(nEncodedLen, IMS_StrLen(BASE64_WITH_TWO_PADDING));
    EXPECT_STREQ(BASE64_WITH_TWO_PADDING, pszEncodedData);

    nSrcLen = IMS_StrLen(DATA_WITH_ONE_PADDING);
    IMS_MEM_Memset(pszEncodedData, 0, DST_DATA_LEN);

    nEncodedLen = ImsBase64_Encode(reinterpret_cast<const IMS_BYTE*>(DATA_WITH_ONE_PADDING),
            nSrcLen, pszEncodedData, DST_DATA_LEN, IMS_FALSE);

    EXPECT_EQ(nEncodedLen, IMS_StrLen(BASE64_WITH_ONE_PADDING));
    EXPECT_STREQ(BASE64_WITH_ONE_PADDING, pszEncodedData);

    nSrcLen = IMS_StrLen(DATA_WITH_NO_PADDING);
    IMS_MEM_Memset(pszEncodedData, 0, DST_DATA_LEN);

    nEncodedLen = ImsBase64_Encode(reinterpret_cast<const IMS_BYTE*>(DATA_WITH_NO_PADDING), nSrcLen,
            pszEncodedData, DST_DATA_LEN, IMS_FALSE);

    EXPECT_EQ(nEncodedLen, IMS_StrLen(BASE64_WITH_NO_PADDING));
    EXPECT_STREQ(BASE64_WITH_NO_PADDING, pszEncodedData);

    nSrcLen = IMS_StrLen(DATA_FOR_CRLF);
    IMS_MEM_Memset(pszEncodedData, 0, DST_DATA_LEN);

    nEncodedLen = ImsBase64_Encode(reinterpret_cast<const IMS_BYTE*>(DATA_FOR_CRLF), nSrcLen,
            pszEncodedData, DST_DATA_LEN);

    EXPECT_EQ(nEncodedLen, IMS_StrLen(BASE64_FOR_CRLF));
    EXPECT_STREQ(BASE64_FOR_CRLF, pszEncodedData);
}

TEST_F(ImsBase64Test, Decode)
{
    IMS_SIZE_T nSrcLen = IMS_StrLen(BASE64_WITH_TWO_PADDING);
    IMS_CHAR pszDecodedData[DST_DATA_LEN] = {
            0,
    };

    IMS_SINT32 nDecodedLen = ImsBase64_Decode(BASE64_WITH_TWO_PADDING, nSrcLen,
            reinterpret_cast<IMS_BYTE*>(pszDecodedData), DST_DATA_LEN);

    EXPECT_EQ(nDecodedLen, IMS_StrLen(DATA_WITH_TWO_PADDING));
    EXPECT_STREQ(DATA_WITH_TWO_PADDING, pszDecodedData);

    nSrcLen = IMS_StrLen(BASE64_WITH_ONE_PADDING);
    IMS_MEM_Memset(pszDecodedData, 0, DST_DATA_LEN);

    nDecodedLen = ImsBase64_Decode(BASE64_WITH_ONE_PADDING, nSrcLen,
            reinterpret_cast<IMS_BYTE*>(pszDecodedData), DST_DATA_LEN);

    EXPECT_EQ(nDecodedLen, IMS_StrLen(DATA_WITH_ONE_PADDING));
    EXPECT_STREQ(DATA_WITH_ONE_PADDING, pszDecodedData);

    nSrcLen = IMS_StrLen(BASE64_WITH_NO_PADDING);
    IMS_MEM_Memset(pszDecodedData, 0, DST_DATA_LEN);

    nDecodedLen = ImsBase64_Decode(BASE64_WITH_NO_PADDING, nSrcLen,
            reinterpret_cast<IMS_BYTE*>(pszDecodedData), DST_DATA_LEN);

    EXPECT_EQ(nDecodedLen, IMS_StrLen(DATA_WITH_NO_PADDING));
    EXPECT_STREQ(DATA_WITH_NO_PADDING, pszDecodedData);

    nSrcLen = IMS_StrLen(BASE64_FOR_CRLF);
    IMS_MEM_Memset(pszDecodedData, 0, DST_DATA_LEN);

    nDecodedLen = ImsBase64_Decode(
            BASE64_FOR_CRLF, nSrcLen, reinterpret_cast<IMS_BYTE*>(pszDecodedData), DST_DATA_LEN);

    EXPECT_EQ(nDecodedLen, IMS_StrLen(DATA_FOR_CRLF));
    EXPECT_STREQ(DATA_FOR_CRLF, pszDecodedData);
}

}  // namespace android
