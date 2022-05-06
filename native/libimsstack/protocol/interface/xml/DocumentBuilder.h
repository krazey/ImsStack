#ifndef DOCUMENT_BUILDER_H_
#define DOCUMENT_BUILDER_H_

#include "IDocument.h"

class DocumentBuilderPrivate;

/**
 * @brief This class provides the API to obtain DOM document instances from an XML document.
 *
 * Using this class, an application can obtain a Document from XML content.
 */
class DocumentBuilder
{
protected:
    DocumentBuilder();
    virtual ~DocumentBuilder();

public:
    DocumentBuilder(IN const DocumentBuilder& objOther) = delete;
    DocumentBuilder& operator=(IN const DocumentBuilder& objOther) = delete;

public:
    /**
     * @brief Parses the content of the given string as an XML document and
     *        returns a new DOM Document object.
     *
     * @param strXml String containing the content to be parsed.
     * @return A new instance of IDocument.
     */
    IDocument* Parse(IN const AString& strXml);

    /**
     * @brief Parses the content of the given string as an XML document and
     *        returns a new DOM Document object.
     *
     * @param pszXml String containing the content to be parsed.
     * @return A new instance of IDocument.
     */
    IDocument* Parse(IN const IMS_CHAR* pszXml);

private:
    DocumentBuilderPrivate* pDocumentBuilderPrivate;
};

#endif
