/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#ifdef __IMS_ASYNC_XML_PARSER__

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "IDocument.h"
#include "IXmlRequest.h"
#include "IXmlResponse.h"
#include "IRegInfoParserListener.h"
#include "RegInfoParser.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegInfoParser::RegInfoParser(
        IN const RegKey& objRegKey_, IN IXmlTransactionProvider*& piXmlTxnProvider_) :
        objRegKey(objRegKey_),
        piXmlTxnProvider(piXmlTxnProvider_),
        piXMLTxn(IMS_NULL),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RegInfoParser::~RegInfoParser()
{
    if (piXMLTxn != IMS_NULL)
    {
        piXmlTxnProvider->DestroyTransaction(piXMLTxn);
        piXMLTxn = IMS_NULL;
    }
}

PUBLIC
const RegKey& RegInfoParser::GetRegKey() const
{
    return objRegKey;
}

PUBLIC
IMS_BOOL RegInfoParser::Parse(IN const AString& strRegInfo)
{
    piXMLTxn = piXmlTxnProvider->CreateTransaction();

    if (piXMLTxn == IMS_NULL)
    {
        return IMS_FALSE;
    }

    piXMLTxn->SetListener(this);

    IXmlRequest* piRequest = piXMLTxn->GetRequest();

    if (piRequest->SetRawXml(strRegInfo) != IMS_SUCCESS)
    {
        piXmlTxnProvider->DestroyTransaction(piXMLTxn);
        piXMLTxn = IMS_NULL;

        return IMS_FALSE;
    }

    if (piXMLTxn->Send() != IMS_SUCCESS)
    {
        piXmlTxnProvider->DestroyTransaction(piXMLTxn);
        piXMLTxn = IMS_NULL;

        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void RegInfoParser::SetListener(IN IRegInfoParserListener* piListener)
{
    this->piListener = piListener;
}

PRIVATE VIRTUAL IMS_RESULT RegInfoParser::XmlTransaction_NotifyParsingCompleted(
        IN IXmlTransaction* piTransaction)
{
    if (piXMLTxn != piTransaction)
    {
        IMS_TRACE_E(0, "No matched XML transaction", 0, 0, 0);
        return IMS_FAILURE;
    }

    if (piXMLTxn == IMS_NULL)
    {
        return IMS_SUCCESS;
    }

    IXmlResponse* piResponse = piXMLTxn->GetResponse();

    if (piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "No 'reginfo' parser listener", 0, 0, 0);

        IDocument* piDocument = piResponse->GetDocument();

        if (piDocument != IMS_NULL)
        {
            piDocument->DestroyDocument();
        }

        piXmlTxnProvider->DestroyTransaction(piXMLTxn);
        piXMLTxn = IMS_NULL;

        return IMS_SUCCESS;
    }

    if (piResponse->GetResponseCode() == IXmlResponse::RESPONSE_CODE_SUCCESS)
    {
        IDocument* piDocument = piResponse->GetDocument();

        // Notify the XML parsing result for "reginfo"
        piListener->RegInfoParser_ParsingCompleted(this, piDocument);

        if (piDocument != IMS_NULL)
        {
            piDocument->DestroyDocument();
        }
    }
    else
    {
        piListener->RegInfoParser_ParsingFailed(this);
    }

    piXmlTxnProvider->DestroyTransaction(piXMLTxn);
    piXMLTxn = IMS_NULL;

    return IMS_SUCCESS;
}

#else

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "DomDocumentBuilderFactory.h"
#include "DocumentBuilderFactory.h"
#include "DocumentBuilder.h"
#include "IDocument.h"
#include "IRegInfoParserListener.h"
#include "RegInfoParser.h"

__IMS_TRACE_TAG_REG__;

PUBLIC
RegInfoParser::RegInfoParser(IN const RegKey& objRegKey_) :
        objRegKey(objRegKey_),
        piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL RegInfoParser::~RegInfoParser() {}

PUBLIC
const RegKey& RegInfoParser::GetRegKey() const
{
    return objRegKey;
}

PUBLIC
IMS_BOOL RegInfoParser::Parse(IN const AString& strRegInfo)
{
    DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
    DocumentBuilder* pDocumentBuilder = pBuilderFactory->NewDocumentBuilder();

    if (pDocumentBuilder == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IDocument* piDocument = pDocumentBuilder->Parse(strRegInfo);

    if (piDocument == IMS_NULL)
    {
        pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

        IMS_TRACE_E(0, "Parsing a 'reginfo' XML failed", 0, 0, 0);
        return IMS_FALSE;
    }

    // Notify the XML parsing result for "reginfo"
    if (piListener != IMS_NULL)
    {
        piListener->RegInfoParser_ParsingCompleted(this, piDocument);
    }

    piDocument->DestroyDocument();
    pBuilderFactory->DestroyDocumentBuilder(pDocumentBuilder);

    return IMS_TRUE;
}

PUBLIC
void RegInfoParser::SetListener(IN IRegInfoParserListener* piListener)
{
    this->piListener = piListener;
}

#endif  // __IMS_ASYNC_XML_PARSER__
