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
#ifndef __INTERFACE_SIP_TXN_LISTENER_H__
#define __INTERFACE_SIP_TXN_LISTENER_H__

#include "SipDatatypes.h"

class ISipUserData;

class ISipTxnListener
{
public:
    ISipTxnListener(){};
    virtual ~ISipTxnListener(){};

    /* read only txn key */
    virtual SIP_BOOL TxnTimeout(ISipUserData* pUserData, SIP_INT32 ms_TimerType) = 0;

    /* read only txn key */
    virtual SIP_BOOL TxnTerminated(ISipUserData* pUserData) = 0;
};
#endif  //__INTERFACE_SIP_TXN_LISTENER_H__
