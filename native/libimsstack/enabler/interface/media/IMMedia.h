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

#ifndef INTERFACE_IMMEDIA_H_
#define INTERFACE_IMMEDIA_H_

#include <AudioConfig.h>
#include <VideoConfig.h>
#include <MediaQualityThreshold.h>
#include "ImsMessageDef.h"
#include "ImsTypeDef.h"
#include "IpAddress.h"
#include "MediaDef.h"

#define IMMEDIA     IMS_MSG_BASE_STREAMEDMEDIA
#define IMMEDIA_IND (IMMEDIA + 100)

using namespace android::telephony::imsmedia;

class IMMedia
{
#define IMMEDIA_CASE_ENUM(name) \
    case name:                  \
        return #name

public:
    static const IMS_CHAR* PrintMsg(IN IMS_SINT32 nMsg)
    {
        switch (nMsg)
        {
            IMMEDIA_CASE_ENUM(REQUEST_OPEN_SESSION);
            IMMEDIA_CASE_ENUM(REQUEST_CLOSE_SESSION);
            IMMEDIA_CASE_ENUM(REQUEST_MODIFY_SESSION);
            IMMEDIA_CASE_ENUM(REQUEST_ADD_CONFIG);
            IMMEDIA_CASE_ENUM(REQUEST_DELETE_CONFIG);
            IMMEDIA_CASE_ENUM(REQUEST_CONFIRM_CONFIG);
            IMMEDIA_CASE_ENUM(REQUEST_SEND_DTMF);
            IMMEDIA_CASE_ENUM(REQUEST_SET_MEDIA_QUALITY);
            IMMEDIA_CASE_ENUM(REQUEST_HEADER_EXTENSION);
            IMMEDIA_CASE_ENUM(REQUEST_QOS);
            IMMEDIA_CASE_ENUM(REQUEST_SET_PREVIEW_SURFACE);
            IMMEDIA_CASE_ENUM(REQUEST_SET_DISPLAY_SURFACE);
            IMMEDIA_CASE_ENUM(REQUEST_VIDEO_DATA_USAGE);
            IMMEDIA_CASE_ENUM(RESPONSE_OPEN_SESSION);
            IMMEDIA_CASE_ENUM(RESPONSE_MODIFY_SESSION);
            IMMEDIA_CASE_ENUM(RESPONSE_ADD_CONFIG);
            IMMEDIA_CASE_ENUM(RESPONSE_CONFIRM_CONFIG);
            IMMEDIA_CASE_ENUM(NOTIFY_FIRST_PACKET);
            IMMEDIA_CASE_ENUM(NOTIFY_HEADER_EXTENSION);
            IMMEDIA_CASE_ENUM(NOTIFY_MEDIA_INACTIVITY);
            IMMEDIA_CASE_ENUM(NOTIFY_PACKET_LOSS);
            IMMEDIA_CASE_ENUM(NOTIFY_JITTER);
            IMMEDIA_CASE_ENUM(NOTIFY_CALL_QUALITY_CHANGE);
            IMMEDIA_CASE_ENUM(NOTIFY_QOS_INFO);
            IMMEDIA_CASE_ENUM(NOTIFY_MEDIA_DETACH);
            IMMEDIA_CASE_ENUM(SEND_DTMF);
            IMMEDIA_CASE_ENUM(SETSURFACE_CMD);
            IMMEDIA_CASE_ENUM(SELECT_CAMERA_CMD);
            IMMEDIA_CASE_ENUM(CHANGE_CAMERA_ZOOM_CMD);
            IMMEDIA_CASE_ENUM(SET_PAUSE_IMAGE_CMD);
            IMMEDIA_CASE_ENUM(CHANGE_ORIENTATION_CMD);
            IMMEDIA_CASE_ENUM(CHANGE_NETWORK_CONNECTION);
            IMMEDIA_CASE_ENUM(CHANGE_MTU);
        }
        return "Unrecognized Msg";
    }

public:
    static const IMS_SINT32 MEDIA_MESSAGE_IDX_START = IMMEDIA + 0;

