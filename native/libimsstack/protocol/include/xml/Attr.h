#ifndef ATTR_H_
#define ATTR_H_

#include "Node.h"

class IElement;

class Attr : public Node
{
public:
    Attr(IN xmlAttrPtr pstAttr, IN IElement* piOwnerElement);
    virtual ~Attr();

    // Node
    void SetNextSibling(IN INode* piNextNode) override;
    void SetPreviousSibling(IN INode* piPreviousNode) override;
    void SetParent(IN INode* piParenetNode) override;
    IMS_RESULT SetChildren(IN INode* piChildNode) override;

    // Attr
    virtual const AString& GetName() const;
    virtual IElement* GetOwnerElement() const;
    virtual IMS_BOOL GetSpecified() const;
    virtual const AString& GetValue() const;
    virtual IMS_BOOL IsId() const;
    virtual void SetValue(IN const AString& strValue);

private:
    IElement* m_piOwnerElement;
    IMS_BOOL m_bSpecified;
};

#endif
