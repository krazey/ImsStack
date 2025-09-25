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

#ifndef INTERFACE_MEDIA_SESSION_H_
#define INTERFACE_MEDIA_SESSION_H_

#include "ISession.h"
#include "ImsTypeDef.h"
#include "MediaDef.h"
#include "MediaNego.h"

class IMediaSessionClientListener;

class IMediaSession
{
public:
    enum OptionType
    {
        /** Set the port number of the media session directly from the mtc module */
        SET_RTP_PORT = 0,
        /** Set the session state is confirmed or not */
        SET_CONFIRMED_SESSION,
        /** Set the media direction to update the SDP */
        SET_DIRECTION,
        /** Set the condition that video conference call is enabled */
        SET_CONFERENCE_ENABLE,
        /** Set the video fast update required */
        SEND_FAST_VIDEO_UPDATE,
        SET_DRA_REPORT_OPTION,  // TODO : remove
    };

    /**
     * @brief Destructor of IMediaSession
     */
    virtual ~IMediaSession() {};

    /**
     * @brief Set the mtc session listener
     *
     * @param pISessionListener The listener instance
     */
    virtual void SetMtcListener(IN IMediaSessionClientListener* pISessionListener) = 0;

    /**
     * @brief Create a session instance of the Audio/Video/TextSession. If the argument nego id is
     * not zero, session will be created as a forking session from the session of the negotiated id
     *
     * @param nNegoId The identification of the session
     * @param eMediaType The type of session
     * @return IMS_UINTP Returns identification of the Audio/Video/TextSession instance represents
     * call dialog
     */
    virtual IMS_UINTP CreateProfile(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO) = 0;

    /**
     * @brief Destroy a session instance of the Audio/Video/TextSession with the given negotiation
     * id
     *
     * @param nNegoId The identification of the session
     * @return IMS_BOOL Returns IMS_TRUE when the destroy the profile successfully
     */
    virtual IMS_BOOL DestroyProfile(IN IMS_UINTP nNegoId) = 0;

    /**
     * @brief Form the SDP to the target dialog with the direction parameters for each
     * Audio/Video/TextSession
     *
     * @param nNegoId The identification of the session
     * @param pSession ISession instance to get the SDP descriptor
     * @param eMediaType The type of media
     * @param nAudioDirection The direction of audio m-line in SDP to form
     * @param nVideoDirection The direction of video m-line in SDP to form
     * @param nTextDirection The direction of text m-line in SDP to form
     * @param bEnforceReofferMode To indicate the SDP should be set using full codec capability
     * @return IMS_BOOL Returns IMS_TRUE when form SDP successfully, IMS_FALSE when it is failed
     */
    virtual IMS_BOOL FormSdp(IN IMS_UINTP nNegoId, OUT ISession* pSession,
            IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_SINT32 nAudioDirection,
            IN IMS_SINT32 nVideoDirection, IN IMS_SINT32 nTextDirection = -1,
            IN IMS_BOOL bEnforceReofferMode = IMS_FALSE) = 0;

    /**
     * @brief Get Supported Media Types from SDP
     *
     * @param nNegoId The identification of the session
     * @param pSession ISession instance to get the SDP descriptor
     * @return MEDIA_CONTENT_TYPE The Supported media types
     */
    virtual MEDIA_CONTENT_TYPE GetSupportedMediaTypesFromSdp(
            IN IMS_UINTP nNegoId, IN ISession* pSession) = 0;

    /**
     * @brief Negotiate the SDP to the target dialog with the direction parameters for each
     * Audio/Video/TextSession
     *
     * @param nNegoId The identification of the session
     * @param pSession ISession instance to get the SDP descriptor
     * @param eMediaType The type of media
     * @param nAudioDirection The direction of audio m-line in SDP to negotiate
     * @param nVideoDirection The direction of video m-line in SDP to negotiate
     * @param nTextDirection The direction of text m-line in SDP to negotiate
     * @param errorReason The error reason when the negotiation is failed
     * @return IMS_BOOL Returns IMS_TRUE when negotiate SDP successfully
     */
    virtual IMS_BOOL NegotiateSdp(IN IMS_UINTP nNegoId, IN ISession* pSession,
            OUT IMS_SINT32* nAudioDirection, OUT IMS_SINT32* nVideoDirection,
            OUT IMS_SINT32* nTextDirection, OUT MediaNego::MediaNegoResult& errorReason) = 0;

    /**
     * @brief request to registering QoS callback of the given session to java layer
     *
     * @param nNegoId The identification of the session
     * @param eMediaType The type of media
     * @return IMS_BOOL Returns IMS_TRUE when request Qos is done successfully, IMS_FALSE if the
     * arguments is invalid.
     */
    virtual IMS_BOOL RequestQos(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType = MEDIA_TYPE_AUDIO) = 0;

