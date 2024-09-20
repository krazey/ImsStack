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
#include <gmock/gmock.h>
#include "UceService.h"
#include "IUce.h"
#include "MockICoreService.h"
#include "MockIFeatureCaps.h"
#include "MockIPageMessage.h"
#include "MockIReference.h"
#include "MockISession.h"
#include "MockIMessage.h"
#include "MockICapabilities.h"
#include "def/UceDef.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;

__IMS_TRACE_TAG_UCE__;

class TestUceService : public UceService
{
public:
    enum
    {
        SUBSCRIBE = 0,
        PUBLISH,
        OPTIONS,
    };

public:
    inline explicit TestUceService(ICoreService* piCoreService) :
            UceService(piCoreService)
    {
    }
    virtual ~TestUceService() {}

    void resetManager()
    {
        m_pUceSubscribeManager = IMS_NULL;
        m_pUcePublishManager = IMS_NULL;
        m_pUceOptionsManager = IMS_NULL;
    }
    void enableManagers() { EnableManager(); }
    void disableManagers() { DisableManager(); }
    IMS_BOOL isNull(IMS_UINT32 manager) const
    {
        switch (manager)
        {
            case SUBSCRIBE:
            {
                if (m_pUceSubscribeManager == IMS_NULL)
                {
                    return IMS_TRUE;
                }
            }
            break;
            case PUBLISH:
            {
                if (m_pUcePublishManager == IMS_NULL)
                {
                    return IMS_TRUE;
                }
            }
            break;
            case OPTIONS:
            {
                if (m_pUceOptionsManager == IMS_NULL)
                {
                    return IMS_TRUE;
                }
            }
            break;
            default:
                break;
        }
        return IMS_FALSE;
    }
    void setFeatures(IFeatureCaps* piFCaps, IMS_BOOL bAddVideoTag, IMS_UINT32 conectedService)
    {
        SetFeatures(piFCaps, bAddVideoTag, conectedService);
    }
    void pageMessageReceived(ICoreService* piService, IPageMessage* piMessage)
    {
        CoreService_PageMessageReceived(piService, piMessage);
    }
    void referenceReceived(ICoreService* piService, IReference* piReference)
    {
        CoreService_ReferenceReceived(piService, piReference);
    }
    void serviceClosed(ICoreService* piService, IReasonInfo* piReasonInfo)
    {
        CoreService_ServiceClosed(piService, piReasonInfo);
    }
    void sessionInvitationReceived(ICoreService* piService, ISession* piSession)
    {
        CoreService_SessionInvitationReceived(piService, piSession);
    }
    void unsolicitedNotifyReceived(ICoreService* piService, IMessage* piNotify)
    {
        CoreService_UnsolicitedNotifyReceived(piService, piNotify);
    }
    void capabilityQueryReceived(ICoreService* piService, ICapabilities* piCapabilities)
    {
        CoreService_CapabilityQueryReceived(piService, piCapabilities);
    }
};

class UceServiceTest : public ::testing::Test
{
public:
    TestUceService* pUceService;
    MockICoreService objMockICoreService;
    MockIFeatureCaps objMockIFeatureCaps;

protected:
    virtual void SetUp() override
    {
        pUceService = new TestUceService(&objMockICoreService);
        ASSERT_TRUE(pUceService != nullptr);
        ON_CALL(objMockICoreService, GetFeatureCaps()).WillByDefault(Return(&objMockIFeatureCaps));
    }

    virtual void TearDown() override
    {
        if (pUceService)
        {
            delete pUceService;
        }
    }
};

TEST_F(UceServiceTest, EnableManager)
{
    IMS_TRACE_D("EnableManager", 0, 0, 0);
    pUceService->resetManager();
    EXPECT_TRUE(pUceService->isNull(TestUceService::SUBSCRIBE));
    EXPECT_TRUE(pUceService->isNull(TestUceService::PUBLISH));
    EXPECT_TRUE(pUceService->isNull(TestUceService::OPTIONS));
    pUceService->enableManagers();
    EXPECT_FALSE(pUceService->isNull(TestUceService::SUBSCRIBE));
    EXPECT_FALSE(pUceService->isNull(TestUceService::PUBLISH));
    EXPECT_FALSE(pUceService->isNull(TestUceService::OPTIONS));
}

TEST_F(UceServiceTest, DisableManager)
{
    IMS_TRACE_D("DisableManager", 0, 0, 0);
    EXPECT_FALSE(pUceService->isNull(TestUceService::SUBSCRIBE));
    EXPECT_FALSE(pUceService->isNull(TestUceService::PUBLISH));
    EXPECT_FALSE(pUceService->isNull(TestUceService::OPTIONS));
    pUceService->disableManagers();
    EXPECT_TRUE(pUceService->isNull(TestUceService::SUBSCRIBE));
    EXPECT_TRUE(pUceService->isNull(TestUceService::PUBLISH));
    EXPECT_TRUE(pUceService->isNull(TestUceService::OPTIONS));
}

TEST_F(UceServiceTest, AoSConnected)
{
    IMS_TRACE_D("AoSConnected", 0, 0, 0);
    ON_CALL(objMockICoreService, GetFeatureCaps()).WillByDefault(Return(&objMockIFeatureCaps));

    EXPECT_CALL(objMockIFeatureCaps, RemoveFeature(_, _)).Times(1);
    EXPECT_CALL(objMockIFeatureCaps, AddFeature(_, _)).Times(1);
    pUceService->AoSConnected(CONNECTED_SERVICE_VIDEO);
}

