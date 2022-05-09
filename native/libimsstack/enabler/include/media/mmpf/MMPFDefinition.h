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

/*!
 *    @brief        Definitions for MMPF interface
 */
#ifndef MMPF_MMPFDEFINITION_H_INCLUDED
#define MMPF_MMPFDEFINITION_H_INCLUDED

#include "mmpf/MMPFConfigure.h"

#ifdef __LP64__
#define __MMPF_LP64__
#endif

/**************************************************
 * variable type
 **************************************************/

typedef signed char mmpf_int8;
typedef signed short mmpf_int16;
typedef signed int mmpf_int32;
typedef long long mmpf_int64;
typedef unsigned long long mmpf_uint64;
typedef unsigned char mmpf_uint8;
typedef unsigned short mmpf_uint16;
typedef unsigned int mmpf_uint32;
typedef unsigned long mmpf_ulong;
typedef float mmpf_float;
typedef double mmpf_double;
typedef char mmpf_str;
typedef short mmpf_wstr;
typedef unsigned int mmpf_bool;
#if defined(__MMPF_LP64__)
typedef unsigned long mmpf_param;
typedef unsigned long mmpf_uintp;
typedef signed long mmpf_intp;

#define M_PFLS_d         "ld"
#define M_PFLS_x         "lx"
#define M_PFLS_X         "lX"
#define M_PFLS_u         "lu"

#define M_LONG_TO_INT(l) ((l)&0xFFFFFFFF)

#else  //__MMPF_LP64__
typedef unsigned int mmpf_param;
typedef unsigned int mmpf_uintp;
typedef signed int mmpf_intp;

#define M_PFLS_d         "d"
#define M_PFLS_x         "x"
#define M_PFLS_X         "X"
#define M_PFLS_u         "u"

#define M_LONG_TO_INT(l) (l)

#endif
typedef unsigned int mmpf_event;

#ifndef NULL
#define NULL 0
#endif

#ifndef MMPF_TRUE
#define MMPF_TRUE 1
#endif

#ifndef MMPF_FALSE
#define MMPF_FALSE 0
#endif

/**************************************************
 * defines
 **************************************************/

#define MMPF_MAX_VIDEO_WIDTH        1280
#define MMPF_MAX_VIDEO_HEIGHT       1280
#define MMPF_MAX_FILE_LEN           256
#define MMPF_MAX_IP_LEN             46
#define MMPF_MAX_CONFIG_LEN         256
#define MMPF_MAX_DTMF_LEN           128
#define MMPF_MAX_RECORD_CONFIG_LEN  64
#define MMPF_MAX_APN_LEN            64
#define MMPF_MAX_AUDIO_CAL_INFO_LEN 64
#define MMPF_MAX_PACKAGE_NAME       40
#define MMPF_MAX_TEXT_REDUNDANCY    3
#define MMPF_MAX_KEY_LEN            32
#define MMPF_RTT_MAX_CHAR_PER_SEC   30  // ATIS_GTT : 30 characters per second
#define MMPF_RTT_MAX_UNICODE_UTF8   4
#define MMPF_MAX_RTT_LEN            MMPF_RTT_MAX_CHAR_PER_SEC* MMPF_RTT_MAX_UNICODE_UTF8
#define MMPF_MAX_RTT_PAYLOAD_HEADER 4 + MMPF_MAX_TEXT_REDUNDANCY * 10
#define MMPF_DEBUG_DUMP_PATH_LEN    256
#define MMPF_T140_BUFFERING_TIME    300
#define MMPF_T140_MAX_CHUNK         2

#ifndef MMPF_IN
#define MMPF_IN
#endif

#ifndef MMPF_OUT
#define MMPF_OUT
#endif

#ifndef MMPF_IN_OUT
#define MMPF_IN_OUT
#endif

typedef void* HMMPFSession;

/**************************************************
 * enum values
 **************************************************/

typedef enum _eMMPFInterfaceID
{
    MMPF_INTERFACEID_ANDROID = 0,    // get binder intraface of MMPF
    MMPF_INTERFACEID_MANAGER = 100,  // get interface of MMPFMngr

    MMPF_INTERFACEID_AUTO = 200,  // depends on MMPF_IF_SERVICE definition

    MMPF_INTERFACEID_MAX
} eMMPFInterfaceID;

typedef enum _eMMPFSessionType
{
    MMPF_SESSION_AP,
    MMPF_SESSION_CP,
    MMPF_SESSION_CORE = MMPF_SESSION_AP,  // internal use only
    MMPF_SESSION_MAX
} eMMPFSessionType;

typedef enum _eMMPFModeType
{
    MMPF_MODE_NONE = -1,
    MMPF_MODE_TX,       // TX
    MMPF_MODE_RX,       // RX
    MMPF_MODE_TRX = 2,  // TX and RX
    MMPF_MODE_MAX = 2,
} eMMPFModeType;

typedef enum _eMMPFMediaType
{
    MMPF_MEDIA_AUDIO = 0,
    MMPF_MEDIA_VIDEO,
    MMPF_MEDIA_TEXT,
    MMPF_MEDIA_MAX,
} eMMPFMediaType;

typedef enum _eMMPFGraphType
{
    MMPF_GRAPHTYPE_TX = MMPF_MODE_TX,
    MMPF_GRAPHTYPE_RX = MMPF_MODE_RX,
    MMPF_GRAPHTYPE_RTCP,
    MMPF_GRAPHTYPE_MAX
} eMMPFGraphType;

typedef enum _eMMPFSourceType
{
    MMPF_SOURCE_NONE,  // default value
    MMPF_SOURCE_LIVE,
    MMPF_SOURCE_FILE_MOVIE,
    MMPF_SOURCE_FILE_IMAGE,
    MMPF_SOURCE_PAUSE_IMAGE,
    MMPF_SOURCE_FILE_YUV,
    MMPF_SOURCE_PREVIEW,
    MMPF_SOURCE_PCAP,

    MMPF_SOURCE_MAX
} eMMPFSourceType;

typedef enum _eMMPFTransportProtocol
{
    MMPF_TRANSPORT_NONE,
    MMPF_TRANSPORT_RTP,
    MMPF_TRANSPORT_MAX
} eMMPFTransportProtocol;

typedef enum _eMMPFSocketType
{
    MMPF_SOCKETTYPE_UDP,
    // MMPF_SOCKETTYPE_TCP, // TCP is not supported yet.

    MMPF_SOCKETTYPE_MAX
} eMMPFSocketType;

typedef enum _eMMPFIPVersion
{
    MMPF_IPV4,
    MMPF_IPV6,

    MMPF_IPV_MAX
} eMMPFIPVersion;

typedef enum _eMMPFMediaProfile
{
    MMPF_PROFILE_AVP,
    MMPF_PROFILE_AVPF,

    MMPF_PROFILE_MAX
} eMMPFMediaProfile;

typedef enum _eMMPFTimeoutCheck
{
    MMPF_TIMEOUT_CHECK_ANY,
    MMPF_TIMEOUT_CHECK_NONE,  // For Backword Compatibility, use NONE (Disable Monitoring) = 1
    MMPF_TIMEOUT_CHECK_RTP,
    MMPF_TIMEOUT_CHECK_RTCP,

    MMPF_TIMEOUT_CHECK_MAX
} eMMPFTimeoutCheck;

typedef enum _eMMPFAudioCodecType
{
    MMPF_AUDIO_CODEC_NONE = 0,
    MMPF_AUDIO_EVRC,
    MMPF_AUDIO_EVRC_B,
    MMPF_AUDIO_AMR,
    MMPF_AUDIO_AMR_WB,
    MMPF_AUDIO_G711_PCMU,
    MMPF_AUDIO_G711_PCMA,
    MMPF_AUDIO_AAC,
    MMPF_AUDIO_EVS,
    MMPF_AUDIO_MAX,
} eMMPFAudioCodecType;

typedef enum _eMMPFVideoCodecType
{
    MMPF_VIDEO_CODEC_NONE = 0,

    MMPF_VIDEO_H263_1996,  // video codec - H.263 profile 0, rtp payload format - RFC 2190
    MMPF_VIDEO_H263_2000,  // video codec - H.263 profile 0, rtp payload format - RFC 2429
    MMPF_VIDEO_MPEG4,      // video codec - MPEG-4 simple profile
    MMPF_VIDEO_H264,       // video codec - H.264 baseline profile
    MMPF_VIDEO_H265,       // video codec - [HEVC] H.265
    MMPF_VIDEO_MAX,
} eMMPFVideoCodecType;

typedef enum _eMMPFTextCodecType
{
    MMPF_TEXT_CODEC_NONE = 0,
    MMPF_TEXT_T140,      // text codec - T140 - RFC 4103
    MMPF_TEXT_T140_RED,  // text codec - T140 - RFC 4103 with redundancy
    MMPF_TEXT_MAX,
} eMMPFTextCodecType;

typedef enum _eMMPFAMRMode
{
    MMPF_AMR_MODE_0475 = 0,    /* 4.75 kbit /s */
    MMPF_AMR_MODE_0515 = 1,    /* 5.15 kbit /s */
    MMPF_AMR_MODE_0590 = 2,    /* 5.90 kbit /s */
    MMPF_AMR_MODE_0670 = 3,    /* 6.70 kbit /s */
    MMPF_AMR_MODE_0740 = 4,    /* 7.40 kbit /s */
    MMPF_AMR_MODE_0795 = 5,    /* 7.95 kbit /s */
    MMPF_AMR_MODE_1020 = 6,    /* 10.2 kbit /s */
    MMPF_AMR_MODE_1220 = 7,    /* 12.2 kbit /s */
                               //    MMPF_AMR_MODE_SID = 8, /* AMR SID */
    MMPF_AMRWB_MODE_660 = 8,   /* 6.60 kbit /s */
    MMPF_AMRWB_MODE_885 = 9,   /* 8.85 kbit /s */
    MMPF_AMRWB_MODE_1265 = 10, /* 12.65 kbit /s */
    MMPF_AMRWB_MODE_1425 = 11, /* 14.25 kbit /s */
    MMPF_AMRWB_MODE_1585 = 12, /* 15.85 kbit /s */
    MMPF_AMRWB_MODE_1825 = 13, /* 18.25 kbit /s */
    MMPF_AMRWB_MODE_1985 = 14, /* 19.85 kbit /s */
    MMPF_AMRWB_MODE_2305 = 15, /* 23.05 kbit /s */
    MMPF_AMRWB_MODE_2385 = 16, /* 23.85 kbit /s */
    MMPF_AMRWB_MODE_SID = 17,  /* AMRWB SID */
    MMPF_AMRWB_MODE_SPL = 18,  /* AMRWB Speech Lost frame */
                               //    MMPF_AMRWB_MODE_NO_DATA=15, /* AMRWB No Data */

    MMPF_AMRWB_MODE_MAX
} eMMPFAMRMode;

