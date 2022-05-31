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

#ifndef _IMMEDIA_H_
#define _IMMEDIA_H_

#include <AudioConfig.h>
#include <VideoConfig.h>
#include <MediaQualityThreshold.h>
#include "ImsMessageDef.h"
#include "IMSTypeDef.h"
#include "IPAddress.h"
#include "MediaDef.h"

#define IMMEDIA     IMS_MSG_BASE_STREAMEDMEDIA
#define IMMEDIA_IND IMMEDIA + 100

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
            IMMEDIA_CASE_ENUM(REQUEST_SET_PREVIEW_SURFACE);
            IMMEDIA_CASE_ENUM(REQUEST_SET_DISPLAY_SURFACE);
            IMMEDIA_CASE_ENUM(REQUEST_VIDEO_DATA_USAGE);
            IMMEDIA_CASE_ENUM(RESPONSE_OPEN_SESSION);
            IMMEDIA_CASE_ENUM(RESPONSE_SESSION_CHANGED);
            IMMEDIA_CASE_ENUM(RESPONSE_MODIFY_SESSION);
            IMMEDIA_CASE_ENUM(RESPONSE_ADD_CONFIG);
            IMMEDIA_CASE_ENUM(RESPONSE_CONFIRM_CONFIG);
            IMMEDIA_CASE_ENUM(NOTIFY_FIRST_PACKET);
            IMMEDIA_CASE_ENUM(NOTIFY_HEADER_EXTENSION);
            IMMEDIA_CASE_ENUM(NOTIFY_MEDIA_INACTIVITY);
            IMMEDIA_CASE_ENUM(NOTIFY_PACKET_LOSS);
            IMMEDIA_CASE_ENUM(NOTIFY_JITTER);
            IMMEDIA_CASE_ENUM(NOTIFY_MEDIA_QUALITY_CHANGE);
            IMMEDIA_CASE_ENUM(NOTIFY_MEDIA_DETACH);
            IMMEDIA_CASE_ENUM(NOTIFY_QOS_INFO);
            IMMEDIA_CASE_ENUM(SETSURFACE_CMD);
            IMMEDIA_CASE_ENUM(SELECT_CAMERA_CMD);
            IMMEDIA_CASE_ENUM(CHANGE_CAMERA_ZOOM_CMD);
            IMMEDIA_CASE_ENUM(SET_PAUSE_IMAGE_CMD);
            IMMEDIA_CASE_ENUM(CHANGE_ORIENTATION_CMD);
        }
        return "Unrecognized Msg";
    }

