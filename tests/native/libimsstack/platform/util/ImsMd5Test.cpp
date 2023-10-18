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

#include "ImsMd5.h"
#include "ImsStrLib.h"

namespace android
{

class ImsMd5Test : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(ImsMd5Test, Md5)
{
    const IMS_CHAR acSrcData[] = "This is an IMS world.";
    // clang-format off
    const IMS_UCHAR acMd5Data[IMS_MD5_DIGEST_SIZE] = {
            0x89, 0xa0, 0xbd, 0x9a, 0x96, 0xe3, 0x68, 0xb1,
            0x4b, 0xe9, 0x75, 0x03, 0x81, 0xbe, 0x73, 0xd5
    };
    // clang-format on

    ImsMd5Context objContext;
    IMS_UCHAR aucDigest[IMS_MD5_DIGEST_SIZE] = {
            0,
    };
    IMS_UINT32 nSrcLen = IMS_StrLen(acSrcData);

    ImsMd5_Initialize(&objContext);
    ImsMd5_Update(reinterpret_cast<const IMS_UCHAR*>(acSrcData), nSrcLen, &objContext);
    ImsMd5_Finalize(&objContext, aucDigest);

    for (IMS_SINT32 i = 0; i < IMS_MD5_DIGEST_SIZE; ++i)
    {
        EXPECT_EQ(acMd5Data[i], aucDigest[i]);
    }
}

}  // namespace android
