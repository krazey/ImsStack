#include "Attr.h"
#include "AttrImpl.h"
#include "IDocument.h"
#include "IElement.h"

PUBLIC
AttrImpl::AttrImpl(IN xmlAttrPtr pstAttr, IN IElement* piOwnerElement) :
        m_pAttr(new Attr(pstAttr, piOwnerElement))
{
}

PUBLIC VIRTUAL AttrImpl::~AttrImpl()
{
    if (m_pAttr != IMS_NULL)
    {
        delete m_pAttr;
        m_pAttr = IMS_NULL;
    }
}

PUBLIC VIRTUAL INode* AttrImpl::AppendChild(IN INode* piChild)
{
    return m_pAttr->AppendChild(piChild);
}

PUBLIC VIRTUAL INode* AttrImpl::CloneNode(IN IMS_BOOL bDeep)
{
    return m_pAttr->CloneNode(bDeep);
}

PUBLIC VIRTUAL INamedNodeMap* AttrImpl::GetAttributes() const
{
    return m_pAttr->GetAttributes();
}

PUBLIC VIRTUAL INodeList* AttrImpl::GetChildNodes() const
{
    return m_pAttr->GetChildNodes();
}

PUBLIC VIRTUAL INode* AttrImpl::GetFirstChild() const
{
    return m_pAttr->GetFirstChild();
}

PUBLIC VIRTUAL INode* AttrImpl::GetLastChild() const
{
    return m_pAttr->GetLastChild();
}

PUBLIC VIRTUAL const AString& AttrImpl::GetLocalName() const
{
    return m_pAttr->GetLocalName();
}

PUBLIC VIRTUAL const AString& AttrImpl::GetNameSpaceUri() const
{
    return m_pAttr->GetNameSpaceUri();
}

PUBLIC VIRTUAL INode* AttrImpl::GetNextSibling() const
{
    return m_pAttr->GetNextSibling();
}

PUBLIC VIRTUAL const AString& AttrImpl::GetNodeName() const
{
    return m_pAttr->GetNodeName();
}

PUBLIC VIRTUAL IMS_SINT32 AttrImpl::GetNodeType() const
{
    return m_pAttr->GetNodeType();
}

PUBLIC VIRTUAL const AString& AttrImpl::GetNodeValue() const
{
    return m_pAttr->GetNodeValue();
}

PUBLIC VIRTUAL IDocument* AttrImpl::GetOwnerDocument() const
{
    return m_pAttr->GetOwnerDocument();
}

PUBLIC VIRTUAL INode* AttrImpl::GetParentNode() const
{
    return m_pAttr->GetParentNode();
}

PUBLIC VIRTUAL const AString& AttrImpl::GetPrefix() const
{
    return m_pAttr->GetPrefix();
}

PUBLIC VIRTUAL INode* AttrImpl::GetPreviousSibling() const
{
    return m_pAttr->GetPreviousSibling();
}

PUBLIC VIRTUAL const AString& AttrImpl::GetTextContent() const
{
    return m_pAttr->GetTextContent();
}

PUBLIC VIRTUAL IMS_BOOL AttrImpl::HasAttribute() const
{
    return m_pAttr->HasAttribute();
}

PUBLIC VIRTUAL IMS_BOOL AttrImpl::HasChildNode() const
{
    return m_pAttr->HasChildNode();
}

PUBLIC VIRTUAL INode* AttrImpl::InsertBefore(IN INode* piNewChild, IN INode* piRefChild)
{
    return m_pAttr->InsertBefore(piNewChild, piRefChild);
}

PUBLIC VIRTUAL IMS_BOOL AttrImpl::IsSupported(
        IN const AString& strFeature, IN const AString& strVersion)
{
    return m_pAttr->IsSupported(strFeature, strVersion);
}

PUBLIC VIRTUAL IMS_RESULT AttrImpl::Normalize()
{
    return m_pAttr->Normalize();
}

PUBLIC VIRTUAL INode* AttrImpl::RemoveChild(IN INode* piChild)
{
    return m_pAttr->RemoveChild(piChild);
}

PUBLIC VIRTUAL INode* AttrImpl::ReplaceChild(IN INode* piNewChild, IN INode* piOldChild)
{
    return m_pAttr->ReplaceChild(piNewChild, piOldChild);
}

PUBLIC VIRTUAL void AttrImpl::SetNodeValue(IN const AString& strNodeValue)
{
    m_pAttr->SetNodeValue(strNodeValue);
}

PUBLIC VIRTUAL void AttrImpl::SetPrefix(IN const AString& strPrefix)
{
    m_pAttr->SetPrefix(strPrefix);
}

PUBLIC VIRTUAL void AttrImpl::SetTextContent(IN const AString& strTextContext)
{
    m_pAttr->SetTextContent(strTextContext);
}

PUBLIC VIRTUAL void AttrImpl::DestroyNodeList(IN INodeList*& piNodeList)
{
    m_pAttr->DestroyNodeList(piNodeList);
}

PUBLIC VIRTUAL void AttrImpl::DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap)
{
    m_pAttr->DestroyNamedNodeMap(piNamedNodeMap);
}

PUBLIC VIRTUAL void AttrImpl::SetNextSibling(IN INode* piNode)
{
    m_pAttr->SetNextSibling(piNode);
}

PUBLIC VIRTUAL void AttrImpl::SetPreviousSibling(IN INode* piNode)
{
    m_pAttr->SetPreviousSibling(piNode);
}

PUBLIC VIRTUAL void AttrImpl::SetParent(IN INode* piNode)
{
    m_pAttr->SetParent(piNode);
}

PUBLIC VIRTUAL IMS_RESULT AttrImpl::SetChildren(IN INode* piNode)
{
    return m_pAttr->SetChildren(piNode);
}

PUBLIC VIRTUAL void AttrImpl::SetOwnerDocument(IN IDocument* piDocument)
{
    m_pAttr->SetOwnerDocument(piDocument);
}

PUBLIC VIRTUAL const AString& AttrImpl::GetName() const
{
    return m_pAttr->GetName();
}

PUBLIC VIRTUAL IElement* AttrImpl::GetOwnerElement() const
{
    return m_pAttr->GetOwnerElement();
}

PUBLIC VIRTUAL IMS_BOOL AttrImpl::GetSpecified() const
{
    return m_pAttr->GetSpecified();
}

PUBLIC VIRTUAL const AString& AttrImpl::GetValue() const
{
    return m_pAttr->GetValue();
}

PUBLIC VIRTUAL IMS_BOOL AttrImpl::IsId() const
{
    return m_pAttr->IsId();
}

PUBLIC VIRTUAL void AttrImpl::SetValue(IN const AString& strValue)
{
    m_pAttr->SetValue(strValue);
}