    // Requests to ImsMedia
    static const IMS_SINT32 MEDIA_MESSAGE_AUDIO_COMMON_IDX_START = IMMEDIA + 0;
    /** create a new session */
    static const IMS_SINT32 REQUEST_OPEN_SESSION = IMMEDIA + 1;
    /** close session */
    static const IMS_SINT32 REQUEST_CLOSE_SESSION = IMMEDIA + 2;
    /** update the existing session */
    static const IMS_SINT32 REQUEST_MODIFY_SESSION = IMMEDIA + 3;
    /** add a stream for forking session */
    static const IMS_SINT32 REQUEST_ADD_CONFIG = IMMEDIA + 4;
    /** delete the stream of forking session */
    static const IMS_SINT32 REQUEST_DELETE_CONFIG = IMMEDIA + 5;
    /** remain only one stream in the session */
    static const IMS_SINT32 REQUEST_CONFIRM_CONFIG = IMMEDIA + 6;
    /** send dtmf digit to audio session */
    static const IMS_SINT32 REQUEST_SEND_DTMF = IMMEDIA + 7;
    /** set media quality theshold to check the packet inacitivty, packet loss and jitter */
    static const IMS_SINT32 REQUEST_SET_MEDIA_QUALITY = IMMEDIA + 8;
    /** send header extension payload to rtp header */
    static const IMS_SINT32 REQUEST_HEADER_EXTENSION = IMMEDIA + 9;
    /** send request qos callback */
    static const IMS_SINT32 REQUEST_QOS = IMMEDIA + 10;
    static const IMS_SINT32 MEDIA_MESSAGE_AUDIO_COMMON_IDX_END = IMMEDIA + 49;

    // Requests to ImsMedia VideoSession
    static const IMS_SINT32 MEDIA_MESSAGE_VIDEO_IDX_START = IMMEDIA + 50;
    /** set the preview surface */
    static const IMS_SINT32 REQUEST_SET_PREVIEW_SURFACE = MEDIA_MESSAGE_VIDEO_IDX_START + 1;
    /** set the display surface */
    static const IMS_SINT32 REQUEST_SET_DISPLAY_SURFACE = MEDIA_MESSAGE_VIDEO_IDX_START + 2;
    /** request to notify the amount of data used in video session  */
    static const IMS_SINT32 REQUEST_VIDEO_DATA_USAGE = MEDIA_MESSAGE_VIDEO_IDX_START + 3;
    static const IMS_SINT32 MEDIA_MESSAGE_VIDEO_IDX_END = IMMEDIA + 79;

    // Requests for text
    static const IMS_SINT32 MEDIA_MESSAGE_TEXT_IDX_START = IMMEDIA + 80;
    static const IMS_SINT32 MEDIA_MESSAGE_TEXT_IDX_END = IMMEDIA + 99;

    static const IMS_SINT32 MEDIA_MESSAGE_IDX_END = MEDIA_MESSAGE_TEXT_IDX_END;

    // Response & Notification
    static const IMS_SINT32 MEDIA_MESSAGE_IND_IDX_START = IMMEDIA_IND + 0;

    static const IMS_SINT32 MEDIA_MESSAGE_AUDIO_COMMON_IND_IDX_START = IMMEDIA_IND + 0;
    /** response of openSession request  */
    static const IMS_SINT32 RESPONSE_OPEN_SESSION = IMMEDIA_IND + 1;
    /** response of modifySession request  */
    static const IMS_SINT32 RESPONSE_MODIFY_SESSION = IMMEDIA_IND + 2;
    /** response of addConfig request  */
    static const IMS_SINT32 RESPONSE_ADD_CONFIG = IMMEDIA_IND + 3;
    /** response of confirmConfig request  */
    static const IMS_SINT32 RESPONSE_CONFIRM_CONFIG = IMMEDIA_IND + 4;
    /** response of closeSession request  */
    static const IMS_SINT32 RESPONSE_SESSION_CLOSED = IMMEDIA_IND + 5;
    /** notification of first packet received in the target session during the streaming */
    static const IMS_SINT32 NOTIFY_FIRST_PACKET = IMMEDIA_IND + 11;
    /** notification of rtp extended header received */
    static const IMS_SINT32 NOTIFY_HEADER_EXTENSION = IMMEDIA_IND + 12;
    /** notification of rtp/rtcp packet inacitivity detected */
    static const IMS_SINT32 NOTIFY_MEDIA_INACTIVITY = IMMEDIA_IND + 13;
    /** notification of packet loss detected */
    static const IMS_SINT32 NOTIFY_PACKET_LOSS = IMMEDIA_IND + 14;
    /** notification of jitter over threshold detected */
    static const IMS_SINT32 NOTIFY_JITTER = IMMEDIA_IND + 15;
    /** notification of media call quality changed */
    static const IMS_SINT32 NOTIFY_CALL_QUALITY_CHANGE = IMMEDIA_IND + 16;
    /** notification of the ImsMedia process disconnected  */
    static const IMS_SINT32 NOTIFY_MEDIA_DETACH = IMMEDIA_IND + 17;
    /** notification of session qos callback */
    static const IMS_SINT32 NOTIFY_QOS_INFO = IMMEDIA_IND + 18;
    /** request from the Ui to send a dtmf digit to the audio session */
    static const IMS_SINT32 SEND_DTMF = IMMEDIA_IND + 19;
    static const IMS_SINT32 MEDIA_MESSAGE_AUDIO_COMMON_IND_IDX_END = IMMEDIA_IND + 49;

