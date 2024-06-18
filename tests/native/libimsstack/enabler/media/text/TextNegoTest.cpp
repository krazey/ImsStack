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
#include "config/MediaSessionConfigFactory.h"
#include "media/MockIMediaDescriptor.h"
#include "offeranswer/SdpAvCodec.h"
#include "text/TextNego.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_SINT32 T140_PAYLOAD = 112;
const IMS_SINT32 RED_PAYLOAD = 111;

class FakeTextNego : public TextNego
{
public:
    explicit FakeTextNego(IN IMS_UINT32 nSlotId) :
            TextNego(nSlotId)
    {
    }
    virtual ~FakeTextNego() {}

protected:
};

class TextNegoTest : public ::testing::Test
{
public:
    FakeTextNego* m_pTextNego;
    MediaEnvironment* m_pEnvironment;
    TextConfiguration* m_pConfig;
    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pTextBundle;
    MockICoreService* m_pICoreService;
    IpAddress m_objIpAddr;

protected:
    virtual void SetUp() override
    {
        CreateEnvironment();
        PrepareTextConfig();

        CreateTextConfig();
        CreateNegoProfile();
    }

    void CreateTextConfig() { m_pConfig->Create(m_pMockICarrierConfig); }

    void CreateNegoProfile()
    {
        m_pTextNego = new FakeTextNego(DEFAULT_SLOT_ID);
        m_pTextNego->CreateProfiles(m_pEnvironment, MEDIA_TYPE_TEXT, m_pConfig);
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

    void PrepareTextConfig()
    {
        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pTextBundle = new MockICarrierConfig();

        m_pConfig = new TextConfiguration(MEDIA_TYPE_AUDIOTEXT);

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pTextBundle));

        ON_CALL(*m_pTextBundle, GetInt(CarrierConfig::ImsRtt::KEY_T140_PAYLOAD_TYPE_INT, -1))
                .WillByDefault(Return(T140_PAYLOAD));
        ON_CALL(*m_pTextBundle, GetInt(CarrierConfig::ImsRtt::KEY_RED_PAYLOAD_TYPE_INT, -1))
                .WillByDefault(Return(RED_PAYLOAD));
    }

    virtual void TearDown() override
    {
        delete m_pTextNego;
        delete m_pEnvironment;
        delete m_pICoreService;
        delete m_pMockICarrierConfig;
        delete m_pTextBundle;
        delete m_pConfig;
    }
};

TEST_F(TextNegoTest, testIsMediaCodecFromSdpSupported)
{
    MockIMediaDescriptor objTextDescriptor;
    MockISessionDescriptor objSessionDescriptor;
    ImsList<SdpMediaFormat*> lstMediaFormat;

    SdpAvCodec objAvCodec;
    objAvCodec.SetValue("112");
    objAvCodec.SetParameters("112 red/1000", "112 111/111/111");
    objAvCodec.SetValue("111");
    objAvCodec.SetParameters("111 t140/1000", "");

    lstMediaFormat.Append(&objAvCodec);
    ON_CALL(objTextDescriptor, GetRemotePort()).WillByDefault(Return(50010));
    ON_CALL(objTextDescriptor, GetMediaFormats()).WillByDefault(ReturnRef(lstMediaFormat));

    EXPECT_EQ(m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objTextDescriptor),
            IMS_TRUE);

    ON_CALL(objTextDescriptor, GetRemotePort()).WillByDefault(Return(0));

    EXPECT_EQ(m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objTextDescriptor),
            IMS_FALSE);

    lstMediaFormat.Clear();
    objAvCodec.SetValue("111");
    objAvCodec.SetParameters("111 utf8/1000", "");

    lstMediaFormat.Append(&objAvCodec);
    ON_CALL(objTextDescriptor, GetRemotePort()).WillByDefault(Return(50010));
    EXPECT_EQ(m_pTextNego->IsMediaCodecFromSdpSupported(&objSessionDescriptor, &objTextDescriptor),
            IMS_FALSE);
}
