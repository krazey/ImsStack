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
#ifndef INTERFACE_SESSION_H_
#define INTERFACE_SESSION_H_

#include "IServiceMethod.h"
#include "SipMethod.h"

class ICapabilities;
class IMedia;
class IPublication;
class IReasonHeaderSetter;
class IReference;
class IRefreshListener;
class ISdpReader;
class ISessionDescriptor;
class ISessionListener;
class ISessionParameter;
class ISipClientConnection;
class ISipHeader;
class ISubscription;
class Replaces;

/**
 * @brief This class is a representation of a media exchange between two IMS endpoints.
 *
 * A ISession can be created either locally through calling ICoreService::CreateSession(...),
 * or by a remote user, in which case the session will be passed as a parameter to
 * ICoreServiceListener::SessionInvitationReceived(...).\n
 * A ISession is able to carry media of the type IMedia. IMedia interfaces represent
 * a media connection and not the media/content itself. Note that to be able to start a ISession
 * it MUST include at least one IMedia component, or an ImsException will be thrown.\n
 * The IMS media connections are negotiated through an offer/answer model.
 * The first offer/answer negotiation may take place during session establishment.
 * However, new offers may be sent by any of the session's parties at any time.
 *
 * @see IServiceMethod, ISessionListener, ISessionDescriptor, IMedia
 */
class ISession : public IServiceMethod
{
protected:
    ~ISession() override = default;

public:
    /**
     * @brief This method can be used to accept a session invitation or a session update
     *       depending on the context.
     *
     * It can be used to accept a session invitation in STATE_NEGOTIATING,
     *     - if the remote endpoint has initiated the session,
     *         the session will transit to STATE_ESTABLISHING.
     * It can be used to accept a session update in STATE_RENEGOTIATING,
     *     - if the remote endpoint has initiated the session update,
     *         the session will transit to STATE_REESTABLISHING.
     *
     * If an error occurs, it returns the following:
     *     - ILLEGAL_STATE - Session is not in the state,
     *                      STATE_NEGOTIATING or STATE_RENEGOTIATING
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Accept() = 0;

    /**
     * @brief This method creates a ICapabilities with the remote endpoint.
     *
     * If an error occurs, it returns the following:
     *     - ILLEGAL_STATE - Session is not in the state, STATE_ESTABLISHING
     *
     * @return Pointer to new ICapabilities.
     */
    virtual ICapabilities* CreateCapabilities() = 0;

    /**
     * @brief Creates a IMedia object with a medit type name and adds it to the ISession.
     *
     * The following type names are recognized in this specification:
     *     - BasicUnreliableMedia, BasicReliableMedia, FramedMedia and StreamMedia.
     *
     * If a IMedia is added to an established ISession, the application has the responsibility
     * to call Update() on the ISession.
     *
     * @param strType Type name of IMedia
     * @param nDirection Direction of the IMedia flow\n
     *                   #IMedia#DIRECTION_INACTIVE\n
     *                   #IMedia#DIRECTION_RECEIVE\n
     *                   #IMedia#DIRECTION_SEND\n
     *                   #IMedia#DIRECTION_SEND_RECEIVE
     * @param nCountOfDescriptor Count of the media descriptor on the IMedia
     * @return Pointer to new IMedia.
     */
    virtual IMedia* CreateMedia(IN const AString& strType, IN IMS_SINT32 nDirection,
            IN IMS_SINT32 nCountOfDescriptor = 0) = 0;

    /**
     * @brief This method is used for referring the remote endpoint
     *        to a third party user or service.
     *
     * @param strReferTo URI with an optional display name to refer to
     * @param strReferMethod Reference method to be used by the reference request;
     *                       "INVITE", "BYE", ...
     * @return Pointer to new IReference.
     */
    virtual IReference* CreateReference(
            IN const AString& strReferTo, IN const AString& strReferMethod) = 0;

    /**
     * @brief Returns the IMedia that are part of this ISession.
     *
     * An empty array will be returned if there are no IMedia in the ISession.
     *
     * @return An array of all IMedia in the ISession.
     */
    virtual ImsList<IMedia*> GetMedia() = 0;

