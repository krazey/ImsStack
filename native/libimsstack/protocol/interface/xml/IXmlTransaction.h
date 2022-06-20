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
#ifndef INTERFACE_XML_TRANSACTION_H_
#define INTERFACE_XML_TRANSACTION_H_

#include "ImsTypeDef.h"

class IXmlRequest;
class IXmlResponse;
class IXmlTransactionListener;

class IXmlTransaction
{
public:
    /**
     * @brief Returns an XML request for this transaction.
     *
     * @return Pointer to IXmlRequest.
     */
    virtual IXmlRequest* GetRequest() const = 0;

    /**
     * @brief Returns an XML response for this transaction.
     *
     * @return Pointer to IXmlResponse.
     */
    virtual IXmlResponse* GetResponse() const = 0;

    /**
     * @brief Sends a request to parse an XML document.
     *
     * @return If the operation is successfully done, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Send() = 0;

    /**
     * @brief Sets an XML transaction listener.
     *
     * @param piListener an XML transaction listener
     */
    virtual void SetListener(IN IXmlTransactionListener* piListener) = 0;
};

#endif
