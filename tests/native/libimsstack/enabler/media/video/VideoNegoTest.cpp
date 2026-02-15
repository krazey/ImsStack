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

#include "video/VideoNego.h"
#include "ImsStrLib.h"
#include "ImsTypeDef.h"
#include "MediaEnvironment.h"
#include "MockICarrierConfig.h"
#include "MockICoreService.h"
#include "MockISessionDescriptor.h"
#include "MockMediaProfileFactory.h"
#include "MockMediaProfileGenerator.h"
#include "core/media/MockIMediaDescriptor.h"
#include "video/MockVideoProfileNegotiator.h"
#include "video/MockVideoSdpGenerator.h"
#include "video/MockVideoSdpParser.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::ReturnRoundRobin;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_SINT32 HEVC_PAYLOAD = 115;
const IMS_SINT32 AVC_PAYLOAD = 104;
const IMS_SINT32 LOCAL_PORT = 10000;

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class VideoNegoTest : public ::testing::Test
{
public:
    std::unique_ptr<VideoNego> m_pVideoNego;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;
    std::unique_ptr<VideoConfiguration> m_pConfig;
    std::unique_ptr<MockICarrierConfig> m_pMockICarrierConfig;
    std::unique_ptr<MockICarrierConfig> m_pVideoBundle;
    std::unique_ptr<MockICarrierConfig> m_pHevcBundle;
    std::unique_ptr<MockICarrierConfig> m_pAvcBundle;
    std::unique_ptr<MockICarrierConfig> m_pHevcSubBundle;
    std::unique_ptr<MockICarrierConfig> m_pAvcSubBundle;
    std::unique_ptr<MockICoreService> m_pICoreService;
    std::shared_ptr<MockMediaProfileGenerator> m_pMockProfileGenerator;
    std::shared_ptr<MockVideoSdpParser> m_pMockVideoSdpParser;
    std::shared_ptr<MockVideoSdpGenerator> m_pMockVideoSdpGenerator;
    std::shared_ptr<MockVideoProfileNegotiator> m_pMockProfileNegotiator;

    std::shared_ptr<VideoProfile> m_pBaseProfile;
    IpAddress m_objIpAddr;
    ImsVector<IMS_SINT32> m_objHevcPayloadType;
    ImsVector<IMS_SINT32> m_objAvcPayloadType;
    AString m_strHevcPayloadTypeNumber;
    AString m_strAvcPayloadTypeNumber;
    MockMediaProfileFactory m_objMediaProfileFactory;

protected:
    virtual void SetUp() override
    {
        MockMediaProfileFactory::SetInstance(&m_objMediaProfileFactory);

        CreateEnvironment();

        PrepareVideoConfig();
        PrepareHevcConfig();
        PrepareAvcConfig();

        CreateVideoConfig();
        CreateNegoProfile();
    }

    void CreateVideoConfig() { m_pConfig->Create(m_pMockICarrierConfig.get()); }

    void CreateNegoProfile()
    {
        m_pVideoNego = std::make_unique<VideoNego>(DEFAULT_SLOT_ID);
        m_pMockVideoSdpGenerator = std::make_shared<MockVideoSdpGenerator>();
        m_pMockVideoSdpParser = std::make_shared<MockVideoSdpParser>();
        m_pMockProfileNegotiator = std::make_shared<MockVideoProfileNegotiator>();
        m_pMockProfileGenerator = std::make_shared<MockMediaProfileGenerator>();

        m_pVideoNego->SetSdpGenerator(m_pMockVideoSdpGenerator);
        m_pVideoNego->SetSdpParser(m_pMockVideoSdpParser);
        m_pVideoNego->SetProfileNegotiator(m_pMockProfileNegotiator);
        m_pVideoNego->SetProfileGenerator(m_pMockProfileGenerator);

        m_pBaseProfile = std::make_shared<VideoProfile>();
        VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
        pAvcPayload->SetRtpMap(AVC_PAYLOAD, "H264", 90000, 1);
        auto pAvcFmtp = std::make_shared<VideoProfile::AvcFmtp>();
        pAvcFmtp->SetProfile(AVC_PROFILE_B);
        pAvcFmtp->SetLevel(12);
        pAvcPayload->SetFmtp(pAvcFmtp);

        m_pBaseProfile->AddPayload(pAvcPayload);
        m_pBaseProfile->SetDataPort(LOCAL_PORT);

        ON_CALL(*m_pMockProfileGenerator, Generate(_, _, _, _))
                .WillByDefault(Return(m_pBaseProfile));
        m_pVideoNego->CreateProfiles(m_pEnvironment, m_pConfig.get());
    }

    void CreateEnvironment()
    {
        m_pEnvironment = std::make_shared<MediaEnvironment>();
        m_pICoreService = std::make_unique<MockICoreService>();

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pICoreService.get(), GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));

        m_pEnvironment->eNetworkType = MEDIA_NETWORK_LTE;
        m_pEnvironment->eServiceType = MEDIA_SERVICE_DEFAULT;
        m_pEnvironment->pIService = m_pICoreService.get();
    }

    void PrepareVideoConfig()
    {
        m_pMockICarrierConfig = std::make_unique<MockICarrierConfig>();
        m_pVideoBundle = std::make_unique<MockICarrierConfig>();

        m_pConfig = std::make_unique<VideoConfiguration>(MEDIA_TYPE_VIDEO);

        ON_CALL(*m_pMockICarrierConfig.get(),
                GetBundle(CarrierConfig::ImsVt::KEY_VIDEO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pVideoBundle.get()));
    }

    void PrepareHevcConfig()
    {
        m_objHevcPayloadType.Add(HEVC_PAYLOAD);
        ON_CALL(*m_pVideoBundle.get(),
                GetIntArray(CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_TYPE_INT_ARRAY, _))
                .WillByDefault(Return(m_objHevcPayloadType));

        m_pHevcBundle = std::make_unique<MockICarrierConfig>();
        ON_CALL(*m_pMockICarrierConfig.get(),
                GetBundle(CarrierConfig::ImsVt::KEY_HEVC_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pHevcBundle.get()));

        m_strHevcPayloadTypeNumber.SetNumber(HEVC_PAYLOAD);
        m_pHevcSubBundle = std::make_unique<MockICarrierConfig>();

        ON_CALL(*m_pHevcBundle.get(), GetBundle(IsSameKey(m_strHevcPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pHevcSubBundle.get()));
    }

    void PrepareAvcConfig()
    {
        m_objAvcPayloadType.Add(AVC_PAYLOAD);
        ON_CALL(*m_pVideoBundle.get(),
                GetIntArray(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_TYPE_INT_ARRAY, _))
                .WillByDefault(Return(m_objAvcPayloadType));

        m_pAvcBundle = std::make_unique<MockICarrierConfig>();
        ON_CALL(*m_pMockICarrierConfig.get(),
                GetBundle(CarrierConfig::ImsVt::KEY_H264_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pAvcBundle.get()));

        m_strAvcPayloadTypeNumber.SetNumber(AVC_PAYLOAD);
        m_pAvcSubBundle = std::make_unique<MockICarrierConfig>();

        ON_CALL(*m_pAvcBundle.get(), GetBundle(IsSameKey(m_strAvcPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pAvcSubBundle.get()));
    }

    virtual void TearDown() override
    {
        m_objHevcPayloadType.Clear();
        m_objAvcPayloadType.Clear();

        MockMediaProfileFactory::SetInstance(IMS_NULL);
    }
};

TEST_F(VideoNegoTest, testIsMediaCodecFromSdpSupported)
{
    MockIMediaDescriptor objVideoDescriptor;
    MockISessionDescriptor objSessionDescriptor;

    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(
            m_pVideoNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objVideoDescriptor),
            IMS_FALSE);

    auto pProfile = std::make_shared<VideoProfile>();
    VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
    pAvcPayload->SetRtpMap(99, "H264", 90000, 1);

    auto pAvcFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pAvcFmtp->SetProfile(AVC_PROFILE_B);
    pAvcFmtp->SetLevel(12);
    pAvcPayload->SetFmtp(pAvcFmtp);

    pProfile->AddPayload(pAvcPayload);
    pProfile->SetDataPort(LOCAL_PORT);

    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, IMS_NULL)).WillByDefault(Return(pProfile));
    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, testing::NotNull()))
            .WillByDefault(Return(IMS_NULL));

    EXPECT_EQ(
            m_pVideoNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objVideoDescriptor),
            IMS_TRUE);

    MockMediaProfileFactory::SetInstance(IMS_NULL);
}

