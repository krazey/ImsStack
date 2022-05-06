#include "INode.h"
#include "NodeList.h"

PUBLIC
NodeList::NodeList() {}

PUBLIC VIRTUAL NodeList::~NodeList() {}

PUBLIC VIRTUAL IMS_SINT32 NodeList::GetLength() const
{
    return m_objNodes.GetSize();
}

PUBLIC VIRTUAL INode* NodeList::Item(IN IMS_SINT32 nIndex) const
{
    if ((nIndex < 0) || (nIndex >= GetLength()))
    {
        return IMS_NULL;
    }

    if (m_objNodes.IsEmpty())
    {
        return IMS_NULL;
    }

    return m_objNodes.GetAt(nIndex);
}

PUBLIC
IMS_RESULT NodeList::AddNode(IN INode* piNode)
{
    if (piNode == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!m_objNodes.Append(piNode))
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}