typedef enum
{
    MMPF_EVS_AMRIO_MODE_00660 = 0,
    MMPF_EVS_AMRIO_MODE_00885 = 1,
    MMPF_EVS_AMRIO_MODE_01265 = 2,
    MMPF_EVS_AMRIO_MODE_01425 = 3,
    MMPF_EVS_AMRIO_MODE_01585 = 4,
    MMPF_EVS_AMRIO_MODE_01825 = 5,
    MMPF_EVS_AMRIO_MODE_01985 = 6,
    MMPF_EVS_AMRIO_MODE_02305 = 7,
    MMPF_EVS_AMRIO_MODE_02385 = 8,

    MMPF_EVS_PRIMARY_MODE_00590 = 9,  /* 5.9 kbps, EVS Primary - SC-VBR 2.8kbps, 7.2kbps, 8kbps*/
    MMPF_EVS_PRIMARY_MODE_00720 = 10, /* 7.2 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_00800 = 11, /* 8 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_00960 = 12, /* 9.6 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_01320 = 13, /* 13.20 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_01640 = 14, /* 16.4 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_02440 = 15, /* 24.4 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_03200 = 16, /* 32 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_04800 = 17, /* 48 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_06400 = 18, /* 64 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_09600 = 19, /* 96 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_12800 = 20, /* 128 kbps, EVS Primary */
    MMPF_EVS_PRIMARY_MODE_SID = 21,   /* 2.4 kbps, EVS Primary SID */

    MMPF_EVS_PRIMARY_MODE_SPEECH_LOST = 22, /* SPEECH LOST */
    MMPF_EVS_PRIMARY_MODE_NO_DATA = 23,     /* NO DATA */

    MMPF_EVS_PRIMARY_MODE_MAX
} eMMPFEVSBitrate;

typedef enum
{
    MMPF_EVS_VOC_BANDWIDTH_NB = 0,
    MMPF_EVS_VOC_BANDWIDTH_WB = 1,
    MMPF_EVS_VOC_BANDWIDTH_SWB = 2,
    MMPF_EVS_VOC_BANDWIDTH_FB = 3,
    MMPF_EVS_VOC_BANDWIDTH_MAX
} eMMPFEVSBandwidth;

// EVRC, EVRC-B
typedef enum _eMMPFEVRCMode
{
    MMPF_EVRC_MODE_FIXEDRATE_FULL,
    MMPF_EVRC_MODE_FIXEDRATE_HALF,
    MMPF_EVRC_MODE_VARIABLERATE,
    MMPF_EVRC_MODE_RATEMAX,
    MMPF_EVRC_MODE_MAX
} eMMPFEVRCMode;

typedef enum _eMMPFG711Mode
{
    MMPF_G711_MODE_MULAW,
    MMPF_G711_MODE_ALAW,
    MMPF_G711_MODE_MAX
} eMMPFG711Mode;

typedef enum _eMMPFGTTMode
{
    MMPF_GTT_MODE_OFF,
    MMPF_GTT_MODE_VCO,
    MMPF_GTT_MODE_HCO,
    MMPF_GTT_MODE_FULL,
    MMPF_GTT_MODE_MAX
} eMMPFGTTMode;

typedef enum _eMMPFRTPPyaloadHeaderMode
{
    // evrc mode
    MMPF_RTPPAYLOADHEADER_MODE_EVRC_BUNDLE = 0,
    MMPF_RTPPAYLOADHEADER_MODE_EVRC_COMPACT = 1,  // evrc encoder should generate fixed rate
    MMPF_RTPPAYLOADHEADER_MODE_EVRC_FREEHEADER = 2,
    // amr mode
    MMPF_RTPPAYLOADHEADER_MODE_AMR_OCTETALIGNED = 0,  // octet aligned mode
    MMPF_RTPPAYLOADHEADER_MODE_AMR_EFFICIENT = 1,     // efficient mode
                                                   // h.264 mode
    MMPF_RTPPAYLOADHEADER_MODE_H264_SINGLE_NAL_UNIT = 0,  // packet mode 0
    MMPF_RTPPAYLOADHEADER_MODE_H264_NON_INTERLEAVED = 1,  // packet mode 1
                                                          // [EVS] evs mode
    MMPF_RTPPAYLOADHEADER_MODE_EVS_COMPACT = 0,      // EVS compact format 0
    MMPF_RTPPAYLOADHEADER_MODE_EVS_HEADER_FULL = 1,  // EVS header-full format 1
    MMPF_RTPPAYLOADHEADER_MODE_MAX
} eMMPFRTPPyaloadHeaderMode;

typedef enum _eMMPFEVSCodecMode
{
    MMPF_EVS_PRIMARY = 0,    // EVS PRIMARY mode 0
    MMPF_EVS_AMR_WB_IO = 1,  // EVS AMR-WB IO mode 1
    MMPF_EVS_CODEC_MODE_MAX
} eMMPFEVSCodecMode;

typedef enum _eMMPFEVSCMRCodeType
{
    MMPF_EVS_CMR_CODETYPE_NB = 0,       // 000
    MMPF_EVS_CMR_CODETYPE_AMRIO = 1,    // 001
    MMPF_EVS_CMR_CODETYPE_WB = 2,       // 010
    MMPF_EVS_CMR_CODETYPE_SWB = 3,      // 011
    MMPF_EVS_CMR_CODETYPE_FB = 4,       // 100
    MMPF_EVS_CMR_CODETYPE_WB_CHA = 5,   // 101
    MMPF_EVS_CMR_CODETYPE_SWB_CHA = 6,  // 110

    MMPF_EVS_CMR_CODETYPE_NO_REQ = 7,  // 111

    MMPF_EVS_CMR_CODETYPE_MAX
} eMMPFEVSCMRCodeType;

typedef enum _eMMPFEVSCMRCodeDefine
{
    MMPF_EVS_CMR_CODEDEFINE_59 = 0,     // 0000
    MMPF_EVS_CMR_CODEDEFINE_72 = 1,     // 0001
    MMPF_EVS_CMR_CODEDEFINE_80 = 2,     // 0010
    MMPF_EVS_CMR_CODEDEFINE_96 = 3,     // 0011
    MMPF_EVS_CMR_CODEDEFINE_132 = 4,    // 0100
    MMPF_EVS_CMR_CODEDEFINE_164 = 5,    // 0101
    MMPF_EVS_CMR_CODEDEFINE_244 = 6,    // 0110
    MMPF_EVS_CMR_CODEDEFINE_320 = 7,    // 0111
    MMPF_EVS_CMR_CODEDEFINE_480 = 8,    // 1000
    MMPF_EVS_CMR_CODEDEFINE_640 = 9,    // 1001
    MMPF_EVS_CMR_CODEDEFINE_960 = 10,   // 1010
    MMPF_EVS_CMR_CODEDEFINE_1280 = 11,  // 1011

    MMPF_EVS_CMR_CODEDEFINE_NO_REQ = 15,  // 1111

    // Ch-A
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_2 = 0,   // 0000
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_3 = 1,   // 0001
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_5 = 2,   // 0010
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_7 = 3,   // 0011
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_H2 = 4,  // 0100
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_H3 = 5,  // 0101
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_H5 = 6,  // 0110
    MMPF_EVS_CMR_CODEDEFINE_CHA_OFFSET_H7 = 7,  // 0111

    // AMR WB-IO
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_660 = 0,   // 0000
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_885 = 1,   // 0001
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_1265 = 2,  // 0010
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_1425 = 3,  // 0011
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_1585 = 4,  // 0100
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_1825 = 5,  // 0101
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_1985 = 6,  // 0110
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_2305 = 7,  // 0111
    MMPF_EVS_CMR_CODEDEFINE_AMRIO_2385 = 8,  // 1000

    MMPF_EVS_CMR_CODEDEFINE_MAX
} eMMPFEVSCMRCodeDefine;

typedef enum _eH263Profile
{
    MMPF_H263_PROFILE_0 = 0,  // The Baseline Profile
    MMPF_H263_PROFILE_1 = 1,  // H.320 Coding Efficiency Version 2 Backward-Compatibility Profile,
                              //   - Baseline + Annex I, J, L.4, T
    MMPF_H263_PROFILE_2 = 2,  // Version 1 Backward-Compatibility Profile, Baseline + Annex F
    MMPF_H263_PROFILE_3 = 3,  // Version 2 Interactive and Streaming Wireless Profile,
                              //   - Baseline + Annex I, J, K, T

    MMPF_H263_PROFILE_MAX
} eH263Profile;

typedef enum _eH263Level
{
    MMPF_H263_LEVEL_10 = 10,  // max 176x144, 64kbps, (30000/2002)fps
    MMPF_H263_LEVEL_20 = 20,  // max 352x288, 128kbps,
                              //     - (30000/2002)fps for CIF, (30000/1001)fps for QCIF
    MMPF_H263_LEVEL_30 = 30,  // max 352x288, 384kbps, (30000/1001)fps
    MMPF_H263_LEVEL_45 = 45,  // max 176x144, 128kbps, (30000/2002)fps
    MMPF_H263_LEVEL_50 = 50,  //
    MMPF_H263_LEVEL_60 = 60,  //
    MMPF_H263_LEVEL_70 = 70,  // max 640x480, 128kbps, (30000/2002)fps

    MMPF_H263_LEVEL_ENUM_MAX = 0x7FFFFFFF
} eH263Level;

typedef enum _eMPEG4Profile
{
    MMPF_MPEG4_PROFILE_SIMPLE,

    MMPF_MPEG4_PROFILE_MAX
} eMPEG4Profile;

typedef enum _eMPEG4Level
{
    MMPF_MPEG4_LEVEL_0,
    MMPF_MPEG4_LEVEL_0B,
    MMPF_MPEG4_LEVEL_1,
    MMPF_MPEG4_LEVEL_2,

    MMPF_MPEG4_LEVEL_MAX
} eMPEG4Level;

typedef enum _eH264Profile
{
    // Constrained Baseline profile is technically identical to specification of the use of the
    // Baseline profile with constraint_set1_flag equal to 1. It corresponds to the subset of
    // features that are in common between the Baseline, Main profile.
    MMPF_H264_PROFILE_CONSTRAINED_BASELINE,
    MMPF_H264_PROFILE_BASELINE,  // Baseline profile
    MMPF_H264_PROFILE_MAIN,      // Main profile
    MMPF_H264_PROFILE_HIGH,      // High profile
    MMPF_H264_PROFILE_MAX
} eH264Profile;

typedef enum _eConfigStringMode
{
    MMPF_CONFIG_SPS = 0,
    MMPF_CONFIG_PPS = 1,
    MMPF_CONFIG_VPS = 2,

    MMPF_CONFIG_MAX
} eConfigStringMode;

