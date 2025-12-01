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

#include "audio/AudioNego.h"
#include "ImsStrLib.h"
#include "MediaEnvironment.h"
#include "MockICarrierConfig.h"
#include "MockICoreService.h"
#include "MockISessionDescriptor.h"
#include "MockMediaProfileFactory.h"
#include "MockMediaProfileGenerator.h"
#include "audio/MockAudioSdpGenerator.h"
#include "audio/MockAudioSdpParser.h"
#include "audio/MockAudioProfileNegotiator.h"
#include "media/MockIMediaDescriptor.h"

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_SINT32 EVS_PAYLOAD = 115;
const IMS_SINT32 AMR_WB_PAYLOAD = 99;
const IMS_SINT32 AMR_NB_PAYLOAD = 97;
const IMS_SINT32 LOCAL_PORT = 10000;

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class AudioNegoTest : public ::testing::Test
{
public:
    std::unique_ptr<AudioNego> m_pAudioNego;
    std::shared_ptr<MediaEnvironment> m_pEnvironment;
    AudioConfiguration* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pAudioBundle;
    MockICarrierConfig* m_pEvsBundle;
    MockICarrierConfig* m_pAmrWbBundle;
    MockICarrierConfig* m_pAmrNbBundle;
    MockICarrierConfig* m_pEvsSubBundle;
    MockICarrierConfig* m_pAmrWbSubBundle;
    MockICarrierConfig* m_pAmrNbSubBundle;
    MockICoreService* m_pICoreService;
    IpAddress m_objIpAddr;
    ImsVector<IMS_SINT32> m_objEvsPayloadType;
    ImsVector<IMS_SINT32> m_objAmrWbPayloadType;
    ImsVector<IMS_SINT32> m_objAmrNbPayloadType;
    AString m_strEvsPayloadTypeNumber;
    AString m_strAmrWbPayloadTypeNumber;
    AString m_strAmrNbPayloadTypeNumber;
    std::shared_ptr<MockAudioSdpGenerator> m_pMockAudioSdpGenerator;
    std::shared_ptr<MockAudioSdpParser> m_pMockAudioSdpParser;
    std::shared_ptr<MockAudioProfileNegotiator> m_pMockProfileNegotiator;
    std::shared_ptr<MockMediaProfileGenerator> m_pMockProfileGenerator;
    std::shared_ptr<AudioProfile> m_pBaseProfile;

protected:
    virtual void SetUp() override
    {
        CreateEnvironment();

        PrepareAudioConfig();
        PrepareEvsConfig();
        PrepareAmrWbConfig();
        PrepareAmrNbConfig();

        CreateAudioConfig();
        CreateNegoProfile();
    }

    void CreateAudioConfig() { m_pConfig->Create(m_pMockICarrierConfig); }

    void CreateNegoProfile()
    {
        m_pAudioNego = std::make_unique<AudioNego>(DEFAULT_SLOT_ID);
        m_pMockAudioSdpGenerator = std::make_shared<MockAudioSdpGenerator>();
        m_pMockAudioSdpParser = std::make_shared<MockAudioSdpParser>();
        m_pMockProfileNegotiator = std::make_shared<MockAudioProfileNegotiator>();
        m_pMockProfileGenerator = std::make_shared<MockMediaProfileGenerator>();

        m_pAudioNego->SetSdpGenerator(m_pMockAudioSdpGenerator);
        m_pAudioNego->SetSdpParser(m_pMockAudioSdpParser);
        m_pAudioNego->SetProfileNegotiator(m_pMockProfileNegotiator);
        m_pAudioNego->SetProfileGenerator(m_pMockProfileGenerator);

        m_pBaseProfile = std::make_shared<AudioProfile>();
        AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
        pAmrPayload->SetRtpMap(99, "AMR-WB", 16000, 1);
        pAmrPayload->SetFmtp(std::make_shared<AudioProfile::AmrFmtp>());
        m_pBaseProfile->AddPayload(pAmrPayload);
        m_pBaseProfile->SetDataPort(LOCAL_PORT);

        ON_CALL(*m_pMockProfileGenerator, Generate(_, _, _, _))
                .WillByDefault(Return(m_pBaseProfile));
        m_pAudioNego->CreateProfiles(m_pEnvironment, m_pConfig);
    }

    void CreateEnvironment()
    {
        m_pEnvironment = std::make_shared<MediaEnvironment>();
        m_pICoreService = new MockICoreService();

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pICoreService, GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));

        m_pEnvironment->eNetworkType = MEDIA_NETWORK_LTE;
        m_pEnvironment->eServiceType = MEDIA_SERVICE_DEFAULT;
        m_pEnvironment->pIService = m_pICoreService;
    }

    void PrepareAudioConfig()
    {
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pAudioBundle = new MockICarrierConfig();

        m_pConfig = new AudioConfiguration(MEDIA_TYPE_AUDIO);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_AUDIO_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pAudioBundle));
    }

    void PrepareEvsConfig()
    {
        ON_CALL(*m_pMockICarrierConfig,
                GetBoolean(CarrierConfig::ImsVoice::KEY_AUDIO_EVS_SUPPORT_BOOL,
                        AudioConfiguration::DEFAULT_SUPPORT_EVS))
                .WillByDefault(Return(IMS_TRUE));

        m_objEvsPayloadType.Add(EVS_PAYLOAD);
        ON_CALL(*m_pAudioBundle,
                GetIntArray(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_TYPE_INT_ARRAY, _))
                .WillByDefault(Return(m_objEvsPayloadType));

        m_pEvsBundle = new MockICarrierConfig();
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pEvsBundle));

        m_strEvsPayloadTypeNumber.SetNumber(EVS_PAYLOAD);
        m_pEvsSubBundle = new MockICarrierConfig();

        ON_CALL(*m_pEvsBundle, GetBundle(IsSameKey(m_strEvsPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pEvsSubBundle));
    }

    void PrepareAmrWbConfig()
    {
        m_objAmrWbPayloadType.Add(AMR_WB_PAYLOAD);
        ON_CALL(*m_pAudioBundle,
                GetIntArray(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY, _))
                .WillByDefault(Return(m_objAmrWbPayloadType));

        m_pAmrWbBundle = new MockICarrierConfig();
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pAmrWbBundle));

        m_strAmrWbPayloadTypeNumber.SetNumber(AMR_WB_PAYLOAD);
        m_pAmrWbSubBundle = new MockICarrierConfig();

        ON_CALL(*m_pAmrWbBundle, GetBundle(IsSameKey(m_strAmrWbPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pAmrWbSubBundle));
    }

    void PrepareAmrNbConfig()
    {
        m_objAmrNbPayloadType.Add(AMR_NB_PAYLOAD);
        ON_CALL(*m_pAudioBundle,
                GetIntArray(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY, _))
                .WillByDefault(Return(m_objAmrNbPayloadType));

        m_pAmrNbBundle = new MockICarrierConfig();
        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_DESCRIPTION_BUNDLE))
                .WillByDefault(Return(m_pAmrNbBundle));

        m_strAmrNbPayloadTypeNumber.SetNumber(AMR_NB_PAYLOAD);
        m_pAmrNbSubBundle = new MockICarrierConfig();

        ON_CALL(*m_pAmrNbBundle, GetBundle(IsSameKey(m_strAmrNbPayloadTypeNumber.GetStr())))
                .WillByDefault(Return(m_pAmrNbSubBundle));
    }

    virtual void TearDown() override
    {
        delete m_pICoreService;
        delete m_pMockICarrierConfig;
        delete m_pAudioBundle;
        delete m_pConfig;
        delete m_pEvsBundle;
        delete m_pEvsSubBundle;
        delete m_pAmrWbBundle;
        delete m_pAmrWbSubBundle;
        delete m_pAmrNbBundle;
        delete m_pAmrNbSubBundle;

        m_objEvsPayloadType.Clear();
        m_objAmrWbPayloadType.Clear();
        m_objAmrNbPayloadType.Clear();
    }

    /**
     * @brief Helper method to simulate a successful negotiation and set up a negotiated profile.
     *
     * This method mocks the negotiation process to populate the internal OA model list of the
     * AudioNego object with a negotiated profile containing a specific audio codec.
     *
     * @param codec The name of the codec to set up (e.g., "AMR-WB", "EVS").
     * @param bwList The bandwidth list to use for an EVS codec.
     */
    void SetUpNegotiatedProfile(const AString& codec, IMS_UINT32 bwList = 0)
    {
        MockISessionDescriptor objSessionDescriptor;
        MockIMediaDescriptor objMediaDescriptor;
        MEDIA_DIRECTION eDirection;

        MockMediaProfileFactory objMediaProfileFactory;
        MockMediaProfileFactory::SetInstance(&objMediaProfileFactory);

        auto pNegoProfile = std::make_shared<AudioProfile>();
        AudioProfile::Payload* pPayload = new AudioProfile::Payload();

        if (codec.EqualsIgnoreCase("EVS"))
        {
            pPayload->SetRtpMap(112, "EVS", 16000, 1);
            auto pEvsFmtp = std::make_shared<AudioProfile::EvsFmtp>();
            pEvsFmtp->SetBwList(bwList);
            pPayload->SetFmtp(pEvsFmtp);
        }
        else if (codec.EqualsIgnoreCase("AMR-WB"))
        {
            pPayload->SetRtpMap(99, "AMR-WB", 16000, 1);
            pPayload->SetFmtp(std::make_shared<AudioProfile::AmrFmtp>());
        }
        else if (codec.EqualsIgnoreCase("AMR"))
        {
            pPayload->SetRtpMap(97, "AMR", 8000, 1);
            pPayload->SetFmtp(std::make_shared<AudioProfile::AmrFmtp>());
        }
        else if (codec.EqualsIgnoreCase("PCMU"))
        {
            pPayload->SetRtpMap(0, "PCMU", 8000, 1);
        }

        pNegoProfile->AddPayload(pPayload);
        pNegoProfile->SetDirection(MEDIA_DIRECTION_SEND_RECEIVE);

        // Mock the negotiation process to populate the negotiated profile
        auto pLocalProfile = std::make_shared<AudioProfile>();
        auto pPeerProfile = std::make_shared<AudioProfile>();

        EXPECT_CALL(objMediaProfileFactory, CreateProfile(_, _))
                .WillOnce(Return(pLocalProfile))
                .WillOnce(Return(pPeerProfile))
                .WillOnce(Return(pNegoProfile));

        ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
        ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));

        m_pAudioNego->NegotiateSdp(
                STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);

        MockMediaProfileFactory::SetInstance(IMS_NULL);
    }
};

