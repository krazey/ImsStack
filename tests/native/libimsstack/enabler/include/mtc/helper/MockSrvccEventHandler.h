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

#ifndef MOCK_I_SRVCC_STATE_LISTENER
#define MOCK_I_SRVCC_STATE_LISTENER

#include <gmock/gmock.h>
#include "ImsTypeDef.h"
#include "helper/SrvccEventHandler.h"

class MockSrvccEventHandler : public SrvccEventHandler
{
public:
    virtual ~MockSrvccEventHandler() {}
    MOCK_METHOD(void, AddListener, (IN ISrvccStateListener*), (override));
    MOCK_METHOD(void, RemoveListener, (IN ISrvccStateListener*), (override));
    MOCK_METHOD(void, UpdateSrvccState, (IN SrvccState), (override));
};

#endif
