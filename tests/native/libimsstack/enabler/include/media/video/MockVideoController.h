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

#ifndef MOCK_VIDEO_CONTROLLER_H_
#define MOCK_VIDEO_CONTROLLER_H_

#include <gmock/gmock.h>

#include "video/VideoController.h"

/**
 * @brief Mock class for TextController using Google Mock.
 */
class MockVideoController : public VideoController
{
public:
    MockVideoController() {}
    virtual ~MockVideoController() {}

    MOCK_METHOD(void, SetCallSessionState, (IN IMS_BOOL bConfirmed), (override));
    MOCK_METHOD(IMS_BOOL, SendMessage, (IN IMS_SINT32 nMsg, IN IMS_UINTP pParam), (override));
    MOCK_METHOD(IMS_BOOL, CreateSession,
            (IMediaSessionListener * pListener, VideoConfiguration* pConfig), (override));
    MOCK_METHOD(IMS_BOOL, OpenSession, (), (override));
    MOCK_METHOD(IMS_BOOL, UpdateSession, (), (override));
    MOCK_METHOD(IMS_BOOL, CloseSession, (), (override));
    MOCK_METHOD(IMS_BOOL, UpdateLocalAddress, (IN std::shared_ptr<VideoNego> pNego), (override));
    MOCK_METHOD(IMS_BOOL, UpdateRtpConfig, (IN std::shared_ptr<VideoNego> pNego, IMS_BOOL bHold),
            (override));
    MOCK_METHOD(void, UpdateAccessNetwork, (IN IMS_UINT32 nAccessNetwork), (override));
    MOCK_METHOD(void, SetMtu, (IN IMS_SINT32 nMtu), (override));
    MOCK_METHOD(
            IMS_BOOL, UpdateQualityThreshold, (IN std::shared_ptr<VideoNego> pNego), (override));
    MOCK_METHOD(IMS_BOOL, IsSessionOpened, (), (override));
};

#endif  // MOCK_VIDEO_CONTROLLER_H_
