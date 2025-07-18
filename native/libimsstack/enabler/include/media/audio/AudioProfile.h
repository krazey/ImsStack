/*
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

#ifndef AUDIO_PROFILE_H_
#define AUDIO_PROFILE_H_

#include "config/CodecAudioConfig.h"
#include "MediaBaseProfile.h"

/**
 * AudioProfile is used to keep the SDP negotiation information for audio like
 * SDP offer, answer and the negotiated media information.
 */
class AudioProfile : public MediaBaseProfile
{
public:
    /**
     * AudioFmtp attributes are used within the SDP to carry audio parameters that provide
     * extra configurations for the specific audio codecs described in the rtpmap.
     */
    class AudioFmtp : public BaseFmtp
    {
    public:
        AudioFmtp() :
                m_nModeSetList(0),
                m_nDefaultRtpModeSet(0),
                m_nModeChangeCapability(CodecAudioConfig::DEFAULT_MODECHANGE_CAPABILITY),
                m_nModeChangePeriod(CodecAudioConfig::DEFAULT_MODECHANGE_PERIOD),
                m_nModeChangeNeighbor(CodecAudioConfig::DEFAULT_MODECHANGE_NEIGHBOR),
                m_nMaxRed(CodecAudioConfig::DEFAULT_MAXRED),
                m_bDtx(CodecAudioConfig::DEFAULT_DTX),
                m_bVisibleModeSet(IMS_FALSE),
                m_bVisibleModeChangeCapability(IMS_FALSE),
                m_bVisibleModeChangePeriod(IMS_FALSE),
                m_bVisibleModeChangeNeighbor(IMS_FALSE),
                m_bVisibleMaxRed(IMS_FALSE),
                m_bVisibleDtx(IMS_FALSE)
        {
        }

        AudioFmtp(IN const AudioFmtp& objFmtp) :
                m_nModeSetList(objFmtp.m_nModeSetList),
                m_nDefaultRtpModeSet(objFmtp.m_nDefaultRtpModeSet),
                m_nModeChangeCapability(objFmtp.m_nModeChangeCapability),
                m_nModeChangePeriod(objFmtp.m_nModeChangePeriod),
                m_nModeChangeNeighbor(objFmtp.m_nModeChangeNeighbor),
                m_nMaxRed(objFmtp.m_nMaxRed),
                m_bDtx(objFmtp.m_bDtx),
                m_bVisibleModeSet(objFmtp.m_bVisibleModeSet),
                m_bVisibleModeChangeCapability(objFmtp.m_bVisibleModeChangeCapability),
                m_bVisibleModeChangePeriod(objFmtp.m_bVisibleModeChangePeriod),
                m_bVisibleModeChangeNeighbor(objFmtp.m_bVisibleModeChangeNeighbor),
                m_bVisibleMaxRed(objFmtp.m_bVisibleMaxRed),
                m_bVisibleDtx(objFmtp.m_bVisibleDtx)
        {
        }

        virtual ~AudioFmtp() override {};

