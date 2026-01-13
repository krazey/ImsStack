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

#include "text/TextNego.h"
#include "MediaDef.h"
#include "MediaEnvironment.h"

#include "MockICarrierConfig.h"
#include "MockICoreService.h"
#include "MockISessionDescriptor.h"
#include "MockMediaProfileFactory.h"
#include "MockMediaProfileGenerator.h"
#include "core/media/MockIMediaDescriptor.h"
#include "media/MockIMediaDescriptor.h"
#include "text/MockTextProfileNegotiator.h"
#include "text/MockTextSdpGenerator.h"
#include "text/MockTextSdpParser.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::ReturnRoundRobin;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_SINT32 LOCAL_PORT = 10000;
const IMS_SINT32 T140_PAYLOAD = 112;
const IMS_SINT32 RED_PAYLOAD = 111;

class TextNegoTest : public ::testing::Test
{
public:
    std::unique_ptr<TextNego> m_pTextNego;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;
    std::unique_ptr<TextConfiguration> m_pConfig;
    std::unique_ptr<MockICarrierConfig> m_pMockICarrierConfig;
    std::unique_ptr<MockICarrierConfig> m_pTextBundle;
    std::unique_ptr<MockICoreService> m_pICoreService;
    IpAddress m_objIpAddr;
    std::shared_ptr<MockMediaProfileGenerator> m_pMockProfileGenerator;
    std::shared_ptr<MockTextSdpParser> m_pMockTextSdpParser;
    std::shared_ptr<MockTextSdpGenerator> m_pMockTextSdpGenerator;
    std::shared_ptr<MockTextProfileNegotiator> m_pMockProfileNegotiator;
    std::shared_ptr<TextProfile> m_pBaseProfile;
    MockMediaProfileFactory m_objMediaProfileFactory;

protected:
    virtual void SetUp() override
    {
        MockMediaProfileFactory::SetInstance(&m_objMediaProfileFactory);

        CreateEnvironment();
        PrepareTextConfig();

        CreateTextConfig();
        CreateNegoProfile();
    }

    void CreateTextConfig() { m_pConfig->Create(m_pMockICarrierConfig.get()); }

    void CreateNegoProfile()
    {
        m_pTextNego = std::make_unique<TextNego>(DEFAULT_SLOT_ID);
        m_pMockTextSdpGenerator = std::make_shared<MockTextSdpGenerator>();
        m_pMockTextSdpParser = std::make_shared<MockTextSdpParser>();
        m_pMockProfileNegotiator = std::make_shared<MockTextProfileNegotiator>();
        m_pMockProfileGenerator = std::make_shared<MockMediaProfileGenerator>();

        m_pTextNego->SetSdpGenerator(m_pMockTextSdpGenerator);
        m_pTextNego->SetSdpParser(m_pMockTextSdpParser);
        m_pTextNego->SetProfileNegotiator(m_pMockProfileNegotiator);
        m_pTextNego->SetProfileGenerator(m_pMockProfileGenerator);

        m_pBaseProfile = std::make_shared<TextProfile>();
        AddT140Payload(m_pBaseProfile, 99);
        m_pBaseProfile->SetDataPort(LOCAL_PORT);

        ON_CALL(*m_pMockProfileGenerator, Generate(_, _, _, _))
                .WillByDefault(Return(m_pBaseProfile));
        m_pTextNego->CreateProfiles(m_pEnvironment, m_pConfig.get());
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

    void PrepareTextConfig()
    {
        m_pMockICarrierConfig = std::make_unique<MockICarrierConfig>();
        m_pTextBundle = std::make_unique<MockICarrierConfig>();

        m_pConfig = std::make_unique<TextConfiguration>(MEDIA_TYPE_AUDIOTEXT);

        ON_CALL(*m_pMockICarrierConfig.get(),
                GetBundle(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pTextBundle.get()));

        ON_CALL(*m_pTextBundle.get(), GetInt(CarrierConfig::ImsRtt::KEY_T140_PAYLOAD_TYPE_INT, -1))
                .WillByDefault(Return(T140_PAYLOAD));
        ON_CALL(*m_pTextBundle.get(), GetInt(CarrierConfig::ImsRtt::KEY_RED_PAYLOAD_TYPE_INT, -1))
                .WillByDefault(Return(RED_PAYLOAD));
    }

    void AddT140Payload(std::shared_ptr<TextProfile> pProfile, IMS_SINT32 nPayloadNum)
    {
        TextProfile::Payload* pT140 = new TextProfile::Payload();
        pT140->SetRtpMap(nPayloadNum, "t140", 1000);
        pT140->SetFmtp(std::make_shared<TextProfile::T140Fmtp>());
        pProfile->AddPayload(pT140);
    }

    virtual void TearDown() override { MockMediaProfileFactory::SetInstance(IMS_NULL); }
};

TEST_F(TextNegoTest, testIsMediaCodecFromSdpSupportedNullArgs)
{
    MockIMediaDescriptor objTextDescriptor;
    MockISessionDescriptor objSessionDescriptor;

    EXPECT_FALSE(m_pTextNego->IsMediaCodecFromSdpSupported(IMS_NULL, &objTextDescriptor));
    EXPECT_FALSE(m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, IMS_NULL));

