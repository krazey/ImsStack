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
#include <MediaQualityThreshold.h>
#include "ImsMessageDef.h"
#include "IMSTypeDef.h"
#include "IPAddress.h"
#include "MediaDef.h"

#define IMMEDIA IMS_MSG_BASE_STREAMEDMEDIA
#define IMMEDIA_IND IMMEDIA + 100

using namespace android::telephony::imsmedia;

class IMMedia
{

#define IMMEDIA_CASE_ENUM(name) case name: return #name

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
        }
        return "Unrecognized Msg";
    }

public:
    static const IMS_SINT32 MEDIA_MESSAGE_IDX_START = IMMEDIA + 0;

    static const IMS_SINT32 REQUEST_OPEN_SESSION = IMMEDIA + 1;
    static const IMS_SINT32 REQUEST_CLOSE_SESSION = IMMEDIA + 2;
    static const IMS_SINT32 REQUEST_MODIFY_SESSION = IMMEDIA + 3;
    static const IMS_SINT32 REQUEST_ADD_CONFIG = IMMEDIA + 4;
    static const IMS_SINT32 REQUEST_DELETE_CONFIG = IMMEDIA + 5;
    static const IMS_SINT32 REQUEST_CONFIRM_CONFIG = IMMEDIA + 6;
    static const IMS_SINT32 REQUEST_SEND_DTMF = IMMEDIA + 7;
    static const IMS_SINT32 REQUEST_SET_MEDIA_QUALITY = IMMEDIA + 8;
    static const IMS_SINT32 REQUEST_HEADER_EXTENSION = IMMEDIA + 9;

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

    static const IMS_SINT32 NOTIFY_QOS_INFO = IMMEDIA_IND + 20;

    static const IMS_SINT32 MEDIA_MESSAGE_IND_IDX_END = IMMEDIA_IND + 99;
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

    // Additional for Ims Internal
    /** The response is not received within certain time */
    RESPONSE_WAIT_TIMEOUT = 7,
};

enum SessionType
{
    SESSION_TYPE_AUDIO = 0,
    SESSION_TYPE_VIDEO = 1,
    SESSION_TYPE_RTT = 2,
};

class ImsMediaMsgParamBase
{
public:
    ImsMediaMsgParamBase() :
            m_eMediaType(MEDIA_TYPE_INVALID)
    {};

public:
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaMsgSetMediaQualityParam :
        public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgSetMediaQualityParam() :
            m_objMediaQualityThreshold(MediaQualityThreshold())
    {};

public:
    MediaQualityThreshold m_objMediaQualityThreshold;
};

class ImsMediaMsgConfigParam :
        public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgConfigParam() :
            m_objAudioConfig(AudioConfig())
    {};

public:
    AudioConfig m_objAudioConfig;
};

class ImsMediaMsgOpenConfigParam :
        public ImsMediaMsgConfigParam
{
public:
    ImsMediaMsgOpenConfigParam() :
            m_objLocalAddress(IPAddress::IPv6NONE),
            m_nLocalPort(0)
    {};

public:
    IPAddress m_objLocalAddress;
    IMS_SINT32 m_nLocalPort;
};

class ImsMediaMsgDtmfParam :
        public ImsMediaMsgParamBase
{
public:
    ImsMediaMsgDtmfParam() :
            m_dtmfCode(-1),
            m_nDuration(-1)
    {};

public:
    IMS_CHAR m_dtmfCode;
    IMS_SINT32 m_nDuration;
};

class ImsMediaResponseParamBase
{
public:
    ImsMediaResponseParamBase() :
            m_eResult(NO_ERROR),
            m_eMediaType(MEDIA_TYPE_INVALID)
    {};

public:
    RtpError m_eResult;
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaResponseConfigParam :
        public ImsMediaResponseParamBase
{
public:
    ImsMediaResponseConfigParam() :
            m_objAudioConfig(AudioConfig())
    {};

public:
    AudioConfig m_objAudioConfig;
};

class ImsMediaNotifyQosParam :
        public ImsMediaResponseParamBase
{
public:
    ImsMediaNotifyQosParam() :
            m_objIpAddr(IPAddress::IPv6NONE),
            m_nPort(0),
            m_bResult(IMS_FALSE)
    {};
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
            m_eMediaType(MEDIA_TYPE_INVALID)
    {};
public:
    MEDIA_TRANSPORT_PROTOCOL m_eMediaProtocolType;
    MEDIA_CONTENT_TYPE m_eMediaType;
};

class ImsMediaNotifyPacketParam :
        public ImsMediaResponseParamBase
{
public:
    ImsMediaNotifyPacketParam() :
            m_nResponse(-1)
    {};
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
            m_eMediaType(MEDIA_TYPE_AUDIO)
    {};
public:
    IMS_UINTP m_nNegoId;
    MEDIA_CONTENT_TYPE m_eMediaType;
};

#endif
