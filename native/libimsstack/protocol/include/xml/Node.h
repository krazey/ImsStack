#ifndef NODE_H_
#define NODE_H_

#include "INode.h"
#include "NamedNodeMap.h"
#include "NodeList.h"
#include "XmlApiParser.h"
#include "XmlApiTree.h"
#include "XmlApiXPath.h"

class IDocument;

class Node : public INode
{
public:
    Node();
    Node(IN xmlNodePtr pstNode);
    Node(IN xmlDocPtr pstDoc);
    Node(IN xmlAttrPtr pstAttr);
    virtual ~Node();
    Node(IN const Node& objOther) = delete;
    Node& operator=(IN const Node& objOther) = delete;

    // INode
    INode* AppendChild(IN INode* piChild) override;
    INode* CloneNode(IN IMS_BOOL bDeep) override;
    INamedNodeMap* GetAttributes() const override;
    INodeList* GetChildNodes() const override;
    INode* GetFirstChild() const override;
    INode* GetLastChild() const override;
    const AString& GetLocalName() const override;
    const AString& GetNameSpaceUri() const override;
    INode* GetNextSibling() const override;
    const AString& GetNodeName() const override;
    IMS_SINT32 GetNodeType() const override;
    const AString& GetNodeValue() const override;
    IDocument* GetOwnerDocument() const override;
    INode* GetParentNode() const override;
    const AString& GetPrefix() const override;
    INode* GetPreviousSibling() const override;
    const AString& GetTextContent() const override;
    IMS_BOOL HasAttribute() const override;
    IMS_BOOL HasChildNode() const override;
    INode* InsertBefore(IN INode* piNewChild, IN INode* piRefChild) override;
    IMS_BOOL IsSupported(IN const AString& strFeature, IN const AString& strVersion) override;
    IMS_RESULT Normalize() override;
    INode* RemoveChild(IN INode* piChild) override;
    INode* ReplaceChild(IN INode* piNewChild, IN INode* piOldChild) override;
    void SetNodeValue(IN const AString& strNodeValue) override;
    void SetPrefix(IN const AString& strPrefix) override;
    void SetTextContent(IN const AString& strTextContext) override;

    // INode: extensions
    void DestroyNodeList(IN INodeList*& piNodeList) override;
    void DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap) override;
    void SetNextSibling(IN INode* piNode) override;
    void SetPreviousSibling(IN INode* piNode) override;
    void SetParent(IN INode* piNode) override;
    IMS_RESULT SetChildren(IN INode* piNode) override;
    void SetOwnerDocument(IN IDocument* piDocument) override;

protected:
    static IMS_SINT32 ConvertXmlNodeType(IN IMS_SINT32 nXmlNodeType);

protected:
    IDocument* m_piOwnerDocument;
    NamedNodeMap* m_pNamedNodeMap;
    NodeList* m_pNodeList;
    INode* m_piParentNode;
    INode* m_piPreviousNode;
    INode* m_piNextNode;

    IMS_SINT32 m_nNodeType;
    AString m_strLocalName;
    AString m_strNodeName;
    AString m_strNodeValue;
    AString m_strTextContent;
    AString m_strNameSpaceUri;
    AString m_strPrefix;

    xmlNodePtr m_pstNode;
};

#endif
