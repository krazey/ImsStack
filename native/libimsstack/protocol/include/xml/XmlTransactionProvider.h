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
#ifndef XML_TRANSACTION_PROVIDER_H_
#define XML_TRANSACTION_PROVIDER_H_

#include "ImsActivityEx.h"
#include "ImsQueue.h"
#include "IXmlTransactionProvider.h"

class XmlApp;

class XmlTransactionProvider final : public ImsActivityEx, public IXmlTransactionProvider
{
public:
    XmlTransactionProvider();
    ~XmlTransactionProvider() override;

    XmlTransactionProvider(IN const XmlTransactionProvider&) = delete;
    XmlTransactionProvider& operator=(IN const XmlTransactionProvider&) = delete;

public:
    IXmlTransaction* CreateTransaction() override;
    void DestroyTransaction(IN IXmlTransaction*& piTransaction) override;
    IMS_RESULT Push(IN IXmlTransaction* piTransaction) override;
    IXmlTransaction* Pop() override;
    inline IMS_SINT32 GetState() const override { return m_nState; }
    inline void SetStateListener(IN IXmlStateListener* piListener) override
    {
        m_piListener = piListener;
    }

private:
    IMS_BOOL OnMessage(IN IMSMSG& objMsg) override;
    void Attach();
    void Detach();
    void SetState(IN IMS_SINT32 nState);

private:
    IMS_SINT32 m_nState;
    IXmlStateListener* m_piListener;
    XmlApp* m_pXmlApp;
    ImsQueue<IXmlTransaction*> m_objTransactions;
};

#endif
