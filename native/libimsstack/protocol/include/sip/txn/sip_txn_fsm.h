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
#ifndef __SIP_TXN_FSM_H__
#define __SIP_TXN_FSM_H__

#include "txn/sip_txn_common.h"

/* Invite Client FSM */
extern SIP_BOOL (
        *gpfSipInvClientTxnFsm[SipTxn::INV_CLI_INVALID_ST + 1][SipTxn::INV_CLI_INVALID_EVT + 1])(
        SipTxn* pTxnInfo, SIP_VOID* pvData, SIP_UINT16* pnError);

/* Non Invite Client FSM */
extern SIP_BOOL (*gpfSipNonInvClientTxnFsm[SipTxn::NON_INV_CLI_INVALID_ST + 1]
                                          [SipTxn::NON_INV_CLI_INVALID_EVT + 1])(
        SipTxn* pTxnInfo, SIP_VOID* pvData, SIP_UINT16* pnError);

/* Invite Server FSM */
extern SIP_BOOL (
        *gpfSipInvSerTxnFsm[SipTxn::INV_SER_INVALID_ST + 1][SipTxn::INV_SER_INVALID_EVT + 1])(
        SipTxn* pTxnInfo, SIP_VOID* pvData, SIP_UINT16* pnError);

/* Non Invite Server FSM */
extern SIP_BOOL (*gpfSipNonInvSerTxnFsm[SipTxn::NON_INV_SER_INVALID_ST + 1]
                                       [SipTxn::NON_INV_SER_INVALID_EVT + 1])(
        SipTxn* pTxnInfo, SIP_VOID* pvData, SIP_UINT16* pnError);

#endif  //__SIP_TXN_FSM_H__
