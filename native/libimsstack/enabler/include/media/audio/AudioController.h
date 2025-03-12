/**
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

#include "MediaDef.h"
#include "IMediaSessionListener.h"
#include "config/AudioConfiguration.h"
#include "audio/AudioSession.h"
#include "audio/AudioNego.h"

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
     * @brief Set the update condition for next transition
     *
     * @param bConfirmed it is IMS_TRUE when the session changed to confirmed session
     */
    void SetCallSessionState(IN IMS_BOOL bConfirmed);

    /**
     * @brief Send dtmf digit to ImsMedia module
     *
     * @param cDtmfCode The digit of dtmf to send
     * @return IMS_BOOL Returns IMS_TRUE when the send message delivered to java correctly,
     * IMS_FALSE when it fails to send to java
     */
    IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode);

    /**
     * @brief Create a AudioSession instance with given parameters
     *
     * @param pListener A listener to IMediaSession
     * @param nNegoId The identification to represent the dialog of the session and it will be the
     * id for the AudioSession instance
     * @param pConfig The configuration instance
     * @param eServiceType The service type for this call - default, emergency
     * @return IMS_BOOL Returns IMS_TRUE when the session created successfully, IMS_FALSE when it is
     * failed with invalid arguments
     */
    IMS_BOOL CreateSession(IN IMediaSessionListener* pListener, IN IMS_UINTP nNegoId,
            AudioConfiguration* pConfig, MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Send openSession message from the given id of the AudioSession instance
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL OpenSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Update session and send modifySesion of confirmConfig based on the update condition
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @param nAccessNetwork AccessNetwork information
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL UpdateSession(const IN IMS_UINTP nNegoId, const IN IMS_UINT32 nAccessNetwork,
            IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Send AddConfig message
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @param nAccessNetwork AccessNetwork information
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL AddSession(IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork,
            IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Send confirmConfig message to java
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL ConfirmSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Send modifySession message to java
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL ModifySession(IN IMS_UINTP nNegoId);

    /**
     * @brief Send SetMediaQuality message to java without any following session changing method
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL SetMediaQuality(IN IMS_UINTP nNegoId);

    /**
     * @brief Send deleteConfig message to java
     *
     * @param nNegoId The identification of the target AudioSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL DeleteSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Send closeSession message to java
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL CloseSession();

    /**
     * @brief Update rtp config parameters from the negotiation profile
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param nAccessNetwork AccessNetwork information
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when there is the parameter to updates IMS_FALSE when there
     * are no parameters updated
     */
    IMS_BOOL UpdateRtpConfig(IN IMS_UINTP nNegoId, IN IMS_UINT32 nAccessNetwork,
            IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Update local address from the parameters of the negotiation profile
     *
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update
     */
    IMS_BOOL UpdateLocalAddress(IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Update AccessNetwork information in the RtpConfig and initiate modifySession
     *
     * @param nAccessNetwork AccessNetwork information
     * @return IMS_BOOL Returns IMS_TRUE when there is the parameter to updates IMS_FALSE when there
     * are no parameters updated
     */
    IMS_BOOL UpdateAccessNetwork(IN IMS_UINT32 accessNetwork);

    /**
     * @brief Update MediaQualityThreshold and send message to java
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL UpdateQualityThreshold(IN IMS_UINTP nNegoId, IN std::shared_ptr<AudioNego> pNego);

    /**
     * @brief Get the size of AudioSession list
     *
     * @return IMS_UINT32 the size of list
     */
    IMS_UINT32 GetAudioSessionSize();

    /**
     * @brief Set and update the media direction of the audio session
     *
     * @param eDirection The direction to update the stream
     * @param bRestore If this set IMS_TRUE, update media direction with previous one
     * @return IMS_TRUE if the update stream is completed
     */
    IMS_BOOL UpdateMediaDirection(IN MEDIA_DIRECTION eDirection, IN IMS_BOOL bRestore = IMS_FALSE);

    /**
     * @brief Set network tone timer
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param nTimer The network tone timer value
     */
    void SetNetworkToneTimer(IN IMS_UINTP nNegoId, IN IMS_UINT32 nTimer);

    /**
     * @brief Get Inactivity timer
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @return IMS_UINT32 Inactivity timer value
     */
    IMS_SINT32 GetInactivityTimer(IN InactivitytimerType eType, IN IMS_UINTP nNegoId);

    /**
     * @brief Update the sdp negotiation result on whether to support anbr feature
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param anbrEnabled Anbr negotiation result, if it is true, anbr feature can be supported on
     * both devices.
     * @return IMS_BOOL Return IMS_TRUE, when the update is done successfully, IMS_FALSE when it is
     * failed
     */
    IMS_BOOL UpdateAnbrEnabledConfig(IN IMS_UINTP nNegoId, IN IMS_BOOL anbrEnabled);

    /**
     * @brief Notify the received ANBR information such as mediaType, bitrate and direction received
     * from the network
     *
     * @param anbrMediaType mediaType such as audio and video
     * @param anbrDirection media stream direction to change the bitrate
     * @param anbrBitrate bitrate the network wants to change
     * @return IMS_BOOL Return IMS_TRUE if the parameter is passed successfully, IMS_FALSE if it is
     * failed
     */
    IMS_BOOL NotifyAnbrReceived(
            IN IMS_UINT32 anbrMediaType, IN IMS_UINT32 anbrDirection, IN IMS_UINT32 anbrBitrate);

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