    /**
     * @brief Returns the session descriptor associated with this ISession.
     *
     * @return Pointer to ISessionDescriptor.
     */
    virtual ISessionDescriptor* GetSessionDescriptor() = 0;

    /**
     * @brief Returns the current state of this ISession.
     *
     * @return The current state of this ISession.\n
     *         #STATE_CREATED\n
     *         #STATE_INITIATED\n
     *         #STATE_NEGOTIATING\n
     *         #STATE_ESTABLISHING\n
     *         #STATE_ESTABLISHED\n
     *         #STATE_RENEGOTIATING\n
     *         #STATE_REESTABLISHING\n
     *         #STATE_TERMINATING\n
     *         #STATE_TERMINATED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief This method checks if there are changes in this ISession
     *        that have not been negotiated.
     *
     * @return If there is a pending update, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL HasPendingUpdate() const = 0;

    /**
     * @brief This method can be used to reject a session invitation or a session update
     *        depending on the context.
     *
     * When calling this method, the IMS engine will respond with an appropriate SIP status code.
     *
     * It can be used to reject a session invitation in STATE_NEGOTIATING
     *     - if the remote endpoint has initiated the session,
     *          the ISession will transit to STATE_TERMINATED.
     * It can be used to reject a session update in STATE_RENEGOTIATING
     *     - if the remote endpoint has initiated the session update,
     *          the ISession will transit to STATE_ESTABLISHED and the update is discarded.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Reject() = 0;

    /**
     * @brief This method can be used a reject a session invitation or a session update
     *        depending on the context with a specific SIP status code.
     *
     * It can be used to reject a session invitation in STATE_NEGOTIATING
     *     - if the remote endpoint has initiated the session,
     *         the ISession will transit to STATE_TERMINATED.
     *
     * @param nStatusCode The status code\n
     *                    #STATUSCODE_433_ANONYMITY_DISALLOWED\n
     *                    #STATUSCODE_480_TEMPORARILY_UNAVAILABLE\n
     *                    #STATUSCODE_488_NOT_ACCEPABLE_HERE\n
     *                    #STATUSCODE_600_BUSY_EVERYWHERE\n
     *                    #STATUSCODE_603_DECLINE
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Reject(IN IMS_SINT32 nStatusCode) = 0;

    /**
     * @brief This method can be used to reject a session invitation with the SIP status code
     *        "302 Moved Temporarily", along with an alternative contact user address.
     *
     * It can be used to reject a session invitation in STATE_NEGOTIATING
     *     - if the remote endpoint has initiated the session,
     *         the ISession will transit to STATE_TERMINATED.
     *
     * @param strAlternativeUserAddress Alternative SIP or TEL URI
     *                                  where the user wants to be contacted
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RejectWithDiversion(IN const AString& strAlternativeUserAddress) = 0;

    /**
     * @brief Removes a IMedia from the ISession.
     *
     * If a IMedia is removed from an established ISession,
     * the application has the responsibility to call Update() on the ISession.
     *
     * @param piMedia IMedia instance to be removed from the ISession
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RemoveMedia(IN IMedia* piMedia) = 0;

    /**
     * @brief This method removes all updates that have been made to this ISession and to medias
     *        that are part of this ISession that have not been negotiated.
     *
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Restore() = 0;

    /**
     * @brief Sets a listener for this ISession, replacing any previous ISessionListener.
     *
     * A null reference is allowed and has the effect of removing any existing listener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetListener(IN ISessionListener* piListener) = 0;

    /**
     * @brief Starts a session. When this method is called the remote endpoint is invited
     *        to the session.
     *
     * All media MUST be initialized (see rules for the respective media type)
     * for the session to start.
     * The ISession will transit to STATE_NEGOTIATING after calling this method.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Start() = 0;

    /**
     * @brief Terminates or cancel a session. A session that has been started should
     *        always be terminated using this method.
     *
     * The ISession will transit to STATE_TERMINATING.
     *    - If the ISession is STATE_TERMINATING or STATE_TERMINATED,
     *      this method will not do anything.
     *    - If the ISession is STATE_INITIATED,
     *      the ISession will transit directly to STATE_TERMINATED,
     *      the SessionTerminated() callback will not be invoked.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Terminate() = 0;

    /**
     * @brief Synchronizes the session modifications that an application has done
     *        with the remote endpoint.
     *
     * Modifications include adding of media, removal of media, and change of existing media
     * (e.g. directionality).\n
     * The ISession will transit to STATE_RENEGOTIATING after calling this method.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT Update() = 0;

    /**
     * @brief Creates a subscription for the specified event using the existing session dialog.
     *
     * @param strEvent Event package name
     * @return Pointer to new ISubscription.
     */
    virtual ISubscription* CreateSubscription(IN const AString& strEvent) = 0;

