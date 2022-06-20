/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef INTERFACE_XML_STREAM_WRITER_H_
#define INTERFACE_XML_STREAM_WRITER_H_

#include "AString.h"

class IXmlStreamWriter
{
public:
    /**
     * @brief Closes this writer and free any resources associated with the writer.
     */
    virtual void Close() = 0;

    /**
     * @brief Returns the duplicate XML string.
     *
     * The caller SHOULD release the resource because it returns the duplicate string.
     *     - #IMS_MEM_Free
     *
     * @return The duplicate XML string.
     */
    virtual IMS_CHAR* Flush() = 0;

    /**
     * @brief Returns the formatted XML string.
     *
     * @return The formatted XML string.
     */
    virtual const AString& GetString() const = 0;

    /**
     * @brief Gets the prefix the URI is bound to.
     *
     * @param strUri the URI to bind to the prefix
     * @return The prefix of specified URI or null string.
     */
    virtual const AString& GetPrefix(IN const AString& strUri) const = 0;

    /**
     * @brief Binds a URI to the default namespace.
     *
     * This URI is bound in the scope of the current START_ELEMENT / END_ELEMENT pair.
     * If this method is called before a START_ELEMENT has been written the URI is bound
     * in the root scope.
     *
     * @param strUri the URI to bind to the default namespace, may be null
     */
    virtual void SetDefaultNamespace(IN const AString& strUri) = 0;

    /**
     * @brief Sets the prefix the URI is bound to.
     *
     * This prefix is bound in the scope of the current START_ELEMENT / END_ELEMENT pair.
     * If this method is called before a START_ELEMENT has been written the prefix is bound
     * in the root scope.
     *
     * @param strPrefix the prefix to bind to the URI, may not be null
     * @param strUri the URI to bind to the prefix, may be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetPrefix(IN const AString& strPrefix, IN const AString& strUri) = 0;

    /**
     * @brief Writes an attribute to the output stream without a prefix.
     *
     * @param strLocalName the local name of the attribute
     * @param strValue the value of the attribute
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteAttribute(
            IN const AString& strLocalName, IN const AString& strValue) = 0;

    /**
     * @brief Writes an attribute to the output stream.
     *
     * @param strNamespaceUri the uri of the default namespace for this attribute
     * @param strLocalName the local name of the attribute
     * @param strValue the value of the attribute
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteAttribute(IN const AString& strNamespaceUri,
            IN const AString& strLocalName, IN const AString& strValue) = 0;

    /**
     * @brief Writes an attribute to the output stream.
     *
     * @param strPrefix the uri of the prefix for this attribute
     * @param strNamespaceUri the uri of the default namespace for this attribute
     * @param strLocalName the local name of the attribute
     * @param strValue the value of the attribute
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteAttribute(IN const AString& strPrefix,
            IN const AString& strNamespaceURI, IN const AString& strLocalName,
            IN const AString& strValue) = 0;

    /**
     * @brief Writes a text to the output stream.
     *
     * @param pszText the value to write
     * @param nStart the starting position in the array
     * @param nLen the number of characters to write
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteCharacters(
            IN const IMS_CHAR* pszText, IN IMS_SINT32 nStart, IN IMS_SINT32 nLen) = 0;

    /**
     * @brief Writes a text to the output stream.
     *
     * @param strText the value to write
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteCharacters(IN const AString& strText) = 0;

    /**
     * @brief Writes an xml comment with the data enclosed.
     *
     * @param strData the data contained in the comment, may be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteComment(IN const AString& strData) = 0;

    /**
     * @brief Writes the default namespace to the output stream.
     *
     * @param strNamespaceUri the uri to bind the default namespace to
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteDefaultNamespace(IN const AString& strNamespaceUri) = 0;

    /**
     * @brief Writes an empty element tag to the output stream.
     *
     * @param strLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteEmptyElement(IN const AString& strLocalName) = 0;

    /**
     * @brief Writes an empty element tag with the namespace URI to the output stream.
     *
     * @param strNamespaceUri the uri to bind the tag to, may not be null
     * @param strLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteEmptyElement(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) = 0;

    /**
     * @brief Writes an empty element tag with the prefix and namespace URI to the output stream.
     *
     * @param strPrefix the prefix of the tag, may not be null
     * @param strLocalName the local name of the tag, may not be null
     * @param strNamespaceUri the uri to bind the tag to, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteEmptyElement(IN const AString& strPrefix,
            IN const AString& strLocalName, IN const AString& strNamespaceUri) = 0;

    /**
     * @brief Closes any start tags and writes corresponding end tags.
     */
    virtual void WriteEndDocument() = 0;

