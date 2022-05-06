#ifndef XML_APP_H_
#define XML_APP_H_

#include "AString.h"
#include "ImsMessageDef.h"

class DocumentBuilder;
class IDocument;
class IXmlTransactionProvider;
class XmlTransaction;

class XmlApp
{
public:
    enum class XmlResult
    {
        XML_RESULT_FAILURE = 0,
        XML_RESULT_SUCCESS,
    };

    class AttachResponseParam
    {
    public:
        XmlResult eResult;
    };

    class ParseResponseParam
    {
    public:
        XmlResult eResult;
        IDocument* piDocument;
        DocumentBuilder* pDocumentBuilder;
    };

public:
    XmlApp(IN IXmlTransactionProvider* piTransactionProvider);
    ~XmlApp();

public:
    void Attach(IN const AString& strTargetName);
    void Detach();
    IMS_RESULT Parse(IN XmlTransaction* pTransaction);

private:
    IMS_RESULT Parse(IN const AString& strRawXml);
    IMS_RESULT SendParseResponse(
            IN XmlResult eResult, IN IDocument* piDocument, IN DocumentBuilder* pDocumentBuilder);

public:
    enum
    {
        AMSG_XML_ATTACH_RESPONSE = IMS_MSG_XML + 1,
        AMSG_XML_PARSE_RESPONSE
    };

private:
    IXmlTransactionProvider* m_piTransactionProvider;
    AString m_strTargetName;
};

#endif
