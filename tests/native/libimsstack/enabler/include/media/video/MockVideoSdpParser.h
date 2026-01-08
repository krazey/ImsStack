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

#ifndef MOCK_VIDEO_SDP_PARSER_H_
#define MOCK_VIDEO_SDP_PARSER_H_

#include <gmock/gmock.h>

#include <video/VideoSdpParser.h>

class MockVideoSdpParser : public VideoSdpParser
{
public:
    explicit MockVideoSdpParser() {}
    virtual ~MockVideoSdpParser() override {}

    MOCK_METHOD(IMS_BOOL, Parse,
            (IN ISessionDescriptor * pSessionDescriptor, IN IMediaDescriptor* pDescriptor,
                    OUT VideoProfile* pProfile),
            (override));
};

#endif
