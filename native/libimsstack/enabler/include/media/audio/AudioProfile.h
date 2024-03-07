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

#include "ImsTypeDef.h"
#include "IpAddress.h"
#include "ImsMap.h"
#include "MediaBaseProfile.h"

/**
 * AudioProfile is used to keep the SDP negotiation information for audio like
 * SDP offer, answer and the negotiated media information.
 */
class AudioProfile : public MediaBaseProfile
{
public:
    /**
     * AmrFmtp attributes are used within the SDP to carry AMR parameters that provide
     * extra configuration details about a specific AMR codec used in the RTP stream.
     */
    class AmrFmtp : public BaseFmtp
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

        AmrFmtp(IN const AmrFmtp& objFmtp) :
                nModeSetList(objFmtp.nModeSetList),
                nDefaultRtpModeSet(objFmtp.nDefaultRtpModeSet),
                bSCREnable(objFmtp.bSCREnable),
                nOctetAlign(objFmtp.nOctetAlign),
                nModeChangeCapability(objFmtp.nModeChangeCapability),
                nModeChangePeriod(objFmtp.nModeChangePeriod),
                nModeChangeNeighbor(objFmtp.nModeChangeNeighbor),
                nMaxRed(objFmtp.nMaxRed),
                nPtime(objFmtp.nPtime),
                nMaxPtime(objFmtp.nMaxPtime),
                bShow_OctetAlign(objFmtp.bShow_OctetAlign),
                bShowModeChangeCapability(objFmtp.bShowModeChangeCapability),
                bShowModeChangePeriod(objFmtp.bShowModeChangePeriod),
                bShowModeChangeNeighbor(objFmtp.bShowModeChangeNeighbor),
                bShow_RobustSorting(objFmtp.bShow_RobustSorting),
                bShowMaxRed(objFmtp.bShowMaxRed),
                bShowPtime(objFmtp.bShowPtime),
                bShowMaxPtime(objFmtp.bShowMaxPtime),
                bShowModeSet(objFmtp.bShowModeSet)
        {
        }

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
                bShow_OctetAlign(octetAlign > 0),
                bShowModeChangeCapability(modeChangeCapability > 0),
                bShowModeChangePeriod(IMS_FALSE),
                bShowModeChangeNeighbor(IMS_FALSE),
                bShow_RobustSorting(IMS_FALSE),
                bShowMaxRed(IMS_FALSE),
                bShowPtime(IMS_FALSE),
                bShowMaxPtime(IMS_FALSE),
                bShowModeSet(IMS_FALSE){};
        virtual ~AmrFmtp(){};
    };

public:
    /**
     * EvsFmtp attributes are used within the SDP to carry EVS parameters that provide
     * extra configuration details about a specific EVS codec used in the RTP stream.
     */
    class EvsFmtp : public BaseFmtp
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

        EvsFmtp(IN const EvsFmtp& objFmtp) :
                nPtime(objFmtp.nPtime),
                nMaxPtime(objFmtp.nMaxPtime),
                nDtx(objFmtp.nDtx),
                nDtx_Recv(objFmtp.nDtx_Recv),
                nHfOnly(objFmtp.nHfOnly),
                nEvsModeSwitch(objFmtp.nEvsModeSwitch),
                nMaxRed(objFmtp.nMaxRed),
                nBrList(objFmtp.nBrList),
                nBrSend(objFmtp.nBrSend),
                nBrRecv(objFmtp.nBrRecv),
                nBwList(objFmtp.nBwList),
                nBwSend(objFmtp.nBwSend),
                nBwRecv(objFmtp.nBwRecv),
                nCmr(objFmtp.nCmr),
                nChAwRecv(objFmtp.nChAwRecv),
                nReceivedChAwRecv(objFmtp.nReceivedChAwRecv),
                nModeSetList(objFmtp.nModeSetList),
                nDefaultRtpModeSet(objFmtp.nDefaultRtpModeSet),
                nModeChangeCapability(objFmtp.nModeChangeCapability),
                nModeChangePeriod(objFmtp.nModeChangePeriod),
                nModeChangeNeighbor(objFmtp.nModeChangeNeighbor),
                bShowPtime(objFmtp.bShowPtime),
                bShowMaxPtime(objFmtp.bShowMaxPtime),
                bShowDtx(objFmtp.bShowDtx),
                bShowHfOnly(objFmtp.bShowHfOnly),
                bShowEvsModeSwitch(objFmtp.bShowEvsModeSwitch),
                bShowMaxRed(objFmtp.bShowMaxRed),
                bShowCmr(objFmtp.bShowCmr),
                bShowChannelAwMode(objFmtp.bShowChannelAwMode),
                bShowModeChangeCapability(objFmtp.bShowModeChangeCapability),
                bShowModeChangePeriod(objFmtp.bShowModeChangePeriod),
                bShowModeChangeNeighbor(objFmtp.bShowModeChangeNeighbor),
                bShowBrList(objFmtp.bShowBrList),
                bShowBwList(objFmtp.bShowBwList),
                bSendCmr(objFmtp.bSendCmr),
                bShowModeSetList(objFmtp.bShowModeSetList)
        {
        }

        virtual ~EvsFmtp(){};
    };

