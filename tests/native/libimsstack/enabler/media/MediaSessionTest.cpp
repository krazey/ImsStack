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

#include <gtest/gtest.h>
#include <MediaSession.h>
#include <MediaEnvironment.h>
#include <IMMedia.h>
#include <MockICoreService.h>
#include <MockIMediaSessionClientListener.h>

using ::testing::ReturnRef;

const AString LOCAL_IP = "127.0.0.1";
const AString REMOTE_IP = "127.0.0.1";
const IMS_UINT32 REMOTE_PORT = 40000;

class MediaSessionTest : public ::testing::Test
{
public:
    MediaSession* m_pSession;
    MediaEnvironment* m_pEnvironment;
    MockICoreService m_objMockICoreService;
    MockIMediaSessionClientListener m_objClientListener;
    IPAddress m_objIpAddr;

protected:
    virtual void SetUp() override
    {
        m_objIpAddr = IPAddress(LOCAL_IP);
        ON_CALL(m_objMockICoreService, GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));

        m_pEnvironment = new MediaEnvironment();
        m_pEnvironment->eNetworkType = MEDIA_NETWORK_LTE;
        m_pEnvironment->eServiceType = MEDIA_SERVICE_DEFAULT;
        m_pEnvironment->pIService = &m_objMockICoreService;

        m_pSession = new MediaSession(MEDIA_SERVICE_DEFAULT, 1, 0);
        m_pSession->SetEnvironment(m_pEnvironment);
        m_pSession->SetMtcListener(&m_objClientListener);
    }

    virtual void TearDown() override { delete m_pSession; }
};

TEST_F(MediaSessionTest, testQosCallback)
{
    IMS_UINTP negoId = m_pSession->CreateProfile(0, MEDIA_TYPE_AUDIO);
    EXPECT_NE(negoId, 0);

    ImsMediaNotifyQosParam* pParam = new ImsMediaNotifyQosParam();
    pParam->m_eMediaType = MEDIA_TYPE_AUDIO;
    pParam->m_objIpAddr = IPAddress(REMOTE_IP);
    pParam->m_nPort = REMOTE_PORT;
    pParam->m_bResult = IMS_TRUE;

    EXPECT_EQ(
            m_pSession->SendMessage(IMMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam)),
            IMS_FALSE);

    EXPECT_EQ(m_pSession->DestroyProfile(negoId), IMS_TRUE);
}