TEST_F(AudioNegoTest, testIsMediaCodecFromSdpSupported)
{
    MockIMediaDescriptor objAudioDescriptor;
    MockISessionDescriptor objSessionDescriptor;
    MockMediaProfileFactory objMediaProfileFactory;
    MockMediaProfileFactory::SetInstance(&objMediaProfileFactory);

    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_EQ(
            m_pAudioNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objAudioDescriptor),
            IMS_FALSE);

    auto pProfile = std::make_shared<AudioProfile>();
    AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
    pAmrPayload->SetRtpMap(99, "AMR-WB", 16000, 1);
    pAmrPayload->SetFmtp(std::make_shared<AudioProfile::AmrFmtp>());
    pProfile->AddPayload(pAmrPayload);
    pProfile->SetDataPort(10000);

    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_CALL(objMediaProfileFactory, CreateProfile(_, testing::NotNull()))
            .WillOnce(Return(pProfile));
    EXPECT_CALL(objMediaProfileFactory, CreateProfile(_, testing::IsNull()))
            .WillOnce(Return(pProfile))
            .WillOnce(Return(pProfile));

    EXPECT_EQ(
            m_pAudioNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objAudioDescriptor),
            IMS_TRUE);

    MockMediaProfileFactory::SetInstance(IMS_NULL);
}