TEST_F(VideoNegoTest, testFormSdpNullArguments)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    EXPECT_FALSE(m_pVideoNego->FormSdp(
            STATE_IDLE, IMS_NULL, &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_IDLE, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_NEGOTIATED, IMS_NULL, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_OFFER_RECEIVED, IMS_NULL, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoNegoTest, testFormSdpOfferIdle)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pVideoNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoNegoTest, testFormSdpOfferNegotiated)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pVideoNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoNegoTest, testFormSdpOfferOfferReceivedFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    // fail case: calling without negotiation
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoNegoTest, testFormSdpReoffer)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pVideoNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_TRUE));
}

TEST_F(VideoNegoTest, testFormSdpInvalid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(
            STATE_IDLE, IMS_NULL, &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_IDLE, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_INVALID, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoNegoTest, testFormSdpOfferIdleGenerateFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(m_pVideoNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoNegoTest, testNegotiateSdpIdleParseFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_FALSE));

    m_pVideoNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(VideoNegoTest, testNegotiateSdpInvalidArguments)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    m_pVideoNego->NegotiateSdp(STATE_IDLE, IMS_NULL, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pVideoNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, IMS_NULL, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pVideoNego->NegotiateSdp(STATE_NEGOTIATED, IMS_NULL, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pVideoNego->NegotiateSdp(STATE_NEGOTIATED, &objSessionDescriptor, IMS_NULL, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pVideoNego->NegotiateSdp(STATE_OFFER_SENT, IMS_NULL, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pVideoNego->NegotiateSdp(STATE_OFFER_SENT, &objSessionDescriptor, IMS_NULL, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(VideoNegoTest, testNegotiateSdpIdleNegotiateFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_FALSE));

    m_pVideoNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(VideoNegoTest, testNegotiateSdpIdleSuccessAndFormSdpOfferReceived)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    // setup the valid OA model
    auto pLocalProfile = std::make_shared<VideoProfile>();
    VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
    pAvcPayload->SetRtpMap(99, "H264", 90000, 1);

    auto pAvcFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pAvcFmtp->SetProfile(AVC_PROFILE_B);
    pAvcFmtp->SetLevel(12);
    pAvcPayload->SetFmtp(pAvcFmtp);

    pLocalProfile->AddPayload(pAvcPayload);
    pLocalProfile->SetDataPort(LOCAL_PORT);
    pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND);

    auto pPeerProfile = std::make_shared<VideoProfile>(*pLocalProfile);
    auto pNegoProfile = std::make_shared<VideoProfile>(*pLocalProfile);

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _))
            .WillByDefault(ReturnRoundRobin({pLocalProfile, pPeerProfile, pNegoProfile}));

    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    m_pVideoNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_SEND);

    ON_CALL(*m_pMockVideoSdpGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pVideoNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(VideoNegoTest, testNegotiateSdpOfferSentFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;
    m_pVideoNego->NegotiateSdp(
            STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(VideoNegoTest, testNegotiateSdpOfferSentSuccess)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    // setup the valid OA model
    auto pLocalProfile = std::make_shared<VideoProfile>();
    VideoProfile::Payload* pAvcPayload = new VideoProfile::Payload();
    pAvcPayload->SetRtpMap(99, "H264", 90000, 1);

    auto pAvcFmtp = std::make_shared<VideoProfile::AvcFmtp>();
    pAvcFmtp->SetProfile(AVC_PROFILE_B);
    pAvcFmtp->SetLevel(12);
    pAvcPayload->SetFmtp(pAvcFmtp);

    pLocalProfile->AddPayload(pAvcPayload);
    pLocalProfile->SetDataPort(LOCAL_PORT);
    pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND);

    auto pPeerProfile = std::make_shared<VideoProfile>(*pLocalProfile);
    auto pNegoProfile = std::make_shared<VideoProfile>(*pLocalProfile);

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _))
            .WillByDefault(ReturnRoundRobin({pLocalProfile, pPeerProfile, pNegoProfile}));

    // form offer in the idle state
    ON_CALL(*m_pMockVideoSdpGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pVideoNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));

    MEDIA_DIRECTION eDirection;
    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    m_pVideoNego->NegotiateSdp(
            STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_SEND);
}

