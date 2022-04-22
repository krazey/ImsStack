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

#ifndef _MEDIA_DEF_H_
#define _MEDIA_DEF_H_

#include "AString.h"

#define MEDIA_MAX_KEY_LEN 32

// == ENUM VALUES ==========================================================

// Service Type
typedef enum
{
    MEDIA_SERVICE_NONE = -1,
    MEDIA_SERVICE_FMC,                      // = 0
    MEDIA_SERVICE_VOIP,                     // = 1
    MEDIA_SERVICE_VSC,                      // = 2
    MEDIA_SERVICE_VT,                       // = 3
    MEDIA_SERVICE_UC,                       // = 4
    MEDIA_SERVICE_E911,                     // = 5
    MEDIA_SERVICE_NOTUSED,

    // Service Type will be remained only default and emergency
    // or
    // media will not care service type.
    MEDIA_SERVICE_DEFAULT, // = 0
    MEDIA_SERVICE_EMERGENCY,
} MEDIA_SERVICE_TYPE;

// Network Type
typedef enum
{
    MEDIA_NETWORK_NONE = 0x00000000,
    MEDIA_NETWORK_EHRPD = 0x00000001,   // 3GPP2
    MEDIA_NETWORK_WCDMA = 0x00000002,   // 3GPP
    MEDIA_NETWORK_HSPA = 0x00000004,   // 3GPP
    MEDIA_NETWORK_HSPA_PLUS = 0x00000008,   // 3GPP
    MEDIA_NETWORK_LTE = 0x00000010,   // 3GPP
    MEDIA_NETWORK_WIFI = 0x00000020,   // Others
}MEDIA_NETWORK_TYPE;

typedef enum
{
    MEDIA_IP_NONE = -1,
    MEDIA_IPV4 = 0,
    MEDIA_IPV6,
} MEDIA_IPTYPE;

// Content Type
typedef enum
{
    MEDIA_TYPE_INVALID          = 0,
    MEDIA_TYPE_AUDIO            = (0x00000001 << 0),
    MEDIA_TYPE_VIDEO            = (0x00000001 << 1),
    MEDIA_TYPE_AUDIOVIDEO       = MEDIA_TYPE_AUDIO | MEDIA_TYPE_VIDEO,
    MEDIA_TYPE_TEXT             = (0x00000001 << 2),
    MEDIA_TYPE_AUDIOTEXT        = MEDIA_TYPE_AUDIO | MEDIA_TYPE_TEXT,
    MEDIA_TYPE_VIDEOTEXT        = MEDIA_TYPE_VIDEO | MEDIA_TYPE_TEXT,
    MEDIA_TYPE_AUDIOVIDEOTEXT   = MEDIA_TYPE_AUDIO | MEDIA_TYPE_VIDEO | MEDIA_TYPE_TEXT,
    MEDIA_TYPE_NOTUSED
}MEDIA_CONTENT_TYPE;

// Media Direction
typedef enum
{
    MEDIA_DIRECTION_INVALID = -1,
    MEDIA_DIRECTION_INACTIVE,
    MEDIA_DIRECTION_RECEIVE,
    MEDIA_DIRECTION_SEND,
    MEDIA_DIRECTION_SEND_RECEIVE,
    MEDIA_DIRECTION_ACCORDING_TO_NEGO,
    MEDIA_DIRECTION_MAX,
}MEDIA_DIRECTION;

typedef enum
{
    MEDIA_NEGOTIATE_FAIL        = -1,
    MEDIA_NEGOTIATE_NOSDP       = 0,
    MEDIA_NEGOTIATE_LIVE        = 1,
    MEDIA_NEGOTIATE_HOLD        = 2,
    MEDIA_NEGOTIATE_NOTUSED     = 99
}MEDIA_NEGOTIATE_TYPE;

typedef enum
{
    MEDIA_FORM_SDP_LIVE         = 0,
    MEDIA_FORM_SDP_HOLD         = 1,
    MEDIA_FORM_SDP_NOTUSED      = 99
}MEDIA_FORM_SDP_TYPE;

typedef enum
{
    AUDIO_CAL_TYPE_NONE             = 0,
    AUDIO_CAL_TYPE_VOLTE_AMRNB      = 1,
    AUDIO_CAL_TYPE_VOLTE_AMRWB      = 2,
    AUDIO_CAL_TYPE_VT_AMRNB         = 3,
    AUDIO_CAL_TYPE_VT_AMRWB         = 4,
    AUDIO_CAL_TYPE_VOLTE_EVS        = 5,
    AUDIO_CAL_TYPE_VT_EVS           = 6,
    AUDIO_CAL_TYPE_NOTUSED          = 99
} AUDIO_CAL_TYPE;

