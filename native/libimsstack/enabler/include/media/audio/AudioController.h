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

#ifndef AUDIO_CONTROLLER_H_
#define AUDIO_CONTROLLER_H_

#include <AudioConfig.h>

#include "IJniMedia.h"
#include "MediaDef.h"

using android::telephony::imsmedia::AudioConfig;

class AudioConfiguration;
class AudioNego;
class AudioSession;
class IMediaSessionListener;

/**
 * @brief Manages audio-related aspects of a media session.
 *
 * This class acts as a central controller for one or more AudioSession objects within a single
 * call. It handles the lifecycle of audio sessions (creation, modification, deletion), processes
 * SDP negotiation results to configure sessions, and manages audio-related operations like sending
 * DTMF tones, handling network changes, and responding to media quality events. It is designed to
 * support complex scenarios like call forking and session updates.
 */

class AudioController
{
public:
    enum AudioCallSessionState
    {
        EARLY_SESSION = 0,
        READY_TO_CONFIRM,   // session just become confirm
        CONFIRMED_SESSION,  // in confirmed already
    };

    AudioController();
    virtual ~AudioController();

    /**
     * @brief Sets the state of the overall call session.
     *
     * This state is used to determine behavior during session updates, especially in early
     * media vs. confirmed call states.
     *
     * @param bConfirmed IMS_TRUE if the call session is confirmed (e.g., 200 OK received),
     *                   IMS_FALSE if it's in an early state.
     */
    virtual void SetCallSessionState(IN IMS_BOOL bConfirmed);

