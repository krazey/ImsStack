#include "DomDocumentBuilderFactory.h"
#include "IXmlRequest.h"
#include "IXmlResponse.h"
#include "IXmlTransactionListener.h"
#include "XmlApp.h"
#include "XmlRequest.h"
#include "XmlResponse.h"
#include "XmlTransaction.h"

PUBLIC
XmlTransaction ::XmlTransaction(IN XmlApp* pXmlApp) :
        m_pXmlApp(pXmlApp),
        m_pRequest(new XmlRequest()),
        m_pResponse(IMS_NULL),
        m_piListener(IMS_NULL),
        m_pDocumentBuilder(IMS_NULL)
{
}

PUBLIC VIRTUAL XmlTransaction::~XmlTransaction()
{
    if (m_pRequest != IMS_NULL)
    {
        delete m_pRequest;
        m_pRequest = IMS_NULL;
    }

    if (m_pResponse != IMS_NULL)
    {
        delete m_pResponse;
        m_pResponse = IMS_NULL;
    }

    if (m_pDocumentBuilder != IMS_NULL)
    {
        DomDocumentBuilderFactory* pBuilderFactory = DomDocumentBuilderFactory::GetInstance();
        pBuilderFactory->DestroyDocumentBuilder(m_pDocumentBuilder);
    }
}

PUBLIC VIRTUAL IXmlResponse* XmlTransaction::GetResponse() const
{
    return m_pResponse;
}

PUBLIC VIRTUAL IXmlRequest* XmlTransaction::GetRequest() const
{
    return m_pRequest;
}

PUBLIC VIRTUAL IMS_RESULT XmlTransaction::Send()
{
    if (m_pRequest == IMS_NULL || m_pXmlApp == IMS_NULL || m_piListener == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_pXmlApp->Parse(this) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL void XmlTransaction::SetListener(IN IXmlTransactionListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
XmlResponse* XmlTransaction::CreateResponse()
{
    m_pResponse = new XmlResponse();
    return m_pResponse;
}

PUBLIC
void XmlTransaction::NotifyParsingCompleted()
{
    if (m_piListener != IMS_NULL)
    {
        m_piListener->XmlTransaction_NotifyParsingCompleted(this);
    }
}

PUBLIC
void XmlTransaction::SetDocumentBuilder(IN DocumentBuilder* pDocumentBuilder)
{
    m_pDocumentBuilder = pDocumentBuilder;
}
