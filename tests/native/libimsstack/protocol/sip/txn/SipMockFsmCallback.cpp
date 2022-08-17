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
#include "txn/SipTxn.h"

SIP_BOOL MockFsm_FetchTransaction(
        SIP_VOID* pvTxnKey, SIP_INT32 nOption, SIP_VOID** /*ppvOutTxnKey*/, SIP_VOID** ppvTxn)
{
    if (nOption == TXN_OPT_CREATE)
    {
        if (strcmp((((SipTxnKey*)pvTxnKey))->GetMethod(), "BYE") == 0)
        {
            return SIP_FALSE;
        }
        if (*ppvTxn != SIP_NULL)
        {
            ((SipTxn*)*ppvTxn)->Decrement();
        }

        ((SipTxnKey*)pvTxnKey)->SipDelete();
        return SIP_TRUE;
    }
    else
    {
        SIP_INT32 eMsgType = ((SipTxnKey*)pvTxnKey)->GetMsgType();

        switch (eMsgType)
        {
            case SipMessage::REQ_TYPE:
            {
                if (strcmp((((SipTxnKey*)pvTxnKey))->GetMethod(), "CANCEL") == 0)
                {
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
    static int count = 0;
    if (count == 0)
    {
        count++;
        return SIP_FALSE;
    }
    count--;
    return SIP_TRUE;
}

SIP_BOOL MockFsm_ReleaseTransaction(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**)
{
    return SIP_TRUE;
}

SIP_VOID* MockFsm_CreateAckRequest(SIP_VOID*, ISipUserData*)
{
    return SIP_NULL;
}
