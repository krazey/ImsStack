/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20090326  toastops@                 Created
    </table>

    Description

*/

#ifndef _SIP_SERVER_TRANSACTION_STATE_H_
#define _SIP_SERVER_TRANSACTION_STATE_H_

#include "ITimer.h"
#include "SIPTransactionState.h"
#include "SIPTransportAddress.h"

class SIPServerTransactionState : public SIPTransactionState, public ITimerListener
{
public:
    SIPServerTransactionState(IN IMS_SINT32 nSlotId, IN CONST SIPTransportAddress& objNearEnd_,
            IN CONST SIPTransportAddress& objFarEnd_);
    virtual ~SIPServerTransactionState();

private:
    SIPServerTransactionState& operator=(IN CONST SIPServerTransactionState& objRHS);

public:
    virtual IMS_SINT32 CheckMessageValidity();
    virtual IMS_BOOL FormMessage();
    virtual IMS_BOOL Send(IN SipTimerValues* pTV = IMS_NULL);
    virtual IMS_BOOL UpdateTransportDetails();

    IMS_BOOL InitResponse(IN IMS_SINT32 nStatusCode);
    IMS_BOOL IsSameTransaction(IN CONST SIPServerTransactionState* pSTState) const;
    IMS_SINT32 MatchTransaction(IN SipMessage* pstMessage);
    void RejectRequest(
            IN IMS_SINT32 nStatusCode, IN CONST AString& strReason = AString::ConstNull());
    void SetDefaultContact(IN CONST AString& strContact);
    IMS_SINT32 HandleRequest(OUT RCPtr<SIPDialogEx>& pOrigDialogEx);

private:
    // ITimerListener interface
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

    IMS_BOOL InitResponse(IN IMS_SINT32 nStatusCode, OUT SipMessage*& pstOutMessage);
    IMS_BOOL UpdateTxnDetails();

    static IMS_BOOL Is100TryingResponseRequired(IN CONST SipMethod& objMethod);
    static IMS_RESULT SendResponse100Trying(IN SIPServerTransactionState* pSTState);
    static void StartTimer100Trying(IN SIPServerTransactionState* pSTState,
            IN IMS_SINT32 nTimerInterval /* milli-seconds */);
    static void StopTimer100Trying(IN SIPServerTransactionState* pSTState);

private:
    enum
    {
        STATE_IDLE = 0,
        STATE_PROCEEDING,
        STATE_COMPLETED,
        STATE_CONFIRMED,  // INVITE server txn only
        STATE_TERMINATED
    };

    AString strDefaultContact;

    // INVITE transaction
    //   The server transaction MUST generate a 100 (Trying) response unless it knows
    //   that the TU will generate a provisional or final response within 200 ms, in which case
    //   it MAY generate a 100 (Tyring) response.
    // Non-INVITE transaction
    //   RFC 4320
    ITimer* piTimer_100Trying;
};

#endif  // _SIP_SERVER_TRANSACTION_STATE_H_
