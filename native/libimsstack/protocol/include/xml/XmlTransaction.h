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
#ifndef XML_TRANSACTION_H_
#define XML_TRANSACTION_H_

#include "IXmlTransaction.h"

class DocumentBuilder;
class XmlApp;
class XmlRequest;
class XmlResponse;

class XmlTransaction : public IXmlTransaction
{
public:
    explicit XmlTransaction(IN XmlApp* pXmlApp);
    ~XmlTransaction() override;

    XmlTransaction(IN const XmlTransaction&) = delete;
    XmlTransaction& operator=(IN const XmlTransaction&) = delete;

public:
    IXmlResponse* GetResponse() const override;
    IXmlRequest* GetRequest() const override;
    IMS_RESULT Send() override;
    void SetListener(IN IXmlTransactionListener* piListener) override;

    XmlResponse* CreateResponse();
    void NotifyParsingCompleted();
    void SetDocumentBuilder(IN DocumentBuilder* pDocumentBuilder);

private:
    XmlApp* m_pXmlApp;
    XmlRequest* m_pRequest;
    XmlResponse* m_pResponse;
    IXmlTransactionListener* m_piListener;
    DocumentBuilder* m_pDocumentBuilder;
};

#endif
