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

#ifndef _IMS_VIDEO_NEGO_PROFILE_H_
#define _IMS_VIDEO_NEGO_PROFILE_H_

#include "IMSTypeDef.h"
#include "IpAddress.h"
#include "ImsMap.h"
#include "MediaDef.h"
#include "VideoDef.h"

class VideoProfile
{
public:
    class RtpMap
    {
    public:
        IMS_UINT32 nPayloadNum;
        AString strPayloadType;
        IMS_UINT32 nSamplingRate;
        IMS_UINT32 nChannel;  // default is 0

    public:
        RtpMap() :
                nPayloadNum(0),
                strPayloadType(AString::ConstNull()),
                nSamplingRate(0),
                nChannel(0){};

        RtpMap(IN const RtpMap& obj)
        {
            this->nPayloadNum = obj.nPayloadNum;
            this->strPayloadType = obj.strPayloadType;
            this->nSamplingRate = obj.nSamplingRate;
            this->nChannel = obj.nChannel;
        }

        RtpMap& operator=(IN const RtpMap& obj)
        {
            this->nPayloadNum = obj.nPayloadNum;
            this->strPayloadType = obj.strPayloadType;
            this->nSamplingRate = obj.nSamplingRate;
            this->nChannel = obj.nChannel;
            return *this;
        }
    };

public:
    class HevcFmtp
    {
    public:
        VIDEO_RESOLUTION eResolution;
        IMS_SINT32 nBitrate;
        IMS_SINT32 nFrameRate;
        IMS_SINT32 nAs;
        VIDEO_PROFILE_HEVC nProfile;
        IMS_SINT32 nLevel;
        AString strVps;
        AString strSps;
        AString strPps;
        AString strSpropParam;
        IMS_SINT32 nPacketizationMode;
        IMS_BOOL bShow_Profile;
        IMS_BOOL bShow_Level;
        IMS_BOOL bShow_SpropParam;
        IMS_BOOL bShow_PacketizationMode;

    public:
        explicit HevcFmtp(IN const HevcFmtp* pFmtp = IMS_NULL) :
                eResolution(VIDEO_RESOLUTION_INVALID),
                nBitrate(0),
                nFrameRate(30),
                nAs(0),
                nProfile(HEVC_PROFILE_NONE),
                nLevel(),
                nPacketizationMode(1),
                bShow_Profile(IMS_FALSE),
                bShow_Level(IMS_FALSE),
                bShow_SpropParam(IMS_FALSE),
                bShow_PacketizationMode(IMS_FALSE)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }

            this->eResolution = pFmtp->eResolution;
            this->nBitrate = pFmtp->nBitrate;
            this->nFrameRate = pFmtp->nFrameRate;
            this->nAs = pFmtp->nAs;
            this->nProfile = pFmtp->nProfile;
            this->nLevel = pFmtp->nLevel;
            this->strVps = pFmtp->strVps;
            this->strSps = pFmtp->strSps;
            this->strPps = pFmtp->strPps;
            this->strSpropParam = pFmtp->strSpropParam;
            this->nPacketizationMode = pFmtp->nPacketizationMode;
            this->bShow_Profile = pFmtp->bShow_Profile;
            this->bShow_Level = pFmtp->bShow_Level;
            this->bShow_SpropParam = pFmtp->bShow_SpropParam;
            this->bShow_PacketizationMode = pFmtp->bShow_PacketizationMode;
        };

        HevcFmtp(IN const VIDEO_RESOLUTION resol, IN const VIDEO_PROFILE_HEVC profile,
                IN const IMS_UINT32 level, IN const IMS_UINT32 packetization,
                IN const AString sprop) :
                eResolution(resol),
                nBitrate(0),
                nFrameRate(30),
                nAs(0),
                nProfile(profile),
                nLevel(level),
                strVps(sprop),
                strSps(sprop),
                strPps(sprop),
                strSpropParam(sprop),
                nPacketizationMode(packetization),
                bShow_Profile(IMS_FALSE),
                bShow_Level(IMS_FALSE),
                bShow_SpropParam(IMS_FALSE),
                bShow_PacketizationMode(IMS_FALSE){};
    };

