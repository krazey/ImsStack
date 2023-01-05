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

#ifndef _IMS_AUDIO_NEGO_PROFILE_H_
#define _IMS_AUDIO_NEGO_PROFILE_H_

#include "IMSTypeDef.h"
#include "IpAddress.h"
#include "ImsMap.h"

class AudioProfile
{
public:
    class RtpMap
    {
    public:
        IMS_UINT32 nPayloadNum;
        AString strPayloadType;
        IMS_UINT32 nSamplingRate;
        IMS_SINT32 nChannel;  // default is 1, if this value is -1 hide channel value at SDP
    public:
        RtpMap() :
                nPayloadNum(0),
                strPayloadType(AString::ConstNull()),
                nSamplingRate(0),
                nChannel(1)
        {
        }

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
    class AmrFmtp
    {
    public:
        IMS_UINT32 nModeSetList;
        IMS_UINT32 nDefaultRtpModeSet;
        IMS_BOOL bSCREnable;

        enum
        {
            DEFAULT_OCTCTALIGN = 0,
            DEFAULT_MODECHANGE_CAPABILITY = 1,
            DEFAULT_MODECHANGE_PERIOD = 1,
            DEFAULT_MODECHANGE_NEIGHBOR = 0,
            DEFAULT_MAXRED = -1,
            DEFAULT_PTIME = -1,
            DEFAULT_MAXPTIME = -1,
        };

        // If each field has non-default value or bShow_xxx is TRUE it will be included at SDP.
        IMS_SINT32 nOctetAlign;
        IMS_SINT32 nModeChangeCapability;
        IMS_SINT32 nModeChangePeriod;
        IMS_SINT32 nModeChangeNeighbor;

        IMS_SINT32 nMaxRed;
        IMS_SINT32 nPtime;
        IMS_SINT32 nMaxPtime;

        IMS_BOOL bShow_OctetAlign;
        IMS_BOOL bShowModeChangeCapability;
        IMS_BOOL bShowModeChangePeriod;
        IMS_BOOL bShowModeChangeNeighbor;
        IMS_BOOL bShow_RobustSorting;

        IMS_BOOL bShowMaxRed;
        IMS_BOOL bShowPtime;
        IMS_BOOL bShowMaxPtime;
        IMS_BOOL bShowModeSet;

    public:
        AmrFmtp() :
                nModeSetList(0),
                nDefaultRtpModeSet(0),
                bSCREnable(IMS_FALSE),
                nOctetAlign(DEFAULT_OCTCTALIGN),
                nModeChangeCapability(DEFAULT_MODECHANGE_CAPABILITY),
                nModeChangePeriod(DEFAULT_MODECHANGE_PERIOD),
                nModeChangeNeighbor(DEFAULT_MODECHANGE_NEIGHBOR),
                nMaxRed(DEFAULT_MAXRED),
                nPtime(DEFAULT_PTIME),
                nMaxPtime(DEFAULT_MAXPTIME),
                bShow_OctetAlign(IMS_FALSE),
                bShowModeChangeCapability(IMS_FALSE),
                bShowModeChangePeriod(IMS_FALSE),
                bShowModeChangeNeighbor(IMS_FALSE),
                bShow_RobustSorting(IMS_FALSE),
                bShowMaxRed(IMS_FALSE),
                bShowPtime(IMS_FALSE),
                bShowMaxPtime(IMS_FALSE),
                bShowModeSet(IMS_FALSE)
        {
        }
        AmrFmtp(IN const AmrFmtp& objFmtp)
        {
            this->nModeSetList = objFmtp.nModeSetList;
            this->nDefaultRtpModeSet = objFmtp.nDefaultRtpModeSet;
            this->bSCREnable = objFmtp.bSCREnable;
            this->nOctetAlign = objFmtp.nOctetAlign;
            this->nModeChangeCapability = objFmtp.nModeChangeCapability;
            this->nModeChangePeriod = objFmtp.nModeChangePeriod;
            this->nModeChangeNeighbor = objFmtp.nModeChangeNeighbor;

            this->nMaxRed = objFmtp.nMaxRed;
            this->nPtime = objFmtp.nPtime;
            this->nMaxPtime = objFmtp.nMaxPtime;

            this->bShow_OctetAlign = objFmtp.bShow_OctetAlign;
            this->bShowModeChangeCapability = objFmtp.bShowModeChangeCapability;
            this->bShowModeChangePeriod = objFmtp.bShowModeChangePeriod;
            this->bShowModeChangeNeighbor = objFmtp.bShowModeChangeNeighbor;
            this->bShow_RobustSorting = objFmtp.bShow_RobustSorting;

            this->bShowMaxRed = objFmtp.bShowMaxRed;
            this->bShowPtime = objFmtp.bShowPtime;
            this->bShowMaxPtime = objFmtp.bShowMaxPtime;
            this->bShowModeSet = objFmtp.bShowModeSet;
        };

