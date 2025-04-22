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

#ifndef TEXT_CONTROLLER_H_
#define TEXT_CONTROLLER_H_

#include "IMediaSessionListener.h"
#include "config/TextConfiguration.h"
#include "text/TextSession.h"
#include "text/TextNego.h"

class TextController
{
public:
    TextController();
    virtual ~TextController();

    /**
     * @brief Create a TextSession instance with given parameters
     *
     * @param pListener A listener to IMediaSession
     * @param pConfig The configuration instance
     * @return IMS_BOOL Returns IMS_TRUE when the session created successfully, IMS_FALSE when it is
     * failed with invalid arguments
     */
    virtual IMS_BOOL CreateSession(IMediaSessionListener* pListener, TextConfiguration* pConfig);

    /**
     * @brief Send openSession message from the given id of the TextSession instance
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    virtual IMS_BOOL OpenSession();

    /**
     * @brief Update session configuration and send modifySesion
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    virtual IMS_BOOL UpdateSession();

    /**
     * @brief Send closeSession message to java
     *
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     *
     */
    virtual IMS_BOOL CloseSession();

    /**
     * @brief Update rtp config parameters from the negotiation profile
     *
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update
     */
    virtual IMS_BOOL UpdateRtpConfig(IN std::shared_ptr<TextNego> pNego);

    /**
     * @brief Update local address from the parameters of the negotiation profile
     *
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when updates successfully, IMS_FALSE when it is
     * failed to update
     */
    virtual IMS_BOOL UpdateLocalAddress(IN std::shared_ptr<TextNego> pNego);

    /**
     * @brief Update AccessNetwork information in the RtpConfig
     *
     * @param nAccessNetwork : AccessNetwork information
     */
    virtual void UpdateAccessNetwork(IN IMS_UINT32 nAccessNetwork);

    /**
     * @brief Update MediaQualityThreshold and send message to java
     *
     * @param pNego The negotiated profile to get the negotiated parameter
     * @return IMS_BOOL Returns IMS_TRUE when the send message successfully, IMS_FALSE when it is
     * failed to send
     */
    virtual IMS_BOOL UpdateQualityThreshold(IN std::shared_ptr<TextNego> pNego);

    /**
     * @brief Check there is a session opened
     *
     * @return IMS_BOOL Return IMS_TRUE when there is a session created
     */
    virtual IMS_BOOL IsSessionOpened();

private:
    TextSession* m_pSession;
    IpAddress m_objLocalAddr;
    IMS_UINT32 m_nPort;
};

#endif
