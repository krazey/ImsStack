/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except compliance with the License.
 * You may obtaa copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_MTC_SESSION_H_
#define MOCK_MTC_SESSION_H_

#include <gmock/gmock.h>
#include "ImsTypeDef.h"
#include "call/MtcSession.h"

class ISession;
class IMtcCallContext;
enum class CallType;

class MockMtcSession : public MtcSession
{
public:
    explicit MockMtcSession(
            IN IMtcCallContext& objContext, IN ISession& objSession, IN CallType eCallType) :
            MtcSession(objContext, objSession, eCallType)
    {
    }
    virtual ~MockMtcSession() {}

    MOCK_METHOD(ISession&, GetISession, (), (override));
};

#endif