typedef enum _eH264Level
{
    MMPF_H264_LEVEL_1,   // level 1      : 176x144, 64kbps, 15.0fps for QCIF
    MMPF_H264_LEVEL_1B,  // level 1b     : 176x144, 128kbps, 15.0fps for QCIF
    MMPF_H264_LEVEL_11,  // level 1.1    : 352x288, 192kbps, 10.0fps for QVGA, 7.5fps for CIF
    MMPF_H264_LEVEL_12,  // level 1.2    : 352x288, 384kbps, 20.0fps for QVGA, 15.1fps for CIF
    MMPF_H264_LEVEL_13,  // level 1.3    : 352x288, 768kbps, 39.6fps for QVGA, 30.0fps for CIF
    MMPF_H264_LEVEL_2,   // level 2.0    : 352x288, 2Mbps
    MMPF_H264_LEVEL_21,  // level 2.1    : 704x288, 352x576, 4Mbps
    MMPF_H264_LEVEL_22,  // level 2.2    : 720x576, 4Mbps
    MMPF_H264_LEVEL_3,   // level 3.0    : 720x576, 10Mbps
    MMPF_H264_LEVEL_31,  // level 3.1    : 1280x720, 14Mbps

    MMPF_H264_LEVEL_MAX
} eH264Level;

/* [HEVC] Profiles */
typedef enum _eH265Profile
{
    MMPF_H265_PROFILE_MAIN = 0x01,
    MMPF_H265_PROFILE_MAIN10 = 0x02,
    MMPF_H265_PROFILE_UNKNOWN = 0x6EFFFFFF,
    MMPF_H265_PROFILE_MAX
} eH265Profile;

/* [HEVC] Levels */
typedef enum _eH265Level
{
    MMPF_H265_LEVEL_0 = 0x0,
    MMPF_H265_MAIN_LEVEL_1 = 0x1,
    MMPF_H265_HIGH_LEVEL_1 = 0x2,
    MMPF_H265_MAIN_LEVEL_2 = 0x4,
    MMPF_H265_HIGH_LEVEL_2 = 0x8,
    MMPF_H265_MAIN_LEVEL_21 = 0x10,
    MMPF_H265_HIGH_LEVEL_21 = 0x20,
    MMPF_H265_MAIN_LEVEL_3 = 0x40,
    MMPF_H265_HIGH_LEVEL_3 = 0x80,
    MMPF_H265_MAIN_LEVEL_31 = 0x100,
    MMPF_H265_HIGH_LEVEL_31 = 0x200,
    MMPF_H265_MAIN_LEVEL_4 = 0x400,
    MMPF_H265_HIGH_LEVEL_4 = 0x800,
    MMPF_H265_MAIN_LEVEL_41 = 0x1000,
    MMPF_H265_HIGH_LEVEL_41 = 0x2000,
    MMPF_H265_LEVEL_UNKNOWN = 0x6EFFFFFF,
    MMPF_H265_LEVEL_MAX
} eH265Level;

typedef enum _eMMPFBinaryFormat
{
    MMPF_BINARY_FORMAT_BASE16,  // mpeg-4 config
    MMPF_BINARY_FORMAT_BASE32,  // not used
    MMPF_BINARY_FORMAT_BASE64,  // h.264 config

    MMPF_BINARY_FORMAT_MAX
} eMMPFBinaryFormat;  // RFC 3548

typedef enum
{
    MMPF_COLOR_FMT_NV12,
    MMPF_COLOR_FMT_NV21,
    MMPF_COLOR_FMT_NV12_MVTB,
    MMPF_COLOR_FMT_NV12_UBWC,
    MMPF_COLOR_FMT_NV12_BPP10_UBWC,
    MMPF_COLOR_FMT_RGBA8888,
    MMPF_COLOR_FMT_RGBA8888_UBWC,
} eMMPFColorFormat;

// android specific start
typedef enum _eDisplayHandleType
{
    MMPF_DISPLAYHANDLE_SURFACE,
    MMPF_DISPLAYHANDLE_ISURFACE,
    MMPF_DISPLAYHANDLE_GRAPHICBUFFER,

    MMPF_DISPLAYHANDLE_MAX
} eDisplayHandleType;

// android specific end

typedef enum _eMMPFStorage
{
    MMPF_STORAGE_FILE,
    MMPF_STORAGE_BUFFER,

    MMPF_STORAGE_MAX
} eMMPFStorage;

typedef enum _RTP_ANALYZER_FEATURE
{
    RTP_ANALYZER_FEATURE_INVALID = 0,
    RTP_ANALYZER_FEATURE_RTP = (0x00000001),
    RTP_ANALYZER_FEATURE_DRA = (0x00000002),
    RTP_ANALYZER_FEATURE_CIQ = (0x00000004),
    RTP_ANALYZER_FEATURE_HASATI = (0x00000008),
    RTP_ANALYZER_FEATURE_CCT = (0x00000010),
    RTP_ANALYZER_FEATURE_DEBUGSCREEN = (0x00000020),
    RTP_ANALYZER_FEATURE_VTDATAUSAGE = (0x00000040)
} RTP_ANALYZER_FEATURE;

typedef enum _eMMPFProperty
{
    MMPF_PROPERTY_TXSOURCE,
    MMPF_PROPERTY_CODEC,
    MMPF_PROPERTY_TRANSPORT,
    MMPF_PROPERTY_NETWORK,
    MMPF_PROPERTY_DISPLAY,
    MMPF_PROPERTY_DEBUG,

    MMPF_PROPERTY_MAX
} eMMPFProperty;

// MMPF_REQUEST_REC_START : set record - structure of tMMPFReqeust
// MMPF_REQUEST_SEND_DTMF : set dtmf - structure of tMMPFRequest
// MMPF_REQUEST_UPDATE_SOURCE : set txsource - structure of tMMPFProperty with setProperty method,
//                                              before sending this request.
// MMPF_REQUEST_UPDATE_CODEC_CONFIG : set codec - structure of tMMPFProperty with setProperty method
//                                              before sending this request.
// MMPF_REQUEST_UPDATE_DISPLAY_CONFIG : set display - structure of tMMPFProperty
//                                            with setProperty method, before sending this request.
typedef enum _eMMPFRequest
{
    MMPF_REQUEST_PREVIEW,   // MMPF_MODE_TX only
    MMPF_REQUEST_PRESTART,  // MMPF_MODE_RX and MMPF_MEDIA_VIDEO only, start rx video without
                            // surface

    MMPF_REQUEST_START,
    MMPF_REQUEST_STOP,

    MMPF_REQUEST_PAUSE,  // If MMPF receive this request, MMPF pause all nodes except RTCP.
    MMPF_REQUEST_RESUME,

    MMPF_REQUEST_PAUSE_EX,   // video RX pause except RTP Session and socket reader
    MMPF_REQUEST_RESUME_EX,  // for MMPF_REQUEST_PUASE_EX

    MMPF_REQUEST_REC_START,  // set tMMPFRequestParam_Record    record parameter
    MMPF_REQUEST_REC_STOP,

    MMPF_REQUEST_UPDATE_SOURCE,  // Change tx source ( live, 3gpp, jpeg),
                                 // this request is valid only for MMPF_MODE_TX mode.
    MMPF_REQUEST_UPDATE_CAMERAID,
    MMPF_REQUEST_UPDATE_CAMERABRIGHT,
    MMPF_REQUEST_UPDATE_CAMERAZOOM,
    MMPF_REQUEST_UPDATE_SOURCETOIMAGE,
    MMPF_REQUEST_UPDATE_SOURCETOLIVE,
    MMPF_REQUEST_UPDATE_CODEC_CONFIG,  // Change tx live video encoder config(framerate, bitrate),
                                       // this request is valid only for MMPF_MODE_TX mode.
    MMPF_REQUEST_UPDATE_DISPLAY_CONFIG,
    MMPF_REQUEST_UPDATE_RENDERERDISPLAY_CONFIG,  // Change display setting
    MMPF_REQUEST_UPDATE_CAMERADISPLAY_CONFIG,
    MMPF_REQUEST_SEND_DTMF,  // Set DTMF parameter, this request is valid only for MMPF_MODE_TX mode
    MMPF_REQUEST_SEND_IFRAME,             // Encoder to end I-Frame
    MMPF_REQUEST_SEND_FULLINTRA_REQUEST,  // Send FIR(Full Intra Request) when AVPF is available
    MMPF_REQUEST_PREPARE_STOP,            // prepare stop
    MMPF_REQUEST_PAUSE_PREVIEW,           // VZW hold-resume case. If MMPF receive this request,
                                          //  MMPF stop Tx Socket and Pause Rx(All).
    MMPF_REQUEST_RESUME_PREVIEW,          // VZW hold-resume case. If MMPF receive this request,
                                          //  MMPF restart Tx Socket and resume Rx(All).
    MMPF_REQUEST_RESET,                   // Reset Jitterbuffer when sdp version change
    MMPF_REQUEST_RESET_RTP_TIMER,         // Reset rtp timer
    MMPF_REQUEST_SET_EPDG_CONDTION,       // Set ePDG condition
    MMPF_REQUEST_SET_NW_NORTP_NOTI_TIMER,
    MMPF_REQUEST_UPDATE_RTP_ENCODER_CONFIG,  // Change tx RTP Encoder to update CVO extension header
    MMPF_REQUEST_UPDATE_SOCKET,
    MMPF_REQUEST_RTPANALYZER_REPORT_OPTION,  // update DRA RTP Stats Report All option
    MMPF_REQUEST_UPDATE_MTU_SIZE,
    MMPF_REQUEST_SEND_RTT,          // RTT Send
    MMPF_REQUEST_SEND_RTT_BOM,      // Send a BOM before transferring a RTT in confirmed session.
    MMPF_REQUEST_ENABLE_RTT_AUDIO,  // Enable Audio report during audio-rtt.
    MMPF_REQUEST_VIDEO_DATA_USAGE,
    MMPF_REQUEST_START_PCAP_TEST,  // RX packet stream simulated mode
    MMPF_REQUEST_MAX
} eMMPFRequest;

typedef enum _eMMPFInfo
{
    MMPF_INFO_FILE_3GPP,         // param - file path
    MMPF_INFO_PARSE_SPROPPARAM,  // param - SPS, PPS
    MMPF_INFO_CP_IP_ADDR,        // input param - NULL
    MMPF_INFO_GET_SPROPPARAM,
    MMPF_INFO_SET_AUDIO_CAL_INFO_PARAM,  // send audio cal info param to audioBSP
    MMPF_INFO_CAMERA_FACING,  // input param - NULL, output param - camera facing for each camera ID
    MMPF_INFO_GET_SPROPPARAM_H265,                 // param - VPS, SPS, PPS for H.265
    MMPF_INFO_PARSE_SPROPPARAM_H265,               // parsing param - VPS, SPS, PPS for H.265
    MMPF_INFO_IS_GETTING_SOCKET_INTERFACEID = 10,  // MTK only, For VoNR

    MMPF_INFO_MAX
} eMMPFInfo;