public:
    class AvcFmtp
    {
    public:
        VIDEO_RESOLUTION eResolution;
        IMS_SINT32 nBitrate;
        IMS_SINT32 nFrameRate;
        IMS_SINT32 nAs;
        VIDEO_PROFILE_AVC nProfile;
        IMS_UINT32 nLevel;
        AString strProfileLevelId;
        IMS_SINT32 nPacketizationMode;
        AString strSpropParam;
        IMS_BOOL bShow_ProfileLevelId;
        IMS_BOOL bShow_PacketizationMode;
        IMS_BOOL bShow_SpropParam;

    public:
        explicit AvcFmtp(IN AvcFmtp* pFmtp = IMS_NULL) :
                eResolution(VIDEO_RESOLUTION_INVALID),
                nBitrate(0),
                nFrameRate(15),
                nAs(0),
                nProfile(AVC_PROFILE_NONE),
                nLevel(12),
                nPacketizationMode(1),
                bShow_ProfileLevelId(IMS_FALSE),
                bShow_PacketizationMode(IMS_FALSE),
                bShow_SpropParam(IMS_FALSE)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }

            this->eResolution = pFmtp->eResolution;
            this->nBitrate = pFmtp->nBitrate;
            this->nFrameRate = pFmtp->nFrameRate;
            this->nAs = pFmtp->nAs;
            this->nProfile = pFmtp->nProfile;
            this->nLevel = pFmtp->nLevel;

            this->strProfileLevelId = pFmtp->strProfileLevelId;
            this->nPacketizationMode = pFmtp->nPacketizationMode;
            this->strSpropParam = pFmtp->strSpropParam;

            this->bShow_ProfileLevelId = pFmtp->bShow_ProfileLevelId;
            this->bShow_PacketizationMode = pFmtp->bShow_PacketizationMode;
            this->bShow_SpropParam = pFmtp->bShow_SpropParam;
        };

        AvcFmtp(IN const VIDEO_RESOLUTION resol, IN const VIDEO_PROFILE_AVC profile,
                IN const IMS_UINT32 level, IN const AString profileLevelID,
                IN const IMS_UINT32 packetization, IN const AString sprop) :
                eResolution(resol),
                nBitrate(0),
                nFrameRate(15),
                nAs(0),
                nProfile(profile),
                nLevel(level),
                strProfileLevelId(profileLevelID),
                nPacketizationMode(packetization),
                strSpropParam(sprop),
                bShow_ProfileLevelId(IMS_FALSE),
                bShow_PacketizationMode(IMS_FALSE),
                bShow_SpropParam(IMS_FALSE){};
    };

public:
    class RtcpFbAttributes
    {
    public:
        IMS_BOOL bTrrSupported;
        IMS_SINT32 nTrrInt;
        IMS_BOOL bNackSupported;
        IMS_BOOL bTmmbrSupported;
        IMS_SINT32 nTmmbrSmaxPr;
        IMS_BOOL bPliSupported;
        IMS_BOOL bFirSupported;

    public:
        RtcpFbAttributes() :
                bTrrSupported(IMS_FALSE),
                nTrrInt(0),
                bNackSupported(IMS_FALSE),
                bTmmbrSupported(IMS_FALSE),
                nTmmbrSmaxPr(-1),
                bPliSupported(IMS_FALSE),
                bFirSupported(IMS_FALSE){};

        RtcpFbAttributes& operator=(IN const RtcpFbAttributes& obj)
        {
            this->bTrrSupported = obj.bTrrSupported;
            this->nTrrInt = obj.nTrrInt;
            this->bNackSupported = obj.bNackSupported;
            this->bTmmbrSupported = obj.bTmmbrSupported;
            this->nTmmbrSmaxPr = obj.nTmmbrSmaxPr;
            this->bPliSupported = obj.bPliSupported;
            this->bFirSupported = obj.bFirSupported;
            return *this;
        }
    };

public:
    class CapaNego
    {
    public:
        IMSMap<IMS_SINT32, AString> mapTransportCapa;
        IMSMap<IMS_SINT32, AString> mapAttributeCapa;
        IMSList<AString> lstPotentialConfig;
        AString strNegotiatedAcfg;
        IMS_BOOL bIsAttCapaInPcfg;

    public:
        CapaNego() :
                strNegotiatedAcfg(""),
                bIsAttCapaInPcfg(IMS_FALSE){};

        CapaNego& operator=(IN const CapaNego& obj)
        {
            this->mapTransportCapa = obj.mapTransportCapa;
            this->mapAttributeCapa = obj.mapAttributeCapa;
            this->lstPotentialConfig = obj.lstPotentialConfig;
            this->strNegotiatedAcfg = obj.strNegotiatedAcfg;
            this->bIsAttCapaInPcfg = obj.bIsAttCapaInPcfg;
            return *this;
        }
    };

