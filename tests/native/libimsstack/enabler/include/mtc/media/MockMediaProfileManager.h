/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_MTC_MEDIA_PROFILE_MANAGER_H_
#define MOCK_MTC_MEDIA_PROFILE_MANAGER_H_

#include "ImsTypeDef.h"
#include "media/MtcMediaProfileManager.h"
#include <gmock/gmock.h>

class IMediaSession;
class ISession;
enum class PemType;

class MockMtcMediaProfileManager : public MtcMediaProfileManager
{
public:
    ~MockMtcMediaProfileManager() override {}
    MOCK_METHOD(void, CreateMediaProfile,
            (IN const ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOriginalProfile,
                    IN MEDIA_CONTENT_TYPE eMediaContents, IN IMediaSession* piMediaSession),
            (override));
    MOCK_METHOD(void, DestroyMediaProfile,
            (IN const ISession* piSession, IN IMediaSession* piMediaSession), (override));
    MOCK_METHOD(void, DestroyAllMediaProfiles, (IN IMediaSession* piMediaSession), (override));
    MOCK_METHOD(IMS_UINTP, GetNegoId, (IN const ISession* piSession), (const, override));
    MOCK_METHOD(PemType, GetPemType, (IN const ISession* piSession), (const, override));
    MOCK_METHOD(IMS_BOOL, IsActive, (IN const ISession* piSession), (const, override));
    MOCK_METHOD(IMS_BOOL, IsConfirmed, (IN const ISession* piSession), (const, override));
    MOCK_METHOD(IMS_BOOL, IsForked, (IN const ISession* piSession), (const, override));
    MOCK_METHOD(void, SetPemType, (IN const ISession* piSession, IN PemType ePemType), (override));
    MOCK_METHOD(void, SetActive, (IN const ISession* piSession, IN IMS_BOOL bActive), (override));
    MOCK_METHOD(
            void, SetConfirmed, (IN const ISession* piSession, IN IMS_BOOL bConfirmed), (override));
    MOCK_METHOD(IMS_BOOL, IsPemSendInOtherEarlySession, (IN const ISession* piSession),
            (const, override));
    MOCK_METHOD(void, UpdateProfileForMediaActivation, (IN const ISession* piActiveSession),
            (override));
    MOCK_METHOD(ISession*, GetSessionWithNegoId, (IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(ISession*, GetActiveSession, (), (const, override));
};

#endif