public:
    enum MessageType
    {
        MSG_NONE = 0,
        MSG_REQUEST,
        MSG_REQUEST_SET_WAIT,
        MSG_RESPONSE,
        MSG_RESPONSE_RELEASE_WAIT,
        MSG_NOTIFICATION,

        // for Video
        MSG_VIDEO_REQUEST,
        MSG_VIDEO_NOTIFICATION,
    };

    static const IMS_SINT32 MEDIA_MESSAGE_IDX_START = IMMEDIA + 0;

    // Requests
    static const IMS_SINT32 REQUEST_OPEN_SESSION = IMMEDIA + 1;
    static const IMS_SINT32 REQUEST_CLOSE_SESSION = IMMEDIA + 2;
    static const IMS_SINT32 REQUEST_MODIFY_SESSION = IMMEDIA + 3;
    static const IMS_SINT32 REQUEST_ADD_CONFIG = IMMEDIA + 4;
    static const IMS_SINT32 REQUEST_DELETE_CONFIG = IMMEDIA + 5;
    static const IMS_SINT32 REQUEST_CONFIRM_CONFIG = IMMEDIA + 6;
    static const IMS_SINT32 REQUEST_SEND_DTMF = IMMEDIA + 7;
    static const IMS_SINT32 REQUEST_SET_MEDIA_QUALITY = IMMEDIA + 8;
    static const IMS_SINT32 REQUEST_HEADER_EXTENSION = IMMEDIA + 9;

    // Requests for video
    static const IMS_SINT32 MEDIA_MESSAGE_VIDEO_IDX_START = IMMEDIA + 50;
    static const IMS_SINT32 REQUEST_SET_PREVIEW_SURFACE = MEDIA_MESSAGE_VIDEO_IDX_START + 1;
    static const IMS_SINT32 REQUEST_SET_DISPLAY_SURFACE = MEDIA_MESSAGE_VIDEO_IDX_START + 2;
    static const IMS_SINT32 REQUEST_VIDEO_DATA_USAGE = MEDIA_MESSAGE_VIDEO_IDX_START + 3;

    static const IMS_SINT32 MEDIA_MESSAGE_IDX_END = IMMEDIA + 99;

    static const IMS_SINT32 MEDIA_MESSAGE_IND_IDX_START = IMMEDIA_IND + 0;

    static const IMS_SINT32 RESPONSE_OPEN_SESSION = IMMEDIA_IND + 1;
    static const IMS_SINT32 RESPONSE_SESSION_CHANGED = IMMEDIA_IND + 2;
    static const IMS_SINT32 RESPONSE_MODIFY_SESSION = IMMEDIA_IND + 3;
    static const IMS_SINT32 RESPONSE_ADD_CONFIG = IMMEDIA_IND + 4;
    static const IMS_SINT32 RESPONSE_CONFIRM_CONFIG = IMMEDIA_IND + 5;
    static const IMS_SINT32 NOTIFY_FIRST_PACKET = IMMEDIA_IND + 6;
    static const IMS_SINT32 NOTIFY_HEADER_EXTENSION = IMMEDIA_IND + 7;
    static const IMS_SINT32 NOTIFY_MEDIA_INACTIVITY = IMMEDIA_IND + 8;
    static const IMS_SINT32 NOTIFY_PACKET_LOSS = IMMEDIA_IND + 9;
    static const IMS_SINT32 NOTIFY_JITTER = IMMEDIA_IND + 10;
    static const IMS_SINT32 NOTIFY_MEDIA_QUALITY_CHANGE = IMMEDIA_IND + 11;
    static const IMS_SINT32 NOTIFY_MEDIA_DETACH = IMMEDIA_IND + 12;

    static const IMS_SINT32 NOTIFY_QOS_INFO = IMMEDIA_IND + 20;

    // Notifications for video
    static const IMS_SINT32 MEDIA_MESSAGE_VIDEO_IND_IDX_START = IMMEDIA_IND + 50;
    static const IMS_SINT32 SETSURFACE_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 1;
    static const IMS_SINT32 SELECT_CAMERA_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 2;
    static const IMS_SINT32 CHANGE_CAMERA_ZOOM_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 3;
    static const IMS_SINT32 SET_PAUSE_IMAGE_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 4;
    static const IMS_SINT32 CHANGE_ORIENTATION_CMD = MEDIA_MESSAGE_VIDEO_IND_IDX_START + 5;

    static const IMS_SINT32 MEDIA_MESSAGE_IND_IDX_END = IMMEDIA_IND + 99;

public:
    static MessageType CategorizeMessageType(IN IMS_SINT32 nMsg)
    {
        MessageType nMsgType = MSG_NONE;

        if (nMsg >= REQUEST_OPEN_SESSION && nMsg <= REQUEST_HEADER_EXTENSION)
        {
            nMsgType = MSG_REQUEST;

            if (nMsg == REQUEST_OPEN_SESSION || nMsg == REQUEST_MODIFY_SESSION ||
                    nMsg == REQUEST_ADD_CONFIG || nMsg == REQUEST_CONFIRM_CONFIG)
            {
                nMsgType = MSG_REQUEST_SET_WAIT;
            }
        }
        else if (nMsg >= RESPONSE_OPEN_SESSION && nMsg <= NOTIFY_FIRST_PACKET)
        {
            nMsgType = MSG_RESPONSE;

            if (nMsg == RESPONSE_OPEN_SESSION || nMsg == RESPONSE_MODIFY_SESSION ||
                    nMsg == RESPONSE_ADD_CONFIG || nMsg == RESPONSE_CONFIRM_CONFIG)
            {
                nMsgType = MSG_RESPONSE_RELEASE_WAIT;
            }
        }
        else if (nMsg >= NOTIFY_HEADER_EXTENSION && nMsg <= NOTIFY_QOS_INFO)
        {
            nMsgType = MSG_NOTIFICATION;
        }
        else if (nMsg >= REQUEST_SET_PREVIEW_SURFACE && nMsg <= REQUEST_VIDEO_DATA_USAGE)
        {
            nMsgType = MSG_VIDEO_REQUEST;
        }
        else if (nMsg >= SETSURFACE_CMD && nMsg <= CHANGE_ORIENTATION_CMD)
        {
            nMsgType = MSG_VIDEO_NOTIFICATION;
        }

        return nMsgType;
    }
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

enum
{
    SURFACE_FAR = 1,
    SURFACE_NEAR = 2,
};

class ImsMediaMsgParamBase
{
public:
    ImsMediaMsgParamBase(MEDIA_CONTENT_TYPE type = MEDIA_TYPE_INVALID) :
            m_eMediaType(type){};

public:
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaMsgSetMediaQualityParam : public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgSetMediaQualityParam() :
            m_objMediaQualityThreshold(MediaQualityThreshold()){};

public:
    MediaQualityThreshold m_objMediaQualityThreshold;
};

