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

#include "IMtcCallController.h"
#include "IMtcService.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include <gmock/gmock.h>

class AString;
class IMtcCallContext;
class ISession;
class ISilentRedialHelper;
class SuppService;
enum class CallType;
enum class KeyType;
enum class SuppType;
struct CallInfo;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;
union Key;

class MockIMtcCallController : public IMtcCallController
{
public:
    MOCK_METHOD(CallKey, Open, (IN ServiceType eServiceType, IN CallInfo& objCallInfo), (override));
    MOCK_METHOD(void, Attach, (IN CallKey nCallKey), (override));
    MOCK_METHOD(void, Detach, (IN CallKey nCallKey), (override));
    MOCK_METHOD(void, HandleIncoming,
            (IN IMtcService* pService, IN ISession* piSession), (override));
    MOCK_METHOD(void, Start,
            (IN CallKey nCallKey, IN CallType eCallType, IN const AString& strTarget,
                    IN MediaInfo& objMediaInfo,
                    (IN const ImsMap<SuppType, SuppService*>& objSuppServices)),
            (override));
    MOCK_METHOD(void, HandleUserAlert, (IN CallKey nCallKey), (override));
    MOCK_METHOD(void, Accept,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(
            void, Reject, (IN CallKey nCallKey, IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, Hold, (IN CallKey nCallKey, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(void, Resume, (IN CallKey nCallKey, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(void, AcceptResume,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(void, RejectResume, (IN CallKey nCallKey, IN const CallReasonInfo& objReason),
            (override));
    MOCK_METHOD(
            void, Terminate, (IN CallKey nCallKey, IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, Update,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(void, CancelUpdate, (IN CallKey nCallKey, IN const CallReasonInfo& objReason),
            (override));
    MOCK_METHOD(void, AcceptUpdate,
            (IN CallKey nCallKey, IN CallType eCallType, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(void, RejectUpdate, (IN CallKey nCallKey, IN const CallReasonInfo& objReason),
            (override));
    MOCK_METHOD(void, SendUssd, (IN CallKey nCallKey, IN const AString& strUssd), (override));
    MOCK_METHOD(void, MergeToConference, (IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers),
            (override));
    MOCK_METHOD(void, AddToConference, (IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers),
            (override));
    MOCK_METHOD(void, RemoveFromConference, (IN CallKey nCallKey, IN ImsList<ConfUser*>& objUsers),
            (override));
    MOCK_METHOD(void, Transfer, (IN CallKey nCallKey, IN const AString& strTarget), (override));
    MOCK_METHOD(ISilentRedialHelper&, GetRedialHelper, (
            IN IMtcCallContext& objContext, IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, ReleaseRedialHelper, (), (override));

    // IEnablerService interface
    MOCK_METHOD(void, NotifyJniEnablerSet, (), (override));
};

#endif