    /**
     * @brief Cleans up media descriptors that are marked as deleted from the session.
     *
     * This is typically called after a session update is confirmed to remove media
     * lines that are no longer part of the session (e.g., video removed from a call).
     *
     * @param nNegoId The ID of the negotiation context.
     * @param pSession The ISession instance to clean up.
     */
    virtual void FinalizeSdp(IN IMS_UINTP nNegoId, IN ISession* pSession) = 0;

    /**
     * @brief Runs target dialog to operate open/update/close session
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @return IMS_BOOL Returns IMS_TRUE when the opeation is done successfully, IMS_FALSE when it
     * is failed
     */
    virtual IMS_BOOL Run(IN IMS_UINTP nNegoId) = 0;

    /**
     * @brief Terminate the all media session in the call dialogs
     *
     * @return IMS_BOOL Returns IMS_TRUE when the termination is done successfully, IMS_FALSE when
     * it is failed
     */
    virtual IMS_BOOL Terminate() = 0;

    /**
     * @brief Get the negotiation state
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @return NEGO_STATE The nego state
     */
    virtual NEGO_STATE GetNegoState(IN IMS_UINTP nNegoId) = 0;

    /**
     * @brief Get the negotiated media type
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @return MEDIA_CONTENT_TYPE The media type
     */
    virtual MEDIA_CONTENT_TYPE GetNegotiatedMediaType(IN IMS_UINTP nNegoId) = 0;

    /**
     * @brief Get the negotiated media quality
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @param type The negotiated media type
     * @return IMS_SINT32 Returns the quality of the target media type
     */
    virtual IMS_SINT32 GetNegotiatedQuality(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type) = 0;

    /**
     * @brief Get the negotiated codec bitrate
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @param type The negotiated media type
     * @return IMS_SINT32 Returns the bitrate of the negotiated codec
     */
    virtual IMS_SINT32 GetNegotiatedCodecBitrate(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type) = 0;

    /**
     * @brief Get the remote port number
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @param type The media type to retrieve
     * @return IMS_SINT32 Returns the remote port number
     *                    Returns -1 when invalid nNegoId
     */
    virtual IMS_SINT32 GetRemotePort(IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type) = 0;
    /**
     * @brief Get the negotiated direction
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @param type The negotiated media type
     * @return MEDIA_DIRECTION Returns media direction
     */
    virtual MEDIA_DIRECTION GetNegotiatedDirection(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE type) = 0;

    /**
     * @brief Set the additional update for the MediaSession
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @param type The additional option type
     * @param param1 The optional parameter to set, if there is no optional parameter, it is zero
     * @param param1 The optional parameter to set, if there is no optional parameter, it is zero
     */
    virtual void SetOptions(IN IMS_UINTP nNegoId, IN OptionType type, IN IMS_SINT32 param1,
            IN IMS_SINT32 param2) = 0;

    /**
     * @brief Set the timer of waiting to check the rtp stream is received from the network
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @param eMediaType The media type to set the wait timer
     * @param nRtpTimer The time in sec units to set
     */
    virtual void SetNetworkToneRtpTimer(
            IN IMS_UINTP nNegoId, IN MEDIA_CONTENT_TYPE eMediaType, IN IMS_UINT32 nRtpTimer);

    /**
     * @brief Notify the srvcc status to MediaSession to update media stream
     *
     * @param nStatus The status defined in MediaDef.h
     * @return IMS_TRUE when the status update is done in right state. The status of the
     * AudioSession is invalid, returns IMS_FALSE.
     */
    virtual IMS_BOOL NotifySrvccStatus(IN MEDIA_SRVCC_STATUS nStatus) = 0;

    /**
     * @brief Send the message event to the Audio/Video/TextSession
     *
     * @param nMsg The message type
     * @param pParam The message parameter
     * @return IMS_BOOL Returns when the message is delivered to the target instance without error
     */
    virtual IMS_BOOL SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam) = 0;

    /**
     * @brief Set p-early media header
     *
     * @param nNegoId The target Audio/Video/TextSession identification
     * @param ePemType The p-early media header value
     * @return
     */
    virtual void SetMediaPemType(IN IMS_UINTP nNegoId, IN MEDIA_PEM_TYPE ePemType) = 0;

    /**
     * @brief Check if the SDP negotiation is done in preview mode
     * @param nNegoId The ID of the MediaNego instance.
     */
    virtual IMS_BOOL IsPreviewMode(IMS_UINTP nNegoId) = 0;
};

#endif
