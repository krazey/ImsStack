#include "ServiceTrace.h"
#include "XmlStreamWriter.h"

__IMS_TRACE_TAG_XML__;

PUBLIC
XmlStreamWriter::XmlStreamWriter() :
        m_strXml(AString::ConstNull()),
        m_strDefaultNamespace(AString::ConstNull()),
        m_pError(IMS_NULL)
{
}

PUBLIC VIRTUAL XmlStreamWriter::~XmlStreamWriter()
{
    if (m_pError != IMS_NULL)
    {
        delete m_pError;
        m_pError = IMS_NULL;
    }
}

PUBLIC VIRTUAL void XmlStreamWriter::Close()
{
    for (IMS_UINT32 i = 0; i < m_objPrefixes.GetSize(); i++)
    {
        XmlPrefix* pPrefix = m_objPrefixes.GetAt(i);

        if (pPrefix != IMS_NULL)
        {
            delete pPrefix;
        }
    }

    m_objPrefixes.Clear();

    IMS_UINT32 nElementCount = m_objElements.GetSize();

    for (IMS_UINT32 i = 0; i < nElementCount; i++)
    {
        XmlElement* pElement = m_objElements.Top();

        if (pElement != IMS_NULL)
        {
            delete pElement;
        }

        m_objElements.Pop();
    }

    if (m_pError != IMS_NULL)
    {
        delete m_pError;
        m_pError = IMS_NULL;
    }
}

PUBLIC VIRTUAL IMS_CHAR* XmlStreamWriter::Flush()
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_NULL;
    }

    if (!m_objElements.IsEmpty())
    {
        // XML content is malformed, so don't return incomplete XML content.
        return IMS_NULL;
    }

    return m_strXml.Duplicate();
}

PUBLIC VIRTUAL const AString& XmlStreamWriter::GetString() const
{
    if (IsOperationFailedAtLeastOnce())
    {
        return AString::ConstNull();
    }

    if (!m_objElements.IsEmpty())
    {
        // XML content is malformed, so don't return incomplete XML content.
        return AString::ConstNull();
    }

    return m_strXml;
}

