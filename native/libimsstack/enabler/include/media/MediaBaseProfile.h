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

#include "ImsMap.h"
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
        RtpMap(IN const IMS_SINT32 channel = 0) :
                nPayloadNumber(0),
                strPayloadType(AString::ConstNull()),
                nSamplingRate(0),
                nChannel(channel)
        {
        }

        RtpMap(IN const RtpMap& obj) :
                nPayloadNumber(obj.nPayloadNumber),
                strPayloadType(obj.strPayloadType),
                nSamplingRate(obj.nSamplingRate),
                nChannel(obj.nChannel)
        {
        }

        RtpMap& operator=(IN const RtpMap& obj)
        {
            if (this != &obj)
            {
                nPayloadNumber = obj.nPayloadNumber;
                strPayloadType = obj.strPayloadType;
                nSamplingRate = obj.nSamplingRate;
                nChannel = obj.nChannel;
            }
            return (*this);
        }

        bool operator==(IN const RtpMap& obj) const
        {
            return (nPayloadNumber == obj.nPayloadNumber &&
                    strPayloadType.EqualsIgnoreCase(obj.strPayloadType) &&
                    nSamplingRate == obj.nSamplingRate && nChannel == obj.nChannel);
        }

        bool operator!=(IN const RtpMap& obj) const
        {
            return (nPayloadNumber != obj.nPayloadNumber ||
                    !strPayloadType.EqualsIgnoreCase(obj.strPayloadType) ||
                    nSamplingRate != obj.nSamplingRate || nChannel != obj.nChannel);
        }

        void SetPayloadNumber(IN const IMS_UINT32 payloadNumber) { nPayloadNumber = payloadNumber; }
        IMS_UINT32 GetPayloadNumber() { return nPayloadNumber; }
        void SetPayloadType(IN const AString& payloadType) { strPayloadType = payloadType; }
        AString& GetPayloadType() { return strPayloadType; }
        void SetSamplingRate(IN const IMS_UINT32 samplingRate) { nSamplingRate = samplingRate; }
        IMS_UINT32 GetSamplingRate() { return nSamplingRate; }
        void SetChannel(IN const IMS_SINT32 channel) { nChannel = channel; }
        IMS_SINT32 GetChannel() { return nChannel; }

    protected:
        IMS_UINT32 nPayloadNumber;  // Payload number
        AString strPayloadType;     // Payload type name
        IMS_UINT32 nSamplingRate;   // Sampling rate
        IMS_SINT32 nChannel;        // Number of channels
    };

    /**
     * This class is base class of Audio/Video/Text Payload.
     * Payload is actual data transported by RTP in a packet.
     */
    class BasePayload
    {
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

        void SetRtpMap(IN const RtpMap& objMap) { objRtpMap = objMap; };

        void SetRtpMap(IN const IMS_UINT32& payloadNum, IN const AString& payloadType,
                IN const IMS_UINT32 samplingRate, IN const IMS_SINT32 nChannel = 0)
        {
            objRtpMap.SetPayloadNumber(payloadNum);
            objRtpMap.SetPayloadType(payloadType);
            objRtpMap.SetSamplingRate(samplingRate);
            objRtpMap.SetChannel(nChannel);
        };

        RtpMap& GetRtpMap() { return objRtpMap; }

        void SetFmtp(IN BaseFmtp* fmtp) { pFmtp = fmtp; }

        BaseFmtp* GetFmtp() { return pFmtp; }

    protected:
        void deleteFmtp()
        {
            if (pFmtp != IMS_NULL)
            {
                delete pFmtp;
                pFmtp = IMS_NULL;
            }
        }

    protected:
        RtpMap objRtpMap;
        BaseFmtp* pFmtp;
    };

