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

#include "SipDatatypes.h"
#include "SipMessageBuffer.h"

namespace android
{

class SipMessageBufferTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipMessageBufferTest, SetGetAndUpdate)
{
    RcPtr<SipMessageBuffer> pMsgBuffer = SipMessageBuffer::GetInstance();
    EXPECT_EQ(SipMessageBuffer::MAX_MSG_SIZE, pMsgBuffer->GetLength());

    const SIP_UCHAR* pBuffer = pMsgBuffer->GetBuffer();
    EXPECT_TRUE(pBuffer != nullptr);

    pBuffer = pMsgBuffer->GetBuffer(0);
    EXPECT_TRUE(pBuffer != nullptr);
}

}  // namespace android