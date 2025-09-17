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
#ifndef __SIP_TRANSPORT_HANDLER_H__
#define __SIP_TRANSPORT_HANDLER_H__

#include "transport/SipTransportInfo.h"
#include "txn/SipTxn.h"

class SipTransportHandler
{
public:
    SipTransportHandler(){};
    virtual ~SipTransportHandler() {}

private:
    SIP_BOOL UpdateViaSipMsg(
            SipMessage* pSipMsg, SipTransportBuffer* pSentBuffer, SIP_INT32 eChangeProto);

    PRIVATE SIP_BOOL GetTxnKeyFromSipMsg(
            IN SipMessage* pSipMsg, OUT SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError);

    PRIVATE SIP_BOOL GetTxnObjFromDb(IN SipTxnKey* pTxnKey, OUT SipTxn** ppTxn,
            OUT SIP_BOOL* pbTxnExist, OUT SIP_UINT16* pnError);

public:
    SIP_BOOL OnSendTransp(IN SipMessage* pSipMsg, IN SipTransportParameter* pTranspParam,
            IN const SIP_CHAR* pSipBuffer, IN SIP_UINT32 nSipBufferLen,
            OUT SipTransportInfo** ppTranspInfo, OUT SIP_UINT16* pnError);

    SIP_BOOL OnRecvTransp(IN SipMessage* pSipMsg, IN SipTransportParameter* pTranspParam,
            OUT SIP_INT32* peTxnStatus, OUT SIP_BOOL* pbTxnExist, OUT SipTxnKey** ppTxnKey,
            OUT SIP_UINT16* pnError);

    SIP_BOOL OnRecvTanspError(SIP_INT32 eTranspError, SipTxnKey* pTxnKey, SIP_INT32* peTxnStatus,
            SipTransportInfo** ppTranspInfo, ISipUserData* pUserData, SIP_UINT16* pnError);
};

#endif  //__SIP_TRANSPORT_HANDLER_H__
