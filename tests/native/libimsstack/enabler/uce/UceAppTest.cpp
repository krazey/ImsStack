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
#include "IUUceService.h"

#include "ServiceMessage.h"
#include "ServiceTimer.h"
#include "ServiceTrace.h"

__IMS_TRACE_TAG_USER_DECL__("UCE");

using ::testing::AnyNumber;
using ::testing::Return;

IMS_SINT32 APP_SIM_SLOT = 20;

class TestUceApp : public UceApp
{
public:
    enum
    {
        AOS_CONNECTED,
        AOS_DISCONNECTING,
        AOS_DISCONNECTED,
        AOS_SUSPENDED,
        AOS_RESUMED,
    };

public:
    TestUceApp() :
            UceApp(APP_SIM_SLOT)
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
    void aosDisConnecting() { ImsAos_Disconnecting(0); }
    void aosDisConnected() { ImsAos_Disconnected(0); }
};

class UceAppTest : public ::testing::Test
{
public:
    TestUceApp* pUceApp;

protected:
    virtual void SetUp() override
    {
        pUceApp = new TestUceApp();
        ASSERT_TRUE(pUceApp != nullptr);
    }

    virtual void TearDown() override
    {
        if (pUceApp)
        {
            delete pUceApp;
        }
    }
};

TEST_F(UceAppTest, stopTimer)
{
    IMS_TRACE_D("stopTimer", 0, 0, 0);
    EXPECT_EQ(pUceApp->IsTimerNull(), IMS_TRUE);
    pUceApp->startTimer();
    EXPECT_EQ(pUceApp->IsTimerNull(), IMS_FALSE);
    pUceApp->stopTimer();
    EXPECT_EQ(pUceApp->IsTimerNull(), IMS_TRUE);
}

TEST_F(UceAppTest, clearTimer)
{
    IMS_TRACE_D("clearTimer", 0, 0, 0);
    EXPECT_EQ(pUceApp->IsTimerNull(), IMS_TRUE);
    pUceApp->startTimer();
    EXPECT_EQ(pUceApp->IsTimerNull(), IMS_FALSE);
    pUceApp->clearTimer();
    EXPECT_EQ(pUceApp->IsTimerNull(), IMS_TRUE);
}

TEST_F(UceAppTest, Aos_Disconnecting)
{
    IMS_TRACE_D("Aos_Disconnecting", 0, 0, 0);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->aosDisConnecting();
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTING);
}

TEST_F(UceAppTest, Aos_Disconnected)
{
    IMS_TRACE_D("Aos_Disconnecting", 0, 0, 0);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
    pUceApp->SetAoSState(TestUceApp::AOS_CONNECTED);
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_CONNECTED);
    pUceApp->aosDisConnected();
    EXPECT_EQ(pUceApp->GetAoSState(), TestUceApp::AOS_DISCONNECTED);
}