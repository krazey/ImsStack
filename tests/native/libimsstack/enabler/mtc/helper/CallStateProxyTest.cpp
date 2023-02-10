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

#include "ImsMessage.h"
#include "ImsTypeDef.h"
#include "MockIMtcCallStateListener.h"
#include "PlatformContext.h"
#include "TestThreadService.h"
#include "call/IMtcCall.h"
#include "call/MockIMtcCall.h"
#include "call/MockIMtcCallContext.h"
#include "call/MockIMtcCallManager.h"
#include "helper/CallStateProxy.h"
#include <gtest/gtest.h>
#include <vector>

// to use CallStateProxy::MESSAGE_ASYNC_NOTIFY = 0
#define MESSAGE_ASYNC_NOTIFY 0

LOCAL CallKey ANY_KEY = 1;
LOCAL IMtcCallStateListener::State ANY_STATE = IMtcCallStateListener::State::ESTABLISHED;
LOCAL IMtcCallStateListener::Type ANY_TYPE = IMtcCallStateListener::Type::VOIP;
LOCAL IMS_BOOL ANY_ECC_BOOL = IMS_FALSE;
LOCAL IMS_UINT32 ANY_REASON = 0;

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace android
{

MATCHER_P(IsSameMessage, message, "")
{
    return arg.nMSG == message.nMSG;
}

class CallStateProxyTest : public ::testing::Test
{
public:
    inline CallStateProxyTest() :
            pProxy(IMS_NULL),
            objManager(),
            objListener(),
            objMockThread(),
            pThreadService(new TestThreadService())
    {
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, pThreadService);
        pThreadService->SetThread(&objMockThread);

        // this must be created after SetService(SERVICE_THREAD)
        pProxy = new CallStateProxy(objManager);
    }

    inline virtual ~CallStateProxyTest()
    {
        delete pProxy;
        PlatformContext::GetInstance()->SetService(PlatformContext::SERVICE_THREAD, IMS_NULL);
        delete pThreadService;
    }

    void SetTotalCallStateBySingleCallState(IN IMtcCall::State eSingleCallState)
    {
        ImsList<IMtcCall*> objCalls;
        objCalls.Append(&objCall);
        ON_CALL(objManager, GetCalls).WillByDefault(Return(objCalls));

        ON_CALL(objCall, GetState).WillByDefault(Return(eSingleCallState));
    }

protected:
    CallStateProxy* pProxy;
    MockIMtcCallManager objManager;
    MockIMtcCallStateListener objListener;
    MockIMtcCall objCall;
    MockIThread objMockThread;
    TestThreadService* pThreadService;
};

TEST_F(CallStateProxyTest, GetControllerReturnsNull)
{
    ASSERT_EQ(pProxy->GetController(), nullptr);
}

TEST_F(CallStateProxyTest, UpdateCallStateNotifiesListenrSynchronously)
{
    ON_CALL(objListener, IsSynchronousCallRequired).WillByDefault(Return(IMS_TRUE));
    pProxy->AddListener(&objListener);
    // duplicated add
    pProxy->AddListener(&objListener);

    // duplicated add but only 1 time called.
    EXPECT_CALL(
            objListener, OnCallStateChanged(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON))
            .Times(1);

    SetTotalCallStateBySingleCallState(ANY_STATE);
    EXPECT_CALL(objListener, OnTotalCallStateChanged(ANY_STATE)).Times(1);

    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);
}

TEST_F(CallStateProxyTest, UpdateCallStateNotifiesListenrAsynchronously)
{
    ON_CALL(objListener, IsSynchronousCallRequired).WillByDefault(Return(IMS_FALSE));
    pProxy->AddListener(&objListener);
    pProxy->AddListener(&objListener);  // duplicated add

    SetTotalCallStateBySingleCallState(ANY_STATE);
    ImsMessage objMessage(MESSAGE_ASYNC_NOTIFY, 0, static_cast<IMS_UINTP>(IMS_TRUE));
    EXPECT_CALL(objMockThread, PostMessageI(IsSameMessage(objMessage)));

    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);
}

TEST_F(CallStateProxyTest, UpdateCallStateDoesNotNotifyIfAllListenersAreRemoved)
{
    ON_CALL(objListener, IsSynchronousCallRequired).WillByDefault(Return(IMS_FALSE));
    pProxy->AddListener(&objListener);
    pProxy->RemoveListener(&objListener);

    ON_CALL(objListener, IsSynchronousCallRequired).WillByDefault(Return(IMS_TRUE));
    pProxy->AddListener(&objListener);
    pProxy->RemoveListener(&objListener);

    EXPECT_CALL(objMockThread, PostMessageI(_)).Times(0);
    EXPECT_CALL(objListener, OnCallStateChanged(_, _, _, _, _)).Times(0);
    EXPECT_CALL(objListener, OnTotalCallStateChanged(_)).Times(0);

    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);
}

