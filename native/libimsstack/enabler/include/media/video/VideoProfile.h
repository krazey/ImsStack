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
     * VideoFmtp attributes are used within the SDP to carry video parameters that provide
     * extra configurations for the specific video codecs described in the rtpmap.
     */
    class VideoFmtp : public BaseFmtp
    {
    public:
        explicit VideoFmtp(IN VideoFmtp* pFmtp = IMS_NULL) :
                m_eResolution(VIDEO_RESOLUTION_INVALID),
                m_nBitrate(0),
                m_nFrameRate(0),
                m_nAs(0),
                m_nProfile(0),
                m_nLevel(0),
                m_nPacketizationMode(1),
                m_strSpropParam(AString::ConstNull()),
                m_bShowPacketizationMode(IMS_FALSE),
                m_bShowSpropParam(IMS_FALSE)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }

            m_eResolution = pFmtp->m_eResolution;
            m_nBitrate = pFmtp->m_nBitrate;
            m_nFrameRate = pFmtp->m_nFrameRate;
            m_nAs = pFmtp->m_nAs;
            m_nProfile = pFmtp->m_nProfile;
            m_nLevel = pFmtp->m_nLevel;
            m_nPacketizationMode = pFmtp->m_nPacketizationMode;
            m_strSpropParam = pFmtp->m_strSpropParam;
            m_bShowPacketizationMode = pFmtp->m_bShowPacketizationMode;
            m_bShowSpropParam = pFmtp->m_bShowSpropParam;
        };

        VideoFmtp(IN const VIDEO_RESOLUTION eResolution, IN const IMS_SINT32 nBitrate,
                IN const IMS_SINT32 nFrameRate, IN const IMS_SINT32 nAs,
                IN const IMS_UINT32 nProfile, IN const IMS_UINT32 nLevel,
                IN const IMS_UINT32 nPacketization, IN const AString strSprop) :
                m_eResolution(eResolution),
                m_nBitrate(nBitrate),
                m_nFrameRate(nFrameRate),
                m_nAs(nAs),
                m_nProfile(nProfile),
                m_nLevel(nLevel),
                m_nPacketizationMode(nPacketization),
                m_strSpropParam(strSprop),
                m_bShowPacketizationMode(IMS_FALSE),
                m_bShowSpropParam(IMS_FALSE) {};

        virtual ~VideoFmtp() {};

        inline void SetResolution(IN const VIDEO_RESOLUTION eResolution)
        {
            m_eResolution = eResolution;
        }
        inline VIDEO_RESOLUTION GetResolution() { return m_eResolution; }
        inline void SetBitrate(IN const IMS_SINT32 nBitrate) { m_nBitrate = nBitrate; }
        inline IMS_SINT32 GetBitrate() { return m_nBitrate; }
        inline void SetFramerate(IN const IMS_SINT32 nFrameRate) { m_nFrameRate = nFrameRate; }
        inline IMS_SINT32 GetFramerate() { return m_nFrameRate; }
        inline void SetAs(IN const IMS_SINT32 nAs) { m_nAs = nAs; }
        inline IMS_SINT32 GetAs() { return m_nAs; }
        inline void SetProfile(IN const IMS_SINT32 nProfile) { m_nProfile = nProfile; }
        inline IMS_SINT32 GetProfile() { return m_nProfile; }
        inline void SetLevel(IN const IMS_SINT32 nLevel) { m_nLevel = nLevel; }
        inline IMS_SINT32 GetLevel() { return m_nLevel; }
        inline void SetPacketizationMode(IN const IMS_SINT32 nMode)
        {
            m_nPacketizationMode = nMode;
        }
        inline IMS_SINT32 GetPacketizationMode() { return m_nPacketizationMode; }
        inline void SetSpropParam(IN const AString strSprop) { m_strSpropParam = strSprop; }
        inline AString& GetSpropParam() { return m_strSpropParam; }
        inline void SetShowPacketizationMode(IN const IMS_BOOL nShow)
        {
            m_bShowPacketizationMode = nShow;
        }
        inline IMS_BOOL IsPacketizationModeVisible() { return m_bShowPacketizationMode; }
        inline void SetShowSpropParam(IN const IMS_BOOL nShow) { m_bShowSpropParam = nShow; }
        inline IMS_BOOL IsSpropParamVisible() { return m_bShowSpropParam; }

    private:
        VIDEO_RESOLUTION m_eResolution;
        IMS_SINT32 m_nBitrate;
        IMS_SINT32 m_nFrameRate;
        IMS_SINT32 m_nAs;
        IMS_UINT32 m_nProfile;
        IMS_UINT32 m_nLevel;
        IMS_SINT32 m_nPacketizationMode;
        AString m_strSpropParam;
        IMS_BOOL m_bShowPacketizationMode;
        IMS_BOOL m_bShowSpropParam;
    };

    /**
     * AvcFmtp attributes are used within the SDP to carry AVC parameters that provide
     * extra configurations for the specific video codecs described in the rtpmap.
     */
    class AvcFmtp : public VideoFmtp
    {
    public:
        explicit AvcFmtp(IN AvcFmtp* pFmtp = IMS_NULL) :
                VideoFmtp(pFmtp),
                m_strProfileLevelId(AString::ConstNull()),
                m_bShowProfileLevelId(IMS_FALSE)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }
            m_strProfileLevelId = pFmtp->m_strProfileLevelId;
            m_bShowProfileLevelId = pFmtp->m_bShowProfileLevelId;
        };

        AvcFmtp(IN const VIDEO_RESOLUTION eResolution, IN const IMS_UINT32 nBitrate,
                IN const IMS_UINT32 nFrameRate, IN const IMS_UINT32 nAs,
                IN const IMS_UINT32 nProfile, IN const IMS_UINT32 nLevel,
                IN const AString strProfileLevelId, IN const IMS_UINT32 nPacketization,
                IN const AString strSprop) :
                VideoFmtp(eResolution, nBitrate, nFrameRate, nAs, nProfile, nLevel, nPacketization,
                        strSprop),
                m_strProfileLevelId(strProfileLevelId),
                m_bShowProfileLevelId(IMS_FALSE) {};

        virtual ~AvcFmtp() {};

        inline void SetProfileLevelId(IN const AString strProfileLevelId)
        {
            m_strProfileLevelId = strProfileLevelId;
        }
        inline AString& GetProfileLevelId() { return m_strProfileLevelId; }
        inline void SetShowProfileLevelId(IN const IMS_BOOL nShow)
        {
            m_bShowProfileLevelId = nShow;
        }
        inline IMS_BOOL IsProfileLevelIdVisible() { return m_bShowProfileLevelId; }

    private:
        AString m_strProfileLevelId;
        IMS_BOOL m_bShowProfileLevelId;
    };

    /**
     * HevcFmtp attributes are used within the SDP to carry HEVC parameters that provide
     * extra configurations for the specific video codecs described in the rtpmap.
     */
    class HevcFmtp : public VideoFmtp
    {
    public:
        explicit HevcFmtp(IN HevcFmtp* pFmtp = IMS_NULL) :
                VideoFmtp(pFmtp),
                m_bShowProfile(IMS_FALSE),
                m_bShowLevel(IMS_FALSE)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }

            m_bShowProfile = pFmtp->m_bShowProfile;
            m_bShowLevel = pFmtp->m_bShowLevel;
        };

        HevcFmtp(IN const VIDEO_RESOLUTION eResolution, IN const IMS_UINT32 nBitrate,
                IN const IMS_UINT32 nFrameRate, IN const IMS_UINT32 nAs,
                IN const IMS_UINT32 nProfile, IN const IMS_UINT32 nLevel,
                IN const IMS_UINT32 nPacketization, IN const AString strSprop) :
                VideoFmtp(eResolution, nBitrate, nFrameRate, nAs, nProfile, nLevel, nPacketization,
                        strSprop),
                m_bShowProfile(IMS_FALSE),
                m_bShowLevel(IMS_FALSE) {};

        virtual ~HevcFmtp() {};

        inline void SetShowProfile(IN const IMS_BOOL nShow) { m_bShowProfile = nShow; }
        inline IMS_BOOL IsProfileVisible() { return m_bShowProfile; }
        inline void SetShowLevel(IN const IMS_BOOL nShow) { m_bShowLevel = nShow; }
        inline IMS_BOOL IsLevelVisible() { return m_bShowLevel; }

    private:
        IMS_BOOL m_bShowProfile;
        IMS_BOOL m_bShowLevel;
    };