class ImsMediaMsgConfigParam : public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgConfigParam() :
            m_objAudioConfig(AudioConfig()){};

public:
    AudioConfig m_objAudioConfig;
};

class ImsMediaMsgOpenConfigParam : public ImsMediaMsgConfigParam
{
public:
    ImsMediaMsgOpenConfigParam() :
            m_objLocalAddress(IPAddress::IPv6NONE),
            m_nLocalPort(0){};

public:
    IPAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
};

class ImsMediaMsgVideoConfigParam : public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgVideoConfigParam() :
            m_objVideoConfig(VideoConfig()){};

public:
    VideoConfig m_objVideoConfig;
};

class ImsMediaMsgVideoOpenConfigParam : public ImsMediaMsgVideoConfigParam
{
public:
    ImsMediaMsgVideoOpenConfigParam() :
            m_objLocalAddress(IPAddress::IPv6NONE),
            m_nLocalPort(0){};

public:
    IPAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
};

class ImsMediaMsgDtmfParam : public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgDtmfParam() :
            m_dtmfCode(-1),
            m_nDuration(-1){};

public:
    IMS_CHAR m_dtmfCode;
    IMS_SINT32 m_nDuration;
};

class ImsMediaResponseParamBase
{
public:
    ImsMediaResponseParamBase() :
            m_eResult(RtpError::NO_ERROR),
            m_eMediaType(MEDIA_TYPE_INVALID){};

public:
    RtpError m_eResult;
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaResponseConfigParam : public ImsMediaResponseParamBase
{
public:
    ImsMediaResponseConfigParam() :
            m_objAudioConfig(AudioConfig()){};

public:
    AudioConfig m_objAudioConfig;
};

class ImsMediaNotifyQosParam : public ImsMediaResponseParamBase
{
public:
    ImsMediaNotifyQosParam() :
            m_objIpAddr(IPAddress::IPv6NONE),
            m_nPort(0),
            m_bResult(IMS_FALSE){};

public:
    IPAddress m_objIpAddr;
    IMS_SINT32 m_nPort;
    IMS_BOOL m_bResult;
};

class ImsMediaNotifyInactivityParam
{
public:
    ImsMediaNotifyInactivityParam() :
            m_eMediaProtocolType(MEDIA_PROTOCOL_NONE),
            m_eMediaType(MEDIA_TYPE_INVALID){};

public:
    MEDIA_TRANSPORT_PROTOCOL m_eMediaProtocolType;
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

/*
class ImsMediaNotifyQualityParam :
        public ImsMediaResponseParamBase
{
public:
    ImsMediaNotifyQualityParam() :
            m_objCallQuality()
    {};
public:
    CallQuality m_objCallQuality;
};
*/

/*
class ImsMediaSessionChangedParam :
        public ImsMediaResponseParamBase
{
public:
    ImsMediaSessionChangedParam() :
            m_objRtpSession(RtpSession())
    {};
public:
    RtpSession m_objRtpSession;
};
*/ // NEXT_ITEM :: OnSessionChanged

/*
class ImsMediaHeaderExtensionParam :
        public ImsMediaResponseParamBase
{
public:
    ImsMediaHeaderExtensionParam() :
            m_objRtpHeaderExtension(RtpHeaderExtension())
    {};
public:
    RtpHeaderExtension m_objRtpHeaderExtension;
};
*/ // NEXT_ITEM :: OnSessionChanged
class ImsMediaBasicSessionInfoParam
{
public:
    ImsMediaBasicSessionInfoParam() :
            m_nNegoId(0),
            m_eMediaType(MEDIA_TYPE_AUDIO){};

public:
    IMS_UINTP m_nNegoId;
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaVideoParam : public ImsMediaMsgParamBase
{
public:
    ImsMediaVideoParam(IMS_SINT32 value = -1) :
            ImsMediaMsgParamBase(MEDIA_TYPE_VIDEO),
            nValue(value){};

public:
    IMS_SINT32 nValue;
};

class ImsMediaVideoResolutionParam : public ImsMediaMsgParamBase
{
public:
    ImsMediaVideoResolutionParam(IMS_SINT32 value1 = -1, IMS_SINT32 value2 = -1) :
            ImsMediaMsgParamBase(MEDIA_TYPE_VIDEO),
            nWidth(value1),
            nHeight(value2){};

public:
    IMS_SINT32 nWidth;
    IMS_SINT32 nHeight;
};

#endif
