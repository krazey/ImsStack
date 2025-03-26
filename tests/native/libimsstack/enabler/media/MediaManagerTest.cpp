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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MediaManager.h"
#include "MockICoreService.h"
#include "MediaResourceManager.h"
#include "MediaSession.h"
#include "MockIMediaSession.h"
#include "MockMediaMsgHandler.h"

using ::testing::Return;
using ::testing::ReturnRef;

const IMS_SINT32 DEFAULT_SLOT_ID = 0;
const IMS_SINT32 CALL_KEY_BROADCAST = 0;
const IMS_SINT32 CALL_KEY_1 = 1;
const IMS_SINT32 CALL_KEY_2 = 2;
const IMS_SINT32 CALL_KEY_3 = 3;
const IMS_SINT32 CALL_KEY_4 = 4;
const IMS_CHAR DEFAULT_THREAD_NAME[] = "ET00.MediaManager";
const AString LOCAL_IP = "127.0.0.1";

class FakeMediaManager : public MediaManager
{
public:
    class FakeMediaSessionNode
    {
    public:
        IMS_SINTP nCallKey;
        MockIMediaSession* pMediaSession;
        MockMediaMsgHandler* pMessageHandler;

    public:
        FakeMediaSessionNode() :
                nCallKey(0),
                pMediaSession(IMS_NULL),
                pMessageHandler(IMS_NULL) {};

        FakeMediaSessionNode(IN IMS_SINTP callKey, IN MockIMediaSession* pSession,
                IN MockMediaMsgHandler* pHandler) :
                nCallKey(callKey),
                pMediaSession(pSession),
                pMessageHandler(pHandler) {};
    };

public:
    FakeMediaManager(IN CONST AString& strName, IN IMS_SINT32 nSlotId) :
            MediaManager(strName, nSlotId),
            m_lstFakeSessionNode(ImsList<FakeMediaSessionNode*>())
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

    MockIMediaSession* CreateFakeSession(
            IN MEDIA_SERVICE_TYPE /* nService */, IN IMS_SINTP nCallKey)
    {
        MockIMediaSession* pSession = new MockIMediaSession();
        MockMediaMsgHandler* pHandler = new MockMediaMsgHandler(m_nSlotId, nCallKey);
        FakeMediaSessionNode* pSessionNode = new FakeMediaSessionNode(nCallKey, pSession, pHandler);

        m_lstFakeSessionNode.Append(pSessionNode);

        return pSession;
    }

    IMS_BOOL SendMessage(IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam) override
    {
        IMS_BOOL bResult = IMS_TRUE;

        if (nCallKey == CALL_KEY_BROADCAST)  // broadcast message to the all sessions
        {
            for (IMS_UINT32 i = 0; i < m_lstFakeSessionNode.GetSize(); i++)
            {
                FakeMediaSessionNode* pSessionNode = m_lstFakeSessionNode.GetAt(i);

                if (pSessionNode == IMS_NULL || pSessionNode->pMediaSession == IMS_NULL)
                {
                    bResult = IMS_FALSE;
                }
                else
                {
                    if (pSessionNode->pMediaSession->SendMessage(nMsg, pParam) == IMS_FALSE)
                    {
                        bResult = IMS_FALSE;
                    }
                }
            }
        }
        else
        {
            if (FakeSendMessageToSessions(nMsg, nCallKey, pParam) != IMS_TRUE)
            {
                bResult = IMS_FALSE;
            }
        }

        return bResult;
    }

    IMS_BOOL HandleRequestMsg(
            IN IMS_SINT32 eEvent, IN IMS_SINTP nCallKey, IN ImsMediaMsgParamBase* param) override
    {
        FakeMediaSessionNode* pSessionNode = FindFakeSessionNode(nCallKey);

        if (pSessionNode == IMS_NULL || pSessionNode->pMessageHandler == IMS_NULL)
        {
            return IMS_FALSE;
        }

        if (pSessionNode->pMessageHandler->SendMessageToJava(eEvent, param))
        {
            return IMS_TRUE;
        }

        return IMS_FALSE;
    }

    IMS_BOOL FakeSendMessageToSessions(
            IN IMS_SINT32 nMsg, IN IMS_SINTP nCallKey, IN IMS_UINTP pParam)
    {
        MockIMediaSession* pMediaSession = GetFakeSession(nCallKey);

        if (pMediaSession != IMS_NULL)
        {
            if (pMediaSession->SendMessage(nMsg, pParam) == IMS_FALSE)
            {
                return IMS_FALSE;
            }
        }

        return IMS_TRUE;
    }

    MockIMediaSession* GetFakeSession(IN IMS_SINTP nCallKey)
    {
        FakeMediaSessionNode* pSessionNode = FindFakeSessionNode(nCallKey);

        if (pSessionNode == IMS_NULL)
        {
            return IMS_NULL;
        }

        return pSessionNode->pMediaSession;
    }

