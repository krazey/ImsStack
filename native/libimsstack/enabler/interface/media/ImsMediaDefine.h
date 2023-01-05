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

enum ImsMediaResult
{
    IMS_MEDIA_OK = 0,
    IMS_MEDIA_ERROR_UNKNOWN,
    IMS_MEDIA_ERROR_INVALID_REQUEST,
    IMS_MEDIA_ERROR_INVALID_ARGUMENT,
};

enum ImsMediaEventType
{
    IMS_MEDIA_EVENT_RESPONSE,
    IMS_MEDIA_EVENT_NOTIFY,
};

enum ImsMediaNotifyType
{
    RECV_AUDIO_RX_PACKET_RECEIVED = 0,
    SEND_AUDIO_TX_FIRST_PACKET_SENT = 1,
    ERROR_RTP_TIMEOUT_NO_AUDIO_RX_PACKET,
    ERROR_SOCKET,
    DTMF_KEY_0 = 600,
    DTMF_KEY_1,
    DTMF_KEY_2,
    DTMF_KEY_3,
    DTMF_KEY_4,
    DTMF_KEY_5,
    DTMF_KEY_6,
    DTMF_KEY_7,
    DTMF_KEY_8,
    DTMF_KEY_9,
    DTMF_KEY_STAR,
    DTMF_KEY_POUND,
    DTMF_KEY_A,
    DTMF_KEY_B,
    DTMF_KEY_C,
    DTMF_KEY_D,
};

enum ImsMediaStreamType
{
    STREAM_MODE_RTP_TX,
    STREAM_MODE_RTP_RX,
    STREAM_MODE_RTCP,
};

enum ImsMediaType
{
    IMS_MEDIA_AUDIO = 0,
    IMS_MEDIA_VIDEO,
    IMS_MEDIA_TEXT,
};

enum ImsMediaSubType
{
    MEDIASUBTYPE_UNDEFINED,
    MEDIASUBTYPE_RTPPAYLOAD, // rtp payload header + encoded bitstream
    MEDIASUBTYPE_RTPPACKET, // rtp packet
    MEDIASUBTYPE_RTCPPACKET, // rtcp packet
    MEDIASUBTYPE_RTCPPACKET_BYE, // rtcp packet
    MEDIASUBTYPE_RAWDATA, // raw yuv or pcm data
    MEDIASUBTYPE_RAWDATA_ROT90, // inverse rotated raw yuv data ( need to ratate 90 degree)
    MEDIASUBTYPE_RAWDATA_ROT90_FLIP, //inverse rotated raw yuv data and flip
    MEDIASUBTYPE_RAWDATA_ROT270,
    MEDIASUBTYPE_RAWDATA_CROP_ROT90, // inverse and crop the rotated raw yuv data
                                     //  (need to ratate 90 degree)
    MEDIASUBTYPE_RAWDATA_CROP_ROT90_FLIP = 10, //inverse and crop the rotated raw yuv data and flip
    MEDIASUBTYPE_RAWDATA_CROP_ROT270, // inverse and crop the rotated raw yuv data
                                      //  S(need to ratate 270 degree)
    MEDIASUBTYPE_RAWDATA_CROP,
    MEDIASUBTYPE_DTMFSTART,
    MEDIASUBTYPE_DTMFEVENT, // rtp payload for dtmf event
    MEDIASUBTYPE_DTMFEND,
    MEDIASUBTYPE_DTXSTART, // EVRC-B, 2011.3.8[MAX_RTP_CONFIGS]
    MEDIASUBTYPE_BITSTREAM_H263, // encoded bitstream
    MEDIASUBTYPE_BITSTREAM_MPEG4, // encoded bitstream
    MEDIASUBTYPE_BITSTREAM_H264, // encoded bitstream
    MEDIASUBTYPE_BITSTREAM_G711_PCMU = 20, // encoded bitstream
    MEDIASUBTYPE_BITSTREAM_G711_PCMA, // encoded bitstream
    MEDIASUBTYPE_BITSTREAM_EVRC,
    MEDIASUBTYPE_BITSTREAM_EVRC_B,
    MEDIASUBTYPE_BITSTREAM_AMR_WB,
    MEDIASUBTYPE_BITSTREAM_AMR,
    MEDIASUBTYPE_BITSTREAM_T140, // T140
    MEDIASUBTYPE_BITSTREAM_T140_RED, // T140 Redendancy
    MEDIASUBTYPE_PCM_DATA, // decoded pcm data
    MEDIASUBTYPE_PCM_NO_DATA, // decoded pcm no data
    MEDIASUBTYPE_NOT_READY, // Jitter Buffer GetData not ready
    MEDIASUBTYPE_BITSTREAM_CODECCONFIG,
    MEDIASUBTYPE_MAX
};

enum RTPPyaloadHeaderMode
{
    // amr mode
    RTPPAYLOADHEADER_MODE_AMR_OCTETALIGNED     = 0, // octet aligned mode
    RTPPAYLOADHEADER_MODE_AMR_EFFICIENT        = 1, // efficient mode
    // avc mode
    RTPPAYLOADHEADER_MODE_AVC_SINGLE_NAL_UNIT = 0, // packet mode 0
    RTPPAYLOADHEADER_MODE_AVC_NON_INTERLEAVED = 1, // packet mode 1
    // evs mode
    RTPPAYLOADHEADER_MODE_EVS_COMPACT          = 0, // EVS compact format 0
    RTPPAYLOADHEADER_MODE_EVS_HEADER_FULL      = 1, // EVS header-full format 1
    RTPPAYLOADHEADER_MODE_MAX
};

enum eIPVersion
{
    IPV4,
    IPV6,
};

enum StreamState
{
    STATE_NULL,
    STATE_CREATED,
    STATE_PREVIEW,
    STATE_PRESTART,
    STATE_RUN,
    STATE_PAUSED,
};