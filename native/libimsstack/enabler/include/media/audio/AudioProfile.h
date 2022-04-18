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

/* == INCLUDES ========================================================= */
#include "IMSTypeDef.h"
#include "IPAddress.h"
#include "IMSMap.h"

class AudioProfile
{
public :
    class RtpMap
    {
    public :
        IMS_UINT32 nPayloadNum;
        AString strPayloadType;
        IMS_UINT32 nSamplingRate;
        IMS_SINT32 nChannel;          // default is 1, if this value is -1 hide channel value at SDP
    public :
        RtpMap() :
                nPayloadNum(0),
                strPayloadType(AString::ConstNull()),
                nSamplingRate(0),
                nChannel(1)
        {};
    };

public :
    class AmrFmtp
    {
    public :
        IMS_UINT32 nModeSetList;
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

    public :
        AmrFmtp(IN AmrFmtp* pFmtp = NULL) :
                nModeSetList(0),
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
                bShowMaxPtime(IMS_FALSE)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }

            this->nModeSetList = pFmtp->nModeSetList;
            this->bSCREnable = pFmtp->bSCREnable;
            this->nOctetAlign = pFmtp->nOctetAlign;
            this->nModeChangeCapability = pFmtp->nModeChangeCapability;
            this->nModeChangePeriod = pFmtp->nModeChangePeriod;
            this->nModeChangeNeighbor = pFmtp->nModeChangeNeighbor;

            this->nMaxRed = pFmtp->nMaxRed;
            this->nPtime = pFmtp->nPtime;
            this->nMaxPtime = pFmtp->nMaxPtime;

            this->bShow_OctetAlign = pFmtp->bShow_OctetAlign;
            this->bShowModeChangeCapability = pFmtp->bShowModeChangeCapability;
            this->bShowModeChangePeriod = pFmtp->bShowModeChangePeriod;
            this->bShowModeChangeNeighbor = pFmtp->bShowModeChangeNeighbor;
            this->bShow_RobustSorting = pFmtp->bShow_RobustSorting;

            this->bShowMaxRed = pFmtp->bShowMaxRed;
            this->bShowPtime = pFmtp->bShowPtime;
            this->bShowMaxPtime = pFmtp->bShowMaxPtime;
        };

        AmrFmtp(IN IMS_UINT32 modeSet, IN IMS_SINT32 octetAlign,
                IN IMS_SINT32 modeChangeCapablity) :
                nModeSetList(modeSet),
                bSCREnable(IMS_TRUE),
                nOctetAlign(octetAlign),
                nModeChangeCapability(modeChangeCapablity),
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
                bShowMaxPtime(IMS_FALSE)
        {
            if (octetAlign > 0)
            {
                bShow_OctetAlign = IMS_TRUE;
            }
            else
            {
                bShow_OctetAlign = IMS_FALSE;
            }

            if (modeChangeCapablity > 0)
            {
                bShowModeChangeCapability = IMS_TRUE;
            }
            else
            {
                bShowModeChangeCapability = IMS_FALSE;
            }
        };
    };
