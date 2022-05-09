#ifndef _SIP_TXN_COMMON_H
#define _SIP_TXN_COMMON_H

#include "sip_pf_datatypes.h"

// SIP_TXN_POOL {
#define TXN_OPT_FETCH  0
#define TXN_OPT_CREATE 1
#define TXN_OPT_REMOVE 2

extern SIP_BOOL sip_cbk_fetchTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);
extern SIP_BOOL sip_cbk_releaseTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);
// }

#define SIP_TXN_WAITING_TIME_FACTOR 64
#define SIP_RETX_WAITING_TIME       32

typedef enum _SipEn_MsgDir
{
    /* set by Stack User for sending request or response */
    ETXN_SEND = 0,
    /* set by transport layer on receiving side of request or response */
    ETXN_RECV,
    ETXN_INVALIDDIR

} SipEn_MsgDir;

/* Type of Transaction is defined */
typedef enum _SipEn_TxnType
{
    ETXN_INVCLITXN = 0,
    ETXN_INVSERTXN,
    ETXN_NONINVCLITXN,
    ETXN_NONINVSERTXN,
    ETXN_TXNTYPEINVALID

} SipEn_TxnType;

typedef enum _SipEn_TxnStatus
{
    /* Request is received and no txn exist (Invite, Non-Invite, ACK) */
    ETXNSTATUS_NEWREQRECVD = 0,
    /* On recv of failure/success responses(INV & Non-INV) and transaction exists in valid state
       On recv of failure ACK,
       Re-transmitted 2xx for Invite.
       Pls Note: On Receive of INVITE Failure response, stack send the failure ACK to network*/
    ETXNSTATUS_VALIDMESSAGE,
    /* Re-transmitted request is received in state where no action is required
       Re-transmitted INVITE request or Retransmitted Failure ACK request received in confirmed
       state Re-transmitted non-INVITE request received in Trying state*/
    ETXNSTATUS_IGNOREREQ,
    /* Response received in state where no action is required
       INVITE 1xx received in completed state
       Non-INVITE response is received in completed state */
    ETXNSTATUS_IGNORERESP,
    /* No Txn exists when Non-INVITE responses or failure INVITE response is received */
    ETXNSTATUS_STRAYRESP,
    /* Re-transmitted req/resp are received in valid state
       For re-transmitted request, stack re-transmit the last response to network
       In INVITE Client txn, retransmitted failure resp can be received for which failure ACK to be
       sent
     */
    ETXNSTATUS_RETRANSMISSION,
    /* Error occurs when stack try to send message to the network */
    ETXNSTATUS_ERRORONSEND,
    /* When txn handler fails to handle receive message --> failure conditions */
    ETXNSTATUS_INVALIDMESSAGE,
    /*For Handling Stray 2xx*/
    ETXNSTATUS_2XX_STRAYRESP,
    ETXNSTATUS_STRAYPRACK,
    ETXNSTATUS_INVALID

} SipEn_TxnStatus;

typedef enum _SipEn_TimerType
{
    ETXN_TIMER1 = 0,
    ETXN_TIMER2,
    ETXN_TIMER4,
    ETXN_TIMERA,
    ETXN_TIMERB,
    ETXN_TIMERC,
    ETXN_TIMERD,
    ETXN_TIMERE,
    ETXN_TIMERF,
    ETXN_TIMERG,
    ETXN_TIMERH,
    ETXN_TIMERI,
    ETXN_TIMERJ,
    ETXN_TIMERK,
    ETXN_TIMEROTHER,
    ETXN_TIMERTYPEINVALID

} SipEn_TimerType;

typedef enum _SipEn_TxnInvCliFsmEvt
{
    ETXNINVCLI_SENDINVREQEVT = 0,
    ETXNINVCLI_TIMERA_B_TIMEOUTEVT,
    ETXNINVCLI_TIMERD_TIMEOUTEVT,
    ETXNINVCLI_RECV1XXRESPEVT,
    ETXNINVCLI_RECV2XXRESPEVT,
    ETXNINVCLI_RECV3XX6XXRESPEVT,
    ETXNINVCLI_TRANSPERROREVT,
    ETXNINVCLI_INVALIDEVT

} SipEn_TxnInvCliFsmEvt;

