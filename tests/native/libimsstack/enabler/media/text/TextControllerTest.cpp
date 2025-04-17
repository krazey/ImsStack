/**
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

#include <gtest/gtest.h>
#include <ServiceConfig.h>
#include <text/TextController.h>
#include <text/MockTextNego.h>
#include <MockIMediaSessionListener.h>

#include "MockICarrierConfig.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const AString LOCAL_IP = "127.0.0.1";
const IMS_UINT32 LOCAL_PORT = 20000;

class TextControllerTest : public ::testing::Test
{
public:
    TextController* m_pController;
    TextConfiguration* m_pConfig;
    FakeIMediaSessionListener m_objFakeListener;
    MockIMediaSessionListener m_objListener;
    std::shared_ptr<MockTextNego> m_pTextNego;

    MockICarrierConfig* m_pMockICarrierConfig;
    MockICarrierConfig* m_pTextPayloads;

    TextProfile* m_pLocalProfile;
    TextProfile* m_pPeerProfile;
    TextProfile* m_pNegoProfile;
    IpAddress m_objIpAddr;

protected:
    virtual void SetUp() override
    {
        m_pController = new TextController();
        m_pConfig = new TextConfiguration(MEDIA_TYPE_TEXT);

        m_pMockICarrierConfig = new MockICarrierConfig();
        m_pTextPayloads = new MockICarrierConfig();

        ON_CALL(*m_pMockICarrierConfig,
                GetBundle(CarrierConfig::ImsRtt::KEY_TEXT_CODEC_CAPABILITY_PAYLOAD_TYPES_BUNDLE))
                .WillByDefault(Return(m_pTextPayloads));

        m_pConfig->Create(m_pMockICarrierConfig);
        m_pTextNego = std::make_shared<MockTextNego>(DEFAULT_SLOT_ID);

        m_objListener.SetDelegate(&m_objFakeListener);
        m_objListener.DelegateToFake();

        m_pLocalProfile = new TextProfile();
        TextProfile::Payload* pLocalT140Payload = new TextProfile::Payload();
        pLocalT140Payload->SetRtpMap(99, "t140", 1000);
        m_pLocalProfile->GetPayloadList().Append(pLocalT140Payload);

        TextProfile::Payload* pLocalRedPayload = new TextProfile::Payload();
        pLocalRedPayload->SetRtpMap(100, "red", 1000);
        pLocalRedPayload->SetFmtp(new TextProfile::RedFmtp(3, 99));
        m_pLocalProfile->GetPayloadList().Append(pLocalRedPayload);

        m_pPeerProfile = new TextProfile(*m_pLocalProfile);
        m_pNegoProfile = new TextProfile(*m_pLocalProfile);

        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(*m_pTextNego, GetLocalAddress()).WillByDefault(ReturnRef(m_objIpAddr));
        ON_CALL(*m_pTextNego, GetLocalPort()).WillByDefault(Return(LOCAL_PORT));
        ON_CALL(*m_pTextNego, GetNegotiatedLocalProfile()).WillByDefault(Return(m_pLocalProfile));
        ON_CALL(*m_pTextNego, GetNegotiatedPeerProfile()).WillByDefault(Return(m_pPeerProfile));
        ON_CALL(*m_pTextNego, GetNegotiatedNegoProfile()).WillByDefault(Return(m_pNegoProfile));
    }

    virtual void TearDown() override
    {
        delete m_pController;
        delete m_pConfig;
        delete m_pMockICarrierConfig;
        delete m_pTextPayloads;
        delete m_pLocalProfile;
        delete m_pPeerProfile;
        delete m_pNegoProfile;
    }
};

TEST_F(TextControllerTest, testCreateSessionFail)
{
    EXPECT_EQ(m_pController->CreateSession(nullptr, nullptr), IMS_FALSE);
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, nullptr), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateLocalAddressFail)
{
    EXPECT_EQ(m_pController->UpdateLocalAddress(nullptr), IMS_FALSE);
}

TEST_F(TextControllerTest, testOpenSessionFail)
{
    EXPECT_EQ(m_pController->OpenSession(), IMS_FALSE);
}

TEST_F(TextControllerTest, testCloseSessionWithSessionCreated)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
}

TEST_F(TextControllerTest, testOpenSessionMultipleTimes)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_FALSE);
}

TEST_F(TextControllerTest, testCloseSessionFail)
{
    EXPECT_EQ(m_pController->CloseSession(), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateRtpConfigWithNoSession)
{
    EXPECT_EQ(m_pController->UpdateRtpConfig(m_pTextNego), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateQualityThresholdWithNoSession)
{
    EXPECT_EQ(m_pController->UpdateQualityThreshold(m_pTextNego), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateSessionWithNoSession)
{
    EXPECT_EQ(m_pController->UpdateSession(), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateSessionBeforeOpenSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateRtpConfigBeforeOpenSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateRtpConfig(m_pTextNego), IMS_TRUE);
}

TEST_F(TextControllerTest, testUpdateQualityThresholdBeforeOpenSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateQualityThreshold(m_pTextNego), IMS_TRUE);
}

TEST_F(TextControllerTest, testCloseSessionAfterOpenSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
}

TEST_F(TextControllerTest, testModifySession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);

    EXPECT_EQ(m_pController->UpdateRtpConfig(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateQualityThreshold(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(), IMS_TRUE);
}

TEST_F(TextControllerTest, testUpdateQualityThreshold)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);

    EXPECT_EQ(m_pController->UpdateQualityThreshold(nullptr), IMS_FALSE);
    EXPECT_EQ(m_pController->UpdateQualityThreshold(m_pTextNego), IMS_TRUE);
}

TEST_F(TextControllerTest, testUpdateSessionAfterCloseSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateSession(), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateRtpConfigAfterCloseSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateRtpConfig(m_pTextNego), IMS_FALSE);
}

TEST_F(TextControllerTest, testUpdateQualityThresholdAfterCloseSession)
{
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    EXPECT_EQ(m_pController->UpdateQualityThreshold(m_pTextNego), IMS_FALSE);
}

TEST_F(TextControllerTest, testIsSessionOpened)
{
    // Initial state: No session
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Create session
    EXPECT_EQ(m_pController->CreateSession(&m_objListener, m_pConfig), IMS_TRUE);
    // Session created but not opened (state is STATE_NONE)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);

    // Open session
    EXPECT_EQ(m_pController->UpdateLocalAddress(m_pTextNego), IMS_TRUE);
    EXPECT_EQ(m_pController->OpenSession(), IMS_TRUE);
    // Session is now open (state is STATE_IDLE or higher)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_TRUE);

    // Close session
    EXPECT_EQ(m_pController->CloseSession(), IMS_TRUE);
    // Session is closed (state is STATE_NONE)
    EXPECT_EQ(m_pController->IsSessionOpened(), IMS_FALSE);
}