public:
    /**
     * TelephoneEventFmtp attributes are used within the SDP to carry TelephoneEvent parameters that
     * provide extra configuration details about a specific TelephoneEventFmtp codec used in the RTP
     * stream.
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

        CapaNego(const CapaNego& obj) :
                mapTransportCapa(obj.mapTransportCapa),
                mapAttributeCapa(obj.mapAttributeCapa),
                lstPotentialConfig(obj.lstPotentialConfig),
                strNegotiatedAcfg(obj.strNegotiatedAcfg),
                bIsAttCapaInPcfg(obj.bIsAttCapaInPcfg)
        {
        }
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
    };

public:
    enum
    {
        // COMMON PARAMETER
        DEFAULT_PTIME = 20,
        DEFAULT_MAXPTIME = 240,
    };

    ImsList<Payload*> lstPayload;
    IMS_SINT32 nPtime;
    IMS_SINT32 nMaxPtime;
    ImsVector<AString> objCandidateAttr;
    IMS_SINT32 nNegotiatedPayloadIndex;
    IMS_BOOL bIsOfferCase;
    CapaNego objCapaNego;
    IMS_BOOL bSupportRtcpXr;
    RTCPXRAttributes objRtcpXrAttr;
    IMS_BOOL bRtcpDisableBeforeSetup;
    IMS_BOOL bAnbr;

public:
    AudioProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVP", 0, 0, 0, 0, MEDIA_DIRECTION_SEND_RECEIVE),
            lstPayload(ImsList<Payload*>()),
            nPtime(0),
            nMaxPtime(0),
            objCandidateAttr(ImsVector<AString>()),
            nNegotiatedPayloadIndex(-1),
            bIsOfferCase(IMS_FALSE),
            objCapaNego(CapaNego()),
            bSupportRtcpXr(IMS_FALSE),
            objRtcpXrAttr(RTCPXRAttributes()),
            bRtcpDisableBeforeSetup(IMS_FALSE),
            bAnbr(IMS_FALSE){};

    virtual ~AudioProfile() { deletePayloads(); };

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
        nNegotiatedPayloadIndex = profile->nNegotiatedPayloadIndex;
        bIsOfferCase = profile->bIsOfferCase;
        objCapaNego = profile->objCapaNego;
        bSupportRtcpXr = profile->bSupportRtcpXr;
        objRtcpXrAttr = profile->objRtcpXrAttr;
        bRtcpDisableBeforeSetup = profile->bRtcpDisableBeforeSetup;
        bAnbr = profile->bAnbr;

        deletePayloads();
        addPayloads(profile->lstPayload);
    }

    AudioProfile(IN const AudioProfile& obj) :
            MediaBaseProfile(obj)
    {
        nPtime = obj.nPtime;
        nMaxPtime = obj.nMaxPtime;
        objCandidateAttr = obj.objCandidateAttr;
        nNegotiatedPayloadIndex = obj.nNegotiatedPayloadIndex;
        bIsOfferCase = obj.bIsOfferCase;
        objCapaNego = obj.objCapaNego;
        bSupportRtcpXr = obj.bSupportRtcpXr;
        objRtcpXrAttr = obj.objRtcpXrAttr;
        bRtcpDisableBeforeSetup = obj.bRtcpDisableBeforeSetup;
        bAnbr = obj.bAnbr;

        deletePayloads();
        addPayloads(obj.lstPayload);
    }

    AudioProfile& operator=(IN const AudioProfile& obj)
    {
        if (this != &obj)
        {
            MediaBaseProfile::operator=(obj);
            nPtime = obj.nPtime;
            nMaxPtime = obj.nMaxPtime;
            objCandidateAttr = obj.objCandidateAttr;
            nNegotiatedPayloadIndex = obj.nNegotiatedPayloadIndex;
            bIsOfferCase = obj.bIsOfferCase;
            objCapaNego = obj.objCapaNego;
            bSupportRtcpXr = obj.bSupportRtcpXr;
            objRtcpXrAttr = obj.objRtcpXrAttr;
            bRtcpDisableBeforeSetup = obj.bRtcpDisableBeforeSetup;
            bAnbr = obj.bAnbr;

            deletePayloads();
            addPayloads(obj.lstPayload);
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

private:
    void deletePayloads()
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
    }

    void addPayloads(IN ImsList<Payload*> payloadList)
    {
        for (IMS_UINT32 i = 0; i < payloadList.GetSize(); i++)
        {
            AudioProfile::Payload* pNewPayload = new AudioProfile::Payload(*payloadList.GetAt(i));
            lstPayload.Append(pNewPayload);
        }
    }
};

#endif
