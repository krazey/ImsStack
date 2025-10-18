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

#ifndef BYE_TRANSACTION_HANDLER_H_
#define BYE_TRANSACTION_HANDLER_H_

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "helper/sipinterfaceholder/IInterfaceHolderListener.h"
#include <functional>

class IByeTransactionHandlerListener;
class ISession;

class ByeTransactionHandler final : public IInterfaceHolderListener
{
public:
    ByeTransactionHandler(IN CallKey nCallKey, IN IByeTransactionHandlerListener& objListener,
            IN std::function<void(ISession&)> objOperation);
    ~ByeTransactionHandler() override;

    ByeTransactionHandler(IN const ByeTransactionHandler&) = delete;
    ByeTransactionHandler& operator=(IN const ByeTransactionHandler&) = delete;

    void OnSessionInterfaceReleased(IN CallKey nKey, IN ISession& objSession) override;

private:
    CallKey m_nCallKey;
    IByeTransactionHandlerListener& m_objListener;
    std::function<void(ISession&)> m_objOperation;
};

#endif
