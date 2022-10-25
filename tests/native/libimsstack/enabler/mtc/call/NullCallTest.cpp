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
#include "CallReasonInfo.h"
#include "call/NullCall.h"

TEST(NullCallTest, NullCallDoesNothing)
{
    NullCall objCall;

    CallType eCallType = CallType::UNKNOWN;
    ImsMap<SuppType, SuppService*> objSuppServices;
    ImsList<ConfUser*> objUsers;
    ISession* piSession = nullptr;
    MediaInfo objMediaInfo;
    CallReasonInfo objReason(CODE_UNSPECIFIED);

    objCall.Attach();
    objCall.Start(eCallType, "", &objMediaInfo, objSuppServices);
    objCall.StartConference(eCallType, "", &objMediaInfo, objSuppServices, objUsers);
    objCall.StartConference(eCallType, "", objUsers);
    objCall.HandleIncoming(piSession);
    objCall.HandleUserAlert();
    objCall.Accept(eCallType, &objMediaInfo);
    objCall.Reject(objReason);
    objCall.Hold(&objMediaInfo);
    objCall.Resume(&objMediaInfo);
    objCall.AcceptResume(eCallType, &objMediaInfo);
    objCall.RejectResume(objReason);
    objCall.Update(eCallType, &objMediaInfo);
    objCall.AcceptUpdate(eCallType, &objMediaInfo);
    objCall.RejectUpdate(objReason);
    objCall.CancelUpdate(objReason);
    objCall.Terminate(objReason);
    objCall.SendDtmf("", 0);
    objCall.SendUssd("");
    objCall.GetCallContext();
}

TEST(NullCallTest, GetKeyReturnsInvalidKey)
{
    NullCall objCall;

    EXPECT_EQ(IMtcCall::CALL_KEY_INVALID, objCall.GetKey());
}

TEST(NullCallTest, GetCallTypeReturnsUnknownType)
{
    NullCall objCall;

    EXPECT_EQ(CallType::UNKNOWN, objCall.GetCallType());
}

TEST(NullCallTest, GetStateReturnsIdle)
{
    NullCall objCall;

    EXPECT_EQ(IMtcCall::State::IDLE, objCall.GetState());
}
