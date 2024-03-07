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

#ifndef VIDEO_PROFILE_H_
#define VIDEO_PROFILE_H_

#include "ImsTypeDef.h"
#include "IpAddress.h"
#include "ImsMap.h"
#include "MediaDef.h"
#include "VideoDef.h"
#include "MediaBaseProfile.h"

/**
 * VideoProfile is used to keep the SDP negotiation information for video like
 * SDP offer, answer and the negotiated media information.
 */
class VideoProfile : public MediaBaseProfile
{
public:
    /**
     * HevcFmtp attributes are used within the SDP to carry HEVC parameters that provide
     * extra configuration details about a specific HEVC codec used in the RTP stream.
     */
    class HevcFmtp : public BaseFmtp
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

            eResolution = pFmtp->eResolution;
            nBitrate = pFmtp->nBitrate;
            nFrameRate = pFmtp->nFrameRate;
            nAs = pFmtp->nAs;
            nProfile = pFmtp->nProfile;
            nLevel = pFmtp->nLevel;
            strVps = pFmtp->strVps;
            strSps = pFmtp->strSps;
            strPps = pFmtp->strPps;
            strSpropParam = pFmtp->strSpropParam;
            nPacketizationMode = pFmtp->nPacketizationMode;
            bShow_Profile = pFmtp->bShow_Profile;
            bShow_Level = pFmtp->bShow_Level;
            bShow_SpropParam = pFmtp->bShow_SpropParam;
            bShow_PacketizationMode = pFmtp->bShow_PacketizationMode;
        };

        HevcFmtp(IN const VIDEO_RESOLUTION resol, IN const VIDEO_PROFILE_HEVC profile,
                IN const IMS_UINT32 level, IN const IMS_UINT32 packetization,
                IN const AString& sprop) :
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

        virtual ~HevcFmtp(){};
    };

public:
    /**
     * AvcFmtp attributes are used within the SDP to carry AVC parameters that provide
     * extra configuration details about a specific AVC codec used in the RTP stream.
     */
    class AvcFmtp : public BaseFmtp
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

            eResolution = pFmtp->eResolution;
            nBitrate = pFmtp->nBitrate;
            nFrameRate = pFmtp->nFrameRate;
            nAs = pFmtp->nAs;
            nProfile = pFmtp->nProfile;
            nLevel = pFmtp->nLevel;

            strProfileLevelId = pFmtp->strProfileLevelId;
            nPacketizationMode = pFmtp->nPacketizationMode;
            strSpropParam = pFmtp->strSpropParam;

            bShow_ProfileLevelId = pFmtp->bShow_ProfileLevelId;
            bShow_PacketizationMode = pFmtp->bShow_PacketizationMode;
            bShow_SpropParam = pFmtp->bShow_SpropParam;
        };

        AvcFmtp(IN const VIDEO_RESOLUTION resol, IN const VIDEO_PROFILE_AVC profile,
                IN const IMS_UINT32& level, IN const AString& profileLevelID,
                IN const IMS_UINT32& packetization, IN const AString& sprop) :
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

        virtual ~AvcFmtp(){};
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
            if (this != &obj)
            {
                bTrrSupported = obj.bTrrSupported;
                nTrrInt = obj.nTrrInt;
                bNackSupported = obj.bNackSupported;
                bTmmbrSupported = obj.bTmmbrSupported;
                nTmmbrSmaxPr = obj.nTmmbrSmaxPr;
                bPliSupported = obj.bPliSupported;
                bFirSupported = obj.bFirSupported;
            }
            return (*this);
        }
    };

public:
    class CapaNego
    {
    public:
        ImsMap<IMS_SINT32, AString> mapTransportCapa;
        ImsMap<IMS_SINT32, AString> mapAttributeCapa;
        ImsList<AString> lstPotentialConfig;
        AString strNegotiatedAcfg;
        IMS_BOOL bIsAttCapaInPcfg;

    public:
        CapaNego() :
                strNegotiatedAcfg(""),
                bIsAttCapaInPcfg(IMS_FALSE){};

        CapaNego& operator=(IN const CapaNego& obj)
        {
            if (this != &obj)
            {
                mapTransportCapa = obj.mapTransportCapa;
                mapAttributeCapa = obj.mapAttributeCapa;
                lstPotentialConfig = obj.lstPotentialConfig;
                strNegotiatedAcfg = obj.strNegotiatedAcfg;
                bIsAttCapaInPcfg = obj.bIsAttCapaInPcfg;
            }
            return (*this);
        }
    };