        AmrFmtp(IN const IMS_UINT32 modeSet, IN const IMS_SINT32 octetAlign,
                IN const IMS_SINT32 modeChangeCapability) :
                nModeSetList(modeSet),
                nDefaultRtpModeSet(modeSet),
                bSCREnable(IMS_TRUE),
                nOctetAlign(octetAlign),
                nModeChangeCapability(modeChangeCapability),
                nModeChangePeriod(DEFAULT_MODECHANGE_PERIOD),
                nModeChangeNeighbor(DEFAULT_MODECHANGE_NEIGHBOR),
                nMaxRed(DEFAULT_MAXRED),
                nPtime(DEFAULT_PTIME),
                nMaxPtime(DEFAULT_MAXPTIME),
                bShowModeChangePeriod(IMS_FALSE),
                bShowModeChangeNeighbor(IMS_FALSE),
                bShow_RobustSorting(IMS_FALSE),
                bShowMaxRed(IMS_FALSE),
                bShowPtime(IMS_FALSE),
                bShowMaxPtime(IMS_FALSE),
                bShowModeSet(IMS_FALSE)
        {
            if (octetAlign > 0)
            {
                bShow_OctetAlign = IMS_TRUE;
            }
            else
            {
                bShow_OctetAlign = IMS_FALSE;
            }

            if (modeChangeCapability > 0)
            {
                bShowModeChangeCapability = IMS_TRUE;
            }
            else
            {
                bShowModeChangeCapability = IMS_FALSE;
            }
        };
    };

public:
    class EvsFmtp
    {
    public:
        enum
        {
            // COMMON PARAMETER
            DEFAULT_PTIME = -1,
            DEFAULT_MAXPTIME = -1,
            DEFAULT_DTX = 1,
            DEFAULT_DTXRECV = 1,
            DEFAULT_HFMODE = 0,
            DEFAULT_EVSMODESWITCH = 0,
            DEFAULT_MAXRED = -1,
            DEFAULT_BANDWIDTHLIST = 0,
            DEFAULT_BITRATELIST = 0,
            // PRIMARY PARAMETER
            // DEFAULT_BITRATE = -1,
            DEFAULT_BANDWIDTH = -1,
            DEFAULT_CMR = 0,
            DEFAULT_CHANNEL_AWMODE = 0,
            // AMR-WB IO PARAMETER
            DEFAULT_MODESETLIST = 0,
            DEFAULT_RTPMODESET = 0,
            DEFAULT_MODECHANGE_CAPABILITY = 1,
            DEFAULT_MODECHANGE_PERIOD = 1,
            DEFAULT_MODECHANGE_NEIGHBOR = 0
        };