    // Notifications for video
    static const IMS_SINT32 MEDIA_MESSAGE_VIDEO_IND_IDX_START = IMMEDIA_IND + 50;
    /** request from the Ui to update a surface buffer */
    static const IMS_SINT32 SETSURFACE_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 1;
    /** request from the Ui to update the camera id */
    static const IMS_SINT32 SELECT_CAMERA_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 2;
    /** request from the Ui to update camera zoom value */
    static const IMS_SINT32 CHANGE_CAMERA_ZOOM_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 3;
    /** request from the Ui to update the path of paused image */
    static const IMS_SINT32 SET_PAUSE_IMAGE_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 4;
    /** request from the Ui to update device orientation change */
    static const IMS_SINT32 CHANGE_ORIENTATION_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 5;
    static const IMS_SINT32 MEDIA_MESSAGE_VIDEO_IND_IDX_END = IMMEDIA_IND + 79;

    static const IMS_SINT32 MEDIA_MESSAGE_TEXT_IND_IDX_START = IMMEDIA_IND + 80;
    static const IMS_SINT32 MEDIA_MESSAGE_TEXT_IND_IDX_END = IMMEDIA_IND + 99;

    static const IMS_SINT32 MEDIA_MESSAGE_INTERNAL_IND_IDX_START = IMMEDIA_IND + 120;
    static const IMS_SINT32 MEDIA_MESSAGE_INTERNAL_IND_IDX_END = IMMEDIA_IND + 129;

    static const IMS_SINT32 CHANGE_NETWORK_CONNECTION = MEDIA_MESSAGE_INTERNAL_IND_IDX_START + 0;
    static const IMS_SINT32 CHANGE_MTU = MEDIA_MESSAGE_INTERNAL_IND_IDX_START + 1;

    static const IMS_SINT32 MEDIA_MESSAGE_IND_IDX_END = MEDIA_MESSAGE_INTERNAL_IND_IDX_END;
};

enum RtpError
{
    /** Success */
    NO_ERROR = 0,
    /** Invalid parameters passed in the request */
    INVALID_PARAM = 1,
    /** The RTP stack is not ready to handle the request */
    NOT_READY = 2,
    /** Unable to handle the request due to memory allocation failure */
    NO_MEMORY = 3,
    /**
     * Unable to handle the request due to no sufficient resources such as
     * Audio output, audio output, codec
     */
    NO_RESOURCES = 4,
    /** The requested port number is not available */
    PORT_UNAVAILABLE = 5,
    /** The request is not supported by the implementation */
    REQUEST_NOT_SUPPORTED = 6,

    // Additional for Ims Internal enums start from 20
    /** The response is not received within certain time */
    RESPONSE_WAIT_TIMEOUT = 20,
};

enum SessionType
{
    SESSION_TYPE_AUDIO = 0,
    SESSION_TYPE_VIDEO = 1,
    SESSION_TYPE_RTT = 2,
};

enum ProtocolType
{
    RTP = 0,
    RTCP = 1,
};

enum
{
    SURFACE_FAR = 1,
    SURFACE_NEAR = 2,
};

class ImsMediaMsgParamBase
{
public:
    explicit ImsMediaMsgParamBase(const MEDIA_CONTENT_TYPE eType = MEDIA_TYPE_INVALID) :
            m_eMediaType(eType){};
    virtual ~ImsMediaMsgParamBase() {}

public:
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaMsgSetMediaQualityParam : public ImsMediaMsgParamBase
{
public:
    explicit ImsMediaMsgSetMediaQualityParam(const MEDIA_CONTENT_TYPE eType) :
            ImsMediaMsgParamBase(eType),
            m_objMediaQualityThreshold(MediaQualityThreshold()){};

public:
    MediaQualityThreshold m_objMediaQualityThreshold;
};

class ImsMediaMsgConfigParam : public ImsMediaMsgParamBase
{
public:
    explicit ImsMediaMsgConfigParam(const MEDIA_CONTENT_TYPE eType) :
            ImsMediaMsgParamBase(eType),
            m_pConfig(IMS_NULL){};
    virtual ~ImsMediaMsgConfigParam()
    {
        if (m_pConfig != IMS_NULL)
        {
            if (m_eMediaType == MEDIA_TYPE_AUDIO)
            {
                delete reinterpret_cast<AudioConfig*>(m_pConfig);
            }
            else if (m_eMediaType == MEDIA_TYPE_VIDEO)
            {
                delete reinterpret_cast<VideoConfig*>(m_pConfig);
            }
        }
    }

public:
    RtpConfig* m_pConfig;
};

class ImsMediaMsgOpenConfigParam : public ImsMediaMsgConfigParam
{
public:
    explicit ImsMediaMsgOpenConfigParam(const MEDIA_CONTENT_TYPE type) :
            ImsMediaMsgConfigParam(type),
            m_objLocalAddress(IPAddress::IPv6NONE),
            m_nLocalPort(0){};
    virtual ~ImsMediaMsgOpenConfigParam() {}

public:
    IPAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
};

class ImsMediaMsgDtmfParam : public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgDtmfParam() :
            ImsMediaMsgParamBase(MEDIA_TYPE_AUDIO),
            m_dtmfCode(-1),
            m_nDuration(0){};

public:
    IMS_CHAR m_dtmfCode;
    IMS_SINT32 m_nDuration;
};