    FakeMediaSessionNode* FindFakeSessionNode(IN IMS_SINTP nCallKey)
    {
        for (IMS_UINT32 i = 0; i < m_lstFakeSessionNode.GetSize(); i++)
        {
            FakeMediaSessionNode* pSessionNode = m_lstFakeSessionNode.GetAt(i);

            if (pSessionNode != IMS_NULL)
            {
                if (pSessionNode->nCallKey == nCallKey)
                {
                    return pSessionNode;
                }
            }
        }

        return IMS_NULL;
    }

    void DestroyFakeSession(IN const MockIMediaSession* piSession)
    {
        if (piSession == IMS_NULL)
        {
            return;
        }

        for (IMS_UINT32 i = 0; i < m_lstFakeSessionNode.GetSize(); i++)
        {
            FakeMediaSessionNode* pSessionNode = m_lstFakeSessionNode.GetAt(i);

            if (pSessionNode->pMediaSession == piSession)
            {
                DeleteFakeMediaSessionNode(pSessionNode, i);
                return;
            }
        }
    }

    void DeleteFakeMediaSessionNode(IN FakeMediaSessionNode* pSessionNode, IMS_UINT32 nIndex)
    {
        if (pSessionNode != IMS_NULL)
        {
            if (pSessionNode->pMediaSession != IMS_NULL)
            {
                delete pSessionNode->pMediaSession;
            }

            if (pSessionNode->pMessageHandler != IMS_NULL)
            {
                delete pSessionNode->pMessageHandler;
            }

            delete pSessionNode;
        }

        m_lstFakeSessionNode.RemoveAt(nIndex);
    }

protected:
    ImsList<FakeMediaSessionNode*> m_lstFakeSessionNode;
};

class MediaManagerTest : public ::testing::Test
{
public:
    FakeMediaManager* m_pMediaManager;
    MockICoreService m_objCoreService;
    IpAddress m_objIpAddr;

protected:
    virtual void SetUp() override
    {
        m_pMediaManager = new FakeMediaManager("MediaManager", DEFAULT_SLOT_ID);
        m_objIpAddr = IpAddress(LOCAL_IP);
        ON_CALL(m_objCoreService, GetIpAddress()).WillByDefault(ReturnRef(m_objIpAddr));
    }

    virtual void TearDown() override { delete m_pMediaManager; }
};

TEST_F(MediaManagerTest, testMediaSessionCreateAndDestroy)
{
    IMediaSession* pIMediaSession = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);

    ASSERT_NE(pIMediaSession, IMS_NULL);
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
    IMediaSession* pIMediaSession1 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    IMediaSession* pIMediaSession2 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_2);
    IMediaSession* pIMediaSession3 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_3);

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
    EXPECT_EQ(m_pMediaManager->GetResourceManager()->GetMtu(), 0);
}

TEST_F(MediaManagerTest, testSendMessage_CommonResponse)
{
    MockIMediaSession* pMockIMediaSession =
            m_pMediaManager->CreateFakeSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);

    ImsMediaResponseConfigParam* pParam1 = new ImsMediaResponseConfigParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::RESPONSE_OPEN_SESSION, reinterpret_cast<IMS_UINTP>(pParam1)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::RESPONSE_OPEN_SESSION, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam1)));

    ImsMediaResponseConfigParam* pParam2 = new ImsMediaResponseConfigParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::RESPONSE_MODIFY_SESSION, reinterpret_cast<IMS_UINTP>(pParam2)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::RESPONSE_MODIFY_SESSION, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam2)));

    ImsMediaResponseConfigParam* pParam3 = new ImsMediaResponseConfigParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::RESPONSE_ADD_CONFIG, reinterpret_cast<IMS_UINTP>(pParam3)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::RESPONSE_ADD_CONFIG, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam3)));

    ImsMediaResponseConfigParam* pParam4 = new ImsMediaResponseConfigParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::RESPONSE_CONFIRM_CONFIG, reinterpret_cast<IMS_UINTP>(pParam4)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::RESPONSE_CONFIRM_CONFIG, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam4)));

    delete pParam1;
    delete pParam2;
    delete pParam3;
    delete pParam4;

    m_pMediaManager->DestroyFakeSession(pMockIMediaSession);
}

