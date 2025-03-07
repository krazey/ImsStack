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

#ifndef MOCK_I_MTC_CALL_STATE_H_
#define MOCK_I_MTC_CALL_STATE_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockChecker.h"
#include "call/state/IMtcCallState.h"
#include "helper/IMtcAosStateListener.h"
#include <gmock/gmock.h>

class AString;
class IReference;
class ISession;
class ISipClientConnection;
class ISipConnection;
class ISipServerConnection;
enum class QosLossPolicy;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;

class MockIMtcCallState : public IMtcCallState
{
public:
    MOCK_METHOD(void, OnEnter, (), (override));
    MOCK_METHOD(void, OnExit, (), (override));
    MOCK_METHOD(CallStateName, GetStateName, (), (const, override));
    MOCK_METHOD(CallStateName, Start,
            (IN CallType eCallType, IN const AString& strTarget, IN MediaInfo& objMediaInfo,
                    (IN const ImsMap<SuppType, SuppService*>& objSuppServices)),
            (override));
    MOCK_METHOD(CallStateName, StartConference,
            (IN CallType eCallType, IN const AString& strTarget, IN MediaInfo& objMediaInfo,
                    (IN const ImsMap<SuppType, SuppService*>& objSuppServices),
                    IN const ImsList<ConfUser*>& lstUsers),
            (override));
    MOCK_METHOD(CallStateName, StartConference,
            (IN CallType eCallType, IN const AString& strTarget,
                    IN const ImsList<ConfUser*>& lstUsers),
            (override));
    MOCK_METHOD(CallStateName, HandleIncoming, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, HandleUserAlert, (), (override));
    MOCK_METHOD(
            CallStateName, Accept, (IN CallType eCallType, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(CallStateName, Reject, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(CallStateName, Hold, (IN MediaInfo & objMediaInfo), (override));
    MOCK_METHOD(CallStateName, Resume, (IN MediaInfo & objMediaInfo), (override));
    MOCK_METHOD(CallStateName, AcceptResume, (IN CallType eCallType, IN MediaInfo& objMediaInfo),
            (override));
    MOCK_METHOD(CallStateName, RejectResume, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(
            CallStateName, Update, (IN CallType eCallType, IN MediaInfo& objMediaInfo), (override));
    MOCK_METHOD(CallStateName, AcceptUpdate, (IN CallType eCallType, IN MediaInfo& objMediaInfo),
            (override));
    MOCK_METHOD(CallStateName, RejectUpdate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(CallStateName, CancelUpdate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(CallStateName, Terminate, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(CallStateName, HandleIncomingUssi, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, OnUssiAttached, (), (override));
    MOCK_METHOD(CallStateName, AcceptUssi, (IN CallType eCallType, IN MediaInfo& objMediaInfo),
            (override));
    MOCK_METHOD(CallStateName, UssiStarted, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, TerminateUssi, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(CallStateName, UssiTerminated, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SendUssd, (IN const AString& strUssd), (override));
    MOCK_METHOD(CallStateName, UssiInfoReceived,
            (IN ISession* piSession, IN ISipServerConnection* piSipServerConnection), (override));
    MOCK_METHOD(CallStateName, NotifyResponseToUssiInfo,
            (IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc), (override));
    MOCK_METHOD(CallStateName, NotifyErrorToUssiInfo,
            (IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage),
            (override));
    MOCK_METHOD(CallStateName, SessionAlerting, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionReferenceReceived,
            (IN ISession* piSession, IN IReference* piReference), (override));
    MOCK_METHOD(CallStateName, SessionStarted, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionStartFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionTerminated, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionUpdated, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionUpdateFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionUpdateReceived, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionCanceledOnAccepted, (IN ISession * piSession), (override));
    MOCK_METHOD(CallStateName, SessionCancelDelivered, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionCancelDeliveryFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionEarlyMediaUpdated, (IN ISession* piSession), (override));
    MOCK_METHOD(
            CallStateName, SessionEarlyMediaUpdateFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(
            CallStateName, SessionEarlyMediaUpdateReceived, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionForkedResponseReceived,
            (IN ISession* piSession, IN ISession* piForkedSession), (override));
    MOCK_METHOD(CallStateName, SessionPrackDelivered, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionPrackDeliveryFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionPrackReceived, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionProvisionalResponseReceived,
            (IN ISession* piSession, IN IMS_UINT32 nIndex), (override));
    MOCK_METHOD(CallStateName, SessionRprDeliveryFailed, (IN ISession* piSession), (override));
    MOCK_METHOD(CallStateName, SessionRprReceived, (IN ISession* piSession, IN IMS_UINT32 nIndex),
            (override));
    MOCK_METHOD(CallStateName, SessionTransactionReceived,
            (IN ISession* piSession, IN ISipServerConnection* piSipServerConnection), (override));
    MOCK_METHOD(
            CallStateName, Refresh_NotifyCompleted, (IN ISipClientConnection* piScc), (override));
    MOCK_METHOD(CallStateName, Refresh_NotifyTerminated, (), (override));
    MOCK_METHOD(CallStateName, Refresh_NotifyTimerExpired, (OUT IMS_BOOL& bDoImplicitRefresh),
            (override));
    MOCK_METHOD(CallStateName, OnTimerExpired, (IN IMS_SINT32 nType), (override));
    MOCK_METHOD(CallStateName, OnBlockChecked, (IN IMtcBlockChecker::Result objResult), (override));
    MOCK_METHOD(CallStateName, QosReserved, (IN ISession* piSession, IN IMS_UINT32 eMediaType),
            (override));
    MOCK_METHOD(CallStateName, QosReserveFailed,
            (IN ISession* piSession, IN QosLossPolicy eNextAction), (override));
    MOCK_METHOD(CallStateName, OnInternalFailure, (), (override));
    MOCK_METHOD(CallStateName, OnAttached, (), (override));
    MOCK_METHOD(CallStateName, ClientConnection_NotifyResponse,
            (IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc), (override));
    MOCK_METHOD(CallStateName, Error_NotifyError,
            (IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage),
            (override));
    MOCK_METHOD(CallStateName, OnReceivingMediaDataStarted,
            (IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType), (override));
    MOCK_METHOD(CallStateName, OnReceivingMediaDataFailed,
            (IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType), (override));
    MOCK_METHOD(CallStateName, OnVideoLowestBitRate, (), (override));
    MOCK_METHOD(CallStateName, OnReceivingNetworkToneStarted, (), (override));
    MOCK_METHOD(CallStateName, OnReceivingNetworkToneFailed, (), (override));
    MOCK_METHOD(CallStateName, OnMediaFailed, (IN const CallReasonInfo& objReason), (override));
    MOCK_METHOD(CallStateName, OnSrvccStateUpdated, (IN SrvccState eState), (override));
    MOCK_METHOD(CallStateName, OnAosStateChanged, (IN MtcAosState eState, IN IMS_UINT32 eAosReason),
            (override));
    MOCK_METHOD(CallStateName, OnIpcanChanged, (IN IMS_UINT32 eIpcan), (override));
    MOCK_METHOD(CallStateName, OnRatChanged, (IN IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType),
            (override));
    MOCK_METHOD(CallStateName, OnConnectionFailed,
            (IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis), (override));
};

#endif
