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
#include "SipUtil.h"
#include "include/MockSipTransaction.h"
#include "platform/SipString.h"
#include "txn/SipTxn.h"

static SIP_INT32 count = 0;
SipVector<MockSipTransaction*> objFsmTxnList;
SIP_BOOL MockFsm_FetchTransaction(
        SIP_VOID* pvTxnKey, SIP_INT32 nOption, SIP_VOID** /*ppvOutTxnKey*/, SIP_VOID** ppvTxn)
{
    if ((pvTxnKey == SIP_NULL) || (ppvTxn == SIP_NULL))
    {
        return SIP_FALSE;
    }

    SipTxnKey* pTxnKey = static_cast<SipTxnKey*>(pvTxnKey);
    if (nOption == SipTxn::OPT_CREATE)
    {
        if (SipPf_Strcmp(pTxnKey->GetMethod(), "BYE") == 0)
        {
            return SIP_FALSE;
        }
        if ((ppvTxn != SIP_NULL) && (*ppvTxn != SIP_NULL))
        {
            MockSipTransaction* pMockTxn =
                    new MockSipTransaction(pTxnKey, reinterpret_cast<SipTxn*>(*ppvTxn));
            objFsmTxnList.Add(pMockTxn);
        }
        return SIP_TRUE;
    }
    else
    {
        SIP_UINT32 nSize = objFsmTxnList.GetSize();

        for (SIP_UINT32 i = 0; i < nSize; i++)
        {
            MockSipTransaction* pMockTxn = objFsmTxnList.GetAt(i);
            SipTxn* pTxn = pMockTxn->GetTxn();

            if (pTxn != SIP_NULL)
            {
                if (pTxnKey->CompareKeys(pTxn->GetTxnKey()) == SIP_MATCHES)
                {
                    if (ppvTxn != SIP_NULL)
                    {
                        *ppvTxn = pTxn;
                        return SIP_TRUE;
                    }
                }
            }
        }

        SIP_INT32 eMsgType = pTxnKey->GetMsgType();

        switch (eMsgType)
        {
            case SipMessage::REQ_TYPE:
            {
                if (SipPf_Strcmp(pTxnKey->GetMethod(), "CANCEL") == 0)
                {
                    return SIP_TRUE;
                }
                return SIP_FALSE;
            }
            case SipMessage::RESP_TYPE:
            {
                if (SipPf_Strcmp(pTxnKey->GetMethod(), "INVITE") == 0)
                {
                    SIP_UINT16 nError;
                    SipMessage* pTempSipMsg = new SipMessage();
                    *ppvTxn = new SipTxn(
                            SipTxn::INVITE_SERVER, pTxnKey, pTempSipMsg, SIP_NULL, &nError);
                    pTempSipMsg->SipDelete();
                    return SIP_TRUE;
                }
                return SIP_FALSE;
            }
            default:
                return SIP_FALSE;
        }
    }
}
/* Mocked the Start Timer api to return failure when called first time/count is 0
   and false when called second time/count is 1 and decremented count to continue.
*/
SIP_BOOL MockFsm_StartTimer(SIP_UINT32, SipTimerCallback, SIP_VOID*, SIP_VOID**)
{
    if (count == 0)
    {
        count++;
        return SIP_FALSE;
    }
    count--;
    return SIP_TRUE;
}

SIP_BOOL MockFsm_ReleaseTransaction(
        SIP_VOID* pvTxnKey, SIP_INT32, SIP_VOID** ppvOutTxnKey, SIP_VOID** ppvTxn)
{
    SIP_UINT32 nSize = objFsmTxnList.GetSize();

    for (SIP_UINT32 i = 0; i < nSize; i++)
    {
        MockSipTransaction* pMockTxn = objFsmTxnList.GetAt(i);
        SipTxn* pTxn = pMockTxn->GetTxn();

        if (pTxn != SIP_NULL)
        {
            if ((static_cast<SipTxnKey*>(pvTxnKey))->CompareKeys(pTxn->GetTxnKey()) == SIP_MATCHES)
            {
                if (ppvOutTxnKey != IMS_NULL)
                {
                    (*ppvOutTxnKey) = pMockTxn->GetKey();
                }

                if (ppvTxn != IMS_NULL)
                {
                    (*ppvTxn) = pTxn;
                }
                delete pMockTxn;
                objFsmTxnList.RemoveAt(i);
                return SIP_TRUE;
            }
        }
    }
    return SIP_TRUE;
}

SIP_VOID* MockFsm_CreateAckRequest(SIP_VOID*, ISipUserData*)
{
    return SIP_NULL;
}

SIP_VOID MockFsm_ResetTimerCount()
{
    count = 0;
}