public :
    class EvsFmtp
    {
    public :
        enum
        {
            // COMMON PARAMETER
            DEFAULT_PTIME = -1,
            DEFAULT_MAXPTIME = -1,
            DEFAULT_DTX = 1,
            DEFAULT_HFMODE = 0,
            DEFAULT_EVSMODESWITCH = 0,
            DEFAULT_MAXRED = -1,

            DEFAULT_BANDWIDTHLIST = 0,
            DEFAULT_BITRATELIST = 0,

            // PRIMARY PARAMETER
            DEFAULT_BITRATE = -1,
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

        // Common parameter
        IMS_SINT32 nPtime;
        IMS_SINT32 nMaxPtime;
        IMS_UINT32 nDtx;            // 1(default) is turn on DTX
        //IMS_UINT32 nDtx_Recv;     // 1(default) is turn on DTX
        IMS_UINT32 nHfOnly;         // 0(default) is compact and hf format used,
                                    //other is only hf format used
        IMS_UINT32 nEvsModeSwitch;  // 0(default) is "primary mode start"
        IMS_SINT32 nMaxRed;

        // Primary parameter
        IMS_UINT32 nBrList;     // EVS primary mode bitrate range (kbps)
        IMS_SINT32 nBrSend;     // EVS primary mode bitrate range (kbps)
                                //  - only send direction (used at sendrecv/sendonly direction)
        IMS_SINT32 nBrRecv;     // EVS primary mode bitrate range (kbps)
                                //  - only recv direction (used at sendrecv/recvonly direction)
        IMS_UINT32 nBwList;     // bw has a value from the set
                                //  : nb, wb, swb, fb, nb-wb, nb-swb, and nb-fb. nb, wb, swb, fb
        IMS_SINT32 nBwSend;
        IMS_SINT32 nBwRecv;
        IMS_SINT32 nCmr;
        // multiple mono ch is not supported yet..
        IMS_SINT32 nChAwRecv;   // -1 is channel aware mode disable,
                                // 0(default) is not used at the start of the session,
                                // but it'll be changed using CMR or RTCP app.
        IMS_SINT32 nReceivedChAwRecv;   // -1 is channel aware mode disable,
                                        // 0(default) is not used at the start of the session,
                                        // but it'll be changed using CMR or RTCP app.
        // AMR-WB IO parameter
        IMS_UINT32 nModeSetList;
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
        IMS_BOOL bSendCmr;             //send cmr option

    public :
        EvsFmtp(IN EvsFmtp* pFmtp = NULL) :
                nPtime(DEFAULT_PTIME),
                nMaxPtime(DEFAULT_MAXPTIME),
                nDtx(DEFAULT_DTX),
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
                bSendCmr(IMS_FALSE)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }
            this->nPtime = pFmtp->nPtime;
            this->nMaxPtime = pFmtp->nMaxPtime;
            this->nDtx = pFmtp->nDtx;
            this->nHfOnly = pFmtp->nHfOnly;
            this->nEvsModeSwitch = pFmtp->nEvsModeSwitch;
            this->nMaxRed = pFmtp->nMaxRed;
            this->nBrList = pFmtp->nBrList;
            this->nBrSend = pFmtp->nBrSend;
            this->nBrRecv = pFmtp->nBrRecv;
            this->nBwList = pFmtp->nBwList;
            this->nBwSend = pFmtp->nBwSend;
            this->nBwRecv = pFmtp->nBwRecv;
            this->nCmr = pFmtp->nCmr;
            this->nChAwRecv = pFmtp->nChAwRecv;
            this->nReceivedChAwRecv = pFmtp->nReceivedChAwRecv;
            this->nModeSetList = pFmtp->nModeSetList;
            this->nModeChangeCapability = pFmtp->nModeChangeCapability;
            this->nModeChangePeriod = pFmtp->nModeChangePeriod;
            this->nModeChangeNeighbor = pFmtp->nModeChangeNeighbor;
            this->bShowPtime = pFmtp->bShowPtime;
            this->bShowMaxPtime = pFmtp->bShowMaxPtime;
            this->bShowDtx = pFmtp->bShowDtx;
            this->bShowHfOnly = pFmtp->bShowHfOnly;
            this->bShowEvsModeSwitch = pFmtp->bShowEvsModeSwitch;
            this->bShowMaxRed = pFmtp->bShowMaxRed;
            this->bShowCmr = pFmtp->bShowCmr;
            this->bShowChannelAwMode = pFmtp->bShowChannelAwMode;
            this->bShowModeChangeCapability= pFmtp->bShowModeChangeCapability;
            this->bShowModeChangePeriod = pFmtp->bShowModeChangePeriod;
            this->bShowModeChangeNeighbor = pFmtp->bShowModeChangeNeighbor;
            this->bShowBrList = pFmtp->bShowBrList;
            this->bShowBwList = pFmtp->bShowBwList;
            this->bSendCmr = pFmtp->bSendCmr;
        };
    };

public :
    class TelephoneEventFmtp
    {
    public :
        AString strEvents;
    public :
        TelephoneEventFmtp() :
                strEvents("0-15")
        {};

        TelephoneEventFmtp(IN AString events) :
                strEvents(events)
        {};

        TelephoneEventFmtp(IN TelephoneEventFmtp* pFmtp)
        {
            if (pFmtp == IMS_NULL)
            {
                return;
            }
            this->strEvents = pFmtp->strEvents;
        };
    };