    /**
     * @brief Writes an end tag to the output relying on the internal state of the writer
     *        to determine the prefix and local name of the event.
     *
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteEndElement() = 0;

    /**
     * @brief Writes a namespace to the output stream.
     *
     * If the prefix argument to this method is the empty string,
     * "xmlns", or null this method will delegate to WriteDefaultNamespace.
     *
     * @param strPrefix the prefix to bind this namespace to
     * @param strNamespaceUri the uri to bind the prefix to
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteNamespace(
            IN const AString& strPrefix, IN const AString& strNamespaceUri) = 0;

    /**
     * @brief Writes the XML Declaration. Defaults the XML version to 1.0,
     *        and the encoding to UTF-8.
     *
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartDocument() = 0;

    /**
     * @brief Writes the XML Declaration. Defaults the XML version to 1.0.
     *
     * @param strVersion the version of the xml document
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartDocument(IN const AString& strVersion) = 0;

    /**
     * @brief Writes the XML Declaration. Note that the encoding parameter does not set the actual
     *        encoding of the underlying output.
     *
     * That must be set when the instance of the XmlStreamWriter is created
     * using the XmlOutputFactory.
     *
     * @param strEncoding the encoding of the xml declaration
     * @param strVersion the version of the xml document
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartDocument(
            IN const AString& strEncoding, IN const AString& strVersion) = 0;

    /**
     * @brief Writes a start tag to the output stream.
     *
     * All WriteStartElement methods open a new scope in the internal namespace context.
     * Writing the corresponding EndElement causes the scope to be closed.
     *
     * @param strLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartElement(IN const AString& strLocalName) = 0;

    /**
     * @brief Writes a start tag to the output stream.
     *
     * @param strNamespaceUri the namespace URI of the prefix to use, may not be null
     * @param strLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartElement(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) = 0;

    /**
     * @brief Writes a start tag to the output stream.
     *
     * @param strPrefix the prefix of the tag, may not be null
     * @param strLocalName the local name of the tag, may not be null
     * @param strNamespaceUri the namespace URI of the prefix to use, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartElement(IN const AString& strPrefix,
            IN const AString& strLocalName, IN const AString& strNamespaceUri) = 0;

    /**
     * @brief Binds a URI to the default namespace.
     *
     * This URI is bound in the scope of the current START_ELEMENT / END_ELEMENT pair.
     * If this method is called before a START_ELEMENT has been written the URI is bound
     * in the root scope.
     *
     * @param pszUri the URI to bind to the default namespace, may be null
     */
    virtual void SetDefaultNamespace(IN const IMS_CHAR* pszUri) = 0;

