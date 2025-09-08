/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef MOCK_I_CONFIG_UPDATE_LISTENER_H_
#define MOCK_I_CONFIG_UPDATE_LISTENER_H_

#include <gmock/gmock.h>

#include "IConfigUpdateListener.h"

class MockIConfigUpdateListener : public IConfigUpdateListener
{
public:
    inline MockIConfigUpdateListener() {}
    ~MockIConfigUpdateListener() override = default;

    MOCK_METHOD(void, ConfigUpdate_NotifyUpdate,
            (IN IMS_SINT32 nCpi, IN const AString& strConfName, IN const AString& strExtraParam),
            (override));
};

#endif