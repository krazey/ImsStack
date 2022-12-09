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
#ifndef INTERFACE_XML_TRANSACTION_PROVIDER_H_
#define INTERFACE_XML_TRANSACTION_PROVIDER_H_

#include "IXmlStateListener.h"
#include "IXmlTransaction.h"

/**
 * @brief This class provides an interface for parsing XML document asynchronously.
 *
 * @see IXmlStateListener, IXmlTransaction
 */
class IXmlTransactionProvider
{
protected:
    virtual ~IXmlTransactionProvider() = default;

public:
    /**
     * @brief Creates a transaction to parse an XML document.
     */
    virtual IXmlTransaction* CreateTransaction() = 0;

    /**
     * @brief Destroys the specified transaction.
     */
    virtual void DestroyTransaction(IN IXmlTransaction*& piTransaction) = 0;

    /**
     * @brief Pushes an XML transaction to be processed.
     *
     * @param piTransaction an XML transaction
     * @return If the transaction is successfully pushed, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Push(IN IXmlTransaction* piTransaction) = 0;

    /**
     * @brief Pops a topmost XML transaction.
     *
     * @return Pointer to IXmlTransaction.
     */
    virtual IXmlTransaction* Pop() = 0;

    /**
     * @brief Returns a state of XML transaction provider.
     *
     * @return The current state of this IXmlTransactionProvider.\n
     *         #STATE_IDLE\n
     *         #STATE_ATTACHING\n
     *         #STATE_ATTACHED\n
     *         #STATE_DETACHED\n
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Sets an XML state listener.
     *
     * @param piListener XML state listener
     */
    virtual void SetStateListener(IN IXmlStateListener* piListener) = 0;

public:
    enum
    {
        STATE_IDLE = 0,
        STATE_ATTACHING,
        STATE_ATTACHED,
        STATE_DETACHED
    };
};

#endif
