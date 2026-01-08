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

#include "ServiceTrace.h"
#include "call/termination/ByeTransactionHandler.h"
#include "call/termination/IByeTransactionHandlerListener.h"
#include <functional>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
ByeTransactionHandler::ByeTransactionHandler(IN CallKey nCallKey,
        IN IByeTransactionHandlerListener& objListener,
        IN std::function<void(ISession&)> objOperation) :
        m_nCallKey(nCallKey),
        m_objListener(objListener),
        m_objOperation(std::move(objOperation))
{
    IMS_TRACE_D("+ByeTransactionHandler - callKey [%lu]", m_nCallKey, 0, 0);
}

PUBLIC ByeTransactionHandler::~ByeTransactionHandler()
{
    IMS_TRACE_D("~ByeTransactionHandler", 0, 0, 0);
    m_objOperation = {};
}

PUBLIC VIRTUAL void ByeTransactionHandler::OnSessionInterfaceReleased(
        IN CallKey nKey, IN ISession& objSession)
{
    if (m_nCallKey != nKey)
    {
        return;
    }

    m_objOperation(objSession);

    m_objListener.OnByeTransactionCompleted(this);
}
