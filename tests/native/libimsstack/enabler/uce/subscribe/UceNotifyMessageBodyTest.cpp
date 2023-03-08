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
#include "subscribe/UceNotifyMessageBody.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

class UceNotifyMessageBodyTest : public ::testing::Test
{
public:
    UceNotifyMessageBody* pUceNotifyMessageBody;

protected:
    virtual void SetUp() override
    {
        pUceNotifyMessageBody = new UceNotifyMessageBody();
        ASSERT_TRUE(pUceNotifyMessageBody != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceNotifyMessageBody)
        {
            delete pUceNotifyMessageBody;
        }
    }
};

TEST_F(UceNotifyMessageBodyTest, SetContentType)
{
    IMS_TRACE_D("SetContentType", 0, 0, 0);
    AString contentType = "contentType";
    pUceNotifyMessageBody->SetContentType(contentType);
    EXPECT_STREQ(pUceNotifyMessageBody->GetContentType().GetStr(), contentType.GetStr());
}

TEST_F(UceNotifyMessageBodyTest, SetNotifyBodyPartData)
{
    IMS_TRACE_D("SetNotifyBodyPartData", 0, 0, 0);
    AString contentType = "contentType";
    AString contentId = "contentId";
    const ByteArray objContent = ByteArray("test");
    UceNotifyBodyPartData* pData = new UceNotifyBodyPartData(contentType, contentId, objContent);
    pUceNotifyMessageBody->SetNotifyBodyPartData(pData);

    ImsList<UceNotifyBodyPartData*> list = pUceNotifyMessageBody->GetNotifyBodyPartDatas();
    EXPECT_EQ(1, list.GetSize());
    UceNotifyBodyPartData* pRet = list.GetAt(0);
    EXPECT_STREQ(contentType.GetStr(), pRet->GetContentType().GetStr());
    EXPECT_STREQ(contentId.GetStr(), pRet->GetContentId().GetStr());
    EXPECT_STREQ(objContent.ToString().GetStr(), pRet->GetBodyContent().ToString().GetStr());
}
