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
#include <gmock/gmock.h>
#include "subscribe/UceRlmiComposer.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

class UceRlmiComposerTest : public ::testing::Test
{
public:
    UceRlmiComposer* pUceRlmiComposer;

protected:
    virtual void SetUp() override
    {
        pUceRlmiComposer = new UceRlmiComposer();
        ASSERT_TRUE(pUceRlmiComposer != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceRlmiComposer)
        {
            delete pUceRlmiComposer;
        }
    }
};

TEST_F(UceRlmiComposerTest, ComposeRLMIList)
{
    IMS_TRACE_D("ComposeRLMIList", 0, 0, 0);
    IMSList<AString> pContactInfoList;
    AString user1 = "+12345678901";
    AString user2 = "+12345678902";
    pContactInfoList.Append(user1);
    pContactInfoList.Append(user2);

    AString expectedXml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
    expectedXml += "\n";
    expectedXml += "<resource-lists xmlns=\"urn:ietf:params:xml:ns:resource-lists\"";
    expectedXml += " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">";
    expectedXml += "\n";
    expectedXml += "<list name=\"dummy-rfc5367\">";
    expectedXml += "\n";
    expectedXml += "<entry uri=\"tel:";
    expectedXml += user1;
    expectedXml += "\"></entry>";
    expectedXml += "\n";
    expectedXml += "<entry uri=\"tel:";
    expectedXml += user2;
    expectedXml += "\"></entry>";
    expectedXml += "\n";
    expectedXml += "</list>";
    expectedXml += "\n";
    expectedXml += "</resource-lists>";
    expectedXml += "\n";

    AString pidfXml = pUceRlmiComposer->ComposeRLMIList(pContactInfoList);
    EXPECT_STREQ(expectedXml.GetStr(), pidfXml.GetStr());
}