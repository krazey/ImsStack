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

#ifndef INTERFACE_MTC_SESSION_H_
#define INTERFACE_MTC_SESSION_H_

#include "ImsTypeDef.h"
#include "MtcDef.h"
#include "call/IMtcCall.h"
#include "call/message/IMtcMessageHandler.h"

class IMessage;
class ISession;
class MessageSender;
class MtcExtensionSet;
struct CallReasonInfo;

/**
 * @brief An interface that manages SIP (Session Initiation Protocol) signaling for a single call
 *        session.
 *
 * This class acts as an abstraction layer over the core #ISession object, providing methods
 * to handle SIP transactions such as session start, accept, reject, update, and terminate.
 * It also manages session-specific properties like call type and media capabilities.
 * Each #IMtcCall instance can have one or more #IMtcSession instances (e.g., in case of forking).
 *
 * @see IMtcMessageHandler
 */
class IMtcSession : public IMtcMessageHandler
{
public:
    /**
     * @brief Starts the session initiation process.
     *
     * This method initiates the SIP session setup, typically by sending an INVITE request.
     *
     * @return IMS_SUCCESS if the request is successfully initiated, otherwise an error code.
     */
    virtual IMS_RESULT Start() = 0;

    /**
     * @brief Sends a provisional response (1xx) to an incoming INVITE.
     *
     * @param bUserAlert Indicates if the user is being alerted (e.g., ringing).
     *                   If true, typically sends 180 Ringing.
     * @param bReliable Indicates if the provisional response should be sent reliably (100rel).
     * @return IMS_SUCCESS if the response is successfully sent, otherwise an error code.
     */
    virtual IMS_RESULT SendProvisionalResponse(IN IMS_BOOL bUserAlert, IN IMS_BOOL bReliable) = 0;

    /**
     * @brief Sends a PRACK request to acknowledge a reliable provisional response.
     *
     * @param bSdpOfferRequired Indicates if an SDP offer is required in the PRACK body.
     * @return IMS_SUCCESS if the PRACK is successfully sent, otherwise an error code.
     */
    virtual IMS_RESULT SendPrack(IN IMS_BOOL bSdpOfferRequired) = 0;

