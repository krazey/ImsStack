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

#include "ImsTypeDef.h"
#include "call/IMtcCall.h"
#include "call/block/IMtcBlockChecker.h"
#include "helper/ISrvccStateListener.h"

class AString;
class IMessage;
class IMtcCallContext;
class IReference;
class ISession;
class ISipClientConnection;
class ISipServerConnection;
class MtcSession;
enum class MtcAosState;
enum class QosLossPolicy;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;
template <class T>
class ImsList;

using CallStateName = IMtcCall::State;

/**
 * @brief Defines the interface for a state in the MTC call state machine.
 *
 * The methods in this interface correspond to events that can occur during a call. When an event
 * occurs, the corresponding method in the current state is invoked. This method is responsible for
 * handling the event, performing any necessary actions, and returning the name of the next state
 * to which the call should transition.
 */
class IMtcCallState
{
public:
    virtual ~IMtcCallState() {}

    /**
     * @brief Called when entering this state.
     */
    virtual void OnEnter() = 0;

    /**
     * @brief Called when exiting this state.
     */
    virtual void OnExit() = 0;

    /**
     * @brief Gets the name of this state.
     *
     * @return The name of the state.
     */
    virtual CallStateName GetStateName() const = 0;

    /** @see IMtcCall#Start */
    virtual CallStateName Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices) = 0;

    /** @see IMtcCall#StartConference */
    virtual CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& lstUsers) = 0;

    /** @see IMtcCall#StartConference */
    virtual CallStateName StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& lstUsers) = 0;

    /** @see IMtcCall#HandleIncoming */
    virtual CallStateName HandleIncoming(IN ISession* piSession) = 0;

    /** @see IMtcCall#HandleUserAlert */
    virtual CallStateName HandleUserAlert() = 0;

    /** @see IMtcCall#Accept */
    virtual CallStateName Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /** @see IMtcCall#Reject */
    virtual CallStateName Reject(IN const CallReasonInfo& objReason) = 0;

    /** @see IMtcCall#Hold */
    virtual CallStateName Hold(IN MediaInfo& objMediaInfo) = 0;

    /** @see IMtcCall#Resume */
    virtual CallStateName Resume(IN MediaInfo& objMediaInfo) = 0;

    /** @see IMtcCall#AcceptResume */
    virtual CallStateName AcceptResume(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /** @see IMtcCall#RejectResume */
    virtual CallStateName RejectResume(IN const CallReasonInfo& objReason) = 0;

    /** @see IMtcCall#Update */
    virtual CallStateName Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /** @see IMtcCall#AcceptUpdate */
    virtual CallStateName AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /** @see IMtcCall#RejectUpdate */
    virtual CallStateName RejectUpdate(IN const CallReasonInfo& objReason) = 0;

    /** @see IMtcCall#CancelUpdate */
    virtual CallStateName CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    /** @see IMtcCall#Terminate */
    virtual CallStateName Terminate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Handles an incoming USSI request.
     *
     * This is used instead of {@link HandleIncoming} for USSI.
     *
     * @param piSession The session associated with the USSI request.
     * @return The next call state.
     */
    virtual CallStateName HandleIncomingUssi(IN ISession* piSession) = 0;

    /**
     * @brief Notifies that the USSI call has been attached to the framework.
     *
     * This is used instead of {@link OnAttached} for USSI.
     *
     * @return The next call state.
     */
    virtual CallStateName OnUssiAttached() = 0;

    /**
     * @brief Accepts an incoming USSI request.
     *
     * This is used instead of {@link Accept} for USSI.
     *
     * @param eCallType The call type for the USSI session.
     * @param objMediaInfo The media information for the USSI session.
     * @return The next call state.
     */
    virtual CallStateName AcceptUssi(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Handles the start of a USSI session.
     *
     * This is used instead of {@link SessionStarted} for USSI.
     *
     * @param piSession The USSI session that has started.
     * @return The next call state.
     */
    virtual CallStateName UssiStarted(IN ISession* piSession) = 0;

    /**
     * @brief Terminates a USSI session.
     *
     * This is used instead of {@link Terminate} for USSI.
     *
     * @param objReason The reason for terminating the USSI session.
     * @return The next call state.
     */
    virtual CallStateName TerminateUssi(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Handles the termination of a USSI session.
     *
     * This is used instead of {@link SessionTerminated} for USSI.
     *
     * @param piSession The USSI session that has been terminated.
     * @return The next call state.
     */
    virtual CallStateName UssiTerminated(IN ISession* piSession) = 0;

    /** @see IMtcCall#SendUssd */
    virtual CallStateName SendUssd(IN const AString& strUssd) = 0;

    /**
     * @brief Handles a received USSI INFO message.
     *
     * This is used instead of {@link SessionTransactionReceived} for USSI.
     *
     * @param piSession The session associated with the USSI.
     * @param piSipServerConnection The SIP server connection.
     * @return The next call state.
     */
    virtual CallStateName UssiInfoReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) = 0;

    /** @see ISessionListener#SessionAlerting */
    virtual CallStateName SessionAlerting(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionReferenceReceived */
    virtual CallStateName SessionReferenceReceived(
            IN ISession* piSession, IN IReference* piReference) = 0;

    /** @see ISessionListener#SessionStarted */
    virtual CallStateName SessionStarted(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionStartFailed */
    virtual CallStateName SessionStartFailed(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionTerminated */
    virtual CallStateName SessionTerminated(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionUpdated */
    virtual CallStateName SessionUpdated(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionUpdateFailed */
    virtual CallStateName SessionUpdateFailed(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionUpdateReceived */
    virtual CallStateName SessionUpdateReceived(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionCanceledOnAccepted */
    virtual CallStateName SessionCanceledOnAccepted(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionCancelDelivered */
    virtual CallStateName SessionCancelDelivered(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionCancelDeliveryFailed */
    virtual CallStateName SessionCancelDeliveryFailed(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionEarlyMediaUpdated */
    virtual CallStateName SessionEarlyMediaUpdated(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionEarlyMediaUpdateFailed */
    virtual CallStateName SessionEarlyMediaUpdateFailed(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionEarlyMediaUpdateReceived */
    virtual CallStateName SessionEarlyMediaUpdateReceived(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionForkedResponseReceived */
    virtual CallStateName SessionForkedResponseReceived(
            IN ISession* piSession, IN ISession* piForkedSession) = 0;

    /** @see ISessionListener#SessionPrackDelivered */
    virtual CallStateName SessionPrackDelivered(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionPrackDeliveryFailed */
    virtual CallStateName SessionPrackDeliveryFailed(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionPrackReceived */
    virtual CallStateName SessionPrackReceived(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionProvisionalResponseReceived */
    virtual CallStateName SessionProvisionalResponseReceived(
            IN ISession* piSession, IN IMS_UINT32 nIndex) = 0;

    /** @see ISessionListener#SessionRprDeliveryFailed */
    virtual CallStateName SessionRprDeliveryFailed(IN ISession* piSession) = 0;

    /** @see ISessionListener#SessionRprReceived */
    virtual CallStateName SessionRprReceived(IN ISession* piSession, IN IMS_UINT32 nIndex) = 0;

    /** @see ISessionListener#SessionTransactionReceived */
    virtual CallStateName SessionTransactionReceived(
            IN ISession* piSession, IN ISipServerConnection* piSipServerConnection) = 0;

    /** @see IRefreshListener#Refresh_NotifyCompleted */
    virtual CallStateName Refresh_NotifyCompleted(IN ISipClientConnection* piScc) = 0;

    /** @see IRefreshListener#Refresh_NotifyTerminated */
    virtual CallStateName Refresh_NotifyTerminated() = 0;

    /** @see IRefreshListener#Refresh_NotifyTimerExpired */
    virtual CallStateName Refresh_NotifyTimerExpired(OUT IMS_BOOL& bDoImplicitRefresh) = 0;

    /** @see IMtcTimerListener#OnTimerExpired */
    virtual CallStateName OnTimerExpired(IN IMS_SINT32 nType) = 0;

    /** @see IMtcBlockCheckListener#OnBlockChecked */
    virtual CallStateName OnBlockChecked(IN IMtcBlockChecker::Result objResult) = 0;

    /** @see IMtcPreconditionListener#QosReserved */
    virtual CallStateName QosReserved(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /** @see IMtcPreconditionListener#QosReserveFailed */
    virtual CallStateName QosReserveFailed(
            IN ISession* piSession, IN QosLossPolicy eNextAction) = 0;

    /**
     * @brief Handles an internal failure (e.g., unexpected null parameter from the other modules).
     *
     * @return The next call state.
     */
    virtual CallStateName OnInternalFailure() = 0;

    /** @see IMtcCall#Attach */
    virtual CallStateName OnAttached() = 0;

    /** @see IMediaReportEventListener#OnReceivingMediaDataStarted */
    virtual CallStateName OnReceivingMediaDataStarted(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) = 0;

    /** @see IMediaReportEventListener#OnReceivingMediaDataFailed */
    virtual CallStateName OnReceivingMediaDataFailed(
            IN IMS_UINT32 eMediaType, IN IMS_UINT32 eProtocolType) = 0;

    /** @see IMediaReportEventListener#OnVideoLowestBitRate */
    virtual CallStateName OnVideoLowestBitRate() = 0;

    /** @see IMediaReportEventListener#OnReceivingNetworkToneStarted */
    virtual CallStateName OnReceivingNetworkToneStarted() = 0;

    /** @see IMediaReportEventListener#OnReceivingNetworkToneFailed */
    virtual CallStateName OnReceivingNetworkToneFailed() = 0;

    /** @see IMediaReportEventListener#OnMediaFailed */
    virtual CallStateName OnMediaFailed(IN const CallReasonInfo& objReason) = 0;

    /** @see ISrvccStateListener#OnSrvccStateUpdated */
    virtual CallStateName OnSrvccStateUpdated(IN SrvccState eState) = 0;

    /** @see IMtcAosStateListener#OnAosStateChanged */
    virtual CallStateName OnAosStateChanged(
            IN MtcAosState eState, IN IMS_UINT32 eAosReason, IN IMS_SINT32 nDataFailureReason) = 0;

    /**
     * @brief Notifies a IP-CAN is changed between IWLAN and mobile RAT.
     *
     * @see IMtcNetworkWatcherListener#OnRatChanged
     */
    virtual CallStateName OnIpcanChanged(IN IMS_UINT32 eIpcan) = 0;

    /**
     * @brief Notifies a mobile RAT change event except for IP-CAN change.
     *
     * @see OnIpcanChanged
     * @see IMtcNetworkWatcherListener#OnRatChanged
     */
    virtual CallStateName OnRatChanged(IN IMS_SINT32 eOldRatType, IN IMS_SINT32 eRatType) = 0;

    /** @see IMtcRadioCheckerListener#OnConnectionFailed */
    virtual CallStateName OnConnectionFailed(
            IN IMS_UINT32 nFailureReason, IN IMS_UINT32 nWaitTimeMillis) = 0;
};

#endif
