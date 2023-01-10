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
#include "JniEnablerConnector.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

using ::testing::_;
using ::testing::Return;

__IMS_TRACE_TAG_USER_DECL__("UCE");

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

public:
    inline explicit TestUceApp(IImsAos* piImsAos) :
            UceApp(0, piImsAos)
    {
    }
    virtual ~TestUceApp() {}
    IMS_BOOL SendMessage(IMSMSG objUIMsg) { return OnMessage(objUIMsg); }
    void SetAoSState(IMS_SINT32 state) { m_eAoSStatus = state; }
    IMS_SINT32 GetAoSState() const { return m_eAoSStatus; }
    IMS_BOOL IsTimerNull()
    {
        if (GetTimer() == IMS_NULL)
        {
            return IMS_TRUE;
        }
        return IMS_FALSE;
    }
    void startTimer() { StartTimer(TIMER_NETWORK_CHANGED, 10000); }
    void stopTimer() { StopTimer(TIMER_NETWORK_CHANGED); }
    void clearTimer() { ClearTimer(); }
    void aosConnecting() { ImsAos_Connecting(); }
    void aosDisConnecting() { ImsAos_Disconnecting(0); }
    void aosDisConnected() { ImsAos_Disconnected(0); }
    void aosSuspend() { ImsAos_Suspended(0); }
    void aosResume() { ImsAos_Resumed(); }
    void aosMonitorConnected() { ImsAosMonitor_Connected(0, 0); }
    void registrationCheck() { ImsRegistrationCheck(); }

    void SendPublishCmd(IMS_UINT32 key, IMS_UINT32 extended, IMS_UINT32 capability,
            const AString& pidfXml, const AString& eTag) override
    {
        (void)key;
        (void)extended;
        (void)capability;
        (void)pidfXml;
        (void)eTag;
    }
    void SendSingleSubscribeCmd(IMS_UINT32 key, const AString& user) override
    {
        (void)key;
        (void)user;
    }
    void SendListSubscribeCmd(IMS_UINT32 key, const IMSList<AString>& userList) override
    {
        (void)key;
        (void)userList;
    }
    void SendOptionsCmd(IMS_UINT32 key, IMS_UINT32 myCaps, const AString& remoteUri) override
    {
        (void)key;
        (void)myCaps;
        (void)remoteUri;
    }
    void SendOptionsRespCmd(IMS_UINT32 key, IMS_SINT32 responseCode, const AString& reason,
            IMS_UINT32 myCaps) override
    {
        (void)key;
        (void)responseCode;
        (void)reason;
        (void)myCaps;
    }
};

class UceAppTest : public ::testing::Test
{
public:
    TestUceApp* pUceApp;
    MockIJniEnabler objMockJniEnabler;
    MockIUceJniThread objMockIUceJniThread;
    MockIImsAos objMockIImsAos;

protected:
    virtual void SetUp() override
    {
        pUceApp = new TestUceApp(&objMockIImsAos);
        ASSERT_TRUE(pUceApp != nullptr);
        ON_CALL(objMockJniEnabler, GetJniThread()).WillByDefault(Return(&objMockIUceJniThread));
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, &objMockJniEnabler);
    }

    virtual void TearDown() override
    {
        if (pUceApp)
        {
            delete pUceApp;
        }
        JniEnablerConnector::GetInstance().SetJniEnabler(0, EnablerType::UCE, IMS_NULL);
    }
};

TEST_F(UceAppTest, stopTimer)
{
    IMS_TRACE_D("stopTimer", 0, 0, 0);
    EXPECT_TRUE(pUceApp->IsTimerNull());
    pUceApp->startTimer();
    EXPECT_FALSE(pUceApp->IsTimerNull());
    pUceApp->stopTimer();
    EXPECT_TRUE(pUceApp->IsTimerNull());
}

TEST_F(UceAppTest, clearTimer)
{
    IMS_TRACE_D("clearTimer", 0, 0, 0);
    EXPECT_TRUE(pUceApp->IsTimerNull());
    pUceApp->startTimer();
    EXPECT_FALSE(pUceApp->IsTimerNull());
    pUceApp->clearTimer();
    EXPECT_TRUE(pUceApp->IsTimerNull());
}

TEST_F(UceAppTest, aosConnecting)
{
    IMS_TRACE_D("aosConnecting", 0, 0, 0);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosConnecting();
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_CONNECTING);
}

TEST_F(UceAppTest, aosDisConnecting)
{
    IMS_TRACE_D("aosDisConnecting", 0, 0, 0);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosDisConnecting();
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTING);
}

TEST_F(UceAppTest, aosDisConnected)
{
    IMS_TRACE_D("aosDisConnected", 0, 0, 0);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->SetAoSState(TestUceApp::AOS_CONNECTED);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_CONNECTED);
    pUceApp->aosDisConnected();
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
}

TEST_F(UceAppTest, NotifyImsDeregistered)
{
    IMS_TRACE_D("NotifyImsDeregistered", 0, 0, 0);
    pUceApp->SetAoSState(TestUceApp::AOS_CONNECTED);

    EXPECT_CALL(objMockIUceJniThread, NotifyImsDeregistered()).Times(1);
    pUceApp->aosDisConnected();
}

TEST_F(UceAppTest, aosSuspend)
{
    IMS_TRACE_D("aosSuspend", 0, 0, 0);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosSuspend();
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_SUSPENDED);
}

TEST_F(UceAppTest, aosResume)
{
    IMS_TRACE_D("aosResume", 0, 0, 0);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosResume();
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_RESUMED);
}

TEST_F(UceAppTest, aosMonitorConnected)
{
    IMS_TRACE_D("aosMonitorConnected", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, NotifyImsRegistered(_, _)).Times(1);
    pUceApp->aosMonitorConnected();

    pUceApp->SetAoSState(TestUceApp::AOS_CONNECTED);

    EXPECT_CALL(objMockIUceJniThread, NotifyImsRegiRefreshed(_)).Times(1);
    pUceApp->aosMonitorConnected();
}

TEST_F(UceAppTest, registrationCheck)
{
    IMS_TRACE_D("registrationCheck", 0, 0, 0);
    EXPECT_CALL(objMockIUceJniThread, NotifyImsDeregistered()).Times(1);
    pUceApp->registrationCheck();

    pUceApp->SetAoSState(TestUceApp::AOS_CONNECTED);

    EXPECT_CALL(objMockIUceJniThread, NotifyImsRegistered(_, _)).Times(1);
    pUceApp->registrationCheck();
}