public :
    class Payload
    {
    public :
        RtpMap objRtpMap;
        void* pFmtp;

    public :
        Payload() :
                pFmtp(IMS_NULL)
        {};

        ~Payload() {
            if (objRtpMap.strPayloadType.EqualsIgnoreCase("AMR-WB") ||
                    objRtpMap.strPayloadType.EqualsIgnoreCase("AMR"))
            {
                AudioProfile::AmrFmtp* fmtp = reinterpret_cast<AudioProfile::AmrFmtp*>(this->pFmtp);
                if (fmtp != IMS_NULL)
                {
                    delete fmtp;
                }
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
            {
                AudioProfile::EvsFmtp* fmtp = reinterpret_cast<AudioProfile::EvsFmtp*>(this->pFmtp);
                if (fmtp != IMS_NULL)
                {
                    delete fmtp;
                }
            }
            else if (objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                AudioProfile::TelephoneEventFmtp* fmtp =
                        reinterpret_cast<AudioProfile::TelephoneEventFmtp*>(this->pFmtp);
                if (fmtp != IMS_NULL)
                {
                    delete fmtp;
                }
            }
        };
    private:
        Payload(IN const Payload& obj);
        Payload& operator=(IN const Payload& obj);

    public :
        void SetRtpMap(IN IMS_UINT32 nPayloadNum,
                IN AString strPayloadType, IN IMS_UINT32 nSamplingRate, IN IMS_UINT32 nChannel)
        {
            objRtpMap.nPayloadNum = nPayloadNum;
            objRtpMap.strPayloadType= strPayloadType;
            objRtpMap.nSamplingRate= nSamplingRate;
            objRtpMap.nChannel= nChannel;
        };

        void SetRtpMap(IN RtpMap* pRtpMap)
        {
            if (pRtpMap == IMS_NULL)
            {
                return;
            }

            objRtpMap.nPayloadNum = pRtpMap->nPayloadNum;
            objRtpMap.strPayloadType= pRtpMap->strPayloadType;
            objRtpMap.nSamplingRate= pRtpMap->nSamplingRate;
            objRtpMap.nChannel= pRtpMap->nChannel;
        };
    };

    class CapaNego
    {
    public :
        IMSMap<IMS_SINT32, AString> mapTransportCapa;
        IMSMap<IMS_SINT32, AString> mapAttributeCapa;
        IMSList<AString> lstPotentialConfig;

        AString strNegotiatedAcfg;
        IMS_BOOL bIsAttCapaInPcfg;

    public :
        CapaNego() :
                strNegotiatedAcfg(""),
                bIsAttCapaInPcfg(IMS_FALSE)
        {};
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
                bSupportPacketDuplicatedRle(IMS_FALSE)
        {};

        RTCPXRAttributes& operator = (const RTCPXRAttributes& p)
        {
            bSupportStatisticMetrics = p.bSupportStatisticMetrics;
            bSupportVoipMatircs = p.bSupportVoipMatircs;
            bSupportPacketLossRle = p.bSupportPacketLossRle;
            bSupportPacketDuplicatedRle = p.bSupportPacketDuplicatedRle;
            return *this;
        }
    };

public :
    enum {
        // COMMON PARAMETER
        DEFAULT_PTIME = -1,
        DEFAULT_MAXPTIME = -1,
    };

    IPAddress objIpAddr;
    IMS_UINT32 nDataPort;
    IMS_UINT32 nControlPort;
    AString strTransportType;          // Default RTP/AVP
    IMS_UINT32 nRtcpInterval;
    IMS_SINT32 nBandwidthAs;
    IMS_SINT32 nBandwidthRs;
    IMS_SINT32 nBandwidthRr;
    IMSList<Payload*> lstPayload;
    MEDIA_DIRECTION eDirection;
    IMS_SINT32 nPtime;
    IMS_SINT32 nMaxPtime;
    // Candidate
    IMS_SINT32 nCandidatePriority;
    IMS_SINT32 nNegotiatedPayloadIndex;
    IMS_BOOL bIsOfferCase;
    //SRTP parameter
    CapaNego objCapaNego;
    IMS_BOOL bSupportSrtp;
    IMS_BOOL bSupportCapaNegoForSrtp;
    IMS_SINT32 nMasterKeyLifeTime;
    eMMPFSrtpCryptoType     eSrtpCryptoType;
    IMS_UINT8 szKey[32]; //MMPF_MAX_KEY_LEN 32
    //RTCP-XR
    IMS_BOOL bSupportRtcpXr;
    RTCPXRAttributes objRtcpXrAttr;
    IMS_BOOL bRtcpDisableBeforeSetup;

public :
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
            eDirection(MEDIA_DIRECTION_INVALID),
            nPtime(0),
            nMaxPtime(0),
            nCandidatePriority(-1),
            nNegotiatedPayloadIndex(-1),
            bIsOfferCase(IMS_FALSE),
            objCapaNego(CapaNego()),
            bSupportSrtp(IMS_FALSE),
            bSupportCapaNegoForSrtp(IMS_FALSE),
            nMasterKeyLifeTime(0),
            eSrtpCryptoType(MMPF_SRTP_CRYPTO_TYPE_NONE),
            szKey{0,},
            bSupportRtcpXr(IMS_FALSE),
            objRtcpXrAttr(RTCPXRAttributes()),
            bRtcpDisableBeforeSetup(IMS_FALSE)
    {};

    AudioProfile(AudioProfile* pProfile)
    {
        Copy(pProfile);
        this->nNegotiatedPayloadIndex = -1;
    }

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

    void Copy(AudioProfile* pProfile)
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

        //RTCP-XR
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

        for (IMS_UINT32 i=0; i<pProfile->lstPayload.GetSize(); i++)
        {
            AudioProfile::Payload* pOldPayload = pProfile->lstPayload.GetAt(i);
            if (pOldPayload == IMS_NULL)
            {
                continue;
            }

            AudioProfile::Payload* pNewPayload = new AudioProfile::Payload();
            pNewPayload->SetRtpMap(&pOldPayload->objRtpMap);

            if (pOldPayload->objRtpMap.strPayloadType.Equals("AMR") ||
                    pOldPayload->objRtpMap.strPayloadType.Equals("AMR-WB"))
            {
                AudioProfile::AmrFmtp* pOld_AMRFmtp =
                        reinterpret_cast<AudioProfile::AmrFmtp*>(pOldPayload->pFmtp);
                if (pOld_AMRFmtp == IMS_NULL)
                {
                    delete pNewPayload;
                    pNewPayload = IMS_NULL;

                    continue;
                }

                AudioProfile::AmrFmtp* pNew_AmrFmtp = new AudioProfile::AmrFmtp(pOld_AMRFmtp);
                pNewPayload->pFmtp = reinterpret_cast<void*>(pNew_AmrFmtp);
            }
            else if (pOldPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("telephone-event"))
            {
                AudioProfile::TelephoneEventFmtp* pOld_TEFmtp =
                        reinterpret_cast<AudioProfile::TelephoneEventFmtp*>(pOldPayload->pFmtp);
                if (pOld_TEFmtp == IMS_NULL)
                {
                    delete pNewPayload;
                    pNewPayload = IMS_NULL;

                    continue;
                }

                AudioProfile::TelephoneEventFmtp* pNew_TEFmtp =
                        new AudioProfile::TelephoneEventFmtp(pOld_TEFmtp);
                pNewPayload->pFmtp = reinterpret_cast<void*>(pNew_TEFmtp);
            }
            else if (pOldPayload->objRtpMap.strPayloadType.EqualsIgnoreCase("EVS"))
            {
                AudioProfile::EvsFmtp* pOld_EVSFmtp =
                        reinterpret_cast<AudioProfile::EvsFmtp*>(pOldPayload->pFmtp);
                if (pOld_EVSFmtp == IMS_NULL)
                {
                    delete pNewPayload;
                    pNewPayload = IMS_NULL;

                    continue;
                }

                AudioProfile::EvsFmtp* pNew_EVSFmtp = new AudioProfile::EvsFmtp(pOld_EVSFmtp);
                pNewPayload->pFmtp = reinterpret_cast<void*>(pNew_EVSFmtp);
            }

            this->lstPayload.Append(pNewPayload);
        }

        this->eDirection = pProfile->eDirection;
        this->nPtime = pProfile->nPtime;
        this->nMaxPtime = pProfile->nMaxPtime;
        this->nCandidatePriority = pProfile->nCandidatePriority;
        this->bIsOfferCase = pProfile->bIsOfferCase;

        this->objCapaNego = pProfile->objCapaNego;
        //SRTP
        this->bSupportSrtp = pProfile->bSupportSrtp;
        this->bSupportCapaNegoForSrtp = pProfile->bSupportCapaNegoForSrtp;
        this->nMasterKeyLifeTime = pProfile->nMasterKeyLifeTime;
        this->eSrtpCryptoType= pProfile->eSrtpCryptoType;
        IMS_MEM_Memcpy(this->szKey, pProfile->szKey, 32);
    };
};
#endif /* End of _IMS_AUDIO_NEGO_PROFILE_H_*/
