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

#ifndef MOCK_I_MTC_SESSION_H_
#define MOCK_I_MTC_SESSION_H_

#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/IMtcSession.h"
#include <gmock/gmock.h>

class IMessage;
class ISession;
class MessageSender;
class MtcExtensionSet;
struct CallReasonInfo;

class MockIMtcSession : public IMtcSession
{
public:
    virtual ~MockIMtcSession() {}

    MOCK_METHOD(IMS_RESULT, Start, (), (override));
    MOCK_METHOD(IMS_RESULT, SendProvisionalResponse,
            (IN IMS_BOOL bUserAlert, IN IMS_BOOL bReliable), (override));
    MOCK_METHOD(IMS_RESULT, SendPrack, (IN IMS_BOOL bSdpOfferRequired), (override));
    MOCK_METHOD(IMS_RESULT, RespondToPrack, (IN IMS_SINT32 eStatusCode), (override));
    MOCK_METHOD(IMS_RESULT, SendEarlyUpdate, (IN UpdateType eUpdateType), (override));
    MOCK_METHOD(IMS_RESULT, RespondToEarlyUpdate, (IN IMS_SINT32 eStatusCode), (override));
    MOCK_METHOD(IMS_RESULT, SendAck, (), (override));
    MOCK_METHOD(IMS_RESULT, Accept, (), (override));
    MOCK_METHOD(IMS_RESULT, Reject, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(IMS_RESULT, Update,
            (IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo, IN IMS_SINT32 eMethod),
            (override));
    MOCK_METHOD(IMS_RESULT, AcceptUpdate, (), (override));
    MOCK_METHOD(IMS_RESULT, CancelUpdate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(IMS_RESULT, Terminate, (IMS_BOOL bUseBye, IN const CallReasonInfo& objReason),
            (override));
    MOCK_METHOD(void, SetSessionTerminatedOrStartFailed, (), (override));
    MOCK_METHOD(
            void, HandleRequest, (IN RequestType eType, IN const IMessage& objRequest), (override));
    MOCK_METHOD(void, HandleResponse, (IN ResponseType eType, IN const IMessage& objResponse),
            (override));
    MOCK_METHOD(void, SetCallType, (IN CallType eCallType), (override));
    MOCK_METHOD(CallType, GetCallType, (), (const, override));
    MOCK_METHOD(CallType, GetPreviousCallType, (), (const, override));
    MOCK_METHOD(ISession&, GetISession, (), (override));
    MOCK_METHOD(MtcExtensionSet&, GetExtensionSet, (), (override));
    MOCK_METHOD(IMS_BOOL, IsVideoCapable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRttCapable, (), (const, override));
    MOCK_METHOD(UpdateType, GetOngoingUpdateType, (), (const, override));
};

#endif
