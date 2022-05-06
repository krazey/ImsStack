#include "AttrImpl.h"
#include "Element.h"
#include "IElement.h"

PUBLIC
Element::Element(IN xmlNodePtr pstNode, IN IElement* piElement) :
        Node(pstNode)
{
    m_nNodeType = INode::ELEMENT_NODE;

    if (pstNode->properties != IMS_NULL)
    {
        AttrImpl* pPrevAttrImpl = IMS_NULL;
        xmlAttrPtr pstAttr = pstNode->properties;

        while (pstAttr != IMS_NULL)
        {
            AttrImpl* pAttrImpl = new AttrImpl(pstAttr, piElement);
            m_pNamedNodeMap->AddNamedItem(pAttrImpl);

            if (pPrevAttrImpl != IMS_NULL)
            {
                pPrevAttrImpl->SetNextSibling(pAttrImpl);
                pAttrImpl->SetPreviousSibling(pPrevAttrImpl);
            }

            pstAttr = pstAttr->next;
            pPrevAttrImpl = pAttrImpl;
        }
    }
}

PUBLIC VIRTUAL Element::~Element()
{
    if (m_pNamedNodeMap != IMS_NULL)
    {
        for (IMS_SINT32 i = 0; i < m_pNamedNodeMap->GetLength(); i++)
        {
            INode* piNode = m_pNamedNodeMap->Item(i);

            if (piNode->GetNodeType() == INode::ATTRIBUTE_NODE)
            {
                AttrImpl* pAttrImpl = DYNAMIC_CAST(AttrImpl*, piNode);
                delete pAttrImpl;
            }
        }

        delete m_pNamedNodeMap;
        m_pNamedNodeMap = IMS_NULL;
    }
}

PUBLIC VIRTUAL const AString& Element::GetAttribute(IN const AString& strName) const
{
    IAttr* piAttr = GetAttributeNode(strName);

    return (piAttr != IMS_NULL) ? piAttr->GetNodeValue() : AString::ConstNull();
}

