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
#ifndef SDP_MEDIA_H_
#define SDP_MEDIA_H_

#include "AStringArray.h"

#include "SdpLine.h"

class SdpMedia : public SdpLine
{
public:
    SdpMedia();
    SdpMedia(IN const SdpMedia& other);
    ~SdpMedia() override;

public:
    SdpMedia& operator=(IN const SdpMedia& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the media line ("m=") in the session description.
     *        The strValue contains a full media line without "m=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the media line ("m=") in the session description.
     *        The returned value contains a full media line with "m=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full media line without "m=".
     */
    AString GetValue() const override;

    /**
     * @brief Returns the media type.
     *
     * @return The media type as predefined enum type.\n
     *         #TYPE_AUDIO\n
     *         #TYPE_VIDEO\n
     *         #TYPE_TEXT\n
     *         #TYPE_APPLICATION\n
     *         #TYPE_MESSAGE\n
     *         #TYPE_OTHER
     */
    inline IMS_SINT32 GetType() const { return m_nMediaType; }

    /**
     * @brief Returns the media type as a string value.
     *
     * @return The media type as string.\n
     *         "audio"\n
     *         "video"\n
     *         "text"\n
     *         "application"\n
     *         "message"
     */
    inline const AString& GetTypeEx() const { return m_strMediaType; }

    /**
     * @brief Returns the transport port to which the media stream is sent.
     */
    inline IMS_SINT32 GetPort() const { return m_nPort; }

    /**
     * @brief Returns the number of transport port.
     */
    inline IMS_SINT32 GetNumOfPort() const { return m_nNumOfPort; }

    /**
     * @brief Returns the transport protocol.
     *
     * @return The transport protocol as predefined enum type.\n
     *         #TRANSPORT_UDP\n
     *         #TRANSPORT_RTP_AVP\n
     *         #TRANSPORT_RTP_AVPF\n
     *         #TRANSPORT_RTP_SAVP\n
     *         #TRANSPORT_RTP_SAVPF\n
     *         #TRANSPORT_UDP_TLS_RTP_SAVP\n
     *         #TRANSPORT_TCP\n
     *         #TRANSPORT_TCP_MSRP\n
     *         #TRANSPORT_TCP_TLS_MSRP\n
     *         #TRANSPORT_OTHER
     */
    inline IMS_SINT32 GetTransportProtocol() const { return m_nTransportProtocol; }

    /**
     * @brief Returns the transport protocol as a string value.
     *
     * @return The transport protocol as string.\n
     *         "udp"\n
     *         "RTP/AVP"\n
     *         "RTP/AVPF"\n
     *         "RTP/SAVP"\n
     *         "RTP/SAVPF"\n
     *         "UDP/TLS/RTP/SAVP"\n
     *         "tcp"\n
     *         "TCP/MSRP"\n
     *         "TCP/TLS/MSRP"
     */
    inline const AString& GetTransportProtocolEx() const { return m_strTransportProtocol; }

    /**
     * @brief Returns the supported media formats.
     */
    inline const AStringArray& GetFormats() const { return m_objFormats; }

    /**
     * @brief Sets the media type to the current media description.
     */
    IMS_BOOL SetType(IN IMS_SINT32 nType, IN const AString& strOtherType = AString::ConstNull());

    /**
     * @brief Sets the transport port to the current media description.
     */
    inline void SetPort(IN IMS_SINT32 nPort) { m_nPort = nPort; }

    /**
     * @brief Sets the number of transport port to the current media description.
     */
    inline void SetNumOfPort(IN IMS_SINT32 nNumOfPort) { m_nNumOfPort = nNumOfPort; }

    /**
     * @brief Sets the transport protocol to the current media description.
     */
    IMS_BOOL SetTransportProtocol(IN IMS_SINT32 nTransportProtocol,
            IN const AString& strOtherTransportProtocol = AString::ConstNull());

    /**
     * @brief Sets the media formats to the current media description.
     */
    IMS_BOOL SetFormats(IN const AStringArray& objFormats);

    /**
     * @brief Sets all the parameters to the current media description.
     */
    IMS_BOOL SetValue(IN IMS_SINT32 nType, IN IMS_SINT32 nPort, IN IMS_SINT32 nTransportProtocol,
            IN const AStringArray& objFormats,
            IN const AString& strOtherType = AString::ConstNull(),
            IN const AString& strOtherTransportProtocol = AString::ConstNull(),
            IN IMS_SINT32 nNumOfPort = 0);

public:
    enum
    {
        TYPE_INVALID = (-1),
        TYPE_AUDIO = 0,
        TYPE_VIDEO,
        TYPE_TEXT,
        TYPE_APPLICATION,
        TYPE_MESSAGE,
        TYPE_OTHER,
        TYPE_MAX
    };

    enum
    {
        TRANSPORT_INVALID = (-1),
        TRANSPORT_UDP = 0,
        TRANSPORT_RTP_AVP,
        // RFC 4585 Extended RTP Profile for RTCP-Based Feedback
        TRANSPORT_RTP_AVPF,
        TRANSPORT_RTP_SAVP,
        TRANSPORT_RTP_SAVPF,
        // RFC 5764 Datagram TLS Extension to Establish Keys for the SRTP
        TRANSPORT_UDP_TLS_RTP_SAVP,
        TRANSPORT_TCP,
        TRANSPORT_TCP_MSRP,
        TRANSPORT_TCP_TLS_MSRP,
        TRANSPORT_OTHER,
        TRANSPORT_MAX
    };

private:
    static const IMS_CHAR* MEDIA[TYPE_MAX];
    static const IMS_CHAR* TRANSPORT[TRANSPORT_MAX];

    // Media type
    IMS_SINT32 m_nMediaType;
    AString m_strMediaType;

    // Transport port
    IMS_SINT32 m_nPort;
    // Number of transport port
    IMS_SINT32 m_nNumOfPort;
    // Transport protocol
    IMS_SINT32 m_nTransportProtocol;
    AString m_strTransportProtocol;

    // Media formats
    AStringArray m_objFormats;
};

#endif
