#ifndef __SIP_TRANSPORT_HANDLER_H__
#define __SIP_TRANSPORT_HANDLER_H__

#include "txn/SipTxn.h"
#include "transport/SipTransportInfo.h"

class SipTransportHandler
{
public:
    SipTransportHandler(){};
    virtual ~SipTransportHandler() {}

private:
    /************************************************************
      Member Functions
     **************************************************************/
    SIP_BOOL UpdateViaSipMsg(
            SipMessage* pSipMsg, SipTransportBuffer* pSentBuffer, SIP_INT32 eChangeProto);

    PRIVATE SIP_BOOL GetTxnKeyFromSipMsg(
            IN SipMessage* pSipMsg, OUT SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError);

    PRIVATE SIP_BOOL GetTxnObjFromDb(IN SipTxnKey* pTxnKey, OUT SipTxn** ppTxn,
            OUT SIP_BOOL* pbTxnExist, OUT SIP_UINT16* pnError);

public:
    /************************************************************
      Member Functions
     **************************************************************/

    SIP_BOOL OnSendTransp(IN SipMessage* pSipMsg, IN SipTransportParameter* pTranspParam,
            IN SIP_CHAR* pSipBuffer, IN SIP_UINT32 nSipBufferLen,
            OUT SipTransportInfo** ppTranspInfo, OUT SIP_UINT16* pnError);

    SIP_BOOL OnRecvTransp(IN SipMessage* pSipMsg, IN SipTransportParameter* pTranspParam,
            OUT SIP_INT32* peTxnStatus, OUT SIP_BOOL* pbTxnExist, OUT SipTxnKey** ppTxnKey,
            OUT SIP_UINT16* pnError);

    SIP_BOOL OnRecvTanspError(SIP_INT32 eTranspError, SipTxnKey* pTxnKey, SIP_INT32* peTxnStatus,
            SipTransportInfo** ppTranspInfo, ISipUserData* pUserData, SIP_UINT16* pnError);

    SIP_BOOL IsInviteTxnPresentForAckTxn(IN SipTxnKey* pAckTxnKey);
};

#endif  //__SIP_TRANSPORT_HANDLER_H__
