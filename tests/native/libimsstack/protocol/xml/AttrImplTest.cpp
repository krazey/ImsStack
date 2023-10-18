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

#include "AttrImpl.h"
#include "ElementImpl.h"
#include "INodeList.h"
#include "INamedNodeMap.h"

namespace android
{
const IMS_UCHAR ELEMENT[] = "ElementName";

class AttrImplTest : public ::testing::Test
{
public:
    inline AttrImplTest() :
            m_pstNode(IMS_NULL)
    {
    }

protected:
    virtual void SetUp() override
    {
        m_pstNode = xmlNewNode(IMS_NULL, ELEMENT);
        ASSERT_NE(m_pstNode, nullptr);
    }

    virtual void TearDown() override { xmlFreeNode(m_pstNode); }

public:
    xmlNodePtr m_pstNode;
};

TEST_F(AttrImplTest, GetAndSetMethods)
{
    const IMS_UCHAR NAMESPACE[] = "test.com";
    const IMS_UCHAR PREFIX[] = "prefix";

    // Add new namespace and prefix
    xmlNewNs(m_pstNode, NAMESPACE, PREFIX);

    IMS_UCHAR arrAttrName[] = "AttributeName";
    IMS_UCHAR arrAttrNameWithPrefix[] = "prefix:AttributeName";
    IMS_UCHAR arrAttrValue[] = "AttributeValue";

    // Add attribute with prefix to element node
    xmlAttrPtr pstAttr = xmlSetProp(m_pstNode, arrAttrNameWithPrefix, arrAttrValue);

    ASSERT_NE(pstAttr, nullptr);

    AttrImpl objAttrImpl(pstAttr, IMS_NULL);

    // Attribute will not have any attributes
    INamedNodeMap* piNamedNodeMap = objAttrImpl.GetAttributes();

    EXPECT_EQ(piNamedNodeMap->GetLength(), 0);
    EXPECT_FALSE(objAttrImpl.HasAttribute());

    objAttrImpl.DestroyNamedNodeMap(piNamedNodeMap);

    // Node type should be attribute
    EXPECT_EQ(objAttrImpl.GetNodeType(), INode::ATTRIBUTE_NODE);

    const IMS_CHAR* pAttrName = reinterpret_cast<const IMS_CHAR*>(&arrAttrName[0]);
    const IMS_CHAR* pAttrValue = reinterpret_cast<const IMS_CHAR*>(&arrAttrValue[0]);
    const IMS_CHAR* pNamespaceUri = reinterpret_cast<const IMS_CHAR*>(&NAMESPACE[0]);
    const IMS_CHAR* pPrefix = reinterpret_cast<const IMS_CHAR*>(&PREFIX[0]);

    // Check attribute name, value, prefix and namespace uri
    EXPECT_EQ(objAttrImpl.GetNodeName(), AString(pAttrName));
    EXPECT_EQ(objAttrImpl.GetLocalName(), AString(pAttrName));
    EXPECT_EQ(objAttrImpl.GetNodeValue(), AString(pAttrValue));
    EXPECT_EQ(objAttrImpl.GetNameSpaceUri(), AString(pNamespaceUri));
    EXPECT_EQ(objAttrImpl.GetPrefix(), AString(pPrefix));

    AString strNewValue("NewValue");

    objAttrImpl.SetValue(strNewValue);

    // Check attribute updated to new value
    EXPECT_EQ(objAttrImpl.GetNodeValue(), strNewValue);

    AString strNewPrefix("NewPrefix");

    objAttrImpl.SetPrefix(strNewPrefix);

    // Check attribute updated to new prefix
    EXPECT_EQ(objAttrImpl.GetPrefix(), strNewPrefix);

    EXPECT_FALSE(objAttrImpl.IsId());
    EXPECT_FALSE(objAttrImpl.GetSpecified());
    EXPECT_EQ(objAttrImpl.GetOwnerElement(), nullptr);

    ElementImpl objElementImpl(m_pstNode);

    objAttrImpl.SetParent(&objElementImpl);

    // Attribute nodes cannot have children or parent nodes
    INodeList* piChildrenNodeList = objAttrImpl.GetChildNodes();

    EXPECT_EQ(piChildrenNodeList->GetLength(), 0);

    objAttrImpl.DestroyNodeList(piChildrenNodeList);

    EXPECT_FALSE(objAttrImpl.HasChildNode());
    EXPECT_EQ(objAttrImpl.GetParentNode(), nullptr);
    EXPECT_EQ(objAttrImpl.GetFirstChild(), nullptr);
    EXPECT_EQ(objAttrImpl.GetLastChild(), nullptr);
    EXPECT_EQ(objAttrImpl.SetChildren(IMS_NULL), IMS_FAILURE);
    EXPECT_EQ(objAttrImpl.CloneNode(IMS_TRUE), nullptr);
    EXPECT_EQ(objAttrImpl.CloneNode(IMS_FALSE), nullptr);
    EXPECT_EQ(objAttrImpl.GetNextSibling(), nullptr);
    EXPECT_EQ(objAttrImpl.GetPreviousSibling(), nullptr);
    EXPECT_EQ(objAttrImpl.GetTextContent(), AString::ConstNull());
}

}  // namespace android