    /**
     * @brief Sends a response to a received PRACK request.
     *
     * @param eStatusCode The #SipStatusCode to send in the response (e.g., 200).
     * @return IMS_SUCCESS if the response is successfully sent, otherwise an error code.
     */
    virtual IMS_RESULT RespondToPrack(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Sends an UPDATE request during the early dialog state.
     *
     * This is typically used for QoS preconditions or early media negotiation.
     *
     * @param eUpdateType The type of update (e.g., for SDP negotiation).(#UpdateType)
     * @return IMS_SUCCESS if the UPDATE is successfully sent, otherwise an error code.
     */
    virtual IMS_RESULT SendEarlyUpdate(IN UpdateType eUpdateType) = 0;

    /**
     * @brief Sends a response to a received UPDATE request in the early dialog state.
     *
     * @param eStatusCode The #SipStatusCode to send (e.g., 200).
     * @return IMS_SUCCESS if the response is successfully sent, otherwise an error code.
     */
    virtual IMS_RESULT RespondToEarlyUpdate(IN IMS_SINT32 eStatusCode) = 0;

    /**
     * @brief Sends an ACK request to acknowledge the final response to an INVITE.
     *
     * @return IMS_SUCCESS if the ACK is successfully sent, otherwise an error code.
     */
    virtual IMS_RESULT SendAck() = 0;

    /**
     * @brief Accepts an incoming session request.
     *
     * Sends a 200 OK response to the initial INVITE.
     *
     * @return IMS_SUCCESS if the acceptance is successfully processed, otherwise an error code.
     */
    virtual IMS_RESULT Accept() = 0;

    /**
     * @brief Rejects an incoming session request.
     *
     * Sends a non-2xx final response (e.g., 486 Busy Here, 603 Decline).
     *
     * @param objReason The #CallReasonInfo for rejecting the call, including the status code.
     * @return IMS_SUCCESS if the rejection is successfully processed, otherwise an error code.
     */
    virtual IMS_RESULT Reject(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Updates the session parameters (e.g., media, codecs) within an established dialog.
     *
     * Sends a re-INVITE or UPDATE request.
     *
     * @param eUpdateType The #UpdateType requested (e.g., hold, resume, video upgrade).
     * @param bIncludeAlertInfo Indicates if Alert-Info header should be included.
     * @param eMethod The SIP method(#SipMethod) to use for the update (e.g., INVITE or UPDATE).
     * @return IMS_SUCCESS if the update request is successfully initiated, otherwise an error code.
     */
    virtual IMS_RESULT Update(
            IN UpdateType eUpdateType, IN IMS_BOOL bIncludeAlertInfo, IN IMS_SINT32 eMethod) = 0;

    /**
     * @brief Accepts a received session update request (re-INVITE or UPDATE).
     *
     * Sends a 200 OK response to the update request.
     *
     * @return IMS_SUCCESS if the update is successfully accepted, otherwise an error code.
     */
    virtual IMS_RESULT AcceptUpdate() = 0;

    /**
     * @brief Cancels an outgoing update request before receiving a final response.
     *
     * This method is used to cancel an update (re-INVITE or UPDATE) request that was initiated
     * locally, before a final response has been received.
     *
     * @param objReason The reason for the cancellation.(#CallReasonInfo)
     * @return IMS_SUCCESS if the cancellation is successfully initiated, otherwise an error code.
     */
    virtual IMS_RESULT CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Terminates the session.
     *
     * Sends a BYE request if the session is established, or a CANCEL request during setup.
     *
     * @param bUseBye If true, forces using BYE.
     *                If false, sends CANCEL or BYE based on the session state.
     * @param objReason The reason for termination. (#CallReasonInfo)
     * @return IMS_SUCCESS if the termination process is successfully initiated,
     *         otherwise an error code.
     * @see #ISession::TerminateEx
     */
    virtual IMS_RESULT Terminate(IMS_BOOL bUseBye, IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Marks the session as terminated or start-failed.
     *
     * This method is called to update the session's state when it is no longer active,
     * specifically in the following cases:
     * - #ISessionListener::SessionTerminated() is invoked.
     * - #ISessionListener::SessionStartFailed() is invoked.
     */
    virtual void SetSessionTerminatedOrStartFailed() = 0;

    /**
     * @brief Sets the current CallType with the input param.
     *
     * @param eCallType The #CallType to set.
     */
    virtual void SetCallType(IN CallType eCallType) = 0;

    /**
     * @brief Sets the call type after adjusting it based on the session's capabilities.
     *
     * @param eCallType The #CallType to set. It's downgraded if the session doesn't support the
     *                  required features.
     */
    virtual void SetCapableCallType(IN CallType eCallType) = 0;

    /**
     * @brief Gets the current CallType.
     *
     * @return The current #CallType.
     *         If it's after handling an incoming SIP message, it will be from the SIP message.
     *         If #IMtcSession::SetCallType is invoked previously and no handling of incoming SIP
     *         message after that, it will be same as the input param of #IMtcSession::SetCallType.
     */
    virtual CallType GetCallType() const = 0;

    /**
     * @brief Gets the previous CallType.
     *
     * @return The previous #CallType.
     */
    virtual CallType GetPreviousCallType() const = 0;

    /**
     * @brief Gets the underlying ISession instance.
     *
     * @return A reference to the #ISession object associated with this IMtcSession.
     */
    virtual ISession& GetISession() = 0;

    /**
     * @brief Gets the extension set associated with the session.
     *
     * @return A reference to the #MtcExtensionSet object.
     */
    virtual MtcExtensionSet& GetExtensionSet() = 0;

    /**
     * @brief Checks if the session is capable of video.
     *
     * @return IMS_TRUE if video is supported/capable, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsVideoCapable() const = 0;

    /**
     * @brief Checks if the session is capable of RTT (Real-time Text).
     *
     * @return IMS_TRUE if RTT is supported/capable, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsRttCapable() const = 0;

    /**
     * @brief Checks if there's a pending PRACK transaction.
     *
     * @return IMS_TRUE if the PRACK transaction is not completed after receiving a reliable 18x,
     *         otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsPrackPending() const = 0;

    /**
     * @brief Gets the UpdateType of Early UPDATE previously sent and not succeeded yet.
     *
     * @return The #UpdateType. If no previous Early UPDATE sent and not succeeded yet exists,
     *         #UpdateType::NONE.
     */
    virtual UpdateType GetOngoingUpdateType() const = 0;
};

#endif