public:
    class RtcpFbAttributes
    {
    public:
        RtcpFbAttributes() :
                m_bTrrSupported(IMS_FALSE),
                m_nTrrInt(0),
                m_bNackSupported(IMS_FALSE),
                m_bTmmbrSupported(IMS_FALSE),
                m_nTmmbrSmaxPr(-1),
                m_bPliSupported(IMS_FALSE),
                m_bFirSupported(IMS_FALSE) {};

        RtcpFbAttributes& operator=(IN const RtcpFbAttributes& obj)
        {
            if (this != &obj)
            {
                m_bTrrSupported = obj.m_bTrrSupported;
                m_nTrrInt = obj.m_nTrrInt;
                m_bNackSupported = obj.m_bNackSupported;
                m_bTmmbrSupported = obj.m_bTmmbrSupported;
                m_nTmmbrSmaxPr = obj.m_nTmmbrSmaxPr;
                m_bPliSupported = obj.m_bPliSupported;
                m_bFirSupported = obj.m_bFirSupported;
            }
            return (*this);
        }

        bool operator==(IN const RtcpFbAttributes& obj) const
        {
            return (m_bTrrSupported == obj.m_bTrrSupported && m_nTrrInt == obj.m_nTrrInt &&
                    m_bNackSupported == obj.m_bNackSupported &&
                    m_bTmmbrSupported == obj.m_bTmmbrSupported &&
                    m_nTmmbrSmaxPr == obj.m_nTmmbrSmaxPr &&
                    m_bPliSupported == obj.m_bPliSupported &&
                    m_bFirSupported == obj.m_bFirSupported);
        }

        bool operator!=(IN const RtcpFbAttributes& obj) const { return !(*this == obj); }

        inline void SetTrrSupported(IN const IMS_BOOL nTrrSupported)
        {
            m_bTrrSupported = nTrrSupported;
        }
        inline IMS_BOOL IsTrrSupported() { return m_bTrrSupported; }
        inline void SetTrrInt(IN const IMS_SINT32 nTrrInt) { m_nTrrInt = nTrrInt; }
        inline IMS_SINT32 GetTrrInt() { return m_nTrrInt; }
        inline void SetNackSupported(IN const IMS_BOOL bNackSupported)
        {
            m_bNackSupported = bNackSupported;
        }
        inline IMS_BOOL IsNackSupported() { return m_bNackSupported; }
        inline void SetTmmbrSupported(IN const IMS_BOOL bTmmbrSupported)
        {
            m_bTmmbrSupported = bTmmbrSupported;
        }
        inline IMS_BOOL IsTmmbrSupported() { return m_bTmmbrSupported; }
        inline void SetTmmbrSmaxPr(IN const IMS_SINT32 nTmmbrSmaxPr)
        {
            m_nTmmbrSmaxPr = nTmmbrSmaxPr;
        }
        inline IMS_SINT32 GetTmmbrSmaxPr() { return m_nTmmbrSmaxPr; }
        inline void SetPliSupported(IN const IMS_BOOL bPliSupported)
        {
            m_bPliSupported = bPliSupported;
        }
        inline IMS_BOOL IsPliSupported() { return m_bPliSupported; }
        inline void SetFirSupported(IN const IMS_BOOL bFirSupported)
        {
            m_bFirSupported = bFirSupported;
        }
        inline IMS_BOOL IsFirSupported() { return m_bFirSupported; }

    private:
        IMS_BOOL m_bTrrSupported;
        IMS_SINT32 m_nTrrInt;
        IMS_BOOL m_bNackSupported;
        IMS_BOOL m_bTmmbrSupported;
        IMS_SINT32 m_nTmmbrSmaxPr;
        IMS_BOOL m_bPliSupported;
        IMS_BOOL m_bFirSupported;
    };