    /**
     * @brief Creates a publication for the specified event using the existing session dialog.
     *
     * @param strEvent Event package name
     * @return Pointer to new IPublication.
     */
    virtual IPublication* CreatePublication(IN const AString& strEvent) = 0;

    /**
     * @brief Creates a new mid-call or in-dialog transaction using this ISession.
     *
     * @param objMethod SIP method for this transaction
     * @return Pointer to new ISipClientConnection.
     */
    virtual ISipClientConnection* CreateTransaction(IN const SipMethod& objMethod) = 0;

    /**
     * @brief Returns the configurations for session control.
     *
     * @return The current configuration value.
     */
    virtual IMS_SINT32 GetConfiguration() const = 0;

    /**
     * @brief Returns the Contact header from this session.
     *
     * NOTE: It can be invoked when the dialog is in EARLY or CONFIRMED state.
     *
     * @return The Contact header of this ISession.
     */
    virtual const ISipHeader* GetContactHeader() const = 0;

    /**
     * @brief Returns the replace information(call-id/from-tag/to-tag) from this session.
     *
     * @return Pointer to Replaces of this ISession.
     */
    virtual const Replaces* GetReplaces() const = 0;

    /**
     * @brief Returns the session identification for 3rd-party call control.
     *
     * @return Session id for this ISession.
     */
    virtual const AString& GetSessionId() const = 0;

    /**
     * @brief Returns the reason of session termination.
     *
     * It will be invoked in the TERMINATED state and usually called inside of
     * SessionTerminated(...) method.
     *
     * @return Reason of session termination.\n
     *         #TERMINATION_REASON_UNKNOWN\n
     *         #TERMINATION_REASON_USER_ACTION\n
     *         #TERMINATION_REASON_REMOTE_ACTION\n
     *         #TERMINATION_REASON_REFRESH_408\n
     *         #TERMINATION_REASON_REFRESH_481\n
     *         #TERMINATION_REASON_REFRESH_TXN_TIMEOUT\n
     *         #TERMINATION_REASON_REFRESH_TIMEOUT\n
     *         #TERMINATION_REASON_SERVICE_CLOSED
     */
    virtual IMS_SINT32 GetTerminationReason() const = 0;

