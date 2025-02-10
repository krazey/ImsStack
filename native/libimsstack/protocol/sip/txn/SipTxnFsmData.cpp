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
#include "txn/SipTxn.h"
#include "txn/SipTxnFsmData.h"

SipTxnFsmData::SipTxnFsmData(
        SipMessage* pSipMsg, SipTransportParameter* pTranspParam, ISipUserData* pUserData) :
        m_pSipMsgIn(pSipMsg),
        m_pTranspParam(pTranspParam),
        m_pUserData(pUserData),
        m_pSendSipMsg(SIP_NULL),
        m_eTxnStatus(SipTxn::STATUS_INVALID),
        m_pOutUserData(SIP_NULL),
        m_pTranspInfo(SIP_NULL),
        m_bTxnTerminated(SIP_FALSE),
        m_bTxnCreated(SIP_FALSE)
{
}

SipTxnFsmData::~SipTxnFsmData() {}