PUBLIC VIRTUAL IAttr* Element::GetAttributeNode(IN const AString& strName) const
{
    if (m_pNamedNodeMap == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (strName.GetLength() == 0)
    {
        return IMS_NULL;
    }

    for (IMS_SINT32 i = 0; i < m_pNamedNodeMap->GetLength(); i++)
    {
        INode* piNode = m_pNamedNodeMap->Item(i);

        if (strName.EqualsIgnoreCase(piNode->GetLocalName()))
        {
            AttrImpl* pAttrImpl = DYNAMIC_CAST(AttrImpl*, piNode);
            return pAttrImpl;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL IAttr* Element::GetAttributeNodeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    if (m_pNamedNodeMap == IMS_NULL)
    {
        return IMS_NULL;
    }

    if ((strNamespaceUri.GetLength() == 0) || (strLocalName.GetLength() == 0))
    {
        return IMS_NULL;
    }

    for (IMS_SINT32 i = 0; i < m_pNamedNodeMap->GetLength(); i++)
    {
        INode* piNode = m_pNamedNodeMap->Item(i);

        if (strLocalName.EqualsIgnoreCase(piNode->GetLocalName()) &&
                strNamespaceUri.EqualsIgnoreCase(piNode->GetNameSpaceUri()))
        {
            AttrImpl* pAttrImpl = DYNAMIC_CAST(AttrImpl*, piNode);
            return pAttrImpl;
        }
    }

    return IMS_NULL;
}

PUBLIC VIRTUAL const AString& Element::GetAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    IAttr* piAttr = GetAttributeNodeNs(strNamespaceUri, strLocalName);

    return (piAttr != IMS_NULL) ? piAttr->GetNodeValue() : AString::ConstNull();
}

PUBLIC VIRTUAL INodeList* Element::GetElementsByTagName(IN const AString& strName) const
{
    if (strName.GetLength() == 0)
    {
        return IMS_NULL;
    }

    if (m_pNodeList->GetLength() == 0)
    {
        return IMS_NULL;
    }

    NodeList* pNewNodeList = new NodeList();

    for (IMS_SINT32 i = 0; i < m_pNodeList->GetLength(); i++)
    {
        INode* piNode = m_pNodeList->Item(i);

        if ((piNode->GetNodeType() == INode::ELEMENT_NODE) &&
                strName.Equals(piNode->GetLocalName()))
        {
            pNewNodeList->AddNode(piNode);
        }
    }

    if (pNewNodeList->GetLength() == 0)
    {
        delete pNewNodeList;
        return IMS_NULL;
    }

    return pNewNodeList;
}

PUBLIC VIRTUAL INodeList* Element::GetElementsByTagNameNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    (void)strNamespaceUri;
    (void)strLocalName;
    return IMS_NULL;
}

PUBLIC VIRTUAL const AString& Element::GetTagName() const
{
    return GetLocalName();
}

PUBLIC VIRTUAL IMS_BOOL Element::HasAttributeB(IN const AString& strName) const
{
    if (m_pNamedNodeMap == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if (strName.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < m_pNamedNodeMap->GetLength(); i++)
    {
        INode* piNode = m_pNamedNodeMap->Item(i);

        if (strName.EqualsIgnoreCase(piNode->GetLocalName()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL IMS_BOOL Element::HasAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    if (m_pNamedNodeMap == IMS_NULL)
    {
        return IMS_FALSE;
    }

    if ((strNamespaceUri.GetLength() == 0) || (strLocalName.GetLength() == 0))
    {
        return IMS_FALSE;
    }

    for (IMS_SINT32 i = 0; i < m_pNamedNodeMap->GetLength(); i++)
    {
        INode* piNode = m_pNamedNodeMap->Item(i);

        if (strLocalName.EqualsIgnoreCase(piNode->GetLocalName()) &&
                strNamespaceUri.EqualsIgnoreCase(piNode->GetNameSpaceUri()))
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC VIRTUAL void Element::RemoveAttribute(IN const AString& strName)
{
    (void)strName;
}

PUBLIC VIRTUAL IAttr* Element::RemoveAttributeNode(IN IAttr* piAttr)
{
    (void)piAttr;
    return IMS_NULL;
}

PUBLIC VIRTUAL void Element::RemoveAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName)
{
    (void)strNamespaceUri;
    (void)strLocalName;
}

PUBLIC VIRTUAL void Element::SetAttribute(IN const AString& strName, IN const AString& strValue)
{
    (void)strName;
    (void)strValue;
}

PUBLIC VIRTUAL IAttr* Element::SetAttributeNode(IN IAttr* piAttr)
{
    (void)piAttr;
    return IMS_NULL;
}

PUBLIC VIRTUAL IAttr* Element::SetAttributeNodeNs(IN IAttr* piAttr)
{
    (void)piAttr;
    return IMS_NULL;
}

PUBLIC VIRTUAL void Element::SetAttributeNs(IN const AString& strNamespaceUri,
        IN const AString& strQualifiedName, IN const AString& strValue)
{
    (void)strNamespaceUri;
    (void)strQualifiedName;
    (void)strValue;
}

PUBLIC VIRTUAL void Element::SetIdAttribute(IN const AString& strName, IN IMS_BOOL bIsId)
{
    (void)strName;
    (void)bIsId;
}

PUBLIC VIRTUAL void Element::SetIdAttributeNode(IN IAttr* piIdAttr, IN IMS_BOOL bIsId)
{
    (void)piIdAttr;
    (void)bIsId;
}

PUBLIC VIRTUAL void Element::SetIdAttributeNodeNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName, IN IMS_BOOL bIsId)
{
    (void)strNamespaceUri;
    (void)strLocalName;
    (void)bIsId;
}

PUBLIC VIRTUAL const AString& Element::GetAttribute(IN const IMS_CHAR* pszName) const
{
    if (pszName == IMS_NULL)
    {
        return AString::ConstNull();
    }

    AString strName(pszName);

    return GetAttribute(strName);
}

PUBLIC VIRTUAL IAttr* Element::GetAttributeNode(IN const IMS_CHAR* pszName) const
{
    if (pszName == IMS_NULL)
    {
        return IMS_NULL;
    }

    AString strName(pszName);

    return GetAttributeNode(strName);
}

PUBLIC VIRTUAL INodeList* Element::GetElementsByTagName(IN const IMS_CHAR* pszName) const
{
    if (pszName == IMS_NULL)
    {
        return IMS_NULL;
    }

    AString strName(pszName);

    return GetElementsByTagName(strName);
}
