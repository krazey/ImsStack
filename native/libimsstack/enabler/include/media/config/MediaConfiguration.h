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

#ifndef _MEDIA_CONFIGURATION1_H_
#define _MEDIA_CONFIGURATION1_H_

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
    MediaConfiguration(MEDIA_CONTENT_TYPE _eSessionType = MEDIA_TYPE_AUDIO);
    virtual ~MediaConfiguration();

public:
    virtual IMS_BOOL Create(IN ICarrierConfig* piCc);
    virtual IMS_BOOL Update(IN ICarrierConfig* piCc);
    virtual CodecConfig* GetCodecConfig(IN IMS_UINT32 nCodec) const;
    virtual const IMSList<CodecConfig*>& GetCodecConfigs() const;

protected:
    virtual IMS_BOOL CreateCodecConfigs(IN ICarrierConfig* piCc);
    virtual IMS_UINT32 MakeEachCodecs(IN ICarrierConfig* piCc, IN IMS_UINT32 nCodec,
            IN IMS_UINT32 nCodecIndex, IN IMSVector<IMS_SINT32> objPayloadTypeArray);
    virtual IMS_UINT32 MakeCodec(IN ICarrierConfig* piCc, IN IMS_UINT32 nCodec,
            IN IMS_UINT32 nCodecIndex, IN IMS_SINT32 nPayloadTypeNum);
    virtual void ToDebugString() const;
    virtual void ToDebugStringCodecs(IN CodecConfig* pCodecConfig) const;
    virtual void Clear();
    IMS_UINT32 GetCodecType(IN IMS_UINT32 nCodec) const;
    void SetPorts(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey);
    void SetRtcpIntervals(IN ICarrierConfig* piCc, IN const IMS_CHAR* pszKey);

public:
    MEDIA_CONTENT_TYPE GetSessionType() const;

    IMS_SINT32 GetPortRtp() const;
    IMS_SINT32 GetPortRtpEnd() const;
    IMS_SINT32 GetPortRtcp() const;
    IMS_SINT32 GetRtcpLiveInterval() const;
    IMS_SINT32 GetRtcpInterval() const;
    IMS_SINT32 GetAsBandwidthKbps() const;
    IMS_SINT32 GetRsBandwidthBps() const;
    IMS_SINT32 GetRrBandwidthBps() const;
    IMS_SINT32 GetRtpInactivityTimerMillis() const;
    IMS_SINT32 GetRtcpInactivityTimerMillis() const;

public:
    // Bandwidth mode
    enum
    {
        BW_MODE_HIDE        = 0,
        BW_MODE_OPTIMAL     = 1,
        BW_MODE_MAX         = 2,
        BW_MODE_MANUAL      = 3,
        BW_MODE_NEGOTIABLE  = 4,
    };

    // Bandwidth RS/RR mode
    enum
    {
        BW_RS_RR_HIDE       = 0,
        BW_RS_RR_PERCENT    = 1,
        BW_RS_RR_MANUAL     = 2
    };

    // Bandwidth type
    enum
    {
        BW_AS = 1,
        BW_RR = 2,
        BW_RS = 3
    };

    /*
        Bandwidth_option_value
        BW_OPTION_SOURCE_VALUE : Use Source Profile's AS/RS/RR Value
        BW_OPTION_NEGOTIATED_VALUE : Use Negotiated AS/RS/RR Value
                                    (compare source profile and dest profile)
    */
    enum
    {
        BW_OPTION_SOURCE_VALUE = 0,
        BW_OPTION_NEGOTIATED_VALUE = 1
    };

    enum
    {
        SOCKET_POS_DEFAULT = 0,
        SOCKET_POS_CP = 1,
        SOCKET_POS_AP = 2
    };

    // check SDPOfferCapNegForAVPF option
    enum
    {
        CAPNEG_OFFER_NONE = 0,
        CAPNEG_OFFER_WITHOUT_ACAP = 1,
        CAPNEG_OFFER_WITH_ACAP = 2
    };
    // Video Resolution Loose Check mode
    enum
    {
        USE_SELF_RESOLUTION_STRICTLY    = 0,
        USE_PEER_RESOLUTION_RX_ONLY     = 1,
        USE_PEER_RESOLUTION_TRX         = 2,
    };


    static const IMS_SINT32 DEFAULT_RTP_PORT = 50010;
    static const IMS_SINT32 DEFAULT_RTP_PORT_END = 50060;
    static const IMS_SINT32 DEFAULT_RTCP_PORT = 50011;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL_LIVE = 5;
    static const IMS_SINT32 DEFAULT_RTCP_INVERVAL = 5;
    static const IMS_SINT32 DEFAULT_AS = 41;
    static const IMS_SINT32 DEFAULT_RS = 600;
    static const IMS_SINT32 DEFAULT_RR = 2000;
    static const IMS_SINT32 DEFAULT_RTP_INACTIVITY = 20000;
    static const IMS_SINT32 DEFAULT_RTCP_INACTIVITY = 20000;

protected:
    MEDIA_CONTENT_TYPE eSessionType;

    IMS_SINT32 nPortRtp;
    IMS_SINT32 nPortRtpEnd;
    IMS_SINT32 nPortRtcp;
    IMS_SINT32 nRtcpLiveInterval;
    IMS_SINT32 nRtcpInterval;
    IMS_SINT32 nAsBandwidthKbps;
    IMS_SINT32 nRsBandwidthBps;
    IMS_SINT32 nRrBandwidthBps;
    IMS_SINT32 nRtpInactivityTimerMillis;
    IMS_SINT32 nRtcpInactivityTimerMillis;

    // Provisioned codecs
    IMSList<CodecConfig*> objCodecConfigs;
};
#endif                                              // _MEDIA_CONFIGURATION_H_
