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
                m_nPayloadNumber(0),
                m_strPayloadType(AString::ConstNull()),
                m_nSamplingRate(0),
                m_nChannel(channel)
        {
        }

        RtpMap(IN const RtpMap& obj) :
                m_nPayloadNumber(obj.m_nPayloadNumber),
                m_strPayloadType(obj.m_strPayloadType),
                m_nSamplingRate(obj.m_nSamplingRate),
                m_nChannel(obj.m_nChannel)
        {
        }

        RtpMap& operator=(IN const RtpMap& obj)
        {
            if (this != &obj)
            {
                m_nPayloadNumber = obj.m_nPayloadNumber;
                m_strPayloadType = obj.m_strPayloadType;
                m_nSamplingRate = obj.m_nSamplingRate;
                m_nChannel = obj.m_nChannel;
            }
            return (*this);
        }

        bool operator==(IN const RtpMap& obj) const
        {
            return (m_nPayloadNumber == obj.m_nPayloadNumber &&
                    m_strPayloadType.EqualsIgnoreCase(obj.m_strPayloadType) &&
                    m_nSamplingRate == obj.m_nSamplingRate && m_nChannel == obj.m_nChannel);
        }

        bool operator!=(IN const RtpMap& obj) const
        {
            return (m_nPayloadNumber != obj.m_nPayloadNumber ||
                    !m_strPayloadType.EqualsIgnoreCase(obj.m_strPayloadType) ||
                    m_nSamplingRate != obj.m_nSamplingRate || m_nChannel != obj.m_nChannel);
        }

        inline void SetPayloadNumber(IN const IMS_UINT32 nPayloadNumber)
        {
            m_nPayloadNumber = nPayloadNumber;
        }
        inline IMS_UINT32 GetPayloadNumber() { return m_nPayloadNumber; }
        inline void SetPayloadType(IN const AString& strPayloadType)
        {
            m_strPayloadType = strPayloadType;
        }
        inline AString& GetPayloadType() { return m_strPayloadType; }
        inline void SetSamplingRate(IN const IMS_UINT32 nSamplingRate)
        {
            m_nSamplingRate = nSamplingRate;
        }
        inline IMS_UINT32 GetSamplingRate() { return m_nSamplingRate; }
        inline void SetChannel(IN const IMS_SINT32 nChannel) { m_nChannel = nChannel; }
        inline IMS_SINT32 GetChannel() { return m_nChannel; }

    protected:
        IMS_UINT32 m_nPayloadNumber;  // Payload number
        AString m_strPayloadType;     // Payload type name
        IMS_UINT32 m_nSamplingRate;   // Sampling rate
        IMS_SINT32 m_nChannel;        // Number of channels
    };

    /**
     * This class is base class of Audio/Video/Text Payload.
     * Payload is actual data transported by RTP in a packet.
     */
    class BasePayload
    {
    public:
        BasePayload(IN const IMS_SINT32 channel = 0) :
                m_objRtpMap(channel),
                m_pFmtp(IMS_NULL)
        {
        }

        BasePayload(IN const BasePayload& obj) :
                m_objRtpMap(obj.m_objRtpMap),
                m_pFmtp(IMS_NULL)
        {
        }

        virtual ~BasePayload() { deleteFmtp(); }

        BasePayload& operator=(IN const BasePayload& obj)
        {
            if (this != &obj)
            {
                m_objRtpMap = obj.m_objRtpMap;
                deleteFmtp();
            }

            return (*this);
        }

        void SetRtpMap(IN const IMS_UINT32& payloadNum, IN const AString& payloadType,
                IN const IMS_UINT32 samplingRate, IN const IMS_SINT32 m_nChannel = 0)
        {
            m_objRtpMap.SetPayloadNumber(payloadNum);
            m_objRtpMap.SetPayloadType(payloadType);
            m_objRtpMap.SetSamplingRate(samplingRate);
            m_objRtpMap.SetChannel(m_nChannel);
        };

        inline void SetRtpMap(IN const RtpMap& objRtpMap) { m_objRtpMap = objRtpMap; };
        inline RtpMap& GetRtpMap() { return m_objRtpMap; }
        inline void SetFmtp(IN BaseFmtp* pFmtp) { m_pFmtp = pFmtp; }
        inline BaseFmtp* GetFmtp() { return m_pFmtp; }

    protected:
        void deleteFmtp()
        {
            if (m_pFmtp != IMS_NULL)
            {
                delete m_pFmtp;
                m_pFmtp = IMS_NULL;
            }
        }

    protected:
        RtpMap m_objRtpMap;
        BaseFmtp* m_pFmtp;
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
                m_mapTcap(ImsMap<IMS_SINT32, AString>()),
                /* Attribute Capability */
                m_mapAcap(ImsMap<IMS_SINT32, AString>()),
                /* Potential Configuration (pcfg) proposed */
                m_lstPcfg(ImsList<AString>()),
                /* Actual configuration (acfg) negotiated */
                m_strAcfg(AString::ConstNull()),
                /* bool to check if acap is existed in pcfg */
                m_bAcapInPcfg(IMS_FALSE) {};

        inline ImsMap<IMS_SINT32, AString>& GetMapTcap() { return m_mapTcap; }
        inline ImsMap<IMS_SINT32, AString>& GetMapAcap() { return m_mapAcap; }
        inline void SetListPcfg(ImsList<AString> lstPcfg) { m_lstPcfg = lstPcfg; }
        inline ImsList<AString>& GetListPcfg() { return m_lstPcfg; }
        inline void SetAcfg(IN const AString strAcfg) { m_strAcfg = strAcfg; }
        inline AString& GetAcfg() { return m_strAcfg; }
        inline void SetAttCapaInPcfg(IMS_BOOL bAcapInPcfg) { m_bAcapInPcfg = bAcapInPcfg; }
        inline IMS_BOOL IsAttCapaInPcfg() { return m_bAcapInPcfg; }

    private:
        ImsMap<IMS_SINT32, AString> m_mapTcap;
        ImsMap<IMS_SINT32, AString> m_mapAcap;
        ImsList<AString> m_lstPcfg;
        AString m_strAcfg;
        IMS_BOOL m_bAcapInPcfg;
    };

