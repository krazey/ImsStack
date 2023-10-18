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
#include "subscribe/UceNotifyBodyPartData.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

class UceNotifyBodyPartDataTest : public ::testing::Test
{
public:
    UceNotifyBodyPartData* pUceNotifyBodyPartData;

protected:
    virtual void SetUp() override {}

    virtual void TearDown() override
    {
        if (pUceNotifyBodyPartData)
        {
            delete pUceNotifyBodyPartData;
        }
    }
};

TEST_F(UceNotifyBodyPartDataTest, GetContentType)
{
    IMS_TRACE_D("GetContentType", 0, 0, 0);
    const ByteArray objContent = ByteArray("test");
    AString strContentType = "ContentType";
    AString strContentId = "ContentId";
    pUceNotifyBodyPartData = new UceNotifyBodyPartData(strContentType, strContentId, objContent);
    ASSERT_TRUE(pUceNotifyBodyPartData != nullptr);

    AString contentType = pUceNotifyBodyPartData->GetContentType();
    EXPECT_STREQ(contentType.GetStr(), strContentType.GetStr());
}

TEST_F(UceNotifyBodyPartDataTest, GetContentId)
{
    IMS_TRACE_D("GetContentId", 0, 0, 0);
    const ByteArray objContent = ByteArray("test");
    AString strContentType = "ContentType";
    AString strContentId = "ContentId";
    pUceNotifyBodyPartData = new UceNotifyBodyPartData(strContentType, strContentId, objContent);
    AString contentId = pUceNotifyBodyPartData->GetContentId();
    EXPECT_STREQ(contentId.GetStr(), strContentId.GetStr());
}

TEST_F(UceNotifyBodyPartDataTest, GetBodyContent)
{
    IMS_TRACE_D("GetBodyContent", 0, 0, 0);
    const ByteArray objContent = ByteArray("test");
    AString strContentType = "ContentType";
    AString strContentId = "ContentId";
    pUceNotifyBodyPartData = new UceNotifyBodyPartData(strContentType, strContentId, objContent);
    ByteArray content = pUceNotifyBodyPartData->GetBodyContent();
    EXPECT_STREQ(content.ToString().GetStr(), objContent.ToString().GetStr());
}