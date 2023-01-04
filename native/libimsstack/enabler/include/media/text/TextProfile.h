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

#ifndef _IMS_TEXT_PROFILE_H_
#define _IMS_TEXT_PROFILE_H_

#include "IMSTypeDef.h"
#include "IpAddress.h"

class TextProfile
{
public:
    class RtpMap
    {
    public:
        IMS_UINT32 nPayloadNum;
        AString strPayloadType;
        IMS_UINT32 nSamplingRate;

    public:
        RtpMap() :
                nPayloadNum(0),
                nSamplingRate(0)
        {
        }
        RtpMap(IN const RtpMap& obj)
        {
            this->nPayloadNum = obj.nPayloadNum;
            this->strPayloadType = obj.strPayloadType;
            this->nSamplingRate = obj.nSamplingRate;
        }

        RtpMap& operator=(IN const RtpMap& obj)
        {
            this->nPayloadNum = obj.nPayloadNum;
            this->strPayloadType = obj.strPayloadType;
            this->nSamplingRate = obj.nSamplingRate;
            return *this;
        }
    };

public:
    class RedFmtp
    {
    public:
        IMS_SINT32 nRedLevel;
        IMS_SINT32 nRedPayload;

    public:
        RedFmtp() :
                nRedLevel(-1),
                nRedPayload(-1){};
        RedFmtp(IN IMS_SINT32 nRed, IN IMS_SINT32 nRedPT)
        {
            nRedLevel = nRed;
            nRedPayload = nRedPT;
        };
        RedFmtp(IN const RedFmtp& obj)
        {
            this->nRedLevel = -1;
            this->nRedPayload = -1;
            this->nRedLevel = obj.nRedLevel;
            this->nRedPayload = obj.nRedPayload;
        }

        RedFmtp& operator=(IN const RedFmtp& obj)
        {
            this->nRedLevel = obj.nRedLevel;
            this->nRedPayload = obj.nRedPayload;
            return *this;
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

            if (objRtpMap.strPayloadType.Equals("red"))
            {
                pFmtp = new TextProfile::RedFmtp(
                        *reinterpret_cast<TextProfile::RedFmtp*>(obj.pFmtp));
            }
        }

        virtual ~Payload()
        {
            if (objRtpMap.strPayloadType.Equals("red"))
            {
                if (this->pFmtp != IMS_NULL)
                {
                    delete reinterpret_cast<TextProfile::RedFmtp*>(this->pFmtp);
                }
            }
        }

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                this->objRtpMap = obj.objRtpMap;

                if (objRtpMap.strPayloadType.Equals("red"))
                {
                    pFmtp = new TextProfile::RedFmtp(
                            *reinterpret_cast<TextProfile::RedFmtp*>(obj.pFmtp));
                }
            }

            return *this;
        }

        void SetRtpMap(IN IMS_UINT32 payloadNum, IN AString payloadType, IN IMS_UINT32 samplingRate)
        {
            this->objRtpMap.nPayloadNum = payloadNum;
            this->objRtpMap.strPayloadType = payloadType;
            this->objRtpMap.nSamplingRate = samplingRate;
        }

        void SetRtpMap(IN const RtpMap& objMap)
        {
            this->objRtpMap.nPayloadNum = objMap.nPayloadNum;
            this->objRtpMap.strPayloadType = objMap.strPayloadType;
            this->objRtpMap.nSamplingRate = objMap.nSamplingRate;
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
    IMS_BOOL bISOfferCase;
    IMS_BOOL bIsHold;
    IMS_BOOL bKeepRedLevel;

public:
    TextProfile() :
            nDataPort(0),
            nControlPort(0),
            strTransportType("RTP/AVP"),
            nRtcpInterval(0),
            nBandwidthAs(0),
            nBandwidthRs(0),
            nBandwidthRr(0),
            eDirection(MEDIA_DIRECTION_INVALID),
            bISOfferCase(IMS_FALSE),
            bIsHold(IMS_FALSE),
            bKeepRedLevel(IMS_TRUE){};

    TextProfile(IN const TextProfile& obj) { copy(&obj); }

    virtual ~TextProfile()
    {
        while (lstPayload.GetSize() > 0)
        {
            TextProfile::Payload* pPayload = lstPayload.GetAt(0);

            if (pPayload != IMS_NULL)
            {
                delete pPayload;
            }

            lstPayload.RemoveAt(0);
        }
    }

    TextProfile& operator=(IN const TextProfile& obj)
    {
        copy(&obj);
        return *this;
    }

    bool operator==(IN const TextProfile& obj) const
    {
        return (this->objIpAddress == obj.objIpAddress && this->nDataPort == obj.nDataPort &&
                this->nControlPort == obj.nControlPort &&
                this->strTransportType == obj.strTransportType &&
                this->nRtcpInterval == obj.nRtcpInterval &&
                this->nBandwidthAs == obj.nBandwidthAs && this->nBandwidthRs == obj.nBandwidthRs &&
                this->nBandwidthRr == obj.nBandwidthRr && this->eDirection == obj.eDirection &&
                this->bISOfferCase == obj.bISOfferCase && this->bIsHold == obj.bIsHold &&
                this->bKeepRedLevel == obj.bKeepRedLevel);
    }

private:
    void copy(const TextProfile* pProfile)
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

        while (lstPayload.GetSize() > 0)
        {
            TextProfile::Payload* pPayload = lstPayload.GetAt(0);

            if (pPayload != IMS_NULL)
            {
                delete pPayload;
            }

            lstPayload.RemoveAt(0);
        }

        for (IMS_UINT32 i = 0; i < pProfile->lstPayload.GetSize(); i++)
        {
            TextProfile::Payload* pNewPayload =
                    new TextProfile::Payload(*pProfile->lstPayload.GetAt(i));
            this->lstPayload.Append(pNewPayload);
        }

        this->eDirection = pProfile->eDirection;
        this->bISOfferCase = pProfile->bISOfferCase;
        this->bIsHold = pProfile->bIsHold;
        this->bKeepRedLevel = pProfile->bKeepRedLevel;
    }
};

#endif
