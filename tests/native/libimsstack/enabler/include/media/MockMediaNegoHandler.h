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

#ifndef MOCK_MEDIA_NEGO_HANDLER_H_
#define MOCK_MEDIA_NEGO_HANDLER_H_

#include <gmock/gmock.h>

#include <media/MediaNegoHandler.h>
#include "MediaDef.h"

/**
 * @brief Mock class for MediaNegoHandler using Google Mock.
 */
class MockMediaNegoHandler : public MediaNegoHandler
{
public:
    MockMediaNegoHandler(IMS_SINT32 nSlotId, std::shared_ptr<MediaEnvironment> pEnvironment,
            std::shared_ptr<IMediaNegoFactory> pFactory) :
            MediaNegoHandler(nSlotId, pEnvironment, pFactory)
    {
    }

    virtual ~MockMediaNegoHandler() override = default;

    MOCK_METHOD(IMS_UINTP, CreateMediaNego, (IMS_UINTP nExistingNegoId), (override));
    MOCK_METHOD(std::shared_ptr<MediaNego>, FindMediaNego, (IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_BOOL, DeleteMediaNego, (IMS_UINTP nNegoId), (override));
    MOCK_METHOD(void, ClearAllMediaNego, (), (override));
    MOCK_METHOD(IMS_BOOL, FormSdp,
            (IMS_UINTP nNegoId, OUT ISession* pSession, MEDIA_CONTENT_TYPE eType,
                    MEDIA_DIRECTION eAudioDirection, MEDIA_DIRECTION eVideoDirection,
                    MEDIA_DIRECTION eTextDirection, IMS_BOOL bEnforceReofferMode),
            (override));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetSupportedMediaTypesFromSdp,
            (IMS_UINTP nNegoId, IN ISession* pSession), (override));
    MOCK_METHOD(SdpNegotiationResult, NegotiateSdp, (IMS_UINTP nNegoId, IN ISession* pSession),
            (override));
    MOCK_METHOD(void, FinalizeSdp, (IMS_UINTP nNegoId, IN ISession* pSession), (override));
    MOCK_METHOD(void, FinalizeNegotiation, (IMS_UINTP nNegoId), (override));
    MOCK_METHOD(NEGO_STATE, GetNegoState, (IMS_UINTP nNegoId), (override));
    MOCK_METHOD(MEDIA_CONTENT_TYPE, GetNegotiatedMediaType, (IMS_UINTP nNegoId), (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedQuality, (IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType),
            (override));
    MOCK_METHOD(IMS_SINT32, GetNegotiatedCodecBitrate,
            (IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType), (override));
    MOCK_METHOD(IMS_FLOAT, GetNegotiatedCodecBitrateKbps,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType), (override));
    MOCK_METHOD(IMS_FLOAT, GetNegotiatedCodecBandwidthKhz,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType), (override));
    MOCK_METHOD(void, GetNegotiatedCodecBitrateRange,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE, OUT IMS_FLOAT&, OUT IMS_FLOAT&),
            (override));
    MOCK_METHOD(void, GetNegotiatedCodecBandwidthRange,
            (IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE, OUT IMS_FLOAT&, OUT IMS_FLOAT&),
            (override));
    MOCK_METHOD(
            IMS_SINT32, GetRemotePort, (IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType), (override));
    MOCK_METHOD(MEDIA_DIRECTION, GetNegotiatedDirection,
            (IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType), (override));
    MOCK_METHOD(const IpAddress&, GetNegotiatedRemoteAddress,
            (IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType), (override));
    MOCK_METHOD(IMS_BOOL, SetRtpPort,
            (IMS_UINTP nNegoId, MEDIA_CONTENT_TYPE eType, IMS_UINT32 nPort), (override));
};
#endif  // MOCK_MEDIA_NEGO_HANDLER_H_
