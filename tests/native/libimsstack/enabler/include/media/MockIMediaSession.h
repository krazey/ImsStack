/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef MOCK_I_MEDIA_SESSION_H_
#define MOCK_I_MEDIA_SESSION_H_

#include <gmock/gmock.h>

#include "IMediaSession.h"
#include "MediaDef.h"

class MockIMediaSession : public IMediaSession
{
public:
    MockIMediaSession() {}
    virtual ~MockIMediaSession() override {}
    MOCK_METHOD(
            void, SetMtcListener, (IN IMediaSessionClientListener * pISessionListener), (override));
    MOCK_METHOD(IMS_UINTP, CreateProfile, (IN IMS_UINTP nNegoID, IN MEDIA_CONTENT_TYPE eMediaType),
            (override));
    MOCK_METHOD(IMS_BOOL, DestroyProfile, (IN IMS_UINTP nNegoID), (override));
    MOCK_METHOD(IMS_BOOL, FormSdp,
            (IN IMS_UINTP nNegoId, OUT ISession* pSession, IN MEDIA_CONTENT_TYPE eMediaType,
                    IN MEDIA_DIRECTION eAudioDirection, IN MEDIA_DIRECTION eVideoDirection,
                    IN MEDIA_DIRECTION eTextDirection, IN IMS_BOOL bEnforceReofferMode),
            (override));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetSupportedMediaTypesFromSdp,
            (IN IMS_UINTP nNegoID, IN ISession* pSession), (override));
    MOCK_METHOD(SdpNegotiationResult, NegotiateSdp, (IN IMS_UINTP nNegoId, IN ISession* pSession),
            (override));
    MOCK_METHOD(IMS_BOOL, IsQosAvailable, (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType),
            (override));
    MOCK_METHOD(void, FinalizeSdp, (IN IMS_UINTP nNegoID, IN ISession* pSession), (override));
    MOCK_METHOD(IMS_BOOL, Run, (IN IMS_UINTP nNegoID), (override));
    MOCK_METHOD(IMS_BOOL, Terminate, (), (override));
    MOCK_METHOD(NEGO_STATE, GetNegoState, (IN IMS_UINTP nNegoID), (override));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetNegotiatedMediaType, (IN IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedQuality,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedCodecBitrate,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type), (override));
    MOCK_METHOD(IMS_FLOAT, GetNegotiatedCodecBitrateKbps, (IN IMS_UINTP), (override));
    MOCK_METHOD(IMS_FLOAT, GetNegotiatedCodecBandwidthKhz, (IN IMS_UINTP), (override));
    MOCK_METHOD(void, GetNegotiatedCodecBitrateRange,
            (IN IMS_UINTP, OUT IMS_FLOAT&, OUT IMS_FLOAT&), (override));
    MOCK_METHOD(void, GetNegotiatedCodecBandwidthRange,
            (IN IMS_UINTP, OUT IMS_FLOAT&, OUT IMS_FLOAT&), (override));
    MOCK_METHOD(IMS_SINT32, GetRemotePort, (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type),
            (override));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedDirection,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type), (override));
    MOCK_METHOD(void, SetOptions,
            (IN IMS_UINTP nNegoId, IN OptionType type, IN IMS_SINT32 param1, IN IMS_SINT32 param2),
            (override));
    MOCK_METHOD(void, SetNetworkToneRtpTimer,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer),
            (override));
    MOCK_METHOD(IMS_BOOL, NotifySrvccStatus, (IN MEDIA_SRVCC_STATUS nStatus), (override));
    MOCK_METHOD(IMS_BOOL, SendMessage, (IN IMS_SINT32 nMsg, IN IMS_UINTP pParam), (override));
    MOCK_METHOD(
            void, SetMediaPemType, (IN IMS_UINTP nNegoId, IN MEDIA_PEM_TYPE ePemType), (override));
    MOCK_METHOD(IMS_BOOL, IsPreviewMode, (IMS_UINTP nNegoId), (override));
};
#endif
