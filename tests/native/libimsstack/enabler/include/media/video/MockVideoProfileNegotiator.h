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

#ifndef MOCK_VIDEO_PROFILE_NEGOTIATOR_H_
#define MOCK_VIDEO_PROFILE_NEGOTIATOR_H_

#include <gmock/gmock.h>

#include <video/VideoProfileNegotiator.h>

class MockVideoProfileNegotiator : public VideoProfileNegotiator
{
public:
    MockVideoProfileNegotiator() {}
    ~MockVideoProfileNegotiator() override {}
    MOCK_METHOD(IMS_BOOL, Negotiate,
            (IN VideoProfile * pLocalProfile, IN VideoProfile* pPeerProfile,
                    IN IMS_BOOL bIsOfferReceived, OUT VideoProfile* pNegotiatedProfile,
                    IN MediaConfiguration* pConfig),
            (override));
    MOCK_METHOD(VIDEO_RESOLUTION, GetNegotiatedResolution,
            (IN MediaBaseProfile::BasePayload * pPayload), (override));
    MOCK_METHOD(MEDIA_DIRECTION, UpdateDirectionToMine,
            (IN MEDIA_DIRECTION ePeerDirection, IN MEDIA_DIRECTION eLocalDirection,
                    IN IMS_BOOL bIsMtCase),
            (override));
};

#endif