typedef enum _eMMPFState
{
    MMPF_STATE_NULL,
    MMPF_STATE_CREATED,
    MMPF_STATE_PREVIEW,
    MMPF_STATE_PRESTART,
    MMPF_STATE_RUN,
    MMPF_STATE_RUN_REC,
    MMPF_STATE_PAUSED,
    MMPF_STATE_PAUSED_EX,

    MMPF_STATE_MAX
} eMMPFState;

typedef enum _eMMPFResult
{
    MMPF_RESULT_OK,
    MMPF_RESULT_ERR_UNKNOWN,
    MMPF_RESULT_ERR_INVALID_INTERFACE,
    MMPF_RESULT_ERR_INVALID_REQUEST,
    MMPF_RESULT_ERR_NULL_ARGUMENT,
    MMPF_RESULT_ERR_INVALID_ARGUMENT,
    MMPF_RESULT_ERR_ENGINE_CANT_CREATE,
    MMPF_RESULT_ERR_GRAPH_NULL_PROPERTY,
    MMPF_RESULT_ERR_GRAPH_BUILD,
    MMPF_RESULT_ERR_GRAPH_REBUILD,
    MMPF_RESULT_ERR_GRAPH_RUN,
    MMPF_RESULT_ERR_GRAPH_STOP,
    MMPF_RESULT_ERR_GRAPH_DESTROY,
    MMPF_RESULT_ERR_GRAPH_NOT_SUPPORTED,
    MMPF_RESULT_ERR_NODE_CANTSTART,
    MMPF_RESULT_ERR_NODE_CANTSTOP,
    MMPF_RESULT_ERR_CANT_ALLOC_MEMORY,
    MMPF_RESULT_ERR_CANTSTART_AUDIOSYSTEM,
    MMPF_RESULT_ERR_INVALID_MEDIATYPE,
    MMPF_RESULT_ERR_FILEPARSER,
    MMPF_RESULT_ERR_NODE_BLOCKED,
    MMPF_RESULT_ERR_CANT_GET_CPADDR,
    MMPF_RESULT_ERR_SWITCH_CAMID_FAIL,
    MMPF_RESULT_ERR_SWITCH_CAMID0_FAIL,
    MMPF_RESULT_ERR_SWITCH_CAMID1_FAIL,
    MMPF_RESULT_ERR_SWITCH_CAMID2_FAIL,
    MMPF_RESULT_ERR_CANTSTART_CAMERA_INVALID_OPERATION,
    MMPF_RESULT_ERR_CANTSTART_CAMERA_CONNECT_FAIL,

    MMPF_RESULT_MAX
} eMMPFResult;

typedef enum _eMMPFResponse
{
    MMPF_RESPONSE_SUCCESS = 0,
    MMPF_RESPONSE_ERR_CANTSTART_AUDIOSYSTEM,
    MMPF_RESPONSE_ERR_UNKNOWN,
    MMPF_RESPONSE_ERR_SWITCH_CAMID_FAIL,
    MMPF_RESPONSE_ERR_SWITCH_CAMID0_FAIL,
    MMPF_RESPONSE_ERR_SWITCH_CAMID1_FAIL,
    MMPF_RESPONSE_ERR_SWITCH_CAMID2_FAIL,
    MMPF_RESPONSE_ERR_CANTSTART_CAMERA,
    MMPF_RESPONSE_ERR_CAMERA_CONNECT_FAIL,

    MMPF_RESPONSE_MAX
} eMMPFResponse;

typedef enum _eMMPFResponseEvent
{
    MMPFRESPONSEEVENT_RESPONSE,
    MMPFRESPONSEEVENT_NOTIFY,
    MMPFRESPONSEEVENT_DESTROY,
    MMPFRESPONSEEVENT_MAX
} eMMPFResponseEvent;

typedef enum _eMMPFNotify
{
    MMPF_NOTIFY_SUCCESS = 0,
    MMPF_NOTIFY_RX_BUFFERING_COMPLETE,
    MMPF_NOTIFY_FILESHARE_STOPPED,
    MMPF_NOTIFY_3GPPREC_STOPPED_MEMORY_FULL,
    MMPF_NOTIFY_RX_FIRST_PACKET_RECEIVED,  // MMPF Media Trace first rx packet LGU+ Knight
    MMPF_NOTIFY_TX_FIRST_PACKET_SENT,      // MMPF Media Trace first tx packet LGU+ Knight

    MMPF_NOTIFY_RECV_AUDIO_RX_PACKET = 20,  // receive audio packet
                                            //  after MMPF_NOTIFY_ERR_RTP_TIMEOUT_NO_AUDIO_RX_PACKET
    MMPF_NOTIFY_RECV_VIDEO_RX_PACKET,       // receive video packet
                                            //  after MMPF_NOTIFY_ERR_RTP_TIMEOUT_NO_VIDEO_RX_PACKET
    MMPF_NOTIFY_RECV_TEXT_RX_PACKET,        // receive text packet
                                            //  after MMPF_NOTIFY_ERR_RTP_TIMEOUT_NO_TEXT_RX_PACKET
    MMPF_NOTIFY_ERR_UNKNOWN = 100,
    MMPF_NOTIFY_ERR_UNSUPPORTED,
    MMPF_NOTIFY_ERR_INVALID_ARGUMENT,
    MMPF_NOTIFY_ERR_INVALID_STATE,
    MMPF_NOTIFY_ERR_FILE_WRITE,
    MMPF_NOTIFY_ERR_CAMERA_TIMEOUT,
    MMPF_NOTIFY_ERR_VIDEO_CODEC,
    MMPF_NOTIFY_ERR_RTP_TIMEOUT_NO_AUDIO_RX_PACKET,
    MMPF_NOTIFY_ERR_RTP_TIMEOUT_NO_VIDEO_RX_PACKET,

    MMPF_NOTIFY_ERR_BINDER_DIED,
    MMPF_NOTIFY_ERR_SOCKET_BINDING_FAIL,

    MMPF_NOTIFY_FARSURFACE_PORTRAIT = 200,
    MMPF_NOTIFY_FARSURFACE_LANDSCAPE,
    MMPF_NOTIFY_LOWEST_VIDEO_BIT_RATE,  // notify lowest video bit rate to videoSession
    MMPF_NOTIFY_CAMERA_FRAME_IND,

    /* [TMUS] - to check RTP - radioConnection check for TMUS requirement(EPDG)*/
    MMPF_NOTIFY_CHECK_RADIO_CONNECTION = 300,
    MMPF_NOTIFY_NETWORK_NORTP_RECEIVED_NOTIFY,
    MMPF_NOTIFY_NETWORK_RTP_RECEIVED_NOTIFY,

    MMPF_NOTIFY_SRTP_SSRC_COLLISION = 400,
    MMPF_NOTIFY_SRTP_KEY_LIMIT_SOFT,
    MMPF_NOTIFY_SRTP_KEY_LIMIT_HARD,
    MMPF_NOTIFY_SRTP_PACKET_INDEX_LIMIT,

    MMPF_NOTIFY_FARFRAME_RESOLUTION_CHANGED = 500,

    // DTMF EVENT Notify
    MMPF_NOTIFY_DTMF_KEY_0 = 600,
    MMPF_NOTIFY_DTMF_KEY_1,
    MMPF_NOTIFY_DTMF_KEY_2,
    MMPF_NOTIFY_DTMF_KEY_3,
    MMPF_NOTIFY_DTMF_KEY_4,
    MMPF_NOTIFY_DTMF_KEY_5,
    MMPF_NOTIFY_DTMF_KEY_6,
    MMPF_NOTIFY_DTMF_KEY_7,
    MMPF_NOTIFY_DTMF_KEY_8,
    MMPF_NOTIFY_DTMF_KEY_9,
    MMPF_NOTIFY_DTMF_KEY_STAR,
    MMPF_NOTIFY_DTMF_KEY_POUND,
    MMPF_NOTIFY_DTMF_KEY_A,
    MMPF_NOTIFY_DTMF_KEY_B,
    MMPF_NOTIFY_DTMF_KEY_C,
    MMPF_NOTIFY_DTMF_KEY_D,

    // modify later
    MMPFRESPONSEEVENT_RTCP_NUMPACKET,
    MMPFRESPONSEEVENT_RTCP_SRIND,
    MMPFRESPONSEEVENT_RTCP_RRIND,
    MMPFRESPONSEEVENT_DEBUGINFO_ONSCREEN,
    MMPFRESPONSEEVENT_CIQ_INFO,
    MMPFRESPONSEEVENT_RTP_INFO,
    MMPFRESPONSEEVENT_DRA_INFO,
    MMPFRESPONSEEVENT_DATAUSAGE_INFO,
    MMPFRESPONSEEVENT_RTT_DATA,

    MMPF_NOTIFY_MAX
} eMMPFNotify;

//// same to VideoDef.h
typedef enum _eMMPF_VIDEO_RESOLUTION
{
    MMPF_VIDEO_RESOLUTION_INVALID,
    MMPF_VIDEO_RESOLUTION_QCIF_LS,  // QCIF Landscape (LGU+ group VT)
    MMPF_VIDEO_RESOLUTION_QVGA_LS,  // QVGA Landscape
    MMPF_VIDEO_RESOLUTION_QVGA_PR,  // QVGA Portrait
    MMPF_VIDEO_RESOLUTION_VGA_LS,   // VGA Landscape
    MMPF_VIDEO_RESOLUTION_VGA_PR,   // VGA Portrait
    MMPF_VIDEO_RESOLUTION_CIF_LS,   // CIF Landscape
    MMPF_VIDEO_RESOLUTION_CIF_PR,   // COF Portrait
    MMPF_VIDEO_RESOLUTION_QCIF_PR,  // QCIF Portrait (base)
    MMPF_VIDEO_RESOLUTION_SQCIF_LS,
    MMPF_VIDEO_RESOLUTION_SQCIF_PR,
    MMPF_VIDEO_RESOLUTION_SIF_LS,
    MMPF_VIDEO_RESOLUTION_SIF_PR,
    MMPF_VIDEO_RESOLUTION_HD_LS,
    MMPF_VIDEO_RESOLUTION_HD_PR,
    MMPF_VIDEO_RESOLUTION_FHD_LS,
    MMPF_VIDEO_RESOLUTION_FHD_PR,
    MMPF_VIDEO_RESOLUTION_NOT_USED = 99,
} eMMPF_VIDEO_RESOLUTION;

typedef enum _eMMPFCameraMode
{
    MMPF_CAMERAMODE_NONE = 0,       //        /** No camera running */
    MMPF_CAMERAMODE_CONNECTED = 1,  //        /** Connected only */
    MMPF_CAMERAMODE_PREVIEW = 2,    //        /** Connected and Preview started */
    MMPF_CAMERAMODE_RECORDING = 3,  //        /** Connected and Recording started */
    MMPF_CAMERAMODE_ALL = 4,        //        /** Connected and Preview & Recording started */

    MMPF_CAMERAMODE_MAX
} eMMPFCameraMode;

