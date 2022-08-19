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

extern SIP_BOOL Sip_Cbk_FetchTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);

extern SIP_BOOL Sip_Cbk_ReleaseTransaction(IN SIP_VOID* pvTxnKey, IN SIP_INT32 nOption,
        OUT SIP_VOID** ppvOutTxnKey, OUT SIP_VOID** ppvTxn);

extern SIP_BOOL Sip_Cbk_StartTimer(IN SIP_UINT32 nDuration, IN SipTimerCallback pfnTimerCallback,
        IN SIP_VOID* pvData, IN SIP_VOID** ppvHandle);

extern SIP_BOOL Sip_Cbk_StopTimer(IN SIP_VOID* pvHandle, IN SIP_VOID** ppvData);

extern SIP_VOID Sip_Cbk_OnTimerExpired(IN ISipUserData* pUserData, IN SIP_INT32 enTimerType);

extern SIP_VOID* Sip_Cbk_CreateAckRequest(IN SIP_VOID* pvRespMsg, IN ISipUserData* pUserData);

extern SIP_VOID Sip_Cbk_PreProcessMessageSentByStack(
        IN SIP_VOID* pvSipMsg, IN ISipUserData* pUserData);

extern SIP_VOID Sip_Cbk_PostProcessMessageSentByStack(IN SIP_VOID* pvSipMsg, IN SIP_CHAR* pBuffer,
        IN SIP_UINT32 nBufferLen, IN ISipUserData* pUserData);

extern SIP_VOID Sip_Cbk_DisplayTxnKey(IN SIP_VOID* pvTxnKey);

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
    EXPECT_EQ(SIP_FALSE, Sip_Cbk_FetchTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE, Sip_Cbk_ReleaseTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE, Sip_Cbk_StartTimer(0, nullptr, nullptr, nullptr));
    EXPECT_EQ(SIP_FALSE, Sip_Cbk_StopTimer(nullptr, nullptr));
    Sip_Cbk_CreateAckRequest(nullptr, nullptr);
    Sip_Cbk_PreProcessMessageSentByStack(nullptr, nullptr);
    Sip_Cbk_PostProcessMessageSentByStack(nullptr, nullptr, 0, nullptr);
    Sip_Cbk_DisplayTxnKey(nullptr);
    Sip_Cbk_OnTimerExpired(nullptr, 0);
    Sip_Cbk_DisplayTxnKey(nullptr);

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

    EXPECT_EQ(SIP_TRUE, Sip_Cbk_FetchTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_TRUE, Sip_Cbk_ReleaseTransaction(nullptr, 0, nullptr, nullptr));
    EXPECT_EQ(SIP_TRUE, Sip_Cbk_StartTimer(0, nullptr, nullptr, nullptr));
    EXPECT_EQ(SIP_TRUE, Sip_Cbk_StopTimer(nullptr, nullptr));
    Sip_Cbk_CreateAckRequest(nullptr, nullptr);
    Sip_Cbk_PreProcessMessageSentByStack(nullptr, nullptr);
    Sip_Cbk_PostProcessMessageSentByStack(nullptr, nullptr, 0, nullptr);
    Sip_Cbk_DisplayTxnKey(nullptr);
    Sip_Cbk_OnTimerExpired(nullptr, 0);
    Sip_Cbk_DisplayTxnKey(nullptr);
}

}  // namespace android