        inline void SetModeSetList(IN const IMS_UINT32 nModeSetList)
        {
            m_nModeSetList = nModeSetList;
        }
        inline IMS_UINT32 GetModeSetList() { return m_nModeSetList; }
        inline void SetDefaultRtpModeSet(IN const IMS_UINT32 nDefaultRtpModeSet)
        {
            m_nDefaultRtpModeSet = nDefaultRtpModeSet;
        }
        inline IMS_UINT32 GetDefaultRtpModeSet() { return m_nDefaultRtpModeSet; }
        inline void SetModeChangeCapability(IN const IMS_SINT32 nModeChangeCapability)
        {
            m_nModeChangeCapability = nModeChangeCapability;
        }
        inline IMS_SINT32 GetModeChangeCapability() { return m_nModeChangeCapability; }
        inline void SetModeChangePeriod(IN const IMS_SINT32 nModeChangePeriod)
        {
            m_nModeChangePeriod = nModeChangePeriod;
        }
        inline IMS_SINT32 GetModeChangePeriod() { return m_nModeChangePeriod; }
        inline void SetModeChangeNeighbor(IN const IMS_SINT32 nModeChangeNeighbor)
        {
            m_nModeChangeNeighbor = nModeChangeNeighbor;
        }
        inline IMS_SINT32 GetModeChangeNeighbor() { return m_nModeChangeNeighbor; }
        inline void SetMaxRed(IN const IMS_SINT32 nMaxRed) { m_nMaxRed = nMaxRed; }
        inline IMS_SINT32 GetMaxRed() { return m_nMaxRed; }
        inline void SetDtx(IN const IMS_BOOL bDtx) { m_bDtx = bDtx; }
        inline IMS_BOOL IsDtxEnabled() { return m_bDtx; }
        inline void SetVisibleModeSet(IN const IMS_BOOL bVisibleModeSet)
        {
            m_bVisibleModeSet = bVisibleModeSet;
        }
        inline IMS_BOOL IsModeSetVisible() { return m_bVisibleModeSet; }
        inline void SetVisibleModeChangeCapability(IN const IMS_BOOL bVisibleModeChangeCapability)
        {
            m_bVisibleModeChangeCapability = bVisibleModeChangeCapability;
        }
        inline IMS_BOOL IsModeChangeCapabilityVisible() { return m_bVisibleModeChangeCapability; }
        inline void SetVisibleModeChangePeriod(IN const IMS_BOOL bVisibleModeChangePeriod)
        {
            m_bVisibleModeChangePeriod = bVisibleModeChangePeriod;
        }
        inline IMS_BOOL IsModeChangePeriodVisible() { return m_bVisibleModeChangePeriod; }
        inline void SetVisibleModeChangeNeighbor(IN const IMS_BOOL bVisibleModeChangeNeighbor)
        {
            m_bVisibleModeChangeNeighbor = bVisibleModeChangeNeighbor;
        }
        inline IMS_BOOL IsModeChangeNeighborVisible() { return m_bVisibleModeChangeNeighbor; }
        inline void SetVisibleMaxRed(IN const IMS_BOOL bVisibleMaxRed)
        {
            m_bVisibleMaxRed = bVisibleMaxRed;
        }
        inline IMS_BOOL IsMaxRedVisible() { return m_bVisibleMaxRed; }
        inline void SetVisibleDtx(IN const IMS_BOOL bVisibleDtx) { m_bVisibleDtx = bVisibleDtx; }
        inline IMS_BOOL IsDtxVisible() { return m_bVisibleDtx; }

    private:
        IMS_UINT32 m_nModeSetList;
        IMS_UINT32 m_nDefaultRtpModeSet;
        IMS_SINT32 m_nModeChangeCapability;
        IMS_SINT32 m_nModeChangePeriod;
        IMS_SINT32 m_nModeChangeNeighbor;
        IMS_SINT32 m_nMaxRed;
        IMS_BOOL m_bDtx;
        IMS_BOOL m_bVisibleModeSet;
        IMS_BOOL m_bVisibleModeChangeCapability;
        IMS_BOOL m_bVisibleModeChangePeriod;
        IMS_BOOL m_bVisibleModeChangeNeighbor;
        IMS_BOOL m_bVisibleMaxRed;
        IMS_BOOL m_bVisibleDtx;
    };

    /**
     * AmrFmtp attributes are used within the SDP to carry AMR parameters that provide
     * extra configurations for the specific audio codecs described in the rtpmap.
     */
    class AmrFmtp : public AudioFmtp
    {
    public:
        enum
        {
            DEFAULT_OCTETALIGN = 0,
        };

        AmrFmtp() :
                AudioFmtp(),
                m_nOctetAlign(DEFAULT_OCTETALIGN),
                m_bVisibleOctetAlign(IMS_FALSE)
        {
        }

        AmrFmtp(IN const AmrFmtp& objFmtp) :
                AudioFmtp(objFmtp),
                m_nOctetAlign(objFmtp.m_nOctetAlign),
                m_bVisibleOctetAlign(objFmtp.m_bVisibleOctetAlign)
        {
        }

        virtual ~AmrFmtp() override {};

        inline void SetOctetAlign(IN const IMS_SINT32 nOctetAlign) { m_nOctetAlign = nOctetAlign; }
        inline IMS_SINT32 GetOctetAlign() { return m_nOctetAlign; }
        inline void SetVisibleOctetAlign(IN const IMS_BOOL bVisibleOctetAlign)
        {
            m_bVisibleOctetAlign = bVisibleOctetAlign;
        }
        inline IMS_BOOL IsOctetAlignVisible() { return m_bVisibleOctetAlign; }

