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

#ifndef TEXT_PROFILE_H_
#define TEXT_PROFILE_H_

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

        RtpMap(IN const RtpMap& obj) :
                nPayloadNum(obj.nPayloadNum),
                strPayloadType(obj.strPayloadType),
                nSamplingRate(obj.nSamplingRate)
        {
        }

        RtpMap& operator=(IN const RtpMap& obj)
        {
            if (this != &obj)
            {
                nPayloadNum = obj.nPayloadNum;
                strPayloadType = obj.strPayloadType;
                nSamplingRate = obj.nSamplingRate;
            }
            return (*this);
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

        RedFmtp(IN IMS_SINT32 nRed, IN IMS_SINT32 nRedPT) :
                nRedLevel(nRed),
                nRedPayload(nRedPT){};

        RedFmtp(IN const RedFmtp& obj) :
                nRedLevel(obj.nRedLevel),
                nRedPayload(obj.nRedPayload){};

        RedFmtp& operator=(IN const RedFmtp& obj)
        {
            if (this != &obj)
            {
                nRedLevel = obj.nRedLevel;
                nRedPayload = obj.nRedPayload;
            }
            return (*this);
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
            objRtpMap = obj.objRtpMap;

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
                if (pFmtp != IMS_NULL)
                {
                    delete reinterpret_cast<TextProfile::RedFmtp*>(pFmtp);
                }
            }
        }

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                objRtpMap = obj.objRtpMap;

                if (objRtpMap.strPayloadType.Equals("red"))
                {
                    pFmtp = new TextProfile::RedFmtp(
                            *reinterpret_cast<TextProfile::RedFmtp*>(obj.pFmtp));
                }
            }

            return (*this);
        }

        void SetRtpMap(IN const IMS_UINT32 payloadNum, IN const AString& payloadType,
                IN const IMS_UINT32 samplingRate)
        {
            objRtpMap.nPayloadNum = payloadNum;
            objRtpMap.strPayloadType = payloadType;
            objRtpMap.nSamplingRate = samplingRate;
        }

        void SetRtpMap(IN const RtpMap& objMap)
        {
            objRtpMap.nPayloadNum = objMap.nPayloadNum;
            objRtpMap.strPayloadType = objMap.strPayloadType;
            objRtpMap.nSamplingRate = objMap.nSamplingRate;
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
        if (this != &obj)
        {
            copy(&obj);
        }
        return (*this);
    }

    bool operator==(IN const TextProfile& obj) const
    {
        return (objIpAddress == obj.objIpAddress && nDataPort == obj.nDataPort &&
                nControlPort == obj.nControlPort && strTransportType == obj.strTransportType &&
                nRtcpInterval == obj.nRtcpInterval && nBandwidthAs == obj.nBandwidthAs &&
                nBandwidthRs == obj.nBandwidthRs && nBandwidthRr == obj.nBandwidthRr &&
                eDirection == obj.eDirection && bISOfferCase == obj.bISOfferCase &&
                bIsHold == obj.bIsHold && bKeepRedLevel == obj.bKeepRedLevel);
    }

private:
    void copy(const TextProfile* pProfile)
    {
        if (pProfile == IMS_NULL)
        {
            return;
        }

        objIpAddress = pProfile->objIpAddress;
        nDataPort = pProfile->nDataPort;
        nControlPort = pProfile->nControlPort;
        strTransportType = pProfile->strTransportType;
        nRtcpInterval = pProfile->nRtcpInterval;
        nBandwidthAs = pProfile->nBandwidthAs;
        nBandwidthRs = pProfile->nBandwidthRs;
        nBandwidthRr = pProfile->nBandwidthRr;

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
            lstPayload.Append(pNewPayload);
        }

        eDirection = pProfile->eDirection;
        bISOfferCase = pProfile->bISOfferCase;
        bIsHold = pProfile->bIsHold;
        bKeepRedLevel = pProfile->bKeepRedLevel;
    }
};

#endif
