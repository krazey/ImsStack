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
#ifndef __SIP_TXN_FSM_DATA_H__
#define __SIP_TXN_FSM_DATA_H__

#include "msg/SipMessage.h"
#include "transport/SipTransportInfo.h"
#include "txn/SipTxn.h"

class SipTxnFsmData
{
public:
    SipTxnFsmData(
            SipMessage* pSipMsg, SipTransportParameter* pTranspParam, ISipUserData* pUserData);
    virtual ~SipTxnFsmData();
    /************ INPUT Variables ***********/
    /* SIP Req/Resp Object */
    SipMessage* m_pSipMsgIn;

    /*Transport parameter given by User */
    SipTransportParameter* m_pTranspParam;

    /* User Specific Data or User Key Info. This is passed by the user */
    ISipUserData* m_pUserData;

    /************ OUTPUT Variables ***********/

    /*Recv Txn handling: When Txn Sending Failure ACK or 100 Trying Response */
    SipMessage* m_pSendSipMsg;

    /* Recv Txn handling: layer status on received side */
    SIP_INT32 m_eTxnStatus;

    /* Recv Txn handling: User while re-tranamission of message on receive of re-transmitted message
     */
    ISipUserData* m_pOutUserData;

    /* Recv Txn handling: User while re-tranamission of message on receive of re-transmitted message
     */
    SipTransportInfo* m_pTranspInfo;

    /* Used for both Send and Recv Txn */
    SIP_BOOL m_bTxnTerminated;
    SIP_BOOL m_bTxnCreated;
};

#endif  //__SIP_TXN_FSM_DATA_H__
