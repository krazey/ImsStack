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
#include "AString.h"
#include "ImsLib.h"
#include "ServiceTrace.h"

#include "ElementImpl.h"
#include "IDocument.h"
#include "NamedNodeMap.h"
#include "Node.h"
#include "NodeList.h"
#include "TextImpl.h"

#ifdef __IMS_XML_DEBUG__
__IMS_TRACE_TAG_XML__;
#endif

PUBLIC
Node::Node() :
        m_piOwnerDocument(IMS_NULL),
        m_pNamedNodeMap(new NamedNodeMap()),
        m_pNodeList(new NodeList()),
        m_piParentNode(IMS_NULL),
        m_piPreviousNode(IMS_NULL),
        m_piNextNode(IMS_NULL),
        m_nNodeType(INode::INVALID_NODE),
        m_strLocalName(AString::ConstNull()),
        m_strNodeName(AString::ConstNull()),
        m_strNodeValue(AString::ConstNull()),
        m_strTextContent(AString::ConstNull()),
        m_strNameSpaceUri(AString::ConstNull()),
        m_strPrefix(AString::ConstNull()),
        m_pstNode(IMS_NULL)
{
#ifdef __IMS_XML_DEBUG__
    IMS_TRACE_D("Constructor :: Node", 0, 0, 0);
#endif
}

PUBLIC
Node::Node(IN xmlNodePtr pstNode) :
        m_piOwnerDocument(IMS_NULL),
        m_pNamedNodeMap(new NamedNodeMap()),
        m_pNodeList(new NodeList()),
        m_piParentNode(IMS_NULL),
        m_piPreviousNode(IMS_NULL),
        m_piNextNode(IMS_NULL),
        m_nNodeType(INode::INVALID_NODE),
        m_strLocalName(reinterpret_cast<const IMS_CHAR*>(pstNode->name)),
        m_strNodeName(reinterpret_cast<const IMS_CHAR*>(pstNode->name)),
        m_strNodeValue(reinterpret_cast<const IMS_CHAR*>(pstNode->content)),
        m_strTextContent(AString::ConstNull()),
        m_strNameSpaceUri(AString::ConstNull()),
        m_strPrefix(AString::ConstNull()),
        m_pstNode(pstNode)
{
#ifdef __IMS_XML_DEBUG__
    IMS_TRACE_D("Constructor :: Node(node) w/ %s", m_strNodeName.GetStr(), 0, 0);
#endif

    if (m_pstNode->nsDef != IMS_NULL)
    {
        m_strNameSpaceUri = reinterpret_cast<const IMS_CHAR*>(m_pstNode->nsDef->href);
        m_strPrefix = reinterpret_cast<const IMS_CHAR*>(m_pstNode->nsDef->prefix);
    }

    m_nNodeType = ConvertXmlNodeType(m_pstNode->type);

    // FIX_TEXT_NODE_VALUE :: remove WSPs if the value contains the white spaces in start or end
    if ((m_nNodeType == INode::TEXT_NODE) && (m_strNodeValue.GetLength() > 0))
    {
        if (IMS_ISSPACE(m_strNodeValue[0]) ||
                IMS_ISSPACE(m_strNodeValue[m_strNodeValue.GetLength() - 1]))
        {
            m_strNodeValue = m_strNodeValue.Trim();
        }
    }
}

