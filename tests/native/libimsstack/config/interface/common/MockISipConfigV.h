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
#ifndef MOCK_I_SIP_CONFIG_V_H_
#define MOCK_I_SIP_CONFIG_V_H_

#include <gmock/gmock.h>

#include "ISipConfigV.h"

class MockISipConfigV : public ISipConfigV
{
public:
    inline MockISipConfigV() {}
    ~MockISipConfigV() override = default;

    MOCK_METHOD(IConfigurable*, GetConfigurable, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetFeatureTagOptions, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetTargetNumberFormat, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetTargetScheme, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetTimerValue, (IN IMS_SINT32 nType), (const, override));
};

#endif
