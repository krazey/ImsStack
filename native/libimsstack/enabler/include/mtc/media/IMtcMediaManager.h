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

#ifndef INTERFACE_MTC_MEDIA_MANAGER_H_
#define INTERFACE_MTC_MEDIA_MANAGER_H_

#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "helper/ISrvccStateListener.h"

class IMediaQosEventListener;
class IMediaReportEventListener;
class IMessage;
class ISession;
enum class CallType;
enum class PemType;
struct MediaInfo;

using NegotiationState = NEGO_STATE;

/**
 * @brief This interface defines the core APIs for managing the media aspects of a call.
 *
 * This class is responsible for all media-related operations, including the creation and
 * destruction of media sessions, Session Description Protocol (SDP) negotiation, management of
 * media attributes (e.g., direction, quality), and handling of special scenarios like Quality of
 * Service (QoS) and Single Radio Voice Call Continuity (SRVCC).
 * Each call (#IMtcCall) instance has its own media manager instance.
 */
class IMtcMediaManager
{
public:
    virtual ~IMtcMediaManager(){};

    /**
     * @brief Sets the listener for media report events.
     *
     * @param pListener A pointer to the #IMediaReportEventListener instance.
     */
    virtual void SetMediaReportEventListener(IN IMediaReportEventListener* pListener) = 0;

    /**
     * @brief Sets the listener for QoS (Quality of Service) events.
     *
     * This listener will be notified of QoS-related events during the call.
     *
     * @param pListener A pointer to the #IMediaQosEventListener instance.
     */
    virtual void SetQosListener(IN IMediaQosEventListener* pListener) = 0;

    /**
     * @brief Sets or updates the media information for a specific session.
     *
     * If a #MediaInfo for the #ISession does not exist, it creates a new one.
     * If it exists, this method backs up the current #MediaInfo before setting the new
     * information. This backup allows for restoration via #RestoreMediaInfo() if a
     * subsequent operation fails.
     *
     * @param objISession The #ISession instance to associate the media information with.
     * @param objInfo The new #MediaInfo object containing the media attributes to set.
     */
    virtual void SetMediaInfo(IN const ISession& objISession, IN const MediaInfo& objInfo) = 0;

    /**
     * @brief Updates the media direction for a specific media type within a session.
     *
     * This method is used to change the direction of a media stream (audio, video, or text)
     * for a given session. It backs up the current direction before applying the new one,
     * allowing for restoration if a subsequent operation (like SDP negotiation) fails.
     *
     * @param objISession #ISession instance whose media direction is to be updated.
     * @param eMediaType The media type to update (e.g., #MEDIATYPE_AUDIO, #MEDIATYPE_VIDEO).
     * @param eDir The new media direction to set (e.g., #DIRECTION_SEND, #DIRECTION_INACTIVE).
     */
    virtual void UpdateMediaDirection(
            IN const ISession& objISession, IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir) = 0;

    /**
     * @brief Gets the current media information for a specific session.
     *
     * @param objISession #ISession instance to query.
     * @return A constant reference to the #MediaInfo object containing the current media attributes
     *         (direction, quality, etc.).
     */
    virtual const MediaInfo& GetMediaInfo(IN const ISession& objISession) const = 0;

    /**
     * @brief Restores the media information of a session to its previous state.
     *
     * This is typically used to revert media attribute changes (e.g., direction) when a
     * subsequent operation like SDP negotiation fails. It copies the backed-up 'old' media
     * information back to the 'current' media information.
     *
     * @param objISession #ISession instance whose media information is to be restored.
     */
    virtual void RestoreMediaInfo(IN const ISession& objISession) = 0;

    /**
     * @brief Creates a media session for media-related operations.
     *
     * Sets the media environment, including NetworkType, ServiceType, and CoreService.
     */
    virtual void CreateMediaSession() = 0;

    /**
     * @brief Destroys the media session.
     *
     * This can be invoked when the call is released or needs to be redialed silently.
     */
    virtual void DestroyMediaSession() = 0;

