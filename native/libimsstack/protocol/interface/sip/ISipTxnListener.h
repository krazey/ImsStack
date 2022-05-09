#ifndef __ISIPTXNLISTENER_H__
#define __ISIPTXNLISTENER_H__

#include "sip_pf_datatypes.h"

class ISipTxnListener
{
public:
    ISipTxnListener(){};
    virtual ~ISipTxnListener(){};

    /* read only txn key */
    virtual SIP_BOOL TxnTimeout(ISipUserData* pUserData, SIP_INT32 ms_TimerType) = 0;

    /* read only txn key */
    virtual SIP_BOOL TxnTerminated(ISipUserData* pUserData) = 0;
};
#endif  //__ISIPTXNLISTENER_H__
