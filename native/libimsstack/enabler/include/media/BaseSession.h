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

#include <RtpConfig.h>
#include <MediaQualityThreshold.h>

#include "ImsList.h"
#include "MediaDef.h"
#include "config/MediaConfiguration.h"

using namespace android::telephony::imsmedia;

class IMediaSessionListener;
class MediaBaseProfile;
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
    virtual void SetConfiguration(IN MediaConfiguration* pConfiguration);

    /**
     * @brief Set the media service type of the session
     *
     * @param eServiceType Defined MEDIA_SERVICE_TYPE in MediaDef.h
     */
    virtual void SetServiceType(MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Get the service type
     *
     * @return MEDIA_SERVICE_TYPE The service type of the current call - default, emergency
     */
    virtual MEDIA_SERVICE_TYPE GetServiceType();

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
     */
    virtual IMS_SINT32 GetState();

    /**
     * @brief Set the session state
     */
    virtual void SetState(IMS_SINT32 nState);

    /**
     * @brief Update AccessNetwork information in the RtpConfig
     *
     * @param nAccessNetwork : AccessNetwork information return IMS_BOOL : Returns false when the
     * parameter is the same or failed to set the RtpConfig, true if the parameter changed
     */
    virtual IMS_BOOL SetAccessNetwork(IN const IMS_UINT32 nAccessNetwork);

    /**
     * @brief Set the ANBR mode
     *
     * @param objAnbrMode The codec mode of the current activated code in EvsParams and AmrParams in
     * the AudioConfig.
     */
    virtual void SetAnbrMode(AnbrMode objAnbrMode);

    /**
     * @brief Set the local ip address and port number
     *
     * @param objLocalAddr The local ip address
     * @param nPort The local port number
     */
    virtual void SetLocalEndPoint(IN const IpAddress& objLocalAddr, IN IMS_UINT32 nPort);

    /** Get the RtpConfig object */
    virtual RtpConfig* GetRtpConfig();

    /** Get local port number */
    virtual IpAddress& GetLocalIpAddress();

    /** Get local port number */
    virtual IMS_SINT32 GetLocalPort();

    /** Get remote port number */
    virtual IMS_SINT32 GetRemotePort();

protected:
    /**
     * @brief Set the remote ip address and port number
     *
     * @param objRemoteAddr The remote ip address
     * @param nPort The remote port number
     */
    void SetRemoteEndPoint(IN const IpAddress& objRemoteAddr, IN IMS_UINT32 nPort);

    IMS_SINT32 m_nSlotId;
    MediaConfiguration* m_pConfiguration;
    MediaQualityThreshold m_objMediaQualityThreshold;
    IpAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
    IMediaSessionListener* m_piMediaSessionListener;
    MediaEnvironment* m_pEnvironment;
    MEDIA_SERVICE_TYPE m_eServiceType;
    RtpConfig* m_pRtpConfig;
    MEDIA_DIRECTION m_ePrevDirection;
    IMS_SINT32 m_nState;
};

#endif
