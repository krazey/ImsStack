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

#include "../../../engine/interface/core/MockICoreService.h"
#include "MockIMtcContext.h"
#include "MockIMtcService.h"
#include "call/IMtcCall.h"
#include "call/MockCallConnectionIdManager.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "conferencecall/ConferenceManager.h"
#include "conferencecall/IConferenceController.h"
#include "conferencecall/IConferenceManager.h"
#include "helper/MockICallStateProxy.h"
#include <gtest/gtest.h>

using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

class ConferenceManagerTest : public ::testing::Test
{
public:
    MockIMtcContext objMockContext;
    MockIMtcCallManager objMockCallManager;
    MockICallStateProxy objMockCallStateProxy;
    MockIMtcService objMtcService;
    MockICoreService objCoreService;
    MockIMtcCallContext objMockCallContext;
    MockCallConnectionIdManager* pIdManager;

    ConferenceManager* pConferenceManager;

protected:
    virtual void SetUp() override
    {
        ON_CALL(objMockContext, GetCallStateProxy).WillByDefault(ReturnRef(objMockCallStateProxy));
        pIdManager = new MockCallConnectionIdManager(objMockContext);
        ON_CALL(objMockContext, GetCallConnectionIdManager).WillByDefault(ReturnRef(*pIdManager));
        ON_CALL(objMockContext, GetCallManager).WillByDefault(ReturnRef(objMockCallManager));

        pConferenceManager = new ConferenceManager(objMockContext);
    }

    virtual void TearDown() override
    {
        delete pConferenceManager;
        delete pIdManager;
    }

    MockIMtcCall* CreateMockIMtcCall(IN CallKey nKey)
    {
        MockIMtcCall* pCall = new MockIMtcCall();
        ON_CALL(*pCall, GetKey()).WillByDefault(Return(nKey));
        ON_CALL(*pCall, GetCallContext()).WillByDefault(ReturnRef(objMockCallContext));

        ON_CALL(objMockCallManager, GetCallByCallKey(nKey)).WillByDefault(Return(pCall));

        return pCall;
    }

    IConferenceController* CreateConferenceController(IN CallKey nKey, IN ConferenceType eType)
    {
        ON_CALL(objMockCallContext, GetService()).WillByDefault(ReturnRef(objMtcService));
        ON_CALL(objMtcService, GetICoreService).WillByDefault(Return(&objCoreService));

        return &pConferenceManager->CreateController(nKey, eType);
    }
};

TEST_F(ConferenceManagerTest, CreateControllerWithDifferentCallKeyNotNull)
{
    const CallKey CONF_CALL_KEY1 = 1;
    const CallKey CONF_CALL_KEY2 = 2;

    MockIMtcCall* pCall1 = CreateMockIMtcCall(CONF_CALL_KEY1);
    MockIMtcCall* pCall2 = CreateMockIMtcCall(CONF_CALL_KEY2);

    IConferenceController* piController1 =
            CreateConferenceController(CONF_CALL_KEY1, ConferenceType::MERGE_CALL);
    IConferenceController* piController2 =
            CreateConferenceController(CONF_CALL_KEY2, ConferenceType::MERGE_CALL);

    ASSERT_NE(piController1, nullptr);
    ASSERT_NE(piController2, nullptr);

    delete pCall1;
    delete pCall2;
}

TEST_F(ConferenceManagerTest, GetControllerWithDifferentCallKey)
{
    const CallKey CONF_CALL_KEY1 = 1;
    const CallKey CONF_CALL_KEY2 = 2;

    MockIMtcCall* pCall1 = CreateMockIMtcCall(CONF_CALL_KEY1);
    MockIMtcCall* pCall2 = CreateMockIMtcCall(CONF_CALL_KEY2);

    IConferenceController* piController1 =
            CreateConferenceController(CONF_CALL_KEY1, ConferenceType::MERGE_CALL);
    IConferenceController* piController2 =
            CreateConferenceController(CONF_CALL_KEY2, ConferenceType::MERGE_CALL);

    EXPECT_EQ(piController1, pConferenceManager->GetController(CONF_CALL_KEY1));
    EXPECT_EQ(piController2, pConferenceManager->GetController(CONF_CALL_KEY2));
    EXPECT_NE(piController1, piController2);

    delete pCall1;
    delete pCall2;
}

TEST_F(ConferenceManagerTest, OnClosedAndGetControllerReturnNull)
{
    const CallKey CONF_CALL_KEY = 1;

    MockIMtcCall* pCall = CreateMockIMtcCall(CONF_CALL_KEY);

    IConferenceController* piController =
            CreateConferenceController(CONF_CALL_KEY, ConferenceType::MERGE_CALL);

    ASSERT_TRUE(pConferenceManager->GetController(CONF_CALL_KEY) != nullptr);
    pConferenceManager->OnClosed(static_cast<ConferenceController*>(piController));
    ASSERT_TRUE(pConferenceManager->GetController(CONF_CALL_KEY) == nullptr);

    delete pCall;
}

}  // namespace android
