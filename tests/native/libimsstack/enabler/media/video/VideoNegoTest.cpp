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

#include <gtest/gtest.h>

#include "MockICarrierConfig.h"
#include "MockICoreService.h"
#include "MockISessionDescriptor.h"
#include "ServiceConfig.h"
#include "config/MediaSessionConfigFactory.h"
#include "media/MockIMediaDescriptor.h"
#include "offeranswer/SdpAvCodec.h"
#include "video/VideoNego.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_SINT32 HEVC_PAYLOAD = 115;
const IMS_SINT32 AVC_PAYLOAD = 104;

class FakeVideoNego : public VideoNego
{
public:
    explicit FakeVideoNego(IN IMS_UINT32 nSlotId) :
            VideoNego(nSlotId)
    {
    }
    virtual ~FakeVideoNego() {}

protected:
};

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class VideoNegoTest : public ::testing::Test
{
public:
    FakeVideoNego* m_pVideoNego;
    MediaEnvironment* m_pEnvironment;
    VideoConfiguration* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pVideoBundle;
    MockICarrierConfig* m_pHevcBundle;
    MockICarrierConfig* m_pAvcBundle;
    MockICarrierConfig* m_pHevcSubBundle;
    MockICarrierConfig* m_pAvcSubBundle;
    MockICoreService* m_pICoreService;
    IpAddress m_objIpAddr;
    ImsVector<IMS_SINT32> m_objHevcPayloadType;
    ImsVector<IMS_SINT32> m_objAvcPayloadType;
    AString m_strHevcPayloadTypeNumber;
    AString m_strAvcPayloadTypeNumber;

protected:
    virtual void SetUp() override
    {
        CreateEnvironment();

        PrepareVideoConfig();
        PrepareHevcConfig();
        PrepareAvcConfig();

        CreateVideoConfig();
        CreateNegoProfile();
    }

    void CreateVideoConfig() { m_pConfig->Create(m_pMockICarrierConfig); }

    void CreateNegoProfile()
    {
        m_pVideoNego = new FakeVideoNego(DEFAULT_SLOT_ID);
        m_pVideoNego->CreateProfiles(m_pEnvironment, MEDIA_TYPE_VIDEO, m_pConfig);
    }

    void CreateEnvironment()
    {
        m_pEnvironment = new MediaEnvironment();
        m_pICoreService = new MockICoreService();

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pICoreService, GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));

        m_pEnvironment->eNetworkType = MEDIA_NETWORK_LTE;
        m_pEnvironment->eServiceType = MEDIA_SERVICE_DEFAULT;
        m_pEnvironment->pIService = m_pICoreService;
    }

    void PrepareVideoConfig()
    {
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pVideoBundle = new MockICarrierConfig();

        m_pConfig = new VideoConfiguration(MEDIA_TYPE_VIDEO);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pVideoBundle));
    }

    void PrepareHevcConfig()
    {
        m_objHevcPayloadType.Add(HEVC_PAYLOAD);
        ON_CALL(*m_pVideoBundle,
                GetIntArray(CarrierConfig::Assets::KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY))
                .WillByDefault(Return(m_objHevcPayloadType));

        m_pHevcBundle = new MockICarrierConfig();
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::Assets::KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pHevcBundle));

        m_strHevcPayloadTypeNumber.SetNumber(HEVC_PAYLOAD);
        m_pHevcSubBundle = new MockICarrierConfig();

        ON_CALL(*m_pHevcBundle, GetBundle(IsSameKey(m_strHevcPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pHevcSubBundle));
    }

    void PrepareAvcConfig()
    {
        m_objAvcPayloadType.Add(AVC_PAYLOAD);
        ON_CALL(*m_pVideoBundle, GetIntArray(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_TYPE_INT_ARRAY))
                .WillByDefault(Return(m_objAvcPayloadType));

        m_pAvcBundle = new MockICarrierConfig();
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pAvcBundle));

        m_strAvcPayloadTypeNumber.SetNumber(AVC_PAYLOAD);
        m_pAvcSubBundle = new MockICarrierConfig();

        ON_CALL(*m_pAvcBundle, GetBundle(IsSameKey(m_strAvcPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pAvcSubBundle));
    }

    virtual void TearDown() override
    {
        delete m_pVideoNego;
        delete m_pEnvironment;
        delete m_pICoreService;
        delete m_pMockICarrierConfig;
        delete m_pVideoBundle;
        delete m_pConfig;
        delete m_pHevcBundle;
        delete m_pHevcSubBundle;
        delete m_pAvcBundle;
        delete m_pAvcSubBundle;

        m_objHevcPayloadType.Clear();
        m_objAvcPayloadType.Clear();
    }
};

TEST_F(VideoNegoTest, testIsMediaCodecFromSdpSupported)
{
    MockIMediaDescriptor objVideoDescriptor;
    MockISessionDescriptor objSessionDescriptor;
    ImsList<SdpMediaFormat*> lstMediaFormat;

    SdpAvCodec objAvCodec;
    objAvCodec.SetValue("115");
    objAvCodec.SetParameters("115 H265/90000", "115 profile-id=1; level-id=93");
    lstMediaFormat.Append(&objAvCodec);

    ON_CALL(objVideoDescriptor, GetRemotePort()).WillByDefault(Return(50010));
    ON_CALL(objVideoDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormat));

    EXPECT_EQ(
            m_pVideoNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objVideoDescriptor),
            IMS_TRUE);

    ON_CALL(objVideoDescriptor, GetRemotePort()).WillByDefault(Return(0));

    EXPECT_EQ(
            m_pVideoNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objVideoDescriptor),
            IMS_FALSE);

    ON_CALL(objVideoDescriptor, GetRemotePort()).WillByDefault(Return(50010));

    lstMediaFormat.Clear();
    objAvCodec.SetValue("104");
    objAvCodec.SetParameters("104 H264/90000", "104 profile-level-id=42E00C;packetization-mode=1;");
    lstMediaFormat.Append(&objAvCodec);

    EXPECT_EQ(
            m_pVideoNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objVideoDescriptor),
            IMS_TRUE);

    lstMediaFormat.Clear();
    objAvCodec.SetValue("25");
    objAvCodec.SetParameters("25 CELB/90000", "25 CIF=2;QCIF=2;SQCIF=2;CUSTOM=360,240,2");
    lstMediaFormat.Append(&objAvCodec);

    EXPECT_EQ(
            m_pVideoNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objVideoDescriptor),
            IMS_FALSE);
}