TEST_F(VideoNegoTest, testNegotiateSdpOfferReceivedFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    m_pVideoNego->NegotiateSdp(
            STATE_OFFER_RECEIVED, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(VideoNegoTest, testCleanupIncompleteOaModels)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    m_pVideoNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(m_pVideoNego->GetOaModelList().GetSize(), 1);
    m_pVideoNego->CleanupIncompleteOaModels();
    EXPECT_EQ(m_pVideoNego->GetOaModelList().GetSize(), 0);
}

TEST_F(VideoNegoTest, testGetNegotiatedPayloadValid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pVideoNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_NE(m_pVideoNego->GetNegotiatedPayload(), IMS_NULL);
}

TEST_F(VideoNegoTest, testGetNegotiatedRtpPortValid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockVideoSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pVideoNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_NE(m_pVideoNego->GetNegotiatedRtpPort(), -1);
}

TEST_F(VideoNegoTest, testSetPort)
{
    EXPECT_TRUE(m_pVideoNego->SetLocalPort(LOCAL_PORT));
    EXPECT_TRUE(m_pVideoNego->SetLocalPort(0));
}

TEST_F(VideoNegoTest, testGetters)
{
    EXPECT_EQ(m_pVideoNego->GetNegotiatedResolution(), VIDEO_RESOLUTION_INVALID);
    EXPECT_EQ(m_pVideoNego->GetLocalPort(), LOCAL_PORT);
    EXPECT_EQ(m_pVideoNego->GetNegotiatedBandwidth(), -1);
    EXPECT_EQ(m_pVideoNego->GetNegotiatedPayload(), IMS_NULL);
    EXPECT_EQ(m_pVideoNego->GetNegotiatedDirection(), MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pVideoNego->GetNegotiatedRemoteAddress(), IpAddress::NONE);
    EXPECT_EQ(m_pVideoNego->GetNegotiatedLocalProfile(), IMS_NULL);
    EXPECT_EQ(m_pVideoNego->GetNegotiatedNegoProfile(), IMS_NULL);
    EXPECT_EQ(m_pVideoNego->GetNegotiatedPeerProfile(), IMS_NULL);
    EXPECT_EQ(m_pVideoNego->GetRemotePort(), MEDIA_PORT_INVALID);
}
