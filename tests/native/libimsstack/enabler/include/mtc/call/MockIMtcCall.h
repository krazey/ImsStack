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
 * distributed under the License is distributed on an "AS IS" B ASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MOCK_I_MTC_CALL_H_
#define MOCK_I_MTC_CALL_H_

#include <gmock/gmock.h>
#include "AString.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "IMtcService.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"

class ConfUser;
class IDialogEvent;
class IMtcCallContext;
class ISession;
class JniMediaSessionThread;
class JniMtcCallThread;
class JniMtcServiceThread;
class MediaInfo;
class SuppService;
enum class UpdateType;
struct CallReasonInfo;

class MockIMtcCall : public IMtcCall
{
public:
    ~MockIMtcCall() {}

    MOCK_METHOD(void, Attach,
            (IN JniMtcCallThread * pJniMtcCallThread, IN JniMediaSessionThread* pJniMediaThread),
            (override));
    MOCK_METHOD(void, Detach, (), (override));
    MOCK_METHOD(void, Start,
            (IN CallType eCallType, IN const AString& strTarget, IN MediaInfo* pMediaInfo,
                    (IN const IMSMap<SuppType, SuppService*>& objSuppServices)),
            (override));
    MOCK_METHOD(void, StartConference,
            (IN CallType eCallType, IN const AString& strTarget, IN MediaInfo* pMediaInfo,
                    (IN const IMSMap<SuppType, SuppService*>& objSuppServices),
                    IN const IMSList<ConfUser*> objUsers),
            (override));
    MOCK_METHOD(void, StartConference,
            (IN CallType eCallType, IN const AString& strTarget,
                    IN const IMSList<ConfUser*> objUsers),
            (override));
    MOCK_METHOD(void, HandleIncoming,
            (IN ISession * piSession, IN JniMtcServiceThread* pServiceThread), (override));
    MOCK_METHOD(void, HandleUserAlert, (), (override));
    MOCK_METHOD(void, Accept, (IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, Reject, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, Hold, (IN MediaInfo * pMediaInfo), (override));
    MOCK_METHOD(void, Resume, (IN MediaInfo * pMediaInfo), (override));
    MOCK_METHOD(void, AcceptResume, (IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, RejectResume, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, Update, (IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, AcceptUpdate, (IN CallType eCallType, IN MediaInfo* pMediaInfo), (override));
    MOCK_METHOD(void, RejectUpdate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, CancelUpdate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, Terminate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(void, SendDtmf, (IN const AString& strSignal, IN IMS_SINT32 nDuration), (override));
    MOCK_METHOD(void, SendUssd, (IN const AString& strUssd), (override));
    MOCK_METHOD(void, HandleSrvccSuccess, (), (override));
    MOCK_METHOD(void, HandleSrvccFailure, (IN UpdateType eUpdateType), (override));
    MOCK_METHOD(void, HandleIpcanChanged, (), (override));
    MOCK_METHOD(CallKey, GetKey, (), (const, override));
    MOCK_METHOD(CallType, GetCallType, (), (const, override));
    MOCK_METHOD(State, GetState, (), (const, override));
    MOCK_METHOD(IMtcCallContext&, GetCallContext, (), (const, override));
};

#endif
