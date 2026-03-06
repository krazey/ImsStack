/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef INTERFACE_BYE_TRANSACTION_HANDLER_LISTENER_H_
#define INTERFACE_BYE_TRANSACTION_HANDLER_LISTENER_H_

class ByeTransactionHandler;

/**
 * @brief Interface for listening to the completion of a BYE transaction.
 *
 * This interface defines a callback to be notified when a BYE transaction, handled by
 * a ByeTransactionHandler, has finished its execution.
 */
class IByeTransactionHandlerListener
{
public:
    virtual ~IByeTransactionHandlerListener() = default;

    /**
     * @brief Notifies that the BYE transaction has completed.
     *
     * @param pHandler The ByeTransactionHandler instance that processed the transaction.
     */
    virtual void OnByeTransactionCompleted(IN ByeTransactionHandler* pHandler) = 0;
};

#endif
