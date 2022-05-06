#ifndef XML_STREAM_WRITER_H_
#define XML_STREAM_WRITER_H_

#include "IMSStack.h"
#include "IXmlStreamWriter.h"

class XmlStreamWriter : public IXmlStreamWriter
{
private:
    class XmlPrefix
    {
    public:
        inline XmlPrefix(IN const AString& strPrefix, IN const AString& strUri) :
                m_strPrefix(strPrefix),
                m_strUri(strUri)
        {
        }
        inline ~XmlPrefix() {}
        XmlPrefix(IN const XmlPrefix& objOther) = delete;
        XmlPrefix& operator=(IN const XmlPrefix& objOther) = delete;

    public:
        inline const AString& GetPrefix() const { return m_strPrefix; }
        inline const AString& GetUri() const { return m_strUri; }

    private:
        AString m_strPrefix;
        AString m_strUri;
    };

    class XmlElement
    {
    public:
        inline XmlElement(IN const AString& strLocalName, IN const AString& strPrefix) :
                m_strLocalName(strLocalName),
                m_strPrefix(strPrefix)
        {
        }
        inline ~XmlElement() {}
        XmlElement(IN const XmlElement& objOther) = delete;
        XmlElement& operator=(IN const XmlElement& objOther) = delete;

    public:
        inline const AString& GetLocalName() const { return m_strLocalName; }
        inline const AString& GetPrefix() const { return m_strPrefix; }

    private:
        AString m_strLocalName;
        AString m_strPrefix;
    };

public:
    XmlStreamWriter();
    virtual ~XmlStreamWriter();
    XmlStreamWriter(IN const XmlStreamWriter& objOther) = delete;
    XmlStreamWriter& operator=(IN const XmlStreamWriter& objOther) = delete;

public:
    void Close() override;
    IMS_CHAR* Flush() override;
    const AString& GetString() const override;

    const AString& GetPrefix(IN const AString& strUri) const override;
    inline void SetDefaultNamespace(IN const AString& strUri) override
    {
        m_strDefaultNamespace = strUri;
    }
    IMS_RESULT SetPrefix(IN const AString& strPrefix, IN const AString& strUri) override;
    inline IMS_RESULT WriteAttribute(
            IN const AString& strLocalName, IN const AString& strValue) override
    {
        return WriteAttribute(AString::ConstNull(), AString::ConstNull(), strLocalName, strValue);
    }
    IMS_RESULT WriteAttribute(IN const AString& strNamespaceUri, IN const AString& strLocalName,
            IN const AString& strValue) override;
    IMS_RESULT WriteAttribute(IN const AString& strPrefix, IN const AString& strNamespaceUri,
            IN const AString& strLocalName, IN const AString& strValue) override;
    IMS_RESULT WriteCharacters(
            IN const IMS_CHAR* pszText, IN IMS_SINT32 nStart, IN IMS_SINT32 nLen) override;
    IMS_RESULT WriteCharacters(IN const AString& strText) override;
    IMS_RESULT WriteComment(IN const AString& strData) override;
    inline IMS_RESULT WriteDefaultNamespace(IN const AString& strNamespaceUri) override
    {
        return WriteNamespace(AString::ConstNull(), strNamespaceUri);
    }
    inline IMS_RESULT WriteEmptyElement(IN const AString& strLocalName) override
    {
        return WriteEmptyElement(AString::ConstNull(), strLocalName);
    }
    IMS_RESULT WriteEmptyElement(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) override;
    IMS_RESULT WriteEmptyElement(IN const AString& strPrefix, IN const AString& strLocalName,
            IN const AString& strNamespaceUri) override;
    inline void WriteEndDocument() override {}
    IMS_RESULT WriteEndElement() override;
    IMS_RESULT WriteNamespace(
            IN const AString& strPrefix, IN const AString& strNamespaceUri) override;
    inline IMS_RESULT WriteStartDocument() override { return WriteStartDocument("UTF-8", "1.0"); }
    IMS_RESULT WriteStartDocument(IN const AString& strVersion) override;
    IMS_RESULT WriteStartDocument(
            IN const AString& strEncoding, IN const AString& strVersion) override;
    inline IMS_RESULT WriteStartElement(IN const AString& strLocalName) override
    {
        return WriteStartElement(AString::ConstNull(), strLocalName, AString::ConstNull());
    }
    IMS_RESULT WriteStartElement(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) override;
    IMS_RESULT WriteStartElement(IN const AString& strPrefix, IN const AString& strLocalName,
            IN const AString& strNamespaceUri) override;

    // APIs with null-terminated string
    void SetDefaultNamespace(IN const IMS_CHAR* pszUri) override;
    IMS_RESULT SetPrefix(IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszUri) override;
    IMS_RESULT WriteAttribute(
            IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszValue) override;
    IMS_RESULT WriteAttribute(IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName,
            IN const IMS_CHAR* pszValue) override;
    IMS_RESULT WriteAttribute(IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszNamespaceUri,
            IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszValue) override;
    IMS_RESULT WriteCharacters(IN const IMS_CHAR* pszText) override;
    IMS_RESULT WriteComment(IN const IMS_CHAR* pszData) override;
    IMS_RESULT WriteDefaultNamespace(IN const IMS_CHAR* pszNamespaceUri) override;
    IMS_RESULT WriteEmptyElement(IN const IMS_CHAR* pszLocalName) override;
    IMS_RESULT WriteEmptyElement(
            IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName) override;
    IMS_RESULT WriteEmptyElement(IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszLocalName,
            IN const IMS_CHAR* pszNamespaceUri) override;
    IMS_RESULT WriteNamespace(
            IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszNamespaceUri) override;
    IMS_RESULT WriteStartDocument(IN const IMS_CHAR* pszVersion) override;
    IMS_RESULT WriteStartDocument(
            IN const IMS_CHAR* pszEncoding, IN const IMS_CHAR* pszVersion) override;
    IMS_RESULT WriteStartElement(IN const IMS_CHAR* pszLocalName) override;
    IMS_RESULT WriteStartElement(
            IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName) override;
    IMS_RESULT WriteStartElement(IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszLocalName,
            IN const IMS_CHAR* pszNamespaceUri) override;
    inline IMS_SINT32 GetContentLength() const override { return m_strXml.GetLength(); }
    inline const XmlError* GetLastError() const override { return m_pError; }

private:
    XmlPrefix* GetPrefixInternal(IN const AString& strUri) const;
    inline IMS_BOOL IsOperationFailedAtLeastOnce() const { return (m_pError != IMS_NULL); }
    void SetLastError(IN IMS_SINT32 nErrorType, IN const IMS_CHAR* pszReason);

private:
    AString m_strXml;
    AString m_strDefaultNamespace;
    XmlError* m_pError;
    IMSList<XmlPrefix*> m_objPrefixes;
    IMSStack<XmlElement*> m_objElements;
};

#endif
