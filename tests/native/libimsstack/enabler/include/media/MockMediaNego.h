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

#ifndef MOCK_MEDIA_NEGO_H_
#define MOCK_MEDIA_NEGO_H_

#include <gmock/gmock.h>

#include <MediaNego.h>

class MockMediaNego : public MediaNego
{
public:
    explicit MockMediaNego(IMS_SINT32 nSlotId) :
            MediaNego(nSlotId) {};
    MOCK_METHOD(IMS_BOOL, CreateProfile, (IN std::shared_ptr<MediaEnvironment> pEnvironment),
            (override));
    MOCK_METHOD(IMS_BOOL, Forking, (IN MediaNego * pMediaNego), (override));
    MOCK_METHOD(IMS_BOOL, FormSdp,
            (OUT ISession * pSession, IN MEDIA_CONTENT_TYPE eMediaType,
                    IN IMS_SINT32 nAudioDirection, IN IMS_SINT32 nVideoDirection,
                    IN IMS_SINT32 nTextDirection, IN IMS_BOOL bEnforceReofferMode),
            (override));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetSupportedMediaTypesFromSdp, (IN ISession * pSession),
            (override));
    MOCK_METHOD(IMS_BOOL, NegotiateSdp,
            (IN ISession * pSession, OUT IMS_SINT32& nAudioDirection,
                    OUT IMS_SINT32& nVideoDirection, OUT IMS_SINT32& nTextDirection,
                    OUT MediaNegoResult& errorReason),
            (override));
    MOCK_METHOD(void, FinalizeSdp, (IN ISession * pSession), (override));
    MOCK_METHOD(void, ConfirmSession, (), (override));
    MOCK_METHOD(void, SetNegoState, (NEGO_STATE eNegoState), (override));
    MOCK_METHOD(NEGO_STATE, GetNegoState, (), (override));
    MOCK_METHOD(std::shared_ptr<AudioNego>, GetAudioNego, (), (override));
    MOCK_METHOD(std::shared_ptr<VideoNego>, GetVideoNego, (), (override));
    MOCK_METHOD(std::shared_ptr<TextNego>, GetTextNego, (), (override));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetSessionType, (), (override));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedAudioDirection, (), (override));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedVideoDirection, (), (override));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedTextDirection, (), (override));
    MOCK_METHOD(AUDIO_CODEC, GetNegotiatedAudioQuality, (), (override));
    MOCK_METHOD(VIDEO_RESOLUTION, GetNegotiatedVideoQuality, (), (override));
    MOCK_METHOD(TEXT_CODEC, GetNegotiatedTextQuality, (), (override));
    MOCK_METHOD(IMediaDescriptor*, GetMediaDescriptor, (IN IMedia * pIMedia), (override));
    MOCK_METHOD(IMS_BOOL, IsForking, (), (override));
    MOCK_METHOD(void, SetPreviewMode, (IMS_BOOL bIsPreview), (override));
};

#endif
