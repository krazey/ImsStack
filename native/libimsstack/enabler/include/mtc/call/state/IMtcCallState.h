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

#ifndef INTERFACE_MTC_CALL_STATE_H_
#define INTERFACE_MTC_CALL_STATE_H_

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockChecker.h"
#include "helper/IMtcAosStateListener.h"

class AString;
class IMessage;
class IMtcCallContext;
class IReference;
class ISession;
class ISipClientConnection;
class ISipConnection;
class ISipServerConnection;
class MtcSession;
enum class QosLossPolicy;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;

using CallStateName = IMtcCall::State;

class IMtcCallState
{
public:
    virtual ~IMtcCallState() {}

    /**
     * @brief Notifies
     *
     */
    virtual void OnEnter() = 0;

    /**
     * @brief Notifies
     *
     */
    virtual void OnExit() = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual CallStateName GetStateName() const = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param strTarget
     * @param objMediaInfo
     * @param objSuppServices
     * @return
     */
    virtual CallStateName Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param strTarget
     * @param objMediaInfo
     * @param objSuppServices
     * @param lstUsers
     * @return
     */
    virtual CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& lstUsers) = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param strTarget
     * @param lstUsers
     * @return
     */
    virtual CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& lstUsers) = 0;

    /**
     * @brief Handles
     *
     * @param piSession
     * @return
     */
    virtual CallStateName HandleIncoming(IN ISession* piSession) = 0;

    /**
     * @brief Handles
     *
     * @return
     */
    virtual CallStateName HandleUserAlert() = 0;

    /**
     * @brief Accepts
     *
     * @param eCallType
     * @param objMediaInfo
     * @return
     */
    virtual CallStateName Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects
     *
     * @param objReason
     * @return
     */
    virtual CallStateName Reject(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Holds
     *
     * @param objMediaInfo
     * @return
     */
    virtual CallStateName Hold(IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Resumes
     *
     * @param objMediaInfo
     * @return
     */
    virtual CallStateName Resume(IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Accepts
     *
     * @param eCallType
     * @param objMediaInfo
     * @return
     */
    virtual CallStateName AcceptResume(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects
     *
     * @param objReason
     * @return
     */
    virtual CallStateName RejectResume(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Updates
     *
     * @param eCallType
     * @param objMediaInfo
     * @return
     */
    virtual CallStateName Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Accepts
     *
     * @param eCallType
     * @param objMediaInfo
     * @return
     */
    virtual CallStateName AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects
     *
     * @param objReason
     * @return
     */
    virtual CallStateName RejectUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Cancels
     *
     * @param objReason
     * @return
     */
    virtual CallStateName CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Terminates
     *
     * @param objReason
     * @return
     */
    virtual CallStateName Terminate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Handles
     *
     * @param piSession
     * @return
     */
    virtual CallStateName HandleIncomingUssi(IN ISession* piSession) = 0;

    /**
     * @brief Notifies
     *
     * @return
     */
    virtual CallStateName OnUssiAttached() = 0;

    /**
     * @brief Accepts
     *
     * @param eCallType
     * @param objMediaInfo
     * @return
     */
    virtual CallStateName AcceptUssi(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Ussis
     *
     * @param piSession
     * @return
     */
    virtual CallStateName UssiStarted(IN ISession* piSession) = 0;

    /**
     * @brief Terminates
     *
     * @param objReason
     * @return
     */
    virtual CallStateName TerminateUssi(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Ussis
     *
     * @param piSession
     * @return
     */
    virtual CallStateName UssiTerminated(IN ISession* piSession) = 0;

    /**
     * @brief Sends
     *
     * @param strUssd
     * @return
     */
    virtual CallStateName SendUssd(IN const AString& strUssd) = 0;

    /**
     * @brief Ussis
     *
     * @param piSession
     * @param piSipServerConnection
     * @return
     */
    virtual CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) = 0;

    /**
     * @brief Notifys
     *
     * @param piScc
     * @param piForkedScc
     * @return
     */
    virtual CallStateName NotifyResponseToUssiInfo(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) = 0;

    /**
     * @brief Notifys
     *
     * @param piSc
     * @param nCode
     * @param strMessage
     * @return
     */
    virtual CallStateName NotifyErrorToUssiInfo(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionAlerting(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @param piReference
     * @return
     */
    virtual CallStateName SessionReferenceReceived(
            IN ISession* piSession, IN IReference* piReference) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionStarted(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionStartFailed(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionTerminated(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionUpdated(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionUpdateFailed(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionUpdateReceived(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionCancelDelivered(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionCancelDeliveryFailed(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @param piForkedSession
     * @return
     */
    virtual CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionPrackDelivered(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionPrackDeliveryFailed(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionPrackReceived(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @param nIndex
     * @return
     */
    virtual CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @return
     */
    virtual CallStateName SessionRprDeliveryFailed(IN ISession* piSession) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @param nIndex
     * @return
     */
    virtual CallStateName SessionRprReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) = 0;

    /**
     * @brief Sessions
     *
     * @param piSession
     * @param piSipServerConnection
     * @return
     */
    virtual CallStateName SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) = 0;

    /**
     * @brief Refresh_s
     *
     * @param piScc
     * @return
     */
    virtual CallStateName Refresh_NotifyCompleted(IN ISipClientConnection* piScc) = 0;

    /**
     * @brief Refresh_s
     *
     * @return
     */
    virtual CallStateName Refresh_NotifyTerminated() = 0;

    /**
     * @brief Refresh_s
     *
     * @param bDoImplicitRefresh
     * @return
     */
    virtual CallStateName Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;

    /**
     * @brief Notifies
     *
     * @param nType
     * @return
     */
    virtual CallStateName OnTimerExpired(IN IMS_SINT32 nType) = 0;

    /**
     * @brief Notifies
     *
     * @param objResult
     * @return
     */
    virtual CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult) = 0;

    /**
     * @brief Qoss
     *
     * @param piSession
     * @param eMediaType
     * @return
     */
    virtual CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Qoss
     *
     * @param piSession
     * @param eNextAction
     * @return
     */
    virtual CallStateName QosReserveFailed(
            IN ISession* piSession, IN QosLossPolicy eNextAction) = 0;

    /**
     * @brief Notifies
     *
     * @return
     */
    virtual CallStateName OnInternalFailure() = 0;

    /**
     * @brief Notifies
     *
     * @return
     */
    virtual CallStateName OnAttached() = 0;

    /**
     * @brief Clients
     *
     * @param piScc
     * @param piForkedScc
     * @return
     */
    virtual CallStateName ClientConnection_NotifyResponse(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc) = 0;

    /**
     * @brief Error_s
     *
     * @param piSc
     * @param nCode
     * @param strMessage
     * @return
     */
    virtual CallStateName Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;

    /**
     * @brief Notifies
     *
     * @param eMediaType
     * @param eProtocolType
     * @return
     */
    virtual CallStateName OnReceivingMediaDataStarted(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) = 0;

    /**
     * @brief Notifies
     *
     * @param eMediaType
     * @param eProtocolType
     * @return
     */
    virtual CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) = 0;

    /**
     * @brief Notifies
     *
     * @return
     */
    virtual CallStateName OnVideoLowestBitRate() = 0;

    /**
     * @brief Notifies
     *
     * @return
     */
    virtual CallStateName OnReceivingNetworkToneStarted() = 0;

    /**
     * @brief Notifies
     *
     * @return
     */
    virtual CallStateName OnReceivingNetworkToneFailed() = 0;

    /**
     * @brief Notifies
     *
     * @param objReason
     * @return
     */
    virtual CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Notifies
     *
     * @param eState
     * @return
     */
    virtual CallStateName OnSrvccStateUpdated(IN SrvccState eState) = 0;

    /**
     * @brief Notifies
     *
     * @param eState
     * @param eAosReason
     * @return
     */
    virtual CallStateName OnAosStateChanged(IN MtcAosState eState, IN IMS_UINT32 eAosReason) = 0;

    /**
     * @brief Notifies
     *
     * @param eIpcan
     * @return
     */
    virtual CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) = 0;
};

#endif