    /**
     * @brief Sends a DTMF digit through all active audio sessions.
     *
     * @param cDtmfCode The DTMF character to send.
     * @return IMS_TRUE if the request is processed, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode);

    /**
     * @brief Creates and registers a new AudioSession.
     *
     * @param pListener A listener to receive media session events.
     * @param nNegoId A unique identifier for the negotiation session associated with this audio
     *        session.
     * @param pConfig The audio configuration to be used for the session.
     * @param eServiceType The service type (e.g., normal, emergency) for this call.
     * @return IMS_TRUE on successful creation, IMS_FALSE on failure (e.g., invalid arguments).
     */
    virtual IMS_BOOL CreateSession(IN IMediaSessionListener* pListener, IN IMS_UINTP nNegoId,
            AudioConfiguration* pConfig, MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Opens a specific audio session, preparing it for media streaming.
     *
     * This sets the local endpoint and sends a request to the media framework to open the
     * underlying resources.
     *
     * @param nNegoId The unique identifier of the audio session to open.
     * @return IMS_TRUE if the open request was sent successfully, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL OpenSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Updates an existing audio session based on the latest SDP negotiation results.
     *
     * This method is a key part of handling session modifications (re-INVITEs). It updates the
     * RTP configuration and media quality thresholds. Depending on the changes and the call state,
     * it may trigger a session modification or confirmation.
     *
     * @param nNegoId The unique identifier of the audio session to update.
     * @param nAccessNetwork The current access network type.
     * @param pNego A shared pointer to the AudioNego object containing the negotiation results.
     * @return IMS_TRUE if the session update was successful or not needed, IMS_FALSE on failure.
     */
    virtual IMS_BOOL UpdateSession(const IN IMS_UINTP nNegoId, const IN IMS_UINT32 nAccessNetwork,
            IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Adds a new configuration to an existing audio session.
     *
     * This is typically used in scenarios like call forking where a new media stream is added.
     *
     * @param nNegoId The unique identifier of the audio session.
     * @param nAccessNetwork The current access network type.
     * @param pNego A shared pointer to the AudioNego object with the new configuration.
     * @return IMS_TRUE if the configuration was added successfully, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL AddSession(IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork,
            IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Confirms a specific audio session and terminates all others.
     *
     * This is used to finalize a call when multiple early media sessions exist (e.g., call
     * forking). The session with the matching nNegoId is kept, and all other sessions are deleted.
     *
     * @param nNegoId The unique identifier of the audio session to confirm.
     * @return IMS_TRUE if the confirmation was successful, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL ConfirmSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Requests a modification of the specified audio session.
     *
     * This typically follows a configuration change that requires updating the media stream.
     *
     * @param nNegoId The unique identifier of the audio session to modify.
     * @return IMS_TRUE if the modify request was sent successfully, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL ModifySession(IN IMS_UINTP nNegoId);

    /**
     * @brief Sends the current media quality thresholds to the media framework for a session.
     *
     * @param nNegoId The unique identifier of the target audio session.
     * @return IMS_TRUE if the request was sent successfully, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL SetMediaQuality(IN IMS_UINTP nNegoId);

    /**
     * @brief Deletes a specific audio session.
     *
     * @param nNegoId The unique identifier of the audio session to delete.
     * @return IMS_TRUE if the session was deleted successfully, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL DeleteSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Closes all managed audio sessions and cleans up resources.
     *
     * @return IMS_TRUE if the close request was sent successfully, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL CloseSession();

    /**
     * @brief Updates the RTP configuration for a specific audio session based on negotiation.
     *
     * @param nNegoId The unique identifier of the audio session.
     * @param nAccessNetwork The current access network type.
     * @param pNego A shared pointer to the AudioNego object with negotiation results.
     * @return IMS_TRUE if the configuration was changed, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL UpdateRtpConfig(IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork,
            IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Updates the local IP address and port to be used for audio sessions.
     *
     * @param pNego A shared pointer to the AudioNego object containing the local endpoint info.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL UpdateLocalAddress(IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Updates the access network for all active audio sessions.
     *
     * If a session is live, this may trigger a session modification to apply network-specific
     * configurations.
     *
     * @param accessNetwork The new access network type.
     * @return IMS_TRUE if any session was successfully updated, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL UpdateAccessNetwork(IN IMS_UINT32 accessNetwork);

    /**
     * @brief Updates the media quality thresholds for a specific session.
     *
     * This determines parameters like RTCP inactivity timers based on the negotiated profile.
     *
     * @param nNegoId The unique identifier of the audio session.
     * @param pNego A shared pointer to the AudioNego object with negotiation results.
     * @return IMS_TRUE if the thresholds were updated, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL UpdateQualityThreshold(
            IN IMS_UINTP nNegoId, IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Get the size of AudioSession list
     *
     * @return IMS_UINT32 the size of list
     */
    virtual IMS_UINT32 GetAudioSessionSize();

    /**
     * @brief Set and update the media direction of the audio session
     *
     * @param eDirection The direction to update the stream
     * @param bRestore If this set IMS_TRUE, update media direction with previous one
     * @return IMS_TRUE if the update stream is completed
     */
    virtual IMS_BOOL UpdateMediaDirection(
            IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bRestore = IMS_FALSE);

    /**
     * @brief Set network tone timer
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param nTimer The network tone timer value
     */
    virtual void SetNetworkToneTimer(IN IMS_UINTP nNegoId, IN IMS_UINT32 nTimer);

    /**
     * @brief Gets the configured value for a specific inactivity timer.
     *
     * @param eType The type of inactivity timer to retrieve.
     * @param nNegoId The unique identifier of the target audio session.
     * @return The timer value in milliseconds, or -1 if not found.
     */
    virtual IMS_SINT32 GetInactivityTimer(IN InactivitytimerType eType, IN IMS_UINTP nNegoId);

    /**
     * @brief Update the sdp negotiation result on whether to support anbr feature
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param bAnbrEnabled The ANBR negotiation result. If true, ANBR is supported by both ends.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL UpdateAnbrEnabledConfig(IN IMS_UINTP nNegoId, IN IMS_BOOL bAnbrEnabled);

    /**
     * @brief Notify the received ANBR information such as mediaType, bitrate and direction received
     * from the network
     *
     * @param nAnbrMediaType The media type (e.g., audio).
     * @param nAnbrDirection The media stream direction (uplink/downlink) to apply the change.
     * @param nAnbrBitrate The new bitrate requested by the network.
     * @return IMS_TRUE if the notification was processed successfully, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL NotifyAnbrReceived(
            IN IMS_UINT32 nAnbrMediaType, IN IMS_UINT32 nAnbrDirection, IN IMS_UINT32 nAnbrBitrate);

    /**
     * @brief Check there is a session opened
     *
     * @return IMS_TRUE if at least one audio session has been opened, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsSessionOpened();

    /**
     * @brief Set p-early media header
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param ePemType The p-early media header value
     */
    virtual void SetMediaPemType(IN IMS_UINTP nNegoId, IN MEDIA_PEM_TYPE ePemType);

    /**
     * @brief Get the media direction of the current active audio session
     *
     * @return MEDIA_DIRECTION The media direction
     */
    virtual MEDIA_DIRECTION GetMediaDirection();

    /**
     * @brief Send RequestRtpReceptionStats message to java to enable the AV sync logic for video
     * call
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @param nReportingIntervalMs The reporting interval for AV sync feature
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    virtual IMS_BOOL RequestRtpReceptionStats(
            IN IMS_UINTP nNegoId, IN IMS_UINT32 nReportingIntervalMs);

protected:
    virtual IMS_BOOL IsAudioConfigChanged(IN AudioConfig* pAudioConfig);

private:
    AudioSession* FindAudioSession(IN IMS_UINTP nNegoId = UNDEFINED_NEGO_ID);
    void ClearSession();

    ImsList<AudioSession*> m_listAudioSession;
    IMS_UINT32 m_eMediaState;
    IMS_UINT32 m_eCallState;
    IpAddress m_objLocalAddr;
    IMS_UINT32 m_nPort;
    IMS_UINTP m_nCurrentActiveNegoId;
    AudioConfig* m_pAudioConfig;
};

#endif
