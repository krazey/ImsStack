#ifndef XML_TRANSACTION_PROVIDER_H_
#define XML_TRANSACTION_PROVIDER_H_

#include "IMSActivityEx.h"
#include "IMSQueue.h"
#include "IXmlTransactionProvider.h"

class XmlApp;

class XmlTransactionProvider : public IMSActivityEx, public IXmlTransactionProvider
{
public:
    XmlTransactionProvider();
    virtual ~XmlTransactionProvider();
    XmlTransactionProvider(IN const XmlTransactionProvider& objOther) = delete;
    XmlTransactionProvider& operator=(IN const XmlTransactionProvider& objOther) = delete;

public:
    IXmlTransaction* CreateTransaction() override;
    void DestroyTransaction(IN IXmlTransaction*& piTransaction) override;
    IMS_RESULT Push(IN IXmlTransaction* piTransaction) override;
    IXmlTransaction* Pop() override;
    inline IMS_SINT32 GetState() const override { return m_nState; }
    inline void SetStateListener(IN IXmlStateListener* piListener) override
    {
        m_piListener = piListener;
    }

private:
    IMS_BOOL OnMessage(IN IMSMSG& objMsg) override;
    void Attach();
    void Detach();
    void SetState(IN IMS_SINT32 nState);

private:
    IMS_SINT32 m_nState;
    IXmlStateListener* m_piListener;
    XmlApp* m_pXmlApp;
    IMSQueue<IXmlTransaction*> m_objTransactions;
};

#endif