TEST_F(MediaManagerTest, testSendMessage_CommonNotification)
{
    MockIMediaSession* pMockIMediaSession =
            m_pMediaManager->CreateFakeSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);

    ImsMediaResponseConfigParam* pParam1 = new ImsMediaResponseConfigParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::NOTIFY_FIRST_PACKET, reinterpret_cast<IMS_UINTP>(pParam1)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::NOTIFY_FIRST_PACKET, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam1)));

    ImsMediaMsgParamBase* pParam2 = new ImsMediaMsgParamBase();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::NOTIFY_MEDIA_INACTIVITY, reinterpret_cast<IMS_UINTP>(pParam2)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::NOTIFY_MEDIA_INACTIVITY, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam2)));

    ImsMediaResponseConfigParam* pParam3 = new ImsMediaResponseConfigParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::NOTIFY_MEDIA_DETACH, reinterpret_cast<IMS_UINTP>(pParam3)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::NOTIFY_MEDIA_DETACH, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam3)));

    ImsMediaMsgQosParam* pParam4 = new ImsMediaMsgQosParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::NOTIFY_QOS_INFO, reinterpret_cast<IMS_UINTP>(pParam4)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::NOTIFY_QOS_INFO, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam4)));

    ImsMediaVideoParam* pParam5 = new ImsMediaVideoParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::NOTIFY_VIDEO_BITRATE, reinterpret_cast<IMS_UINTP>(pParam5)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::NOTIFY_VIDEO_BITRATE, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam5)));

    delete pParam1;
    delete pParam2;
    delete pParam3;
    delete pParam4;
    delete pParam5;

    m_pMediaManager->DestroyFakeSession(pMockIMediaSession);
}

TEST_F(MediaManagerTest, testSendMessage_VideoIndication)
{
    MockIMediaSession* pMockIMediaSession =
            m_pMediaManager->CreateFakeSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);

    ImsMediaVideoParam* pParam1 = new ImsMediaVideoParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::SETSURFACE_CMD, reinterpret_cast<IMS_UINTP>(pParam1)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::SETSURFACE_CMD, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam1)));

    ImsMediaVideoParam* pParam2 = new ImsMediaVideoParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::SELECT_CAMERA_CMD, reinterpret_cast<IMS_UINTP>(pParam2)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::SELECT_CAMERA_CMD, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam2)));

    ImsMediaVideoParam* pParam3 = new ImsMediaVideoParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::CHANGE_CAMERA_ZOOM_CMD, reinterpret_cast<IMS_UINTP>(pParam3)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::CHANGE_CAMERA_ZOOM_CMD, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam3)));

    ImsMediaVideoParam* pParam4 = new ImsMediaVideoParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::SET_PAUSE_IMAGE_CMD, reinterpret_cast<IMS_UINTP>(pParam4)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::SET_PAUSE_IMAGE_CMD, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam4)));

    ImsMediaVideoParam* pParam5 = new ImsMediaVideoParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::CHANGE_ORIENTATION_CMD, reinterpret_cast<IMS_UINTP>(pParam5)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::CHANGE_ORIENTATION_CMD, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam5)));

    delete pParam1;
    delete pParam2;
    delete pParam3;
    delete pParam4;
    delete pParam5;

    m_pMediaManager->DestroyFakeSession(pMockIMediaSession);
}

TEST_F(MediaManagerTest, testSendMessage_Etc)
{
    MockIMediaSession* pMockIMediaSession =
            m_pMediaManager->CreateFakeSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);

    ImsMediaMsgDtmfParam* pParam1 = new ImsMediaMsgDtmfParam();
    ON_CALL(*pMockIMediaSession,
            SendMessage(IJniMedia::SEND_DTMF, reinterpret_cast<IMS_UINTP>(pParam1)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(
            IJniMedia::SEND_DTMF, CALL_KEY_1, reinterpret_cast<IMS_UINTP>(pParam1)));

    IMS_UINT32 nRatType = 3;
    ON_CALL(*pMockIMediaSession, SendMessage(IJniMedia::CHANGE_NETWORK_CONNECTION, nRatType))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->SendMessage(IJniMedia::CHANGE_NETWORK_CONNECTION, 0, nRatType));

    delete pParam1;

    m_pMediaManager->DestroyFakeSession(pMockIMediaSession);
}

