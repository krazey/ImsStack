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
#include "helper/ISrvccStateListener.h"
#include "media/MediaNego.h"

class IMediaQosEventListener;
class IMediaReportEventListener;
class IMessage;
class ISession;
enum class CallType;
enum class PemType;
struct MediaInfo;

using NegotiationResult = MediaNego::MediaNegoResult;
using NegotiationState = NEGO_STATE;

class IMtcMediaManager
{
public:
    virtual ~IMtcMediaManager(){};

    /**
     * @brief
     * @param pListener
     */
    virtual void SetMediaReportEventListener(IN IMediaReportEventListener* pListener) = 0;

    /**
     * @brief
     * @param pListener
     */
    virtual void SetQosListener(IN IMediaQosEventListener* pListener) = 0;

    /**
     * @brief Sets or updates the media information for a specific session.
     *
     * If a MediaInfo for the ISession does not exist, it creates a new one.
     * If it exists, this method backs up the current MediaInfo before setting the new
     * information. This backup allows for restoration via RestoreMediaInfo() if a
     * subsequent operation fails.
     *
     * @param objISession The ISession instance to associate the media information with.
     * @param objInfo The new MediaInfo object containing the media attributes to set.
     */
    virtual void SetMediaInfo(IN const ISession& objISession, IN const MediaInfo& objInfo) = 0;

    /**
     * @brief Updates the media direction for a specific media type within a session.
     *
     * This method is used to change the direction of a media stream (audio, video, or text)
     * for a given session. It backs up the current direction before applying the new one,
     * allowing for restoration if a subsequent operation (like SDP negotiation) fails.
     *
     * @param objISession ISession instance whose media direction is to be updated.
     * @param eMediaType The media type to update (e.g., MEDIATYPE_AUDIO, MEDIATYPE_VIDEO).
     * @param eDir The new media direction to set (e.g., DIRECTION_SEND, DIRECTION_INACTIVE).
     */
    virtual void UpdateMediaDirection(
            IN const ISession& objISession, IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir) = 0;

    /**
     * @brief Gets the current media information for a specific session.
     *
     * @param objISession ISession instance to query.
     * @return A constant reference to the MediaInfo object containing the current media attributes
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
     * @param objISession ISession instance whose media information is to be restored.
     */
    virtual void RestoreMediaInfo(IN const ISession& objISession) = 0;

    /**
     * @brief This method is to create a media session for the operation related to Media. And set
     *        the media environment which has NetworkType, ServiceType, and CoreService.
     */
    virtual void CreateMediaSession() = 0;

    /**
     * @brief Destroys the media session.
     *
     * It can be invoked when the call is released or needs to be redialed silently.
     */
    virtual void DestroyMediaSession() = 0;

    /**
     * @brief This method to create a media profile.
     *        It can be called when a dialog is added like outgoing call, incoming call,
     *        forking case, and early-session case(video ring back tone).
     * @param piSession ISession instance is used for managing the media profile.
     * @param bForked Set IMS_TRUE if it is forking case, otherwise set IMS_FALSE.
     * @param bOrigin Set IMS_FALSE if it is an early-session case, otherwise set IMS_TRUE.
     */
    virtual void CreateMediaProfile(
            IN ISession* piSession, IN IMS_BOOL bForked, IN IMS_BOOL bOrigin) = 0;

    /**
     * @brief Destroys the media profile and session media for a given session.
     *
     * @param piSession The ISession instance for which to destroy media resources.
     */
    virtual void DestroyMediaForSession(IN ISession* piSession) = 0;

    /**
     * @brief To check if the ringback tone is played with a locally generated tone.
     * @return If the ringback tone is a locally generated tone, return IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsLocalTone() = 0;

    /**
     * @brief This method calls the media interface API to form SDP attributes.
     * @param piSession ISession instance is used for managing the media profile.
     * @param eCallType To deliver MEDIA_CONTENT_TYPE as a parameter for the media interface API.
     * @return It returns the result of forming SDP attributes - IMS_SUCCESS or IMS_FAILURE.
     */
    virtual IMS_RESULT FormSdp(IN ISession* piSession, IN CallType eCallType,
            IN IMS_BOOL bAnswerForOfferlessReInvite = IMS_FALSE) = 0;

    /**
     * @brief This method calls the media interface API to negotiate SDP.
     * @param piSession ISession instance is used for managing the media profile.
     * @return It returns the result of the negotiation as NegotiationResult.
     */
    virtual NegotiationResult NegotiateSdp(IN ISession* piSession) = 0;

    /**
     * @brief Restore the media when the call fails to convert, hold, and resume.
     * @param piSession ISession instance is used for managing the media profile.
     */
    virtual void RestoreSdp(IN ISession* piSession) = 0;

    /**
     * @brief Finalize the Sdp. It will reset the media negotiation state.
     *
     * @param piSession ISession instance is used for managing the media profile.
     */
    virtual void FinalizeSdp(IN ISession* piSession) = 0;

    /**
     * @brief Update P-Early-Media header value and decide whether local tone should be played or
     *        not through the PemType and SIP status codes in the early dialog state.
     *        This method can be called if the UE receives
     *        - 18x response to INVITE request,
     *        - initial INVITE, PRACK, early UPDATE request,
     *        - and 2xx response to early UPDATE and PRACK request
     *        regardless of whether the SIP message has SDP or not.
     * @param piSession PemType in the MtcMediaManager is managed with ISession instance and it is
     *                  used for finding NegoId to check negotiation state.
     * @param piMessage To get P-Early-Media value from the SIP Message.
     */
    virtual void UpdatePemType(IN ISession* piSession, IN IMessage* piMessage) = 0;

