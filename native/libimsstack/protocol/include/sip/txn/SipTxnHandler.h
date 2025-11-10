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
#ifndef __SIP_TXN_HANDLER_H__
#define __SIP_TXN_HANDLER_H__

#include "msg/SipMessage.h"
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"
#include "txn/SipTxnInfo.h"

/* Transaction Key Used to Match Response to Request vice versa */
class SipTxnHandler
{
public:
    SipTxnHandler(){};
    virtual ~SipTxnHandler(){};

private:
    /* Based on Send/Receiving, Method and Request/Respose type Transaction is returnred */
    SIP_INT32 GetTxnType(SIP_INT32 eMsgDir, /* SEND or RECV */
            SIP_INT32 eMethodType, SIP_INT32 eMsgType);

    SIP_BOOL GetTxnObjFromDb(
            SipTxnKey* pTxnKey, SipTxn** ppTxn, SIP_BOOL* pbTxnExist, SIP_UINT16* pnError);

    SIP_BOOL GetTxnObjFromDb(SipTxnKey* pTxnKey, SipTxn** ppTxn, SipTxnKey** ppOutTxnKey,
            SIP_BOOL* pbTxnExist, SIP_UINT16* pnError);

    PRIVATE SIP_BOOL ValidateSendTxn(IN SipMessage* pSipMsg, OUT SIP_INT32* peTxnType,
            OUT SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError);

    /* method validates txn params from sip message and returns txn type */
    PRIVATE SIP_BOOL ValidateRecvTxn(IN SipMessage* pSipMsg, OUT SIP_INT32* peTxnType);

    /* invoking client FSM to send request. returns new txn object*/
    PRIVATE SIP_BOOL HandleClientTxnSend(IN SIP_INT32 eTxnType, IN const SipTxnKey* pTxnKey,
            IN SipTxnFsmData* pTxnFsmData, OUT SIP_UINT16* pnError);

    /* invokes server FSM to send response */
    PRIVATE SIP_BOOL HandleServerTxnSend(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey,
            IN SipTxnFsmData* pTxnFsmData, IN SIP_UINT16* pnError);

    /* invoking client FSM to to handle received response. */
    PRIVATE SIP_BOOL HandleClientTxnRecv(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey,
            IN SipTxnFsmData* pTxnFsmData, OUT SIP_UINT16* pnError);

    /* create server FSM to handle new received request. returns new txn object */
    PRIVATE SIP_BOOL HandleServerTxnRecv(IN SIP_INT32 eTxnType, IN SipTxnKey* pTxnKey,
            IN SipTxnFsmData* pTxnFsmData, OUT SIP_UINT16* pnError);

    /* Notifies to Transaction User using registered listener */
    PRIVATE SIP_VOID NotifyTxnTermination(SipTxn* pTxn);

public:
    SIP_BOOL OnSendTxn(SipMessage* pSipMsg, IN_OUT SipTransportParameter* pTranspParam,
            IN ISipUserData* pUserData, IN SipTxnKey** ppTxnKey, OUT SipTxnInfo* pTxnInfo,
            OUT SIP_UINT16* pnError);

    SIP_BOOL OnRecvTxn(IN SipMessage* pSipMsg, IN SipTxnKey* pTxnKey, IN ISipUserData* pUserData,
            OUT SipTxnInfo* pTxnInfo, OUT SIP_UINT16* pnError);

    SIP_BOOL UpdateTxnDetails(
            SipTxnKey* pTxnKey, SipTransportInfo* pTranspInfo, SIP_UINT16* pnError);

    SIP_BOOL OnRecvTranspError(SIP_INT32 eTransErrro, SipTxnKey* pTxnKey, SIP_UINT16* pnError);

    SIP_BOOL OnSendTranspError(SipTxnKey* pTxnKey);

    SIP_BOOL TerminateTxn(SipTxnKey* pTxnKey);

    SIP_BOOL DeleteTxn(SipTxnKey* pTxnKey);
};
#endif  //__SIP_TXN_HANDLER_H__
