#ifndef TEXT_IMPL_H_
#define TEXT_IMPL_H_

#include "IText.h"
#include "XmlApiTree.h"

class IElement;
class INode;
class Text;

class TextImpl : public IText
{
public:
    TextImpl();
    TextImpl(IN xmlNodePtr pstNode);
    virtual ~TextImpl();
    TextImpl(IN const TextImpl& objOther) = delete;
    TextImpl& operator=(IN const TextImpl& objOther) = delete;

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

    // ICharacterData
    void AppendData(IN const AString& strData) override;
    void DeleteData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount) override;
    const AString& GetData() const override;
    IMS_SINT32 GetLength() const override;
    void InsertData(IN IMS_SINT32 nOffset, IN const AString& strData) override;
    void ReplaceData(
            IN IMS_SINT32 nOffSet, IN IMS_SINT32 nCount, IN const AString& strData) override;
    void SetData(IN const AString& strData) override;
    AString SubstringData(IN IMS_SINT32 nOffSet, IN IMS_SINT32 nCount) override;

    // IText
    IText* SplitText(IMS_SINT32 nOffset) override;

private:
    Text* m_pText;
};

#endif