class ImsMediaResponseParamBase : public ImsMediaMsgParamBase
{
public:
    explicit ImsMediaResponseParamBase(const MEDIA_CONTENT_TYPE type = MEDIA_TYPE_AUDIO) :
            ImsMediaMsgParamBase(type),
            m_eResult(RtpError::NO_ERROR){};
    virtual ~ImsMediaResponseParamBase() {}

public:
    IMS_SINT32 m_eResult;
};

class ImsMediaResponseConfigParam : public ImsMediaResponseParamBase
{
public:
    explicit ImsMediaResponseConfigParam(RtpConfig* config = IMS_NULL) :
            ImsMediaResponseParamBase(),
            m_pConfig(config){};
    virtual ~ImsMediaResponseConfigParam()
    {
        if (m_pConfig != IMS_NULL)
        {
            if (m_eMediaType == MEDIA_TYPE_AUDIO)
            {
                delete reinterpret_cast<AudioConfig*>(m_pConfig);
            }
            else if (m_eMediaType == MEDIA_TYPE_VIDEO)
            {
                delete reinterpret_cast<VideoConfig*>(m_pConfig);
            }
        }
    }

public:
    RtpConfig* m_pConfig;
};

class ImsMediaMsgQosParam : public ImsMediaResponseParamBase
{
public:
    explicit ImsMediaMsgQosParam(const MEDIA_CONTENT_TYPE type = MEDIA_TYPE_AUDIO,
            const IPAddress& address = IPAddress(IPAddress::IPv6NONE), const IMS_SINT32 port = 0) :
            ImsMediaResponseParamBase(type),
            m_objIpAddress(address),
            m_nPort(port),
            m_bCallback(IMS_FALSE),
            m_bResult(IMS_FALSE)
    {
    }

    bool operator==(const ImsMediaMsgQosParam& param)
    {
        return (this->m_eMediaType == param.m_eMediaType &&
                this->m_objIpAddress == param.m_objIpAddress && this->m_nPort == param.m_nPort);
    }

public:
    IPAddress m_objIpAddress;
    IMS_SINT32 m_nPort;
    IMS_BOOL m_bCallback;
    IMS_BOOL m_bResult;
};

class ImsMediaNotifyInactivityParam
{
public:
    ImsMediaNotifyInactivityParam() :
            m_eMediaProtocolType(RTP),
            m_eMediaType(MEDIA_TYPE_INVALID){};

public:
    ProtocolType m_eMediaProtocolType;
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaNotifyPacketParam : public ImsMediaResponseParamBase
{
public:
    ImsMediaNotifyPacketParam() :
            m_nResponse(-1){};

public:
    IMS_SINT32 m_nResponse;
};

class ImsMediaVideoParam : public ImsMediaMsgParamBase
{
public:
    explicit ImsMediaVideoParam(IMS_SINT32 value = -1) :
            ImsMediaMsgParamBase(MEDIA_TYPE_VIDEO),
            nValue(value){};

public:
    IMS_SINT32 nValue;
};

class ImsMediaVideoResolutionParam : public ImsMediaMsgParamBase
{
public:
    explicit ImsMediaVideoResolutionParam(IMS_SINT32 value1 = -1, IMS_SINT32 value2 = -1) :
            ImsMediaMsgParamBase(MEDIA_TYPE_VIDEO),
            nWidth(value1),
            nHeight(value2){};

public:
    IMS_SINT32 nWidth;
    IMS_SINT32 nHeight;
};

#endif