public:
    /**
     * Payload for video is the actual video data transported by RTP in a packet.
     */
    class Payload : public BasePayload
    {
    public:
        Payload() :
                BasePayload(),
                m_bIncludeImageAttr(IMS_FALSE),
                m_bIncludeFrameSize(IMS_FALSE),
                m_strImageAttr(AString::ConstNull()),
                m_objRtcpFbAttr() {};

        Payload(IN const Payload& obj) :
                BasePayload(obj),
                m_bIncludeImageAttr(obj.m_bIncludeImageAttr),
                m_bIncludeFrameSize(obj.m_bIncludeFrameSize),
                m_strImageAttr(obj.m_strImageAttr),
                m_objRtcpFbAttr(obj.m_objRtcpFbAttr)
        {
            CreateVideoFmtp(obj);
        }

        virtual ~Payload() {}

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                BasePayload::operator=(obj);
                CreateVideoFmtp(obj);
                m_bIncludeImageAttr = obj.m_bIncludeImageAttr;
                m_bIncludeFrameSize = obj.m_bIncludeFrameSize;
                m_strImageAttr = obj.m_strImageAttr;
                m_objRtcpFbAttr = obj.m_objRtcpFbAttr;
            }

            return (*this);
        }

        bool operator==(IN const Payload& obj) const
        {
            return (BasePayload::operator==(obj) &&
                    m_bIncludeImageAttr == obj.m_bIncludeImageAttr &&
                    m_bIncludeFrameSize == obj.m_bIncludeFrameSize &&
                    m_strImageAttr == obj.m_strImageAttr && m_objRtcpFbAttr == obj.m_objRtcpFbAttr);
        }

        bool operator!=(IN const Payload& obj) const { return !(*this == obj); }

        inline void CreateVideoFmtp(IN const Payload& obj)
        {
            if (obj.m_pFmtp != IMS_NULL)
            {
                if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("H264"))
                {
                    m_pFmtp = new VideoProfile::AvcFmtp(
                            static_cast<VideoProfile::AvcFmtp*>(obj.m_pFmtp));
                }
                else if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("H265"))
                {
                    m_pFmtp = new VideoProfile::HevcFmtp(
                            static_cast<VideoProfile::HevcFmtp*>(obj.m_pFmtp));
                }
            }
        }

        inline void SetIncludeImageAttr(IN const IMS_BOOL bIncludeImageAttr)
        {
            m_bIncludeImageAttr = bIncludeImageAttr;
        }
        inline IMS_BOOL IsImageAttrIncluded() { return m_bIncludeImageAttr; }
        inline void SetIncludeFrameSize(IN const IMS_BOOL bIncludeFrameSize)
        {
            m_bIncludeFrameSize = bIncludeFrameSize;
        }
        inline IMS_BOOL IsFrameSizeIncluded() { return m_bIncludeFrameSize; }
        inline void SetImageAttr(IN const AString strImageAttr) { m_strImageAttr = strImageAttr; }
        inline AString& GetImageAttr() { return m_strImageAttr; }
        inline void SetRtcpFbAttr(IN const RtcpFbAttributes objRtcpFbAttr)
        {
            m_objRtcpFbAttr = objRtcpFbAttr;
        }
        inline RtcpFbAttributes& GetRtcpFbAttr() { return m_objRtcpFbAttr; }

    private:
        IMS_BOOL m_bIncludeImageAttr;
        IMS_BOOL m_bIncludeFrameSize;
        AString m_strImageAttr;
        RtcpFbAttributes m_objRtcpFbAttr;
    };

