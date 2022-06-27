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
#ifndef __SIP_TXN_DB_H__
#define __SIP_TXN_DB_H__

#include "sip_pf_datatypes.h"
#include "msg/sip_comdef.h"
#include "SipHash.h"

class SipTxnDb
{
public:
    SipTxnDb();
    virtual ~SipTxnDb();

    SIP_BOOL AddElement(SIP_VOID* pvElement, SIP_VOID* pvKey, SIP_UINT16* pnError);

    SIP_BOOL FetchElement(SIP_VOID* pvKey, SIP_VOID** ppElement, SIP_UINT16* pnError);

    SIP_BOOL FetchElement(
            SIP_VOID* pvKey, SIP_VOID** ppElement, SIP_VOID** ppKey, SIP_UINT16* pnError);

    SIP_BOOL RemoveElement(SIP_VOID* pvKey, SIP_UINT16* pnError);

private:
    SipHash* m_pTxnHash;

    /**************************************************
      Private Member Functions
     ***************************************************/
    SipTxnDb& operator=(IN const SipTxnDb& objRHS);
    SipTxnDb(IN const SipTxnDb& objRHS);
};

void SipTxnDb_Construct();
void SipTxnDb_Destruct();

SipTxnDb* SipTxnDb_GetInstance();
SIP_CHAR sipTxnCompareHashKey(SIP_VOID* pvStoredKey, SIP_VOID* pvUserKey);

#endif  //__SIP_TXN_DB_H__