    /**
     * @brief Start or update the media session. In early dialog state, it checks PemType and local
     *        ringback tone to play the media. In confirmed state, set confirmed state to
     *        MediaSession and finalize media to remove the unused offer/answer model. If it plays
     *        the media session, it sets the timer for dynamic network tone including ringback tone
     *        and hold tone.
     * @param piSession ISession instance to get media profile id and PemType.
     * @param piMessage IMessage instance to check SDP body and get SIP response code to the initial
     *                  INVITE request.
     * @param bEarly If the call is in early dialog state, set to IMS_TRUE.
     *               Otherwise it is in confirmed state, set to IMS_FALSE.
     */
    virtual void Run(IN ISession* piSession, IN IMessage* piMessage, IN IMS_BOOL bEarly) = 0;

    /**
     * @brief Set RTP port with the specific media line by force.
     * @param piSession ISession instance to get media profile id.
     * @param eMediaType Media type for setting the RTP port.
     * @param nPort The number to set the port.
     */
    virtual void SetRtpPort(
            IN ISession* piSession, IN IMS_UINT32 eMediaType, IN IMS_UINT32 nPort) = 0;

    /**
     * @brief Gets RTP port with the specific media line by force.
     *
     * @param piSession ISession instance to get media profile id.
     * @param eMediaType Media type for getting the RTP port.
     * @return The port number
     */
    virtual IMS_SINT32 GetRemoteRtpPort(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Sets the conference mode to the MediaSession.
     *        This method is to be called when the call is a conference host or participant.
     */
    virtual void SetConferenceCall() = 0;
    virtual void SetConfirmedSession(IN ISession* piSession) = 0;

    /**
     * @brief Get NegotiationState via MediaSession.
     * @param piSession ISession instance to get media profile id.
     * @return The result of negotiation as NegotiationState.
     */
    virtual NegotiationState GetNegotiationState(IN ISession* piSession) = 0;

    /**
     * @brief Get the negotiated direction via MediaSession.
     * @param piSession ISession instance to get media profile id.
     * @return Negotiated direction.
     */
    virtual IMS_SINT32 GetNegotiatedDirection(
            IN const ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Get the negotiated quality via MediaSession.
     * @param piSession ISession instance to get media profile id.
     * @return Negotiated quality.
     */
    virtual IMS_SINT32 GetNegotiatedQuality(
            IN const ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Get the session type from the media contents via MediaSession.
     * @param piSession ISession instance to get media profile id.
     * @return Session type from the negotiated media contents.
     */
    virtual CallType GetNegotiatedCallType(IN ISession* piSession) = 0;

    /**
     * @brief Gets
     *
     * @param piSession
     * @return
     */
    virtual PemType GetPemType(IN ISession* piSession) = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsAudioInactive() = 0;

    /**
     * @brief Adjusts media direction to respond for an update that doesn't contain offer.
     *
     * @param objISession ISession instance whose media direction is to be updated.
     * @param eCallType Call type to set the media directions in the auto offer.
     */
    virtual void AdjustDirectionForAutoOffer(IN const ISession& objISession, IN CallType eCallType);

    /**
     * @brief Adjusts media direction to respond for a hold or resume request.
     *
     * @param objISession ISession instance whose media direction is to be updated.
     */
    virtual void AdjustDirectionForAutoAnswer(IN const ISession& objISession);

    /**
     * @brief Adjusts media direction for local resource confirmation.
     *
     * @param objISession ISession instance to get media profile id.
     * @param eCallType Call type to set the media directions in the SDP offer.
     */
    virtual void AdjustDirectionForLocalResourceConfirmation(
            IN const ISession& objISession, IN CallType eCallType);

    /**
     * @brief Sets
     *
     * @param eState
     */
    virtual void SetSrvccState(IN SrvccState eState) = 0;

    /**
     * @brief Checks if the session is on hold based on its audio media direction.
     *
     * A session is considered on hold if the audio direction is valid but not
     * `DIRECTION_SEND_RECEIVE`. This typically means the direction is `DIRECTION_INACTIVE`,
     * `DIRECTION_SEND`, or `DIRECTION_RECEIVE`.
     *
     * @param objISession ISession instance to be checked.
     * @return IMS_TRUE if the session is on hold, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsOnHold(IN const ISession& objISession) = 0;

    /**
     * @brief Gets the supported Media Types from SDP Body
     *
     * @param piSession ISession instance to get media profile id.
     * @return Media Types which are supported.
     */
    virtual IMS_UINT32 GetSupportedMediaTypesFromSdp(IN ISession* piSession) = 0;

    /**
     * @brief Checks if the SDP negotiation is done in preview mode
     * @param piSession ISession instance to get media profile id.
     * @return IMS_TRUE if the SDP negotiation is done in preview mode, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsPreviewMode(IN ISession* piSession) const = 0;

    /**
     * @brief Checks if the given session is a forked session.
     *
     * @param piSession The ISession instance to check.
     * @return IMS_TRUE if the session is a forked session, otherwise IMS_FALSE.
     */
    virtual IMS_BOOL IsForkedSession(IN const ISession* piSession) const = 0;
};

#endif
