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

#ifndef VIDEO_CONTROLLER_H_
#define VIDEO_CONTROLLER_H_

#include "IMediaSessionListener.h"
#include "config/VideoConfiguration.h"
#include "video/VideoSession.h"
#include "video/VideoNego.h"

class VideoController
{
public:
    enum VideoCallSessionState
    {
        EARLY_SESSION = 0,
        CONFIRMED_SESSION,  // in confirmed already
    };

    VideoController();
    virtual ~VideoController();

    /**
     * @brief Set the update condition for next transition
     *
     * @param bConfirmed it is IMS_TRUE when the session changed to confirmed session
     */
    void SetCallSessionState(IN IMS_BOOL bConfirmed);

    IMS_BOOL SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);

    /**
     * @brief Create a VideoSession instance with given parameters
     *
     * @param pListener A listener to IMediaSession
     * @param pConfig The configuration instance
     * @return IMS_BOOL Returns IMS_TRUE when the session created successfully, IMS_FALSE when it is
     * failed with invalid arguments
     */
    IMS_BOOL CreateSession(IMediaSessionListener* pListener, VideoConfiguration* pConfig);

    /**
     * @brief Send openSession message from the given id of the VideoSession instance
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL OpenSession();

    /**
     * @brief Update session and send modifySession of confirmConfig based on the update condition
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL UpdateSession();

    /**
     * @brief Send closeSession message to java
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     *
     */
    IMS_BOOL CloseSession();

    /**
     * @brief Update local address from the parameters of the negotiation profile
     *
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update
     */
    IMS_BOOL UpdateLocalAddress(IN std::shared_ptr<VideoNego> pNego);

    /**
     * @brief Update rtp config parameters from the negotiation profile.
     *
     * @param pNego The VideoNego object to get the local, peer and the negotiated profile.
     * @param bHold The option to enable the video hold when the direction is not sendrecv.
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update.
     */
    IMS_BOOL UpdateRtpConfig(IN std::shared_ptr<VideoNego> pNego, IMS_BOOL bHold);

    /**
     * @brief Update AccessNetwork information in the RtpConfig
     *
     * @param nAccessNetwork : AccessNetwork information
     */
    void UpdateAccessNetwork(IN IMS_UINT32 nAccessNetwork);

    /**
     * @brief Set MTU size in the VideoConfig
     *
     * @param nMtu : The MTU size to be set to VideoConfig
     */
    void SetMtu(IN IMS_SINT32 nMtu);

    /**
     * @brief Update MediaQualityThreshold and send message to java
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL UpdateQualityThreshold(IN std::shared_ptr<VideoNego> pNego);

    /**
     * @brief Check there is a session opened
     *
     * @return IMS_BOOL Return IMS_TRUE when there is a session created
     */
    IMS_BOOL IsSessionOpened();

private:
    VideoSession* m_pSession;
    IMS_UINT32 m_eCallState;
    IpAddress m_objLocalAddr;
    IMS_UINT32 m_nPort;
};

#endif
