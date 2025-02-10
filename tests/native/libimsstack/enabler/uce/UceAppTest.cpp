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
#include "UceApp.h"
#include "IUce.h"
#include "MockIJniEnabler.h"
#include "MockIUceJniThread.h"
#include "MockIImsAos.h"
#include "MockIImsAosInfo.h"
#include "MockINetworkWatcher.h"
#include "MockITimer.h"
#include "PlatformContext.h"
#include "TestTimerService.h"
#include "JniEnablerConnector.h"
#include "AosAppRequestType.h"
#include "def/UceDef.h"
#include "UceService.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;

__IMS_TRACE_TAG_UCE__;

class TestUceApp : public UceApp
{
public:
    enum
    {
        AOS_CONNECTING,
        AOS_CONNECTED,
        AOS_DISCONNECTING,
        AOS_DISCONNECTED,
        AOS_SUSPENDED,
        AOS_RESUMED,
    };

    enum
    {
        AMSG_BASE = IMS_MSG_USER + 1,
        AMSG_CREATE_SERVICE,
        AMSG_MAX
    };

public:
    inline explicit TestUceApp(IImsAos* piImsAos) :
            UceApp(0, piImsAos)
    {
    }
    virtual ~TestUceApp() {}
    IMS_BOOL callPre(IMSMSG objUIMsg) { return OnPreprocess(objUIMsg); }
    IMS_BOOL sendMessage(IMSMSG& objMSG) { return OnMessage(objMSG); }
    IMS_BOOL callPost(IMSMSG objUIMsg) { return OnPostprocess(objUIMsg); }
    IMS_BOOL callControl() { return Control(0, 0, IMS_NULL); }
    void notifyNetworkStatus(INetworkWatcher* piNetWatcherInfo)
    {
        NetworkWatcher_NotifyStatus(piNetWatcherInfo);
    }
    void setNetworkWatcher(INetworkWatcher* piNetWatcherInfo)
    {
        m_piNetWatcherInfo = piNetWatcherInfo;
    }
    void setNetworkType(IMS_SINT32 network) { m_eCurrentNetwork = network; }
    IMS_SINT32 getNetworkType() const { return m_eCurrentNetwork; }
    void setAoSState(IMS_SINT32 state) { m_eAoSStatus = state; }
    IMS_SINT32 getAoSState() const { return m_eAoSStatus; }
    IMS_BOOL isTimerNull()
    {
        if (GetTimer() == IMS_NULL)
        {
            return IMS_TRUE;
        }
        return IMS_FALSE;
    }
    void setTimer(ITimer* piTimer) { m_piDeBounceTimer = piTimer; }
    void startTimer() { StartTimer(TIMER_NETWORK_CHANGED, 10000); }
    void stopTimer() { StopTimer(TIMER_NETWORK_CHANGED); }
    void clearTimer() { ClearTimer(); }
    void expiredTimer(ITimer* piTimer) { Timer_TimerExpired(piTimer); }
    void aosConnecting() { ImsAos_Connecting(); }
    void aosDisConnecting() { ImsAos_Disconnecting(0); }
    void aosDisConnected() { ImsAos_Disconnected(0); }
    void aosSuspend() { ImsAos_Suspended(0); }
    void aosResume() { ImsAos_Resumed(); }
    void aosMonitorConnected() { ImsAosMonitor_Connected(0, 0); }
    void registrationCheck() { ImsRegistrationCheck(); }
    IMS_UINT32 getService(IMS_UINT32 features) { return GetRegisteredService(features); }
    IMS_BOOL sendPublishCmd() { return SendPublishCmd(0, 0, 0, "", ""); }
    IMS_BOOL sendSingleSubscribeCmd() { return SendSingleSubscribeCmd(0, ""); }
    IMS_BOOL sendListSubscribeCmd()
    {
        const ImsList<AString> userList;
        return SendListSubscribeCmd(0, userList);
    }
    IMS_BOOL sendOptionsCmd() { return SendOptionsCmd(0, 0, ""); }
    IMS_BOOL sendOptionsRespCmd() { return SendOptionsRespCmd(0, 0, "", 0); }
    void setUceService(UceService* pUceService) { m_pUceService = pUceService; }
};