PUBLIC
Node::Node(IN xmlDocPtr pstDoc) :
        m_piOwnerDocument(IMS_NULL),
        m_pNamedNodeMap(new NamedNodeMap()),
        m_pNodeList(new NodeList()),
        m_piParentNode(IMS_NULL),
        m_piPreviousNode(IMS_NULL),
        m_piNextNode(IMS_NULL),
        m_nNodeType(INode::INVALID_NODE),
        m_strLocalName(reinterpret_cast<const IMS_CHAR*>(pstDoc->name)),
        m_strNodeName(reinterpret_cast<const IMS_CHAR*>(pstDoc->name)),
        m_strNodeValue(AString::ConstNull()),
        m_strTextContent(AString::ConstNull()),
        m_strNameSpaceUri(AString::ConstNull()),
        m_strPrefix(AString::ConstNull()),
        m_pstNode(IMS_NULL)
{
#ifdef __IMS_XML_DEBUG__
    IMS_TRACE_D("Constructor :: Node(doc) w/ %s", m_strNodeName.GetStr(), 0, 0);
#endif

    if (pstDoc->oldNs != IMS_NULL)
    {
        m_strNameSpaceUri = reinterpret_cast<const IMS_CHAR*>(pstDoc->oldNs->href);
        m_strPrefix = reinterpret_cast<const IMS_CHAR*>(pstDoc->oldNs->prefix);
    }

    m_nNodeType = ConvertXmlNodeType(pstDoc->type);
}

PUBLIC
Node::Node(IN xmlAttrPtr pstAttr) :
        m_piOwnerDocument(IMS_NULL),
        m_pNamedNodeMap(new NamedNodeMap()),
        m_pNodeList(new NodeList()),
        m_piParentNode(IMS_NULL),
        m_piPreviousNode(IMS_NULL),
        m_piNextNode(IMS_NULL),
        m_nNodeType(INode::INVALID_NODE),
        m_strLocalName(reinterpret_cast<const IMS_CHAR*>(pstAttr->name)),
        m_strNodeName(reinterpret_cast<const IMS_CHAR*>(pstAttr->name)),
        m_strNodeValue(AString::ConstNull()),
        m_strTextContent(AString::ConstNull()),
        m_strNameSpaceUri(AString::ConstNull()),
        m_strPrefix(AString::ConstNull()),
        m_pstNode(IMS_NULL)
{
#ifdef __IMS_XML_DEBUG__
    IMS_TRACE_D("Constructor :: Node(attr) w/ %s", m_strNodeName.GetStr(), 0, 0);
#endif

    if (pstAttr->children != IMS_NULL)
    {
        m_strNodeValue = reinterpret_cast<const IMS_CHAR*>(pstAttr->children->content);
    }

    if (pstAttr->ns != IMS_NULL)
    {
        m_strNameSpaceUri = reinterpret_cast<const IMS_CHAR*>(pstAttr->ns->href);
        m_strPrefix = reinterpret_cast<const IMS_CHAR*>(pstAttr->ns->prefix);
    }

    m_nNodeType = ConvertXmlNodeType(pstAttr->type);
}

PUBLIC VIRTUAL Node::~Node()
{
#ifdef __IMS_XML_DEBUG__
    IMS_TRACE_D("Destructor :: Node(%s)", m_strNodeName.GetStr(), 0, 0);
#endif

    if (m_pNodeList != IMS_NULL)
    {
        for (IMS_SINT32 i = 0; i < m_pNodeList->GetLength(); i++)
        {
            INode* piNode = m_pNodeList->Item(i);

            if (piNode == IMS_NULL)
            {
                continue;
            }

            IMS_SINT32 nNodeType = piNode->GetNodeType();

            if (nNodeType == INode::ELEMENT_NODE)
            {
                ElementImpl* pElementImpl = DYNAMIC_CAST(ElementImpl*, piNode);
                delete pElementImpl;
            }
            else if (nNodeType == INode::TEXT_NODE)
            {
                TextImpl* pTextImpl = DYNAMIC_CAST(TextImpl*, piNode);
                delete pTextImpl;
            }
        }

        delete m_pNodeList;
        m_pNodeList = IMS_NULL;
    }

    if (m_pNamedNodeMap != IMS_NULL)
    {
        delete m_pNamedNodeMap;
        m_pNamedNodeMap = IMS_NULL;
    }

    m_piOwnerDocument = IMS_NULL;
    m_piParentNode = IMS_NULL;
    m_piPreviousNode = IMS_NULL;
    m_pstNode = IMS_NULL;
}

