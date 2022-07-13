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

#ifndef MOCK_I_MTC_CALL_CONTROLLER_H_
#define MOCK_I_MTC_CALL_CONTROLLER_H_

#include <gmock/gmock.h>
#include "ImsTypeDef.h"
#include "IMtcCallController.h"
#include "IMtcService.h"
#include "IuMtcCall.h"
#include "IuMtcService.h"
#include "call/IMtcCall.h"

class ISession;
class JniMediaSessionThread;
class JniMtcCallThread;
class JniMtcServiceThread;
enum class KeyType;
struct ConfUser;
union Key;

class MockIMtcCallController : public IMtcCallController
{
public:
    MOCK_METHOD(void, TerminateCalls,
            (IN KeyType eKeyType, IN Key nKey, IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, RemoveCalls, (IN KeyType eKeyType, IN Key nKey), (override));
    MOCK_METHOD(CallKey, Open, (IN ServiceType eServiceType, IN CallInfo& objCallInfo), (override));
    MOCK_METHOD(void, Attach,
            (IN CallKey nCallKey, IN JniMtcCallThread* pJniMtcCallThread,
                    IN JniMediaSessionThread* pJniMediaThread),
            (override));
    MOCK_METHOD(void, Detach, (IN CallKey nCallKey), (override));
    MOCK_METHOD(void, HandleIncoming,
            (IN IMtcService* pService, IN ISession* piSession,
                    IN JniMtcServiceThread* pServiceThread),
            (override));
    MOCK_METHOD(void, Start,
            (IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
                    IN MediaInfo* pMediaInfo,
                    (IN const IMSMap<SuppType, SuppService*>& objSuppServices),
                    IN IDialogEvent* pDialog),
            (override));
    MOCK_METHOD(void, HandleUserAlert, (IN CallKey nCallKey), (override));
    MOCK_METHOD(void, Accept,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(
            void, Reject, (IN CallKey nCallKey, IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, Hold, (IN CallKey nCallKey, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, Resume, (IN CallKey nCallKey, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, AcceptResume,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, RejectResume, (IN CallKey nCallKey, IN const CallReasonInfo& objReason),
            (override));
    MOCK_METHOD(
            void, Terminate, (IN CallKey nCallKey, IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, Update,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, CancelUpdate, (IN CallKey nCallKey, IN const CallReasonInfo& objReason),
            (override));
    MOCK_METHOD(void, AcceptUpdate,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, RejectUpdate, (IN CallKey nCallKey, IN const CallReasonInfo& objReason),
            (override));
    MOCK_METHOD(void, SendUssd, (IN CallKey nCallKey, IN const AString& strUssd), (override));
    MOCK_METHOD(void, MergeToConference, (IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers),
            (override));
    MOCK_METHOD(void, AddToConference, (IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers),
            (override));
    MOCK_METHOD(void, RemoveFromConference, (IN CallKey nCallKey, IN IMSList<ConfUser*>& objUsers),
            (override));
    MOCK_METHOD(void, Transfer, (IN CallKey nCallKey, IN const AString& strTarget), (override));
    MOCK_METHOD(void, HandleIpcanChanged, (), (override));
};

#endif
