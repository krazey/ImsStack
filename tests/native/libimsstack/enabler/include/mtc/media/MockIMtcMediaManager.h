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

#ifndef MOCK_I_MTC_MEDIA_MANAGER_H_
#define MOCK_I_MTC_MEDIA_MANAGER_H_

#include "ImsTypeDef.h"
#include "helper/ISrvccStateListener.h"
#include "media/IMtcMediaManager.h"
#include "media/MediaNego.h"
#include <gmock/gmock.h>

class IMediaQosEventListener;
class IMediaReportEventListener;
class IMessage;
class ISession;
enum class CallType;
enum class PemType;
struct MediaInfo;

class MockIMtcMediaManager : public IMtcMediaManager
{
public:
    MOCK_METHOD(void, SetMediaReportEventListener, (IN IMediaReportEventListener* pListener),
            (override));
    MOCK_METHOD(void, SetQosListener, (IN IMediaQosEventListener* pListener), (override));
    MOCK_METHOD(void, SetMediaInfo, (IN const ISession& objISession, IN const MediaInfo& objInfo),
            (override));
    MOCK_METHOD(void, UpdateMediaDirection,
            (IN const ISession& objISession, IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir),
            (override));
    MOCK_METHOD(
            const MediaInfo&, GetMediaInfo, (IN const ISession& objISession), (const, override));
    MOCK_METHOD(void, RestoreMediaInfo, (IN const ISession& objISession), (override));
    MOCK_METHOD(void, CreateMediaSession, (), (override));
    MOCK_METHOD(void, DestroyMediaSession, (), (override));
    MOCK_METHOD(void, CreateMediaProfile,
            (IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOriginalProfile),
            (override));
    MOCK_METHOD(void, DestroyMediaForSession, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_BOOL, IsLocalTone, (), (override));
    MOCK_METHOD(IMS_RESULT, FormSdp,
            (IN ISession* piSession, IN CallType eCallType,
                    IN IMS_BOOL bAnswerForOfferlessReInvite),
            (override));
    MOCK_METHOD(NegotiationResult, NegotiateSdp, (IN ISession* piSession), (override));
    MOCK_METHOD(void, RestoreSdp, (IN ISession* piSession), (override));
    MOCK_METHOD(void, FinalizeSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(void, UpdatePemType, (IN ISession* piSession, IN IMessage* piMessage), (override));
    MOCK_METHOD(void, Run, (IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly),
            (override));
    MOCK_METHOD(void, SetRtpPort,
            (IN ISession * piSession, IN IMS_UINT32 eMediaTypes, IN IMS_UINT32 nPort), (override));
    MOCK_METHOD(IMS_SINT32, GetRemoteRtpPort, (IN ISession * piSession, IN IMS_UINT32 eMediaTypes),
            (override));
    MOCK_METHOD(void, SetConferenceCall, (), (override));
    MOCK_METHOD(void, SetConfirmedSession, (IN ISession * piSession), (override));
    MOCK_METHOD(NegotiationState, GetNegotiationState, (IN ISession* piSession), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedDirection,
            (IN const ISession* piSession, IN IMS_UINT32 eMediaType), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedQuality,
            (IN const ISession* piSession, IN IMS_UINT32 eMediaType), (override));
    MOCK_METHOD(CallType, GetNegotiatedCallType, (IN ISession * piSession), (override));
    MOCK_METHOD(PemType, GetPemType, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_BOOL, IsAudioInactive, (), (override));
    MOCK_METHOD(void, AdjustDirectionForAutoOffer,
            (IN const ISession& objISession, IN CallType eCallType), (override));
    MOCK_METHOD(void, AdjustDirectionForAutoAnswer, (IN const ISession& objISession), (override));
    MOCK_METHOD(void, AdjustDirectionForLocalResourceConfirmation,
            (IN const ISession& objISession, IN CallType eCallType), (override));
    MOCK_METHOD(void, SetSrvccState, (IN SrvccState eState), (override));
    MOCK_METHOD(IMS_BOOL, IsOnHold, (IN const ISession& objISession), (override));
    MOCK_METHOD(IMS_UINT32, GetSupportedMediaTypesFromSdp, (IN ISession * piSession), (override));
    MOCK_METHOD(IMS_BOOL, IsPreviewMode, (IN ISession * piSession), (const, override));
    MOCK_METHOD(IMS_BOOL, IsForkedSession, (IN const ISession* piSession), (const, override));
};

#endif