public:
    /**
     * Payload for video is the actual video data transported by RTP in a packet.
     */
    class Payload : public BasePayload
    {
    public:
        IMS_BOOL bIncludeImageAttr;
        IMS_BOOL bIncludeFrameSize;
        AString strImageAttr;
        RtcpFbAttributes objRtcpFbAttr;

    public:
        Payload() :
                BasePayload(),
                bIncludeImageAttr(IMS_FALSE),
                bIncludeFrameSize(IMS_FALSE),
                strImageAttr(AString::ConstNull()),
                objRtcpFbAttr(){};

        Payload(IN const Payload& obj) :
                BasePayload(obj),
                bIncludeImageAttr(obj.bIncludeImageAttr),
                bIncludeFrameSize(obj.bIncludeFrameSize),
                strImageAttr(obj.strImageAttr),
                objRtcpFbAttr(obj.objRtcpFbAttr)
        {
            if (objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
            {
                pFmtp = new VideoProfile::AvcFmtp(static_cast<VideoProfile::AvcFmtp*>(obj.pFmtp));
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
            {
                pFmtp = new VideoProfile::HevcFmtp(static_cast<VideoProfile::HevcFmtp*>(obj.pFmtp));
            }
        }

        virtual ~Payload() {}

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                BasePayload::operator=(obj);

                if (objRtpMap.strPayloadType.EqualsIgnoreCase("H264"))
                {
                    pFmtp = new VideoProfile::AvcFmtp(
                            static_cast<VideoProfile::AvcFmtp*>(obj.pFmtp));
                }
                else if (objRtpMap.strPayloadType.EqualsIgnoreCase("H265"))
                {
                    pFmtp = new VideoProfile::HevcFmtp(
                            static_cast<VideoProfile::HevcFmtp*>(obj.pFmtp));
                }

                bIncludeImageAttr = obj.bIncludeImageAttr;
                bIncludeFrameSize = obj.bIncludeFrameSize;
                strImageAttr = obj.strImageAttr;
                objRtcpFbAttr = obj.objRtcpFbAttr;
            }

            return (*this);
        }
    };

public:
    ImsList<Payload*> lstPayload;
    IMS_SINT32 nFrameRate;
    IMS_BOOL bSupportAvpf;
    IMS_SINT32 nCvoId;
    IMS_BOOL bSupportCapaNegoForAvpf;
    CapaNego objCapaNego;
    IMS_SINT32 nNegotiatedPayloadIndex;

public:
    VideoProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVPF", 0, 0, 0, 0, MEDIA_DIRECTION_INVALID),
            lstPayload(ImsList<Payload*>()),
            nFrameRate(0),
            bSupportAvpf(IMS_FALSE),
            nCvoId(-1),
            bSupportCapaNegoForAvpf(IMS_FALSE),
            objCapaNego(CapaNego()),
            nNegotiatedPayloadIndex(-1){};

    virtual ~VideoProfile() { deletePayloads(); };

    VideoProfile(IN VideoProfile* profile) :
            MediaBaseProfile(profile)
    {
        if (profile == nullptr)
        {
            return;
        }
        nFrameRate = profile->nFrameRate;
        bSupportAvpf = profile->bSupportAvpf;
        nCvoId = profile->nCvoId;
        bSupportCapaNegoForAvpf = profile->bSupportCapaNegoForAvpf;
        objCapaNego = profile->objCapaNego;
        nNegotiatedPayloadIndex = -1;

        deletePayloads();
        addPayloads(profile->lstPayload);
    }

    VideoProfile(IN const VideoProfile& obj) :
            MediaBaseProfile(obj)
    {
        nFrameRate = obj.nFrameRate;
        bSupportAvpf = obj.bSupportAvpf;
        nCvoId = obj.nCvoId;
        bSupportCapaNegoForAvpf = obj.bSupportCapaNegoForAvpf;
        objCapaNego = obj.objCapaNego;
        nNegotiatedPayloadIndex = -1;

        deletePayloads();
        addPayloads(obj.lstPayload);
    }

    VideoProfile& operator=(IN const VideoProfile& obj)
    {
        if (this != &obj)
        {
            MediaBaseProfile::operator=(obj);
            nFrameRate = obj.nFrameRate;
            bSupportAvpf = obj.bSupportAvpf;
            nCvoId = obj.nCvoId;
            bSupportCapaNegoForAvpf = obj.bSupportCapaNegoForAvpf;
            objCapaNego = obj.objCapaNego;

            deletePayloads();
            addPayloads(obj.lstPayload);
        }

        nNegotiatedPayloadIndex = -1;
        return (*this);
    }

private:
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

    void addPayloads(IN ImsList<Payload*> payloadList)
    {
        for (IMS_UINT32 i = 0; i < payloadList.GetSize(); i++)
        {
            VideoProfile::Payload* pNewPayload = new VideoProfile::Payload(*payloadList.GetAt(i));
            lstPayload.Append(pNewPayload);
        }
    }
};

#endif
