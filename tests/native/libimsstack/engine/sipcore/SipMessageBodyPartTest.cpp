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

#include "SipMessageBodyPart.h"

namespace android
{

class SipMessageBodyPartTest : public ::testing::Test
{
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipMessageBodyPartTest, Clone)
{
    AString strContent("This is a text message.");
    SipMsgBody* pMsgBody = new SipMsgBody();

    ASSERT_TRUE(pMsgBody != nullptr);

    pMsgBody->SetMsgBuffer(strContent.GetStr(), strContent.GetLength());

    SipMessageBodyPart objMsgBodyPart(pMsgBody);
    SipMessageBodyPart* pClonedMsgBodyPart =
            static_cast<SipMessageBodyPart*>(objMsgBodyPart.Clone());

    pMsgBody->SipDelete();

    ASSERT_TRUE(pClonedMsgBodyPart != nullptr);

    AString strClonedContent = pClonedMsgBodyPart->GetContent().ToString();

    EXPECT_EQ(strContent, strClonedContent);

    pClonedMsgBodyPart->Destroy();
}

}  // namespace android