TEST_F(AudioNegoTest, testFormSdpInvalidObject)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(*m_pMockAudioSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(
            STATE_IDLE, nullptr, &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptor, nullptr,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));

    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_NEGOTIATED, nullptr, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, nullptr,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));

    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_OFFER_RECEIVED, nullptr, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor, nullptr,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(AudioNegoTest, testFormSdpOfferIdle)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(*m_pMockAudioSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(AudioNegoTest, testFormSdpOfferNegotiated)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(*m_pMockAudioSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pAudioNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(AudioNegoTest, testFormSdpOfferOfferReceivedFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    // fail case: calling without negotiation
    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(AudioNegoTest, testFormSdpReoffer)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(*m_pMockAudioSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pAudioNego->FormSdp(STATE_NEGOTIATED, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_TRUE));
}

TEST_F(AudioNegoTest, testFormSdpInvalid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(
            STATE_IDLE, nullptr, &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptor, nullptr,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_INVALID, IMS_FALSE, IMS_FALSE));
}

TEST_F(AudioNegoTest, testFormSdpOfferIdleGenerateFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    ON_CALL(*m_pMockAudioSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_FALSE));
    EXPECT_FALSE(m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));
}

TEST_F(AudioNegoTest, testNegotiateSdpInvalidDescriptor)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    m_pAudioNego->NegotiateSdp(STATE_IDLE, nullptr, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pAudioNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, nullptr, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);

    m_pAudioNego->NegotiateSdp(STATE_NEGOTIATED, nullptr, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pAudioNego->NegotiateSdp(STATE_NEGOTIATED, &objSessionDescriptor, nullptr, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);

    m_pAudioNego->NegotiateSdp(STATE_OFFER_SENT, nullptr, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
    m_pAudioNego->NegotiateSdp(STATE_OFFER_SENT, &objSessionDescriptor, nullptr, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(AudioNegoTest, testNegotiateSdpIdleFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;
    m_pAudioNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(AudioNegoTest, testNegotiateSdpIdleParseFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_FALSE));
    m_pAudioNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(AudioNegoTest, testNegotiateSdpIdleNegotiateFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;
    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_FALSE));
    m_pAudioNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(AudioNegoTest, testNegotiateSdpIdleSuccessAndFormSdpOfferReceived)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    // setup the valid OA model
    MockMediaProfileFactory objMediaProfileFactory;
    MockMediaProfileFactory::SetInstance(&objMediaProfileFactory);

    auto pLocalProfile = std::make_shared<AudioProfile>();
    AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
    pAmrPayload->SetRtpMap(99, "AMR-WB", 16000, 1);
    pAmrPayload->SetFmtp(std::make_shared<AudioProfile::AmrFmtp>());
    pLocalProfile->AddPayload(pAmrPayload);
    pLocalProfile->SetDataPort(10000);

    auto pPeerProfile = std::make_shared<AudioProfile>(*pLocalProfile);
    auto pNegoProfile = std::make_shared<AudioProfile>(*pLocalProfile);
    pNegoProfile->SetDirection(MEDIA_DIRECTION_SEND);

    EXPECT_CALL(objMediaProfileFactory, CreateProfile(_, _))
            .WillOnce(Return(pLocalProfile))
            .WillOnce(Return(pPeerProfile))
            .WillOnce(Return(pNegoProfile));

    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pAudioNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_SEND);

    ON_CALL(*m_pMockAudioSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pAudioNego->FormSdp(STATE_OFFER_RECEIVED, &objSessionDescriptor,
            &objMediaDescriptor, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));

    MockMediaProfileFactory::SetInstance(IMS_NULL);
}

