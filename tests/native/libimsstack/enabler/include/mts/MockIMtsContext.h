/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_I_MTS_CONTEXT_H_
#define MOCK_I_MTS_CONTEXT_H_

#include "IMtsContext.h"
#include "MtsDef.h"
#include <gmock/gmock.h>

class IJniMtsAppThread;
class IMtsDynamicLoader;
class IMtsMessageController;
class IMtsNetworkTracker;
class IMtsService;

class MockIMtsContext : public IMtsContext
{
public:
    virtual ~MockIMtsContext() override {}

    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(
            const IMtsService&, GetService, (IN MtsServiceType eServiceType), (const, override));
    MOCK_METHOD(IMtsMessageController&, GetMessageController, (), (override));
    MOCK_METHOD(const IMtsNetworkTracker&, GetNetworkTracker, (), (const, override));
    MOCK_METHOD(const IMtsDynamicLoader&, GetDynamicLoader, (), (const, override));
    MOCK_METHOD(IJniMtsAppThread*, GetJniAppThread, (), (const, override));
};

#endif