class UceAppTest : public ::testing::Test
{
public:
    inline UceAppTest() :
            objTimerService(),
            objTimer(objTimerService.GetMockTimer())
    {
        pUceApp = IMS_NULL;
    }
    TestTimerService objTimerService;
    MockITimer& objTimer;
    TestUceApp* pUceApp;
    MockIJniEnabler objMockJniEnabler;
    MockIUceJniThread objMockIUceJniThread;
    MockIImsAos objMockIImsAos;
    MockIImsAosInfo objMockIImsAosInfo;

protected:
    virtual void SetUp() override
    {
        pUceApp = new TestUceApp(&objMockIImsAos);
        ASSERT_TRUE(pUceApp != nullptr);
        ON_CALL(objMockJniEnabler, GetJniThread()).WillByDefault(Return(&objMockIUceJniThread));
        ON_CALL(objMockIImsAos, GetAosInfo()).WillByDefault(Return(&objMockIImsAosInfo));
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, &objMockJniEnabler);
        PlatformContext::GetInstance()->SetService(
                PlatformContext::SERVICE_TIMER, &objTimerService);
    }

    virtual void TearDown() override
    {
        if (pUceApp)
        {
            delete pUceApp;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_TIMER, IMS_NULL);
    }
};

TEST_F(UceAppTest, CallPre)
{
    IMS_TRACE_D("CallPre", 0, 0, 0);
    IMSMSG objMsg(AosAppRequest::COMMAND_SET_PUBLISH_STARTED, 0, 0);

    EXPECT_FALSE(pUceApp->callPre(objMsg));
}

TEST_F(UceAppTest, SendMessage)
{
    IMS_TRACE_D("SendMessage", 0, 0, 0);
    IMSMSG objMsg(TestUceApp::AMSG_CREATE_SERVICE, 0, 0);
    EXPECT_TRUE(pUceApp->sendMessage(objMsg));

    IMSMSG objMsg2(AosAppRequest::COMMAND_SET_PUBLISH_TERMINATED, 0, 0);
    EXPECT_TRUE(pUceApp->sendMessage(objMsg2));

    IMSMSG objMsg3(AosAppRequest::COMMAND_REGISTER_RECOVERY, 0, 0);
    EXPECT_TRUE(pUceApp->sendMessage(objMsg3));
}

TEST_F(UceAppTest, NotifyPublishState)
{
    IMS_TRACE_D("NotifyPublishState", 0, 0, 0);
    EXPECT_CALL(objMockIImsAosInfo, NotifyPublishState(_)).Times(1);

    IMSMSG objMsg(AosAppRequest::COMMAND_SET_PUBLISH_STARTED, 0, 0);
    EXPECT_TRUE(pUceApp->sendMessage(objMsg));
}

TEST_F(UceAppTest, AoSControl)
{
    IMS_TRACE_D("NotifyPublishState", 0, 0, 0);
    EXPECT_CALL(objMockIImsAos, Control(_)).Times(1);
    pUceApp->setNetworkType(eUCE_RAT_GERAN);
    IMSMSG objMsg(AosAppRequest::COMMAND_REGISTER_RECOVERY, 0, ImsAosControl::REGISTER_REINITIATE);
    EXPECT_TRUE(pUceApp->sendMessage(objMsg));
}

TEST_F(UceAppTest, CallPost)
{
    IMS_TRACE_D("CallPost", 0, 0, 0);
    IMSMSG objMsg(AosAppRequest::COMMAND_SET_PUBLISH_STARTED, 0, 0);

    EXPECT_TRUE(pUceApp->callPost(objMsg));
}

TEST_F(UceAppTest, CallControl)
{
    IMS_TRACE_D("CallControl", 0, 0, 0);
    EXPECT_FALSE(pUceApp->callControl());
}

TEST_F(UceAppTest, NotifyNetworkStatus)
{
    IMS_TRACE_D("NotifyNetworkStatus", 0, 0, 0);
    MockINetworkWatcher objMockINetworkWatcher;
    ON_CALL(objMockINetworkWatcher, GetNetworkType())
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_UMTS));

    pUceApp->setNetworkWatcher(&objMockINetworkWatcher);
    pUceApp->notifyNetworkStatus(&objMockINetworkWatcher);
    EXPECT_EQ(pUceApp->getNetworkType(), eUCE_RAT_UTRAN);
    pUceApp->setNetworkWatcher(IMS_NULL);
}

TEST_F(UceAppTest, NotifyNetworkStatusWithNoMatched)
{
    IMS_TRACE_D("NotifyNetworkStatusWithNoMatched", 0, 0, 0);
    MockINetworkWatcher objMockINetworkWatcher;
    pUceApp->notifyNetworkStatus(&objMockINetworkWatcher);
    EXPECT_EQ(pUceApp->getNetworkType(), eUCE_RAT_INVALID);
}