TEST_F(AudioNegoTest, testNegotiateSdpOfferSentFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;
    m_pAudioNego->NegotiateSdp(
            STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}

TEST_F(AudioNegoTest, testNegotiateSdpOfferSentSuccess)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MockMediaProfileFactory objMediaProfileFactory;
    MockMediaProfileFactory::SetInstance(&objMediaProfileFactory);

    auto pLocalProfile = std::make_shared<AudioProfile>();
    AudioProfile::Payload* pAmrPayload = new AudioProfile::Payload();
    pAmrPayload->SetRtpMap(99, "AMR-WB", 16000, 1);
    pAmrPayload->SetFmtp(std::make_shared<AudioProfile::AmrFmtp>());
    pLocalProfile->AddPayload(pAmrPayload);
    pLocalProfile->SetDataPort(10000);

    auto pPeerProfile = std::make_shared<AudioProfile>(*pLocalProfile);
    auto pNegoProfile = std::make_shared<AudioProfile>(*pLocalProfile);
    pNegoProfile->SetDirection(MEDIA_DIRECTION_SEND);

    EXPECT_CALL(objMediaProfileFactory, CreateProfile(_, _))
            .WillOnce(Return(pLocalProfile))
            .WillOnce(Return(pPeerProfile))
            .WillOnce(Return(pNegoProfile));

    // form offer in the idle state
    ON_CALL(*m_pMockAudioSdpGenerator, Generate(_, _, _)).WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE));

    MEDIA_DIRECTION eDirection;
    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pAudioNego->NegotiateSdp(
            STATE_OFFER_SENT, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_SEND);

    MockMediaProfileFactory::SetInstance(IMS_NULL);
}