        IMS_SINT32 nPtime;
        IMS_SINT32 nMaxPtime;
        /** 1(default) is turn on DTX */
        IMS_UINT32 nDtx;
        /** 1(default) is dependent on DTX */
        IMS_UINT32 nDtx_Recv;
        /** 0(default) is compact and hf format used, other is only hf format used */
        IMS_UINT32 nHfOnly;
        /** 0(default) is "primary mode start" */
        IMS_UINT32 nEvsModeSwitch;
        IMS_SINT32 nMaxRed;
        /** EVS primary mode bitrate range (kbps) */
        IMS_UINT32 nBrList;
        /** EVS primary mode bitrate range (kbps), only send direction (used at sendrecv/sendonly
         direction) */
        IMS_SINT32 nBrSend;
        /** EVS primary mode bitrate range (kbps), only recv direction (used at sendrecv/recvonly
        direction) */
        IMS_SINT32 nBrRecv;
        /** bw has a value from the set : nb, wb, swb, fb, nb-wb, nb-swb, and nb-fb. nb, wb, swb,
         * fb*/
        IMS_UINT32 nBwList;
        IMS_SINT32 nBwSend;
        IMS_SINT32 nBwRecv;
        IMS_SINT32 nCmr;
        /** -1 is channel aware mode disable, 0(default) is not used at the start of the session,
         * but it'll be changed using CMR or RTCP app. */
        IMS_SINT32 nChAwRecv;
        /** -1 is channel aware mode disable, 0(default) is not used at the start of the session,
         * but it'll be changed using CMR or RTCP app. */
        IMS_SINT32 nReceivedChAwRecv;
        // AMR-WB IO parameter
        IMS_UINT32 nModeSetList;
        IMS_UINT32 nDefaultRtpModeSet;
        IMS_SINT32 nModeChangeCapability;
        IMS_SINT32 nModeChangePeriod;
        IMS_SINT32 nModeChangeNeighbor;
        // showable check variable
        IMS_BOOL bShowPtime;
        IMS_BOOL bShowMaxPtime;
        IMS_BOOL bShowDtx;
        IMS_BOOL bShowHfOnly;
        IMS_BOOL bShowEvsModeSwitch;
        IMS_BOOL bShowMaxRed;
        IMS_BOOL bShowCmr;
        IMS_BOOL bShowChannelAwMode;
        IMS_BOOL bShowModeChangeCapability;
        IMS_BOOL bShowModeChangePeriod;
        IMS_BOOL bShowModeChangeNeighbor;
        IMS_BOOL bShowBrList;
        IMS_BOOL bShowBwList;
        IMS_BOOL bSendCmr;          // send cmr option
        IMS_BOOL bShowModeSetList;  // send ModeSetList option

    public:
        EvsFmtp() :
                nPtime(DEFAULT_PTIME),
                nMaxPtime(DEFAULT_MAXPTIME),
                nDtx(DEFAULT_DTX),
                nDtx_Recv(DEFAULT_DTXRECV),
                nHfOnly(DEFAULT_HFMODE),
                nEvsModeSwitch(DEFAULT_EVSMODESWITCH),
                nMaxRed(DEFAULT_MAXRED),
                nBrList(0),
                nBrSend(0),
                nBrRecv(0),
                nBwList(0),
                nBwSend(0),
                nBwRecv(0),
                nCmr(DEFAULT_CMR),
                nChAwRecv(DEFAULT_CHANNEL_AWMODE),
                nReceivedChAwRecv(DEFAULT_CHANNEL_AWMODE),
                nModeSetList(DEFAULT_MODESETLIST),
                nDefaultRtpModeSet(DEFAULT_MODESETLIST),
                nModeChangeCapability(DEFAULT_MODECHANGE_CAPABILITY),
                nModeChangePeriod(DEFAULT_MODECHANGE_PERIOD),
                nModeChangeNeighbor(DEFAULT_MODECHANGE_NEIGHBOR),
                bShowPtime(IMS_FALSE),
                bShowMaxPtime(IMS_FALSE),
                bShowDtx(IMS_FALSE),
                bShowHfOnly(IMS_FALSE),
                bShowEvsModeSwitch(IMS_FALSE),
                bShowMaxRed(IMS_FALSE),
                bShowCmr(IMS_FALSE),
                bShowChannelAwMode(IMS_FALSE),
                bShowModeChangeCapability(IMS_FALSE),
                bShowModeChangePeriod(IMS_FALSE),
                bShowModeChangeNeighbor(IMS_FALSE),
                bShowBrList(IMS_TRUE),
                bShowBwList(IMS_TRUE),
                bSendCmr(IMS_FALSE),
                bShowModeSetList(IMS_FALSE)
        {
        }

