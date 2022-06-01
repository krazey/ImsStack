/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140318  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _SIP_ACK_H_
#define _SIP_ACK_H_

#include "ITimer.h"
#include "SipClientTransactionState.h"

class SIPAck : public ITimerListener
{
public:
    SIPAck(IN SIPClientTransactionState* pCTState_, IN IMS_SINT32 nAliveInterval);
    virtual ~SIPAck();

private:
    SIPAck();
    SIPAck(IN CONST SIPAck& objRHS);
    SIPAck& operator=(IN CONST SIPAck& objRHS);

public:
    IMS_BOOL IsSameTransaction(IN SipTxnKey* pstTxnKey) const;
    IMS_BOOL IsStrayAck() const;
    void RetransmitMessage();

private:
    // ITimerListener class
    virtual void Timer_TimerExpired(IN ITimer* piTimer);

private:
    RCPtr<SIPClientTransactionState> pCTState;
    ITimer* piTimer;
};

#endif  // _SIP_ACK_H_
