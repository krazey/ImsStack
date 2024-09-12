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

#ifndef AUDIO_PROFILE_H_
#define AUDIO_PROFILE_H_

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
        enum
        {
            DEFAULT_MODESETLIST = 0,
            DEFAULT_MODECHANGE_CAPABILITY = 1,
            DEFAULT_MODECHANGE_PERIOD = 1,
            DEFAULT_MODECHANGE_NEIGHBOR = 0,
            DEFAULT_MAXRED = -1,
            DEFAULT_PTIME = -1,
            DEFAULT_MAXPTIME = -1,
            DEFAULT_DTX = 1,
        };

    public:
        AudioFmtp() :
                m_nModeSetList(DEFAULT_MODESETLIST),
                m_nDefaultRtpModeSet(DEFAULT_MODESETLIST),
                m_nModeChangeCapability(DEFAULT_MODECHANGE_CAPABILITY),
                m_nModeChangePeriod(DEFAULT_MODECHANGE_PERIOD),
                m_nModeChangeNeighbor(DEFAULT_MODECHANGE_NEIGHBOR),
                m_nMaxRed(DEFAULT_MAXRED),
                m_nPtime(DEFAULT_PTIME),
                m_nMaxPtime(DEFAULT_MAXPTIME),
                m_bDtx(DEFAULT_DTX),
                m_bShowModeSet(IMS_FALSE),
                m_bShowModeChangeCapability(IMS_FALSE),
                m_bShowModeChangePeriod(IMS_FALSE),
                m_bShowModeChangeNeighbor(IMS_FALSE),
                m_bShowMaxRed(IMS_FALSE),
                m_bShowPtime(IMS_FALSE),
                m_bShowMaxPtime(IMS_FALSE),
                m_bShowDtx(IMS_FALSE)
        {
        }

        AudioFmtp(IN const AudioFmtp& objFmtp) :
                m_nModeSetList(objFmtp.m_nModeSetList),
                m_nDefaultRtpModeSet(objFmtp.m_nDefaultRtpModeSet),
                m_nModeChangeCapability(objFmtp.m_nModeChangeCapability),
                m_nModeChangePeriod(objFmtp.m_nModeChangePeriod),
                m_nModeChangeNeighbor(objFmtp.m_nModeChangeNeighbor),
                m_nMaxRed(objFmtp.m_nMaxRed),
                m_nPtime(objFmtp.m_nPtime),
                m_nMaxPtime(objFmtp.m_nMaxPtime),
                m_bDtx(objFmtp.m_bDtx),
                m_bShowModeSet(objFmtp.m_bShowModeSet),
                m_bShowModeChangeCapability(objFmtp.m_bShowModeChangeCapability),
                m_bShowModeChangePeriod(objFmtp.m_bShowModeChangePeriod),
                m_bShowModeChangeNeighbor(objFmtp.m_bShowModeChangeNeighbor),
                m_bShowMaxRed(objFmtp.m_bShowMaxRed),
                m_bShowPtime(objFmtp.m_bShowPtime),
                m_bShowMaxPtime(objFmtp.m_bShowMaxPtime),
                m_bShowDtx(objFmtp.m_bShowDtx)
        {
        }

        virtual ~AudioFmtp(){};

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
        inline void SetPtime(IN const IMS_SINT32 nPtime) { m_nPtime = nPtime; }
        inline IMS_SINT32 GetPtime() { return m_nPtime; }
        inline void SetMaxPtime(IN const IMS_SINT32 nMaxPtime) { m_nMaxPtime = nMaxPtime; }
        inline IMS_SINT32 GetMaxPtime() { return m_nMaxPtime; }
        inline void SetDtx(IN const IMS_BOOL bDtx) { m_bDtx = bDtx; }
        inline IMS_BOOL IsDtxEnabled() { return m_bDtx; }
        inline void SetShowModeSet(IN const IMS_BOOL bShowModeSet)
        {
            m_bShowModeSet = bShowModeSet;
        }
        inline IMS_BOOL IsModeSetVisible() { return m_bShowModeSet; }
        inline void SetShowModeChangeCapability(IN const IMS_BOOL bShowModeChangeCapability)
        {
            m_bShowModeChangeCapability = bShowModeChangeCapability;
        }
        inline IMS_BOOL IsModeChangeCapabilityVisible() { return m_bShowModeChangeCapability; }
        inline void SetShowModeChangePeriod(IN const IMS_BOOL bShowModeChangePeriod)
        {
            m_bShowModeChangePeriod = bShowModeChangePeriod;
        }
        inline IMS_BOOL IsModeChangePeriodVisible() { return m_bShowModeChangePeriod; }
        inline void SetShowModeChangeNeighbor(IN const IMS_BOOL bShowModeChangeNeighbor)
        {
            m_bShowModeChangeNeighbor = bShowModeChangeNeighbor;
        }
        inline IMS_BOOL IsModeChangeNeighborVisible() { return m_bShowModeChangeNeighbor; }
        inline void SetShowMaxRed(IN const IMS_BOOL bShowMaxRed) { m_bShowMaxRed = bShowMaxRed; }
        inline IMS_BOOL IsMaxRedVisible() { return m_bShowMaxRed; }
        inline void SetShowPtime(IN const IMS_BOOL bShowPtime) { m_bShowPtime = bShowPtime; }
        inline IMS_BOOL IsPtimeVisible() { return m_bShowPtime; }
        inline void SetShowMaxPtime(IN const IMS_BOOL bShowMaxPtime)
        {
            m_bShowMaxPtime = bShowMaxPtime;
        }
        inline IMS_BOOL IsMaxPtimeVisible() { return m_bShowMaxPtime; }
        inline void SetShowDtx(IN const IMS_BOOL bShowDtx) { m_bShowDtx = bShowDtx; }
        inline IMS_BOOL IsDtxVisible() { return m_bShowDtx; }

    private:
        IMS_UINT32 m_nModeSetList;
        IMS_UINT32 m_nDefaultRtpModeSet;
        IMS_SINT32 m_nModeChangeCapability;
        IMS_SINT32 m_nModeChangePeriod;
        IMS_SINT32 m_nModeChangeNeighbor;
        IMS_SINT32 m_nMaxRed;
        IMS_SINT32 m_nPtime;
        IMS_SINT32 m_nMaxPtime;
        IMS_BOOL m_bDtx;
        IMS_BOOL m_bShowModeSet;
        IMS_BOOL m_bShowModeChangeCapability;
        IMS_BOOL m_bShowModeChangePeriod;
        IMS_BOOL m_bShowModeChangeNeighbor;
        IMS_BOOL m_bShowMaxRed;
        IMS_BOOL m_bShowPtime;
        IMS_BOOL m_bShowMaxPtime;
        IMS_BOOL m_bShowDtx;
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
                m_bShowOctetAlign(IMS_FALSE)
        {
        }

        AmrFmtp(IN const AmrFmtp& objFmtp) :
                AudioFmtp(objFmtp),
                m_nOctetAlign(objFmtp.m_nOctetAlign),
                m_bShowOctetAlign(objFmtp.m_bShowOctetAlign)
        {
        }

        virtual ~AmrFmtp(){};

        inline void SetOctetAlign(IN const IMS_SINT32 nOctetAlign) { m_nOctetAlign = nOctetAlign; }
        inline IMS_SINT32 GetOctetAlign() { return m_nOctetAlign; }
        inline void SetShowOctetAlign(IN const IMS_BOOL bShowOctetAlign)
        {
            m_bShowOctetAlign = bShowOctetAlign;
        }
        inline IMS_BOOL IsOctetAlignVisible() { return m_bShowOctetAlign; }

    private:
        IMS_SINT32 m_nOctetAlign;
        IMS_BOOL m_bShowOctetAlign;
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
                m_bShowHfOnly(IMS_FALSE),
                m_bShowEvsModeSwitch(IMS_FALSE),
                m_bShowCmr(IMS_FALSE),
                m_bShowChannelAwMode(IMS_FALSE),
                m_bShowBrList(IMS_TRUE),
                m_bShowBwList(IMS_TRUE),
                m_bSendCmr(IMS_FALSE)
        {
        }

        EvsFmtp(IN const EvsFmtp& objFmtp) :
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
                m_bShowHfOnly(objFmtp.m_bShowHfOnly),
                m_bShowEvsModeSwitch(objFmtp.m_bShowEvsModeSwitch),
                m_bShowCmr(objFmtp.m_bShowCmr),
                m_bShowChannelAwMode(objFmtp.m_bShowChannelAwMode),
                m_bShowBrList(objFmtp.m_bShowBrList),
                m_bShowBwList(objFmtp.m_bShowBwList),
                m_bSendCmr(objFmtp.m_bSendCmr)
        {
        }

        virtual ~EvsFmtp(){};

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
        inline void SetShowHfOnly(IN const IMS_BOOL bShowHfOnly) { m_bShowHfOnly = bShowHfOnly; }
        inline IMS_BOOL IsHfOnlyVisible() { return m_bShowHfOnly; }
        inline void SetShowEvsModeSwitch(IN const IMS_BOOL bShowEvsModeSwitch)
        {
            m_bShowEvsModeSwitch = bShowEvsModeSwitch;
        }
        inline IMS_BOOL IsEvsModeSwitchVisible() { return m_bShowEvsModeSwitch; }
        inline void SetShowCmr(IN const IMS_BOOL bShowCmr) { m_bShowCmr = bShowCmr; }
        inline IMS_BOOL IsCmrVisible() { return m_bShowCmr; }
        inline void SetShowChannelAwMode(IN const IMS_BOOL bShowChannelAwMode)
        {
            m_bShowChannelAwMode = bShowChannelAwMode;
        }
        inline IMS_BOOL IsChannelAwModeVisible() { return m_bShowChannelAwMode; }
        inline void SetShowBrList(IN const IMS_BOOL bShowBrList) { m_bShowBrList = bShowBrList; }
        inline IMS_BOOL IsBrListVisible() { return m_bShowBrList; }
        inline void SetShowBwList(IN const IMS_BOOL bShowBwList) { m_bShowBwList = bShowBwList; }
        inline IMS_BOOL IsBwListVisible() { return m_bShowBwList; }
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
        IMS_BOOL m_bShowHfOnly;
        IMS_BOOL m_bShowEvsModeSwitch;
        IMS_BOOL m_bShowCmr;
        IMS_BOOL m_bShowChannelAwMode;
        IMS_BOOL m_bShowBrList;
        IMS_BOOL m_bShowBwList;
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
                m_strEvents("0-15"){};

        explicit TelephoneEventFmtp(IN const AString& events) :
                m_strEvents(events){};

        TelephoneEventFmtp(IN const TelephoneEventFmtp& objFmtp) :
                m_strEvents(objFmtp.m_strEvents){};

        virtual ~TelephoneEventFmtp(){};

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

        bool operator!=(IN const TelephoneEventFmtp& obj) const
        {
            return (m_strEvents != obj.m_strEvents);
        }

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
                BasePayload(1){};
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

        virtual ~Payload() {}

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
                m_bSupportPacketDuplicatedRle(IMS_FALSE){};

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

        bool operator!=(IN const RtcpXrAttributes& obj) const
        {
            return (m_bSupportStatisticMetrics != obj.m_bSupportStatisticMetrics ||
                    m_bSupportVoipMetrics != obj.m_bSupportVoipMetrics ||
                    m_bSupportPacketLossRle != obj.m_bSupportPacketLossRle ||
                    m_bSupportPacketDuplicatedRle != obj.m_bSupportPacketDuplicatedRle);
        }

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
        DEFAULT_PTIME = 20,
        DEFAULT_MAXPTIME = 240,
    };

    AudioProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVP", 0, 0, 0, 0, MEDIA_DIRECTION_SEND_RECEIVE),
            m_nPtime(0),
            m_nMaxPtime(0),
            m_objCandidateAttr(ImsVector<AString>()),
            m_bSupportRtcpXr(IMS_FALSE),
            m_objRtcpXrAttr(RtcpXrAttributes()),
            m_bAnbr(IMS_FALSE){};

    virtual ~AudioProfile(){};

    AudioProfile(IN const AudioProfile& obj) :
            MediaBaseProfile(obj)
    {
        m_nPtime = obj.m_nPtime;
        m_nMaxPtime = obj.m_nMaxPtime;
        m_objCandidateAttr = obj.m_objCandidateAttr;
        m_bSupportRtcpXr = obj.m_bSupportRtcpXr;
        m_objRtcpXrAttr = obj.m_objRtcpXrAttr;
        m_bAnbr = obj.m_bAnbr;
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
        return (MediaBaseProfile::operator==(obj) && m_bSupportRtcpXr == obj.m_bSupportRtcpXr &&
                m_objRtcpXrAttr == obj.m_objRtcpXrAttr && m_bAnbr == obj.m_bAnbr);
    }

    bool operator!=(IN const AudioProfile& obj) const
    {
        return (MediaBaseProfile::operator!=(obj) || m_bSupportRtcpXr != obj.m_bSupportRtcpXr ||
                m_objRtcpXrAttr != obj.m_objRtcpXrAttr || m_bAnbr != obj.m_bAnbr);
    }

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
