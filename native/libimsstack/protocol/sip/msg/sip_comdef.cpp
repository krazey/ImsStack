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
#include "msg/sip_comdef.h"
#include "msg/SipMessage.h"
#include "SipTrace.h"
#include "sip_debug.h"
#include "platform/sip_pf_string.h"
#include "platform/sip_pf_memory.h"

SIP_UINT32 SipNPower(SIP_UINT16 nBase, SIP_UINT16 nIndex)
{
    SIP_UINT32 nRes = SIP_ONE;

    while (nIndex)
    {
        nRes = nRes * nBase;
        nIndex = nIndex - 1;
    }

    return nRes;
}

SIP_BOOL SipMemCheck(SIP_VOID* pvData, SIP_UINT16* pnError)
{
    if (pvData == SIP_NULL)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "New fails", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    if (*pnError == E_ERR_PF_MALLOCFAILED)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "Malloc fails", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    return SIP_TRUE;
}

/******************************************************************************
 * Function name    : CheckTxnMadatoryParams
 * Description    : Check Key Parameter to Start a Txn [Friend Function]
 * Return type    : SIP_BOOL
 *          Success/Failure
 * Argument     : pnError: <Out>
 *          Error code.
 * Preconditions/
 * Side Effects    : None
 *****************************************************************************/
SIP_BOOL CheckTxnMadatoryParams(SipMessage* pSipMsg, SIP_INT32* peMsgType, SIP_INT32* peMethodType)
{
    SIP_INT32 eMsgType = pSipMsg->GetMsgType();
    if (eMsgType == SipMessage::TYPE_INVALID)
    {
        return SIP_FALSE;
    }

    SIP_INT32 eMethodType = pSipMsg->GetMethodType();

    if (eMethodType == SipMessage::METHOD_INVALID)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "Invalid Method Type(%d != %d)", eMethodType,
                pSipMsg->GetMethodType());
        return SIP_FALSE;
    }

    /* Step 1: Check for Mandatory Headers for Transaction key */
    /* Check for To Hdr */
    if (pSipMsg->HasHeader(SipHeaderBase::TO) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'To' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::FROM) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'From' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::CSEQ) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'CSeq' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::CALL_ID) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'CallID' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }
    else if (pSipMsg->HasHeader(SipHeaderBase::VIA) == SIP_FALSE)
    {
        SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "'Via' Hdr not Found", SIP_ZERO, SIP_ZERO);
        return SIP_FALSE;
    }

    /* Step 2: Check for Request line */
    if (eMsgType == SipMessage::REQ_TYPE)
    {
        if (pSipMsg->IsReqLineExists() == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "IsReqLineExists Fails", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    else
    {
        if (pSipMsg->IsStatusLineExists() == SIP_FALSE)
        {
            SIP_DEBUG_WARNING(ESIPTRACE_MODTXN, "IsStatusLineExists Fails", SIP_ZERO, SIP_ZERO);
            return SIP_FALSE;
        }
    }
    *peMsgType = eMsgType;
    *peMethodType = eMethodType;

    return SIP_TRUE;
}

SIP_UINT32 GetRSeqNum(SipMessage* pMessage, SIP_INT32 eHdrType)
{
    SIP_UINT32 nRSeqNum = 0;

    if (pMessage != SIP_NULL &&
            (eHdrType == SipHeaderBase::RSEQ || eHdrType == SipHeaderBase::RACK))
    {
        SipHeaderBase* pHeader = pMessage->GetHeader(eHdrType, 0);

        if (pHeader != SIP_NULL)
        {
            if (eHdrType == SipHeaderBase::RSEQ)
            {
                SipIntegerHeader* pRSeq = (SipIntegerHeader*)pHeader;
                nRSeqNum = pRSeq->GetValueInt();
            }
            else
            {
                SipRAcKHeader* pRAck = (SipRAcKHeader*)pHeader;
                nRSeqNum = pRAck->GetResponseNum();
            }

            pHeader->SipDelete();
        }
    }

    return nRSeqNum;
}