    /**
     * @brief Checks if the final response of INVITE request is received via the current dialog
     *        or other forked dialogs.
     *
     * It's only valid when INVITE response is forked and the session
     * is in the early dialog state.\n
     * Generally, this method will be invoked when PRACK can't be sent because the final
     * response of INVITE request is already received.
     *
     * @return If the final response of INVITE request is received, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsFinalResponseReceivedForInitialInviteRequest() const = 0;

    /**
     * @brief Checks if the remote endpoint supports the reliable provisional response
     *        when the incoming session is received.
     *
     * @return If the remote endpoint supports the reliable provisional response, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsReliableProvResponseSupported() const = 0;

    /**
     * @brief Checks if SDP negotiation is allowed for non-RPR message.
     *
     * @return If the SDP negotiation is allowed for non-RPR message, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsSdpNegotiationAllowedForNonRpr() const = 0;

    /**
     * @brief Checks if SDP OA state is in preview mode or not.
     *
     * @return If SDP OA state is in preview mode, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsSdpOaInPreviewMode() const = 0;

    /**
     * @brief This method can be used a reject a session invitation or a session update depending
     *        on the context with a specific SIP status code.
     *
     * All the SIP status code can be used.
     * It can be used to reject a session invitation in STATE_NEGOTIATING
     *     - if the remote endpoint has initiated the session,
     *         the ISession will transit to STATE_TERMINATED.
     *
     * @param nStatusCode SIP status code
     * @param strReasonPhrase SIP reason phrase
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RejectEx(IN IMS_SINT32 nStatusCode,
            IN const AString& strReasonPhrase = AString::ConstNull()) = 0;

    /**
     * @brief Responds to the incoming UPDATE request with the specified status code
     *        and reason phrase.
     *
     * @param nStatusCode SIP status code
     * @param strReason Application-defined any reason phrase;
                        default value is pre-defined string
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RespondToEarlyUpdate(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull()) = 0;

    /**
     * @brief Responds to the incoming PRACK request with the specified status code
     *        and reason phrase.
     *
     * @param nStatusCode SIP status code
     * @param strReason Application-defined any reason phrase;
                        default value is pre-defined string
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT RespondToPrack(
            IN IMS_SINT32 nStatusCode, IN const AString& strReason = AString::ConstNull()) = 0;

    /**
     * @brief Sends an ACK request for the successful final response
     *        to the originated INVITE request.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SendAck() = 0;

    /**
     * @brief Sends a PRACK request for the incoming reliable provisional response.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SendPrack() = 0;

    /**
     * @brief Sends a provisional response (not RPR) to the incoming INVITE request.
     *
     * @param nStatusCode SIP status code
     * @param strReason Application-defined any reason phrase;
     *                  default value is pre-defined string
     * @param nFlags The additional option flags when sending the provisional response\n
     *               #FLAG_REMOVE_RECORD_ROUTES
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SendProvisionalResponse(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_SINT32 nFlags = 0) = 0;

    /**
     * @brief Sends a reliable provisional response (not provisional response) to
     *        the incoming INVITE request.
     *
     * This method will set a Require header with '100rel' and add a RSeq header by default.
     *
     * @param nStatusCode SIP status code
     * @param strReason Application-defined any reason phrase;
     *                  default value is pre-defined string
     * @param bSdp Flag to indicate that the RPR SHOULD contain the SDP if it is available\n
     *             By default, the RPR contains the SDP if it is available
     *             according to the specification.
     * @param nFlags The additional option flags when sending the provisional response\n
     *               #FLAG_REMOVE_RECORD_ROUTES
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SendRpr(IN IMS_SINT32 nStatusCode,
            IN const AString& strReason = AString::ConstNull(), IN IMS_BOOL bSdp = IMS_TRUE,
            IN IMS_SINT32 nFlags = 0) = 0;

    /**
     * @brief Sets the caller preference for the mobile terminated session.
     *
     * It will be used for session refresh, hold/resume operation by MT session.
     *
     * @param objCallerPreference Caller preference to be set (Accept-Contact header format)
     * @return If the caller preference is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetCallerPreference(IN const ImsList<AString>& objCallerPreference) = 0;

    /**
     * @brief Sets the configurations for session control.
     *
     * @param nConfigValue Configuration value to be set
     */
    virtual void SetConfiguration(IN IMS_SINT32 nConfigValue) = 0;

    /**
     * @brief Sets the contact header parameter on the early or confirmed state.
     *
     * @param strParameter Header parameter
     * @param nOperation Operation (0: add or 1: remove)
     * @return If the contact parameter is successfully set, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     * @note CONTACT_HEADER_PARAMETER_CONTROL_FOR_MID_DIALOG_REQUEST
     */
    virtual IMS_RESULT SetContactParameter(IN const AString& strParameter,
            IN IMS_SINT32 nOperation = 0 /* (0: ADD, 1: REMOVE) */) = 0;