TEST_F(CallStateProxyTest, DispatchMessageNotifiesListenerSynchronouslyAndReturnsTrue)
{
    ON_CALL(objListener, IsSynchronousCallRequired).WillByDefault(Return(IMS_FALSE));
    pProxy->AddListener(&objListener);

    EXPECT_CALL(objListener,
            OnCallStateChanged(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON));
    SetTotalCallStateBySingleCallState(ANY_STATE);

    // m_eTotalState is the default value.
    EXPECT_CALL(objListener, OnTotalCallStateChanged(IMtcCallStateListener::State::IDLE));

    CallStateDetails* pDetails = new CallStateDetails(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL,
            ANY_REASON);  // deleted inside CallStateProxy
    ImsMessage objMessage(MESSAGE_ASYNC_NOTIFY, reinterpret_cast<IMS_UINTP>(pDetails),
            static_cast<IMS_UINTP>(IMS_TRUE));
    EXPECT_TRUE(pProxy->DispatchMessage(objMessage));
}

TEST_F(CallStateProxyTest, DispatchMessageOtherThanAsyncNotifyReturnsFalse)
{
    EXPECT_CALL(objListener, OnCallStateChanged(_, _, _, _, _)).Times(0);
    SetTotalCallStateBySingleCallState(ANY_STATE);
    EXPECT_CALL(objListener, OnTotalCallStateChanged(_)).Times(0);

    ImsMessage objMessage(1, 0, 0);
    EXPECT_FALSE(pProxy->DispatchMessage(objMessage));
}

TEST_F(CallStateProxyTest, UpdateTotalCallStateForAllCasesExceptIdleNotifiesConvertedState)
{
    ON_CALL(objListener, IsSynchronousCallRequired).WillByDefault(Return(IMS_TRUE));
    pProxy->AddListener(&objListener);

    // clang-format off
    std::vector<IMtcCallStateListener::State> objCallStates{
            IMtcCallStateListener::State::OUTGOING, IMtcCallStateListener::State::INCOMING,
            IMtcCallStateListener::State::ESTABLISHED};
    // clang-format on
    for (IMtcCallStateListener::State eCallState : objCallStates)
    {
        SetTotalCallStateBySingleCallState(eCallState);
        EXPECT_CALL(objListener, OnTotalCallStateChanged(eCallState));
        pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);
    }

    SetTotalCallStateBySingleCallState(IMtcCallStateListener::State::ALERTING);
    EXPECT_CALL(objListener, OnTotalCallStateChanged(IMtcCallStateListener::State::INCOMING));
    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);

    SetTotalCallStateBySingleCallState(IMtcCallStateListener::State::UPDATING);
    EXPECT_CALL(objListener, OnTotalCallStateChanged(IMtcCallStateListener::State::ESTABLISHED));
    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);

    SetTotalCallStateBySingleCallState(IMtcCallStateListener::State::TERMINATING);
    EXPECT_CALL(objListener, OnTotalCallStateChanged(IMtcCallStateListener::State::IDLE));
    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);
}

TEST_F(CallStateProxyTest, UpdateTotalCallStateChecksPeerTypeIfIdleState)
{
    ON_CALL(objListener, IsSynchronousCallRequired).WillByDefault(Return(IMS_TRUE));
    pProxy->AddListener(&objListener);

    MockIMtcCallContext objCallContext;
    ON_CALL(objCall, GetCallContext).WillByDefault(ReturnRef(objCallContext));
    CallInfo objCallInfo;
    ON_CALL(objCallContext, GetCallInfo()).WillByDefault(ReturnRef(objCallInfo));
    SetTotalCallStateBySingleCallState(IMtcCallStateListener::State::IDLE);

    objCallInfo.ePeerType = PeerType::MO;
    EXPECT_CALL(objListener, OnTotalCallStateChanged(IMtcCallStateListener::State::OUTGOING));
    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);

    objCallInfo.ePeerType = PeerType::MT;
    EXPECT_CALL(objListener, OnTotalCallStateChanged(IMtcCallStateListener::State::INCOMING));
    pProxy->UpdateCallState(ANY_KEY, ANY_STATE, ANY_TYPE, ANY_ECC_BOOL, ANY_REASON);
}

}  // namespace android