typedef enum _eMMPFCameraFacing
{
    // CAMERA_FACING_BACK       The facing of the camera is opposite to that of the screen.
    MMPF_CAMERA_REAR = 0,
    // CAMERA_FACING_FRONT      The facing of the camera is the same as that of the screen.
    MMPF_CAMERA_FRONT = 1,
    // CAMERA_FACING_EXTERNAL   The facing of the camera is not fixed relative to the screen.
    //                          The cameras with this facing are external cameras, e.g. USB cameras
    // MMPF_CAMERA_UNDEFINED = 2,

    MMPF_CAMERA_MAX
} eMMPFCameraFacing;

typedef enum _eMMPFOrientation
{
    MMPF_ORIENTATION_PORTRAIT = 0,
    MMPF_ORIENTATION_PORTRAIT_0 = 0,
    MMPF_ORIENTATION_LANDSCAPE = 1,    // right = 90CW/270CCW
    MMPF_ORIENTATION_LANDSCAPE_0 = 1,  // right
    MMPF_ORIENTATION_PORTRAIT_180 = 2,
    MMPF_ORIENTATION_LANDSCAPE_180 = 3,              // left = 270CW/90CCW
    MMPF_ORIENTATION_UNDEFINED = 4,                  // No orientation (Undefined yet)
    MMPF_ORIENTATION_UI_NOCHANGE_LANDSCAPE = 5,      // right = 90CW/270CCW
    MMPF_ORIENTATION_UI_NOCHANGE_LANDSCAPE_180 = 6,  // left = 270CW/90CCW
    MMPF_ORIENTATION_MAX,
} eMMPFOrientation;

typedef enum _eMMPFOrientationDegree
{
    MMPF_ORIENTATION_0,
    MMPF_ORIENTATION_90,
    MMPF_ORIENTATION_180,
    MMPF_ORIENTATION_270,
} eMMPFOrientationDegree;

typedef enum
{
    MMPF_MEDIA_NETWORK_ANY = 0,
    MMPF_MEDIA_NETWORK_3G,   // = 1
    MMPF_MEDIA_NETWORK_LTE,  // = 2
    MMPF_MEDIA_NETWORK_MAX
} eMMPFNetworkType;

typedef enum
{
    MMPF_SOCKET_POS_DEFAULT = 0,  // use default sockeet
    MMPF_SOCKET_POS_CP = 1,       // force to use cp socket
    MMPF_SOCKET_POS_AP = 2,       // force to use ap socket,
                                  // do NOT use this option for commercial project!!!
    MMPF_SOCKET_POS_AP_MEDIA_ENGINE = 3,
    MMPF_SOCKET_POS_MAX
} eMMPFSocketPos;

typedef enum _eMMPFVideoCVOType
{
    MMPF_VIDEO_CVO_NONE = 0,
    MMPF_VIDEO_CVO_LS = 1,  // landscape nego, displaying port
    MMPF_VIDEO_CVO_PR = 2,  // others
    MMPF_VIDEO_CVO_MAX
} eMMPFVideoCVOType;

// (moved from MMPFInternal.h)
typedef enum _eMMPFMediaSubType
{
    MMPF_MEDIASUBTYPE_UNDEFINED,
    MMPF_MEDIASUBTYPE_RTPPAYLOAD,      // rtp payload header + encoded bitstream
    MMPF_MEDIASUBTYPE_RTPPACKET,       // rtp packet
    MMPF_MEDIASUBTYPE_RTCPPACKET,      // rtcp packet
    MMPF_MEDIASUBTYPE_RTCPPACKET_BYE,  // rtcp packet
    MMPF_MEDIASUBTYPE_RAWDATA,         // raw yuv or pcm data
    MMPF_MEDIASUBTYPE_RAWDATA_ROT90,   // inverse rotated raw yuv data (need to ratate 90 degree)
    MMPF_MEDIASUBTYPE_RAWDATA_ROT90_FLIP,  // inverse rotated raw yuv data and flip
    MMPF_MEDIASUBTYPE_RAWDATA_ROT270,
    MMPF_MEDIASUBTYPE_RAWDATA_CROP_ROT90,            // inverse and crop the rotated raw yuv data
                                                     //  (need to ratate 90 degree)
    MMPF_MEDIASUBTYPE_RAWDATA_CROP_ROT90_FLIP = 10,  // inverse and crop the
                                                     // rotated raw yuv data and flip
    MMPF_MEDIASUBTYPE_RAWDATA_CROP_ROT270,           // inverse and crop the rotated raw yuv data
                                                     // (need to ratate 270 degree)
    MMPF_MEDIASUBTYPE_RAWDATA_CROP,
    MMPF_MEDIASUBTYPE_DTMFSTART,
    MMPF_MEDIASUBTYPE_DTMFEVENT,  // rtp payload for dtmf event
    MMPF_MEDIASUBTYPE_DTMFEND,
    MMPF_MEDIASUBTYPE_DTXSTART,                  // EVRC-B, 2011.3.8
    MMPF_MEDIASUBTYPE_BITSTREAM_H263,            // encoded bitstream
    MMPF_MEDIASUBTYPE_BITSTREAM_MPEG4,           // encoded bitstream
    MMPF_MEDIASUBTYPE_BITSTREAM_H264,            // encoded bitstream
    MMPF_MEDIASUBTYPE_BITSTREAM_G711_PCMU = 20,  // encoded bitstream
    MMPF_MEDIASUBTYPE_BITSTREAM_G711_PCMA,       // encoded bitstream
    MMPF_MEDIASUBTYPE_BITSTREAM_EVRC,
    MMPF_MEDIASUBTYPE_BITSTREAM_EVRC_B,
    MMPF_MEDIASUBTYPE_BITSTREAM_AMR_WB,
    MMPF_MEDIASUBTYPE_BITSTREAM_AMR,
    MMPF_MEDIASUBTYPE_BITSTREAM_T140,      // T140
    MMPF_MEDIASUBTYPE_BITSTREAM_T140_RED,  // T140 Redendancy
    MMPF_MEDIASUBTYPE_PCM_DATA,            // decoded pcm data
    MMPF_MEDIASUBTYPE_PCM_NO_DATA,         // decoded pcm no data
    MMPF_MEDIASUBTYPE_NOT_READY,           // Jitter Buffer GetData not ready
    MMPF_MEDIASUBTYPE_BITSTREAM_CODECCONFIG,
    // [CVO] for RTP Header Extension - Insert CVO information when the last packet of IDR frame
    MMPF_MEDIASUBTYPE_IDR_FRAME = 40,  // rtp payload header + extension header + encoded bitstream
    MMPF_MEDIASUBTYPE_ROT0,
    MMPF_MEDIASUBTYPE_ROT90,
    MMPF_MEDIASUBTYPE_ROT180,
    MMPF_MEDIASUBTYPE_ROT270,
    MMPF_MEDIASUBTYPE_REFRESHED,       // for refreshing involved nodes(Initial purpose is jitter
                                       // bufffer)
    MMPF_MEDIASUBTYPE_BITSTREAM_H265,  // [HEVC] add for H.265 encoded bitstream
    MMPF_MEDIASUBTYPE_BITSTREAM_EVS = 50,
    MMPF_MEDIASUBTYPE_AMR_QBIT_SID_BAD,  // Q_Sid_Speech
    MMPF_MEDIASUBTYPE_AMR_QBIT_SPEECH_BAD,
    MMPF_MEDIASUBTYPE_PACKET_OPTIONAL_TTL,  // IP layer data, TTL value for VZW
    MMPF_MEDIASUBTYPE_VIDEO_CONFIGSTRING,   // Config String (SPS/PPS)
    MMPF_MEDIASUBTYPE_FLIP_HORIZONTAL,
    MMPF_MEDIASUBTYPE_FLIP_VERTICAL,
    MMPF_MEDIASUBTYPE_NON_IDR_FRAME,
    MMPF_MEDIASUBTYPE_H264_SEI_FRAME,
    MMPF_MEDIASUBTYPE_MAX
} eMMPFMediaSubType;

// Type of socket option
typedef enum _eMMPFSocketOpt
{
    MMPF_OPT_BASE = 0,
    MMPF_OPT_IP_QOS = 1,

    MMPF_OPT_MAX
} eMMPFSocketOpt;

// SRTP
#ifndef MMPF_SRTP_CRYPTO_TYPE_DEFINED
#define MMPF_SRTP_CRYPTO_TYPE_DEFINED
typedef enum _eMMPFSrtpCryptoType
{
    MMPF_SRTP_CRYPTO_TYPE_NONE = 0,
    MMPF_SRTP_CRYPTO_TYPE_RESERVED = 1,
    MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_80 = 2,
    MMPF_SRTP_CRYPTO_TYPE_AES128_CM_SHA1_32 = 3,
    MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_80 = 4,
    MMPF_SRTP_CRYPTO_TYPE_AES256_CM_SHA1_32 = 5,
    MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_80 = 6,
    MMPF_SRTP_CRYPTO_TYPE_NULL_SHA1_32 = 7,

    MMPF_SRTP_PROFILE_MAX
} eMMPFSrtpCryptoType;
#endif

typedef enum _eMMPFSrtpEvent
{
    MMPF_EVENT_SRTP_SSRC_COLLISION = 0,
    MMPF_EVENT_SRTP_KEY_SOFT_LIMIT = 1,
    MMPF_EVENT_SRTP_KEY_HARD_LIMIT = 2,
    MMPF_EVENT_SRTP_PACKET_INDEX_LIMIT = 3,

    MMPF_EVENT_SRTP_MAX
} eMMPFSrtpEvent;

typedef enum _eMMPFAnalEventType
{
    MMPF_ANAL_EVENT_INVALID = -1,

    MMPF_ANAL_EVENT_PACKET_LOSS = 0,
    MMPF_ANAL_EVENT_JITTER_RESET = 1,

    MMPF_ANAL_EVENT_MAX
} eMMPFAnalEventType;

typedef enum _eMMPFDumpOption
{
    MMPF_DUMP_CAMERA = 16,
    MMPF_DUMP_ENCODER_IN = 8,
    MMPF_DUMP_ENCODER_OUT = 4,
    MMPF_DUMP_DECODER_IN = 2,
    MMPF_DUMP_DECODER_OUT = 1,
    MMPF_DUMP_MAX
} eMMPFDumpOption;

typedef enum _eMMPFMediaProtocol
{
    MMPF_MEDIA_RTP = 0,
    MMPF_MEDIA_RTCP,
    MMPF_MEDIA_PROTOCOL_MAX,
} eMMPFMediaProtocol;

// End SRTP

/**************************************************
 * definitions for Property
 **************************************************/

