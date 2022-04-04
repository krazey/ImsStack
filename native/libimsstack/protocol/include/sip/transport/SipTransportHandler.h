/******************************************************************************
 * Project Name     : SIP_RTP
 * Group            : IP-CS [MSG-2]
 * Security         : Confidential
 *****************************************************************************/

/******************************************************************************
 * Filename          : SipTransportInfo.hander.h
 * Purpose           : SIP Transport Functions : Ref: SipTransportInfo.handler.h
 * Platform          : Windows OR Android
 * Author(s)         : Syed Malgimani
 * E-mail id.        : syed.malgimani@
 * Creation date     : July. 26,2010
 *
 * Edit History             Modification                         Description(s)
 *
 * Date                Name            Version        Bug-ID        Description
 * ----------        ----------        -------        ------        -------------
 * Month. Date,10        Name                 0.0a            Initial creation

 *****************************************************************************/

#ifndef __SIP_TRANSPORT_HANDLER_H__
#define __SIP_TRANSPORT_HANDLER_H__

#include "txn/SipTxn.h"
#include "transport/SipTransportInfo.h"

/*****************************************************************************
  Enum Declaration
 *****************************************************************************/

/****************************************************************************
SipTransportHandler: Class Declaration starts
 *****************************************************************************/
class SipTransportHandler
{
    public:
        SipTransportHandler(){};
        virtual ~SipTransportHandler(){}
    private:
        /************************************************************
          Member Functions
         **************************************************************/
        SIP_BOOL UpdateViaSipMsg(SipMessage* pSipMsg, SipTransportBuffer* pSentBuffer,
                SIP_INT32 eChangeProto);

        PRIVATE SIP_BOOL GetTxnKeyFromSipMsg(IN SipMessage* pSipMsg, OUT SipTxnKey** ppTxnKey,
                OUT SIP_UINT16* pnError);

        PRIVATE SIP_BOOL GetTxnObjFromDb(IN SipTxnKey* pTxnKey, OUT SipTxn** ppTxn,
                OUT SIP_BOOL* pbTxnExist, OUT SIP_UINT16* pnError);

    public:

        /************************************************************
          Member Functions
         **************************************************************/

        SIP_BOOL OnSendTransp(IN SipMessage* pSipMsg, IN SipTransportParameter* pTranspParam,
                IN SIP_CHAR* pSipBuffer, IN  SIP_UINT32 nSipBufferLen,
                OUT SipTransportInfo** ppTranspInfo, OUT SIP_UINT16* pnError);

        SIP_BOOL OnRecvTransp(IN SipMessage* pSipMsg, IN SipTransportParameter* pTranspParam,
                OUT SIP_INT32* peTxnStatus, OUT SIP_BOOL* pbTxnExist, OUT SipTxnKey** ppTxnKey,
                OUT SIP_UINT16* pnError);

        SIP_BOOL OnRecvTanspError(SIP_INT32 eTranspError, SipTxnKey* pTxnKey,
                SIP_INT32* peTxnStatus, SipTransportInfo** ppTranspInfo, ISipUserData* pUserData,
                SIP_UINT16* pnError);

        SIP_BOOL IsInviteTxnPresentForAckTxn(IN SipTxnKey* pAckTxnKey);
};
/****************************************************************************
SipTransportHandler: Class Declaration End
 *****************************************************************************/

#endif //__SIP_TRANSPORT_HANDLER_H__
