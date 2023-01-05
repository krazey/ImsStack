/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ReliableProvResponseHelper.h"

__IMS_TRACE_TAG_IMS_CORE__;

PUBLIC
ReliableProvResponseHelper::ReliableProvResponseHelper(IN IMS_BOOL bIsMobileOriginated) :
        m_bIsMobileOriginated(bIsMobileOriginated),
        m_nState(STATE_IDLE),
        m_nRSeqNumber(1),
        m_nCSeqNumber(0)
{
    IMS_TRACE_D("Constructor :: ReliableProvResponseHelper", 0, 0, 0);
}

PUBLIC
ReliableProvResponseHelper::~ReliableProvResponseHelper()
{
    IMS_TRACE_D("Destructor :: ReliableProvResponseHelper", 0, 0, 0);
}

PUBLIC
void ReliableProvResponseHelper::Initialize(IN ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    IMS_BOOL bOk;
    IMS_SINT32 nNumber;

    if (m_bIsMobileOriginated)
    {
        // When the first RPR is received ...

        AString strRSeq = piSipMsg->GetHeader(ISipHeader::RSEQ);

        nNumber = strRSeq.ToInt32(&bOk);

        if (bOk)
        {
            m_nRSeqNumber = nNumber;
        }
    }
    else
    {
        // When a new request is received ...
        m_nRSeqNumber = 1;
    }

    AString strCSeq = piSipMsg->GetHeader(ISipHeader::CSEQ);
    IMS_SINT32 nIndex = strCSeq.GetIndexOf(' ');

    strCSeq.Truncate(nIndex);

    nNumber = strCSeq.ToInt32(&bOk);

    if (bOk)
    {
        m_nCSeqNumber = nNumber;
    }

    m_objMethod = piSipMsg->GetMethod();
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::SetRAckHeader(IN_OUT ISipMessage*& piSipMsg) const
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    // response-num LWS CSeq-num LWS METHOD
    AString strRAck;

    strRAck.Sprintf("%u %u %s", m_nRSeqNumber, m_nCSeqNumber, m_objMethod.ToString().GetStr());

    if (piSipMsg->SetHeader(ISipHeader::RACK, strRAck) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting RAck header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::SetRSeqHeader(IN_OUT ISipMessage*& piSipMsg) const
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strRSeq;
    strRSeq.SetNumber(m_nRSeqNumber);

    if (piSipMsg->SetHeader(ISipHeader::RSEQ, strRSeq) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Setting RSeq header failed", 0, 0, 0);
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::UpdateOnMessageReceived(IN ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (objMethod.Equals(SipMethod::PRACK))
    {
        if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
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
        if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            AString strRSeq = piSipMsg->GetHeader(ISipHeader::RSEQ);
            IMS_BOOL bOk = IMS_FALSE;
            IMS_SINT32 nNumber = strRSeq.ToInt32(&bOk);

            if (!bOk)
            {
                return IMS_FALSE;
            }

            m_nRSeqNumber = nNumber;

            SetState(STATE_RPR_RECEIVED);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::UpdateOnMessageSent(IN ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const SipMethod& objMethod = piSipMsg->GetMethod();

    if (objMethod.Equals(SipMethod::PRACK))
    {
        if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
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
        if (piSipMsg->GetType() == ISipMessage::TYPE_RESPONSE)
        {
            // Increment the response sequence number for the following RPRs
            ++m_nRSeqNumber;

            SetState(STATE_RPR_SENT);
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL ReliableProvResponseHelper::UpdateOnOperationFailed()
{
    SetState(STATE_IDLE);
    return IMS_TRUE;
}

PRIVATE
void ReliableProvResponseHelper::SetState(IN IMS_SINT32 nState)
{
    IMS_TRACE_I(
            "ReliableProvResponse :: %s to %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
}

PRIVATE
const IMS_CHAR* ReliableProvResponseHelper::StateToString(IN IMS_SINT32 nState)
{
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
