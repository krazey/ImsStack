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
#ifndef __SIP_TXN_INFO_H__
#define __SIP_TXN_INFO_H__

#include "msg/SipMessage.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTxn.h"

class SipTxnInfo
{
public:
    SipTxnInfo();
    virtual ~SipTxnInfo();

    /*Recv Txn handling: When Txn Sending Failure ACK or 100 Trying Response */
    SipMessage* m_pSendSipMsg;

    /* Recv Txn handling: User data of existing transaction */
    ISipUserData* m_pUserData;

    /* Recv Txn handling: Transport info of existing transaction */
    SipTransportInfo* m_pTranspInfo;

    /* Recv Txn handling : transaction status */
    SIP_INT32 m_eTxnStatus;

    /* Recv/Send Txn handling : transaction termination status */
    SIP_BOOL m_bTxnTerminated;

    /* Recv/Send Txn handling : transaction creation status */
    SIP_BOOL m_bTxnCreated;
};

#endif  //__SIP_TXN_INFO_H__