public:
    /**
     * CapaNego for SDP Capability Negotiation Model (RFC 5939)
     */
    class CapaNego
    {
    public:
        CapaNego() :
                /* Transport Capability */
                mapTcap(ImsMap<IMS_SINT32, AString>()),
                /* Attribute Capability */
                mapAcap(ImsMap<IMS_SINT32, AString>()),
                /* Potential Configuration (pcfg) proposed */
                listPcfg(ImsList<AString>()),
                /* Actual configuration (acfg) negotiated */
                strAcfg(AString::ConstNull()),
                /* bool to check if acap is existed in pcfg */
                bAcapInPcfg(IMS_FALSE){};

        ImsMap<IMS_SINT32, AString>& GetMapTcap() { return mapTcap; }
        ImsMap<IMS_SINT32, AString>& GetMapAcap() { return mapAcap; }
        void SetListPcfg(ImsList<AString> pcfg) { listPcfg = pcfg; }
        ImsList<AString>& GetListPcfg() { return listPcfg; }
        void SetAcfg(IN const AString acfg) { strAcfg = acfg; }
        AString& GetAcfg() { return strAcfg; }
        void SetAttCapaInPcfg(IMS_BOOL acapInPcfg) { bAcapInPcfg = acapInPcfg; }
        IMS_BOOL IsAttCapaInPcfg() { return bAcapInPcfg; }

    private:
        ImsMap<IMS_SINT32, AString> mapTcap;
        ImsMap<IMS_SINT32, AString> mapAcap;
        ImsList<AString> listPcfg;
        AString strAcfg;
        IMS_BOOL bAcapInPcfg;
    };

public:
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
            eDirection(direction),
            objCapaNego(CapaNego()),
            nNegotiatedPayloadIndex(-1),
            listPayload(ImsList<BasePayload*>())
    {
    }

    virtual ~MediaBaseProfile() { DeletePayloads(); }

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
        objCapaNego = profile->objCapaNego;
        nNegotiatedPayloadIndex = profile->nNegotiatedPayloadIndex;

        DeletePayloads();
        CopyPayloads(profile->listPayload);
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
        objCapaNego = obj.objCapaNego;
        nNegotiatedPayloadIndex = obj.nNegotiatedPayloadIndex;

        DeletePayloads();
        CopyPayloads(obj.listPayload);
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
            objCapaNego = obj.objCapaNego;
            nNegotiatedPayloadIndex = obj.nNegotiatedPayloadIndex;

            DeletePayloads();
            CopyPayloads(obj.listPayload);
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

    virtual BasePayload* GetPayloadAt(IN IMS_UINT32 nIndex)
    {
        return (listPayload.GetSize() > nIndex) ? listPayload.GetAt(nIndex) : IMS_NULL;
    }

    void DeletePayloads();
    void CopyPayloads(IN ImsList<BasePayload*> payloadList);

    void SetIpAddress(IN const IpAddress ipAddress) { objIpAddress = ipAddress; }
    IpAddress& GetIpAddress() { return objIpAddress; }
    void SetDataPort(IN const IMS_UINT32 port) { nDataPort = port; }
    IMS_UINT32 GetDataPort() { return nDataPort; }
    void SetControlPort(IN const IMS_UINT32 port) { nControlPort = port; }
    IMS_UINT32 GetControlPort() { return nControlPort; }
    void SetTransportType(IN const AString transportType) { strTransportType = transportType; }
    AString& GetTransportType() { return strTransportType; }
    void SetRtcpInterval(IN const IMS_UINT32 interval) { nRtcpInterval = interval; }
    IMS_UINT32 GetRtcpInterval() { return nRtcpInterval; }
    void SetBandwidthAs(IN const IMS_SINT32 as) { nBandwidthAs = as; }
    IMS_SINT32 GetBandwidthAs() { return nBandwidthAs; }
    void SetBandwidthRs(IN const IMS_SINT32 rs) { nBandwidthRs = rs; }
    IMS_SINT32 GetBandwidthRs() { return nBandwidthRs; }
    void SetBandwidthRr(IN const IMS_SINT32 rr) { nBandwidthRr = rr; }
    IMS_SINT32 GetBandwidthRr() { return nBandwidthRr; }
    void SetDirection(IN const MEDIA_DIRECTION direction) { eDirection = direction; }
    MEDIA_DIRECTION GetDirection() { return eDirection; }
    CapaNego& GetCapaNego() { return objCapaNego; }
    void SetNegotiatedPayloadIndex(IN const IMS_SINT32 index) { nNegotiatedPayloadIndex = index; }
    IMS_SINT32 GetNegotiatedPayloadIndex() { return nNegotiatedPayloadIndex; }
    ImsList<BasePayload*>& GetPayloadList() { return listPayload; }

private:
    IpAddress objIpAddress;
    IMS_UINT32 nDataPort;
    IMS_UINT32 nControlPort;
    AString strTransportType;
    IMS_UINT32 nRtcpInterval;
    IMS_SINT32 nBandwidthAs;
    IMS_SINT32 nBandwidthRs;
    IMS_SINT32 nBandwidthRr;
    MEDIA_DIRECTION eDirection;
    CapaNego objCapaNego;
    IMS_SINT32 nNegotiatedPayloadIndex;
    ImsList<BasePayload*> listPayload;
};

#endif