/* States for INVITE Client FSM */
typedef enum _SipEn_TxnInvCliFsmSt
{
    ETXNINVCLI_IDLEST = 0,
    ETXNINVCLI_CALLINGST,
    ETXNINVCLI_PROCEEDINGST,
    ETXNINVCLI_COMPLETEDST,
    ETXNINVCLI_TERMINATEDST,
    ETXNINVCLI_INVALIDST

} SipEn_TxnInvCliFsmSt;

typedef enum _SipEn_TxnInvSerFsmEvt
{
    ETXNINVSER_RECVINVREQEVT = 0,
    ETXNINVSER_SENDNON100PROVRESPEVT,
    ETXNINVSER_SEND3XX6XXFAILURERESPEVT,
    ETXNINVSER_SEND2XXSUCCESSRESPEVT,
    ETXNINVSER_TRANSPERROREVT,
    ETXNINVSER_RECVACKREQEVT,
    ETXNINVSER_TIMERG_H_TIMEOUTEVT,
    ETXNINVSER_TIMERI_TIMEOUTEVT,
    ETXNINVSER_INVALIDEVT

} SipEn_TxnInvSerFsmEvt;

/* States for INVITE Server FSM */
typedef enum _SipEn_TxnInvSerFsmSt
{
    ETXNINVSER_IDLEST = 0,
    ETXNINVSER_PROCEEDINGST,
    ETXNINVSER_COMPLETEDST,
    ETXNINVSER_CONFIRMEDST,
    ETXNINVSER_TERMINATEDST,
    ETXNINVSER_INVALIDST

} SipEn_TxnInvSerFsmSt;

typedef enum _SipEn_TxnNonInvCliFsmEvt
{
    ETXNNONINVCLI_SENDNONINVREQEVT = 0,
    ETXNNONINVCLI_TIMER_E_F_TIMEOUTEVT,
    ETXNNONINVCLI_RECV1XXRESPEVT,
    ETXNNONINVCLI_RECV2XX6XXRESPEVT,
    ETXNNONINVCLI_TRANSPERROREVT,
    ETXNNONINVCLI_TIMER_K_TIMEOUTEVT,
    ETXNNONINVCLI_INVALIDEVT

} SipEn_TxnNonInvCliFsmEvt;

/* States for non-INVITE Client FSM */
typedef enum _SipEn_TxnNonInvCliFsmSt
{
    ETXNNONINVCLI_IDLEST = 0,
    ETXNNONINVCLI_TRYINGST,
    ETXNNONINVCLI_PROCEEDINGST,
    ETXNNONINVCLI_COMPLETEDST,
    ETXNNONINVCLI_TERMINATEDST,
    ETXNNONINVCLI_INVALIDST

} SipEn_TxnNonInvCliFsmSt;

/* Events for non-INVITE Server FSM */
typedef enum _SipEn_TxnNonInvSerFsmEvt
{
    ETXNNONINVSER_RECVNONINVREQEVT = 0,
    ETXNNONINVSER_SEND1XXRESPEVT,
    ETXNNONINVSER_SEND2XX6XXRESPEVT,
    ETXNNONINVSER_TRANSPERROREVT,
    ETXNNONINVSER_TIMER_J_TIMEOUTEVT,
    ETXNNONINVSER_INVALIDEVT

} SipEn_TxnNonInvSerFsmEvt;

/* States for non-INVITE Server FSM */
typedef enum _SipEn_TxnNonInvSerFsmSt
{
    ETXNNONINVSER_IDLEST = 0,
    ETXNNONINVSER_TRYINGST,
    ETXNNONINVSER_PROCEEDINGST,
    ETXNNONINVSER_COMPLETEDST,
    ETXNNONINVSER_TERMINATEDST,
    ETXNNONINVSER_INVALIDST

} SipEn_TxnNonInvSerFsmSt;

#endif
