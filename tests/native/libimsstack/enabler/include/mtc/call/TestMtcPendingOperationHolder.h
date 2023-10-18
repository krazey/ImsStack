/*
 * Copyright (C) 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TEST_MTC_PENDING_OPERATION_HOLDER_H_
#define TEST_MTC_PENDING_OPERATION_HOLDER_H_

#include "ImsTypeDef.h"
#include "call/MtcPendingOperationHolder.h"
#include "call/state/MockIMtcCallState.h"
#include <functional>

class TestMtcPendingOperationHolder : public MtcPendingOperationHolder
{
public:
    virtual ~TestMtcPendingOperationHolder() {}

    virtual void PushPendingOperation(
            IN const std::function<IMtcCall::State(IMtcCallState*)>& objPendingOperation)
    {
        objPendingOperation(&objState);
    }

    MockIMtcCallState& GetMock() { return objState; }

private:
    MockIMtcCallState objState;
};

#endif
