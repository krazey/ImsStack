/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtaa copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_I_MTC_MEDIA_MANAGER_H_
#define MOCK_I_MTC_MEDIA_MANAGER_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "media/IMtcMediaManager.h"
#include "media/MediaNego.h"
class IMediaQosEventListener;
class IMediaReportEventListener;
class IMessage;
class ISession;
class JniMediaSessionThread;
class MediaInfo;
enum class CallType;
enum class MediaState;
enum class PemType;

class MockIMtcMediaManager : public IMtcMediaManager
{
public:
    ~MockIMtcMediaManager() {}
    MOCK_METHOD(void, SetMediaReportEventListener, (IN IMediaReportEventListener * pListener),
            (override));
    MOCK_METHOD(void, SetQosListener, (IN IMediaQosEventListener * pListener), (override));
    MOCK_METHOD(void, SetMediaInfo, (IN const MediaInfo& objInfo), (override));
    MOCK_METHOD(
            void, UpdateMediaDirection, (IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir), (override));
    MOCK_METHOD(void, UpdateMediaQuality, (IN IMS_UINT32 eMediaType, IN IMS_SINT32 eQuality),
            (override));
    MOCK_METHOD(void, GetMediaInfo, (OUT MediaInfo & objInfo), (override));
    MOCK_METHOD(void, GetOldMediaInfo, (OUT MediaInfo & objInfo), (override));
    MOCK_METHOD(void, RestoreMediaInfo, (), (override));
    MOCK_METHOD(void, CreateMediaSession, (IN JniMediaSessionThread * pJniMediaThread), (override));
    MOCK_METHOD(void, DestroyMediaSession, (), (override));
    MOCK_METHOD(void, CreateMediaProfile,
            (IN ISession * piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOriginalProfile),
            (override));
    MOCK_METHOD(void, DestroyMediaProfile, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_BOOL, IsLocalTone, (), (override));
    MOCK_METHOD(MediaState, GetState, (), (override));
    MOCK_METHOD(MediaState, GetOldState, (), (override));
    MOCK_METHOD(IMS_RESULT, FormSdp, (IN ISession * piSession, IN CallType eCallType), (override));
    MOCK_METHOD(NegotiationResult, NegotiateSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(void, RestoreSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(void, UpdatePemType, (IN ISession * piSession, IN IMessage* piMessage), (override));
    MOCK_METHOD(void, Run, (IN ISession * piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly),
            (override));
    MOCK_METHOD(void, Terminate, (), (override));
    MOCK_METHOD(void, SetRtpPort,
            (IN ISession * piSession, IN IMS_UINT32 eMediaTypes, IN IMS_UINT32 nPort), (override));
    MOCK_METHOD(void, RequestVideoDataUsage, (), (override));
    MOCK_METHOD(void, SetEnforcedDirection, (IN IMS_UINT32 eMediaTypes, IN IMS_SINT32 eDir),
            (override));
    MOCK_METHOD(IMS_BOOL, GetCvoResult, (IN ISession * piSession), (override));
    MOCK_METHOD(void, SendFastVideoUpdate, (), (override));
    MOCK_METHOD(void, SetConferenceCall, (IN IMS_BOOL bConference), (override));
    MOCK_METHOD(void, SetConfirmedSession, (IN ISession * piSession), (override));
    MOCK_METHOD(void, UpdateStatsReportOption, (IN IMS_UINT32 eAction), (override));
    MOCK_METHOD(NegotiationState, GetNegotiationState, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedDirection,
            (IN ISession * piSession, IN IMS_UINT32 eMediaType), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedQuality,
            (IN ISession * piSession, IN IMS_UINT32 eMediaType), (override));
    MOCK_METHOD(CallType, GetNegotiatedCallType, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_BOOL, IsAudioQualityHd, (), (override));
    MOCK_METHOD(PemType, GetPemType, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_BOOL, IsAudioMediaActivated, (), (override));
};

#endif
