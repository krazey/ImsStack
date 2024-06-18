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

        IMS_UINT32 nModeSetList;
        IMS_UINT32 nDefaultRtpModeSet;
        IMS_SINT32 nModeChangeCapability;
        IMS_SINT32 nModeChangePeriod;
        IMS_SINT32 nModeChangeNeighbor;
        IMS_SINT32 nMaxRed;
        IMS_SINT32 nPtime;
        IMS_SINT32 nMaxPtime;
        IMS_BOOL bDtx;

        IMS_BOOL bShowModeSet;
        IMS_BOOL bShowModeChangeCapability;
        IMS_BOOL bShowModeChangePeriod;
        IMS_BOOL bShowModeChangeNeighbor;
        IMS_BOOL bShowMaxRed;
        IMS_BOOL bShowPtime;
        IMS_BOOL bShowMaxPtime;
        IMS_BOOL bShowDtx;

    public:
        AudioFmtp() :
                nModeSetList(DEFAULT_MODESETLIST),
                nDefaultRtpModeSet(DEFAULT_MODESETLIST),
                nModeChangeCapability(DEFAULT_MODECHANGE_CAPABILITY),
                nModeChangePeriod(DEFAULT_MODECHANGE_PERIOD),
                nModeChangeNeighbor(DEFAULT_MODECHANGE_NEIGHBOR),
                nMaxRed(DEFAULT_MAXRED),
                nPtime(DEFAULT_PTIME),
                nMaxPtime(DEFAULT_MAXPTIME),
                bDtx(DEFAULT_DTX),
                bShowModeSet(IMS_FALSE),
                bShowModeChangeCapability(IMS_FALSE),
                bShowModeChangePeriod(IMS_FALSE),
                bShowModeChangeNeighbor(IMS_FALSE),
                bShowMaxRed(IMS_FALSE),
                bShowPtime(IMS_FALSE),
                bShowMaxPtime(IMS_FALSE),
                bShowDtx(IMS_FALSE)
        {
        }

        AudioFmtp(IN const AudioFmtp& objFmtp) :
                nModeSetList(objFmtp.nModeSetList),
                nDefaultRtpModeSet(objFmtp.nDefaultRtpModeSet),
                nModeChangeCapability(objFmtp.nModeChangeCapability),
                nModeChangePeriod(objFmtp.nModeChangePeriod),
                nModeChangeNeighbor(objFmtp.nModeChangeNeighbor),
                nMaxRed(objFmtp.nMaxRed),
                nPtime(objFmtp.nPtime),
                nMaxPtime(objFmtp.nMaxPtime),
                bDtx(objFmtp.bDtx),
                bShowModeSet(objFmtp.bShowModeSet),
                bShowModeChangeCapability(objFmtp.bShowModeChangeCapability),
                bShowModeChangePeriod(objFmtp.bShowModeChangePeriod),
                bShowModeChangeNeighbor(objFmtp.bShowModeChangeNeighbor),
                bShowMaxRed(objFmtp.bShowMaxRed),
                bShowPtime(objFmtp.bShowPtime),
                bShowMaxPtime(objFmtp.bShowMaxPtime),
                bShowDtx(objFmtp.bShowDtx)
        {
        }

        virtual ~AudioFmtp(){};
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

        IMS_SINT32 nOctetAlign;
        IMS_BOOL bShowOctetAlign;

        AmrFmtp() :
                AudioFmtp(),
                nOctetAlign(DEFAULT_OCTETALIGN),
                bShowOctetAlign(IMS_FALSE)
        {
        }

        AmrFmtp(IN const AmrFmtp& objFmtp) :
                AudioFmtp(objFmtp),
                nOctetAlign(objFmtp.nOctetAlign),
                bShowOctetAlign(objFmtp.bShowOctetAlign)
        {
        }

        virtual ~AmrFmtp(){};
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

        IMS_UINT32 nHfOnly;
        IMS_UINT32 nEvsModeSwitch;
        IMS_SINT32 nMaxRed;
        IMS_UINT32 nBrList;
        IMS_SINT32 nBrSend;
        IMS_SINT32 nBrRecv;
        IMS_UINT32 nBwList;
        IMS_SINT32 nBwSend;
        IMS_SINT32 nBwRecv;
        IMS_SINT32 nCmr;
        IMS_SINT32 nChAwRecv;
        IMS_SINT32 nReceivedChAwRecv;

        IMS_BOOL bShowHfOnly;
        IMS_BOOL bShowEvsModeSwitch;
        IMS_BOOL bShowCmr;
        IMS_BOOL bShowChannelAwMode;
        IMS_BOOL bShowBrList;
        IMS_BOOL bShowBwList;
        IMS_BOOL bSendCmr;

    public:
        EvsFmtp() :
                nHfOnly(DEFAULT_HFMODE),
                nEvsModeSwitch(DEFAULT_EVSMODESWITCH),
                nBrList(0),
                nBrSend(0),
                nBrRecv(0),
                nBwList(0),
                nBwSend(0),
                nBwRecv(0),
                nCmr(DEFAULT_CMR),
                nChAwRecv(DEFAULT_CHANNEL_AWMODE),
                nReceivedChAwRecv(DEFAULT_CHANNEL_AWMODE),
                bShowHfOnly(IMS_FALSE),
                bShowEvsModeSwitch(IMS_FALSE),
                bShowCmr(IMS_FALSE),
                bShowChannelAwMode(IMS_FALSE),
                bShowBrList(IMS_TRUE),
                bShowBwList(IMS_TRUE),
                bSendCmr(IMS_FALSE)
        {
        }

        EvsFmtp(IN const EvsFmtp& objFmtp) :
                nHfOnly(objFmtp.nHfOnly),
                nEvsModeSwitch(objFmtp.nEvsModeSwitch),
                nBrList(objFmtp.nBrList),
                nBrSend(objFmtp.nBrSend),
                nBrRecv(objFmtp.nBrRecv),
                nBwList(objFmtp.nBwList),
                nBwSend(objFmtp.nBwSend),
                nBwRecv(objFmtp.nBwRecv),
                nCmr(objFmtp.nCmr),
                nChAwRecv(objFmtp.nChAwRecv),
                nReceivedChAwRecv(objFmtp.nReceivedChAwRecv),
                bShowHfOnly(objFmtp.bShowHfOnly),
                bShowEvsModeSwitch(objFmtp.bShowEvsModeSwitch),
                bShowCmr(objFmtp.bShowCmr),
                bShowChannelAwMode(objFmtp.bShowChannelAwMode),
                bShowBrList(objFmtp.bShowBrList),
                bShowBwList(objFmtp.bShowBwList),
                bSendCmr(objFmtp.bSendCmr)
        {
        }

        virtual ~EvsFmtp(){};
    };