public:
    VideoProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVPF", 0, 0, 0, 0, MEDIA_DIRECTION_INVALID),
            m_nFrameRate(0),
            m_bSupportAvpf(IMS_FALSE),
            m_nCvoId(-1),
            m_bSupportCapaNegoForAvpf(IMS_FALSE) {};

    virtual ~VideoProfile() {}

    VideoProfile(IN const VideoProfile& obj) :
            MediaBaseProfile(obj)
    {
        m_nFrameRate = obj.m_nFrameRate;
        m_bSupportAvpf = obj.m_bSupportAvpf;
        m_nCvoId = obj.m_nCvoId;
        m_bSupportCapaNegoForAvpf = obj.m_bSupportCapaNegoForAvpf;
    }

    VideoProfile& operator=(IN const VideoProfile& obj)
    {
        if (this != &obj)
        {
            MediaBaseProfile::operator=(obj);
            m_nFrameRate = obj.m_nFrameRate;
            m_bSupportAvpf = obj.m_bSupportAvpf;
            m_nCvoId = obj.m_nCvoId;
            m_bSupportCapaNegoForAvpf = obj.m_bSupportCapaNegoForAvpf;
        }

        return (*this);
    }

    bool operator==(IN const VideoProfile& obj) const
    {
        return (MediaBaseProfile::operator==(obj) && m_nFrameRate == obj.m_nFrameRate &&
                m_bSupportAvpf == obj.m_bSupportAvpf && m_nCvoId == obj.m_nCvoId &&
                m_bSupportCapaNegoForAvpf == obj.m_bSupportCapaNegoForAvpf);
    }

    bool operator!=(IN const VideoProfile& obj) const { return !(*this == obj); }

    Payload* GetPayloadAt(IN IMS_UINT32 nIndex) override
    {
        BasePayload* pPayload = MediaBaseProfile::GetPayloadAt(nIndex);
        return (pPayload != IMS_NULL) ? static_cast<Payload*>(pPayload) : IMS_NULL;
    }

    inline void SetFrameRate(IN const IMS_SINT32 nFrameRate) { m_nFrameRate = nFrameRate; }
    inline IMS_SINT32 GetFrameRate() { return m_nFrameRate; }
    inline void SetSupportAvpf(IN const IMS_BOOL bSupportAvpf) { m_bSupportAvpf = bSupportAvpf; }
    inline IMS_BOOL IsAvpfSupported() { return m_bSupportAvpf; }
    inline void SetCvoId(IN const IMS_SINT32 nCvoId) { m_nCvoId = nCvoId; }
    inline IMS_SINT32 GetCvoId() { return m_nCvoId; }
    inline void SetSupportCapaNegoForAvpf(IN const IMS_BOOL bSupportCapaNegoForAvpf)
    {
        m_bSupportCapaNegoForAvpf = bSupportCapaNegoForAvpf;
    }
    inline IMS_BOOL IsCapaNegoForAvpfSupported() { return m_bSupportCapaNegoForAvpf; }

private:
    IMS_SINT32 m_nFrameRate;
    IMS_BOOL m_bSupportAvpf;
    IMS_SINT32 m_nCvoId;
    IMS_BOOL m_bSupportCapaNegoForAvpf;
};

#endif
