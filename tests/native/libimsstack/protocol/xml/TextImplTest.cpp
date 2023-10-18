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

#include "DocumentBuilder.h"
#include "DocumentImpl.h"
#include "DomDocumentBuilderFactory.h"
#include "IElement.h"
#include "ImsStrLib.h"
#include "INodeList.h"
#include "IText.h"
#include "TextImpl.h"
#include "XmlApiParser.h"

namespace android
{

class TextImplTest : public ::testing::Test
{
public:
    inline TextImplTest() :
            m_pDocumentBuilder(IMS_NULL),
            m_piDocument(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override {}
    virtual void TearDown() override
    {
        if (m_piDocument != IMS_NULL)
        {
            m_piDocument->DestroyDocument();
            m_piDocument = IMS_NULL;
        }
        if (m_pDocumentBuilder != IMS_NULL)
        {
            DomDocumentBuilderFactory::GetInstance()->DestroyDocumentBuilder(m_pDocumentBuilder);
            m_pDocumentBuilder = IMS_NULL;
        }
    }

protected:
    DocumentBuilder* m_pDocumentBuilder;
    IDocument* m_piDocument;
};

TEST_F(TextImplTest, DocumentInfo)
{
    const IMS_CHAR XML_DATA[] = {"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                 "<bookstore>"
                                 "<about>All type of books</about>"
                                 "</bookstore>"};

    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    m_pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    ASSERT_NE(m_pDocumentBuilder, nullptr);

    m_piDocument = m_pDocumentBuilder->Parse(XML_DATA);
    ASSERT_NE(m_piDocument, nullptr);

    // Element Info
    IElement* piElement = m_piDocument->GetDocumentElement();
    EXPECT_NE(piElement, nullptr);

    INodeList* piNodeList = piElement->GetChildNodes();
    ASSERT_NE(piNodeList, nullptr);

    INode* piNodeEvent = piNodeList->Item(0);
    ASSERT_NE(piNodeEvent, nullptr);

    INode* piNode = piNodeEvent->GetFirstChild();
    EXPECT_NE(piNode, nullptr);

    ASSERT_EQ(piNode->GetNodeType(), INode::TEXT_NODE);
    IText* piText = DYNAMIC_CAST(IText*, piNode);

    EXPECT_EQ(piText->GetLength(), 17);
    EXPECT_EQ(piText->GetData(), AString("All type of books"));
    EXPECT_EQ(piText->SplitText(5), nullptr);

    piText->AppendData(AString(" available"));
    EXPECT_EQ(piText->GetData(), AString("All type of books available"));
    piText->InsertData(11, " school");
    EXPECT_EQ(piText->GetData(), AString("All type of school books available"));

    piText->ReplaceData(4, 7, "high");
    EXPECT_EQ(piText->GetData(), AString("All high school books available"));

    piText->DeleteData(4, 11);
    EXPECT_EQ(piText->GetData(), AString("All  books available"));

    piText->SetData("Test Data");
    EXPECT_EQ(piText->GetData(), AString("Test Data"));

    EXPECT_EQ(piText->SubstringData(0, 4), AString("Test"));

    // Attribute Info
    EXPECT_FALSE(piText->HasAttribute());
    INamedNodeMap* piNodeMap = piText->GetAttributes();
    ASSERT_NE(piNodeMap, nullptr);
    piText->DestroyNamedNodeMap(piNodeMap);

    // Node Info
    EXPECT_EQ(piText->GetNodeName(), AString("text"));
    EXPECT_EQ(piText->GetNodeValue(), AString("All type of books"));
    EXPECT_EQ(piText->GetLocalName(), AString("text"));
    EXPECT_TRUE(piText->GetNameSpaceUri().IsNull());
    EXPECT_EQ(piText->GetNodeType(), 11);

    EXPECT_NE(piText->GetParentNode(), nullptr);

    EXPECT_FALSE(piText->HasChildNode());

    EXPECT_NE(piText->GetChildNodes(), nullptr);
    EXPECT_EQ(piText->GetFirstChild(), nullptr);
    EXPECT_EQ(piText->GetLastChild(), nullptr);

    EXPECT_EQ(piText->CloneNode(IMS_TRUE), nullptr);
    EXPECT_EQ(piText->GetNextSibling(), nullptr);
    EXPECT_EQ(piText->GetPreviousSibling(), nullptr);

    piText->SetPrefix(AString("prefix"));
    EXPECT_EQ(piText->GetPrefix(), AString("prefix"));

    piText->SetTextContent(AString("text"));
    EXPECT_EQ(piText->GetTextContent(), AString("text"));

    piText->SetOwnerDocument(m_piDocument);
    EXPECT_EQ(piText->GetOwnerDocument(), m_piDocument);

    EXPECT_EQ(piText->RemoveChild(piNode), nullptr);

    piElement->DestroyNodeList(piNodeList);

    TextImpl objText;
    EXPECT_EQ(objText.Normalize(), IMS_SUCCESS);
    EXPECT_FALSE(objText.IsSupported(AString::ConstNull(), AString::ConstNull()));
}

}  // namespace android
