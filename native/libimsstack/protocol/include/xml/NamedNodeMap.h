#ifndef NAEMD_NODE_MAP_H_
#define NAEMD_NODE_MAP_H_

#include "IMSList.h"
#include "INamedNodeMap.h"

class NamedNodeMap : public INamedNodeMap
{
public:
    NamedNodeMap();
    virtual ~NamedNodeMap();

    // INamedNodeMap
    IMS_SINT32 GetLength() const override;
    INode* GetNamedItem(IN const AString& strName) const override;
    INode* GetNamedItemNs(
            IN const AString& strNamespaceUri, IN const AString& strName) const override;
    INode* Item(IN IMS_SINT32 nIndex) const override;
    INode* RemoveNamedItem(IN const AString& strName) override;
    INode* RemoveNamedItemNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) override;
    INode* SetNamedItem(IN INode* piNode) override;
    INode* SetNamedItemNs(IN INode* piNode) override;

public:
    IMS_RESULT AddNamedItem(IN INode* piNode);

private:
    IMSList<INode*> m_objNamedItems;
};

#endif