        EvsFmtp(IN const EvsFmtp& objFmtp)
        {
            this->nPtime = objFmtp.nPtime;
            this->nMaxPtime = objFmtp.nMaxPtime;
            this->nDtx = objFmtp.nDtx;
            this->nDtx_Recv = objFmtp.nDtx_Recv;
            this->nHfOnly = objFmtp.nHfOnly;
            this->nEvsModeSwitch = objFmtp.nEvsModeSwitch;
            this->nMaxRed = objFmtp.nMaxRed;
            this->nBrList = objFmtp.nBrList;
            this->nBrSend = objFmtp.nBrSend;
            this->nBrRecv = objFmtp.nBrRecv;
            this->nBwList = objFmtp.nBwList;
            this->nBwSend = objFmtp.nBwSend;
            this->nBwRecv = objFmtp.nBwRecv;
            this->nCmr = objFmtp.nCmr;
            this->nChAwRecv = objFmtp.nChAwRecv;
            this->nReceivedChAwRecv = objFmtp.nReceivedChAwRecv;
            this->nModeSetList = objFmtp.nModeSetList;
            this->nDefaultRtpModeSet = objFmtp.nDefaultRtpModeSet;
            this->nModeChangeCapability = objFmtp.nModeChangeCapability;
            this->nModeChangePeriod = objFmtp.nModeChangePeriod;
            this->nModeChangeNeighbor = objFmtp.nModeChangeNeighbor;
            this->bShowPtime = objFmtp.bShowPtime;
            this->bShowMaxPtime = objFmtp.bShowMaxPtime;
            this->bShowDtx = objFmtp.bShowDtx;
            this->bShowHfOnly = objFmtp.bShowHfOnly;
            this->bShowEvsModeSwitch = objFmtp.bShowEvsModeSwitch;
            this->bShowMaxRed = objFmtp.bShowMaxRed;
            this->bShowCmr = objFmtp.bShowCmr;
            this->bShowChannelAwMode = objFmtp.bShowChannelAwMode;
            this->bShowModeChangeCapability = objFmtp.bShowModeChangeCapability;
            this->bShowModeChangePeriod = objFmtp.bShowModeChangePeriod;
            this->bShowModeChangeNeighbor = objFmtp.bShowModeChangeNeighbor;
            this->bShowBrList = objFmtp.bShowBrList;
            this->bShowBwList = objFmtp.bShowBwList;
            this->bSendCmr = objFmtp.bSendCmr;
            this->bShowModeSetList = objFmtp.bShowModeSetList;
        }
    };

public:
    class TelephoneEventFmtp
    {
    public:
        AString strEvents;

    public:
        TelephoneEventFmtp() :
                strEvents("0-15"){};

        explicit TelephoneEventFmtp(IN AString events) :
                strEvents(events){};

        TelephoneEventFmtp(IN const TelephoneEventFmtp& objFmtp)
        {
            this->strEvents = objFmtp.strEvents;
        };

        TelephoneEventFmtp& operator=(IN const TelephoneEventFmtp& obj)
        {
            this->strEvents = obj.strEvents;
            return *this;
        }

        bool operator==(IN const TelephoneEventFmtp& obj)
        {
            return (this->strEvents == obj.strEvents);
        }
    };

public:
    class Payload
    {
    public:
        RtpMap objRtpMap;
        void* pFmtp;