    private:
        IMS_SINT32 m_nOctetAlign;
        IMS_BOOL m_bVisibleOctetAlign;
    };

public:
    /**
     * EvsFmtp attributes are used within the SDP to carry EVS parameters that provide
     * extra configurations for the specific audio codecs described in the rtpmap.
     */
    class EvsFmtp : public AudioFmtp
    {
    public:
        enum
        {
            DEFAULT_HFMODE = 0,
            DEFAULT_EVSMODESWITCH = 0,
            DEFAULT_BANDWIDTHLIST = 0,
            DEFAULT_BITRATELIST = 0,
            DEFAULT_BANDWIDTH = -1,
            DEFAULT_CMR = 0,
            DEFAULT_CHANNEL_AWMODE = 0,
        };

    public:
        EvsFmtp() :
                AudioFmtp(),
                m_nHfOnly(DEFAULT_HFMODE),
                m_nEvsModeSwitch(DEFAULT_EVSMODESWITCH),
                m_nBrList(0),
                m_nBrSend(0),
                m_nBrRecv(0),
                m_nBwList(0),
                m_nBwSend(0),
                m_nBwRecv(0),
                m_nCmr(DEFAULT_CMR),
                m_nChAwRecv(DEFAULT_CHANNEL_AWMODE),
                m_nReceivedChAwRecv(DEFAULT_CHANNEL_AWMODE),
                m_bVisibleHfOnly(IMS_FALSE),
                m_bVisibleEvsModeSwitch(IMS_FALSE),
                m_bVisibleCmr(IMS_FALSE),
                m_bVisibleChannelAwMode(IMS_FALSE),
                m_bVisibleBrList(IMS_TRUE),
                m_bVisibleBwList(IMS_TRUE),
                m_bSendCmr(IMS_FALSE)
        {
        }

        EvsFmtp(IN const EvsFmtp& objFmtp) :
                AudioFmtp(objFmtp),
                m_nHfOnly(objFmtp.m_nHfOnly),
                m_nEvsModeSwitch(objFmtp.m_nEvsModeSwitch),
                m_nBrList(objFmtp.m_nBrList),
                m_nBrSend(objFmtp.m_nBrSend),
                m_nBrRecv(objFmtp.m_nBrRecv),
                m_nBwList(objFmtp.m_nBwList),
                m_nBwSend(objFmtp.m_nBwSend),
                m_nBwRecv(objFmtp.m_nBwRecv),
                m_nCmr(objFmtp.m_nCmr),
                m_nChAwRecv(objFmtp.m_nChAwRecv),
                m_nReceivedChAwRecv(objFmtp.m_nReceivedChAwRecv),
                m_bVisibleHfOnly(objFmtp.m_bVisibleHfOnly),
                m_bVisibleEvsModeSwitch(objFmtp.m_bVisibleEvsModeSwitch),
                m_bVisibleCmr(objFmtp.m_bVisibleCmr),
                m_bVisibleChannelAwMode(objFmtp.m_bVisibleChannelAwMode),
                m_bVisibleBrList(objFmtp.m_bVisibleBrList),
                m_bVisibleBwList(objFmtp.m_bVisibleBwList),
                m_bSendCmr(objFmtp.m_bSendCmr)
        {
        }

        virtual ~EvsFmtp() override {};