TEST_F(UceAppTest, TimerExpired)
{
    MockINetworkWatcher objMockINetworkWatcher;
    ON_CALL(objMockINetworkWatcher, GetNetworkType())
            .WillByDefault(Return(INetworkWatcher::RADIOTECH_TYPE_EHRPD));

    EXPECT_CALL(objTimer, KillTimer).Times(1);
    EXPECT_CALL(objMockIUceJniThread, NotifyNetworkChanged(_)).Times(1);

    pUceApp->setNetworkWatcher(&objMockINetworkWatcher);
    pUceApp->setTimer(&objTimer);
    pUceApp->expiredTimer(&objTimer);
    pUceApp->setNetworkWatcher(IMS_NULL);
}

TEST_F(UceAppTest, StopTimer)
{
    IMS_TRACE_D("StopTimer", 0, 0, 0);
    EXPECT_TRUE(pUceApp->isTimerNull());
    pUceApp->startTimer();
    EXPECT_FALSE(pUceApp->isTimerNull());
    pUceApp->stopTimer();
    EXPECT_TRUE(pUceApp->isTimerNull());
}

TEST_F(UceAppTest, ClearTimer)
{
    IMS_TRACE_D("ClearTimer", 0, 0, 0);
    EXPECT_TRUE(pUceApp->isTimerNull());
    pUceApp->startTimer();
    EXPECT_FALSE(pUceApp->isTimerNull());
    pUceApp->clearTimer();
    EXPECT_TRUE(pUceApp->isTimerNull());
}

TEST_F(UceAppTest, AoSConnecting)
{
    IMS_TRACE_D("AoSConnecting", 0, 0, 0);
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosConnecting();
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_CONNECTING);
}

TEST_F(UceAppTest, AoSDisConnecting)
{
    IMS_TRACE_D("AoSDisConnecting", 0, 0, 0);
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTED);
    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    pUceApp->aosDisConnecting();
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTING);
    pUceApp->aosDisConnecting();
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTING);
}

TEST_F(UceAppTest, AoSDisConnected)
{
    IMS_TRACE_D("AoSDisConnected", 0, 0, 0);
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->setAoSState(TestUceApp::AOS_CONNECTED);
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_CONNECTED);
    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    pUceApp->aosDisConnected();
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosDisConnected();
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTED);
}

TEST_F(UceAppTest, NotifyImsDeregistered)
{
    IMS_TRACE_D("NotifyImsDeregistered", 0, 0, 0);
    pUceApp->setAoSState(TestUceApp::AOS_CONNECTED);

    EXPECT_CALL(objMockIUceJniThread, NotifyImsDeregistered()).Times(1);
    pUceApp->aosDisConnected();
}

TEST_F(UceAppTest, NotNotifyImsDeregistered)
{
    IMS_TRACE_D("NotNotifyImsDeregistered", 0, 0, 0);
    ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(nullptr));
    pUceApp->setAoSState(TestUceApp::AOS_CONNECTED);

    EXPECT_CALL(objMockIUceJniThread, NotifyImsDeregistered()).Times(0);
    pUceApp->aosDisConnected();
}

TEST_F(UceAppTest, AoSSuspend)
{
    IMS_TRACE_D("AoSSuspend", 0, 0, 0);
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosSuspend();
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_SUSPENDED);
}

TEST_F(UceAppTest, AoSResume)
{
    IMS_TRACE_D("AoSResume", 0, 0, 0);
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosResume();
    EXPECT_EQ(pUceApp->getAoSState(), TestUceApp::AOS_RESUMED);
}

TEST_F(UceAppTest, AoSMonitorConnected)
{
    IMS_TRACE_D("AoSMonitorConnected", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, NotifyImsRegistered(_, _)).Times(1);
    pUceApp->aosMonitorConnected();

    pUceApp->setAoSState(TestUceApp::AOS_CONNECTED);

    EXPECT_CALL(objMockIUceJniThread, NotifyImsRegiRefreshed(_)).Times(1);
    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    pUceApp->aosMonitorConnected();
}

