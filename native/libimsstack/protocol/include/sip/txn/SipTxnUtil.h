/*
   Author
   <table>
   date      author                 description
   --------  --------------         ----------
   20100000  syed.malgimani@        Created
   20170110  vijay.nair@            Modified
   </table>

   Description

 */

#ifndef __SIP_TXN_UTIL_H__
#define __SIP_TXN_UTIL_H__

#include "sip_pf_datatypes.h"
#include "msg/sip_comdef.h"
#include "msg/SipMessage.h"

class SipTxnUtil
{
    private:
        static SipTxnUtil* m_pSipTxnUtil;
        SipVector<SipTxnKey*> m_txnKeyList;
        SipTxnUtil();
        ~SipTxnUtil(){}
        SipTxnUtil(SipTxnUtil const& copy);//Not Implemented
        SIP_BOOL IsTxnKeyMatched(SipTxnKey* pUserTxnkey, SipTxnKey* pStoredTxnkey);

    public:
        static SipTxnUtil* GetInstance();
        SipTxnKey* SearchTxnKey(SipTxnKey* pTxn, SIP_BOOL bCheckRSeq = SIP_TRUE);
        SIP_BOOL AddTxnKey(SipTxnKey* pTxnKey);
        SIP_BOOL DeleteTxnKey(SipTxnKey* pTxnKey, SIP_BOOL bCheckToTag = SIP_FALSE);
};
#endif
