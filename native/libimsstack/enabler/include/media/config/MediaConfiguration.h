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

#ifndef MEDIA_CONFIGURATION_H_
#define MEDIA_CONFIGURATION_H_

#include "MediaDef.h"
#include "CarrierConfig.h"
#include "ICarrierConfig.h"

class CodecConfig;

/*!
 * @class   MediaConfiguration
 * @brief   MediaConfiguration class for android
 * @details Get audio/video common configurations from DB
 */
class MediaConfiguration
{
public:
    /**
     * @brief Construct a new media configuration
     *
     * @param eSessionType Set media ssession_type (ex: audio, video etc)
     */
    explicit MediaConfiguration(MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_AUDIO);
    /**
     * @brief Destroy the media configuration
     *
     */
    virtual ~MediaConfiguration();
    /**
     * @brief Create codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    /**
     * @brief Update codec using the configuration
     *
     * @param piCc configuration
     * @return IMS_BOOL Return true if the create function is executed without error
     * Return false if the create function is failed
     */
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc);
    /**
     * @brief Get the codec config
     *
     * @param nCodec codec type
     * @return CodecConfig* Return the current codec config
     */
    virtual CodecConfig* GetCodecConfig(IN IMS_UINT32 nCodec) const;
    /**
     * @brief Get the codec configs
     *
     * @return const ImsList<CodecConfig*>& Return the codec config
     */
    virtual const ImsList<CodecConfig*>& GetCodecConfigs() const;

    /**
     * @brief Get media session type
     *
     * @return MEDIA_CONTENT_TYPE Return media session type
     */
    virtual MEDIA_CONTENT_TYPE GetSessionType() const;
    /**
     * @brief Get the start index of the rtp port range
     *
     * @return IMS_SINT32 Return port index
     */
    virtual IMS_SINT32 GetPortRtp() const;
    /**
     * @brief Get the end index of the rtp port range
     *
     * @return IMS_SINT32 Return the end index
     */
    virtual IMS_SINT32 GetPortRtpEnd() const;
    /**
     * @brief Get the rtcp port number (rtp port number + 1)
     *
     * @return IMS_SINT32 Return the rtcp port number
     */
    virtual IMS_SINT32 GetPortRtcp() const;
    /**
     * @brief Get the rtcp interval in active state
     *
     * @return IMS_SINT32 Return the rtcp interval. The timer for this interval runs in seconds.
     */
    virtual IMS_SINT32 GetRtcpIntervalOnActive() const;
    /**
     * @brief Get the rtcp interval in hold state
     *
     * @return IMS_SINT32 Return the rtcp interval. The timer for this interval runs in seconds.
     */
    virtual IMS_SINT32 GetRtcpIntervalOnHold() const;
    /**
     * @brief Get the as bandwidth kbps
     *
     * @return IMS_SINT32 Return as value
     */
    virtual IMS_SINT32 GetAsBandwidthKbps() const;
    /**
     * @brief Get the rs bandwidth bps value
     *
     * @return IMS_SINT32 Return rs value
     */
    virtual IMS_SINT32 GetRsBandwidthBps() const;
    /**
     * @brief Get the Rr Bandwidth bps value
     *
     * @return IMS_SINT32 Return rr value
     */
    virtual IMS_SINT32 GetRrBandwidthBps() const;
    /**
     * @brief Get the rtp inactivity timer in milli seconds unit
     *
     * @return IMS_SINT32 Return the rtp inactivitity timer
     */
    virtual IMS_SINT32 GetRtpInactivityTimerMillis() const;
    /**
     * @brief Get the rtcp inactivity timer in milli seconds unit
     *
     * @return IMS_SINT32 Return the rtcp inactivitity timer
     */
    virtual IMS_SINT32 GetRtcpInactivityTimerMillis() const;
    /**
     * @brief Get the receive only direction feature is enabled in early session.
     *
     * @return IMS_BOOL Return whether the recvonly direction feature is enabled in early session.
     */
    virtual IMS_BOOL IsRecvOnlyEarlySessionEnabled() const;

protected:
    virtual IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc);
    virtual IMS_UINT32 MakeEachCodecs(IN ICarrierConfig* piCc, IN IMS_UINT32 nCodec,
            IN IMS_UINT32 nCodecIndex, IN ImsVector<IMS_SINT32> objPayloadTypeArray);
    virtual IMS_UINT32 MakeCodec(IN ICarrierConfig* piCc, IN IMS_UINT32 nCodec,
            IN IMS_UINT32 nCodecIndex, IN IMS_SINT32 nPayloadTypeNum);
    virtual void ToDebugString() const;
    virtual void ToDebugStringCodecs(IN const CodecConfig* pCodecConfig) const;
    virtual void Clear();
    virtual IMS_UINT32 GetCodecType(IN IMS_UINT32 nCodec) const;
    virtual void SetPorts(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey);
    virtual void SetRtcpIntervals(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey);

public:
    /** Bandwidth type */
    enum
    {
        /** Application Specific Maximum Bandwidth */
        BW_AS = 1,
        /** RTCP bandwidth allocated to active data senders */
        BW_RR = 2,
        /** RTCP bandwidth allocated to other participants in the RTP session */
        BW_RS = 3
    };

    /*
        Bandwidth_option_value
        BW_OPTION_LOCAL_VALUE : Use Local Profile's AS/RS/RR Value
        BW_OPTION_REMOTE_VALUE : Use Remote Profile's AS/RS/RR Value (If sdp negotiationsucceeds,
                the value of remote profile is used as negotiated profile.)
    */
    enum
    {
        BW_OPTION_LOCAL_VALUE = 0,
        BW_OPTION_REMOTE_VALUE = 1
    };

    /** check SDPOfferCapNegForAVPF option
     *  acap (Attribute Capability Attribute)
     *  tcap (Transport Protocol Capability Attribute)
     */
    enum
    {
        /** No capability attribute */
        CAPNEG_OFFER_NONE = 0,
        /** No acap, tcap only */
        CAPNEG_OFFER_WITHOUT_ACAP = 1,
        /** Acap and tcap */
        CAPNEG_OFFER_WITH_ACAP = 2
    };

    static const IMS_SINT32 DEFAULT_RTP_PORT = 50010;
    static const IMS_SINT32 DEFAULT_RTP_PORT_END = 50060;
    static const IMS_SINT32 DEFAULT_RTCP_PORT = 50011;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_ACTIVE = 5;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_HOLD = 5;
    static const IMS_SINT32 DEFAULT_AS = 41;
    static const IMS_SINT32 DEFAULT_RS = 600;
    static const IMS_SINT32 DEFAULT_RR = 2000;
    static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = 20000;
    static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = 20000;

protected:
    MEDIA_CONTENT_TYPE m_eSessionType;
    IMS_SINT32 m_nPortRtp;
    IMS_SINT32 m_nPortRtpEnd;
    IMS_SINT32 m_nPortRtcp;
    IMS_SINT32 m_nRtcpIntervalOnActive;
    IMS_SINT32 m_nRtcpIntervalOnHold;
    IMS_SINT32 m_nAsBandwidthKbps;
    IMS_SINT32 m_nRsBandwidthBps;
    IMS_SINT32 m_nRrBandwidthBps;
    IMS_SINT32 m_nRtpInactivityTimerMillis;
    IMS_SINT32 m_nRtcpInactivityTimerMillis;
    IMS_BOOL m_bRecvOnlyEarlySessionEnabled;

    // Provisioned codecs
    ImsList<CodecConfig*> m_objCodecConfigs;
};

#endif