PUBLIC VIRTUAL const AString& XmlStreamWriter::GetPrefix(IN const AString& strUri) const
{
    XmlPrefix* pPrefix = GetPrefixInternal(strUri);

    return (pPrefix != IMS_NULL) ? pPrefix->GetPrefix() : AString::ConstNull();
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::SetPrefix(
        IN const AString& strPrefix, IN const AString& strUri)
{
    if (strUri.GetLength() == 0)
    {
        return IMS_FAILURE;
    }

    XmlPrefix* pPrefix = GetPrefixInternal(strUri);

    if (pPrefix != IMS_NULL)
    {
        IMS_TRACE_D("Namespace(%s=%s) is already set", pPrefix->GetPrefix().GetStr(),
                pPrefix->GetUri().GetStr(), 0);
        return IMS_SUCCESS;
    }

    pPrefix = new XmlPrefix(strPrefix, strUri);

    if (pPrefix == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (!m_objPrefixes.Append(pPrefix))
    {
        delete pPrefix;
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteAttribute(IN const AString& strNamespaceUri,
        IN const AString& strLocalName, IN const AString& strValue)
{
    const AString& strPrefix = GetPrefix(strNamespaceUri);

    return WriteAttribute(strPrefix, strNamespaceUri, strLocalName, strValue);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteAttribute(IN const AString& strPrefix,
        IN const AString& strNamespaceUri, IN const AString& strLocalName,
        IN const AString& strValue)
{
    (void)strNamespaceUri;

    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    if ((strLocalName.GetLength() == 0) || m_objElements.IsEmpty())
    {
        SetLastError(XmlError::TYPE_ATTRIBUTE, "Attribute - No Element");
        return IMS_FAILURE;
    }

    m_strXml = m_strXml.Left(m_strXml.GetLength() - 1);
    m_strXml += XMLSTREAMWRITER_SPACE;

    if (strPrefix.GetLength() != 0)
    {
        m_strXml += strPrefix;
        m_strXml += ":";
    }

    m_strXml += strLocalName;
    m_strXml += "=\"";
    m_strXml += strValue;
    m_strXml += "\">";

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteCharacters(
        IN const IMS_CHAR* pszText, IN IMS_SINT32 nStart, IN IMS_SINT32 nLen)
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    if (pszText == IMS_NULL)
    {
        // no-op
        return IMS_SUCCESS;
    }

    AString strText(pszText);

    m_strXml += strText.GetSubStr(nStart, nLen);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteCharacters(IN const AString& strText)
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    m_strXml += strText;

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteComment(IN const AString& strData)
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    m_strXml += "<!-- ";
    m_strXml += strData;
    m_strXml += " -->";

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteEmptyElement(
        IN const AString& strNamespaceUri, IN const AString& strLocalName)
{
    const AString& strPrefix = GetPrefix(strNamespaceUri);

    return WriteEmptyElement(strPrefix, strLocalName, strNamespaceUri);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteEmptyElement(IN const AString& strPrefix,
        IN const AString& strLocalName, IN const AString& strNamespaceUri)
{
    (void)strNamespaceUri;

    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    if (strLocalName.GetLength() == 0)
    {
        SetLastError(XmlError::TYPE_ELEMENT, "EmptyElement - No element");
        return IMS_FAILURE;
    }

    m_strXml += "<";

    if (strPrefix.GetLength() != 0)
    {
        m_strXml += strPrefix;
        m_strXml += ":";
    }

    m_strXml += strLocalName;
    m_strXml += "/>";

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteEndElement()
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    XmlElement* pElement = m_objElements.Top();

    if (pElement == IMS_NULL)
    {
        SetLastError(XmlError::TYPE_ELEMENT, "EndElement - No element");
        return IMS_FAILURE;
    }

    m_strXml += "</";

    if (pElement->GetPrefix().GetLength() != 0)
    {
        m_strXml += pElement->GetPrefix();
        m_strXml += ":";
    }

    m_strXml += pElement->GetLocalName();
    m_strXml += ">";

    delete pElement;
    m_objElements.Pop();

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteNamespace(
        IN const AString& strPrefix, IN const AString& strNamespaceUri)
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    if (strNamespaceUri.GetLength() == 0)
    {
        SetLastError(XmlError::TYPE_NAMESPACE, "Namespace - URN is empty");
        return IMS_FAILURE;
    }

    m_strXml = m_strXml.Left(m_strXml.GetLength() - 1);

    if (strPrefix.GetLength() != 0)
    {
        m_strXml += " xmlns:";
        m_strXml += strPrefix;
    }
    else
    {
        m_strXml += " xmlns";
    }

    m_strXml += "=\"";
    m_strXml += strNamespaceUri;
    m_strXml += "\">";

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartDocument(IN const AString& strVersion)
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    m_strXml += "<?xml version=\"";
    m_strXml += strVersion;
    m_strXml += "\"?>";

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartDocument(
        IN const AString& strEncoding, IN const AString& strVersion)
{
    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    m_strXml += "<?xml version=\"";
    m_strXml += strVersion;
    m_strXml += "\" encoding=\"";
    m_strXml += strEncoding;
    m_strXml += "\"?>";

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartElement(
        IN const AString& strNamespaceUri, IN const AString& strLocalName)
{
    const AString& strPrefix = GetPrefix(strNamespaceUri);

    return WriteStartElement(strPrefix, strLocalName, strNamespaceUri);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartElement(IN const AString& strPrefix,
        IN const AString& strLocalName, IN const AString& strNamespaceUri)
{
    (void)strNamespaceUri;

    if (IsOperationFailedAtLeastOnce())
    {
        return IMS_FAILURE;
    }

    if (strLocalName.GetLength() == 0)
    {
        SetLastError(XmlError::TYPE_ELEMENT, "StartElement - No element");
        return IMS_FAILURE;
    }

    m_strXml += "<";

    if (strPrefix.GetLength() != 0)
    {
        m_strXml += strPrefix;
        m_strXml += ":";
    }

    m_strXml += strLocalName;
    m_strXml += ">";

    XmlElement* pElement = new XmlElement(strLocalName, strPrefix);

    m_objElements.Push(pElement);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL void XmlStreamWriter::SetDefaultNamespace(IN const IMS_CHAR* pszUri)
{
    AString strUri(pszUri);

    SetDefaultNamespace(strUri);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::SetPrefix(
        IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszUri)
{
    AString strPrefix(pszPrefix);
    AString strUri(pszUri);

    return SetPrefix(strPrefix, strUri);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteAttribute(
        IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszValue)
{
    AString strLocalName(pszLocalName);
    AString strValue(pszValue);

    return WriteAttribute(strLocalName, strValue);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteAttribute(IN const IMS_CHAR* pszNamespaceUri,
        IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszValue)
{
    AString strNameSpaceUri(pszNamespaceUri);
    AString strLocalName(pszLocalName);
    AString strValue(pszValue);

    return WriteAttribute(strNameSpaceUri, strLocalName, strValue);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteAttribute(IN const IMS_CHAR* pszPrefix,
        IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName,
        IN const IMS_CHAR* pszValue)
{
    AString strPrefix(pszPrefix);
    AString strNameSpaceUri(pszNamespaceUri);
    AString strLocalName(pszLocalName);
    AString strValue(pszValue);

    return WriteAttribute(strPrefix, strNameSpaceUri, strLocalName, strValue);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteCharacters(IN const IMS_CHAR* pszText)
{
    AString strText(pszText);

    return WriteCharacters(strText);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteComment(IN const IMS_CHAR* pszData)
{
    AString strData(pszData);

    return WriteComment(strData);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteDefaultNamespace(IN const IMS_CHAR* pszNamespaceUri)
{
    AString strNameSpaceUri(pszNamespaceUri);

    return WriteDefaultNamespace(strNameSpaceUri);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteEmptyElement(IN const IMS_CHAR* pszLocalName)
{
    AString strLocalName(pszLocalName);

    return WriteEmptyElement(strLocalName);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteEmptyElement(
        IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName)
{
    AString strNamespaceUri(pszNamespaceUri);
    AString strLocalName(pszLocalName);

    return WriteEmptyElement(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteEmptyElement(IN const IMS_CHAR* pszPrefix,
        IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszNamespaceUri)
{
    AString strPrefix(pszPrefix);
    AString strNamespaceUri(pszNamespaceUri);
    AString strLocalName(pszLocalName);

    return WriteEmptyElement(strPrefix, strLocalName, strNamespaceUri);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteNamespace(
        IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszNamespaceUri)
{
    AString strPrefix(pszPrefix);
    AString strNamespaceUri(pszNamespaceUri);

    return WriteNamespace(strPrefix, strNamespaceUri);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartDocument(IN const IMS_CHAR* pszVersion)
{
    AString strVersion(pszVersion);

    return WriteStartDocument(strVersion);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartDocument(
        IN const IMS_CHAR* pszEncoding, IN const IMS_CHAR* pszVersion)
{
    AString strEncoding(pszEncoding);
    AString strVersion(pszVersion);

    return WriteStartDocument(strEncoding, strVersion);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartElement(IN const IMS_CHAR* pszLocalName)
{
    AString strLocalName(pszLocalName);

    return WriteStartElement(strLocalName);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartElement(
        IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName)
{
    AString strNamespaceUri(pszNamespaceUri);
    AString strLocalName(pszLocalName);

    return WriteStartElement(strNamespaceUri, strLocalName);
}

PUBLIC VIRTUAL IMS_RESULT XmlStreamWriter::WriteStartElement(IN const IMS_CHAR* pszPrefix,
        IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszNamespaceUri)
{
    AString strPrefix(pszPrefix);
    AString strNamespaceUri(pszNamespaceUri);
    AString strLocalName(pszLocalName);

    return WriteStartElement(strPrefix, strLocalName, strNamespaceUri);
}

PRIVATE
XmlStreamWriter::XmlPrefix* XmlStreamWriter::GetPrefixInternal(IN const AString& strUri) const
{
    if (strUri.GetLength() == 0)
    {
        return IMS_NULL;
    }

    for (IMS_UINT32 i = 0; i < m_objPrefixes.GetSize(); i++)
    {
        XmlPrefix* pPrefix = m_objPrefixes.GetAt(i);

        if (pPrefix->GetUri().EqualsIgnoreCase(strUri))
        {
            return pPrefix;
        }
    }

    return IMS_NULL;
}

PRIVATE
void XmlStreamWriter::SetLastError(IN IMS_SINT32 nErrorType, IN const IMS_CHAR* pszReason)
{
    IMS_TRACE_E(0, "LastError - type=%d, error=%s", nErrorType, _TRACE_S_(pszReason), 0);

    if (m_pError == IMS_NULL)
    {
        m_pError = new XmlError();
    }

    m_pError->SetErrorType(nErrorType);
    m_pError->SetReason(pszReason);
}