    // Test with null base profile by having the generator return null
    ON_CALL(*m_pMockProfileGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_NULL));
    m_pTextNego->CreateProfiles(m_pEnvironment, m_pConfig.get());

    EXPECT_FALSE(
            m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objTextDescriptor));
}

TEST_F(TextNegoTest, testIsMediaCodecFromSdpSupported)
{
    MockIMediaDescriptor objTextDescriptor;
    MockISessionDescriptor objSessionDescriptor;

    // Test parse fail
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objTextDescriptor),
            IMS_FALSE);

    // Test negotiate fail
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(
            m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objTextDescriptor));

    TextProfile::Payload* pT140 = new TextProfile::Payload();
    pT140->SetRtpMap(99, "t140", 1000);
    auto pT140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    pT140->SetFmtp(pT140Fmtp);

    std::shared_ptr<TextProfile> pProfile = std::make_shared<TextProfile>();
    pProfile->AddPayload(pT140);
    pProfile->SetDataPort(LOCAL_PORT);

    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, IMS_NULL)).WillByDefault(Return(pProfile));
    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, testing::NotNull()))
            .WillByDefault(Return(IMS_NULL));

    EXPECT_EQ(m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objTextDescriptor),
            IMS_TRUE);
}

TEST_F(TextNegoTest, testFormSdpNullArguments)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    EXPECT_FALSE(m_pTextNego->FormSdp(
            STATE_IDLE, IMS_NULL, &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_NEGOTIATED, IMS_NULL, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_OFFER_RECEIVED, IMS_NULL, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testFormSdpOfferIdle)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testFormSdpOfferNegotiated)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testFormSdpOfferOfferReceivedFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    // fail case: calling without negotiation
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testFormSdpReoffer)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));

    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_TRUE));
}

TEST_F(TextNegoTest, testFormReofferWithDirection)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    // Negotiate first to have a previous state
    MEDIA_DIRECTION eDirection;
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);

    // Expect Generate to be called, and capture the profile to verify direction
    EXPECT_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _))
            .WillOnce(testing::Invoke(
                    [&](ISessionDescriptor*, IMediaDescriptor*, MediaBaseProfile* pProfile)
                    {
                        auto* pTextProfile = static_cast<TextProfile*>(pProfile);
                        // Check if the reoffer uses the base profile as its foundation
                        EXPECT_EQ(pTextProfile->GetDirection(), MEDIA_DIRECTION_SEND);
                        return IMS_TRUE;
                    }));

    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_TRUE));
}

TEST_F(TextNegoTest, testFormReofferDisabled)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    // Negotiate first
    MEDIA_DIRECTION eDirection;
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);

    // Expect Generate to be called, and capture the profile to verify disabled state
    EXPECT_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _))
            .WillOnce(testing::Invoke(
                    [&](ISessionDescriptor*, IMediaDescriptor*, MediaBaseProfile* pProfile)
                    {
                        auto* pTextProfile = static_cast<TextProfile*>(pProfile);
                        EXPECT_EQ(pTextProfile->GetDataPort(), 0);
                        EXPECT_EQ(pTextProfile->GetControlPort(), 0);
                        EXPECT_EQ(pTextProfile->GetBandwidthAs(), 0);
                        EXPECT_EQ(pTextProfile->GetBandwidthRs(), 0);
                        EXPECT_EQ(pTextProfile->GetBandwidthRr(), 0);
                        return IMS_TRUE;
                    }));

    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_INACTIVE, IMS_TRUE, IMS_TRUE));
}