TEST_F(AudioNegoTest, testNegotiateSdpOfferReceivedFail)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;
    m_pAudioNego->NegotiateSdp(
            STATE_OFFER_RECEIVED, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_EQ(eDirection, MEDIA_DIRECTION_INVALID);
}
TEST_F(AudioNegoTest, testCleanupIncompleteOaModels)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;

    m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE);
    EXPECT_EQ(m_pAudioNego->GetOaModelList().GetSize(), 1);

    m_pAudioNego->CleanupIncompleteOaModels();
    EXPECT_EQ(m_pAudioNego->GetOaModelList().GetSize(), 0);
}

TEST_F(AudioNegoTest, testCleanupIncompleteOaModelsRemovesIncomplete)
{
    MockISessionDescriptor objSessionDescriptorComplete;
    MockIMediaDescriptor objMediaDescriptorComplete;

    m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptorComplete, &objMediaDescriptorComplete,
            MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE);
    auto pCompleteOaModel = m_pAudioNego->GetOaModelList().GetAt(0);

    MockISessionDescriptor objSessionDescriptorIncomplete;
    MockIMediaDescriptor objMediaDescriptorIncomplete;
    m_pAudioNego->FormSdp(STATE_IDLE, &objSessionDescriptorIncomplete,
            &objMediaDescriptorIncomplete, MEDIA_DIRECTION_SEND, IMS_FALSE, IMS_FALSE);

    EXPECT_EQ(m_pAudioNego->GetOaModelList().GetSize(), 2);

    m_pAudioNego->CleanupIncompleteOaModels();
    EXPECT_EQ(m_pAudioNego->GetOaModelList().GetSize(), 1);
}

TEST_F(AudioNegoTest, testGetNegotiatedPayloadValid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pAudioNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_NE(m_pAudioNego->GetNegotiatedPayload(), IMS_NULL);
}

TEST_F(AudioNegoTest, testGetNegotiatedRtpPortValid)
{
    MockISessionDescriptor objSessionDescriptor;
    MockIMediaDescriptor objMediaDescriptor;
    MEDIA_DIRECTION eDirection;

    ON_CALL(*m_pMockAudioSdpParser, Parse(_, _, _)).WillByDefault(Return(IMS_TRUE));
    ON_CALL(*m_pMockProfileNegotiator, Negotiate(_, _, _, _, _)).WillByDefault(Return(IMS_TRUE));
    m_pAudioNego->NegotiateSdp(STATE_IDLE, &objSessionDescriptor, &objMediaDescriptor, eDirection);
    EXPECT_NE(m_pAudioNego->GetNegotiatedRtpPort(), -1);
}

TEST_F(AudioNegoTest, testSetPort)
{
    EXPECT_TRUE(m_pAudioNego->SetLocalPort(LOCAL_PORT));
    EXPECT_TRUE(m_pAudioNego->SetLocalPort(0));
}

