#ifndef XML_TRANSACTION_H_
#define XML_TRANSACTION_H_

#include "IXmlTransaction.h"

class DocumentBuilder;
class XmlApp;
class XmlRequest;
class XmlResponse;

class XmlTransaction : public IXmlTransaction
{
public:
    XmlTransaction(IN XmlApp* pXmlApp);
    virtual ~XmlTransaction();
    XmlTransaction(IN const XmlTransaction& objOther) = delete;
    XmlTransaction& operator=(IN const XmlTransaction& objOther) = delete;

public:
    IXmlResponse* GetResponse() const override;
    IXmlRequest* GetRequest() const override;
    IMS_RESULT Send() override;
    void SetListener(IN IXmlTransactionListener* piListener) override;

    XmlResponse* CreateResponse();
    void NotifyParsingCompleted();
    void SetDocumentBuilder(IN DocumentBuilder* pDocumentBuilder);

private:
    XmlApp* m_pXmlApp;
    XmlRequest* m_pRequest;
    XmlResponse* m_pResponse;
    IXmlTransactionListener* m_piListener;
    DocumentBuilder* m_pDocumentBuilder;
};

#endif
