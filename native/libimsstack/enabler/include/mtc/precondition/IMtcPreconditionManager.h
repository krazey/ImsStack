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

#ifndef INTERFACE_MTC_PRECONDITION_MANAGER_H_
#define INTERFACE_MTC_PRECONDITION_MANAGER_H_

#include "ImsTypeDef.h"
#include "MediaDef.h"

class IMediaSession;
class IMessage;
class IMtcPreconditionListener;
class ISession;

/**
 * @brief Manages the end-to-end lifecycle of Quality of Service (QoS) preconditions (RFC 3312).
 *
 * This interface is responsible for tracking the status of local and remote resource reservations,
 * modifying Session Description Protocol (SDP) to include precondition attributes, handling
 * network events that affect QoS (like RAT changes), and determining when call setup can proceed
 * based on the reservation status.
 */
class IMtcPreconditionManager
{
public:
    virtual ~IMtcPreconditionManager(){};

    /**
     * @brief Creates and initializes QoS (Quality of Service) resources for a given session.
     *
     * This sets up the necessary data structures (QosTimer, QosData, QosStatusTable) to manage
     * the state of precondition attributes for the session.
     *
     * @param piSession The session for which to create QoS resources.
     */
    virtual void CreateQos(IN ISession* piSession) = 0;

    /**
     * @brief Destroys the QoS resources associated with a given session.
     *
     * This cleans up all data structures that were allocated by CreateQos for the session.
     *
     * @param piSession The session whose QoS resources should be destroyed.
     */
    virtual void DestroyQos(IN ISession* piSession) = 0;

    /**
     * @brief Sets the listener for precondition-related events.
     *
     * The listener will be notified of events like QoS reservation success or failure.
     *
     * @param pListener A pointer to the listener implementation.
     */
    virtual void SetListener(IN IMtcPreconditionListener* pListener) = 0;

    /**
     * @brief Initializes or resets the mobile RAT (Radio Access Technology) information.
     *
     * This function is called on creation and during certain redial scenarios to ensure the
     * manager has the correct current and previous RAT types.
     */
    virtual void InitializeMobileRatInformation() = 0;

    /**
     * @brief Checks if QoS preconditions are supported locally.
     *
     * This is determined by carrier configuration based on the call type (e.g., normal,
     * emergency) and media types involved.
     *
     * @return IMS_TRUE if preconditions are supported, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsPreconditionSupportedInLocal() const = 0;

    /**
     * @brief Checks if a dedicated bearer has been successfully allocated for a specific media
     * type.
     *
     * @param piSession The session to check.
     * @param eMediaType The media type (e.g., MEDIATYPE_AUDIO, MEDIATYPE_VIDEO) to check.
     * @return IMS_TRUE if the dedicated bearer is available, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsDedicatedBearerAllocated(
            IN ISession* piSession, IN IMS_UINT32 eMediaType) const = 0;

    /**
     * @brief Determines if the device must wait for QoS resource reservation to be confirmed
     * before alerting the user of an incoming call.
     *
     * @param piSession The session of the incoming call.
     * @return IMS_TRUE if waiting is required, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsCheckingResourcesRequiredToAlertUser(IN ISession* piSession) const = 0;

    /**
     * @brief Checks if all necessary QoS resources are reserved, making it permissible to alert the
     * user for an incoming call.
     *
     * @param piSession The session to check.
     * @return IMS_TRUE if resources are ready and the user can be alerted, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsAvailableToAlertUser(IN ISession* piSession) const = 0;

    /**
     * @brief Determines whether the device needs to send a confirmation to the network that local
     * QoS resources have been successfully reserved.
     *
     * @param piSession The session to check.
     * @return IMS_TRUE if a confirmation is required, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsLocalResourceConfirmationRequired(IN ISession* piSession) const = 0;

    /**
     * @brief Checks if the conditions are met to send a local resource confirmation message (e.g.,
     * in a PRACK or UPDATE).
     *
     * @param piSession The session to check.
     * @return IMS_TRUE if a confirmation can be sent, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsAvailableToSendLocalResourceConfirmation(IN ISession* piSession) const = 0;

    /**
     * @brief Checks if the SDP for the session contains any QoS precondition attributes.
     *
     * @param piSession The ISession instance to examine.
     * @return IMS_TRUE if QoS precondition attributes are included, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsPreconditionIncludedInSdp(IN ISession* piSession) const = 0;

    /**
     * @brief Modifies the session's SDP to add or update precondition attributes based on the
     * current QoS status.
     *
     * @param piSession The session whose SDP is to be modified.
     * @param bFailure If true, forms an SDP indicating precondition failure.
     */
    virtual void FormPreconditionSdp(IN ISession* piSession, IN IMS_BOOL bFailure) = 0;

