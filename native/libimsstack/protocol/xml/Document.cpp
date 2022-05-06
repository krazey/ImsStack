#include "Document.h"
#include "ElementImpl.h"
#include "IAttr.h"
#include "IText.h"

PUBLIC
Document::Document(xmlDocPtr pstDoc, xmlXPathContextPtr pstXpathContext) :
        Node(pstDoc),
        m_strVersion((const IMS_CHAR*)pstDoc->version),
        m_strEncoding((const IMS_CHAR*)pstDoc->encoding),
        m_strUrl((const IMS_CHAR*)pstDoc->URL),
        m_pstDoc(pstDoc),
        m_pstXpathContext(pstXpathContext)
{
}

PUBLIC VIRTUAL Document::~Document()
{
    /* Free Libxml Object */
    if (m_pstXpathContext != IMS_NULL)
    {
        XmlApi_XPathFreeContext(m_pstXpathContext);
    }

    if (m_pstDoc != IMS_NULL)
    {
        XmlApi_FreeDoc(m_pstDoc);
    }
}

PUBLIC VIRTUAL INode* Document::AdoptNode(IN INode* piNode)
{
    (void)piNode;
    return IMS_NULL;
}

PUBLIC VIRTUAL IAttr* Document::CreateAttribute(IN const AString& strName)
{
    (void)strName;
    return IMS_NULL;
}

PUBLIC VIRTUAL IAttr* Document::CreateAttributeNs(
        IN const AString& strNamespaceUri, IN const AString& strQualifiedName)
{
    (void)strNamespaceUri;
    (void)strQualifiedName;
    return IMS_NULL;
}

PUBLIC VIRTUAL IElement* Document::CreateElement(IN const AString& strTagName)
{
    (void)strTagName;
    return IMS_NULL;
}

PUBLIC VIRTUAL IElement* Document::CreateElementNs(
        IN const AString& strNamespaceUri, IN const AString& strQualifiedName)
{
    (void)strNamespaceUri;
    (void)strQualifiedName;
    return IMS_NULL;
}

PUBLIC VIRTUAL IText* Document::CreateTextNode(IN const AString& strData)
{
    (void)strData;
    return IMS_NULL;
}

PUBLIC VIRTUAL IElement* Document::GetDocumentElement() const
{
    INode* piNode = m_pNodeList->Item(0);

    if (piNode == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (piNode->GetNodeType() != INode::ELEMENT_NODE)
    {
        return IMS_NULL;
    }

    return DYNAMIC_CAST(ElementImpl*, piNode);
}

PUBLIC VIRTUAL IElement* Document::GetElementById(IN const AString& strElementId) const
{
    (void)strElementId;
    return IMS_NULL;
}

PUBLIC VIRTUAL INodeList* Document::GetElementsByTagName(IN const AString& strTagName) const
{
    if (strTagName.GetLength() == 0)
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

        if (piNode->GetNodeType() == INode::ELEMENT_NODE)
        {
            if (strTagName.Equals(piNode->GetNodeName()))
            {
                pNewNodeList->AddNode(piNode);
            }
        }
    }

    if (pNewNodeList->GetLength() == 0)
    {
        delete pNewNodeList;
        return IMS_NULL;
    }

    return pNewNodeList;
}

PUBLIC VIRTUAL INodeList* Document::GetElementsByTagNameNs(
        IN const AString& strNamespaceUri, IN const AString& strLocalName) const
{
    if (strLocalName.GetLength() == 0)
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

        if (piNode->GetNodeType() == INode::ELEMENT_NODE)
        {
            if (strLocalName.Equals(piNode->GetLocalName()) &&
                    strNamespaceUri.Equals(piNode->GetNameSpaceUri()))
            {
                pNewNodeList->AddNode(piNode);
            }
        }
    }

    if (pNewNodeList->GetLength() == 0)
    {
        delete pNewNodeList;
        return IMS_NULL;
    }

    return pNewNodeList;
}

PUBLIC VIRTUAL INode* Document::ImportNode(IN INode* piNode, IN IMS_BOOL bDeep)
{
    (void)piNode;
    (void)bDeep;
    return IMS_NULL;
}

PUBLIC VIRTUAL const AString& Document::GetEncodingScheme() const
{
    return m_strEncoding;
}

PUBLIC VIRTUAL const AString& Document::GetUrl() const
{
    return m_strUrl;
}

PUBLIC VIRTUAL const AString& Document::GetVersion() const
{
    return m_strVersion;
}
