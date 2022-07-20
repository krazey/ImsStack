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

#ifndef _AUDIO_CONTROLLER_H_
#define _AUDIO_CONTROLLER_H_

#include "MediaDef.h"
#include "IMediaSessionListener.h"
#include "config/AudioConfiguration.h"
#include "audio/AudioMediaSession.h"
#include "audio/AudioNego.h"

class AudioController
{
public:
    enum AudioUpdateCondition
    {
        EARLY_SESSION = 0,
        READY_TO_CONFIRM,   // session just become confirm
        CONFIRMED_SESSION,  // in confirmed already
    };

    AudioController();
    ~AudioController();
    /**
     * @brief Get the given session with nego id is in hold state
     *
     * @param nNegoId The identification of AudioMediaSession
     * @return IMS_BOOL Returns IMS_TRUE when the target session is in hold state, IMS_FALSE in it
     * is in live state
     */
    IMS_BOOL IsHoldSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Transition the all AudioMediaSession instances to hold
     *
     * @return IMS_BOOL Returns IMS_TRUE when the changing to hold successfully
     */
    IMS_BOOL HoldSession();

    /**
     * @brief Set the update condition for next transition
     *
     * @param bConfirmed it is IMS_TRUE when the session changed to confirmed session
     */
    void SetConfirmSession(IN IMS_BOOL bConfirmed);

    /**
     * @brief Send dtmf digit to ImsMedia module
     *
     * @param cDtmfCode The digit of dtmf to send
     * @return IMS_BOOL Returns IMS_TRUE when the send message delivered to java correctly,
     * IMS_FALSE when it fails to send to java
     */
    IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode);

    /**
     * @brief Create a AudioMediaSession instance with given parameters
     *
     * @param pListener A listener to IMediaSession
     * @param nNegoId The identification to represent the dialog of the session and it will be the
     * id for the AudioMediaSession instance
     * @param pConfig The configuration instance
     * @return IMS_BOOL Returns IMS_TRUE when the session created successfully, IMS_FALSE when it is
     * failed with invalid arguments
     */
    IMS_BOOL CreateSession(
            IN IMediaSessionListener* pListener, IN IMS_UINTP nNegoId, AudioConfiguration* pConfig);

    /**
     * @brief Send openSession message from the given id of the AudioMediaSession instance
     *
     * @param nNegoId The identification of the target AudioMediaSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL OpenSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Update session and send modifySesion of confirmConfig based on the update condition
     *
     * @param nNegoId The identification of the target AudioMediaSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL UpdateSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Send AddConfig message
     *
     * @param nNegoId The identification of the target AudioMediaSession instance
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL AddSession(IN IMS_UINTP nNegoId, IN AudioNego* pNego);

    /**
     * @brief Send confirmConfig message to java
     *
     * @param nNegoId The identification of the target AudioMediaSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL ConfirmSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Send modifySession message to java
     *
     * @param nNegoId The identification of the target AudioMediaSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL ModifySession(IN IMS_UINTP nNegoId);

    /**
     * @brief Send deleteConfig message to java
     *
     * @param nNegoId The identification of the target AudioMediaSession instance
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL DeleteSession(IN IMS_UINTP nNegoId);

    /**
     * @brief Send closeSession message to java
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     *
     */
    IMS_BOOL CloseSession();

    /**
     * @brief Update rtp config parameters from the negotiation profile
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update
     */
    IMS_BOOL UpdateRtpConfig(IN IMS_UINTP nNegoId, IN AudioNego* pNego);

    /**
     * @brief Update local address from the parameters of the negotiation profile
     *
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update
     */
    IMS_BOOL UpdateLocalAddress(IN AudioNego* pNego);

    /**
     * @brief Update MediaQualityThreshold and send message to java
     *
     * @param nNegoId The identification to get the audio profile from negotiated parameter
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL UpdateQualityThreshold(IN IMS_UINTP nNegoId, IN AudioNego* pNego);

    /**
     * @brief Get the size of AudioMediaSession list
     *
     * @return IMS_UINT32 the size of list
     */
    IMS_UINT32 GetAudioSessionSize();

private:
    AudioMediaSession* FindAudioSession(IN IMS_UINTP nNegoId = IMS_NULL);
    void ClearSession();

    IMSList<AudioMediaSession*> m_listAudioSession;
    IMS_SINT32 m_nAudioSessionState;
    IMS_UINT32 m_eUpdateCondition;
    IPAddress m_objLocalAddr;
    IMS_UINT32 m_nPort;
};

#endif