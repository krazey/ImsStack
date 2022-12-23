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

#include "ImsMessage.h"
#include "ServiceTrace.h"
#include "helper/OperationAsyncRunner.h"
#include <functional>
#include <utility>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
OperationAsyncRunner::OperationAsyncRunner(IN std::function<void()> objOperation) :
        m_objOperation(std::move(objOperation))
{
    IMS_TRACE_D("+OperationAsyncRunner", 0, 0, 0);
    PostMessage(0, 0, 0);
}

PUBLIC VIRTUAL OperationAsyncRunner::~OperationAsyncRunner()
{
    IMS_TRACE_D("~OperationAsyncRunner", 0, 0, 0);

    m_objOperation = {};
}

PUBLIC VIRTUAL IMS_BOOL OperationAsyncRunner::OnMessage(IN ImsMessage& /*objMsg*/)
{
    if (m_objOperation)
    {
        m_objOperation();
    }

    delete this;
    return IMS_TRUE;
}
