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
#ifndef SIP_STACK_TRANSACTION_H_
#define SIP_STACK_TRANSACTION_H_

#include "ImsTypeDef.h"

class SipStackTransaction
{
public:
    SipStackTransaction();
    SipStackTransaction(IN ::SipTxnKey* pKey, IN SipTxn* pTxn);
    SipStackTransaction(IN const SipStackTransaction& other);
    virtual ~SipStackTransaction();

public:
    SipStackTransaction& operator=(IN const SipStackTransaction& other);

public:
    IMS_BOOL CompareKey(IN ::SipTxnKey* pKey);
    inline SipTxn* GetTxn() const { return m_pTxn; }
    inline ::SipTxnKey* GetKey() const { return m_pKey; }

private:
    ::SipTxnKey* m_pKey;
    SipTxn* m_pTxn;
};

#endif