public:
    class Payload
    {
    public:
        RtpMap objRtpMap;
        void* pFmtp;
        IMS_BOOL bIncludeImageAttr;
        IMS_BOOL bIncludeFrameSize;
        AString strImageAttr;
        RtcpFbAttributes objRtcpFbAttr;

    public:
        Payload() :
                pFmtp(IMS_NULL),
                bIncludeImageAttr(IMS_FALSE),
                bIncludeFrameSize(IMS_FALSE),
                strImageAttr(AString::ConstNull()){};

        Payload(IN const Payload& obj)
        {
            this->objRtpMap = obj.objRtpMap;

            if (objRtpMap.strPayloadType.Equals("H264"))
            {
                pFmtp = new VideoProfile::AvcFmtp(
                        reinterpret_cast<VideoProfile::AvcFmtp*>(obj.pFmtp));
            }
            else if (objRtpMap.strPayloadType.Equals("H265"))
            {
                pFmtp = new VideoProfile::HevcFmtp(
                        reinterpret_cast<VideoProfile::HevcFmtp*>(obj.pFmtp));
            }

            this->bIncludeImageAttr = obj.bIncludeImageAttr;
            this->bIncludeFrameSize = obj.bIncludeFrameSize;
            this->strImageAttr = obj.strImageAttr;
            this->objRtcpFbAttr = obj.objRtcpFbAttr;
        }

        virtual ~Payload() { deleteFmtp(); }

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                this->objRtpMap = obj.objRtpMap;
                deleteFmtp();

                if (objRtpMap.strPayloadType.Equals("H264"))
                {
                    pFmtp = new VideoProfile::AvcFmtp(
                            reinterpret_cast<VideoProfile::AvcFmtp*>(obj.pFmtp));
                }
                else if (objRtpMap.strPayloadType.Equals("H265"))
                {
                    pFmtp = new VideoProfile::HevcFmtp(
                            reinterpret_cast<VideoProfile::HevcFmtp*>(obj.pFmtp));
                }

                this->bIncludeImageAttr = obj.bIncludeImageAttr;
                this->bIncludeFrameSize = obj.bIncludeFrameSize;
                this->strImageAttr = obj.strImageAttr;
                this->objRtcpFbAttr = obj.objRtcpFbAttr;
            }

            return *this;
        }

        void SetRtpMap(IN const IMS_UINT32 payloadNum, IN const AString payloadType,
                const IN IMS_UINT32 samplingRate, IN const IMS_SINT32 nChannel)
        {
            objRtpMap.nPayloadNum = payloadNum;
            objRtpMap.strPayloadType = payloadType;
            objRtpMap.nSamplingRate = samplingRate;
            objRtpMap.nChannel = nChannel;
        };

    private:
        void deleteFmtp()
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }

            if (objRtpMap.strPayloadType.Equals("H264"))
            {
                delete reinterpret_cast<VideoProfile::AvcFmtp*>(this->pFmtp);
            }
            else if (objRtpMap.strPayloadType.Equals("H265"))
            {
                delete reinterpret_cast<VideoProfile::HevcFmtp*>(this->pFmtp);
            }
        }
    };

public:
    IPAddress objIpAddress;
    IMS_UINT32 nDataPort;
    IMS_UINT32 nControlPort;
    AString strTransportType;
    IMS_UINT32 nRtcpInterval;
    IMS_SINT32 nBandwidthAs;
    IMS_SINT32 nBandwidthRs;
    IMS_SINT32 nBandwidthRr;
    IMSList<Payload*> lstPayload;
    MEDIA_DIRECTION eDirection;
    IMS_SINT32 nFrameRate;
    IMS_BOOL bSupportAvpf;
    IMS_SINT32 nCvoId;
    IMS_BOOL bSupportCapaNegoForAvpf;
    CapaNego objCapaNego;
    IMS_SINT32 nNegotiatedPayloadIndex;

public:
    VideoProfile() :
            nDataPort(0),
            nControlPort(0),
            strTransportType("RTP/AVPF"),
            nRtcpInterval(0),
            nBandwidthAs(0),
            nBandwidthRs(0),
            nBandwidthRr(0),
            eDirection(MEDIA_DIRECTION_INVALID),
            nFrameRate(0),
            bSupportAvpf(IMS_FALSE),
            nCvoId(-1),
            bSupportCapaNegoForAvpf(IMS_FALSE),
            nNegotiatedPayloadIndex(-1){};

    VideoProfile(IN const VideoProfile& objProfile)
    {
        copy(&objProfile);
        nNegotiatedPayloadIndex = -1;
    }

    VideoProfile& operator=(IN const VideoProfile& obj)
    {
        copy(&obj);
        nNegotiatedPayloadIndex = -1;
        return *this;
    }

    virtual ~VideoProfile() { deletePayloads(); };

private:
    void copy(const VideoProfile* pProfile)
    {
        if (pProfile == IMS_NULL)
        {
            return;
        }

        this->objIpAddress = pProfile->objIpAddress;
        this->nDataPort = pProfile->nDataPort;
        this->nControlPort = pProfile->nControlPort;
        this->strTransportType = pProfile->strTransportType;
        this->nRtcpInterval = pProfile->nRtcpInterval;
        this->nBandwidthAs = pProfile->nBandwidthAs;
        this->nBandwidthRs = pProfile->nBandwidthRs;
        this->nBandwidthRr = pProfile->nBandwidthRr;

        deletePayloads();

        for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
        {
            VideoProfile::Payload* pNewPayload =
                    new VideoProfile::Payload(*pProfile->lstPayload.GetAt(i));
            this->lstPayload.Append(pNewPayload);
        }

        this->eDirection = pProfile->eDirection;
        this->nFrameRate = pProfile->nFrameRate;
        this->bSupportAvpf = pProfile->bSupportAvpf;
        this->nCvoId = pProfile->nCvoId;
        this->bSupportCapaNegoForAvpf = pProfile->bSupportCapaNegoForAvpf;
        this->objCapaNego = pProfile->objCapaNego;
    }

    void deletePayloads()
    {
        while (lstPayload.GetSize() > 0)
        {
            VideoProfile::Payload* pPayload = lstPayload.GetAt(0);

            if (pPayload != IMS_NULL)
            {
                delete pPayload;
            }

            lstPayload.RemoveAt(0);
        }
    }
};
#endif /* End of _IMS_VIDEO_NEGO_PROFILE_H_*/
