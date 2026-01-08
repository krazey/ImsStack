/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef MOCK_TEXT_SDP_GENERATOR_H_
#define MOCK_TEXT_SDP_GENERATOR_H_

#include <gmock/gmock.h>

#include <text/TextSdpGenerator.h>

class MockTextSdpGenerator : public TextSdpGenerator
{
public:
    MockTextSdpGenerator() {}
    virtual ~MockTextSdpGenerator() override {}
    MOCK_METHOD(IMS_BOOL, Generate,
            (OUT ISessionDescriptor * pSessionDescriptor, OUT IMediaDescriptor* pDescriptor,
                    IN MediaBaseProfile* pBaseProfile),
            (override));
};

#endif
