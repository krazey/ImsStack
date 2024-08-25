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

#ifndef MEDIA_DEF_H_
#define MEDIA_DEF_H_

#include <IpAddress.h>
#include <list>
#include <algorithm>

inline constexpr IMS_UINTP UNDEFINED_NEGO_ID = 0;

/** Service Type */
typedef enum
{
    MEDIA_SERVICE_NONE = -1,
    MEDIA_SERVICE_DEFAULT = 0,
    MEDIA_SERVICE_EMERGENCY,
} MEDIA_SERVICE_TYPE;

/** Network Type */
typedef enum
{
    MEDIA_NETWORK_NONE = 0x00000000,
    MEDIA_NETWORK_EHRPD = 0x00000001,      // 3GPP2
    MEDIA_NETWORK_WCDMA = 0x00000002,      // 3GPP
    MEDIA_NETWORK_HSPA = 0x00000004,       // 3GPP
    MEDIA_NETWORK_HSPA_PLUS = 0x00000008,  // 3GPP
    MEDIA_NETWORK_LTE = 0x00000010,        // 3GPP
    MEDIA_NETWORK_WIFI = 0x00000020,       // Others
} MEDIA_NETWORK_TYPE;

/** Ip Type */
typedef enum
{
    MEDIA_IP_NONE = -1,
    MEDIA_IPV4 = 0,
    MEDIA_IPV6,
} MEDIA_IPTYPE;

/** Media Content Type */
typedef enum
{
    MEDIA_TYPE_INVALID = 0,
    MEDIA_TYPE_AUDIO = (0x00000001 << 0),
    MEDIA_TYPE_VIDEO = (0x00000001 << 1),
    MEDIA_TYPE_AUDIOVIDEO = MEDIA_TYPE_AUDIO | MEDIA_TYPE_VIDEO,
    MEDIA_TYPE_TEXT = (0x00000001 << 2),
    MEDIA_TYPE_AUDIOTEXT = MEDIA_TYPE_AUDIO | MEDIA_TYPE_TEXT,
    MEDIA_TYPE_VIDEOTEXT = MEDIA_TYPE_VIDEO | MEDIA_TYPE_TEXT,
    MEDIA_TYPE_AUDIOVIDEOTEXT = MEDIA_TYPE_AUDIO | MEDIA_TYPE_VIDEO | MEDIA_TYPE_TEXT,
    MEDIA_TYPE_NOTUSED
} MEDIA_CONTENT_TYPE;

/** Media Direction */
typedef enum
{
    MEDIA_DIRECTION_INVALID = -1,
    MEDIA_DIRECTION_INACTIVE,
    MEDIA_DIRECTION_RECEIVE,
    MEDIA_DIRECTION_SEND,
    MEDIA_DIRECTION_SEND_RECEIVE,
} MEDIA_DIRECTION;

/** Media Service profile */
typedef enum
{
    MEDIA_SERVICE_PROFILE_AVP = 0,
    MEDIA_SERVICE_PROFILE_AVPF = 1,
} MEDIA_SERVICE_PROFILE;

/** Media Transport Type Definition for checking RTP-RTCP Timeout */
typedef enum
{
    /** No Check */
    MEDIA_PROTOCOL_NONE = 0,
    /** RTP or RTCP any */
    MEDIA_PROTOCOL_ANY = 1,
    /** RTP only */
    MEDIA_PROTOCOL_RTP = 2,
    /** RTCP only */
    MEDIA_PROTOCOL_RTCP = 3,
    /** Maintain Previous Setting */
    MEDIA_PROTOCOL_NO_CHANGE = 4,
    // RTP and RTCP both
    MEDIA_PROTOCOL_BOTH = 5,
} MEDIA_TRANSPORT_PROTOCOL;

/** Media Inactivity Call End Reason for checking RTP-RTCP Timeout */
typedef enum
{
    /**  RTCP inactivity occurred when call is on HOLD. */
    RTCP_INACTIVITY_ON_HOLD = 0,
    /**  RTCP inactivity occurred when call is connected. */
    RTCP_INACTIVITY_ON_CONNECTED = 1,
    /**  RTP inactivity occurred when call is connected. */
    RTP_INACTIVITY_ON_CONNECTED = 2,
    /**  E911 RTCP inactivity occurred when call is connected. */
    E911_RTCP_INACTIVITY_ON_CONNECTED = 3,
    /**  E911 RTP inactivity occurred when call is connected. */
    E911_RTP_INACTIVITY_ON_CONNECTED = 4,
} MEDIA_INACTIVITY_CALL_END_REASON;

typedef enum
{
    /** The state that the MediaNego created */
    STATE_IDLE = 0,
    /** The state that SDP offer received */
    STATE_OFFER_RECEIVED,
    /** The state that SDP offer sent */
    STATE_OFFER_SENT,
    /** The state that SDP negotiation is finished */
    STATE_NEGOTIATED,
    /** The state is invalid */
    STATE_NOTUSED
} NEGO_STATE;

