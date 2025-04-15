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

#include "ITimer.h"
#include "BaseSession.h"
#include "IJniMedia.h"
#include "audio/AudioProfile.h"

class AudioConfiguration;

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
        /** The state that the RTP/RTCP stream is running */
        STATE_LIVE,
        /** The state that the RTP stream is stopped but the RTCP stream is running */
        STATE_PAUSED,
    };

    enum MEDIA_DIRECTION_ANBR
    {
        /** The media direction when the ANBR is none (default value). */
        DIRECTION_NONE = 0,
        /** The media direction when the ANBR is uplink */
        DIRECTION_UPLINK,
        /** The media direction when the ANBR is downlink */
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
     * Implementation of the ITimerListener interfaces.
     */
    void Timer_TimerExpired(IN ITimer* piTimer) override;

    /**
     * @brief Set the negotiation id
     *
     * @param nNegoId The unique identification of the AudioSession instance
     */
    void SetNegoId(IMS_UINTP nNegoId);

    /**
     * @brief Check the negotiation id is the same with given parameter
     *
     * @param nNegoId The id to check
     * @return IMS_BOOL Returns IMS_TRUE when it is same, IMS_FALSE when it is different with
     * parameters
     */
    IMS_BOOL IsSameNegoId(IMS_UINTP nNegoId);

    /**
     * @brief Set the AudioConfig for the ImsMedia from the src/dest/negotiated profiles.
     * @param nAccessNetwork : The AccessNetwork information
     * @param pLocalProfile : The local profile of the SDP negotiation
     * @param pPeerProfile : The peer profile of the SDP negotiation
     * @param pNegoProfile : The negotiated profile of the SDP negotiation
     * return IMS_BOOL : false for not updated parameter, true for there is updates
     */
    AudioConfig* UpdateRtpConfig(IN const IMS_UINT32 nAccessNetwork, IN AudioProfile* pLocalProfile,
            IN AudioProfile* pPeerProfile, IN AudioProfile* pNegoProfile,
            IN IMS_BOOL bConfirmedSession);

    /**
     * @brief Update the MediaQualityThreshold parameters and send it to the java
     *
     * @param bActiveSession Set IMS_TRUE if this session is active
     * @param bConfirmedSession Set IMS_TRUE if this session is confirmed session
     * @param bEnableRtcp Set IMS_TRUE to enable monitoring the RTCP inactivity, IMS_FALSE to
     * disable the RTCP monitoring
     * @return IMS_BOOL Returns IMS_TRUE when the sending MediaQualityThreshold is done
     * successfully, IMS_FALSE when it is failed with invalid arguments
     */
    IMS_BOOL UpdateMediaQualityThreshold(
            IN IMS_BOOL bActiveSession, IN IMS_BOOL bConfirmedSession, IN IMS_BOOL bEnableRtcp);

    /**
     * @brief Get the RTCP is enabled
     */
    IMS_BOOL GetEnabledRtcp();

    /*
     * Request the OPEN_SESSION with the updated AudioConfig
     */
    IMS_BOOL Open();

    /*
     * Request the MODIFY_SESSION with the updated AudioConfig
     */
    IMS_BOOL Modify();

    /*
     * Request the ADD_CONFIG with the updated AudioConfig
     */
    IMS_BOOL Add();

    /*
     * Request the DELETE_CONFIG with the updated AudioConfig
     */
    IMS_BOOL Delete();

    /*
     * Request the CONFIRM_CONFIG with the updated AudioConfig
     */
    IMS_BOOL Confirm();

    /*
     * Request the CLOSE_SESSION with the updated AudioConfig
     */
    IMS_BOOL Close();

    /**
     * @brief request SET_MEDIA_QUALITY with Audio Media qualityThreshold
     *
     * @param bConfirmedSession Set IMS_TRUE if the session is confirmed
     * @return IMS_BOOL returns IMS_TRUE when the SetMediaQualityThreshold request is triggered
     */
    IMS_BOOL SetMediaQuality(IN IMS_BOOL bConfirmedSession);

    /**
     * @brief Send the DTMF digit to the ImsMedia
     *
     * @param cDtmfCode The digit to send
     * @return IMS_BOOL Return IMS_TRUE, when the DTMF is sent successfully, IMS_FALSE when it is
     * failed
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
     * @param eType Inactivity timer type
     * @return IMS_UINT32 Inactivity timer value
     */
    IMS_SINT32 GetInactivityTimer(IN InactivitytimerType eType);

    /**
     * @brief Update the sdp negotiation result on whether to support the ANBR feature
     *
     * @param bAnbrEnabled The ANBR negotiation result, if it is true, the ANBR feature can be
     * supported on both devices.
     * @return IMS_BOOL Return IMS_TRUE, when the update is done successfully, IMS_FALSE when it is
     * failed
     */
    IMS_BOOL UpdateAnbrEnabledConfig(IN IMS_BOOL bAnbrEnabled);

    /**
     * @brief Notify the received the ANBR information such as media type, bit rate and direction
     * received from the network
     *
     * @param nAnbrMediaType The media type such as audio and video
     * @param nAnbrDirection The media stream direction to change the bit rate
     * @param nAnbrBitRate The bit rate what the network wants to change
     * @return IMS_BOOL Return IMS_TRUE if the parameter is passed successfully, IMS_FALSE if it is
     * failed
     */
    IMS_BOOL NotifyAnbrReceived(
            IN IMS_UINT32 nAnbrMediaType, IN IMS_UINT32 nAnbrDirection, IN IMS_UINT32 nAnbrBitRate);

    /**
     * @brief Set p-early media header
     *
     * @param ePemType The p-early media header value
     */
    void SetMediaPemType(IN MEDIA_PEM_TYPE ePemType);

private:
    IMS_SINT32 ConvertBitrateToCodecMode(IMS_UINT32 nBitRate, IMS_UINT32 nCodecType);
    void NetworkToneTimerExpired();
    IMS_RESULT StartTimer(IN IMS_SINT32 nDuration);
    void StopTimer();
    IMS_SINT32 GetRtpInactivityTimer(IN IMS_BOOL bActiveSession);
    IMS_SINT32 GetRtcpInactivityTimer(IN IMS_BOOL bActiveSession);
    IMS_BOOL IsRtpInactivityForQnsNeeded(IN IMS_BOOL bConfirmedSession);
    AudioConfiguration* GetConfiguration();

protected:
    ImsList<IMS_UINTP> m_listNegoId;
    IMS_SINT32 m_nNetworkToneTimer;
    IMS_SINT32 m_nRtpInactivityTimer;
    IMS_SINT32 m_nRtcpInactivityTimer;
    IMS_BOOL m_bAnbrEnabled;
    ITimer* m_piNetworkToneWaitTimer;
    MEDIA_PEM_TYPE m_ePemType;
};

#endif