typedef struct _tMMPFAVPRateAdaptationInfo
{
    mmpf_uint32 nAVPRateAdaptation_Cycle;            // Time interval to downward (rtcp cycle)
    mmpf_uint32 nAVPRateAdaptation_DevRatio;         // Time interval to upward (rtcp cycle)
    mmpf_uint32 nAVPRateAdaptation_Threshold;        // Bitrate Ratio for a stage (%)
    mmpf_uint32 nAVPRateAdaptation_MinBitrateRatio;  // Minimum bitrate ratio (%)
    mmpf_uint32 nAVPRateAdaptation_ASBandWidth;      // AS bandwidth to calc. max bitrate
} tMMPFAVPRateAdaptationInfo;

typedef struct _tMMPFRTCPFeedbackInfo
{
    mmpf_bool bSupportedNACK;
    mmpf_bool bSendNACKImmediately;
    mmpf_bool bEnableNACKResponder;
    mmpf_bool bSupportedTMMBRN;
    mmpf_bool bSupportedPLI;
    // TMMBR/TMMBN Parameter
    mmpf_uint32 nTMMBRDownInterval;        // Time interval (seconds) to determine downward
    mmpf_uint32 nTMMBRUpInterval;          // Time interval (seconds) to determine upward
    mmpf_uint32 nTMMBRLossThreshold;       // Threshold of RTP loss rate to cause TMMBR
                                           //  (Loss rate for 1 level of bitrate change)
    mmpf_uint32 nTMMBRMinBitrateRatio;     // Minimum threshold of bitrate
    mmpf_uint32 nTMMBRBitrateLevel;        // Level of bitrate change (n step)
    mmpf_uint32 nTMMBRUpLevel;             // Level of encreasing bitrate (1 ~ nTMMBRBitrateLevel)
    mmpf_uint32 nTMMBREnforceVTDowngrade;  // Forcingly set video downgrade
    mmpf_uint32 nTMMBRASBandWidth;         // Negotiated AS value to calc the maximum bitrate.
} tMMPFRTCPFeedbackInfo;

typedef struct _tMMPFPortInfo
{
    eMMPFSocketType type;

    mmpf_uint32 nLocalPortNumber;
    mmpf_uint32 nPeerPortNumber;

    mmpf_uint8 nTOS;  // value of DS field in IP header, default value is 0x00
} tMMPFPortInfo;

typedef struct _tMMPFAudioDTXInfo
{
    mmpf_uint32 nDtxMax;
    mmpf_uint32 nDtxMin;
    mmpf_uint32 nHangover;
} tMMPFAudioDTXInfo;

typedef struct _tMMPFJitterBufferOptions
{
    mmpf_uint32 nInitSize;  // initial jitter buffer size, in packet unit(20msec)
    mmpf_uint32 nMinSize;   // minimum jitter buffer size, in packet unit(20msec)
    mmpf_uint32 nMaxSize;   // maximum jitter buffer size, in packet unit(20msec)
    mmpf_uint32 nReduceThreshold;
    mmpf_uint32 nStepSize;
    mmpf_uint32 zValue;
    mmpf_bool bIgnoreSID;
    mmpf_bool bImprovement;
} tMMPFJitterBufferOptions;

typedef struct _tMMPFAudioCodecInfo
{
    eMMPFAudioCodecType nCodecType;
    mmpf_uint32 nCodecOption;  // so far, it is CMR send option
    mmpf_uint32 nCmr;          // CMR value

    union
    {
        struct
        {
            mmpf_uint32 mode;   // change mode type for CMR
            mmpf_bool bScrOff;  // Source Controlled Rate(same as DTX),
                                // false is turn on, true is turn off
        } amr;
        struct
        {
            eMMPFEVRCMode mode;
            tMMPFAudioDTXInfo dtxInfo;
            mmpf_bool bDTXSupport;
        } evrc;
        struct
        {
            eMMPFEVSCodecMode eEVSMode;
            mmpf_uint32 nBandwidth;
            mmpf_uint32 nBitrate;
            mmpf_bool bScrOff;
            mmpf_uint32 nChannelAwareMode;
        } evs;
    };

    tMMPFJitterBufferOptions tJitterOptions;
} tMMPFAudioCodecInfo;

typedef struct _tMMPFVideoCodecInfo
{
    eMMPFVideoCodecType nCodecType;

    mmpf_uint32 nWidth;
    mmpf_uint32 nHeight;

    mmpf_uint32 nVideoBitRate;       // this value is used only for tx live/image source
    mmpf_uint32 nVideoFrameRate;     // this value is used only for tx live/image source
    mmpf_bool bBitRateControl;       // 0: CBR, 1:VBR
    eMMPFVideoCVOType nCVORotation;  // 0: CVO disabled, 1: CVO LS, 2 : CVO PR
    mmpf_bool bRcsVideo;             // 0: false, 1:true
    mmpf_uint32 nIframeInterval;     // Set I-frame interval

    union
    {
        struct
        {
            eH263Profile eProfile;
            eH263Level eLevel;
        } h263;
        struct
        {
            // profile, level will be used for encoder setting
            eMPEG4Profile eProfile;
            eMPEG4Level eLevel;
            // set szConfig will be used for decoder setting
            eMMPFBinaryFormat eConfigFormat;         // set MMPF_BINARY_FORMAT_BASE16 for mpeg4
            mmpf_str szConfig[MMPF_MAX_CONFIG_LEN];  // null terminated ASCII string
            mmpf_uint32 nProfile_level_id;
        } mpeg4;
        struct
        {
            // profile, level will be used for encoder setting
            eH264Profile eProfile;
            eH264Level eLevel;
            // drop p frame is reference I frame has loss
            mmpf_bool bDropPFrame;
            // set szConfig will be used for decoder setting
            eMMPFBinaryFormat eConfigFormat;         // set MMPF_BINARY_FORMAT_BASE64 for h.264
            mmpf_str szConfig[MMPF_MAX_CONFIG_LEN];  // null terminated ASCII string,
                                                     //  sprop-parameter-sets
            mmpf_str szProfile_level_id[7];          // profile-level-id
        } h264;
        struct
        {
            // profile, level will be used for encoder setting
            eH265Profile eProfile;
            eH265Level eLevel;
            // drop p frame is reference I frame has loss
            mmpf_bool bDropPFrame;
            // set szConfig will be used for decoder setting
            eMMPFBinaryFormat eConfigFormat;         // set MMPF_BINARY_FORMAT_BASE64 for h.265
            mmpf_str szConfig[MMPF_MAX_CONFIG_LEN];  // null terminated ASCII string,
                                                     //  sprop-parameter-sets
        } h265;                                      /* [HEVC] struct */
    };
    // if this value is enabled in AVP profile, adjust video adaptation using RTCP Report
    mmpf_bool bAVPVideoAdaptation;
    tMMPFAVPRateAdaptationInfo stAVPRateAdaptationInfo;
    // Ignoring Multiple marker(TRUE) for same timestamp -- Exception for SKT VT Conference Server
    mmpf_uint32 nVideoMarkerOption;
    // Deliver MediaSubType information for Video Rotation
    eMMPFMediaSubType eMediaSubType;
    // RTCP Feedback type (for support AVPF/CCM), generally used Video
    mmpf_bool bSupportedRTCPFeedback;
    tMMPFRTCPFeedbackInfo stRTCPFeedbackInfo;
} tMMPFVideoCodecInfo;

typedef struct _tMMPFTextCodecInfo
{
    eMMPFTextCodecType nCodecType;
    eMMPFGTTMode nGTTMode;
    mmpf_uint32 nBitrate;
} tMMPFTextCodecInfo;

typedef struct _tMMPFLiveSourceInfo
{
    mmpf_uint32 nCameraID;  // camera id
    // eFrameOrientation: 0 -portrait, 1-landscape_0, 2-landscape_180
    eMMPFOrientation eFrameOrientation;
    // eDeviceOrientation: 0 - 0 degree, 1 - 90 degree, 2 - 180 degree, 3 - 270 degree
    eMMPFOrientationDegree eDeviceOrientation;
    mmpf_int32 nZoomLevel;
    mmpf_int32 nBrightnessLevel;
    void* pvDispHandle;
    // nCVORotation:  0: CVO disabled, 1: CVO LS, 2 : CVO PR
    eMMPFVideoCVOType nCVORotation;
    // nMediaSubType:  5: Raw data, 8: Rot90, etc.
    eMMPFMediaSubType nMediaSubType;
    // camera Facing (0 : REAR / 1 : FRONT / 2 : UNDEFINED)
    eMMPFCameraFacing eCameraFacing;
    // 0 - 0 degree, 1 - 90 degree, 2 - 180 degree, 3 - 270 degree
    eMMPFOrientationDegree eCameraSensorOrientation;
} tMMPFLiveSourceInfo;

typedef struct _tMMPFMovieFileSourceInfo
{
    mmpf_str szSourceFileName[MMPF_MAX_FILE_LEN];
    mmpf_bool bLoop;
} tMMPFMovieFileSourceInfo;

typedef struct _tMMPFImageFileSourceInfo
{
    mmpf_str szSourceFileName[MMPF_MAX_FILE_LEN];
} tMMPFImageFileSourceInfo;

typedef struct _tMMPFYUVFileSourceInfo
{
    mmpf_str szSourceFileName[MMPF_MAX_FILE_LEN];
} tMMPFYUVFileSourceInfo;

typedef struct _tMMPFSourceInfo
{
    eMMPFSourceType nSourceType;

    union
    {
        tMMPFLiveSourceInfo live;
        tMMPFMovieFileSourceInfo movie_file;
        tMMPFImageFileSourceInfo img_file;
        tMMPFYUVFileSourceInfo yuv_file;
    };
} tMMPFSourceInfo;

// media info in SDP
typedef struct _tMMPFCodecInfo
{
    union
    {
        tMMPFAudioCodecInfo audio;
        tMMPFVideoCodecInfo video;
        tMMPFTextCodecInfo text;
    };
} tMMPFCodecInfo;

typedef struct _tMMPFDTMFInfo
{
    mmpf_uint32 nDuration;            // msec unit, duration of one DTMF tone
    mmpf_uint32 nRetransmitDuration;  // msec unit, duration of retransmitting
                                      //  the last packet for a tone
    mmpf_uint32 nInterval;            // msec unit, interval of two DTMF signals
                                      // If you send N DTMF signals, it will take
    //( nDuration + nRetransmitDuration + nInterval) * N - nInterval msec.
    mmpf_uint32 nVolume;  // Volume of DTMF, 0~63, default value is 10.
    mmpf_uint32 nSamplingRate;
} tMMPFDTMFInfo;

/*  2014/12/1 SRTP START */
typedef struct _tMMPFSRTPInfo
{
    mmpf_bool bEnableSRTP;  // Enable Secure RTP
    mmpf_int32 nMasterKeyLifeTime;
    eMMPFSrtpCryptoType eSrtpCryptoType;
    mmpf_uint8 szKey[MMPF_MAX_KEY_LEN];
} tMMPFSRTPInfo;
/*  2014/12/1 SRTP END */