typedef enum _REPORT_TYPE
{
    REPORT_INVALID = -1,
    REPORT_SUCCESS = 0,
    // Failures, based on IJniMedia.h - RtpError
    REPORT_FAILURE,
    // No received RTP or RTCP packets
    REPORT_DATA_RECEIVE_FAILED,
    // Notify the first packet received
    REPORT_DATA_RECEIVE_STARTED,
    // Notify the qos callback
    REPORT_QOS,
    // Notify that the video bitrate is decreased under the threshold
    REPORT_VIDEO_LOWEST_BITRATE,
    // Notify that the radio connection is failed
    REPORT_CHECK_RADIO_CONNECTION,
    // Notify that the network rtp packets are received in early media session
    REPORT_NW_TONE_RTP_RECEIVE_STARTED,
    // Notify that the network rtp packets are not received in early media session
    REPORT_NW_TONE_RTP_RECEIVE_FAILED,
    // Notify that the DTMF packets are received
    REPORT_RECEIVED_DTMF_EVENT,
    // Notify that the ImsMedia process disconnected
    REPORT_MEDIA_DETACH,
    // Notify that the ImsMedia triggers ANBR_Query
    REPORT_TRIGGER_ANBR_QUERY,
    // Notify the ANBR negotiation result
    REPORT_ANBR_NEGOTIATION_RESULT,
    REPORT_NOTUSED
} REPORT_TYPE;

enum MEDIA_SRVCC_STATUS
{
    /** SRVCC has no status defined */
    MEDIA_SRVCC_IDLE = -1,
    /** SRVCC is in started state */
    MEDIA_SRVCC_STARTED,
    /** SRVCC is finished successfully */
    MEDIA_SRVCC_SUCCEED,
    /** SRVCC has been failed */
    MEDIA_SRVCC_FAILED,
    /** SRVCC transition has been canceled */
    MEDIA_SRVCC_CANCELED
};

struct QosRequestParam
{
public:
    QosRequestParam(const MEDIA_CONTENT_TYPE type, const IpAddress& address, const IMS_SINT32 port,
            const IMS_BOOL result = IMS_FALSE) :
            m_eMediaType(type),
            m_objIpAddress(address),
            m_nPort(port),
            m_bResult(result)
    {
    }

    bool operator==(const QosRequestParam& param)
    {
        return (m_eMediaType == param.m_eMediaType && m_objIpAddress == param.m_objIpAddress &&
                m_nPort == param.m_nPort);
    }

    QosRequestParam(const QosRequestParam& param) :
            m_eMediaType(param.m_eMediaType),
            m_objIpAddress(param.m_objIpAddress),
            m_nPort(param.m_nPort),
            m_bResult(param.m_bResult)
    {
    }

    void AddNegoId(const IMS_UINTP id)
    {
        std::list<IMS_UINTP>::iterator foundId =
                std::find(m_objListNegoId.begin(), m_objListNegoId.end(), id);
        if (foundId == m_objListNegoId.end())
        {
            m_objListNegoId.push_back(id);
        }
    }

public:
    MEDIA_CONTENT_TYPE m_eMediaType;
    IpAddress m_objIpAddress;
    IMS_SINT32 m_nPort;
    IMS_BOOL m_bResult;
    std::list<IMS_UINTP> m_objListNegoId;
};

#define MEDIA_PORT_INVALID                       (-1)
#define MEDIA_IS_CONTAINED_THIS_TYPE(eDst, eSrc) (((eDst) & (eSrc)) != 0)
#define MEDIA_TYPE_WITHOUT_TEXT(eSrc)            ((eSrc) & (~MEDIA_TYPE_TEXT))

#define MEDIA_DIRECTION_INVOLVED_RECV(eDir) \
    (((eDir) == MEDIA_DIRECTION_SEND_RECEIVE) || ((eDir) == MEDIA_DIRECTION_RECEIVE))
#define MEDIA_DIRECTION_INVOLVED_SEND(eDir) \
    (((eDir) == MEDIA_DIRECTION_SEND_RECEIVE) || ((eDir) == MEDIA_DIRECTION_SEND))
#define IS_VALID_MEDIA_DIRECTION(eDir) \
    (((eDir) >= MEDIA_DIRECTION_INACTIVE) && ((eDir) <= MEDIA_DIRECTION_SEND_RECEIVE))

#define MEDIA_DIRECTION_IS_AUDIO_RUNNABLE(eDir)                                         \
    (((eDir) == MEDIA_DIRECTION_RECEIVE) || ((eDir) == MEDIA_DIRECTION_SEND_RECEIVE) || \
            ((eDir) == MEDIA_DIRECTION_SEND))
#define MEDIA_DIRECTION_IS_VIDEO_RUNNABLE(eDir) \
    ((MEDIA_DIRECTION_RECEIVE <= (eDir)) && ((eDir) <= MEDIA_DIRECTION_SEND_RECEIVE))
#define MEDIA_DIRECTION_IS_TEXT_RUNNABLE(eDir) \
    (((eDir) == MEDIA_DIRECTION_RECEIVE) || ((eDir) == MEDIA_DIRECTION_SEND_RECEIVE))

#define MEDIA_DIRECTION_IS_AUDIO_HOLD(eDir) ((eDir) != MEDIA_DIRECTION_SEND_RECEIVE)
#define MEDIA_DIRECTION_IS_VIDEO_HOLD(eDir) \
    ((eDir) == MEDIA_DIRECTION_INACTIVE || (eDir) == MEDIA_DIRECTION_INVALID)
#define MEDIA_DIRECTION_IS_TEXT_HOLD(eDir) \
    (((eDir) == MEDIA_DIRECTION_SEND) || ((eDir) == MEDIA_DIRECTION_INACTIVE))

#endif
