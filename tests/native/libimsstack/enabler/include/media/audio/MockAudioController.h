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

#ifndef MOCK_AUDIO_CONTROLLER_H_
#define MOCK_AUDIO_CONTROLLER_H_

#include <gmock/gmock.h>

#include <audio/AudioController.h>

/**
 * @brief Mock class for AudioController using Google Mock.
 */
class MockAudioController : public AudioController
{
public:
    MockAudioController() {}
    virtual ~MockAudioController() {}

    MOCK_METHOD(void, SetCallSessionState, (IN IMS_BOOL bConfirmed), (override));
    MOCK_METHOD(IMS_BOOL, SendDtmf, (IN IMS_CHAR cDtmfCode), (override));
    MOCK_METHOD(IMS_BOOL, CreateSession,
            (IN IMediaSessionListener * pListener, IN IMS_UINTP nNegoId,
                    AudioConfiguration* pConfig, MEDIA_SERVICE_TYPE eServiceType),
            (override));
    MOCK_METHOD(IMS_BOOL, OpenSession, (IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_BOOL, UpdateSession,
            (const IN IMS_UINTP nNegoId, const IN IMS_UINT32 nAccessNetwork,
                    IN std::shared_ptr<AudioNego> pNego),
            (override));
    MOCK_METHOD(IMS_BOOL, AddSession,
            (IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork,
                    IN std::shared_ptr<AudioNego> pNego),
            (override));
    MOCK_METHOD(IMS_BOOL, ConfirmSession, (IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_BOOL, ModifySession, (IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_BOOL, SetMediaQuality, (IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_BOOL, DeleteSession, (IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_BOOL, CloseSession, (), (override));
    MOCK_METHOD(IMS_BOOL, UpdateRtpConfig,
            (IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork,
                    IN std::shared_ptr<AudioNego> pNego),
            (override));
    MOCK_METHOD(IMS_BOOL, UpdateLocalAddress, (IN std::shared_ptr<AudioNego> pNego), (override));
    MOCK_METHOD(IMS_BOOL, UpdateAccessNetwork, (IN IMS_UINT32 accessNetwork), (override));
    MOCK_METHOD(IMS_BOOL, UpdateQualityThreshold,
            (IN IMS_UINTP nNegoId, IN std::shared_ptr<AudioNego> pNego), (override));
    MOCK_METHOD(IMS_UINT32, GetAudioSessionSize, (), (override));
    MOCK_METHOD(IMS_BOOL, UpdateMediaDirection,
            (IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bRestore), (override));
    MOCK_METHOD(
            void, SetNetworkToneTimer, (IN IMS_UINTP nNegoId, IN IMS_UINT32 nTimer), (override));
    MOCK_METHOD(IMS_SINT32, GetInactivityTimer,
            (IN InactivitytimerType eType, IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_BOOL, UpdateAnbrEnabledConfig, (IN IMS_UINTP nNegoId, IN IMS_BOOL bAnbrEnabled),
            (override));
    MOCK_METHOD(IMS_BOOL, NotifyAnbrReceived,
            (IN IMS_UINT32 nAnbrMediaType, IN IMS_UINT32 nAnbrDirection,
                    IN IMS_UINT32 nAnbrBitrate),
            (override));
    MOCK_METHOD(IMS_BOOL, IsAudioConfigChanged, (IN AudioConfig * pAudioConfig), (override));
    MOCK_METHOD(IMS_BOOL, IsSessionOpened, (), (override));
    MOCK_METHOD(
            void, SetMediaPemType, (IN IMS_UINTP nNegoId, IN MEDIA_PEM_TYPE ePemType), (override));
};

#endif