        inline void SetHfOnly(IN const IMS_UINT32 nHfOnly) { m_nHfOnly = nHfOnly; }
        inline IMS_UINT32 GetHfOnly() { return m_nHfOnly; }
        inline void SetEvsModeSwitch(IN const IMS_UINT32 nEvsModeSwitch)
        {
            m_nEvsModeSwitch = nEvsModeSwitch;
        }
        inline IMS_UINT32 GetEvsModeSwitch() { return m_nEvsModeSwitch; }
        inline void SetBrList(IN const IMS_UINT32 nBrList) { m_nBrList = nBrList; }
        inline IMS_UINT32 GetBrList() { return m_nBrList; }
        inline void SetBrSend(IN const IMS_SINT32 nBrSend) { m_nBrSend = nBrSend; }
        inline IMS_SINT32 GetBrSend() { return m_nBrSend; }
        inline void SetBrRecv(IN const IMS_SINT32 nBrRecv) { m_nBrRecv = nBrRecv; }
        inline IMS_SINT32 GetBrRecv() { return m_nBrRecv; }
        inline void SetBwList(IN const IMS_UINT32 nBwList) { m_nBwList = nBwList; }
        inline IMS_UINT32 GetBwList() { return m_nBwList; }
        inline void SetBwSend(IN const IMS_SINT32 nBwSend) { m_nBwSend = nBwSend; }
        inline IMS_SINT32 GetBwSend() { return m_nBwSend; }
        inline void SetBwRecv(IN const IMS_SINT32 nBwRecv) { m_nBwRecv = nBwRecv; }
        inline IMS_SINT32 GetBwRecv() { return m_nBwRecv; }
        inline void SetCmr(IN const IMS_SINT32 nCmr) { m_nCmr = nCmr; }
        inline IMS_SINT32 GetCmr() { return m_nCmr; }
        inline void SetChAwRecv(IN const IMS_SINT32 nChAwRecv) { m_nChAwRecv = nChAwRecv; }
        inline IMS_SINT32 GetChAwRecv() { return m_nChAwRecv; }
        inline void SetReceivedChAwRecv(IN const IMS_SINT32 nReceivedChAwRecv)
        {
            m_nReceivedChAwRecv = nReceivedChAwRecv;
        }
        inline IMS_SINT32 GetReceivedChAwRecv() { return m_nReceivedChAwRecv; }
        inline void SetShowHfOnly(IN const IMS_BOOL bVisibleHfOnly)
        {
            m_bVisibleHfOnly = bVisibleHfOnly;
        }
        inline IMS_BOOL IsHfOnlyVisible() { return m_bVisibleHfOnly; }
        inline void SetShowEvsModeSwitch(IN const IMS_BOOL bVisibleEvsModeSwitch)
        {
            m_bVisibleEvsModeSwitch = bVisibleEvsModeSwitch;
        }
        inline IMS_BOOL IsEvsModeSwitchVisible() { return m_bVisibleEvsModeSwitch; }
        inline void SetShowCmr(IN const IMS_BOOL bVisibleCmr) { m_bVisibleCmr = bVisibleCmr; }
        inline IMS_BOOL IsCmrVisible() { return m_bVisibleCmr; }
        inline void SetShowChannelAwMode(IN const IMS_BOOL bVisibleChannelAwMode)
        {
            m_bVisibleChannelAwMode = bVisibleChannelAwMode;
        }
        inline IMS_BOOL IsChannelAwModeVisible() { return m_bVisibleChannelAwMode; }
        inline void SetShowBrList(IN const IMS_BOOL bVisibleBrList)
        {
            m_bVisibleBrList = bVisibleBrList;
        }
        inline IMS_BOOL IsBrListVisible() { return m_bVisibleBrList; }
        inline void SetShowBwList(IN const IMS_BOOL bVisibleBwList)
        {
            m_bVisibleBwList = bVisibleBwList;
        }
        inline IMS_BOOL IsBwListVisible() { return m_bVisibleBwList; }
        inline void SetSendCmr(IN const IMS_BOOL bSendCmr) { m_bSendCmr = bSendCmr; }
        inline IMS_BOOL IsSendCmrEnabled() { return m_bSendCmr; }

    private:
        IMS_UINT32 m_nHfOnly;
        IMS_UINT32 m_nEvsModeSwitch;
        IMS_UINT32 m_nBrList;
        IMS_SINT32 m_nBrSend;
        IMS_SINT32 m_nBrRecv;
        IMS_UINT32 m_nBwList;
        IMS_SINT32 m_nBwSend;
        IMS_SINT32 m_nBwRecv;
        IMS_SINT32 m_nCmr;
        IMS_SINT32 m_nChAwRecv;
        IMS_SINT32 m_nReceivedChAwRecv;
        IMS_BOOL m_bVisibleHfOnly;
        IMS_BOOL m_bVisibleEvsModeSwitch;
        IMS_BOOL m_bVisibleCmr;
        IMS_BOOL m_bVisibleChannelAwMode;
        IMS_BOOL m_bVisibleBrList;
        IMS_BOOL m_bVisibleBwList;
        IMS_BOOL m_bSendCmr;
    };

public:
    /**
     * TelephoneEventFmtp attributes are used within the SDP to carry TelephoneEvent parameters that
     * provide extra configurations for the specific TelephoneEvent codecs described in the rtpmap.
     */
    class TelephoneEventFmtp : public BaseFmtp
    {
    public:
        TelephoneEventFmtp() :
                m_strEvents("0-15") {};

        explicit TelephoneEventFmtp(IN const AString& events) :
                m_strEvents(events) {};

        TelephoneEventFmtp(IN const TelephoneEventFmtp& objFmtp) :
                m_strEvents(objFmtp.m_strEvents) {};

        virtual ~TelephoneEventFmtp() override {};

        TelephoneEventFmtp& operator=(IN const TelephoneEventFmtp& obj)
        {
            if (this != &obj)
            {
                m_strEvents = obj.m_strEvents;
            }

            return (*this);
        }

