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
#ifndef MOCK_I_REASON_HEADER_SETTER_H_
#define MOCK_I_REASON_HEADER_SETTER_H_

#include <gmock/gmock.h>

#include "IReasonHeaderSetter.h"

class MockIReasonHeaderSetter : public IReasonHeaderSetter
{
public:
    ~MockIReasonHeaderSetter() override = default;

public:
    MOCK_METHOD(void, ReasonHeaderSetter_SetHeader,
            (IN ISipMessage * piSipMsg, IN IMS_SINT32 nTerminationReason), (override));
    MOCK_METHOD(void, ReasonHeaderSetter_SetPrivateHeader,
            (IN ISipMessage * piOldSipMsg, IN_OUT ISipMessage* piNewSipMsg), (override));
};

#endif