typedef enum
{
    MODE_NONE           = 0,
    MODE_NEGOCAPA       = 1,
    MODE_FULLCAPA       = 2
}REOFFER_MODE;

// GTT Mode
typedef enum
{
    TEXT_GTT_MODE_INVALID = -1,
    TEXT_GTT_MODE_OFF,
    TEXT_GTT_MODE_VCO,
    TEXT_GTT_MODE_HCO,
    TEXT_GTT_MODE_FULL,

    TEXT_GTT_MODE_MAX,
}TEXT_GTT_MODE;

typedef enum
{
    MEDIA_SERVICE_PROFILE_AVP       = 0,
    MEDIA_SERVICE_PROFILE_AVPF      = 1,
    MEDIA_SERVICE_PROFILE_RCS       = 2
} MEDIA_SERVICE_PROFILE;

typedef enum
{
    MEDIA_SERVICE_MODEM_FEATURE_DEFAULT     = 0,
    MEDIA_SERVICE_MODEM_FEATURE_ATT         = 1,
    MEDIA_SERVICE_MODEM_FEATURE_CMCC        = 2,
    MEDIA_SERVICE_MODEM_FEATURE_KDDI        = 3,
    MEDIA_SERVICE_MODEM_FEATURE_VZW         = 4,
}MEDIA_SERVICE_MODEM_FEATURE;

typedef enum    // Media Transport Type Definition for checking RTP-RTCP Timeout
{
    MEDIA_PROTOCOL_NONE             = 0,    // No Check
    MEDIA_PROTOCOL_ANY              = 1,    // RTP or RTCP any
    MEDIA_PROTOCOL_RTP              = 2,    // RTP only
    MEDIA_PROTOCOL_RTCP             = 3,    // RTCP only
    MEDIA_PROTOCOL_NO_CHANGE        = 4,    // Maintain Previous Setting
} MEDIA_TRANSPORT_PROTOCOL;

typedef enum
{
    STATE_IDLE = 0,
    STATE_OFFER_RECEIVED,
    STATE_OFFER_SENT,
    STATE_NEGOTIATED,
    STATE_NOTUSED
}NEGO_STATE;

//== PUBLIC ENUM TYPE ==========================================================
typedef enum _START_TYPE
{
    START_INVALID = -1,
    START_PREVIEW, // Start only camera preview
    START_LIVE, // Start audio & video according to result of negotiation  (BASIC)
    START_CONF, // Start conference call (ex, U+ conf VT) : It must be implemented in child class
    START_NOTUSED
} START_TYPE;

typedef enum _REPORT_TYPE
{
    REPORT_INVALID = -1,
    REPORT_SUCCESS = 0,
    REPORT_FAILURE, // Failures, based on IMMedia.h - RtpError
    REPORT_CLOSE_SESSION, // After sending CloseSession successfully to ImsMedia
    REPORT_DATA_RECEIVE_FAILED, // No received RTP or RTCP Data - NOTIFY_MEDIA_INACTIVITY
    REPORT_DATA_RECEIVE_STARTED, // NOTIFY_FIRST_PACKET
    REPORT_QOS,
    REPORT_VIDEO_LOWEST_BIT_RATE,
    REPORT_CHECK_RADIO_CONNECTION,
    REPORT_NW_TONE_RTP_RECEIVE_STARTED,
    REPORT_NW_TONE_RTP_RECEIVE_FAILED,
    REPORT_RECEIVED_DTMF_EVENT,             // Received dtmf event
    REPORT_NOTUSED
} REPORT_TYPE;


/*Must Sync with MMPFDefinition*/
/********************************************************/
#ifndef MMPF_SRTP_CRYPTO_TYPE_DEFINED
#define MMPF_SRTP_CRYPTO_TYPE_DEFINED
typedef enum _eMMPFSrtpCryptoType
{
    MMPF_SRTP_CRYPTO_TYPE_NONE                  = 0,
    MMPF_SRTP_CRYPTO_TYPE_RESERVED              = 1,
    MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_80     = 2,
    MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_32     = 3,
    MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_80     = 4,
    MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_32     = 5,
    MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_80          = 6,
    MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_32          = 7,

    MMPF_SRTP_PROFILE_MAX = 0x7FFFFFFF
}eMMPFSrtpCryptoType;
#endif
/*******************************************************/