typedef struct _tMMPFRequestParam_RTPANALYZER_Option
{
    mmpf_int32 nReportOption_LDB;
    mmpf_int32 nReportOption_DRA;
    mmpf_int32 nReportOption_CIQ;
    mmpf_int32 nReportOption_IMSANALYZER;
    mmpf_int32 nReportOption_HASATI;
    mmpf_int32 nReportOption_MOCA;
    mmpf_int32 nReportOption_CCT;
    mmpf_int32 nReportOption_DEBUGSCREEN;
} tMMPFRequestParam_RTPANALYZER_Option;

typedef struct _tMMPFRTPAnalyzerInfo
{
    mmpf_int32 nRtpAnalyzerlist;  // Rtp/Rtcp Analyzer list.
    tMMPFRequestParam_RTPANALYZER_Option rtpAnalyzerOptionlist;
} tMMPFRTPAnalyzerInfo;

typedef struct _tMMPFRTPInfo
{
    // separate local & peer payload type
    mmpf_uint32 nLocalPayloadType;  // RX payload type
    mmpf_uint32 nPeerPayloadType;   // TX payload type
    mmpf_uint32 nSamplingRate;
    // msec unit, packet time, used for only audio, default value is 20, RFC 2327
    mmpf_uint32 nPtime;
    // amr payload header mode
    eMMPFRTPPyaloadHeaderMode ePayloadHdrMode;
    // In case of TX mode, this value is used to send DTMF.
    // In case of RX mode, this value is used to recognize/ignore DTMF packet.
    mmpf_uint32 nPayloadTypeDTMF;
    // Enable RFC 2833 DTMF, if this value is MMPF_FALSE, By-pass mode DTMF will be generated.
    mmpf_bool bEnableDTMF;
    // set this structure if bEnableDTMF is MMPF_TRUE
    tMMPFDTMFInfo stDTMFInfo;
    // disable RTCP on active call
    mmpf_bool bDisableRTCPActivation;
    // enable processing for NAT
    // send RTCP packet at start time and send RTP packet until receiving rtp packet
    // when mmpf is started as RX only mode.
    // RTP keep alive is not implemented.
    mmpf_bool bEnableNATProcessing;
    // (Do not use - replaced with eTimeoutCheckType) Set False to Enable rtp timeout,
    // Set True to Disable rtp timeout, only for CP media
    // mmpf_bool     bDisableRTPTimeOut;
    // (Do not use - replaced with nTimeoutThreshold) Timeout Threshold, in second unit, only for CP
    // mmpf_uint32   nRtpTimeOutThreshold;
    eMMPFTimeoutCheck eTimeoutCheckType;  // 0 : Any / 1 : No check / 2 : RTP only / 3 : RTCP only
    mmpf_uint32 nTimeoutThreshold;        // Timeout Threshold, in second unit, only for CP media
    mmpf_bool bSendRTCPByePacket;         // true to send rtcp bye packet when closing session.
    // true for loopback mode, this field is available only for tx setting.
    // When you set rx property this field is ignored.
    mmpf_bool bLoopback;
    mmpf_uint32 nSendSPSPPS;  // only in case of H.264(video)-LGU+, it is true.
    // (Do not use - merged on nTimeoutThreshold) Initial RTP Timeout Threshold,
    // in second unit, only for CP media
    mmpf_uint32 nInitialRTPTimeOutThreshold;
    tMMPFRTPAnalyzerInfo stRtpAnalyzerInfo;  // Rtp/Rtcp packet Analyzer list.
    mmpf_bool bTextKeepRedLevel;  // [GTT/TTY] - option for sending empty redundant payload
                                  // in Text Redundancy Level for T.140
    mmpf_bool bExtensionHeader;   // [CVO] - Enable extension header
    mmpf_int32 nExtensionIdCVO;   // [CVO] - Extension ID for CVO (1-14)
    mmpf_int32 nRtcpInterval;     // RTCP Interval
    mmpf_uint32 nTextRedPayload;  // [GTT/TTY] - Text Redundancy Payload for T.140
    mmpf_uint32 nTextRedLevel;    // [GTT/TTY] - Text Redundancy Level for T.140
    mmpf_int32 nRadioCcTimer;     // Radio Connection Check timer
    tMMPFSRTPInfo stSrtpInfo;     // for SRTP
} tMMPFRTPInfo;

typedef struct _tMMPFRTPHeaderExtensionInfo
{
    mmpf_uint16 nDefinedByProfile;
    mmpf_uint16 nLength;
    mmpf_uint16 nExtensionData;
} tMMPFRTPHeaderExtensionInfo;

typedef struct _tMMPFTransportInfo
{
    eMMPFTransportProtocol protocol;

    union
    {
        tMMPFRTPInfo rtp;
    };

} tMMPFTransportInfo;

// rtcp-xr
typedef struct _tMMPFRTCPXrInfo
{
    mmpf_bool bSupportStatisticMetrics;
    mmpf_bool bSupportVoipMatircs;
    mmpf_bool bSupportPacketLossRle;
    mmpf_bool bSupportPacketDuplicatedRle;
} tMMPFRTCPXrInfo;

typedef struct _tMMPFDebugInfo
{
    mmpf_uint32 nDebugLog;
    mmpf_uint32 nDebugDump;
    mmpf_uint8 sDebugDumpPath[MMPF_DEBUG_DUMP_PATH_LEN];
} tMMPFDebugInfo;

typedef struct _tMMPFNetworkInfo
{
    eMMPFTransportProtocol protocol;

    eMMPFIPVersion eLocalIPVersion;
    eMMPFIPVersion eRemoteIPVersion;

    mmpf_str szLocalIp[MMPF_MAX_IP_LEN];
    mmpf_str szPeerIp[MMPF_MAX_IP_LEN];

    struct
    {
        tMMPFPortInfo rtp_port;
        tMMPFPortInfo rtcp_port;
    } rtp;

    mmpf_int32 nPdpProfileNum;         // 0 for default
    mmpf_str szAPN[MMPF_MAX_APN_LEN];  // APN, If APN=="", don't set APN configue.
                                       // If nPdpProfileNum is not 0, this field is ignored.
    mmpf_int32 nPdpProfileNumOf3G;     // -1 (Do not use) or operator-specific value

    eMMPFSocketPos eSocketPos;
    mmpf_bool bDebugDumpPcap;
    // rtcp-xr
    mmpf_bool bSupportedRTCPXr;
    tMMPFRTCPXrInfo stRTCPXrInfo;

    mmpf_bool bReceivingIPFiltering;
    mmpf_uint32 nMaxFragmentUnitSize;
    /* For MTK model becaure of Chipset Limitation */
    mmpf_bool bEPDGCondition;
    mmpf_uint32 nNetworkID;
    // RFC 3550 6.4 Sender and Receiver Reports
    // The SR is issued if a site has sent any data packets during the interval
    // since issuing the last report or the previous one,
    // otherwise the RR is issued.
    mmpf_bool bSendRedundantRTCPSR;
    mmpf_int32 nSlot_ID;  // Slot ID of AP for dual SIM Models.
} tMMPFNetworkInfo;

typedef struct _tMMPFDisplayInfo
{
    mmpf_bool nEnable;  // 0 - disable, 1 - enable
    mmpf_uint32 nRot;   // (0, 90, 180, 270)
                        // In case of Android platform, this value is not used.
    eMMPFOrientation eOrientation;
    mmpf_uint32 nX;                      // In case of Android platform, this value is not used.
    mmpf_uint32 nY;                      // In case of Android platform, this value is not used.
    mmpf_uint32 nWidth;                  // In case of Android platform,
                                         // this value must be same with width of codec parameter.
    mmpf_uint32 nHeight;                 // In case of Android platform,
                                         // this value must be same with height of codec parameter.
    eDisplayHandleType eDispHandleType;  // type of pvDispHandle,
    void* pvDispHandle;                  // android specific
                                         // pointer of Surface or ISurface object
                                         //  : Surface* or ISurface*
                                         // If this value is NULL, MMPF will create Surface.
                                         // If this value is NOT NULL,
                         //  MMPF will use this Surface to display video(recommended)
} tMMPFDisplayInfo;

typedef struct _tMMPFProperty
{
    eMMPFProperty type_property;
    eMMPFModeType type_mode;
    eMMPFMediaType type_media;

    union
    {
        tMMPFSourceInfo txsource;  // MMPF_MODE_TX only
        tMMPFCodecInfo codec;
        tMMPFTransportInfo transport;
        tMMPFNetworkInfo network;
        tMMPFDisplayInfo display;  // MMPF_MEDIA_VIDEO only
        tMMPFDebugInfo debug;
    };
} tMMPFProperty;

/**************************************************
 * definitions for Request
 **************************************************/

typedef struct _tMMPFRequestParam_Record
{
    mmpf_str szDestinationFileName[MMPF_MAX_FILE_LEN];
    mmpf_str szSpsConfig[MMPF_MAX_RECORD_CONFIG_LEN];
    mmpf_str szPpsConfig[MMPF_MAX_RECORD_CONFIG_LEN];
    mmpf_uint32 nBitRate;
    mmpf_uint32 nFrameRate;
    mmpf_uint32 nWidth;
    mmpf_uint32 nHeight;
    mmpf_uint32 nLevel;
    mmpf_bool bOverwrite;
    mmpf_bool bBitRateCtrl;
} tMMPFRequestParam_Record;

typedef struct _tMMPFRequestParam_DTMF
{
    mmpf_str szEvent[MMPF_MAX_DTMF_LEN];
} tMMPFRequestParam_DTMF;

typedef struct _tMMPFRequestParam_EPDG
{
    mmpf_bool bEPDGCondition;
    mmpf_uint32 nToSValue;
    /* For MTK model becaure of Chipset Limitation */
    mmpf_uint32 nNetworkID;
} tMMPFRequestParam_EPDG;

typedef struct _tMMPFRequestParam_NetworkNoRTPTimer
{
    mmpf_uint32 nNoNetworkNoRTPTimer;
} tMMPFRequestParam_NetworkNoRTPTimer;

typedef struct _tMMPFRequestParam_MTU
{
    mmpf_uint32 nMtu;
} tMMPFRequestParam_MTU;

typedef struct _tMMPFDataParam_RTT
{
    mmpf_uint32 nSize;
    mmpf_str szEvent[MMPF_MAX_RTT_LEN];
} tMMPFDataParam_RTT;

typedef struct _tMMPFDataParam_RTT_Audio
{
    mmpf_bool bRttAudioActivated;
} tMMPFDataParam_RTT_Audio;

typedef struct _tMMPFRequest
{
    eMMPFRequest type_request;
    eMMPFModeType type_mode;
    eMMPFMediaType type_media;

    union
    {
        tMMPFRequestParam_Record record;
        tMMPFRequestParam_DTMF dtmf;
        tMMPFRequestParam_EPDG EPDG;
        tMMPFRequestParam_NetworkNoRTPTimer rtpTimer;
        tMMPFRequestParam_RTPANALYZER_Option RTPAnalyzerOption;
        tMMPFRequestParam_MTU MTUSize;
        tMMPFDataParam_RTT rtt;
        tMMPFDataParam_RTT_Audio RttAudio;
    };
} tMMPFRequest;

