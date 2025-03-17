/**
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
    MOCK_METHOD(void, CreateProfile, (IN std::shared_ptr<MediaEnvironment> pEnvironment), (const));
    MOCK_METHOD(IMS_BOOL, Forking, (IN MediaNego * pMediaNego), (const));
    MOCK_METHOD(IMS_BOOL, FormSdp,
            (OUT ISession * pSession, IN MEDIA_CONTENT_TYPE eMediaType,
                    IN IMS_SINT32 nAudioDirection, IN IMS_SINT32 nVideoDirection,
                    IN IMS_SINT32 nTextDirection, IN IMS_BOOL bEnforceReofferMode),
            (const));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetSupportedMediaTypesFromSdp, (IN ISession * pSession),
            (override));
    MOCK_METHOD(IMS_BOOL, NegotiateSdp,
            (IN ISession * pSession, OUT IMS_SINT32& nAudioDirection,
                    OUT IMS_SINT32& nVideoDirection, OUT IMS_SINT32& nTextDirection,
                    OUT MediaNegoResult& errorReason),
            (const));
    MOCK_METHOD(void, FinalizeSdp, (IN ISession * pSession), (const));

    MOCK_METHOD(void, SetNegoState, (NEGO_STATE eNegoState), (const));
    MOCK_METHOD(NEGO_STATE, GetNegoState, (), (const));

    MOCK_METHOD(void, SetAudioNego, (AudioNego * pAudioNego), (const));
    MOCK_METHOD(std::shared_ptr<AudioNego>, GetAudioNego, (), (const));
    MOCK_METHOD(void, SetVideoNego, (VideoNego * pVideoNego), (const));
    MOCK_METHOD(std::shared_ptr<VideoNego>, GetVideoNego, (), (const));
    MOCK_METHOD(void, SetTextNego, (TextNego * pTextNego), (const));
    MOCK_METHOD(std::shared_ptr<TextNego>, GetTextNego, (), (const));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetSessionType, (), (const));

    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedAudioDirection, (), (const));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedVideoDirection, (), (const));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedTextDirection, (), (const));
    MOCK_METHOD(AUDIO_CODEC, GetNegotiatedAudioQuality, (), (const));
    MOCK_METHOD(VIDEO_RESOLUTION, GetNegotiatedVideoQuality, (), (const));
    MOCK_METHOD(TEXT_CODEC, GetNegotiatedTextQuality, (), (const));

    MOCK_METHOD(IMediaDescriptor*, GetMediaDescriptor, (IN IMedia * pIMedia), (const));
    MOCK_METHOD(IMS_BOOL, IsForking, (), (const));
};

#endif
