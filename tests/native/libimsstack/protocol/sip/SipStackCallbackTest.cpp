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

#include "SipStackCallback.h"

extern SIP_BOOL sip_cbk_fetchTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);

extern SIP_BOOL sip_cbk_releaseTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);

extern SIP_BOOL sip_cbk_startTimer(IN SIP_UINT32 nDuration, IN SipTimerCallback pfnTimerCallback,
        IN SIP_VOID* pvData, IN SIP_VOID** ppvHandle);

extern SIP_BOOL sip_cbk_stopTimer(IN SIP_VOID* pvHandle, IN SIP_VOID** ppvData);

extern SIP_VOID sip_cbk_onTimerExpired(IN ISipUserData* pUserData, IN SIP_INT32 enTimerType);

extern SIP_VOID* sip_cbk_createAckRequest(IN SIP_VOID* pvRespMsg, IN ISipUserData* pUserData);

extern SIP_VOID sip_cbk_preProcessMessageSentByStack(
        IN SIP_VOID* pvSipMsg, IN ISipUserData* pUserData);

extern SIP_VOID sip_cbk_postProcessMessageSentByStack(IN SIP_VOID* pvSipMsg, IN SIP_CHAR* pBuffer,
        IN SIP_UINT32 nBufferLen, IN ISipUserData* pUserData);

extern SIP_VOID sip_cbk_displayTxnKey(IN SIP_VOID* pvTxnKey);

namespace android
{
SIP_BOOL FetchTransaction_callbackStub(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**)
{
    return SIP_TRUE;
}

SIP_BOOL ReleaseTransaction_callbackStub(SIP_VOID*, SIP_INT32, SIP_VOID**, SIP_VOID**)
{
    return SIP_TRUE;
}

SIP_BOOL StartTimer_callbackStub(SIP_UINT32, SipTimerCallback, SIP_VOID*, SIP_VOID**)
{
    return SIP_TRUE;
}

SIP_BOOL StopTimer_callbackStub(IN SIP_VOID*, OUT SIP_VOID**)
{
    return SIP_TRUE;
}

SIP_VOID* CreateAckRequest_callbackStub(SIP_VOID*, ISipUserData*)
{
    return SIP_NULL;
}

SIP_VOID PreprocessMessage_callbackStub(SIP_VOID*, ISipUserData*) {}

SIP_VOID PostprocessMessage_callbackStub(
        IN SIP_VOID*, IN SIP_CHAR*, IN SIP_UINT32, IN ISipUserData*)
{
}

SIP_VOID DisplayTxnKey_callbackStub(IN SIP_VOID*) {}

SIP_VOID OnTimerExpired_callbackStub(IN ISipUserData*, IN SIP_INT32) {}

class SipStackCallbackTest : public ::testing::Test
{
public:
protected:
    virtual void SetUp() override {}

    virtual void TearDown() override {}
};

TEST_F(SipStackCallbackTest, CheckCallbacks)
{
    EXPECT_EQ(SIP_FALSE, sip_cbk_fetchTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE, sip_cbk_releaseTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE, sip_cbk_startTimer(0, nullptr, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE, sip_cbk_stopTimer(nullptr, nullptr));
    sip_cbk_createAckRequest(nullptr, nullptr);
    sip_cbk_preProcessMessageSentByStack(nullptr, nullptr);
    sip_cbk_postProcessMessageSentByStack(nullptr, nullptr, 0, nullptr);
    sip_cbk_displayTxnKey(nullptr);
    sip_cbk_onTimerExpired(nullptr, 0);
    sip_cbk_displayTxnKey(nullptr);

    // clang-format off
    SipStackCallbacks stCallbacks = {
            &FetchTransaction_callbackStub,
            &ReleaseTransaction_callbackStub,
            &StartTimer_callbackStub,
            &StopTimer_callbackStub,
            &OnTimerExpired_callbackStub,
            &CreateAckRequest_callbackStub,
            &PreprocessMessage_callbackStub,
            &PostprocessMessage_callbackStub,
            &DisplayTxnKey_callbackStub,
        };
    // clang-format on
    SipStackCallback_SetCallbacks(stCallbacks);

    EXPECT_EQ(SIP_TRUE, sip_cbk_fetchTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_TRUE, sip_cbk_releaseTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_TRUE, sip_cbk_startTimer(0, nullptr, nullptr, nullptr));
    EXPECT_EQ(SIP_TRUE, sip_cbk_stopTimer(nullptr, nullptr));
    sip_cbk_createAckRequest(nullptr, nullptr);
    sip_cbk_preProcessMessageSentByStack(nullptr, nullptr);
    sip_cbk_postProcessMessageSentByStack(nullptr, nullptr, 0, nullptr);
    sip_cbk_displayTxnKey(nullptr);
    sip_cbk_onTimerExpired(nullptr, 0);
    sip_cbk_displayTxnKey(nullptr);
}

}  // namespace android