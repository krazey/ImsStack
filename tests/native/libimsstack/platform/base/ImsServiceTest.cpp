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
 * WITHout WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>

#include "ImsService.h"
#include "ImsMessageDef.h"

enum
{
    STATE_NOTREADY = 0,
    STATE_READY
};

namespace android
{

class TestImsActivity : public ImsActivity
{
public:
    inline TestImsActivity() :
            ImsActivity()
    {
    }
    inline ~TestImsActivity() {}

    IMS_BOOL DispatchMessage(IN ImsMessage& /*objMsg*/) { return IMS_TRUE; }
};

class TestImsService : public ImsService
{
    DECLARE_STATE_MAP()

    DECLARE_STATE_MSG_MAP(STATE_NOTREADY)
    DECLARE_STATE_MSG_MAP(STATE_READY)

public:
    inline TestImsService() :
            ImsService(AString::ConstNull())
    {
    }
    inline ~TestImsService() {}

    IMS_BOOL TestDispatchMsg(IN ImsMessage& /*objMsg*/) { return IMS_TRUE; }

    IMS_BOOL TestSetState(IN IMS_UINT32 nState) { return SetState(nState); }

    IMS_UINT32 TestGetState() { return GetState(); }

    IMS_UINT32 TestGetOldState() { return GetOldState(); }
};

BEGIN_STATE_MAP(TestImsService)
STATE_ENTRY(STATE_NOTREADY)
STATE_ENTRY(STATE_READY)
END_STATE_MAP()

BEGIN_STATE_MSG_MAP(TestImsService, STATE_NOTREADY)
STATE_MSG_ENTRY(1, &TestImsService::TestDispatchMsg)
STATE_MSG_ENTRY(2, &TestImsService::TestDispatchMsg)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(TestImsService, STATE_READY)
STATE_MSG_ENTRY(1, &TestImsService::TestDispatchMsg)
STATE_MSG_ENTRY(2, &TestImsService::TestDispatchMsg)
END_STATE_MSG_MAP()

class ImsServiceTest : public ::testing::Test
{
public:
    TestImsService* m_pService;

protected:
    virtual void SetUp() override
    {
        m_pService = new TestImsService();
        ASSERT_TRUE(m_pService != nullptr);
    }

    virtual void TearDown() override
    {
        if (m_pService != IMS_NULL)
        {
            delete m_pService;
            m_pService = IMS_NULL;
        }
    }
};

TEST_F(ImsServiceTest, StateTest)
{
    EXPECT_EQ(m_pService->TestGetOldState(), IMS_INVALID_STATE);
    EXPECT_EQ(m_pService->TestGetState(), IMS_INVALID_STATE);
    EXPECT_EQ(m_pService->TestSetState(STATE_NOTREADY), IMS_TRUE);
    EXPECT_EQ(m_pService->TestGetOldState(), IMS_INVALID_STATE);
    EXPECT_EQ(m_pService->TestGetState(), STATE_NOTREADY);
}

TEST_F(ImsServiceTest, DispatchMessage)
{
    TestImsActivity* pTestActivity = DYNAMIC_CAST(TestImsActivity*, m_pService);
    ImsMessage objMsg(0, 0, 0);
    EXPECT_EQ(pTestActivity->DispatchMessage(objMsg), IMS_FALSE);
    EXPECT_EQ(m_pService->TestSetState(STATE_NOTREADY), IMS_TRUE);

    ImsMessage objMsg1(1, 0, 0);
    EXPECT_EQ(pTestActivity->DispatchMessage(objMsg1), IMS_FALSE);
    EXPECT_EQ(m_pService->TestSetState(STATE_READY), IMS_TRUE);
    EXPECT_EQ(m_pService->TestGetOldState(), STATE_NOTREADY);

    ImsMessage objMsg2(2, 0, 0);
    EXPECT_EQ(pTestActivity->DispatchMessage(objMsg2), IMS_FALSE);
}

}  // namespace android
