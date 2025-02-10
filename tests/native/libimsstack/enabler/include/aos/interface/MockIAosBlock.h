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

#ifndef MOCK_I_AOS_BLOCK_H_
#define MOCK_I_AOS_BLOCK_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "interface/IAosBlock.h"

class MockIAosBlock : public IAosBlock {
public:
    MockIAosBlock()
    {
        ON_CALL(*this, GetBlockReasons)
                .WillByDefault(
                        [this](OUT ImsList<IMS_UINT32>& objReasons, IN SERVICE_TYPE eType)
                        {
                            GetBlockReasonsInternal(&objReasons, eType);
                        });
    }

    MOCK_METHOD(void, SetListener, (IN IAosBlockListener* piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosBlockListener* piListener), (override));
    MOCK_METHOD(void, SetSilentListener, (IN IAosBlockSilentListener * piListener), (override));
    MOCK_METHOD(void, RemoveSilentListener, (IN IAosBlockSilentListener * piListener), (override));
    MOCK_METHOD(IMS_BOOL, SetBlockReason, (IN BLOCK_REASON eReason, IN IMS_BOOL bNotify),
            (override));
    MOCK_METHOD(IMS_BOOL, ResetBlockReason, (IN BLOCK_REASON eReason, IN IMS_BOOL bNotify),
            (override));
    MOCK_METHOD(void, ClearAllBlockReasons, (), (override));
    MOCK_METHOD(IMS_BOOL, PrintBlockReasons, (), (override));
    MOCK_METHOD(void, GetBlockReasons,
            (OUT ImsList<IMS_UINT32> & objReasons, IN SERVICE_TYPE eType), (override));
    MOCK_METHOD(IMS_BOOL, IsReasonBlocked, (IN BLOCK_REASON eReason, IN IMS_BOOL bOnlyEnabled,
            IN SERVICE_TYPE eType), (override));
    MOCK_METHOD(IMS_BOOL, IsCleared, (IN SERVICE_TYPE eType), (override));

    // Add mock method that can set OUT parameter
    MOCK_METHOD(void, GetBlockReasonsInternal,
            (OUT ImsList<IMS_UINT32> * objReasons, IN SERVICE_TYPE eType));
};

#endif // MOCK_I_AOS_BLOCK_H_
