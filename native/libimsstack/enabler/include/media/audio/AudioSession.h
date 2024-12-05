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

#ifndef AUDIO_SESSION_H_
#define AUDIO_SESSION_H_

#include <AudioConfig.h>
#include <MediaQualityThreshold.h>

#include "ITimer.h"
#include "BaseSession.h"
#include "IJniMedia.h"
#include "audio/AudioDef.h"
#include "audio/AudioProfile.h"
#include "config/AudioConfiguration.h"

using namespace android::telephony::imsmedia;

class AudioSession : public BaseSession, ITimerListener
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

    enum MEDIA_DIRECTION_ANBR
    {
        /** The media direction for Anbr is none (default value). */
        DIRECTION_NONE = 0,
        /** The media direction for Anbr is uplink */
        DIRECTION_UPLINK,
        /** The media direction for Anbr is downlink */
        DIRECTION_DOWNLINK
    };

    enum CodecType_ANBR
    {
        /** Adaptive Multi-Rate */
        CODEC_AMR = 1 << 0,
        /** Adaptive Multi-Rate Wide Band */
        CODEC_AMR_WB = 1 << 1,
        /** Enhanced Voice Services */
        CODEC_EVS = 1 << 2,
        /** G.711 A-law i.e. Pulse Code Modulation using A-law */
        CODEC_PCMA = 1 << 3,
        /** G.711 μ-law i.e. Pulse Code Modulation using μ-law */
        CODEC_PCMU = 1 << 4,
    };

    explicit AudioSession(IN IMS_SINT32 nSlotId = 0);
    virtual ~AudioSession();

    /**
     * implements ITimerListener interfaces.
     */
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    /**
     * @brief Set the service type
     *
     * @param eServiceType The service type of the current call - default, emergency,
     * For testing purpose
     */
    void SetServiceType(MEDIA_SERVICE_TYPE eServiceType);

    /**
     * @brief Get the service type
     *
     * @return MEDIA_SERVICE_TYPE The service type of the current call - default, emergency
     */
    MEDIA_SERVICE_TYPE GetServiceType();

    /**
     * @brief Set the negotiation id
     *
     * @param nNegoId The unique identification of the AudioSession instance
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
     * @brief Set AudioConfig for libpixelimsmedia from src/dest/negotiated profile
     * @param nAccessNetwork : AccessNetwork information
     * @param pLocalProfile : local profile of the SDP negotiation
     * @param pPeerProfile : peer profile of the SDP negotiation
     * @param pNegoProfile : negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for not updated parameter, true for there is updates
     */
    AudioConfig* UpdateRtpConfig(IN const IMS_UINT32 nAccessNetwork, IN AudioProfile* pLocalProfile,
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
     * @param bConfirmedSession Set IMS_TRUE if this session is confirmed session
     * @param bEnableRtcp Set IMS_TRUE to enable monitoring Rtcp inacitivity, IMS_FALSE to disable
     * rtcp monitoring
     * @return IMS_BOOL Returns IMS_TRUE when the sending MediaQualityThreshold is done
     * successfully, IMS_FALSE when it is failed with invalid arguments
     */
    IMS_BOOL UpdateMediaQualityThreshold(
            IN IMS_BOOL bActiveSession, IN IMS_BOOL bConfirmedSession, IN IMS_BOOL bEnableRtcp);

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

    /**
     * @brief request SET_MEDIA_QUALITY with Audio Media qualityThreshold
     *
     * @param bConfirmedSession Set IMS_TRUE if this session is confirmed session
     * @return IMS_BOOL returns IMS_TRUE when the SetMediaQualityThreshold request is triggered
     */
    IMS_BOOL SetMediaQuality(IN IMS_BOOL bConfirmedSession);

    /**
     * @brief Send a dtmf digit to the ImsMedia
     *
     * @param cDtmfCode The digit to send
     * @return IMS_BOOL Return IMS_TRUE, when the send dtmf is done successfully, IMS_FALSE when it
     * is failed
     */
    IMS_BOOL SendDtmf(IN IMS_CHAR cDtmfCode);

    /**
     * @brief Set network tone timer
     *
     * @param nTimer The network tone timer value
     */
    void SetNetworkToneTimer(IN IMS_UINT32 nTimer);

    /**
     * @brief Get Inactivity timer
     *
     * @param eType Inactivitiy timer type
     * @return IMS_UINT32 Inactivity timer value
     */
    IMS_SINT32 GetInactivityTimer(IN InactivitytimerType eType);

    /**
     * @brief Update the sdp negotiation result on whether to support anbr feature
     *
     * @param anbrEnabled Anbr negotiation result, if it is true, anbr feature can be supported on
     * both devices.
     * @return IMS_BOOL Return IMS_TRUE, when the update is done successfully, IMS_FALSE when it is
     * failed
     */
    IMS_BOOL UpdateAnbrEnabledConfig(IN IMS_BOOL anbrEnabled);

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

private:
    IMS_SINT32 ConvertBitrateToCodecMode(IMS_UINT32 bitrate, IMS_UINT32 codecType);
    void NetworkToneTimerExpired();
    IMS_RESULT StartTimer(IN IMS_SINT32 nDuration);
    void StopTimer();
    IMS_SINT32 GetRtpInactivityTimer(IN IMS_BOOL bActiveSession);
    IMS_SINT32 GetRtcpInactivityTimer(IN IMS_BOOL bActiveSession);
    IMS_BOOL IsRtpInactivityForQnsNeeded(IN IMS_BOOL bConfirmedSession);

protected:
    AudioConfiguration* m_pConfig;
    MediaQualityThreshold m_objMediaQualityThreshold;
    IpAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
    ImsList<IMS_UINTP> m_listNegoId;
    IMS_SINT32 m_nNetworkToneTimer;
    IMS_SINT32 m_nRtpInactivityTimer;
    IMS_SINT32 m_nRtcpInactivityTimer;
    IMS_BOOL m_bAnbrEnabled;
    ITimer* m_piNetworkToneWaitTimer;
    MEDIA_SERVICE_TYPE m_eServiceType;
};

#endif