        bool operator==(IN const TelephoneEventFmtp& obj) const
        {
            return (m_strEvents == obj.m_strEvents);
        }

        bool operator!=(IN const TelephoneEventFmtp& obj) const { return !(*this == obj); }

        inline void SetEvents(IN const AString strEvents) { m_strEvents = strEvents; }
        inline AString& GetEvents() { return m_strEvents; }

    private:
        AString m_strEvents;
    };

public:
    /**
     * Payload for audio is the actual audio data transported by RTP in a packet.
     */
    class Payload : public BasePayload
    {
    public:
        Payload() :
                BasePayload(1) {};
        Payload(IN const Payload& obj) :
                BasePayload(obj)
        {
            CreateAudioFmtp(obj);
        }

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                BasePayload::operator=(obj);
                CreateAudioFmtp(obj);
            }

            return (*this);
        }

        virtual ~Payload() override {}

        inline void CreateAudioFmtp(IN const Payload& obj)
        {
            if (obj.m_pFmtp != IMS_NULL)
            {
                if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("AMR-WB") ||
                        m_objRtpMap.GetPayloadType().EqualsIgnoreCase("AMR"))
                {
                    m_pFmtp = new AudioProfile::AmrFmtp(
                            *static_cast<AudioProfile::AmrFmtp*>(obj.m_pFmtp));
                }
                else if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("EVS"))
                {
                    m_pFmtp = new AudioProfile::EvsFmtp(
                            *static_cast<AudioProfile::EvsFmtp*>(obj.m_pFmtp));
                }
                else if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("telephone-event"))
                {
                    m_pFmtp = new AudioProfile::TelephoneEventFmtp(
                            *static_cast<AudioProfile::TelephoneEventFmtp*>(obj.m_pFmtp));
                }
            }
        }
    };

public:
    class RtcpXrAttributes
    {
    public:
        RtcpXrAttributes() :
                m_bSupportStatisticMetrics(IMS_FALSE),
                m_bSupportVoipMetrics(IMS_FALSE),
                m_bSupportPacketLossRle(IMS_FALSE),
                m_bSupportPacketDuplicatedRle(IMS_FALSE) {};

        RtcpXrAttributes& operator=(const RtcpXrAttributes& p)
        {
            if (this != &p)
            {
                m_bSupportStatisticMetrics = p.m_bSupportStatisticMetrics;
                m_bSupportVoipMetrics = p.m_bSupportVoipMetrics;
                m_bSupportPacketLossRle = p.m_bSupportPacketLossRle;
                m_bSupportPacketDuplicatedRle = p.m_bSupportPacketDuplicatedRle;
            }
            return (*this);
        }

        bool operator==(IN const RtcpXrAttributes& obj) const
        {
            return (m_bSupportStatisticMetrics == obj.m_bSupportStatisticMetrics &&
                    m_bSupportVoipMetrics == obj.m_bSupportVoipMetrics &&
                    m_bSupportPacketLossRle == obj.m_bSupportPacketLossRle &&
                    m_bSupportPacketDuplicatedRle == obj.m_bSupportPacketDuplicatedRle);
        }

        bool operator!=(IN const RtcpXrAttributes& obj) const { return !(*this == obj); }

        inline void SetSupportStatisticMetrics(IN const IMS_BOOL bSupportStatisticMetrics)
        {
            m_bSupportStatisticMetrics = bSupportStatisticMetrics;
        }
        inline IMS_BOOL IsStatisticMetricsSupported() { return m_bSupportStatisticMetrics; }
        inline void SetSupportVoipMetrics(IN const IMS_BOOL bSupportVoipMetrics)
        {
            m_bSupportVoipMetrics = bSupportVoipMetrics;
        }
        inline IMS_BOOL IsVoipMetricsSupported() { return m_bSupportVoipMetrics; }
        inline void SetSupportPacketLossRle(IN const IMS_BOOL bSupportPacketLossRle)
        {
            m_bSupportPacketLossRle = bSupportPacketLossRle;
        }
        inline IMS_BOOL IsPacketLossRleSupported() { return m_bSupportPacketLossRle; }
        inline void SetSupportPacketDuplicatedRle(IN const IMS_BOOL bSupportPacketDuplicatedRle)
        {
            m_bSupportPacketDuplicatedRle = bSupportPacketDuplicatedRle;
        }
        inline IMS_BOOL IsPacketDuplicatedRleSupported() { return m_bSupportPacketDuplicatedRle; }

    private:
        IMS_BOOL m_bSupportStatisticMetrics;
        IMS_BOOL m_bSupportVoipMetrics;
        IMS_BOOL m_bSupportPacketLossRle;
        IMS_BOOL m_bSupportPacketDuplicatedRle;
    };

