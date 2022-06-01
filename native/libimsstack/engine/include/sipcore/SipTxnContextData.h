/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090725  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_TXN_CONTEXT_DATA_H_
#define _SIP_TXN_CONTEXT_DATA_H_

#include "SipTransactionState.h"

class SIPTxnContextData
{
public:
    SIPTxnContextData();
    ~SIPTxnContextData();

private:
    SIPTxnContextData(IN CONST SIPTxnContextData& objRHS);
    SIPTxnContextData& operator=(IN CONST SIPTxnContextData& objRHS);

public:
    const SipMethod& GetMethod() const;
    SIPTransactionState* GetTxnState() const;
    void SetMethod(IN CONST SipMethod& objMethod);
    void SetTxnState(IN SIPTransactionState* pTxnState);

private:
    SipMethod objMethod;
    RCPtr<SIPTransactionState> pTxnState;
};

#endif  // _SIP_TXN_CONTEXT_DATA_H_
