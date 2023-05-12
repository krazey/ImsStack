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

#ifndef AUDIO_MEDIA_SESSION_H_
#define AUDIO_MEDIA_SESSION_H_

#include <AudioConfig.h>
#include <MediaQualityThreshold.h>
#include "BaseSession.h"
#include "IJniMedia.h"
#include "audio/AudioDef.h"
#include "audio/AudioProfile.h"
#include "config/AudioConfiguration.h"

using namespace android::telephony::imsmedia;

class AudioMediaSession : public BaseSession
{
public:
    enum
    {
        /** The default state when the instance is just created*/
        STATE_NONE = 0,
        /** The state that the openSession is done */
        STATE_IDLE,
        /** The state that the rtp/rtcp stream is running */
        STATE_LIVE,
        /** The state that the rtp stream is stopped but the rtcp stream is running */
        STATE_PAUSED,
    };

    explicit AudioMediaSession(IN IMS_SINT32 nSlodId = 0);
    virtual ~AudioMediaSession();

    /**
     * @brief Set the negotiation id
     *
     * @param nNegoId The unique identification of the AudioMediaSession instance
     */
    void SetNegoId(IMS_UINTP nNegoId);

    /**
     * @brief Check the negotiation id is same with given parameter
     *
     * @param nNegoId The id to check
     * @return IMS_BOOL Returns IMS_TRUE when it is same, IMS_FALSE when it is different with
     * parameters
     */
    IMS_BOOL IsSameNegoId(IMS_UINTP nNegoId);

    /**
     * @brief Set the audio configuration
     *
     * @param pConfig The AudioConfiguration instance to set
     */
    void SetConfig(IN AudioConfiguration* pConfig);

    /**
     * @brief Set AudioConfig for libimsmedia from src/dest/negotiated profile
     * @param nAccessNetwork : AccessNetwork information
     * @param pLocalProfile : local profile of the SDP negotiation
     * @param pPeerProfile : peer profile of the SDP negotiation
     * @param pNegoProfile : negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for not updated parameter, true for there is updates
     */
    IMS_BOOL UpdateRtpConfig(IN const IMS_UINT32 nAccessNetwork, IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegoProfile);

    /**
     * @brief Update AccessNetwork information in the RtpConfig
     *
     * @param nAccessNetwork : AccessNetwork information
     * return IMS_BOOL : false for parameter is same, true if the parameter changed
     */
    IMS_BOOL UpdateAccessNetwork(IN const IMS_UINT32 nAccessNetwork);

    /**
     * @brief Update MediaQualityThreshold parameters and send it to the java
     *
     * @param bActiveSession Set IMS_TRUE if this session is active
     * @param bEnableRtcp Set IMS_TRUE to enable monitoring Rtcp inacitivity, IMS_FALSE to disable
     * rtcp monitoring
     * @return IMS_BOOL Returns IMS_TRUE when the sending MediaQualityThreshold is done
     * successfully, IMS_FALSE when it is failed with invalid arguments
     */
    IMS_BOOL UpdateMediaQualityThreshold(IN IMS_BOOL bActiveSession, IN IMS_BOOL bEnableRtcp);

    /**
     * @brief Set the local ip address and port number
     *
     * @param objLocalAddr The local ip address
     * @param nPort The local port number
     */
    void SetLocalEndPoint(IN const IpAddress& objLocalAddr, IN IMS_UINT32 nPort);

    /**
     * @brief Get the rtcp is enabled
     */
    IMS_BOOL GetEnabledRtcp();

    /*
     * request OPEN_SESSION with updated AudioConfig
     */
    IMS_BOOL Open();

    /*
     * request MODIFY_SESSION with updated AudioConfig
     */
    IMS_BOOL Modify();

    /*
     * request ADD_CONFIG with updated AudioConfig
     */
    IMS_BOOL Add();

    /*
     * request DELETE_CONFIG with updated AudioConfig
     */
    IMS_BOOL Delete();

    /*
     * request CONFIRM_CONFIG with updated AudioConfig
     */
    IMS_BOOL Confirm();

    /*
     * request CLOSE_SESSION with updated AudioConfig
     */
    IMS_BOOL Close();

    /*
     * request SET_MEDIA_QUALITY with Audio Media qualityThreshold
     */
    IMS_BOOL SetMediaQuality();

    /**
     * @brief Send a dtmf digit to the ImsMedia
     *
     * @param cDtmfCode The digit to send
     * @return IMS_BOOL Return IMS_TRUE, when the send dtmf is done successfully, IMS_FALSE when it
     * is failed
     */
    IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode);

    /**
     * @brief Set Inactivity timer separately
     *
     * @param nTimer Inactivity timer value
     */
    void SetInactivityTimer(IN IMS_UINT32 nTimer);

    /**
     * @brief Get Inactivity timer
     *
     * @param eType Inactivitiy timer type
     * @return IMS_UINT32 Inactivity timer value
     */
    IMS_SINT32 GetInactivityTimer(IN InactivitytimerType eType);

protected:
    AudioConfiguration* m_pConfig;
    MediaQualityThreshold m_objMediaQualityThreshold;
    IpAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
    ImsList<IMS_UINTP> m_listNegoId;
    IMS_SINT32 m_nInactivityTimer;
};

#endif