TEST_F(TextNegoTest, testFormReofferEnforceReofferMode)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    // Negotiate first
    MEDIA_DIRECTION eDirection;
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);

    // Expect Generate to be called. The key check is that it uses the base profile.
    // We can verify this by checking if the port is the original local port.
    EXPECT_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _))
            .WillOnce(testing::Invoke(
                    [&](ISessionDescriptor*, IMediaDescriptor*, MediaBaseProfile* pProfile)
                    {
                        EXPECT_EQ(pProfile->GetDataPort(), m_pBaseProfile->GetDataPort());
                        EXPECT_TRUE(pProfile->ComparePayloadList(m_pBaseProfile->GetPayloadList()));
                        EXPECT_EQ(pProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
                        return IMS_TRUE;
                    }));

    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND_RECEIVE, IMS_FALSE, IMS_TRUE));
}

TEST_F(TextNegoTest, testFormReofferInvalidDirection)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    // No negotiation needed as it should fail on argument check
    // bDisable is false, so invalid direction should cause failure
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_INVALID, IMS_FALSE, IMS_TRUE));
}

TEST_F(TextNegoTest, testFormReofferUpgradeFromDisabled)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    // 1. First negotiation results in a disabled profile (port 0)
    MEDIA_DIRECTION eDirection;
    auto pPeerProfile = std::make_shared<TextProfile>();
    pPeerProfile->SetDataPort(0);  // Peer has port 0
    auto pNegoProfile = std::make_shared<TextProfile>();
    pNegoProfile->SetDataPort(0);  // Negotiated port will be 0

    EXPECT_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _))
            .WillOnce(testing::DoAll(testing::SetArgPointee<3>(*pNegoProfile), Return(IMS_TRUE)));
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);

    // 2. Now, form a re-offer to re-enable the stream (bDisable = false)
    EXPECT_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _))
            .WillOnce(testing::Invoke(
                    [&](ISessionDescriptor*, IMediaDescriptor*, MediaBaseProfile* pProfile)
                    {
                        // Check that the new offer is based on the original base profile, not the
                        // disabled one.
                        EXPECT_EQ(pProfile->GetDataPort(), m_pBaseProfile->GetDataPort());
                        EXPECT_TRUE(pProfile->ComparePayloadList(m_pBaseProfile->GetPayloadList()));
                        EXPECT_EQ(pProfile->GetDirection(), MEDIA_DIRECTION_SEND_RECEIVE);
                        return IMS_TRUE;
                    }));

    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND_RECEIVE, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testFormSdpInvalid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(
            STATE_IDLE, IMS_NULL, &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, IMS_NULL,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_INVALID, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testFormAnswerInvalid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    // No OaModel in the list
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));

    // Add an invalid OaModel
    m_pTextNego->GetOaModelList().Append(std::make_shared<BaseNego::OaModel>());
    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testFormReofferInvalid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    // Test with null base profile by having the generator return null
    ON_CALL(*m_pMockProfileGenerator, Generate(_, _, _, _)).WillByDefault(Return(IMS_NULL));

    m_pTextNego->CreateProfiles(m_pEnvironment, m_pConfig.get());

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_TRUE));
}

