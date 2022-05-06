#include "Attr.h"
#include "IElement.h"

PUBLIC
Attr::Attr(IN xmlAttrPtr pstAttr, IN IElement* piOwnerElement) :
        Node(pstAttr),
        m_piOwnerElement(piOwnerElement),
        m_bSpecified(IMS_FALSE)
{
}

PUBLIC VIRTUAL Attr::~Attr() {}

PUBLIC VIRTUAL void Attr::SetNextSibling(IN INode* piNode)
{
    (void)piNode;
}

PUBLIC VIRTUAL void Attr::SetPreviousSibling(IN INode* piNode)
{
    (void)piNode;
}

PUBLIC VIRTUAL void Attr::SetParent(IN INode* piNode)
{
    (void)piNode;
}

PUBLIC VIRTUAL IMS_RESULT Attr::SetChildren(IN INode* piNode)
{
    (void)piNode;
    return IMS_FAILURE;
}

PUBLIC VIRTUAL const AString& Attr::GetName() const
{
    // FIXME: GetNodeName() ?
    return GetLocalName();
}

PUBLIC VIRTUAL IElement* Attr::GetOwnerElement() const
{
    return m_piOwnerElement;
}

PUBLIC VIRTUAL IMS_BOOL Attr::GetSpecified() const
{
    return m_bSpecified;
}

PUBLIC VIRTUAL const AString& Attr::GetValue() const
{
    return GetNodeValue();
}

PUBLIC VIRTUAL IMS_BOOL Attr::IsId() const
{
    return m_bSpecified;
}

PUBLIC VIRTUAL void Attr::SetValue(IN const AString& strValue)
{
    SetNodeValue(strValue);
}