typedef struct _tMMPFGetInfoParam_CPAddr
{
    mmpf_int32 nPdpProfileNum;         // 0 for default
    mmpf_str szAPN[MMPF_MAX_APN_LEN];  // APN, If APN=="", don't set APN configue.
                                       // If nPdpProfileNum is not 0,
                                       //  this field is ignored.
    mmpf_int32 nPdpProfileNumOf3G;     // -1 (Do not use) or operator-specific value
    // eMMPFNetworkType     eNetworkType;              // distinguish 3G/LTE, U+ only => [DO NOT
    // USE]
    //   substitute : nPdpProfileNumOf3G
    mmpf_int32 nSlot_ID;  // Slot ID of AP for dual SIM Models.
} tPDPInfo;

typedef struct _tMMPFVideoResolution
{
    mmpf_uint32 nWidth;
    mmpf_uint32 nHeight;
} tMMPFVideoResolution;

typedef struct _tMMPFCodecAttribute
{
    mmpf_uint32 nWidth;
    mmpf_uint32 nHeight;
    mmpf_uint32 nProfile;
    mmpf_uint32 nLevel;
} tMMPFCodecAttribute;

typedef struct _tMMPFGetInfoParam
{
    eMMPFInfo type;

    union
    {
        mmpf_str szFileName[MMPF_MAX_FILE_LEN];      // MMPF_INFO_FILE_3GPP
        mmpf_str szSpropParam[MMPF_MAX_CONFIG_LEN];  // MMPF_INFO_PARSE_SPROPPARAM, sprop-param
        tPDPInfo tGetCPAddrInfoParam;                // MMPF_INFO_CP_IP_ADDR
        tMMPFCodecAttribute tCodecAttribute;         // MMPF_INFO_GET_CODEC_ATTRIBUTE
        // MMPF_INFO_SET_AUDIO_CAL_INFO_PARAM,
        mmpf_str szAudioCalInfoParam[MMPF_MAX_AUDIO_CAL_INFO_LEN];
    };
} tMMPFGetInfoParam;

typedef struct _tMMPFSetDebugLogParam
{
    mmpf_uint32 nParamDebugLog;

} tMMPFSetDebugLogParam;

typedef struct _tMMPFFileInfo3GPP
{
    eMMPFVideoCodecType eVideoCodec;  // video codec type
    eMMPFAudioCodecType eAudioCodec;  // audio codec type
    eMMPFTextCodecType eTextCodec;    // text codec type

    // video info
    struct
    {
        mmpf_uint32 nWidth;
        mmpf_uint32 nHeight;
        mmpf_uint32 nBitRate;
        mmpf_uint32 nFrameRate;

        union
        {
            struct
            {
                mmpf_uint32 nProfile;
                mmpf_uint32 nLevel;
            } H263;

            struct
            {
                // mepg-4(part2, part10) config string size
                mmpf_uint32 nConfigSize;
                // mpeg-4(part2, part10) config string, binary format
                mmpf_uint8 pbConfigString[MMPF_MAX_CONFIG_LEN];
            } MP4V;
        };
    } tVideoSpecificData;

    // audio info
    struct
    {
        union
        {
            struct
            {
                mmpf_uint16 nModeSet;  // parser output, AMR mode_set, 3GPP TS 26.244
            } AMR;
        };
    } tAudioSpecificData;
} tMMPFFileInfo3GPP;

typedef struct _tMMPFFileInfoJPEG
{
    mmpf_uint32 nVideoWidth;
    mmpf_uint32 nVideoHeight;
} tMMPFFileInfoJPEG;

typedef struct _tMMPFGetInfoCameraResult
{
    mmpf_int32 nCameraFacing;
    mmpf_int32 nCameraSensorOrientation;
} tMMPFGetInfoCameraResult;

typedef struct _tMMPFGetInfoResult
{
    eMMPFInfo type;

    union
    {
        tMMPFFileInfo3GPP fileinfo3gpp;
        tMMPFCodecAttribute tCodecAttribute;  // MMPF_INFO_GET_CODEC_ATTRIBUTE
        mmpf_str szIpAddrOfCP[MMPF_MAX_IP_LEN];
        mmpf_str szSpropParam[MMPF_MAX_CONFIG_LEN];
        tMMPFGetInfoCameraResult strCameraResult;
    };
} tMMPFGetInfoResult;

typedef struct _tMMPFStatisticsVideoDebugInfo  // Send FrameRate and BitRate
{
    eMMPFMediaType eMediaType;  // Audio: 0, Video: 1, audioVideo: 2,
    mmpf_uint32 nRecvFrameRate;
    mmpf_uint32 nRecvBitRate;
    mmpf_uint32 nSendFrameRate;
    mmpf_uint32 nSendBitRate;
    mmpf_uint32 nVQIndicator;
} tMMPFStatisticsVideoDebugInfo;

typedef struct _tMMPFStatisticsCIQ
{
    eMMPFMediaType mediaType;          // audio? video?
    eMMPFModeType modeType;            // RX? TX?
    mmpf_uint32 firstSSRC;             // SSRC
    mmpf_uint32 durationTime;          // duration Time during a call
    mmpf_uint32 totalPacket;           // total packet count
    mmpf_uint32 networkLostPacketCnt;  // lost packet count by network
    mmpf_uint32 droppedPacketCnt;      // deleted packet count caused by playback keep time
                                       //  in jitter buffer
    mmpf_uint32 latePacketCnt;         // deleted packet count caused by lating in jitter buffer
    mmpf_uint32 meanJitter;            // meanJitter , (use converting un-signed int value)
    mmpf_uint32 maxJitter;             // J(i) = J(i-1) + (|D(i-1,i)| - J(i-1))/16 ,
                                       //  (use converting un-signed int value)
    mmpf_uint32 maxDelta;              // D(i,j) = (Rj - Ri) - (Sj - Si) = (Rj - Sj) - (Ri - Si)
                                       //  (use converting un-signed int value)
    mmpf_uint32 meanDejitterbufferSize;
    mmpf_uint32 meanDecoderDelay;
    mmpf_uint32 updatedCMR;
    mmpf_bool bPktAudio;
} tMMPFStatisticsCIQ;

typedef struct _tMMPFStatisticsRTP
{
    eMMPFMediaType eMediaType;  // Audio: 0, Video: 1, text: 2,
    mmpf_uint32 nRxTotalPacketCnt;
    mmpf_uint32 nRtpTimeThresholdCnt;
    mmpf_uint32 nMeanJitter;
    mmpf_uint32 nMaxJitter;
    mmpf_uint32 nLostPacketCnt;  // lost packet count in local DUT
} tMMPFStatisticsRTP;

typedef struct _tMMPFStatisticsDRA  // TMOS DRA MMPF Media Trace
{
    eMMPFMediaType eMediaType;  // Audio: 0, Video: 1, audioVideo: 2,
    eMMPFModeType eModeType;    // Tx, Rx, TRx,
    mmpf_int32 nLossRate;
    mmpf_int32 nDelay;
    mmpf_int32 nJitter;
    mmpf_int32 nMeasuredPeriod;
    mmpf_bool bPktAudio;
} tMMPFStatisticsDRA;

typedef struct _tMMPFVideoDataUsage
{
    mmpf_int64 nIpDataSizeRx;
    mmpf_int64 nIpDataSizeTx;
    mmpf_int64 nReserved1;
    mmpf_int64 nReserved2;
} tMMPFVideoDataUsage;

typedef struct
{
    unsigned int ssrc;
    unsigned int fractionLost;
    unsigned int cumPktsLost;
    unsigned int extHighSeqNum;
    unsigned int jitter;
    unsigned int lsr;
    unsigned int delayLsr;
} tRtpSvcRecvReport;

typedef struct
{
    unsigned int ntpTimeStampMsw;
    unsigned int ntpTimeStampLsw;
    unsigned int rtpTimeStamp;
    unsigned int sendPktCount;
    unsigned int sendOctCount;
    tRtpSvcRecvReport stRecvRpt;  // only one RR block is supported.
} tMMPFNotifyReceiveRtcpSrInd;

typedef struct
{
    tRtpSvcRecvReport stRecvRpt;  // only one RR block is supported.
} tMMPFNotifyReceiveRtcpRrInd;

typedef struct
{
    unsigned int nRtpPacket;
    unsigned int nRtcpSRPacket;
    unsigned int nRtcpRRPacket;
} tMMPFNotifyNumReceivePkt;

typedef struct _tMMPFNotificationData
{
    union
    {
        tMMPFNotifyNumReceivePkt numPacket;
        tMMPFNotifyReceiveRtcpSrInd rtcpSrInd;
        tMMPFNotifyReceiveRtcpRrInd rtcpRrInd;
        tMMPFVideoResolution videoResolution;
        tMMPFStatisticsVideoDebugInfo statDebug;
        tMMPFStatisticsCIQ statCIQ;
        tMMPFStatisticsRTP statRTP;
        tMMPFStatisticsDRA statDRA;
        tMMPFVideoDataUsage videoDataUsage;
        tMMPFDataParam_RTT rtt;
    };
} tMMPFNotificationData;

typedef enum _tMMPFEvent_JNI
{
    MMPF_EVENT_TYPE_CAMERA = 0,
    MMPF_EVENT_TYPE_DECODER = 1,
    MMPF_EVENT_TYPE_ENCODER = 2,
    MMPF_EVENT_TYPE_IMAGE = 3,
    MMPF_EVENT_TYPE_AUDIOENCODER = 4,
    MMPF_EVENT_TYPE_AUDIODECODER = 5,
} tMMPFEvent_JNI;

typedef enum _eMMPFJNIEventResponse
{
    MMPF_OK = 0,
    MMPF_NOK = 1,
    MMPF_ERROR = 99,
} eMMPFJNIEventResponse;

/****************************************************
 * definitions for RTCP SR, RR
 *    use same structures with RTCP SR/RR structures in feature phone
 ****************************************************/

#ifndef _MMPF_RTP_COMMON_DEFINITION_
#define _MMPF_RTP_COMMON_DEFINITION_

struct MMPFSize
{
    mmpf_uint32 nWidth, nHeight;
    MMPFSize() :
            nWidth(0),
            nHeight(0)
    {
    }

    MMPFSize(mmpf_uint32 w, mmpf_uint32 h) :
            nWidth(w),
            nHeight(h)
    {
    }

    MMPFSize& operator=(const MMPFSize& p)
    {
        nWidth = p.nWidth;
        nHeight = p.nHeight;
        return *this;
    }
};

#endif  // _MMPF_RTP_COMMON_DEFINITION_

#endif  // MMPF_MMPFDEFINITION_H_INCLUDED
