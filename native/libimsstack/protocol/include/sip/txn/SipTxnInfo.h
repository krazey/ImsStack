/*
   Author
   <table>
   date      author                description
   --------  --------------        ----------
   20170110  vijay.nair@           Created
   </table>

   Description

 */

#ifndef _SIP_TXN_INFO_H
#define _SIP_TXN_INFO_H

#include "txn/sip_txn_common.h"

#include "msg/SipMessage.h"

#include "transport/SipTransportInfo.h"

class SipTxnInfo
{
    public:
        SipTxnInfo    ();
        virtual ~SipTxnInfo();

        /*Recv Txn handling: When Txn Sending Failure ACK or 100 Trying Response */
        SipMessage* m_pSendSipMsg;

        /* Recv Txn handling: User data of existing transaction */
        ISipUserData* m_pUserData;

        /* Recv Txn handling: Transport info of existing transaction */
        SipTransportInfo* m_pTranspInfo;

        /* Recv Txn handling : transaction status */
        SIP_INT32 eTxnStatus;

        /* Recv/Send Txn handling : transaction termination status */
        SIP_BOOL bTxnTerminated;

        /* Recv/Send Txn handling : transaction creation status */
        SIP_BOOL bTxnCreated;
};

#endif
