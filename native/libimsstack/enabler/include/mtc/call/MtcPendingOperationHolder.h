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

#ifndef MTC_PENDING_OPERATION_HOLDER_H_
#define MTC_PENDING_OPERATION_HOLDER_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include <functional>

class IMtcCallContext;
class IMtcCallState;

class MtcPendingOperationHolder
{
public:
    explicit MtcPendingOperationHolder();
    virtual ~MtcPendingOperationHolder();
    MtcPendingOperationHolder(IN const MtcPendingOperationHolder&) = delete;
    MtcPendingOperationHolder& operator=(IN const MtcPendingOperationHolder&) = delete;

    IMS_BOOL HasPendingOperation() const;
    virtual void PushPendingOperation(
            IN std::function<IMtcCall::State(IMtcCallState*)> objPendingOperation);
    std::function<IMtcCall::State(IMtcCallState*)> PopPendingOperation();

private:
    ImsList<std::function<IMtcCall::State(IMtcCallState*)>> m_objPendingOperations;
};

#endif
