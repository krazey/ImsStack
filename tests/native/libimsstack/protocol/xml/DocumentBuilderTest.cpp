/*
 * Copyright (C) 2025 The Android Open Source Project
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

#include "AString.h"
#include "ServiceTrace.h"

#include "DocumentBuilder.h"
#include "IDocument.h"
#include "IElement.h"
#include "INode.h"
#include "INodeList.h"
#include "IText.h"

__IMS_TRACE_TAG_XML__;

namespace android
{

class TestDocumentBuilder : public DocumentBuilder
{
public:
    inline TestDocumentBuilder() :
            DocumentBuilder()
    {
    }
    inline ~TestDocumentBuilder() override = default;
};

class DocumentBuilderTest : public ::testing::Test
{
protected:
    virtual void SetUp() override { m_pBuilder = new TestDocumentBuilder(); }
    virtual void TearDown() override { delete m_pBuilder; }

public:
    TestDocumentBuilder* m_pBuilder;
};

TEST_F(DocumentBuilderTest, ParseWithUssdXmlBody)
{
    const IMS_CHAR* pszXmlForUssd("<?xml version=\"1.0\"?>\n"
                                  "<ussd-data>\n"
                                  "  <ussd-string>\n"
                                  "16 Unlimited &amp; RM15/7 Days\n"
                                  "1 Buy &gt; Data Plans\n"
                                  "2 Top Up &quot; Services\n"
                                  "3 Download &apos; MyUMobile App\n"
                                  "4 Roam &lt; Travel\n"
                                  "5 Services\n"
                                  "6 My Account & Balance\n"
                                  "88 Previous\n"
                                  "</ussd-string>\n"
                                  "  <anyExt>\n"
                                  "    <UnstructuredSS-Request/>\n"
                                  "  </anyExt>\n"
                                  "</ussd-data>\n");
    IDocument* piDocument = m_pBuilder->Parse(pszXmlForUssd);

    ASSERT_NE(piDocument, nullptr);

    const IElement* piElement = piDocument->GetDocumentElement();

    ASSERT_NE(piElement, nullptr);
    EXPECT_EQ(piElement->GetTagName(), AString("ussd-data"));

    const INodeList* piNodeList = piElement->GetChildNodes();

    ASSERT_NE(piNodeList, nullptr);

    for (IMS_SINT32 i = 0; i < piNodeList->GetLength(); i++)
    {
        const INode* piNode = piNodeList->Item(i);
        const AString& strName = piNode->GetLocalName();

        if (strName.EqualsIgnoreCase("ussd-string"))
        {
            const INode* piUssdString = piNode->GetFirstChild();

            ASSERT_NE(piUssdString, nullptr);
            ASSERT_EQ(piUssdString->GetNodeType(), INode::TEXT_NODE);

            const IText* piText = static_cast<const IText*>(piUssdString);
            const AString& strUssdString = piText->GetData();

            IMS_TRACE_D("ussd-string=%s", strUssdString.GetStr(), 0, 0);

            EXPECT_TRUE(strUssdString.Contains('<'));
            EXPECT_TRUE(strUssdString.Contains('>'));
            EXPECT_TRUE(strUssdString.Contains('"'));
            EXPECT_TRUE(strUssdString.Contains('\''));
            break;
        }
    }

    if (piDocument != IMS_NULL)
    {
        piDocument->DestroyDocument();
    }
}

}  // namespace android
