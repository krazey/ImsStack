/**
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef MEDIA_BASE_PROFILE_H_
#define MEDIA_BASE_PROFILE_H_

#include "ImsTypeDef.h"
#include "IpAddress.h"
#include "MediaDef.h"

/**
 * The class is a base class of the Media (Audio/Video/Text) Profile.
 * Media Profile is used to keep the SDP negotiation information like
 * SDP offer, answer and the negotiated media information.
 */
class MediaBaseProfile
{
public:
    /**
     * Stands for "format parameter"
     * This class is a base class of each media fmtp.
     * Fmtp attributes are used within the SDP to carry parameters that provide
     * extra configuration details about a specific media codec used in the RTP stream.
     */
    class BaseFmtp
    {
    protected:
        BaseFmtp() {}

    public:
        virtual ~BaseFmtp() {}
    };

    /**
     * This class maps from a RTP payload type number (as used in an "m=" line)
     * to an encoding name denoting the payload format to be used.
     * It also provides information on the clock rate and encoding parameters.
     */
    class RtpMap
    {
    public:
        IMS_UINT32 nPayloadNum;    // Payload number
        AString strPayloadType;    // Payload type name
        IMS_UINT32 nSamplingRate;  // Sampling rate
        IMS_SINT32 nChannel;       // Number of channels

    public:
        RtpMap(IN const IMS_SINT32 channel = 0) :
                nPayloadNum(0),
                strPayloadType(AString::ConstNull()),
                nSamplingRate(0),
                nChannel(channel)
        {
        }

        RtpMap(IN const RtpMap& obj) :
                nPayloadNum(obj.nPayloadNum),
                strPayloadType(obj.strPayloadType),
                nSamplingRate(obj.nSamplingRate),
                nChannel(obj.nChannel)
        {
        }

        RtpMap& operator=(IN const RtpMap& obj)
        {
            if (this != &obj)
            {
                nPayloadNum = obj.nPayloadNum;
                strPayloadType = obj.strPayloadType;
                nSamplingRate = obj.nSamplingRate;
                nChannel = obj.nChannel;
            }
            return (*this);
        }

        bool operator==(IN const RtpMap& obj) const
        {
            return (nPayloadNum == obj.nPayloadNum && strPayloadType == obj.strPayloadType &&
                    nSamplingRate == obj.nSamplingRate && nChannel == obj.nChannel);
        }
    };

    /**
     * This class is base class of Audio/Video/Text Payload.
     * Payload is actual data transported by RTP in a packet.
     */
    class BasePayload
    {
    public:
        RtpMap objRtpMap;
        BaseFmtp* pFmtp;

    public:
        BasePayload(IN const IMS_SINT32 channel = 0) :
                objRtpMap(channel),
                pFmtp(IMS_NULL)
        {
        }

        BasePayload(IN const BasePayload& obj) :
                objRtpMap(obj.objRtpMap),
                pFmtp(IMS_NULL)
        {
        }

        virtual ~BasePayload() { deleteFmtp(); }

        BasePayload& operator=(IN const BasePayload& obj)
        {
            if (this != &obj)
            {
                objRtpMap = obj.objRtpMap;
                deleteFmtp();
            }

            return (*this);
        }

        void SetRtpMap(IN const RtpMap& objMap)
        {
            objRtpMap.nPayloadNum = objMap.nPayloadNum;
            objRtpMap.strPayloadType = objMap.strPayloadType;
            objRtpMap.nSamplingRate = objMap.nSamplingRate;
            objRtpMap.nChannel = objMap.nChannel;
        };

        void SetRtpMap(IN const IMS_UINT32& payloadNum, IN const AString& payloadType,
                IN const IMS_UINT32 samplingRate, IN const IMS_SINT32 nChannel = 0)
        {
            objRtpMap.nPayloadNum = payloadNum;
            objRtpMap.strPayloadType = payloadType;
            objRtpMap.nSamplingRate = samplingRate;
            objRtpMap.nChannel = nChannel;
        };