    /**
     * @brief Sets the prefix the URI is bound to.
     *
     * This prefix is bound in the scope of the current START_ELEMENT / END_ELEMENT pair.
     * If this method is called before a START_ELEMENT has been written the prefix is bound
     * in the root scope.
     *
     * @param pszPrefix the prefix to bind to the URI, may not be null
     * @param pszUri the URI to bind to the prefix, may be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetPrefix(IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszUri) = 0;

    /**
     * @brief Writes an attribute to the output stream without a prefix.
     *
     * @param pszLocalName the local name of the attribute
     * @param pszValue the value of the attribute
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteAttribute(
            IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszValue) = 0;

    /**
     * @brief Writes an attribute to the output stream.
     *
     * @param pszNamespaceUri the uri of the default namespace for this attribute
     * @param pszLocalName the local name of the attribute
     * @param pszValue the value of the attribute
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteAttribute(IN const IMS_CHAR* pszNamespaceUri,
            IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszValue) = 0;

    /**
     * @brief Writes an attribute to the output stream.
     *
     * @param pszPrefix the uri of the prefix for this attribute
     * @param pszNamespaceUri the uri of the default namespace for this attribute
     * @param pszLocalName the local name of the attribute
     * @param pszValue the value of the attribute
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteAttribute(IN const IMS_CHAR* pszPrefix,
            IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName,
            IN const IMS_CHAR* pszValue) = 0;

    /**
     * @brief Writes a text to the output stream.
     *
     * @param pszText the value to write
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteCharacters(IN const IMS_CHAR* pszText) = 0;

    /**
     * @brief Writes an xml comment with the data enclosed.
     *
     * @param pszData the data contained in the comment, may be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteComment(IN const IMS_CHAR* pszData) = 0;

    /**
     * @brief Writes the default namespace to the output stream.
     *
     * @param pszNamespaceUri the uri to bind the default namespace to
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteDefaultNamespace(IN const IMS_CHAR* pszNamespaceUri) = 0;

    /**
     * @brief Writes an empty element tag to the output stream.
     *
     * @param pszLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteEmptyElement(IN const IMS_CHAR* pszLocalName) = 0;

    /**
     * @brief Writes an empty element tag with the namespace URI to the output stream.
     *
     * @param pszNamespaceUri the uri to bind the tag to, may not be null
     * @param pszLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteEmptyElement(
            IN const IMS_CHAR* pszNamespaceUri, IN const IMS_CHAR* pszLocalName) = 0;

    /**
     * @brief Writes an empty element tag with the prefix and namespace URI to the output stream.
     *
     * @param pszPrefix the prefix of the tag, may not be null
     * @param pszLocalName the local name of the tag, may not be null
     * @param pszNamespaceUri the uri to bind the tag to, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteEmptyElement(IN const IMS_CHAR* pszPrefix,
            IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszNamespaceUri) = 0;

    /**
     * @brief Writes a namespace to the output stream.
     *
     * If the prefix argument to this method is the empty string,
     * "xmlns", or null this method will delegate to WriteDefaultNamespace.
     *
     * @param pszPrefix the prefix to bind this namespace to
     * @param pszNamespaceUri the uri to bind the prefix to
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteNamespace(
            IN const IMS_CHAR* pszPrefix, IN const IMS_CHAR* pszNamespaceUri) = 0;

    /**
     * @brief Writes the XML Declaration. Defaults the XML version to 1.0.
     *
     * @param pszVersion the version of the xml document
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartDocument(IN const IMS_CHAR* pszVersion) = 0;

    /**
     * @brief Writes the XML Declaration. Note that the encoding parameter does not set the actual
     *        encoding of the underlying output.
     *
     * That must be set when the instance of the XmlStreamWriter is created
     * using the XmlOutputFactory.
     *
     * @param pszEncoding the encoding of the xml declaration
     * @param pszVersion the version of the xml document
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartDocument(
            IN const IMS_CHAR* pszEncoding, IN const IMS_CHAR* pszVersion) = 0;

    /**
     * @brief Writes a start tag to the output stream.
     *
     * All WriteStartElement methods open a new scope in the internal namespace context.
     * Writing the corresponding EndElement causes the scope to be closed.
     *
     * @param pszLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartElement(IN const IMS_CHAR* pszLocalName) = 0;

    /**
     * @brief Writes a start tag to the output stream.
     *
     * @param pszNamespaceUri the namespace URI of the prefix to use, may not be null
     * @param pszLocalName the local name of the tag, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartElement(
            IN const IMS_CHAR* pszNamespaceUri, const IMS_CHAR* pszLocalName) = 0;

    /**
     * @brief Writes a start tag to the output stream.
     *
     * @param pszPrefix the prefix of the tag, may not be null
     * @param pszLocalName the local name of the tag, may not be null
     * @param pszNamespaceUri the namespace URI of the prefix to use, may not be null
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT WriteStartElement(IN const IMS_CHAR* pszPrefix,
            IN const IMS_CHAR* pszLocalName, IN const IMS_CHAR* pszNamespaceUri) = 0;

    /**
     * @brief Returns a length of the output stream.
     *
     * @return A length of XML content.
     */
    virtual IMS_SINT32 GetContentLength() const = 0;

    class XmlError
    {
    public:
        inline XmlError() :
                m_nErrorType(TYPE_NONE),
                m_strReason(AString::ConstNull())
        {
        }
        inline ~XmlError() {}
        XmlError(IN const XmlError& objOther) = delete;
        XmlError& operator=(IN const XmlError& objOther) = delete;

    public:
        inline IMS_SINT32 GetErrorType() const { return m_nErrorType; }
        inline const AString& GetReason() const { return m_strReason; }

        inline void SetErrorType(IN IMS_SINT32 nErrorType) { m_nErrorType = nErrorType; }
        inline void SetReason(IN const AString& strReason) { m_strReason = strReason; }

    public:
        enum
        {
            TYPE_NONE = 0,
            TYPE_ELEMENT,
            TYPE_ATTRIBUTE,
            TYPE_NAMESPACE
        };

    private:
        IMS_SINT32 m_nErrorType;
        AString m_strReason;
    };

    /**
     * @brief Returns a last error information of this writer.
     *
     * @return Pointer to XmlError.
     */
    virtual const XmlError* GetLastError() const = 0;

#define XMLSTREAMWRITER_SPACE " "
#define XMLSTREAMWRITER_TAB "\t"
#define XMLSTREAMWRITER_RETURN "\n"
};

#endif
