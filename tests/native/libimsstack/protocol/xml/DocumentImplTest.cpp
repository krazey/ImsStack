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
#include "ImsStrLib.h"
#include "XmlApiParser.h"

namespace android
{

class DocumentImplTest : public ::testing::Test
{
public:
    inline DocumentImplTest() :
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

TEST_F(DocumentImplTest, DocumentInfo)
{
    const IMS_CHAR XML_DATA[] = {"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
                                 "<reginfo xmlns=\"urn:ietf:params:xml:ns:reginfo\" "
                                 "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
                                 "version=\"0\" state=\"full\">"
                                 "<registration aor=\"sip:+11234567890@test.3gpp.com\" "
                                 "id=\"889160506\" state=\"active\">"
                                 "<contact id=\"20533260\" state=\"active\" event=\"registered\">"
                                 "<uri>sip:+11234567890@test.3gpp.com</uri>"
                                 "</contact>"
                                 "</registration>"
                                 "</reginfo>"};
    xmlDocPtr pstDoc = XmlApi_ReadMemory(
            XML_DATA, IMS_StrLen(XML_DATA), IMS_NULL, IMS_NULL, XML_PARSE_NOBLANKS);
    xmlXPathContextPtr pstXPathContext = XmlApi_XPathNewContext(pstDoc);

    DocumentImpl objDocument(pstDoc, pstXPathContext);

    EXPECT_EQ(objDocument.GetVersion(), AString("1.0"));
    EXPECT_TRUE(objDocument.GetUrl().IsNull());
    EXPECT_EQ(objDocument.GetEncodingScheme(), AString("utf-8"));
    EXPECT_EQ(objDocument.GetOwnerDocument(), nullptr);

    EXPECT_EQ(objDocument.GetElementsByTagName(AString::ConstNull()), nullptr);
    EXPECT_EQ(objDocument.GetElementsByTagName("registration"), nullptr);

    EXPECT_EQ(objDocument.GetElementsByTagNameNs(AString::ConstNull(), AString::ConstNull()),
            nullptr);
    EXPECT_EQ(objDocument.GetElementsByTagNameNs(AString::ConstNull(), "registration"), nullptr);

    EXPECT_EQ(objDocument.CreateAttribute("attr"), nullptr);
    EXPECT_EQ(objDocument.CreateAttributeNs(AString::ConstNull(), "Name"), nullptr);
    EXPECT_EQ(objDocument.CreateElement("registeredinfo"), nullptr);
    EXPECT_EQ(objDocument.CreateElementNs("Uri", AString::ConstNull()), nullptr);
    EXPECT_EQ(objDocument.CreateTextNode("Data"), nullptr);
}

TEST_F(DocumentImplTest, GetNodeInfo)
{
    const IMS_CHAR XML_DATA[] = {"<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                                 "<bookstore>"
                                 "<book category=\"Health\">"
                                 "<title lang=\"en\">Morning Walk</title>"
                                 "<author>Lina</author>"
                                 "<year>2015</year>"
                                 "<price>100.00</price>"
                                 "</book>"
                                 "</bookstore>"};

    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    m_pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    ASSERT_NE(m_pDocumentBuilder, nullptr);

    m_piDocument = m_pDocumentBuilder->Parse(XML_DATA);
    ASSERT_NE(m_piDocument, nullptr);

    // Element Info
    IElement* piElement = m_piDocument->GetDocumentElement();
    EXPECT_NE(piElement, nullptr);

    EXPECT_EQ(m_piDocument->GetElementById("category"), nullptr);
    INodeList* piNodeList = m_piDocument->GetElementsByTagName("bookstore");
    ASSERT_NE(piNodeList, nullptr);
    m_piDocument->DestroyNodeList(piNodeList);

    piNodeList = m_piDocument->GetElementsByTagNameNs(AString::ConstNull(), "bookstore");
    ASSERT_NE(piNodeList, nullptr);
    m_piDocument->DestroyNodeList(piNodeList);

    // Attribute Info
    EXPECT_FALSE(m_piDocument->HasAttribute());
    INamedNodeMap* piNodeMap = m_piDocument->GetAttributes();
    ASSERT_NE(piNodeMap, nullptr);
    m_piDocument->DestroyNamedNodeMap(piNodeMap);

    // Node Info
    EXPECT_TRUE(m_piDocument->GetNodeName().IsNull());
    EXPECT_EQ(m_piDocument->GetNodeType(), -1);

    EXPECT_EQ(m_piDocument->GetParentNode(), nullptr);

    EXPECT_TRUE(m_piDocument->HasChildNode());
    piNodeList = m_piDocument->GetChildNodes();
    ASSERT_NE(piNodeList, nullptr);
    m_piDocument->DestroyNodeList(piNodeList);

    INode* piNode = m_piDocument->GetFirstChild();
    EXPECT_NE(piNode, nullptr);
    EXPECT_NE(m_piDocument->GetLastChild(), nullptr);

    EXPECT_EQ(m_piDocument->CloneNode(IMS_TRUE), nullptr);
    EXPECT_EQ(m_piDocument->GetNextSibling(), nullptr);
    EXPECT_EQ(m_piDocument->GetPreviousSibling(), nullptr);

    m_piDocument->SetPrefix(AString("prefix"));
    EXPECT_EQ(m_piDocument->GetPrefix(), AString("prefix"));

    m_piDocument->SetTextContent(AString("text"));
    EXPECT_EQ(m_piDocument->GetTextContent(), AString("text"));

    m_piDocument->SetOwnerDocument(m_piDocument);
    EXPECT_EQ(m_piDocument->GetOwnerDocument(), m_piDocument);

    EXPECT_EQ(m_piDocument->Normalize(), IMS_SUCCESS);
    EXPECT_FALSE(m_piDocument->IsSupported(AString::ConstNull(), AString::ConstNull()));

    EXPECT_EQ(m_piDocument->AdoptNode(m_piDocument->GetLastChild()), nullptr);
    EXPECT_EQ(m_piDocument->ImportNode(m_piDocument->GetLastChild(), IMS_TRUE), nullptr);
    EXPECT_EQ(m_piDocument->RemoveChild(m_piDocument->GetLastChild()), nullptr);
}

}  // namespace android