    protected:
        void deleteFmtp()
        {
            if (pFmtp != IMS_NULL)
            {
                delete pFmtp;
                pFmtp = IMS_NULL;
            }
        }
    };

public:
    IpAddress objIpAddress;
    IMS_UINT32 nDataPort;
    IMS_UINT32 nControlPort;
    AString strTransportType;
    IMS_UINT32 nRtcpInterval;
    IMS_SINT32 nBandwidthAs;
    IMS_SINT32 nBandwidthRs;
    IMS_SINT32 nBandwidthRr;
    MEDIA_DIRECTION eDirection;

    MediaBaseProfile(IN const IpAddress ipAddress = IpAddress::IPv6NONE,
            IN const IMS_UINT32 dataPort = 0, IN const IMS_UINT32 controlPort = 0,
            IN const AString transportType = "RTP/AVP", IN const IMS_UINT32 rtcpInterval = 0,
            IN const IMS_SINT32 bandwidthAs = 0, IN const IMS_SINT32 bandwidthRs = 0,
            IN const IMS_SINT32 bandwidthRr = 0,
            IN const MEDIA_DIRECTION direction = MEDIA_DIRECTION_INVALID) :
            objIpAddress(ipAddress),
            nDataPort(dataPort),
            nControlPort(controlPort),
            strTransportType(transportType),
            nRtcpInterval(rtcpInterval),
            nBandwidthAs(bandwidthAs),
            nBandwidthRs(bandwidthRs),
            nBandwidthRr(bandwidthRr),
            eDirection(direction)
    {
    }

    virtual ~MediaBaseProfile() {}

    MediaBaseProfile(MediaBaseProfile* profile)
    {
        if (profile == nullptr)
        {
            return;
        }
        objIpAddress = profile->objIpAddress;
        nDataPort = profile->nDataPort;
        nControlPort = profile->nControlPort;
        strTransportType = profile->strTransportType;
        nRtcpInterval = profile->nRtcpInterval;
        nBandwidthAs = profile->nBandwidthAs;
        nBandwidthRs = profile->nBandwidthRs;
        nBandwidthRr = profile->nBandwidthRr;
        eDirection = profile->eDirection;
    }

    MediaBaseProfile(const MediaBaseProfile& obj)
    {
        objIpAddress = obj.objIpAddress;
        nDataPort = obj.nDataPort;
        nControlPort = obj.nControlPort;
        strTransportType = obj.strTransportType;
        nRtcpInterval = obj.nRtcpInterval;
        nBandwidthAs = obj.nBandwidthAs;
        nBandwidthRs = obj.nBandwidthRs;
        nBandwidthRr = obj.nBandwidthRr;
        eDirection = obj.eDirection;
    }

    MediaBaseProfile& operator=(IN const MediaBaseProfile& obj)
    {
        if (this != &obj)
        {
            objIpAddress = obj.objIpAddress;
            nDataPort = obj.nDataPort;
            nControlPort = obj.nControlPort;
            strTransportType = obj.strTransportType;
            nRtcpInterval = obj.nRtcpInterval;
            nBandwidthAs = obj.nBandwidthAs;
            nBandwidthRs = obj.nBandwidthRs;
            nBandwidthRr = obj.nBandwidthRr;
            eDirection = obj.eDirection;
        }
        return (*this);
    }

    bool operator==(IN const MediaBaseProfile& obj) const
    {
        return (objIpAddress == obj.objIpAddress && nDataPort == obj.nDataPort &&
                nControlPort == obj.nControlPort && strTransportType == obj.strTransportType &&
                nRtcpInterval == obj.nRtcpInterval && nBandwidthAs == obj.nBandwidthAs &&
                nBandwidthRs == obj.nBandwidthRs && nBandwidthRr == obj.nBandwidthRr &&
                eDirection == obj.eDirection);
    }

    bool operator!=(IN const MediaBaseProfile& obj) const
    {
        return (objIpAddress != obj.objIpAddress || nDataPort != obj.nDataPort ||
                nControlPort != obj.nControlPort || strTransportType != obj.strTransportType ||
                nRtcpInterval != obj.nRtcpInterval || nBandwidthAs != obj.nBandwidthAs ||
                nBandwidthRs != obj.nBandwidthRs || nBandwidthRr != obj.nBandwidthRr ||
                eDirection != obj.eDirection);
    }
};

#endif