    public:
        Payload() :
                pFmtp(IMS_NULL){};
        Payload(IN const Payload& obj)
        {
            this->objRtpMap = obj.objRtpMap;

            if (objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB") ||
                    objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
            {
                pFmtp = new AudioProfile::AmrFmtp(
                        *reinterpret_cast<AudioProfile::AmrFmtp*>(obj.pFmtp));
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
            {
                pFmtp = new AudioProfile::EvsFmtp(
                        *reinterpret_cast<AudioProfile::EvsFmtp*>(obj.pFmtp));
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                pFmtp = new AudioProfile::TelephoneEventFmtp(
                        *reinterpret_cast<AudioProfile::TelephoneEventFmtp*>(obj.pFmtp));
            }
        }

        ~Payload()
        {
            if (this->pFmtp == IMS_NULL)
            {
                return;
            }

            if (objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB") ||
                    objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
            {
                delete reinterpret_cast<AudioProfile::AmrFmtp*>(this->pFmtp);
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
            {
                delete reinterpret_cast<AudioProfile::EvsFmtp*>(this->pFmtp);
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                delete reinterpret_cast<AudioProfile::TelephoneEventFmtp*>(this->pFmtp);
            }
        }

    public:
        void SetRtpMap(IN const IMS_UINT32 nPayloadNum, IN const AString strPayloadType,
                IN const IMS_UINT32 nSamplingRate, IN const IMS_SINT32 nChannel)
        {
            objRtpMap.nPayloadNum = nPayloadNum;
            objRtpMap.strPayloadType = strPayloadType;
            objRtpMap.nSamplingRate = nSamplingRate;
            objRtpMap.nChannel = nChannel;
        }

        void SetRtpMap(IN const RtpMap& objMap)
        {
            objRtpMap.nPayloadNum = objMap.nPayloadNum;
            objRtpMap.strPayloadType = objMap.strPayloadType;
            objRtpMap.nSamplingRate = objMap.nSamplingRate;
            objRtpMap.nChannel = objMap.nChannel;
        };
    };

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

        CapaNego(const CapaNego& obj)
        {
            this->mapTransportCapa = obj.mapTransportCapa;
            this->mapAttributeCapa = obj.mapAttributeCapa;
            this->lstPotentialConfig = obj.lstPotentialConfig;
            this->strNegotiatedAcfg = obj.strNegotiatedAcfg;
            this->bIsAttCapaInPcfg = obj.bIsAttCapaInPcfg;
        }
    };

public:
    class RTCPXRAttributes
    {
    public:
        IMS_BOOL bSupportStatisticMetrics;
        IMS_BOOL bSupportVoipMatircs;
        IMS_BOOL bSupportPacketLossRle;
        IMS_BOOL bSupportPacketDuplicatedRle;

    public:
        RTCPXRAttributes() :
                bSupportStatisticMetrics(IMS_FALSE),
                bSupportVoipMatircs(IMS_FALSE),
                bSupportPacketLossRle(IMS_FALSE),
                bSupportPacketDuplicatedRle(IMS_FALSE){};

        RTCPXRAttributes& operator=(const RTCPXRAttributes& p)
        {
            bSupportStatisticMetrics = p.bSupportStatisticMetrics;
            bSupportVoipMatircs = p.bSupportVoipMatircs;
            bSupportPacketLossRle = p.bSupportPacketLossRle;
            bSupportPacketDuplicatedRle = p.bSupportPacketDuplicatedRle;
            return *this;
        }

        bool operator==(IN const RTCPXRAttributes& obj) const
        {
            return (bSupportStatisticMetrics == obj.bSupportStatisticMetrics &&
                    bSupportVoipMatircs == obj.bSupportVoipMatircs &&
                    bSupportPacketLossRle == obj.bSupportPacketLossRle &&
                    bSupportPacketDuplicatedRle == obj.bSupportPacketDuplicatedRle);
        }
    };

public:
    enum
    {
        // COMMON PARAMETER
        DEFAULT_PTIME = 20,
        DEFAULT_MAXPTIME = 240,
    };

    IPAddress objIpAddr;
    IMS_UINT32 nDataPort;
    IMS_UINT32 nControlPort;
    AString strTransportType;
    IMS_UINT32 nRtcpInterval;
    IMS_SINT32 nBandwidthAs;
    IMS_SINT32 nBandwidthRs;
    IMS_SINT32 nBandwidthRr;
    IMSList<Payload*> lstPayload;
    MEDIA_DIRECTION eDirection;
    IMS_SINT32 nPtime;
    IMS_SINT32 nMaxPtime;
    IMSVector<AString> objCandidateAttr;
    IMS_SINT32 nNegotiatedPayloadIndex;
    IMS_BOOL bIsOfferCase;
    CapaNego objCapaNego;
    IMS_BOOL bSupportRtcpXr;
    RTCPXRAttributes objRtcpXrAttr;
    IMS_BOOL bRtcpDisableBeforeSetup;

public:
    AudioProfile() :
            objIpAddr(IPAddress::IPv6NONE),
            nDataPort(0),
            nControlPort(0),
            strTransportType("RTP/AVP"),
            nRtcpInterval(0),
            nBandwidthAs(0),
            nBandwidthRs(0),
            nBandwidthRr(0),
            lstPayload(IMSList<Payload*>()),
            eDirection(MEDIA_DIRECTION_SEND_RECEIVE),
            nPtime(0),
            nMaxPtime(0),
            objCandidateAttr(IMSVector<AString>()),
            nNegotiatedPayloadIndex(-1),
            bIsOfferCase(IMS_FALSE),
            objCapaNego(CapaNego()),
            bSupportRtcpXr(IMS_FALSE),
            objRtcpXrAttr(RTCPXRAttributes()),
            bRtcpDisableBeforeSetup(IMS_FALSE){};

    ~AudioProfile()
    {
        while (lstPayload.GetSize() > 0)
        {
            AudioProfile::Payload* pPayload = lstPayload.GetAt(0);

            if (pPayload != IMS_NULL)
            {
                delete pPayload;
            }

            lstPayload.RemoveAt(0);
        }
    };

    AudioProfile& operator=(IN const AudioProfile& obj)
    {
        copy(&obj);
        return *this;
    }

    bool operator==(IN const AudioProfile& obj) const
    {
        return (this->objIpAddr == obj.objIpAddr && this->nDataPort == obj.nDataPort &&
                this->nControlPort == obj.nControlPort &&
                this->strTransportType == obj.strTransportType &&
                this->nRtcpInterval == obj.nRtcpInterval &&
                this->nBandwidthAs == obj.nBandwidthAs && this->nBandwidthRs == obj.nBandwidthRs &&
                this->nBandwidthRr == obj.nBandwidthRr &&
                this->bSupportRtcpXr == obj.bSupportRtcpXr &&
                this->objRtcpXrAttr == obj.objRtcpXrAttr &&
                this->bRtcpDisableBeforeSetup == obj.bRtcpDisableBeforeSetup);
    }

    AudioProfile(IN const AudioProfile& obj) { copy(&obj); }

private:
    void copy(IN const AudioProfile* pProfile)
    {
        if (pProfile == IMS_NULL)
        {
            return;
        }

        this->objIpAddr = pProfile->objIpAddr;
        this->nDataPort = pProfile->nDataPort;
        this->nControlPort = pProfile->nControlPort;
        this->strTransportType = pProfile->strTransportType;
        this->nRtcpInterval = pProfile->nRtcpInterval;
        this->nBandwidthAs = pProfile->nBandwidthAs;
        this->nBandwidthRs = pProfile->nBandwidthRs;
        this->nBandwidthRr = pProfile->nBandwidthRr;
        this->bSupportRtcpXr = pProfile->bSupportRtcpXr;
        this->objRtcpXrAttr = pProfile->objRtcpXrAttr;
        this->bRtcpDisableBeforeSetup = pProfile->bRtcpDisableBeforeSetup;

        while (lstPayload.GetSize() > 0)
        {
            AudioProfile::Payload* pPayload = lstPayload.GetAt(0);

            if (pPayload != IMS_NULL)
            {
                delete pPayload;
            }

            lstPayload.RemoveAt(0);
        }

        for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* pNewPayload =
                    new AudioProfile::Payload(*pProfile->lstPayload.GetAt(i));
            this->lstPayload.Append(pNewPayload);
        }

        this->eDirection = pProfile->eDirection;
        this->nPtime = pProfile->nPtime;
        this->nMaxPtime = pProfile->nMaxPtime;
        this->objCandidateAttr = pProfile->objCandidateAttr;
        this->bIsOfferCase = pProfile->bIsOfferCase;
        this->objCapaNego = pProfile->objCapaNego;
        this->nNegotiatedPayloadIndex = -1;
    }
};
#endif /* End of _IMS_AUDIO_NEGO_PROFILE_H_*/