TEST_F(TextNegoTest, testFormSdpOfferIdleGenerateFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_FALSE));

    EXPECT_FALSE(m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(TextNegoTest, testNegotiateSdpIdleParseFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_FALSE));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(TextNegoTest, testNegotiateSdpInvalidArguments)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    m_pTextNego->NegotiateSdp(STATE_IDLE, IMS_NULL, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, IMS_NULL, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pTextNego->NegotiateSdp(STATE_NEGOTIATED, IMS_NULL, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pTextNego->NegotiateSdp(STATE_NEGOTIATED, &objSessionDescriptor, IMS_NULL, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pTextNego->NegotiateSdp(STATE_OFFER_SENT, IMS_NULL, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pTextNego->NegotiateSdp(STATE_OFFER_SENT, &objSessionDescriptor, IMS_NULL, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(TextNegoTest, testNegotiateSdpIdleNegotiateFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_FALSE));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(TextNegoTest, testNegotiateSdpAnswerParseFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    // Form offer first to have an OaModel
    m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND_RECEIVE, IMS_FALSE, IMS_FALSE);

    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_FALSE));

    m_pTextNego->NegotiateSdp(
            STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(TextNegoTest, testNegotiateSdpIdleSuccessAndFormSdpOfferReceived)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    // setup the valid OA model
    TextProfile::Payload* pT140 = new TextProfile::Payload();
    pT140->SetRtpMap(99, "t140", 1000);
    auto pT140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    pT140->SetFmtp(pT140Fmtp);

    auto pLocalProfile = std::make_shared<TextProfile>();
    pLocalProfile->AddPayload(pT140);
    pLocalProfile->SetDataPort(LOCAL_PORT);
    pLocalProfile->SetControlPort(LOCAL_PORT + 1);
    pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    auto pPeerProfile = std::make_shared<TextProfile>(*pLocalProfile);
    pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND);
    auto pNegoProfile = std::make_shared<TextProfile>(*pLocalProfile);
    pNegoProfile->SetDirection(MEDIA_DIRECTION_RECEIVE);

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _))
            .WillByDefault(ReturnRoundRobin({pLocalProfile, pPeerProfile, pNegoProfile}));

    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_RECEIVE);

    ON_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_RECEIVE, IMS_FALSE, IMS_FALSE));

    EXPECT_EQ(pNegoProfile->GetDirection(), MEDIA_DIRECTION_RECEIVE);
    EXPECT_NE(pNegoProfile->GetDataPort(), 0);
    EXPECT_NE(pNegoProfile->GetControlPort(), 0);
}

TEST_F(TextNegoTest, testNegotiateSdpIdleSuccessAndFormSdpOfferReject)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    // setup the valid OA model
    TextProfile::Payload* pT140 = new TextProfile::Payload();
    pT140->SetRtpMap(99, "t140", 1000);
    auto pT140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    pT140->SetFmtp(pT140Fmtp);

    auto pLocalProfile = std::make_shared<TextProfile>();
    pLocalProfile->AddPayload(pT140);
    pLocalProfile->SetDataPort(LOCAL_PORT);
    pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

    auto pPeerProfile = std::make_shared<TextProfile>(*pLocalProfile);
    pPeerProfile->SetDirection(MEDIA_DIRECTION_SEND);
    auto pNegoProfile = std::make_shared<TextProfile>(*pLocalProfile);
    pNegoProfile->SetDirection(MEDIA_DIRECTION_RECEIVE);

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _))
            .WillByDefault(ReturnRoundRobin({pLocalProfile, pPeerProfile, pNegoProfile}));

    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_RECEIVE);

    ON_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_INVALID, IMS_TRUE, IMS_FALSE));

    EXPECT_EQ(pNegoProfile->GetDirection(), MEDIA_DIRECTION_RECEIVE);
    EXPECT_EQ(pNegoProfile->GetDataPort(), 0);
    EXPECT_EQ(pNegoProfile->GetControlPort(), 0);
}

TEST_F(TextNegoTest, testNegotiateSdpOfferSentFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    m_pTextNego->NegotiateSdp(
            STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(TextNegoTest, testNegotiateSdpOfferSentSuccess)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    // setup the valid OA model
    TextProfile::Payload* pT140 = new TextProfile::Payload();
    pT140->SetRtpMap(99, "t140", 1000);
    auto pT140Fmtp = std::make_shared<TextProfile::T140Fmtp>();
    pT140->SetFmtp(pT140Fmtp);

    auto pLocalProfile = std::make_shared<TextProfile>();
    pLocalProfile->AddPayload(pT140);
    pLocalProfile->SetDataPort(LOCAL_PORT);
    pLocalProfile->SetDirection(MEDIA_DIRECTION_SEND);

    auto pPeerProfile = std::make_shared<TextProfile>(*pLocalProfile);
    auto pNegoProfile = std::make_shared<TextProfile>(*pLocalProfile);

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _))
            .WillByDefault(ReturnRoundRobin({pLocalProfile, pPeerProfile, pNegoProfile}));

    // form offer in the idle state
    ON_CALL(*m_pMockTextSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));

    MEDIA_DIRECTION eDirection;
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    m_pTextNego->NegotiateSdp(
            STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_SEND);
}