typedef enum _RTP_ANALYZER_TYPE
{
    RTP_ANALYZER_INVALID            = 0,
    RTP_ANALYZER_RTP                = (0x00000001),     // 1 3 7 15 31 63 127
    RTP_ANALYZER_DRA                = (0x00000002),     //   2 6 14 30 62 126
    RTP_ANALYZER_CIQ                = (0x00000004),     //     4 12 28 60 124
    RTP_ANALYZER_HASATI             = (0x00000010),     //          16 48 112
    RTP_ANALYZER_CCT                = (0x00000040),     //                64
    RTP_ANALYZER_DEBUGSCREEN        = (0x00000080),
    RTP_ANALYZER_DATAUSAGE          = (0x00000100)
}RTP_ANALYZER_TYPE;

typedef struct _tRTP_ANALYZER_OPTION
{
    IMS_SINT32     nRtpAnalyzerOptionRTP;
    IMS_SINT32     nRtpAnalyzerOptionDRA;
    IMS_SINT32     nRtpAnalyzerOptionCIQ;
    IMS_SINT32     nRtpAnalyzerOptionHASATI;
    IMS_SINT32     nRtpAnalyzerOptionCCT;
    IMS_SINT32     nRtpAnalyzerOptionDEBUGSCREEN;
}tRTP_ANALYZER_OPTION;

// == Struct define ==========================================================

typedef struct _tMediaValueInfo
{
    AString     strAudioCodecQuality;
    AString     strAudioCodecBandwidth;
    AString     strAudioCodecBitrate;
} tMediaValueInfo;

// == INTERNAL MESSAGE ==========================================================
#define MEDIA_INTERNAL_MSG_BASE             1500    // IMS_MSG_BASE_STREAMEDMEDIA + 100
#define MEDIA_INTERNAL_MSG_CONFIG_UPDATED   1501

// == MACROS ==========================================================
#define MEDIA_IS_CONTAINED_THIS_TYPE(eDst, eSrc)    ((eDst & eSrc) != 0)
#define MEDIA_TYPE_WITHOUT_TEXT(eSrc)               (eSrc & (~MEDIA_TYPE_TEXT))

#define MEDIA_DIRECTION_INVOLVED_RECV(eDir)         ((eDir == MEDIA_DIRECTION_SEND_RECEIVE) || (eDir == MEDIA_DIRECTION_RECEIVE))
#define MEDIA_DIRECTION_INVOLVED_SEND(eDir)         ((eDir == MEDIA_DIRECTION_SEND_RECEIVE) || (eDir == MEDIA_DIRECTION_SEND))
#define MEDIA_DIRECTION_IS_VALID(eDir)              ((eDir >= MEDIA_DIRECTION_INACTIVE) || (eDir <= MEDIA_DIRECTION_SEND_RECEIVE))

#define MEDIA_DIRECTION_IS_AUDIO_RUNNABLE(eDir)     ((eDir == MEDIA_DIRECTION_RECEIVE) || (eDir == MEDIA_DIRECTION_SEND_RECEIVE))
#define MEDIA_DIRECTION_IS_VIDEO_RUNNABLE(eDir)     ((MEDIA_DIRECTION_RECEIVE <= eDir) && (eDir <= MEDIA_DIRECTION_SEND_RECEIVE))
#define MEDIA_DIRECTION_IS_TEXT_RUNNABLE(eDir)      ((eDir == MEDIA_DIRECTION_RECEIVE) || (eDir == MEDIA_DIRECTION_SEND_RECEIVE))

#define MEDIA_DIRECTION_IS_AUDIO_HOLD(eDir)         ((eDir == MEDIA_DIRECTION_SEND) || (eDir == MEDIA_DIRECTION_INACTIVE))
#define MEDIA_DIRECTION_IS_VIDEO_HOLD(eDir)         (eDir == MEDIA_DIRECTION_INACTIVE)
#define MEDIA_DIRECTION_IS_TEXT_HOLD(eDir)          ((eDir == MEDIA_DIRECTION_SEND) || (eDir == MEDIA_DIRECTION_INACTIVE))

#endif                                              /* _MEDIA_DEF_H_ */