    /**
     * @brief Sets the flag if the mid-dialog request needs to be implicitly routed
     *        to the preloaded topmost route address.
     *
     * NOTE: It MUST be invoked when the route set (R-R headers) is not present, and the mid-dialog
     * request needs to be routed to the preloaded topmost address (i.e. P-CSCF or SBC address).
     * This option will be applied when no Route header exists in the request and an INVITE-dialog
     * is in the early/confirmed state.
     *
     * @param bFlag Indicates the flag if the implicit routing is required or not
     * @note IMPLICIT_ROUTING_FOR_MID_DIALOG
     */
    virtual void SetImplicitRoutingRequired(IN IMS_BOOL bFlag) = 0;

    /**
     * @brief Sets the reason of call termination from the application.
     *
     * @param nReason Reason of call termination
     */
    virtual void SetReasonForCallTermination(IN IMS_SINT32 nReason) = 0;

    /**
     * @brief Sets a refresh listener for this ISession, replacing any previous IRefreshListener.
     *
     * A null reference is allowed and has the effect of removing any existing listener.
     *
     * @param piListener Listener to be set
     */
    virtual void SetRefreshListener(IN IRefreshListener* piListener) = 0;

    /**
     * @brief Sets the refresh policy for the session refresh (it's only for 'uas' behavior).
     *
     * This policy will be applied from when the refresh operation is not occurred
     * in the non-refresher side. The session refresher ('uac') will start the refresh operation
     * in the half of the session interval.
     *
     * @param nPolicy Refresh policy to be set\n
     *                #REFRESH_POLICY_NO_REFRESH\n
     *                #REFRESH_POLICY_SPEC\n
     *                #REFRESH_POLICY_REMAIN_TIME\n
     *                #REFRESH_POLICY_RATIO
     * @param nCriteriaInterval Criteria interval to determine the refresh interval
     * @param nValueEorLt Interval value when the refresh duration is equal or less
     *                    than the criteria interval
     * @param nValueGt Interval value when the refresh duration is greater
     *                 than the criteria interval
     */
    virtual void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt) = 0;

    /**
     * @brief Terminates or cancel (including cancel to re-INVITE request) a session.
     *
     * A session that has been started should always be terminated using this method
     * except for cancel case to re-INVITE.\n
     * The ISession will transit to STATE_TERMINATING or STATE_ESTABLISHED.
     *     - If the ISession is STATE_TERMINATING or STATE_TERMINATED,
     *       this method will not do anything.
     *     - If the ISession is STATE_INITIATED,
     *       the ISession will transit directly to STATE_TERMINATED,
     *       the SessionTerminated() callback will not be invoked.
     *     - If the ISession is STATE_RENEGOTIATED, it is a update requestor
     *       and the final response to re-INVITE is not received,
     *       CANCEL request will be sent and finally the SessionUpdateFailed()
     *       callback will be invoked.
     *
     * @param bTerminateMethodBye Flag to indicate that BYE method will be used
     *                            to terminate the session (default is CANCEL)
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT TerminateEx(IN IMS_BOOL bTerminateMethodBye = IMS_FALSE) = 0;

    /**
     * @brief Sends an UPDATE request before a final response to the INVITE request is generated.
     *
     * If the session state is STATE_ESTABLISHED, the application MUST use Update() method
     * instead of this method.
     *
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT UpdateEarlyMedia() = 0;

    /**
     * @brief Updates the session without the session modifications.
     *
     * If bSessionRefresh is false, the ISession will transit to STATE_RENEGOTIATING
     * after calling this method. Otherwise, it just generates the session refresh.\n
     * If method is INVALID & bRefresh is false, the re-INVITE is selected as the default.\n
     * If method is INVALID & bRefresh is true, the refresh method will be selected
     * by the configuration.
     *
     * @param nMethod SIP method to be used to update the session
     * @param bSessionRefresh Flag to indicate that the method calling is for session refresh
     * @return If the message is successfully sent, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT UpdateEx(IN IMS_SINT32 nMethod = SipMethod::INVALID,
            IN IMS_BOOL bSessionRefresh = IMS_FALSE) = 0;

    // REFUSE_SDP_OFFER_ANSWER_EXCHANGE {
    /**
     * @brief Creates the SDP information based on the current SDP offer/answer state.
     *
     * It's used to send 580 Precondition Failure response or BYE/CANCEL request
     * when SDP offer/answer exchange is refused by some reasons.
     *
     * Preference:
     *   1) current view if present
     *   2) proposal view if present
     *   3) peer view if present
     *   4) capability
     *
     * @return If the failure SDP is successfully created, returns IMS_SUCCESS.
     *         Otherwise, returns IMS_FAILURE.
     * @note REFUSE_SDP_OFFER_ANSWER_EXCHANGE
     */
    virtual IMS_RESULT CreateFailureSdp() = 0;

    /**
     * @brief Destroys the object if the failure SDP information is created.
     *
     * @note REFUSE_SDP_OFFER_ANSWER_EXCHANGE
     */
    virtual void DestroyFailureSdp() = 0;

    /**
     * @brief Returns an interface of session parameter (SDP).
     *
     * @return Pointer to ISessionParameter for failure SDP.
     * @note REFUSE_SDP_OFFER_ANSWER_EXCHANGE
     */
    virtual ISessionParameter* GetFailureSdp() const = 0;
    // }

    // EARLY_SESSION_MODEL {
    /**
     * @brief Returns ISession interface.
     *
     * If this is not a virtual session, it returns null.
     *
     * @return Pointer to owner ISession.
     * @note EARLY_SESSION_MODEL
     */
    virtual ISession* GetOwnerSession() const = 0;

    /**
     * @brief Returns ISession interface.
     *
     * If this is a virtual session, it returns null.
     * If this is a session and early session model is enabled, it returns a virtual session.
     *
     * @return Pointer to virtual ISession.
     * @note EARLY_SESSION_MODEL
     */
    virtual ISession* GetVirtualSession() const = 0;
    // }

    /**
     * @brief Checks if the session refresh is in progress or not.
     *
     * @return IMS_TRUE if the session refresh is in progress, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsSessionRefreshInProgress() const = 0;

    /**
     * @brief Sets the setter interface for Reason header.
     *
     * @param piSetter The setter interface for Reason header
     */
    virtual void SetReasonHeaderSetter(IN IReasonHeaderSetter* piSetter) = 0;

    /**
     * @brief Returns a SDP reader if a failure response (488 or 606) for INVITE request contains
     *        the SDP body part.
     *
     * This SDP reader contains the SDP body part that is most recently received.
     *
     * @return A SDP reader if a failure response for INVITE request contains the SDP body part,
     *         IMS_NULL otherwise.
     */
    virtual ISdpReader* GetRemoteMediaCapabilities() const = 0;

    /**
     * @brief Checks whether this session received the CANCEL request while the session is in
     *        STATE_ESTABLISHING state (i.e. user accepts the call - 200 OK sent).
     *
     * NOTE: Basically, the remote endpoint will terminate the session using BYE request
     *       after receiving 200 OK for CANCEL request.
     *
     * @return IMS_TRUE if the CANCEL request is received while the session is
     *         in STATE_ESTABLISHING state, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsSessionCanceledOnAccepted() const = 0;

    /**
     * @brief Aborts an early UPDATE transaction sent before the session is established
     *        or reestablished.
     */
    virtual void AbortEarlyUpdateTransaction() = 0;

