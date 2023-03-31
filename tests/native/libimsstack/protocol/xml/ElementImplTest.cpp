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

#include "ElementImpl.h"
#include "INodeList.h"
#include "INamedNodeMap.h"

namespace android
{
const IMS_UCHAR PARENT[] = "Parent";

class ElementImplTest : public ::testing::Test
{
protected:
    virtual void SetUp() override
    {
        m_pstNode = xmlNewNode(IMS_NULL, PARENT);

        ASSERT_NE(m_pstNode, nullptr);
    }
    virtual void TearDown() override { xmlFreeNode(m_pstNode); }

public:
    xmlNodePtr m_pstNode;
};

TEST_F(ElementImplTest, GetAndSetMethods)
{
    const IMS_UCHAR NAMESPACE[] = "test.com";
    const IMS_UCHAR PREFIX[] = "prefix";

    // Add namespace and prefix
    xmlNewNs(m_pstNode, NAMESPACE, PREFIX);

    const IMS_UCHAR ATTRNAME[] = "AttributeName";
    const IMS_UCHAR ATTRNAME_PREFIX[] = "prefix:AttributeName";
    const IMS_UCHAR ATTRVALUE[] = "AttributeValue";

    // Add attributes with prefix to element node
    xmlSetProp(m_pstNode, ATTRNAME_PREFIX, ATTRVALUE);

    // Add attributes without prefix to element node
    xmlSetProp(m_pstNode, ATTRNAME, ATTRVALUE);

    ElementImpl objElementImpl(m_pstNode);

    const IMS_UCHAR CHILD_1[] = "Child1";
    const IMS_UCHAR CONTENT_1[] = "Child1 content";

    // Add child1
    xmlNodePtr pstChildNode1 = xmlNewChild(m_pstNode, IMS_NULL, CHILD_1, CONTENT_1);

    ASSERT_NE(pstChildNode1, nullptr);

    ElementImpl* pChildImpl1 = new ElementImpl(pstChildNode1);

    objElementImpl.AppendChild(pChildImpl1);

    const IMS_UCHAR CHILD_2[] = "Child2";
    const IMS_UCHAR CONTENT_2[] = "Child2 content";

    // Add child2
    xmlNodePtr pstChildNode2 = xmlNewChild(m_pstNode, IMS_NULL, CHILD_2, CONTENT_2);

    ASSERT_NE(pstChildNode2, nullptr);

    ElementImpl* pChildImpl2 = new ElementImpl(pstChildNode2);

    objElementImpl.SetChildren(pChildImpl2);

    // 1 attribute
    INamedNodeMap* piNamedNodeMap = objElementImpl.GetAttributes();

    EXPECT_EQ(piNamedNodeMap->GetLength(), 2);

    const IMS_CHAR* pAttrName = reinterpret_cast<const IMS_CHAR*>(&ATTRNAME[0]);
    const IMS_CHAR* pNamespaceUri = reinterpret_cast<const IMS_CHAR*>(&NAMESPACE[0]);

    EXPECT_EQ(objElementImpl.HasAttributeB(AString(pAttrName)), IMS_TRUE);
    EXPECT_EQ(objElementImpl.HasAttributeNs(AString(pNamespaceUri), AString(pAttrName)), IMS_TRUE);

    objElementImpl.DestroyNamedNodeMap(piNamedNodeMap);

    // 2 children
    INodeList* piChildrenNodeList = objElementImpl.GetChildNodes();

    EXPECT_EQ(piChildrenNodeList->GetLength(), 2);

    objElementImpl.DestroyNodeList(piChildrenNodeList);

    // Node type should be Element
    EXPECT_EQ(objElementImpl.GetNodeType(), INode::ELEMENT_NODE);

    const IMS_CHAR* pElementName = reinterpret_cast<const IMS_CHAR*>(&PARENT[0]);
    const IMS_CHAR* pPrefix = reinterpret_cast<const IMS_CHAR*>(&PREFIX[0]);

    // Check Element name, value, prefix and namespace uri
    EXPECT_EQ(objElementImpl.GetNodeName(), AString(pElementName));
    EXPECT_EQ(objElementImpl.GetLocalName(), AString(pElementName));
    EXPECT_EQ(objElementImpl.GetTagName(), AString(pElementName));
    EXPECT_EQ(objElementImpl.GetNodeValue(), AString::ConstNull());
    EXPECT_EQ(objElementImpl.GetTextContent(), AString::ConstNull());
    EXPECT_EQ(objElementImpl.GetNameSpaceUri(), AString(pNamespaceUri));
    EXPECT_EQ(objElementImpl.GetPrefix(), AString(pPrefix));

    AString strNewPrefix("NewPrefix");

    objElementImpl.SetPrefix(strNewPrefix);

    // Check Element updated to new prefix
    EXPECT_EQ(objElementImpl.GetPrefix(), strNewPrefix);
    EXPECT_EQ(objElementImpl.GetOwnerDocument(), nullptr);
    EXPECT_EQ(objElementImpl.HasChildNode(), IMS_TRUE);
    EXPECT_EQ(objElementImpl.GetFirstChild(), pChildImpl1);
    EXPECT_EQ(objElementImpl.GetLastChild(), pChildImpl2);
    EXPECT_EQ(objElementImpl.CloneNode(IMS_TRUE), nullptr);
    EXPECT_EQ(objElementImpl.CloneNode(IMS_FALSE), nullptr);

    EXPECT_EQ(objElementImpl.GetTextContent(), AString::ConstNull());

    EXPECT_EQ(objElementImpl.InsertBefore(IMS_NULL, IMS_NULL), nullptr);
    EXPECT_EQ(objElementImpl.IsSupported(AString::ConstNull(), AString::ConstNull()), IMS_FALSE);
    EXPECT_EQ(objElementImpl.Normalize(), IMS_SUCCESS);
    EXPECT_EQ(objElementImpl.RemoveChild(IMS_NULL), nullptr);
    EXPECT_EQ(objElementImpl.ReplaceChild(IMS_NULL, IMS_NULL), nullptr);

    objElementImpl.SetNodeValue(AString("NodeValue"));
    EXPECT_EQ(objElementImpl.GetNodeValue(), AString("NodeValue"));

    objElementImpl.SetNextSibling(IMS_NULL);
    EXPECT_EQ(objElementImpl.GetNextSibling(), nullptr);

    objElementImpl.SetPreviousSibling(IMS_NULL);
    EXPECT_EQ(objElementImpl.GetPreviousSibling(), nullptr);

    objElementImpl.SetParent(IMS_NULL);
    EXPECT_EQ(objElementImpl.GetParentNode(), nullptr);

    const IMS_CHAR* pAttrValue = reinterpret_cast<const IMS_CHAR*>(&ATTRVALUE[0]);

    EXPECT_EQ(objElementImpl.GetAttribute(AString(pAttrName)), AString(pAttrValue));
    EXPECT_EQ(objElementImpl.GetAttribute(pAttrName), AString(pAttrValue));
    EXPECT_NE(objElementImpl.GetAttributeNode(AString(pAttrName)), nullptr);
    EXPECT_NE(
            objElementImpl.GetAttributeNodeNs(AString(pNamespaceUri), AString(pAttrName)), nullptr);
    EXPECT_EQ(objElementImpl.GetAttributeNs(AString(pNamespaceUri), AString(pAttrName)),
            AString(pAttrValue));

    const IMS_CHAR* pChildName = reinterpret_cast<const IMS_CHAR*>(&CHILD_1[0]);

    INodeList* piNodeList = objElementImpl.GetElementsByTagName(AString(pChildName));
    EXPECT_NE(piNodeList, nullptr);
    objElementImpl.DestroyNodeList(piNodeList);

    piNodeList = nullptr;
    piNodeList = objElementImpl.GetElementsByTagName(pChildName);
    EXPECT_NE(piNodeList, nullptr);
    objElementImpl.DestroyNodeList(piNodeList);

    piNodeList = nullptr;
    piNodeList = objElementImpl.GetElementsByTagNameNs(AString(pNamespaceUri), AString(pChildName));
    EXPECT_EQ(piNodeList, nullptr);

    // Below methods are empty method, no functionality
    objElementImpl.RemoveAttribute(AString(pAttrName));
    objElementImpl.RemoveAttributeNode(nullptr);
    objElementImpl.SetAttribute(AString::ConstNull(), AString::ConstNull());
    objElementImpl.SetAttributeNode(nullptr);
    objElementImpl.SetAttributeNodeNs(nullptr);
    objElementImpl.SetAttributeNs(
            AString(pNamespaceUri), AString::ConstNull(), AString::ConstNull());
    objElementImpl.SetIdAttribute(AString(pAttrName), IMS_TRUE);
    objElementImpl.SetIdAttributeNode(nullptr, IMS_TRUE);
    objElementImpl.SetIdAttributeNodeNs(AString(pNamespaceUri), AString(pAttrName), IMS_TRUE);
}

}  // namespace android