TEST_F(UceServiceTest, SetFeatures)
{
    IMS_TRACE_D("SetFeatures", 0, 0, 0);

    EXPECT_CALL(objMockIFeatureCaps, RemoveFeature(_, _, _, _)).Times(1);
    EXPECT_CALL(objMockIFeatureCaps, AddFeature(_, _, _, _)).Times(1);
    pUceService->setFeatures(&objMockIFeatureCaps, IMS_TRUE, CONNECTED_SERVICE_VIDEO);
}

TEST_F(UceServiceTest, PageMessageReceived)
{
    IMS_TRACE_D("PageMessageReceived", 0, 0, 0);
    MockIPageMessage objMockIPageMessage;
    EXPECT_CALL(objMockIPageMessage, Reject).Times(1);
    EXPECT_CALL(objMockIPageMessage, Destroy).Times(1);
    pUceService->pageMessageReceived(&objMockICoreService, &objMockIPageMessage);
}

TEST_F(UceServiceTest, ReferenceReceived)
{
    IMS_TRACE_D("ReferenceReceived", 0, 0, 0);
    MockIReference objMockIReference;
    EXPECT_CALL(objMockIReference, RejectEx).Times(1);
    EXPECT_CALL(objMockIReference, Destroy).Times(1);
    pUceService->referenceReceived(&objMockICoreService, &objMockIReference);
}

TEST_F(UceServiceTest, ServiceClosed)
{
    IMS_TRACE_D("ServiceClosed", 0, 0, 0);
    EXPECT_CALL(objMockICoreService, GetFeatureCaps).Times(0);
    pUceService->serviceClosed(&objMockICoreService, IMS_NULL);
}

TEST_F(UceServiceTest, SessionInvitationReceived)
{
    IMS_TRACE_D("SessionInvitationReceived", 0, 0, 0);
    MockISession objMockISession;
    EXPECT_CALL(objMockISession, RejectEx).Times(1);
    EXPECT_CALL(objMockISession, Destroy).Times(1);
    pUceService->sessionInvitationReceived(&objMockICoreService, &objMockISession);
}

TEST_F(UceServiceTest, UnsolicitedNotifyReceived)
{
    IMS_TRACE_D("UnsolicitedNotifyReceived", 0, 0, 0);
    MockIMessage objMockIMessage;
    EXPECT_CALL(objMockIMessage, CreateBodyPart).Times(0);
    pUceService->unsolicitedNotifyReceived(&objMockICoreService, &objMockIMessage);
}

TEST_F(UceServiceTest, CapabilityQueryReceived)
{
    IMS_TRACE_D("CapabilityQueryReceived", 0, 0, 0);
    MockICoreService objOtherMockICoreService;
    MockICapabilities objMockICapabilities;
    EXPECT_CALL(objMockICapabilities, Destroy).Times(0);
    pUceService->capabilityQueryReceived(&objOtherMockICoreService, &objMockICapabilities);
}

TEST_F(UceServiceTest, SendPublishCmd)
{
    IMS_UINT32 key = 1;
    IMS_UINT32 extended = 1;
    IMS_UINT32 capability = 1;
    AString pidfXml = AString::ConstEmpty();
    AString eTag = AString::ConstEmpty();

    IMS_TRACE_D("SendPublishCmd", 0, 0, 0);
    pUceService->disableManagers();
    EXPECT_FALSE(pUceService->SendPublishCmd(key, extended, capability, pidfXml, eTag));

    pUceService->enableManagers();
    EXPECT_TRUE(pUceService->SendPublishCmd(key, extended, capability, pidfXml, eTag));
}

TEST_F(UceServiceTest, SendOptionsCmd)
{
    IMS_UINT32 key = 1;
    IMS_UINT32 myCaps = 1;
    AString remoteUri = AString::ConstEmpty();

    IMS_TRACE_D("SendOptionsCmd", 0, 0, 0);
    pUceService->disableManagers();
    EXPECT_FALSE(pUceService->SendOptionsCmd(key, myCaps, remoteUri));

    pUceService->enableManagers();
    EXPECT_TRUE(pUceService->SendOptionsCmd(key, myCaps, remoteUri));
}

TEST_F(UceServiceTest, SendOptionsRespCmd)
{
    IMS_UINT32 key = 1;
    IMS_SINT32 responseCode = 200;
    AString reason = AString::ConstEmpty();
    IMS_UINT32 myCaps = 1;

    IMS_TRACE_D("SendOptionsRespCmd", 0, 0, 0);
    pUceService->disableManagers();
    EXPECT_FALSE(pUceService->SendOptionsRespCmd(key, responseCode, reason, myCaps));

    pUceService->enableManagers();
    EXPECT_TRUE(pUceService->SendOptionsRespCmd(key, responseCode, reason, myCaps));
}

TEST_F(UceServiceTest, SendSingleSubscribeCmd)
{
    IMS_UINT32 key = 1;
    AString user = AString::ConstEmpty();

    IMS_TRACE_D("SendSingleSubscribeCmd", 0, 0, 0);

    pUceService->disableManagers();
    EXPECT_FALSE(pUceService->SendSingleSubscribeCmd(key, user));

    pUceService->enableManagers();
    EXPECT_TRUE(pUceService->SendSingleSubscribeCmd(key, user));
}

TEST_F(UceServiceTest, SendListSubscribeCmd)
{
    IMS_UINT32 key = 1;
    ImsList<AString> userList;

    IMS_TRACE_D("SendListSubscribeCmd", 0, 0, 0);
    pUceService->disableManagers();
    EXPECT_FALSE(pUceService->SendListSubscribeCmd(key, userList));

    pUceService->enableManagers();
    EXPECT_TRUE(pUceService->SendListSubscribeCmd(key, userList));
}