#ifndef _SIP_TXN_TIMEOUT_DATA_H
#define _SIP_TXN_TIMEOUT_DATA_H

#include "txn/sip_txn_common.h"
#include "txn/SipTxnKey.h"

/*Timerdata returned at timeout of Timer */
class SipTimeoutData
{
    /* : Txn which started the timer and its type */
    SIP_INT32 m_eTxnType;
    SIP_INT32 m_eTimerType;
    SipTxnKey* m_pTxnKey;

    /*******************************************************
      Private Member Functions
     ********************************************************/
    SipTimeoutData& operator=(IN const SipTimeoutData& objRHS);
    SipTimeoutData(IN const SipTimeoutData& objRHS);

public:
    SipTimeoutData();
    SipTimeoutData(SIP_INT32 eTxnType, SIP_INT32 eTimerType, SipTxnKey* pTxnKey);
    virtual ~SipTimeoutData();

    SipTxnKey* GetTxnKey() const;
    SIP_INT32 GetTimerType() const;
    SIP_INT32 GetTxnType() const;
    SIP_BOOL SetTxnKey(SipTxnKey* pTxnKey);
    SIP_BOOL SetTimerType(SIP_INT32 eTimerType);
    SIP_BOOL SetTxnType(SIP_INT32 eTxnType);
};

#endif
