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

#ifndef OPERATION_ASYNC_RUNNER_H_
#define OPERATION_ASYNC_RUNNER_H_

#include "ImsActivityEx.h"
#include "ImsTypeDef.h"
#include <functional>

class ImsMessage;

class OperationAsyncRunner : public ImsActivityEx
{
public:
    explicit OperationAsyncRunner(IN std::function<void()> objOperation);

private:
    ~OperationAsyncRunner();

public:
    OperationAsyncRunner(IN const OperationAsyncRunner&) = delete;
    OperationAsyncRunner& operator=(IN const OperationAsyncRunner&) = delete;

    IMS_BOOL OnMessage(IN ImsMessage& objMsg) override;

private:
    std::function<void()> m_objOperation;
};

#endif
