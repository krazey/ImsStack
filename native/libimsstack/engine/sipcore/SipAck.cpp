/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20140318  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTimer.h"
#include "IMSStrLib.h"
#include "SipStackHeaders.h"
#include "SipAck.h"

PUBLIC
SIPAck::SIPAck(IN SIPClientTransactionState* pCTState_, IN IMS_SINT32 nAliveInterval) :
        pCTState(pCTState_),
        piTimer(IMS_NULL)
{
    if (nAliveInterval > 0)
    {
        piTimer = TimerService::GetTimerService()->CreateTimer();

        if (piTimer != IMS_NULL)
        {
            piTimer->SetTimer(nAliveInterval, this);
        }
    }
}

PUBLIC VIRTUAL SIPAck::~SIPAck()
{
    if (piTimer != IMS_NULL)
    {
        piTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(piTimer);
        piTimer = IMS_NULL;
    }

    pCTState = IMS_NULL;
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPAck::IsSameTransaction(IN SipTxnKey* pstTxnKey) const
{
    //---------------------------------------------------------------------------------------------

    if (pCTState.IsNull())
    {
        return IMS_FALSE;
    }

    return SIPStack::CompareTxnKeysForAck(pCTState->GetTxnKey(), pstTxnKey);
}

/*

Remarks

*/
PUBLIC
IMS_BOOL SIPAck::IsStrayAck() const
{
    //---------------------------------------------------------------------------------------------

    return (piTimer == IMS_NULL);
}

/*

Remarks

*/
PUBLIC
void SIPAck::RetransmitMessage()
{
    //---------------------------------------------------------------------------------------------

    if (pCTState.IsNull())
    {
        return;
    }

    (void)pCTState->RetransmitMessage();
}

/*

Remarks

*/
PRIVATE VIRTUAL void SIPAck::Timer_TimerExpired(IN ITimer* piTimer)
{
    //---------------------------------------------------------------------------------------------

    if (this->piTimer == IMS_NULL)
    {
        return;
    }

    if (this->piTimer != piTimer)
    {
        return;
    }

    this->piTimer->KillTimer();
    TimerService::GetTimerService()->DestroyTimer(this->piTimer);
    this->piTimer = IMS_NULL;

    pCTState = IMS_NULL;
}
