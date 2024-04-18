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

#include "MediaBaseProfile.h"

/**
 * TextProfile is used to keep the SDP negotiation information for text like
 * SDP offer, answer and the negotiated media information.
 */
class TextProfile : public MediaBaseProfile
{
public:
    /**
     * RedFmtp attributes are used within the SDP to carry RED parameters that provide
     * extra configuration details about a specific RED codec used in the RTP stream.
     */
    class RedFmtp : public BaseFmtp
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

        virtual ~RedFmtp(){};

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
    /**
     * Payload for text is the actual text data transported by RTP in a packet.
     */
    class Payload : public BasePayload
    {
    public:
        Payload() :
                BasePayload(){};
        Payload(IN const Payload& obj) :
                BasePayload(obj)
        {
            if (objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
            {
                pFmtp = new TextProfile::RedFmtp(*static_cast<TextProfile::RedFmtp*>(obj.pFmtp));
            }
        }

        virtual ~Payload() {}

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                BasePayload::operator=(obj);

                if (objRtpMap.strPayloadType.EqualsIgnoreCase("red"))
                {
                    pFmtp = new TextProfile::RedFmtp(
                            *static_cast<TextProfile::RedFmtp*>(obj.pFmtp));
                }
            }

            return (*this);
        }
    };

public:
    ImsList<Payload*> lstPayload;
    IMS_BOOL bIsHold;
    IMS_BOOL bKeepRedLevel;

public:
    TextProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVP", 0, 0, 0, 0, MEDIA_DIRECTION_INVALID),
            lstPayload(ImsList<Payload*>()),
            bIsHold(IMS_FALSE),
            bKeepRedLevel(IMS_TRUE){};

    virtual ~TextProfile() { deletePayloads(); }

    TextProfile(IN TextProfile* profile) :
            MediaBaseProfile(profile)
    {
        if (profile == nullptr)
        {
            return;
        }

        bIsHold = profile->bIsHold;
        bKeepRedLevel = profile->bKeepRedLevel;

        deletePayloads();
        addPayloads(profile->lstPayload);
    }

    TextProfile(IN const TextProfile& obj) :
            MediaBaseProfile(obj)
    {
        bIsHold = obj.bIsHold;
        bKeepRedLevel = obj.bKeepRedLevel;

        deletePayloads();
        addPayloads(obj.lstPayload);
    }

    TextProfile& operator=(IN const TextProfile& obj)
    {
        if (this != &obj)
        {
            MediaBaseProfile::operator=(obj);
            bIsHold = obj.bIsHold;
            bKeepRedLevel = obj.bKeepRedLevel;

            deletePayloads();
            addPayloads(obj.lstPayload);
        }
        return (*this);
    }

    bool operator==(IN const TextProfile& obj) const
    {
        return (MediaBaseProfile::operator==(obj) && bIsHold == obj.bIsHold &&
                bKeepRedLevel == obj.bKeepRedLevel);
    }

    bool operator!=(IN const TextProfile& obj) const
    {
        return (MediaBaseProfile::operator!=(obj) || bIsHold != obj.bIsHold ||
                bKeepRedLevel != obj.bKeepRedLevel);
    }

private:
    void deletePayloads()
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

    void addPayloads(IN ImsList<Payload*> payloadList)
    {
        for (IMS_UINT32 i = 0; i < payloadList.GetSize(); i++)
        {
            TextProfile::Payload* pNewPayload = new TextProfile::Payload(*payloadList.GetAt(i));
            lstPayload.Append(pNewPayload);
        }
    }
};

#endif
