#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include "Node.h"

class IAttr;
class IElement;
class IText;

class Document : public Node
{
public:
    Document(IN xmlDocPtr pstDoc, IN xmlXPathContextPtr pstXpathContext);
    virtual ~Document();

    // Document
    virtual INode* AdoptNode(IN INode* piNode);
    virtual IAttr* CreateAttribute(IN const AString& strName);
    virtual IAttr* CreateAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName);
    virtual IElement* CreateElement(IN const AString& strTagName);
    virtual IElement* CreateElementNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName);
    virtual IText* CreateTextNode(IN const AString& strData);
    virtual IElement* GetDocumentElement() const;
    virtual IElement* GetElementById(IN const AString& strElementId) const;

    // Notice: INodeList* MUST be freed after calling these methods.
    virtual INodeList* GetElementsByTagName(IN const AString& strTagName) const;
    virtual INodeList* GetElementsByTagNameNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const;
    virtual INode* ImportNode(IN INode* piNode, IN IMS_BOOL bDeep);

    virtual const AString& GetEncodingScheme() const;
    virtual const AString& GetUrl() const;
    virtual const AString& GetVersion() const;

private:
    AString m_strVersion;
    AString m_strEncoding;
    AString m_strUrl;
    xmlDocPtr m_pstDoc;
    xmlXPathContextPtr m_pstXpathContext;
};

#endif