    /**
     * @brief Creates a media profile.
     *
     * This method is called when a dialog is added, such as for an outgoing call, incoming call,
     * forking case, or early-session case (video ring back tone).
     *
     * @param piSession The #ISession instance used for managing the media profile.
     * @param bForked Set to IMS_TRUE if it is a forking case, otherwise IMS_FALSE.
     * @param bOrigin Set to IMS_FALSE if it is an early-session case, otherwise IMS_TRUE.
     */
    virtual void CreateMediaProfile(
            IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOrigin) = 0;

    /**
     * @brief Destroys the media profile and session media for a given session.
     *
     * @param piSession The #ISession instance for which to destroy media resources.
     */
    virtual void DestroyMediaForSession(IN ISession* piSession) = 0;

    /**
     * @brief Checks if the ringback tone is played with a locally generated tone.
     *
     * @return IMS_TRUE if the ringback tone is a locally generated tone, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsLocalTone() = 0;

    /**
     * @brief Forms SDP attributes by calling the media interface API.
     *
     * @param piSession The #ISession instance used for managing the media profile.
     * @param eCallType The #CallType to deliver MEDIA_CONTENT_TYPE as a parameter for the media
     *                  interface API.
     * @return IMS_SUCCESS if the SDP attributes are successfully formed, otherwise IMS_FAILURE.
     */
    virtual IMS_RESULT FormSdp(IN ISession* piSession, IN CallType eCallType,
            IN IMS_BOOL bAnswerForOfferlessReInvite = IMS_FALSE) = 0;

    /**
     * @brief Negotiates SDP by calling the media interface API.
     *
     * @param piSession The #ISession instance used for managing the media profile.
     * @return The result of the negotiation as #SdpNegotiationResult.
     */
    virtual SdpNegotiationResult NegotiateSdp(IN ISession* piSession) = 0;

    /**
     * @brief Restores the media when the call fails to convert, hold, or resume.
     *
     * @param piSession The #ISession instance used for managing the media profile.
     */
    virtual void RestoreSdp(IN ISession* piSession) = 0;

    /**
     * @brief Finalizes the SDP and resets the media negotiation state.
     *
     * @param piSession The #ISession instance used for managing the media profile.
     */
    virtual void FinalizeSdp(IN ISession* piSession) = 0;

    /**
     * @brief Updates the P-Early-Media header value and decides whether a local tone should be
     *        played.
     *
     * This decision is based on the PemType and SIP status codes in the early dialog state.
     * This method can be called if the UE receives:
     * - 18x response to INVITE request,
     * - initial INVITE, PRACK, early UPDATE request,
     * - and 2xx response to early UPDATE and PRACK request,
     * regardless of whether the SIP message has SDP or not.
     *
     * @param piSession The #ISession instance used to manage #PemType and check negotiation state.
     * @param piMessage The #IMessage instance to get the P-Early-Media value from.
     */
    virtual void UpdatePemType(IN ISession* piSession, IN IMessage* piMessage) = 0;

    /**
     * @brief Starts or updates the media session.
     *
     * In the early dialog state, it checks PemType and local ringback tone to play the media.
     * In the confirmed state, it sets the confirmed state to MediaSession and finalizes media to
     * remove the unused offer/answer model. If it plays the media session, it sets the timer for
     * dynamic network tones, including ringback and hold tones.
     *
     * @param piSession The #ISession instance to get media profile ID and #PemType.
     * @param piMessage The #IMessage instance to check SDP body and get SIP response code to the
     * initial INVITE request.
     * @param bEarly Set to IMS_TRUE if the call is in the early dialog state, otherwise IMS_FALSE.
     */
    virtual void Run(IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly) = 0;

    /**
     * @brief Sets the RTP port for a specific media line forcibly.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @param eMediaType The media type for setting the RTP port.
     * @param nPort The port number to set.
     */
    virtual void SetRtpPort(
            IN ISession* piSession, IN IMS_UINT32 eMediaType, IN IMS_UINT32 nPort) = 0;