public:
    /**
     * TelephoneEventFmtp attributes are used within the SDP to carry TelephoneEvent parameters that
     * provide extra configurations for the specific TelephoneEvent codecs described in the rtpmap.
     */
    class TelephoneEventFmtp : public BaseFmtp
    {
    public:
        AString strEvents;

    public:
        TelephoneEventFmtp() :
                strEvents("0-15"){};

        explicit TelephoneEventFmtp(IN const AString& events) :
                strEvents(events){};

        TelephoneEventFmtp(IN const TelephoneEventFmtp& objFmtp) :
                strEvents(objFmtp.strEvents){};

        virtual ~TelephoneEventFmtp(){};

        TelephoneEventFmtp& operator=(IN const TelephoneEventFmtp& obj)
        {
            if (this != &obj)
            {
                strEvents = obj.strEvents;
            }

            return (*this);
        }

        bool operator==(IN const TelephoneEventFmtp& obj) { return (strEvents == obj.strEvents); }
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
            if (objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB") ||
                    objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
            {
                pFmtp = new AudioProfile::AmrFmtp(*static_cast<AudioProfile::AmrFmtp*>(obj.pFmtp));
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
            {
                pFmtp = new AudioProfile::EvsFmtp(*static_cast<AudioProfile::EvsFmtp*>(obj.pFmtp));
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                pFmtp = new AudioProfile::TelephoneEventFmtp(
                        *static_cast<AudioProfile::TelephoneEventFmtp*>(obj.pFmtp));
            }
        }

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                BasePayload::operator=(obj);

                if (objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB") ||
                        objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
                {
                    pFmtp = new AudioProfile::AmrFmtp(
                            *static_cast<AudioProfile::AmrFmtp*>(obj.pFmtp));
                }
                else if (objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
                {
                    pFmtp = new AudioProfile::EvsFmtp(
                            *static_cast<AudioProfile::EvsFmtp*>(obj.pFmtp));
                }
                else if (objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
                {
                    pFmtp = new AudioProfile::TelephoneEventFmtp(
                            *static_cast<AudioProfile::TelephoneEventFmtp*>(obj.pFmtp));
                }
            }

            return (*this);
        }

        virtual ~Payload() {}
    };

public:
    class RTCPXRAttributes
    {
    public:
        IMS_BOOL bSupportStatisticMetrics;
        IMS_BOOL bSupportVoipMetrics;
        IMS_BOOL bSupportPacketLossRle;
        IMS_BOOL bSupportPacketDuplicatedRle;

    public:
        RTCPXRAttributes() :
                bSupportStatisticMetrics(IMS_FALSE),
                bSupportVoipMetrics(IMS_FALSE),
                bSupportPacketLossRle(IMS_FALSE),
                bSupportPacketDuplicatedRle(IMS_FALSE){};

        RTCPXRAttributes& operator=(const RTCPXRAttributes& p)
        {
            if (this != &p)
            {
                bSupportStatisticMetrics = p.bSupportStatisticMetrics;
                bSupportVoipMetrics = p.bSupportVoipMetrics;
                bSupportPacketLossRle = p.bSupportPacketLossRle;
                bSupportPacketDuplicatedRle = p.bSupportPacketDuplicatedRle;
            }
            return (*this);
        }

        bool operator==(IN const RTCPXRAttributes& obj) const
        {
            return (bSupportStatisticMetrics == obj.bSupportStatisticMetrics &&
                    bSupportVoipMetrics == obj.bSupportVoipMetrics &&
                    bSupportPacketLossRle == obj.bSupportPacketLossRle &&
                    bSupportPacketDuplicatedRle == obj.bSupportPacketDuplicatedRle);
        }

        bool operator!=(IN const RTCPXRAttributes& obj) const
        {
            return (bSupportStatisticMetrics != obj.bSupportStatisticMetrics ||
                    bSupportVoipMetrics != obj.bSupportVoipMetrics ||
                    bSupportPacketLossRle != obj.bSupportPacketLossRle ||
                    bSupportPacketDuplicatedRle != obj.bSupportPacketDuplicatedRle);
        }
    };

public:
    enum
    {
        // COMMON PARAMETER
        DEFAULT_PTIME = 20,
        DEFAULT_MAXPTIME = 240,
    };

    IMS_SINT32 nPtime;
    IMS_SINT32 nMaxPtime;
    ImsVector<AString> objCandidateAttr;
    IMS_BOOL bSupportRtcpXr;
    RTCPXRAttributes objRtcpXrAttr;
    IMS_BOOL bRtcpDisableBeforeSetup;
    IMS_BOOL bAnbr;

public:
    AudioProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVP", 0, 0, 0, 0, MEDIA_DIRECTION_SEND_RECEIVE),
            nPtime(0),
            nMaxPtime(0),
            objCandidateAttr(ImsVector<AString>()),
            bSupportRtcpXr(IMS_FALSE),
            objRtcpXrAttr(RTCPXRAttributes()),
            bRtcpDisableBeforeSetup(IMS_FALSE),
            bAnbr(IMS_FALSE){};

    virtual ~AudioProfile(){};

    AudioProfile(IN AudioProfile* profile) :
            MediaBaseProfile(profile)
    {
        if (profile == nullptr)
        {
            return;
        }
        nPtime = profile->nPtime;
        nMaxPtime = profile->nMaxPtime;
        objCandidateAttr = profile->objCandidateAttr;
        bSupportRtcpXr = profile->bSupportRtcpXr;
        objRtcpXrAttr = profile->objRtcpXrAttr;
        bRtcpDisableBeforeSetup = profile->bRtcpDisableBeforeSetup;
        bAnbr = profile->bAnbr;
    }

    AudioProfile(IN const AudioProfile& obj) :
            MediaBaseProfile(obj)
    {
        nPtime = obj.nPtime;
        nMaxPtime = obj.nMaxPtime;
        objCandidateAttr = obj.objCandidateAttr;
        bSupportRtcpXr = obj.bSupportRtcpXr;
        objRtcpXrAttr = obj.objRtcpXrAttr;
        bRtcpDisableBeforeSetup = obj.bRtcpDisableBeforeSetup;
        bAnbr = obj.bAnbr;
    }

    AudioProfile& operator=(IN const AudioProfile& obj)
    {
        if (this != &obj)
        {
            MediaBaseProfile::operator=(obj);
            nPtime = obj.nPtime;
            nMaxPtime = obj.nMaxPtime;
            objCandidateAttr = obj.objCandidateAttr;
            bSupportRtcpXr = obj.bSupportRtcpXr;
            objRtcpXrAttr = obj.objRtcpXrAttr;
            bRtcpDisableBeforeSetup = obj.bRtcpDisableBeforeSetup;
            bAnbr = obj.bAnbr;
        }
        return (*this);
    }

    bool operator==(IN const AudioProfile& obj) const
    {
        return (MediaBaseProfile::operator==(obj) && bSupportRtcpXr == obj.bSupportRtcpXr &&
                objRtcpXrAttr == obj.objRtcpXrAttr &&
                bRtcpDisableBeforeSetup == obj.bRtcpDisableBeforeSetup && bAnbr == obj.bAnbr);
    }

    bool operator!=(IN const AudioProfile& obj) const
    {
        return (MediaBaseProfile::operator!=(obj) || bSupportRtcpXr != obj.bSupportRtcpXr ||
                objRtcpXrAttr != obj.objRtcpXrAttr ||
                bRtcpDisableBeforeSetup != obj.bRtcpDisableBeforeSetup || bAnbr != obj.bAnbr);
    }

    Payload* GetPayloadAt(IN IMS_UINT32 nIndex) override
    {
        BasePayload* pPayload = MediaBaseProfile::GetPayloadAt(nIndex);
        return (pPayload != IMS_NULL) ? static_cast<Payload*>(pPayload) : IMS_NULL;
    }
};

#endif
