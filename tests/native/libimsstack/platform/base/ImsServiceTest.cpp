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

namespace android
{

class TestImsService : public ImsService
{
    DECLARE_STATE_MAP()

    DECLARE_STATE_MSG_MAP(STATE_NOTREADY)
    DECLARE_STATE_MSG_MAP(STATE_READY)

public:
    inline TestImsService() :
            ImsService(AString::ConstNull())
    {
        SetState(STATE_NOTREADY);
    }
    inline ~TestImsService() {}

    inline IMS_BOOL StateNotReady_OnMessage(IN ImsMessage& objMsg)
    {
        if (objMsg.GetName() == MSG1)
        {
            SetState(STATE_READY);
        }
        return IMS_TRUE;
    }

    inline IMS_BOOL StateReady_OnMessage(IN ImsMessage& objMsg)
    {
        if (objMsg.GetName() == MSG2)
        {
            SetState(STATE_NOTREADY);
        }
        return IMS_TRUE;
    }

    inline IMS_BOOL TestDispatchMessage(IN ImsMessage& objMsg) { return DispatchMessage(objMsg); }
    inline IMS_UINT32 TestGetState() { return GetState(); }
    inline IMS_UINT32 TestGetOldState() { return GetOldState(); }

public:
    enum
    {
        STATE_NOTREADY = 0,
        STATE_READY
    };

    static constexpr IMS_SINT32 MSG1 = 1;
    static constexpr IMS_SINT32 MSG2 = 2;
};

BEGIN_STATE_MAP(TestImsService)
STATE_ENTRY(STATE_NOTREADY)
STATE_ENTRY(STATE_READY)
END_STATE_MAP()

BEGIN_STATE_MSG_MAP(TestImsService, STATE_NOTREADY)
STATE_MSG_ENTRY(MSG1, &TestImsService::StateNotReady_OnMessage)
STATE_MSG_ENTRY(MSG2, &TestImsService::StateNotReady_OnMessage)
END_STATE_MSG_MAP()

BEGIN_STATE_MSG_MAP(TestImsService, STATE_READY)
STATE_MSG_ENTRY(MSG1, &TestImsService::StateReady_OnMessage)
STATE_MSG_ENTRY(MSG2, &TestImsService::StateReady_OnMessage)
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

TEST_F(ImsServiceTest, DispatchMessage)
{
    ImsMessage objMsg(IMS_INVALID_MSG, 0, 0);
    EXPECT_FALSE(m_pService->TestDispatchMessage(objMsg));
    EXPECT_EQ(m_pService->TestGetState(), TestImsService::STATE_NOTREADY);
    EXPECT_EQ(m_pService->TestGetOldState(), IMS_INVALID_STATE);

    ImsMessage objMsg1(TestImsService::MSG1, 0, 0);
    EXPECT_TRUE(m_pService->TestDispatchMessage(objMsg1));
    EXPECT_EQ(m_pService->TestGetState(), TestImsService::STATE_READY);
    EXPECT_EQ(m_pService->TestGetOldState(), TestImsService::STATE_NOTREADY);

    // Same state.
    EXPECT_TRUE(m_pService->TestDispatchMessage(objMsg1));
    EXPECT_EQ(m_pService->TestGetState(), TestImsService::STATE_READY);
    EXPECT_EQ(m_pService->TestGetOldState(), TestImsService::STATE_NOTREADY);

    ImsMessage objMsg2(TestImsService::MSG2, 0, 0);
    EXPECT_TRUE(m_pService->TestDispatchMessage(objMsg2));
    EXPECT_EQ(m_pService->TestGetState(), TestImsService::STATE_NOTREADY);
    EXPECT_EQ(m_pService->TestGetOldState(), TestImsService::STATE_READY);

    // Same state.
    EXPECT_TRUE(m_pService->TestDispatchMessage(objMsg2));
    EXPECT_EQ(m_pService->TestGetState(), TestImsService::STATE_NOTREADY);
    EXPECT_EQ(m_pService->TestGetOldState(), TestImsService::STATE_READY);
}

}  // namespace android