public:
    MediaBaseProfile(IN const IpAddress ipAddress = IpAddress::IPv6NONE,
            IN const IMS_UINT32 dataPort = 0, IN const IMS_UINT32 controlPort = 0,
            IN const AString transportType = "RTP/AVP", IN const IMS_UINT32 rtcpInterval = 0,
            IN const IMS_SINT32 bandwidthAs = 0, IN const IMS_SINT32 bandwidthRs = 0,
            IN const IMS_SINT32 bandwidthRr = 0,
            IN const MEDIA_DIRECTION direction = MEDIA_DIRECTION_INVALID) :
            m_objIpAddress(ipAddress),
            m_nDataPort(dataPort),
            m_nControlPort(controlPort),
            m_strTransportType(transportType),
            m_nRtcpInterval(rtcpInterval),
            m_nBandwidthAs(bandwidthAs),
            m_nBandwidthRs(bandwidthRs),
            m_nBandwidthRr(bandwidthRr),
            m_eDirection(direction),
            m_objCapaNego(CapaNego()),
            m_nNegotiatedPayloadIndex(-1),
            m_lstPayload(ImsList<BasePayload*>())
    {
    }

    virtual ~MediaBaseProfile() { DeletePayloads(); }

    MediaBaseProfile(MediaBaseProfile* profile)
    {
        if (profile == nullptr)
        {
            return;
        }
        m_objIpAddress = profile->m_objIpAddress;
        m_nDataPort = profile->m_nDataPort;
        m_nControlPort = profile->m_nControlPort;
        m_strTransportType = profile->m_strTransportType;
        m_nRtcpInterval = profile->m_nRtcpInterval;
        m_nBandwidthAs = profile->m_nBandwidthAs;
        m_nBandwidthRs = profile->m_nBandwidthRs;
        m_nBandwidthRr = profile->m_nBandwidthRr;
        m_eDirection = profile->m_eDirection;
        m_objCapaNego = profile->m_objCapaNego;
        m_nNegotiatedPayloadIndex = profile->m_nNegotiatedPayloadIndex;

        DeletePayloads();
        CopyPayloads(profile->m_lstPayload);
    }

    MediaBaseProfile(const MediaBaseProfile& obj)
    {
        m_objIpAddress = obj.m_objIpAddress;
        m_nDataPort = obj.m_nDataPort;
        m_nControlPort = obj.m_nControlPort;
        m_strTransportType = obj.m_strTransportType;
        m_nRtcpInterval = obj.m_nRtcpInterval;
        m_nBandwidthAs = obj.m_nBandwidthAs;
        m_nBandwidthRs = obj.m_nBandwidthRs;
        m_nBandwidthRr = obj.m_nBandwidthRr;
        m_eDirection = obj.m_eDirection;
        m_objCapaNego = obj.m_objCapaNego;
        m_nNegotiatedPayloadIndex = obj.m_nNegotiatedPayloadIndex;

        DeletePayloads();
        CopyPayloads(obj.m_lstPayload);
    }

    MediaBaseProfile& operator=(IN const MediaBaseProfile& obj)
    {
        if (this != &obj)
        {
            m_objIpAddress = obj.m_objIpAddress;
            m_nDataPort = obj.m_nDataPort;
            m_nControlPort = obj.m_nControlPort;
            m_strTransportType = obj.m_strTransportType;
            m_nRtcpInterval = obj.m_nRtcpInterval;
            m_nBandwidthAs = obj.m_nBandwidthAs;
            m_nBandwidthRs = obj.m_nBandwidthRs;
            m_nBandwidthRr = obj.m_nBandwidthRr;
            m_eDirection = obj.m_eDirection;
            m_objCapaNego = obj.m_objCapaNego;
            m_nNegotiatedPayloadIndex = obj.m_nNegotiatedPayloadIndex;

            DeletePayloads();
            CopyPayloads(obj.m_lstPayload);
        }
        return (*this);
    }

    bool operator==(IN const MediaBaseProfile& obj) const
    {
        return (m_objIpAddress == obj.m_objIpAddress && m_nDataPort == obj.m_nDataPort &&
                m_nControlPort == obj.m_nControlPort &&
                m_strTransportType == obj.m_strTransportType &&
                m_nRtcpInterval == obj.m_nRtcpInterval && m_nBandwidthAs == obj.m_nBandwidthAs &&
                m_nBandwidthRs == obj.m_nBandwidthRs && m_nBandwidthRr == obj.m_nBandwidthRr &&
                m_eDirection == obj.m_eDirection);
    }

    bool operator!=(IN const MediaBaseProfile& obj) const
    {
        return (m_objIpAddress != obj.m_objIpAddress || m_nDataPort != obj.m_nDataPort ||
                m_nControlPort != obj.m_nControlPort ||
                m_strTransportType != obj.m_strTransportType ||
                m_nRtcpInterval != obj.m_nRtcpInterval || m_nBandwidthAs != obj.m_nBandwidthAs ||
                m_nBandwidthRs != obj.m_nBandwidthRs || m_nBandwidthRr != obj.m_nBandwidthRr ||
                m_eDirection != obj.m_eDirection);
    }

    virtual BasePayload* GetPayloadAt(IN IMS_UINT32 nIndex)
    {
        return (m_lstPayload.GetSize() > nIndex) ? m_lstPayload.GetAt(nIndex) : IMS_NULL;
    }

    void DeletePayloads();
    void CopyPayloads(IN ImsList<BasePayload*> payloadList);

    inline void SetIpAddress(IN const IpAddress objIpAddress) { m_objIpAddress = objIpAddress; }
    inline IpAddress& GetIpAddress() { return m_objIpAddress; }
    inline void SetDataPort(IN const IMS_UINT32 nDataPort) { m_nDataPort = nDataPort; }
    inline IMS_UINT32 GetDataPort() { return m_nDataPort; }
    inline void SetControlPort(IN const IMS_UINT32 nControlPort) { m_nControlPort = nControlPort; }
    inline IMS_UINT32 GetControlPort() { return m_nControlPort; }
    inline void SetTransportType(IN const AString strTransportType)
    {
        m_strTransportType = strTransportType;
    }
    inline AString& GetTransportType() { return m_strTransportType; }
    inline void SetRtcpInterval(IN const IMS_UINT32 nRtcpInterval)
    {
        m_nRtcpInterval = nRtcpInterval;
    }
    inline IMS_UINT32 GetRtcpInterval() { return m_nRtcpInterval; }
    inline void SetBandwidthAs(IN const IMS_SINT32 nBandwidthAs) { m_nBandwidthAs = nBandwidthAs; }
    inline IMS_SINT32 GetBandwidthAs() { return m_nBandwidthAs; }
    inline void SetBandwidthRs(IN const IMS_SINT32 nBandwidthRs) { m_nBandwidthRs = nBandwidthRs; }
    inline IMS_SINT32 GetBandwidthRs() { return m_nBandwidthRs; }
    inline void SetBandwidthRr(IN const IMS_SINT32 nBandwidthRr) { m_nBandwidthRr = nBandwidthRr; }
    inline IMS_SINT32 GetBandwidthRr() { return m_nBandwidthRr; }
    inline void SetDirection(IN const MEDIA_DIRECTION eDirection) { m_eDirection = eDirection; }
    inline MEDIA_DIRECTION GetDirection() { return m_eDirection; }
    inline CapaNego& GetCapaNego() { return m_objCapaNego; }
    inline void SetNegotiatedPayloadIndex(IN const IMS_SINT32 nNegotiatedPayloadIndex)
    {
        m_nNegotiatedPayloadIndex = nNegotiatedPayloadIndex;
    }
    inline IMS_SINT32 GetNegotiatedPayloadIndex() { return m_nNegotiatedPayloadIndex; }
    inline ImsList<BasePayload*>& GetPayloadList() { return m_lstPayload; }

private:
    IpAddress m_objIpAddress;
    IMS_UINT32 m_nDataPort;
    IMS_UINT32 m_nControlPort;
    AString m_strTransportType;
    IMS_UINT32 m_nRtcpInterval;
    IMS_SINT32 m_nBandwidthAs;
    IMS_SINT32 m_nBandwidthRs;
    IMS_SINT32 m_nBandwidthRr;
    MEDIA_DIRECTION m_eDirection;
    CapaNego m_objCapaNego;
    IMS_SINT32 m_nNegotiatedPayloadIndex;
    ImsList<BasePayload*> m_lstPayload;
};

#endif
