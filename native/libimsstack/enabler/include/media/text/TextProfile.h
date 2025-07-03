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
        RedFmtp() :
                m_nRedLevel(-1),
                m_nRedPayload(-1) {};

        RedFmtp(IN IMS_SINT32 nRed, IN IMS_SINT32 nRedPT) :
                m_nRedLevel(nRed),
                m_nRedPayload(nRedPT) {};

        RedFmtp(IN const RedFmtp& obj) :
                m_nRedLevel(obj.m_nRedLevel),
                m_nRedPayload(obj.m_nRedPayload) {};

        virtual ~RedFmtp() override {};

        RedFmtp& operator=(IN const RedFmtp& obj)
        {
            if (this != &obj)
            {
                m_nRedLevel = obj.m_nRedLevel;
                m_nRedPayload = obj.m_nRedPayload;
            }
            return (*this);
        }

        inline void SetRedLevel(IN const IMS_SINT32 nRedLevel) { m_nRedLevel = nRedLevel; }
        inline IMS_SINT32 GetRedLevel() { return m_nRedLevel; }
        inline void SetRedPayload(IN const IMS_SINT32 nRedPayload) { m_nRedPayload = nRedPayload; }
        inline IMS_SINT32 GetRedPayload() { return m_nRedPayload; }

    private:
        IMS_SINT32 m_nRedLevel;
        IMS_SINT32 m_nRedPayload;
    };

public:
    /**
     * Payload for text is the actual text data transported by RTP in a packet.
     */
    class Payload : public BasePayload
    {
    public:
        Payload() :
                BasePayload() {};
        Payload(IN const Payload& obj) :
                BasePayload(obj)
        {
            if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("red") && obj.m_pFmtp != IMS_NULL)
            {
                m_pFmtp =
                        new TextProfile::RedFmtp(*static_cast<TextProfile::RedFmtp*>(obj.m_pFmtp));
            }
        }

        virtual ~Payload() override {}

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                BasePayload::operator=(obj);

                if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("red") && obj.m_pFmtp != IMS_NULL)
                {
                    m_pFmtp = new TextProfile::RedFmtp(
                            *static_cast<TextProfile::RedFmtp*>(obj.m_pFmtp));
                }
            }

            return (*this);
        }
    };

public:
    TextProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVP", 0, 0, 0, 0, MEDIA_DIRECTION_INVALID),
            m_bKeepRedLevel(IMS_TRUE) {};

    virtual ~TextProfile() override {}

    explicit TextProfile(IN TextProfile* profile) :
            MediaBaseProfile(profile)
    {
        if (profile == nullptr)
        {
            return;
        }

        m_bKeepRedLevel = profile->m_bKeepRedLevel;
    }

    TextProfile(IN const TextProfile& obj) :
            MediaBaseProfile(obj)
    {
        m_bKeepRedLevel = obj.m_bKeepRedLevel;
    }

    TextProfile& operator=(IN const TextProfile& obj)
    {
        if (this != &obj)
        {
            MediaBaseProfile::operator=(obj);
            m_bKeepRedLevel = obj.m_bKeepRedLevel;
        }
        return (*this);
    }

    bool operator==(IN const TextProfile& obj) const
    {
        return (MediaBaseProfile::operator==(obj) && m_bKeepRedLevel == obj.m_bKeepRedLevel);
    }

    bool operator!=(IN const TextProfile& obj) const { return !(*this == obj); }

    Payload* GetPayloadAt(IN IMS_UINT32 nIndex) override
    {
        BasePayload* pPayload = MediaBaseProfile::GetPayloadAt(nIndex);
        return (pPayload != IMS_NULL) ? static_cast<Payload*>(pPayload) : IMS_NULL;
    }

    inline void SetKeepRedundantLevel(IN const IMS_BOOL bKeepRedLevel)
    {
        m_bKeepRedLevel = bKeepRedLevel;
    }
    inline IMS_BOOL GetKeepRedundantLevel() { return m_bKeepRedLevel; }

private:
    IMS_BOOL m_bKeepRedLevel;
};

#endif