    /**
     * @brief Gets the RTP port for a specific media line forcibly.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @param eMediaType The media type for getting the RTP port.
     * @return The port number.
     */
    virtual IMS_SINT32 GetRemoteRtpPort(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Sets the conference mode to the MediaSession.
     *
     * This method is called when the call is a conference host or participant.
     */
    virtual void SetConferenceCall() = 0;

    /**
     * @brief Sets the session as confirmed.
     *
     * @param piSession The #ISession instance to be marked as confirmed.
     */
    virtual void SetConfirmedSession(IN ISession* piSession) = 0;

    /**
     * @brief Gets the NegotiationState via MediaSession.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @return The result of negotiation as #NegotiationState.
     */
    virtual NegotiationState GetNegotiationState(IN ISession* piSession) = 0;

    /**
     * @brief Gets the negotiated direction via MediaSession.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @param eMediaType The media type.
     * @return The negotiated direction.
     */
    virtual IMS_SINT32 GetNegotiatedDirection(
            IN const ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Gets the negotiated quality via MediaSession.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @param eMediaType The media type.
     * @return The negotiated quality.
     */
    virtual IMS_SINT32 GetNegotiatedQuality(
            IN const ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Gets the session type from the media contents via MediaSession.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @return The session type from the negotiated media contents.
     */
    virtual CallType GetNegotiatedCallType(IN ISession* piSession) = 0;

    /**
     * @brief Gets the P-Early-Media (PEM) type for a given session.
     *
     * The PEM type indicates the early media handling preference.
     *
     * @param piSession The #ISession instance to query.
     * @return The #PemType associated with the session.
     */
    virtual PemType GetPemType(IN ISession* piSession) = 0;

    /**
     * @brief Checks if the audio media is currently inactive.
     *
     * @return IMS_TRUE if audio is inactive, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsAudioInactive() = 0;

    /**
     * @brief Adjusts media direction to respond for an update that doesn't contain an offer.
     *
     * @param objISession The #ISession instance whose media direction is to be updated.
     * @param eCallType The #CallType to set the media directions in the auto offer.
     */
    virtual void AdjustDirectionForAutoOffer(IN const ISession& objISession, IN CallType eCallType);

    /**
     * @brief Adjusts media direction to respond for a hold or resume request.
     *
     * @param objISession The #ISession instance whose media direction is to be updated.
     */
    virtual void AdjustDirectionForAutoAnswer(IN const ISession& objISession);

    /**
     * @brief Adjusts media direction for local resource confirmation.
     *
     * @param objISession The #ISession instance to get media profile ID.
     * @param eCallType The #CallType to set the media directions in the SDP offer.
     */
    virtual void AdjustDirectionForLocalResourceConfirmation(
            IN const ISession& objISession, IN CallType eCallType);

    /**
     * @brief Sets the SRVCC (Single Radio Voice Call Continuity) state for media handling.
     *
     * This informs the media manager about the current SRVCC state, which might affect
     * how media resources are managed or reconfigured.
     *
     * @param eState The current #SrvccState (e.g., STARTED, SUCCEEDED, FAILED).
     */
    virtual void SetSrvccState(IN SrvccState eState) = 0;

    /**
     * @brief Checks if the session is on hold based on its audio media direction.
     *
     * A session is considered on hold if the audio direction is valid but not
     * #DIRECTION_SEND_RECEIVE. This typically means the direction is #DIRECTION_INACTIVE,
     * #DIRECTION_SEND, or #DIRECTION_RECEIVE.
     *
     * @param objISession #ISession instance to be checked.
     * @return IMS_TRUE if the session is on hold, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsOnHold(IN const ISession& objISession) = 0;

    /**
     * @brief Gets the supported Media Types from the SDP Body.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @return The supported Media Types.
     */
    virtual IMS_UINT32 GetSupportedMediaTypesFromSdp(IN ISession* piSession) = 0;

    /**
     * @brief Checks if the SDP negotiation is done in preview mode.
     *
     * @param piSession The #ISession instance to get media profile ID.
     * @return IMS_TRUE if the SDP negotiation is done in preview mode, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsPreviewMode(IN ISession* piSession) const = 0;

    /**
     * @brief Checks if the given session is a forked session.
     *
     * @param piSession The #ISession instance to check.
     * @return IMS_TRUE if the session is a forked session, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsForkedSession(IN const ISession* piSession) const = 0;

    /**
     * @brief Sets the flag indicating that a 180 Ringing response has been received for the call.
     */
    virtual void Set180Received() = 0;
};

#endif