TEST_F(UceAppTest, AoSMonitorConnectedWithNullJniThread)
{
    IMS_TRACE_D("AoSMonitorConnectedWithNullJniThread", 0, 0, 0);
    ON_CALL(objMockJniEnabler, GetJniThread).WillByDefault(Return(nullptr));
    EXPECT_CALL(objMockIUceJniThread, NotifyImsRegistered(_, _)).Times(0);
    pUceApp->aosMonitorConnected();
}

TEST_F(UceAppTest, RegistrationCheck)
{
    IMS_TRACE_D("RegistrationCheck", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, NotifyImsDeregistered()).Times(1);
    pUceApp->registrationCheck();

    pUceApp->setAoSState(TestUceApp::AOS_CONNECTED);

    EXPECT_CALL(objMockIUceJniThread, NotifyImsRegistered(_, _)).Times(1);
    pUceApp->registrationCheck();
}

TEST_F(UceAppTest, SendPublishCmd)
{
    IMS_TRACE_D("SendPublishCmd", 0, 0, 0);
    EXPECT_FALSE(pUceApp->sendPublishCmd());

    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    EXPECT_TRUE(pUceApp->sendPublishCmd());
}

TEST_F(UceAppTest, SendSingleSubscribeCmd)
{
    IMS_TRACE_D("SendSingleSubscribeCmd", 0, 0, 0);
    EXPECT_FALSE(pUceApp->sendSingleSubscribeCmd());

    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    EXPECT_TRUE(pUceApp->sendSingleSubscribeCmd());
}

TEST_F(UceAppTest, SendListSubscribeCmd)
{
    IMS_TRACE_D("SendListSubscribeCmd", 0, 0, 0);
    EXPECT_FALSE(pUceApp->sendListSubscribeCmd());

    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    EXPECT_TRUE(pUceApp->sendListSubscribeCmd());
}

TEST_F(UceAppTest, SendOptionsCmd)
{
    IMS_TRACE_D("SendOptionsCmd", 0, 0, 0);
    EXPECT_FALSE(pUceApp->sendOptionsCmd());

    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    EXPECT_TRUE(pUceApp->sendOptionsCmd());
}

TEST_F(UceAppTest, SendOptionsRespCmd)
{
    IMS_TRACE_D("SendOptionsRespCmd", 0, 0, 0);
    EXPECT_FALSE(pUceApp->sendOptionsRespCmd());

    UceService* pUceService = new UceService(IMS_NULL);
    pUceApp->setUceService(pUceService);
    EXPECT_TRUE(pUceApp->sendOptionsRespCmd());
}

TEST_F(UceAppTest, GetService)
{
    IMS_TRACE_D("GetService", 0, 0, 0);
    IMS_UINT32 features = ImsAosFeature::VIDEO | ImsAosFeature::STANDALONE_MSG |
            ImsAosFeature::CHAT_SESSION | ImsAosFeature::FILE_TRANSFER |
            ImsAosFeature::FILE_TRANSFER_VIA_SMS | ImsAosFeature::CALL_COMPOSER_ENRICHED_CALLING |
            ImsAosFeature::GEO_PUSH | ImsAosFeature::GEO_PUSH_VIA_SMS |
            ImsAosFeature::CHATBOT_COMMUNICATION_USING_SESSION |
            ImsAosFeature::CHATBOT_COMMUNICATION_USING_STANDALONE_MSG |
            ImsAosFeature::CHATBOT_VERSION_SUPPORTED | ImsAosFeature::CHATBOT_VERSION_V2_SUPPORTED |
            ImsAosFeature::PRESENCE;
    IMS_UINT32 registeredService = 0;

    IMS_UINT32 expectedService = CONNECTED_SERVICE_VIDEO | CONNECTED_SERVICE_CPM_MSG |
            CONNECTED_SERVICE_CPM_LARGEMSG | CONNECTED_SERVICE_CPM_SESSION |
            CONNECTED_SERVICE_HTTPFT | CONNECTED_SERVICE_FTSMS | CONNECTED_SERVICE_CALL_COMPOSER |
            CONNECTED_SERVICE_GEOPUSH | CONNECTED_SERVICE_GEOSMS | CONNECTED_SERVICE_CHATBOT |
            CONNECTED_SERVICE_CHATBOT_STANDALONE_MSG | CONNECTED_SERVICE_CHATBOT_V1 |
            CONNECTED_SERVICE_CHATBOT_V2 | CONNECTED_SERVICE_PRESENCE;

    registeredService = pUceApp->getService(features);
    EXPECT_EQ(registeredService, expectedService);
}
