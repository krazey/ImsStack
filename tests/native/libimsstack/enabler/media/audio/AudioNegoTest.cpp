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
#include "audio/AudioNego.h"
#include "config/MediaSessionConfigFactory.h"
#include "media/MockIMediaDescriptor.h"
#include "offeranswer/SdpAvCodec.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_SINT32 EVS_PAYLOAD = 115;
const IMS_SINT32 AMR_WB_PAYLOAD = 99;
const IMS_SINT32 AMR_NB_PAYLOAD = 97;

class FakeAudioNego : public AudioNego
{
public:
    explicit FakeAudioNego(IN IMS_UINT32 nSlotId) :
            AudioNego(nSlotId)
    {
    }
    virtual ~FakeAudioNego() {}

protected:
};

MATCHER_P(IsSameKey, key, "")
{
    return IMS_StrCmp(arg, key) == 0;
}

class AudioNegoTest : public ::testing::Test
{
public:
    FakeAudioNego* m_pAudioNego;
    MediaEnvironment* m_pEnvironment;
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
        m_pAudioNego = new FakeAudioNego(DEFAULT_SLOT_ID);
        m_pAudioNego->CreateProfiles(m_pEnvironment, m_pConfig);
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
                GetIntArray(CarrierConfig::ImsVoice::KEY_EVS_PAYLOAD_TYPE_INT_ARRAY))
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
                GetIntArray(CarrierConfig::ImsVoice::KEY_AMRWB_PAYLOAD_TYPE_INT_ARRAY))
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
                GetIntArray(CarrierConfig::ImsVoice::KEY_AMRNB_PAYLOAD_TYPE_INT_ARRAY))
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
        delete m_pAudioNego;
        delete m_pEnvironment;
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
};

TEST_F(AudioNegoTest, testIsMediaCodecFromSdpSupported)
{
    MockIMediaDescriptor objAudioDescriptor;
    MockISessionDescriptor objSessionDescriptor;
    ImsList<SdpMediaFormat*> lstMediaFormat;

    SdpAvCodec objAvCodec;
    objAvCodec.SetValue("99");
    objAvCodec.SetParameters("99 AMR-WB/16000/1", "99 mode-set=2,4,6");

    lstMediaFormat.Append(&objAvCodec);
    ON_CALL(objAudioDescriptor, GetRemotePort()).WillByDefault(Return(50010));
    ON_CALL(objAudioDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormat));
    ON_CALL(objAudioDescriptor, GetAttribute(SdpAttribute::ANBR, AString::ConstNull()))
            .WillByDefault(ReturnRef(AString::ConstEmpty()));

    EXPECT_EQ(
            m_pAudioNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objAudioDescriptor),
            IMS_TRUE);

    ON_CALL(objAudioDescriptor, GetRemotePort()).WillByDefault(Return(0));
    EXPECT_EQ(
            m_pAudioNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objAudioDescriptor),
            IMS_FALSE);

    lstMediaFormat.Clear();
    objAvCodec.SetValue("100");
    objAvCodec.SetParameters("100 AMR/16000/1", "100 mode-set=2,4,6");

    lstMediaFormat.Append(&objAvCodec);
    ON_CALL(objAudioDescriptor, GetRemotePort()).WillByDefault(Return(50010));
    EXPECT_EQ(
            m_pAudioNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objAudioDescriptor),
            IMS_FALSE);
}
