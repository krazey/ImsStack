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

#include "ImsSha1.h"
#include "ImsStrLib.h"

namespace android
{

class ImsSha1Test : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(ImsSha1Test, Sha1)
{
    const IMS_CHAR acSrcData[] = "This is an IMS world.";
    // clang-format off
    const IMS_UCHAR acSha1Data[IMS_SHA1_HASH_SIZE] = {
            0xbf, 0x0d, 0xd5, 0x08, 0x41, 0x20, 0xa0, 0xda, 0x67, 0xe1,
            0x12, 0xcf, 0x53, 0x0e, 0x16, 0x71, 0xaf, 0x97, 0x3d, 0xc9
    };
    // clang-format on

    ImsSha1Context objContext;
    IMS_UCHAR aucHash[IMS_SHA1_HASH_SIZE] = {
            0,
    };
    IMS_UINT32 nSrcLen = IMS_StrLen(acSrcData);

    ImsSha1_Initialize(&objContext);
    ImsSha1_Update(reinterpret_cast<const IMS_UCHAR*>(acSrcData), nSrcLen, &objContext);
    ImsSha1_Finalize(&objContext, aucHash);

    for (IMS_SINT32 i = 0; i < IMS_SHA1_HASH_SIZE; ++i)
    {
        EXPECT_EQ(acSha1Data[i], aucHash[i]);
    }
}

}  // namespace android
