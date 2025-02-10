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

#ifndef BASE_SESSION_H_
#define BASE_SESSION_H_

#include "ImsList.h"
#include "MediaDef.h"
#include "config/MediaConfiguration.h"
#include <RtpConfig.h>
using namespace android::telephony::imsmedia;

class IMediaSessionListener;
class MediaEnvironment;

class BaseSession
{
public:
    explicit BaseSession(IN IMS_SINT32 nSlotId = 0);
    virtual ~BaseSession();

    /**
     * @brief Set the text configuration
     *
     * @param pConfiguration The Media(Audio/Video/Text)Configuration instance to set
     */
    void SetConfiguration(IN MediaConfiguration* pConfiguration);

    /**
     * @brief Set the media service type of the sesison
     *
     * @param eServiceType Defined MEDIA_SERVICE_TYPE in MediaDef.h
     */
    virtual void SetServiceType(MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Set the listener to MediaSession
     *
     * @param pListener The listener instance to set
     */
    virtual void SetMediaSessionListener(IN IMediaSessionListener* pListener);

    /**
     * @brief Set the MediaEnvironment instance to get the common parameters of the call
     *
     * @param pEnvironment The instance to set
     */
    virtual void SetMediaEnvironment(MediaEnvironment* pEnvironment);

    /**
     * @brief Set the media direction
     */
    virtual void SetDirection(MEDIA_DIRECTION eDir);

    /**
     * @brief Get the media direction
     */
    virtual MEDIA_DIRECTION GetDirection();

    /**
     * @brief Set the cached media direction
     */
    virtual void SetPrevDirection(MEDIA_DIRECTION eDir);

    /**
     * @brief Get the cached media direction
     */
    virtual MEDIA_DIRECTION GetPrevDirection();

    /**
     * @brief Get the session state
     *
     * @return IMS_SINT32 The state
     */
    virtual IMS_SINT32 GetState();

    /**
     * @brief Set the session state
     *
     * @param state The state to set
     */
    virtual void SetState(IMS_SINT32 state);

    /**
     * @brief Set the AnbrMode parameter
     *
     * @param anbrMode The codec mode of the current activated code in EvsParams and AmrParams
     */
    virtual void SetAnbrMode(AnbrMode AnbrMode);

    /**
     * @brief Update the local ip address and port number
     *
     * @param objLocalAddr The local ip address
     * @param nPort The local port number
     */
    void UpdateLocalEndPoint(IN const IpAddress& objLocalAddr, IN IMS_UINT32 nPort);

protected:
    IMS_SINT32 m_nSlotId;
    MediaConfiguration* m_pConfiguration;
    IpAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
    IMediaSessionListener* m_piMediaSessionListener;
    MediaEnvironment* m_pEnvironment;
    RtpConfig* m_pRtpConfig;
    MEDIA_DIRECTION m_ePrevDirection;
    IMS_SINT32 m_nState;
};

#endif
