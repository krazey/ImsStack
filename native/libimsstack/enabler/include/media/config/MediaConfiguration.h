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

/**
 * @class MediaConfiguration
 * @brief Base class for holding media configuration settings.
 * @details This class manages common configurations for media sessions, such as ports, bandwidth,
 * and codec settings, which are typically derived from carrier-specific configurations.
 */
class MediaConfiguration
{
public:
    /**
     * @brief Constructs a new MediaConfiguration object.
     *
     * @param eSessionType The type of media session (e.g., audio, video).
     */
    explicit MediaConfiguration(MEDIA_CONTENT_TYPE eSessionType = MEDIA_TYPE_AUDIO);

    /**
     * @brief Destroys the MediaConfiguration object.
     */
    virtual ~MediaConfiguration();
    /**
     * @brief Initializes the configuration by reading carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);

    /**
     * @brief Updates the configuration with new carrier-specific settings.
     *
     * @param piCc A pointer to the carrier configuration interface.
     * @return IMS_TRUE on success, IMS_FALSE on failure.
     */
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc);

    /**
     * @brief Gets the configuration for a specific codec.
     *
     * @param nCodec The identifier of the codec.
     * @return A pointer to the CodecConfig object, or IMS_NULL if not found.
     */
    virtual CodecConfig* GetCodecConfig(IN IMS_UINT32 nCodec) const;

    /**
     * @brief Gets the list of all configured codecs.
     *
     * @return A constant reference to the list of CodecConfig pointers.
     */
    virtual const ImsList<CodecConfig*>& GetCodecConfigs() const;

    /**
     * @brief Gets the media session type.
     *
     * @return The media content type for this configuration.
     */
    virtual MEDIA_CONTENT_TYPE GetSessionType() const;

    /**
     * @brief Gets the start of the RTP port range.
     *
     * @return The starting RTP port number.
     */
    virtual IMS_SINT32 GetPortRtp() const;

    /**
     * @brief Gets the end of the RTP port range.
     *
     * @return The ending RTP port number.
     */
    virtual IMS_SINT32 GetPortRtpEnd() const;

    /**
     * @brief Gets the RTCP port number.
     *
     * @return The RTCP port number, typically the RTP port + 1.
     */
    virtual IMS_SINT32 GetPortRtcp() const;

    /**
     * @brief Gets the RTCP reporting interval for an active call.
     *
     * @return The RTCP interval in seconds.
     */
    virtual IMS_SINT32 GetRtcpIntervalOnActive() const;

    /**
     * @brief Gets the RTCP reporting interval when the call is on hold.
     *
     * @return The RTCP interval in seconds.
     */
    virtual IMS_SINT32 GetRtcpIntervalOnHold() const;

    /**
     * @brief Gets the Application Specific (AS) maximum bandwidth.
     *
     * @return The AS bandwidth in kilobits per second (kbps).
     */
    virtual IMS_SINT32 GetAsBandwidthKbps() const;

    /**
     * @brief Gets the RTCP bandwidth for active data senders (RS).
     *
     * @return The RS bandwidth in bits per second (bps).
     */
    virtual IMS_SINT32 GetRsBandwidthBps() const;

    /**
     * @brief Gets the RTCP bandwidth for receivers (RR).
     *
     * @return The RR bandwidth in bits per second (bps).
     */
    virtual IMS_SINT32 GetRrBandwidthBps() const;

    /**
     * @brief Gets the RTP inactivity timer duration.
     *
     * @return The timer duration in milliseconds.
     */
    virtual IMS_SINT32 GetRtpInactivityTimerMillis() const;

    /**
     * @brief Gets the RTCP inactivity timer duration.
     *
     * @return The timer duration in milliseconds.
     */
    virtual IMS_SINT32 GetRtcpInactivityTimerMillis() const;

    /**
     * @brief Checks if the 'recvonly' media direction is enabled for early media sessions.
     *
     * @return IMS_TRUE if enabled, otherwise IMS_FALSE.
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
    /**
     * @brief Converts a codec type to its corresponding media type.
     *
     * @param nCodec The codec identifier.
     * @return IMS_UINT32 Returns the media type (e.g., AUDIO_MAX, VIDEO_MAX, TEXT_MAX).
     */
    virtual IMS_UINT32 ConvertCodecType(IN IMS_UINT32 nCodec) const;

    virtual void SetPorts(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey);

    virtual void SetRtcpIntervals(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey);

public:
    /** Bandwidth type */
    enum
    {
        /** Application-Specific Maximum Bandwidth (RFC 3556). */
        BW_AS = 1,
        /** RTCP bandwidth for receivers (RFC 3556). */
        BW_RR = 2,
        /** RTCP bandwidth for active data senders (RFC 3556). */
        BW_RS = 3
    };

    /**
     * @brief Defines whether to use local or remote bandwidth values for negotiation.
     */
    enum
    {
        /** Use the local device's configured AS, RS, and RR bandwidth values. */
        BW_OPTION_LOCAL_VALUE = 0,
        /**
         * Use the remote party's AS, RS, and RR values. If SDP negotiation succeeds, the remote
         * values are used for the negotiated profile.
         */
        BW_OPTION_REMOTE_VALUE = 1
    };

    /**
     * @brief Defines capability negotiation options for AVPF (Audio-Visual Profile with Feedback)
     * in an SDP offer.
     *
     * This controls the inclusion of 'acap' (Attribute Capability) and 'tcap'
     * (Transport Protocol Capability) attributes.
     */
    enum
    {
        /** No capability attributes are included in the SDP offer. */
        CAPNEG_OFFER_NONE = 0,
        /** Only 'tcap' is included in the SDP offer. */
        CAPNEG_OFFER_WITHOUT_ACAP = 1,
        /** Both 'acap' and 'tcap' are included in the SDP offer. */
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
