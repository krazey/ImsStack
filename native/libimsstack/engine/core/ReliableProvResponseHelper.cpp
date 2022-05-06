/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100614  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ReliableProvResponseHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
ReliableProvResponseHelper::ReliableProvResponseHelper(IN IMS_BOOL bIsMobileOriginated_) :
        bIsMobileOriginated(bIsMobileOriginated_),
        nState(STATE_IDLE),
        nRSeqNumber(1),
        nCSeqNumber(0)
{
    IMS_TRACE_D("Constructor :: ReliableProvResponseHelper", 0, 0, 0);
}

PUBLIC
ReliableProvResponseHelper::~ReliableProvResponseHelper()
{
    IMS_TRACE_D("Destructor :: ReliableProvResponseHelper", 0, 0, 0);
}

PUBLIC
IMS_SINT32 ReliableProvResponseHelper::GetState() const
{
    //---------------------------------------------------------------------------------------------

    return nState;
}

PUBLIC
void ReliableProvResponseHelper::Initialize(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return;
    }

    IMS_BOOL bOK;
    IMS_SINT32 nNumber;

    if (bIsMobileOriginated)
    {
        // When the first RPR is received ...

        AString strRSeq = piSIPMsg->GetHeader(ISipHeader::RSEQ);

        nNumber = strRSeq.ToInt32(&bOK);

        if (bOK)
        {
            nRSeqNumber = nNumber;
        }
    }
    else
    {
        // When a new request is received ...

        nRSeqNumber = 1;
    }

    AString strCSeq = piSIPMsg->GetHeader(ISipHeader::CSEQ);
    IMS_SINT32 nIndex = strCSeq.GetIndexOf(' ');

    strCSeq.Truncate(nIndex);

    nNumber = strCSeq.ToInt32(&bOK);

    if (bOK)
    {
        nCSeqNumber = nNumber;
    }

    objMethod = piSIPMsg->GetMethod();
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::SetRAckHeader(IN_OUT ISipMessage*& piSIPMsg) const
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // response-num LWS CSeq-num LWS METHOD
    AString strRAck;

    strRAck.Sprintf("%u %u %s", nRSeqNumber, nCSeqNumber, objMethod.ToString().GetStr());

    if (piSIPMsg->SetHeader(ISipHeader::RACK, strRAck) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting RAck header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::SetRSeqHeader(IN_OUT ISipMessage*& piSIPMsg) const
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strRSeq;
    strRSeq.SetNumber(nRSeqNumber);

    if (piSIPMsg->SetHeader(ISipHeader::RSEQ, strRSeq) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting RSeq header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::UpdateOnMessageReceived(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSIPMsg->GetMethod();

    if (objMethod.Equals(SipMethod::PRACK))
    {
        if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            SetState(STATE_IDLE);
        }
        else
        {
            SetState(STATE_PRACK_RECEIVED);
        }
    }
    else
    {
        if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            AString strRSeq = piSIPMsg->GetHeader(ISipHeader::RSEQ);
            IMS_BOOL bOK = IMS_FALSE;
            IMS_SINT32 nNumber = strRSeq.ToInt32(&bOK);

            if (!bOK)
            {
                return IMS_FALSE;
            }

            nRSeqNumber = nNumber;

            SetState(STATE_RPR_RECEIVED);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::UpdateOnMessageSent(IN ISipMessage* piSIPMsg)
{
    //---------------------------------------------------------------------------------------------

    if (piSIPMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSIPMsg->GetMethod();

    if (objMethod.Equals(SipMethod::PRACK))
    {
        if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            SetState(STATE_IDLE);
        }
        else
        {
            SetState(STATE_PRACK_SENT);
        }
    }
    else
    {
        if (piSIPMsg->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            // Increment the response sequence number for the following RPRs
            ++nRSeqNumber;

            SetState(STATE_RPR_SENT);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::UpdateOnOperationFailed()
{
    //---------------------------------------------------------------------------------------------

    SetState(STATE_IDLE);

    return IMS_TRUE;
}

PRIVATE
void ReliableProvResponseHelper::SetState(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    IMS_TRACE_I("ReliableProvResponse :: %s to %s", StateToString(this->nState),
            StateToString(nState), 0);

    this->nState = nState;
}

PRIVATE
const IMS_CHAR* ReliableProvResponseHelper::StateToString(IN IMS_SINT32 nState)
{
    //---------------------------------------------------------------------------------------------

    switch (nState)
    {
        case STATE_IDLE:
            return "STATE_IDLE";
        case STATE_RPR_RECEIVED:
            return "STATE_RPR_RECEIVED";
        case STATE_PRACK_SENT:
            return "STATE_PRACK_SENT";
        case STATE_RPR_SENT:
            return "STATE_RPR_SENT";
        case STATE_PRACK_RECEIVED:
            return "STATE_PRACK_RECEIVED";
        default:
            return "__INVALID__";
    }
}