public:
    /// States of ISession
    enum
    {
        STATE_CREATED = 0,
        STATE_INITIATED = 1,
        STATE_NEGOTIATING = 2,
        STATE_ESTABLISHING = 3,
        STATE_ESTABLISHED = 4,
        STATE_RENEGOTIATING = 5,
        STATE_REESTABLISHING = 6,
        STATE_TERMINATING = 7,
        STATE_TERMINATED = 8
    };

    /// SIP status codes for Rejection
    enum
    {
        STATUSCODE_433_ANONYMITY_DISALLOWED = 433,
        STATUSCODE_480_TEMPORARILY_UNAVAILABLE = 480,
        STATUSCODE_486_BUSY_HERE = 486,
        STATUSCODE_488_NOT_ACCEPTABLE_HERE = 488,
        STATUSCODE_600_BUSY_EVERYWHERE = 600,
        STATUSCODE_603_DECLINE = 603
    };

    /// Defines the reasons of session termination
    enum
    {
        TERMINATION_REASON_INVALID = (-1),

        /// Unknown reason
        TERMINATION_REASON_UNKNOWN = 0,
        /// CANCEL or BYE is sent by the user
        TERMINATION_REASON_USER_ACTION,
        /// CANCEL or BYE is received from the remote user or CSCF
        TERMINATION_REASON_REMOTE_ACTION,
        /// Session refresh was failed by 408 response
        TERMINATION_REASON_REFRESH_408,
        /// Session refresh was failed by 481 response
        TERMINATION_REASON_REFRESH_481,
        /// Session refresh was failed by the transaction timeout
        TERMINATION_REASON_REFRESH_TXN_TIMEOUT,
        /// Session refresh was failed by the session refresh timeout
        TERMINATION_REASON_REFRESH_TIMEOUT,
        /// Session is terminated by the de-registration (by user or network)
        TERMINATION_REASON_SERVICE_CLOSED,

        TERMINATION_REASON_MAX
    };

    /// Policy for session refresh
    enum
    {
        /// No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        /// Default policy; Select the refresh time according to 3GPP spec.
        ///    - nCriteriaInterval : Criteria value for the refresh duration
        ///    - nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100; default 50)
        ///    - nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        /// Set the remain time before it is expired
        ///    - nCriteriaInterval : Criteria value for the refresh duration
        ///    - nValueEorLT : Interval value when the refresh duration is equal or less
        ///              than the criteria interval
        ///    - nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        /// Set the ratio before it is expired
        ///    - nCriteriaInterval : Criteria value for the refresh duration
        ///    - nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100)
        ///    - nValueGT : Ratio when the refresh duration is greater
        ///              than the criteria interval (1 ~ 100)
        /// Ex) Expires: 3600, Ratio: 10
        ///        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };

    /// Option flags for sending SIP request/response
    enum
    {
        FLAG_NONE = 0x0000,
        /// If the application wants to remove Record-Route header
        /// when sending the response
        FLAG_REMOVE_RECORD_ROUTES = 0x0001
    };

    /// Runtime configuration for session control
    enum
    {
        CONFIG_NONE = 0,
        /// RFC 6337, Section 3.1.1\n
        /// This will be applied for MO signaling.
        CONFIG_IGNORE_SDP_IN_SUBSEQUENT_RESPONSE = 1 << 0,
        /// This is to avoid the session update failure when the service is disconnected by re-REG.
        CONFIG_IGNORE_DEREG_ON_SESSION_UPDATE = 1 << 1,
        /// This is to override SIP configuration for SDP handling of non-RPR.
        CONFIG_ALLOW_SDP_NEGOTIATION_ON_NON_RPR = 1 << 2,
        /// This is to indicate that early session model is supported.\n
        /// ex. VRBT (Video RingBack Tone) for KR operators
        CONFIG_SUPPORT_EARLY_SESSION_MODEL = 1 << 3,
        /// This is to specify whether the 100 Trying response needs to be notified to the enabler.
        CONFIG_NOTIFY_100_TRYING_RESPONSE_RECEIVED = 1 << 4,
        /// Specify whether the subsequent SDP answer should be ignored
        /// when SDP OA state is in preview mode.
        CONFIG_IGNORE_SUBSEQUENT_SDP_ANSWER_IN_PREVIEW_MODE = 1 << 5
    };

    /// Index for the most recent response message
    enum
    {
        INDEX_MOST_RECENT_MESSAGE = 0xFFFFFFFF
    };
};

#endif