    /**
     * @brief Handles an incoming SDP message.
     *
     * It parses precondition attributes from the remote party's SDP, updates the session's QoS
     * state, and may start timers for waiting on dedicated bearers.
     *
     * @param piSession The session that received the SDP.
     */
    virtual void OnSdpReceived(IN ISession* piSession) = 0;

    /**
     * @brief Handles an outgoing SDP message.
     *
     * It may start timers related to waiting for QoS bearers based on the sent SDP.
     *
     * @param piSession The session that sent the SDP.
     * @param bInitialInvite True if this is for the initial INVITE, false otherwise.
     */
    virtual void OnSdpSent(IN ISession* piSession, IN IMS_BOOL bInitialInvite = IMS_FALSE) = 0;

    /**
     * @brief Handles an incoming SIP message.
     *
     * It inspects headers (`Supported`, `Require`) and status codes to update the remote
     * party's precondition support and resource status.
     *
     * @param piSession The session that received the message.
     * @param piMessage The received SIP message.
     */
    virtual void OnMessageReceived(IN ISession* piSession, IN IMessage* piMessage) = 0;

    /**
     * @brief Handles the call established event.
     *
     * It may start timers to wait for QoS for non-audio media types (video, RTT) if they
     * haven't been allocated yet.
     *
     * @param piSession The session that was established.
     */
    virtual void OnCallEstablished(IN ISession* piSession) = 0;

    /**
     * @brief Handles the call modified event (e.g., after a re-INVITE).
     *
     * It updates QoS states, starts timers for new media, and cleans up state for any media
     * that was removed from the session.
     *
     * @param piSession The session that was modified.
     */
    virtual void OnCallModified(IN ISession* piSession) = 0;

    /**
     * @brief Handles a change in the Radio Access Technology (RAT).
     *
     * This manages QoS state transitions during network handovers (e.g., Wi-Fi to LTE, LTE to
     * Wi-Fi) and EPS Fallback scenarios by starting or stopping relevant timers.
     *
     * @param eRatType The new RAT type from the network.
     */
    virtual void OnRatChanged(IN IMS_SINT32 eRatType) = 0;

    /**
     * @brief Updates the QoS status for media types if the underlying media session reports that
     * QoS has become available.
     *
     * @param piSession The session to update.
     * @param nNegoId The ID of the media negotiation.
     * @param eNegotiatedMediaType The bitmap of media types whose QoS status is available.
     * @param piMediaSession The media session providing the QoS status.
     */
    virtual void UpdateQosIfAvailable(IN ISession* piSession, IN IMS_UINTP nNegoId,
            IN MEDIA_CONTENT_TYPE eNegotiatedMediaType, IN IMediaSession* piMediaSession) = 0;

    /**
     * @brief Checks if Audio QoS has ever been available during the call.
     *
     * In a multiple session (forking) environment, this returns true as long as at least one
     * session has acquired QoS.
     * Note that this only returns true upon actual QoS acquisition, independent of default bearer
     * supportability.
     *
     * @return IMS_TRUE if Audio QoS has ever been available for at least one session during the
     * call (independent of default bearer support), IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsAudioQosEverAvailable() const = 0;
};

#endif
