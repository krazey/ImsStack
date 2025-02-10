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
     * @brief Sets
     *
     * @param objInfo
     */
    virtual void SetMediaInfo(IN const MediaInfo& objInfo) = 0;

    /**
     * @brief Updates
     *
     * @param eMediaType
     * @param eDir
     */
    virtual void UpdateMediaDirection(IN IMS_UINT32 eMediaType, IN IMS_SINT32 eDir) = 0;

    /**
     * @brief Gets
     *
     */
    virtual const MediaInfo& GetMediaInfo() const = 0;

    /**
     * @brief Restores
     *
     */
    virtual void RestoreMediaInfo() = 0;

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
     * @brief Destroys
     *
     * @param piSession
     */
    virtual void DestroyMediaProfile(IN ISession* piSession) = 0;

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

    virtual void SetConferenceCall(/* IN ISession* piSession, */ IN IMS_BOOL bConference) = 0;
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
    virtual IMS_SINT32 GetNegotiatedDirection(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

    /**
     * @brief Get the negotiated quality via MediaSession.
     * @param piSession ISession instance to get media profile id.
     * @return Negotiated quality.
     */
    virtual IMS_SINT32 GetNegotiatedQuality(IN ISession* piSession, IN IMS_UINT32 eMediaType) = 0;

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
     * @param eCallType Call type to set the media directions in the auto offer.
     */
    virtual void AdjustDirectionForAutoOffer(IN CallType eCallType);

    /**
     * @brief Adjusts media direction to respond for a hold or resume request.
     */
    virtual void AdjustDirectionForAutoAnswer();

    /**
     * @brief Adjusts media direction for local resource confirmation.
     *
     * @param eCallType Call type to set the media directions in the SDP offer.
     */
    virtual void AdjustDirectionForLocalResourceConfirmation(IN CallType eCallType);

    /**
     * @brief Sets
     *
     * @param eState
     */
    virtual void SetSrvccState(IN SrvccState eState) = 0;

    /**
     * @brief Checks
     *
     * @return
     */
    virtual IMS_BOOL IsOnHold() = 0;

    /**
     * @brief Gets the supported Media Types from SDP Body
     *
     * @param piSession ISession instance to get media profile id.
     * @return Media Types which are supported.
     */
    virtual IMS_UINT32 GetSupportedMediaTypesFromSdp(IN ISession* piSession) = 0;
};

#endif