TEST_F(AudioNegoTest, testGetters)
{
    EXPECT_EQ(m_pAudioNego->GetNegotiatedCodec(), AUDIO_CODEC_NONE);
    EXPECT_EQ(m_pAudioNego->GetLocalPort(), LOCAL_PORT);
    EXPECT_EQ(m_pAudioNego->GetNegotiatedBandwidth(), -1);
    EXPECT_EQ(m_pAudioNego->GetNegotiatedPayload(), nullptr);
    EXPECT_EQ(m_pAudioNego->GetNegotiatedDirection(), MEDIA_DIRECTION_INVALID);
    EXPECT_EQ(m_pAudioNego->GetNegotiatedRemoteAddress(), IpAddress::NONE);
    EXPECT_EQ(m_pAudioNego->GetNegotiatedLocalProfile(), nullptr);
    EXPECT_EQ(m_pAudioNego->GetNegotiatedNegoProfile(), nullptr);
    EXPECT_EQ(m_pAudioNego->GetNegotiatedPeerProfile(), nullptr);
    EXPECT_EQ(m_pAudioNego->GetRemotePort(), MEDIA_PORT_INVALID);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthKhzNoPayload)
{
    // Verifies behavior when no payload has been negotiated.
    // Expects 0.0f as there is no codec information.
    EXPECT_EQ(m_pAudioNego->GetNegotiatedCodecBandwidthKhz(), 0.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthKhzAmrWb)
{
    // Verifies correct bandwidth for a negotiated AMR-WB codec.
    SetUpNegotiatedProfile("AMR-WB");
    // AMR-WB has a fixed bandwidth of 8.0 kHz.
    EXPECT_EQ(m_pAudioNego->GetNegotiatedCodecBandwidthKhz(), 8.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthKhzAmr)
{
    // Verifies correct bandwidth for a negotiated AMR (narrowband) codec.
    SetUpNegotiatedProfile("AMR");
    // AMR-NB has a fixed bandwidth of 4.0 kHz.
    EXPECT_EQ(m_pAudioNego->GetNegotiatedCodecBandwidthKhz(), 4.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthKhzEvs)
{
    // Verifies correct bandwidth for a negotiated EVS codec.
    // The function should return the highest supported bandwidth.
    SetUpNegotiatedProfile("EVS", EVS_BW_NB | EVS_BW_WB | EVS_BW_SWB);
    // SWB (16.0 kHz) is the highest bandwidth in the list.
    EXPECT_EQ(m_pAudioNego->GetNegotiatedCodecBandwidthKhz(), 16.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthKhzUnknownCodec)
{
    // Verifies behavior with a codec not specifically handled by the function.
    SetUpNegotiatedProfile("PCMU");
    // Expects 0.0f for unhandled codecs.
    EXPECT_EQ(m_pAudioNego->GetNegotiatedCodecBandwidthKhz(), 0.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthRangeNoPayload)
{
    // Verifies behavior when no payload has been negotiated.
    IMS_FLOAT start = -1.0f, end = -1.0f;
    m_pAudioNego->GetNegotiatedCodecBandwidthRange(start, end);
    // Expects 0.0f for both start and end.
    EXPECT_EQ(start, 0.0f);
    EXPECT_EQ(end, 0.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthRangeAmrWb)
{
    // Verifies correct bandwidth range for a negotiated AMR-WB codec.
    SetUpNegotiatedProfile("AMR-WB");
    IMS_FLOAT start, end;
    m_pAudioNego->GetNegotiatedCodecBandwidthRange(start, end);
    // AMR-WB has a fixed bandwidth, so start and end should be the same.
    EXPECT_EQ(start, 8.0f);
    EXPECT_EQ(end, 8.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthRangeAmr)
{
    // Verifies correct bandwidth range for a negotiated AMR (narrowband) codec.
    SetUpNegotiatedProfile("AMR");
    IMS_FLOAT start, end;
    m_pAudioNego->GetNegotiatedCodecBandwidthRange(start, end);
    // AMR-NB has a fixed bandwidth, so start and end should be the same.
    EXPECT_EQ(start, 4.0f);
    EXPECT_EQ(end, 4.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthRangeEvs)
{
    // Verifies correct bandwidth range for a negotiated EVS codec.
    SetUpNegotiatedProfile("EVS", EVS_BW_NB | EVS_BW_WB | EVS_BW_SWB);
    IMS_FLOAT start, end;
    m_pAudioNego->GetNegotiatedCodecBandwidthRange(start, end);
    // The range should be from the lowest (NB) to the highest (SWB) supported bandwidth.
    EXPECT_EQ(start, 4.0f);
    EXPECT_EQ(end, 16.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthRangeEvsWbStart)
{
    // Verifies correct bandwidth range for EVS starting from WB.
    SetUpNegotiatedProfile("EVS", EVS_BW_WB | EVS_BW_SWB | EVS_BW_FB);
    IMS_FLOAT start, end;
    m_pAudioNego->GetNegotiatedCodecBandwidthRange(start, end);
    // The range should be from the lowest (WB) to the highest (FB) supported bandwidth.
    EXPECT_EQ(start, 8.0f);
    EXPECT_EQ(end, 20.0f);
}

TEST_F(AudioNegoTest, GetNegotiatedCodecBandwidthRangeUnknownCodec)
{
    // Verifies behavior with a codec not specifically handled by the function.
    SetUpNegotiatedProfile("PCMU");
    IMS_FLOAT start = -1.0f, end = -1.0f;
    m_pAudioNego->GetNegotiatedCodecBandwidthRange(start, end);
    // Expects 0.0f for both start and end for unhandled codecs.
    EXPECT_EQ(start, 0.0f);
    EXPECT_EQ(end, 0.0f);
}
