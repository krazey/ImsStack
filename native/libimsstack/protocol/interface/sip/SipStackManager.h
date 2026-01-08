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
#ifndef __SIP_STACK_MANAGER_H__
#define __SIP_STACK_MANAGER_H__

#include "SipDatatypes.h"

class ISipNetworkUtil;
class ISipTransactionCallback;
class ISipUserData;
class SipMessage;
class SipTransportInfo;
class SipTransportParameter;
class SipTxnKey;

class SipStackManager
{
    SipStackManager();

public:
    virtual ~SipStackManager();
    static SipStackManager* GetInstance();
    static void Destruct();

    SIP_BOOL SendMsg(SipMessage* pSipMsg, IN_OUT SipTransportParameter* pTranspParam,
            IN ISipUserData* pUserData, IN const SIP_CHAR* pSipBuffer, IN SIP_UINT32 nSipBufferLen,
            IN SipTxnKey** ppTxnKey, OUT SIP_UINT16* pnError);
    SIP_BOOL OnRecvMessage(SipMessage* pSipMsg, IN_OUT SipTransportParameter* pTranspParam,
            IN ISipUserData* pUserData, IN SIP_INT32* peTxnStatus, OUT SipTxnKey** ppTxnKey,
            OUT SIP_UINT16* pnError);
    SIP_BOOL OnRecvTanspError(SIP_INT32 eTranspError, SipTxnKey* pTxnKey, SIP_UINT16* pnError);
    SIP_BOOL TerminateTxn(SipTxnKey* pTxnKey);
    SIP_VOID RegisterNetwork(ISipNetworkUtil* pNetworkUtil);
    SIP_VOID RegisterTransactionCallback(IN ISipTransactionCallback* pCallback);

private:
    PRIVATE SIP_BOOL SendToNetwork(IN SipTransportInfo* pTranspInfo, IN ISipUserData* pUserData);
};

#endif  //__SIP_STACK_MANAGER_H__