public:
    enum
    {
        // COMMON PARAMETER
        DEFAULT_PTIME = -1,
        DEFAULT_MAXPTIME = -1,
    };

    AudioProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVP", 0, 0, 0, 0, MEDIA_DIRECTION_SEND_RECEIVE),
            m_nPtime(0),
            m_nMaxPtime(0),
            m_objCandidateAttr(ImsVector<AString>()),
            m_bSupportRtcpXr(IMS_FALSE),
            m_objRtcpXrAttr(RtcpXrAttributes()),
            m_bAnbr(IMS_FALSE) {};

    virtual ~AudioProfile() override {};

    AudioProfile(IN const AudioProfile& obj) :
            MediaBaseProfile(obj),
            m_nPtime(obj.m_nPtime),
            m_nMaxPtime(obj.m_nMaxPtime),
            m_objCandidateAttr(obj.m_objCandidateAttr),
            m_bSupportRtcpXr(obj.m_bSupportRtcpXr),
            m_objRtcpXrAttr(obj.m_objRtcpXrAttr),
            m_bAnbr(obj.m_bAnbr)
    {
    }

    AudioProfile& operator=(IN const AudioProfile& obj)
    {
        if (this != &obj)
        {
            MediaBaseProfile::operator=(obj);
            m_nPtime = obj.m_nPtime;
            m_nMaxPtime = obj.m_nMaxPtime;
            m_objCandidateAttr = obj.m_objCandidateAttr;
            m_bSupportRtcpXr = obj.m_bSupportRtcpXr;
            m_objRtcpXrAttr = obj.m_objRtcpXrAttr;
            m_bAnbr = obj.m_bAnbr;
        }
        return (*this);
    }

    bool operator==(IN const AudioProfile& obj) const
    {
        return (MediaBaseProfile::operator==(obj) && m_nPtime == obj.m_nPtime &&
                m_nMaxPtime == obj.m_nMaxPtime && m_objCandidateAttr == obj.m_objCandidateAttr &&
                m_bSupportRtcpXr == obj.m_bSupportRtcpXr &&
                m_objRtcpXrAttr == obj.m_objRtcpXrAttr && m_bAnbr == obj.m_bAnbr);
    }

    bool operator!=(IN const AudioProfile& obj) const { return !(*this == obj); }

    Payload* GetPayloadAt(IN IMS_UINT32 nIndex) override
    {
        BasePayload* pPayload = MediaBaseProfile::GetPayloadAt(nIndex);
        return (pPayload != IMS_NULL) ? static_cast<Payload*>(pPayload) : IMS_NULL;
    }

    inline void SetPtime(IN const IMS_SINT32 nPtime) { m_nPtime = nPtime; }
    inline IMS_SINT32 GetPtime() { return m_nPtime; }
    inline void SetMaxPtime(IN const IMS_SINT32 nMaxPtime) { m_nMaxPtime = nMaxPtime; }
    inline IMS_SINT32 GetMaxPtime() { return m_nMaxPtime; }
    inline void SetCandidateAttr(IN const ImsVector<AString> objCandidateAttr)
    {
        m_objCandidateAttr = objCandidateAttr;
    }
    inline ImsVector<AString>& GetCandidateAttr() { return m_objCandidateAttr; }
    inline void SetSupportRtcpXr(IN const IMS_BOOL bSupportRtcpXr)
    {
        m_bSupportRtcpXr = bSupportRtcpXr;
    }
    inline IMS_BOOL IsRtcpXrSupported() { return m_bSupportRtcpXr; }
    inline void SetRtcpXrAttr(IN const RtcpXrAttributes objRtcpXrAttr)
    {
        m_objRtcpXrAttr = objRtcpXrAttr;
    }
    inline RtcpXrAttributes& GetRtcpXrAttr() { return m_objRtcpXrAttr; }
    inline void SetAnbr(IN const IMS_BOOL bAnbr) { m_bAnbr = bAnbr; }
    inline IMS_BOOL IsAnbrSupported() { return m_bAnbr; }

private:
    IMS_SINT32 m_nPtime;
    IMS_SINT32 m_nMaxPtime;
    ImsVector<AString> m_objCandidateAttr;
    IMS_BOOL m_bSupportRtcpXr;
    RtcpXrAttributes m_objRtcpXrAttr;
    IMS_BOOL m_bAnbr;
};

#endif
