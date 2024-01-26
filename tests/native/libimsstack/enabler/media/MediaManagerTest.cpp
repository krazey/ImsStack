/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "IMediaSession.h"
#include "MediaManager.h"
#include "MediaSession.h"
#include "MockIMediaSession.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const IMS_SINT32 CALL_KEY_BROADCAST = 0;
const IMS_SINT32 CALL_KEY_1 = 1;
const IMS_SINT32 CALL_KEY_2 = 2;
const IMS_SINT32 CALL_KEY_3 = 3;
const IMS_SINT32 CALL_KEY_4 = 4;
const IMS_CHAR DEFAULT_THREAD_NAME[] = "ET00.MediaManager";

class FakeMediaManager : public MediaManager
{
public:
    FakeMediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId) :
            MediaManager(strName, nSlotId)
    {
    }

    virtual ~FakeMediaManager() {}

    void FakeClearMediaSessionNode() { ClearMediaSessionNode(); }

    MediaSessionNode* FakeGetSessionNode(IMS_UINT32 nIndex)
    {
        return (m_lstSessionNode.GetSize() > nIndex) ? m_lstSessionNode.GetAt(nIndex) : IMS_NULL;
    }

    void FakeDeleteMediaSessionNode(IN MediaSessionNode* pSessionNode, IMS_UINT32 nIndex)
    {
        DeleteMediaSessionNode(pSessionNode, nIndex);
    }

    MediaSessionNode* FakeFindSessionNode(IMS_SINTP nCallKey) { return FindSessionNode(nCallKey); }
};

class MediaManagerTest : public ::testing::Test
{
public:
    FakeMediaManager* m_pMediaManager;

protected:
    virtual void SetUp() override
    {
        m_pMediaManager = new FakeMediaManager("MediaManager", DEFAULT_SLOT_ID);
    }

    virtual void TearDown() override { delete m_pMediaManager; }
};

TEST_F(MediaManagerTest, testMediaSessionCreateAndDestroy)
{
    IMediaSession* pIMediaSession =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), static_cast<MediaSession*>(pIMediaSession));
    EXPECT_NE(m_pMediaManager->GetHandler(CALL_KEY_1), nullptr);

    m_pMediaManager->DestroySession(pIMediaSession);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), nullptr);
    EXPECT_EQ(m_pMediaManager->GetHandler(CALL_KEY_1), nullptr);
}

TEST_F(MediaManagerTest, testGetThreadName)
{
    AString strThreadName = DEFAULT_THREAD_NAME;
    EXPECT_EQ(m_pMediaManager->GetThreadName(DEFAULT_SLOT_ID), strThreadName);
}

TEST_F(MediaManagerTest, testDestroySession)
{
    IMediaSession* pIMediaSession1 =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);
    IMediaSession* pIMediaSession2 =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_2);
    IMediaSession* pIMediaSession3 =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_3);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), static_cast<MediaSession*>(pIMediaSession1));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), static_cast<MediaSession*>(pIMediaSession2));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), static_cast<MediaSession*>(pIMediaSession3));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_4), nullptr);

    m_pMediaManager->DestroySession(pIMediaSession1);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), nullptr);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), static_cast<MediaSession*>(pIMediaSession2));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), static_cast<MediaSession*>(pIMediaSession3));

    m_pMediaManager->DestroySession(pIMediaSession2);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), nullptr);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), nullptr);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), static_cast<MediaSession*>(pIMediaSession3));

    m_pMediaManager->DestroySession(pIMediaSession3);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), nullptr);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), nullptr);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_3), nullptr);
}

TEST_F(MediaManagerTest, testGetResourceManager)
{
    EXPECT_NE(m_pMediaManager->GetResourceManager(), nullptr);
    EXPECT_EQ(m_pMediaManager->GetResourceManager()->GetMtu(), 1500);
}

TEST_F(MediaManagerTest, testSendMessage)
{
    // TODO : after changing void SendMessage() to bool.
}

TEST_F(MediaManagerTest, testHandleRequestMsg)
{
    // TODO : after changing void HandleRequestMsg() to bool.
}

TEST_F(MediaManagerTest, testClearMediaSessionNode)
{
    IMediaSession* pIMediaSession1 =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);
    IMediaSession* pIMediaSession2 =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_2);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), static_cast<MediaSession*>(pIMediaSession1));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), static_cast<MediaSession*>(pIMediaSession2));

    m_pMediaManager->FakeClearMediaSessionNode();

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), nullptr);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), nullptr);
}

TEST_F(MediaManagerTest, testDeleteMediaSessionNode)
{
    m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);

    IMS_UINT32 nIndex = 0;

    FakeMediaManager::MediaSessionNode* pSessionNode = m_pMediaManager->FakeGetSessionNode(nIndex);
    EXPECT_NE(pSessionNode, nullptr);
    EXPECT_NE(pSessionNode->pMediaSession, nullptr);
    EXPECT_NE(pSessionNode->pMessageHandler, nullptr);

    m_pMediaManager->FakeDeleteMediaSessionNode(pSessionNode, nIndex);

    pSessionNode = m_pMediaManager->FakeGetSessionNode(nIndex);
    EXPECT_EQ(pSessionNode, nullptr);
}

TEST_F(MediaManagerTest, testFindSessionNode)
{
    IMediaSession* pIMediaSession1 =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);
    IMediaSession* pIMediaSession2 =
            m_pMediaManager->CreateSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_2);

    FakeMediaManager::MediaSessionNode* pSessionNode1 = m_pMediaManager->FakeGetSessionNode(0);
    FakeMediaManager::MediaSessionNode* pSessionNode2 = m_pMediaManager->FakeGetSessionNode(1);

    EXPECT_EQ(m_pMediaManager->FakeFindSessionNode(CALL_KEY_1), pSessionNode1);
    EXPECT_EQ(m_pMediaManager->FakeFindSessionNode(CALL_KEY_2), pSessionNode2);

    EXPECT_EQ(m_pMediaManager->FakeFindSessionNode(CALL_KEY_1)->pMediaSession,
            static_cast<MediaSession*>(pIMediaSession1));
    EXPECT_EQ(m_pMediaManager->FakeFindSessionNode(CALL_KEY_2)->pMediaSession,
            static_cast<MediaSession*>(pIMediaSession2));

    m_pMediaManager->DestroySession(pIMediaSession1);
    m_pMediaManager->DestroySession(pIMediaSession2);
}