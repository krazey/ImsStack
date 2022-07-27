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
#ifndef __SIP_TXN_UTIL_H__
#define __SIP_TXN_UTIL_H__

#include "sip_pf_datatypes.h"
#include "msg/sip_comdef.h"
#include "msg/SipMessage.h"

class SipTxnUtil
{
private:
    static SIP_BOOL IsTxnKeyMatched(SipTxnKey* pUserTxnkey, SipTxnKey* pStoredTxnkey);

public:
    static SipTxnKey* SearchTxnKey(SipTxnKey* pTxn, SIP_BOOL bCheckRSeq = SIP_TRUE);
    static SIP_BOOL AddTxnKey(SipTxnKey* pTxnKey);
    static SIP_BOOL DeleteTxnKey(SipTxnKey* pTxnKey, SIP_BOOL bCheckToTag = SIP_FALSE);
};
#endif  //__SIP_TXN_UTIL_H__
