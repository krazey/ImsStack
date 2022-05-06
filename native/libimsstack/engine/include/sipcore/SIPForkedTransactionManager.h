/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100707  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_FORKED_TRANSACTION_MANAGER_H_
#define _SIP_FORKED_TRANSACTION_MANAGER_H_

#include "SIPStackHeaders.h"
#include "RCObject.h"

class SIPClientTransactionState;

class SIPForkedTransactionManager : public RCObject
{
public:
    SIPForkedTransactionManager();
    SIPForkedTransactionManager(IN const SIPForkedTransactionManager& objRHS);
    virtual ~SIPForkedTransactionManager();

private:
    SIPForkedTransactionManager& operator=(IN CONST SIPForkedTransactionManager& objRHS);

public:
    IMS_BOOL Add(IN SIPClientTransactionState* pCTState);
    IMS_BOOL IsEmpty() const;
    IMS_BOOL IsTransactionCompleted() const;
    SIPClientTransactionState* Lookup(IN SipMessage* pstMessage) const;
    void Remove(IN SIPClientTransactionState* pCTState);
    void SetTransactionCompleted(IN IMS_SINT32 nStatusCode);

private:
    // FIX_NO_ACK_RETRANSMISSION :: this will be used for 2xx response received case only
    IMS_SINT32 nStatusCode;
    IMSList<RCPtr<SIPClientTransactionState>> objTxnStates;
};

#endif  // _SIP_FORKED_TRANSACTION_MANAGER_H_
