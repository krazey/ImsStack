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

#ifndef _VIDEO_CONTROLLER_H_
#define _VIDEO_CONTROLLER_H_

#include "MediaDef.h"
#include "IMediaSessionListener.h"
#include "config/VideoConfiguration.h"
#include "video/VideoMediaSession.h"
#include "video/VideoNego.h"

class VideoController
{
public:
    VideoController();
    ~VideoController();
    IMS_BOOL SendMessage(IN IMS_SINT32 nMsg, IN IMS_UINTP pParam);

    /**
     * @brief Create a VideoMediaSession instance with given parameters
     *
     * @param pListener A listener to IMediaSession
     * @param pConfig The configuration instance
     * @return IMS_BOOL Returns IMS_TRUE when the session created successfully, IMS_FALSE when it is
     * failed with invalid arguments
     */
    IMS_BOOL CreateSession(IMediaSessionListener* pListener, VideoConfiguration* pConfig);

    /**
     * @brief Send openSession message from the given id of the VideoMediaSession instance
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL OpenSession();

    /**
     * @brief Update session and send modifySesion of confirmConfig based on the update condition
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
    IMS_BOOL UpdateLocalAddress(IN VideoNego* pNego);

    /**
     * @brief Update rtp config parameters from the negotiation profile
     *
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update
     */
    IMS_BOOL UpdateRtpConfig(IN VideoNego* pNego);

    /**
     * @brief Update AccessNetwork information in the RtpConfig
     *
     * @param nAccessNetwork : AccessNetwork information
     */
    void UpdateAccessNetwork(IN IMS_UINT32 nAccessNetwork);

    /**
     * @brief Update MediaQualityThreshold and send message to java
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    IMS_BOOL UpdateQualityThreshold(IN VideoNego* pNego);

    /**
     * @brief Check there is a session opened
     *
     * @return IMS_BOOL Return IMS_TRUE when there is a session created
     */
    IMS_BOOL IsSessionOpened();

private:
    VideoMediaSession* m_pSession;
    IPAddress m_objLocalAddr;
    IMS_UINT32 m_nPort;
};

#endif