PUBLIC VIRTUAL INode* Node::AppendChild(IN INode* piChild)
{
    if (piChild == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (m_pNodeList->AddNode(piChild) != IMS_SUCCESS)
    {
        return IMS_NULL;
    }

    return piChild;
}

PUBLIC VIRTUAL INode* Node::CloneNode(IN IMS_BOOL bDeep)
{
    (void)bDeep;
    return IMS_NULL;
}

PUBLIC VIRTUAL INamedNodeMap* Node::GetAttributes() const
{
    NamedNodeMap* pNewNameNodeMap = new NamedNodeMap();

    for (IMS_SINT32 i = 0; i < m_pNamedNodeMap->GetLength(); i++)
    {
        INode* piNode = m_pNamedNodeMap->Item(i);

        if (piNode->GetNodeType() == INode::ATTRIBUTE_NODE)
        {
            pNewNameNodeMap->AddNamedItem(piNode);
        }
    }

    return pNewNameNodeMap;
}

PUBLIC VIRTUAL INodeList* Node::GetChildNodes() const
{
    NodeList* pNewNodeList = new NodeList();

    for (IMS_SINT32 i = 0; i < m_pNodeList->GetLength(); i++)
    {
        pNewNodeList->AddNode(m_pNodeList->Item(i));
    }

    return pNewNodeList;
}

PUBLIC VIRTUAL INode* Node::GetFirstChild() const
{
    return m_pNodeList->Item(0);
}

PUBLIC VIRTUAL INode* Node::GetLastChild() const
{
    return m_pNodeList->Item(m_pNodeList->GetLength() - 1);
}

PUBLIC VIRTUAL const AString& Node::GetLocalName() const
{
    return m_strLocalName;
}

PUBLIC VIRTUAL const AString& Node::GetNameSpaceUri() const
{
    return m_strNameSpaceUri;
}

PUBLIC VIRTUAL INode* Node::GetNextSibling() const
{
    return m_piNextNode;
}

PUBLIC VIRTUAL const AString& Node::GetNodeName() const
{
    return m_strNodeName;
}

PUBLIC VIRTUAL IMS_SINT32 Node::GetNodeType() const
{
    return m_nNodeType;
}

PUBLIC VIRTUAL const AString& Node::GetNodeValue() const
{
    return m_strNodeValue;
}

PUBLIC VIRTUAL IDocument* Node::GetOwnerDocument() const
{
    return m_piOwnerDocument;
}

PUBLIC VIRTUAL INode* Node::GetParentNode() const
{
    return m_piParentNode;
}

PUBLIC VIRTUAL const AString& Node::GetPrefix() const
{
    return m_strPrefix;
}

PUBLIC VIRTUAL INode* Node::GetPreviousSibling() const
{
    return m_piPreviousNode;
}

PUBLIC VIRTUAL const AString& Node::GetTextContent() const
{
    return m_strTextContent;
}

PUBLIC VIRTUAL IMS_BOOL Node::HasAttribute() const
{
    return (m_pNamedNodeMap->GetLength() != 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL Node::HasChildNode() const
{
    return (m_pNodeList->GetLength() != 0) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL INode* Node::InsertBefore(IN INode* piNewChild, IN INode* piRefChild)
{
    (void)piNewChild;
    (void)piRefChild;
    return IMS_NULL;
}

PUBLIC VIRTUAL IMS_BOOL Node::IsSupported(
        IN const AString& strFeature, IN const AString& strVersion)
{
    (void)strFeature;
    (void)strVersion;
    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_RESULT Node::Normalize()
{
    return IMS_SUCCESS;
}

PUBLIC VIRTUAL INode* Node::RemoveChild(IN INode* piChild)
{
    (void)piChild;
    return IMS_NULL;
}

PUBLIC VIRTUAL INode* Node::ReplaceChild(IN INode* piNewChild, IN INode* piOldChild)
{
    (void)piNewChild;
    (void)piOldChild;
    return IMS_NULL;
}

PUBLIC VIRTUAL void Node::SetNodeValue(IN const AString& strNodeValue)
{
    m_strNodeValue = strNodeValue;
}

PUBLIC VIRTUAL void Node::SetPrefix(IN const AString& strPrefix)
{
    m_strPrefix = strPrefix;
}

PUBLIC VIRTUAL void Node::SetTextContent(IN const AString& strTextContext)
{
    m_strTextContent = strTextContext;
}

PUBLIC VIRTUAL void Node::DestroyNodeList(IN INodeList*& piNodeList)
{
#ifdef __IMS_XML_DEBUG__
    IMS_TRACE_D("Node::DestroyNodeList", 0, 0, 0);
#endif

    NodeList* pNodeList = DYNAMIC_CAST(NodeList*, piNodeList);

    if (pNodeList != IMS_NULL)
    {
        delete pNodeList;
    }

    piNodeList = IMS_NULL;
}

PUBLIC VIRTUAL void Node::DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap)
{
#ifdef __IMS_XML_DEBUG__
    IMS_TRACE_D("Node::DestroyNamedNodeMap", 0, 0, 0);
#endif

    NamedNodeMap* pNamedNodeMap = DYNAMIC_CAST(NamedNodeMap*, piNamedNodeMap);

    if (pNamedNodeMap != IMS_NULL)
    {
        delete pNamedNodeMap;
    }

    piNamedNodeMap = IMS_NULL;
}

PUBLIC VIRTUAL void Node::SetNextSibling(IN INode* piNode)
{
    m_piNextNode = piNode;
}

PUBLIC VIRTUAL void Node::SetPreviousSibling(IN INode* piNode)
{
    m_piPreviousNode = piNode;
}

PUBLIC VIRTUAL void Node::SetParent(IN INode* piNode)
{
    m_piParentNode = piNode;
}

PUBLIC VIRTUAL IMS_RESULT Node::SetChildren(IN INode* piNode)
{
    return m_pNodeList->AddNode(piNode);
}

PUBLIC VIRTUAL void Node::SetOwnerDocument(IN IDocument* piDocument)
{
    m_piOwnerDocument = piDocument;
}

PROTECTED GLOBAL IMS_SINT32 Node::ConvertXmlNodeType(IN IMS_SINT32 nXmlNodeType)
{
    switch (nXmlNodeType)
    {
        case XML_ELEMENT_NODE:
            return INode::ELEMENT_NODE;
        case XML_ATTRIBUTE_NODE:
            return INode::ATTRIBUTE_NODE;
        case XML_TEXT_NODE:
            return INode::TEXT_NODE;
#if 0
        case XML_CDATA_SECTION_NODE:
            return INode::CDATA_SECTION_NODE;
        case XML_ENTITY_REF_NODE:
            return INode::ENTITY_REFERENCE_NODE;
        case XML_ENTITY_NODE:
            return INode::ENTITY_NODE;
        case XML_COMMENT_NODE:
            return INode::COMMENT_NODE;
        case XML_DOCUMENT_NODE:
            return INode::DOCUMENT_NODE;
        case XML_DOCUMENT_TYPE_NODE:
            return INode::DOCUMENT_TYPE_NODE;
        case XML_DOCUMENT_FRAG_NODE:
            return INode::DOCUMENT_FRAGMENT_NODE;
        case XML_NOTATION_NODE:
            return INode::NOTATION_NODE;
        case XML_PI_NODE:
        case XML_HTML_DOCUMENT_NODE:
        case XML_DTD_NODE:
        case XML_ELEMENT_DECL:
        case XML_ATTRIBUTE_DECL:
        case XML_ENTITY_DECL:
        case XML_NAMESPACE_DECL:
        case XML_XINCLUDE_START:
        case XML_XINCLUDE_END:
            // No Type
            return INode::INVALID_NODE;
#endif
        default:
            return INode::INVALID_NODE;
    }
}