TEST_F(MediaManagerTest, testHandleRequestMsg_Common)
{
    MockIMediaSession* pMockIMediaSession =
            m_pMediaManager->CreateFakeSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);
    FakeMediaManager::FakeMediaSessionNode* pNode =
            m_pMediaManager->FindFakeSessionNode(CALL_KEY_1);
    MockMediaMsgHandler* pHandler = pNode->pMessageHandler;

    ImsMediaMsgOpenConfigParam* pParam1 = new ImsMediaMsgOpenConfigParam(MEDIA_TYPE_AUDIO);
    ON_CALL(*pHandler,
            SendMessageToJava(
                    IJniMedia::REQUEST_OPEN_SESSION, static_cast<ImsMediaMsgParamBase*>(pParam1)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_OPEN_SESSION, CALL_KEY_1, pParam1));

    ImsMediaMsgConfigParam* pParam2 = new ImsMediaMsgConfigParam(MEDIA_TYPE_AUDIO);
    ON_CALL(*pHandler, SendMessageToJava(IJniMedia::REQUEST_MODIFY_SESSION, pParam2))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_MODIFY_SESSION, CALL_KEY_1, pParam2));

    ON_CALL(*pHandler, SendMessageToJava(IJniMedia::REQUEST_ADD_CONFIG, pParam2))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(
            m_pMediaManager->HandleRequestMsg(IJniMedia::REQUEST_ADD_CONFIG, CALL_KEY_1, pParam2));

    ON_CALL(*pHandler, SendMessageToJava(IJniMedia::REQUEST_DELETE_CONFIG, pParam2))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_DELETE_CONFIG, CALL_KEY_1, pParam2));

    ON_CALL(*pHandler, SendMessageToJava(IJniMedia::REQUEST_CONFIRM_CONFIG, pParam2))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_CONFIRM_CONFIG, CALL_KEY_1, pParam2));

    ON_CALL(*pHandler, SendMessageToJava(IJniMedia::REQUEST_CLOSE_SESSION, pParam2))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_CLOSE_SESSION, CALL_KEY_1, pParam2));

    ImsMediaMsgSetMediaQualityParam* pParam3 =
            new ImsMediaMsgSetMediaQualityParam(MEDIA_TYPE_AUDIO);
    ON_CALL(*pHandler,
            SendMessageToJava(IJniMedia::REQUEST_SET_MEDIA_QUALITY,
                    static_cast<ImsMediaMsgParamBase*>(pParam3)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_SET_MEDIA_QUALITY, CALL_KEY_1, pParam3));

    delete pParam1;
    delete pParam2;
    delete pParam3;

    m_pMediaManager->DestroyFakeSession(pMockIMediaSession);
}

TEST_F(MediaManagerTest, testHandleRequestMsg_Audio)
{
    MockIMediaSession* pMockIMediaSession =
            m_pMediaManager->CreateFakeSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);
    FakeMediaManager::FakeMediaSessionNode* pNode =
            m_pMediaManager->FindFakeSessionNode(CALL_KEY_1);
    MockMediaMsgHandler* pHandler = pNode->pMessageHandler;

    ImsMediaMsgDtmfParam* pParam = new ImsMediaMsgDtmfParam;
    ON_CALL(*pHandler,
            SendMessageToJava(
                    IJniMedia::REQUEST_SEND_DTMF, static_cast<ImsMediaMsgParamBase*>(pParam)))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(
            m_pMediaManager->HandleRequestMsg(IJniMedia::REQUEST_SEND_DTMF, CALL_KEY_1, pParam));

    delete pParam;

    m_pMediaManager->DestroyFakeSession(pMockIMediaSession);
}

TEST_F(MediaManagerTest, testHandleRequestMsg_Video)
{
    MockIMediaSession* pMockIMediaSession =
            m_pMediaManager->CreateFakeSession(MEDIA_SERVICE_DEFAULT, CALL_KEY_1);
    FakeMediaManager::FakeMediaSessionNode* pNode =
            m_pMediaManager->FindFakeSessionNode(CALL_KEY_1);
    MockMediaMsgHandler* pHandler = pNode->pMessageHandler;

    ImsMediaMsgParamBase* pParam = new ImsMediaMsgParamBase(MEDIA_TYPE_VIDEO);
    ON_CALL(*pHandler, SendMessageToJava(IJniMedia::REQUEST_SET_DISPLAY_SURFACE, pParam))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_SET_DISPLAY_SURFACE, CALL_KEY_1, pParam));

    ON_CALL(*pHandler, SendMessageToJava(IJniMedia::REQUEST_SET_PREVIEW_SURFACE, pParam))
            .WillByDefault(Return(IMS_TRUE));
    EXPECT_TRUE(m_pMediaManager->HandleRequestMsg(
            IJniMedia::REQUEST_SET_PREVIEW_SURFACE, CALL_KEY_1, pParam));

    delete pParam;

    m_pMediaManager->DestroyFakeSession(pMockIMediaSession);
}

TEST_F(MediaManagerTest, testClearMediaSessionNode)
{
    IMediaSession* pIMediaSession1 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    IMediaSession* pIMediaSession2 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_2);

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), static_cast<MediaSession*>(pIMediaSession1));
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), static_cast<MediaSession*>(pIMediaSession2));

    m_pMediaManager->FakeClearMediaSessionNode();

    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_1), nullptr);
    EXPECT_EQ(m_pMediaManager->GetSession(CALL_KEY_2), nullptr);
}

TEST_F(MediaManagerTest, testDeleteMediaSessionNode)
{
    m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);

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
    IMediaSession* pIMediaSession1 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_1);
    IMediaSession* pIMediaSession2 = m_pMediaManager->CreateSession(
            MEDIA_NETWORK_WIFI, MEDIA_SERVICE_DEFAULT, &m_objCoreService, CALL_KEY_2);

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
