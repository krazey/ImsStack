#ifndef NODE_LIST_H_
#define NODE_LIST_H_

#include "IMSList.h"
#include "INodeList.h"

class NodeList : public INodeList
{
public:
    NodeList();
    virtual ~NodeList();

    // INodeList
    IMS_SINT32 GetLength() const override;
    INode* Item(IN IMS_SINT32 nIndex) const override;

    IMS_RESULT AddNode(IN INode* piNode);

private:
    IMSList<INode*> m_objNodes;
};

#endif