TEST_F(TextNegoTest, testNegotiateSdpOfferReceivedFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    m_pTextNego->NegotiateSdp(
            STATE_OFFER_RECEIVED, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(TextNegoTest, testCleanupIncompleteOaModels)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));

    m_pTextNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(m_pTextNego->GetOaModelList().GetSize(), 1);
    m_pTextNego->CleanupIncompleteOaModels();
    EXPECT_EQ(m_pTextNego->GetOaModelList().GetSize(), 0);
}

TEST_F(TextNegoTest, testGetNegotiatedPayloadValid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_NE(m_pTextNego->GetNegotiatedPayload(), IMS_NULL);
}

TEST_F(TextNegoTest, testGetNegotiatedRtpPortValid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(m_pBaseProfile));
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_NE(m_pTextNego->GetNegotiatedRtpPort(), -1);
}

TEST_F(TextNegoTest, testGetNegotiatedCodec)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    // Setup for T140 negotiation
    auto pNegoProfileT140 = std::make_shared<TextProfile>();
    TextProfile::Payload* pT140Payload = new TextProfile::Payload();
    pT140Payload->SetRtpMap(T140_PAYLOAD, "t140", 1000);
    pNegoProfileT140->AddPayload(pT140Payload);

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(pNegoProfileT140));
    ON_CALL(*m_pMockTextSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);

    EXPECT_EQ(m_pTextNego->GetNegotiatedCodec(), TEXT_CODEC_T140);

    // Setup for RED negotiation
    auto pNegoProfileRed = std::make_shared<TextProfile>();
    TextProfile::Payload* pRedPayload = new TextProfile::Payload();
    pRedPayload->SetRtpMap(RED_PAYLOAD, "red", 1000);
    pNegoProfileRed->AddPayload(pRedPayload);

    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(pNegoProfileRed));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);

    EXPECT_EQ(m_pTextNego->GetNegotiatedCodec(), TEXT_CODEC_T140_RED);

    // Setup for unsupported codec
    auto pNegoProfileOther = std::make_shared<TextProfile>();
    TextProfile::Payload* pOtherPayload = new TextProfile::Payload();
    pOtherPayload->SetRtpMap(120, "other", 1000);
    pNegoProfileOther->AddPayload(pOtherPayload);
    ON_CALL(m_objMediaProfileFactory, CreateProfile(_, _)).WillByDefault(Return(pNegoProfileOther));

    m_pTextNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(m_pTextNego->GetNegotiatedCodec(), TEXT_CODEC_NONE);
}

TEST_F(TextNegoTest, testSetPort)
{
    EXPECT_TRUE(m_pTextNego->SetLocalPort(LOCAL_PORT));
    EXPECT_TRUE(m_pTextNego->SetLocalPort(0));
}

TEST_F(TextNegoTest, testGetters)
{
    EXPECT_EQ(m_pTextNego->GetNegotiatedCodec(), TEXT_CODEC_NONE);
    EXPECT_EQ(m_pTextNego->GetLocalPort(), LOCAL_PORT);
    EXPECT_EQ(m_pTextNego->GetNegotiatedBandwidth(), -1);
    EXPECT_EQ(m_pTextNego->GetNegotiatedPayload(), IMS_NULL);
    EXPECT_EQ(m_pTextNego->GetNegotiatedDirection(), MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pTextNego->GetNegotiatedRemoteAddress(), IpAddress::NONE);
    EXPECT_EQ(m_pTextNego->GetNegotiatedLocalProfile(), IMS_NULL);
    EXPECT_EQ(m_pTextNego->GetNegotiatedNegoProfile(), IMS_NULL);
    EXPECT_EQ(m_pTextNego->GetNegotiatedPeerProfile(), IMS_NULL);
    EXPECT_EQ(m_pTextNego->GetNegotiatedRtpPort(), -1);
    EXPECT_EQ(m_pTextNego->GetRemotePort(), MEDIA_PORT_INVALID);
}
