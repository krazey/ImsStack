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
     * TextFmtp is a base class for text-related format-specific parameters.
     */
    class TextFmtp
    {
    public:
        TextFmtp() {}
        virtual ~TextFmtp() {}
    };

    /**
     * T140Fmtp attributes are used within the SDP to carry T140 parameters.
     */
    class T140Fmtp : public TextFmtp
    {
    public:
        T140Fmtp() :
                m_nCps(30),
                m_bIsCpsVisible(IMS_FALSE)
        {
        }

        T140Fmtp(IN IMS_SINT32 nCps, IN IMS_BOOL bIsCpsVisible) :
                m_nCps(nCps),
                m_bIsCpsVisible(bIsCpsVisible)
        {
        }

        T140Fmtp(IN const T140Fmtp& obj) :
                TextFmtp(obj),
                m_nCps(obj.m_nCps),
                m_bIsCpsVisible(obj.m_bIsCpsVisible)
        {
        }

        ~T140Fmtp() override {}

        T140Fmtp& operator=(IN const T140Fmtp& obj)
        {
            if (this != &obj)
            {
                m_nCps = obj.m_nCps;
                m_bIsCpsVisible = obj.m_bIsCpsVisible;
            }
            return (*this);
        }

        bool operator==(IN const T140Fmtp& obj) const
        {
            return (m_nCps == obj.m_nCps && m_bIsCpsVisible == obj.m_bIsCpsVisible);
        }

        bool operator!=(IN const T140Fmtp& obj) const { return !(*this == obj); }

        inline void SetCps(IN const IMS_SINT32 nCps) { m_nCps = nCps; }
        inline IMS_SINT32 GetCps() { return m_nCps; }
        inline void SetVisibleCps(IN const IMS_BOOL bIsCpsVisible)
        {
            m_bIsCpsVisible = bIsCpsVisible;
        }
        inline IMS_BOOL IsCpsVisible() { return m_bIsCpsVisible; }

    private:
        IMS_SINT32 m_nCps;
        IMS_BOOL m_bIsCpsVisible;
    };

    /**
     * RedFmtp attributes are used within the SDP to carry RED parameters that provide
     * extra configuration details about a specific RED codec used in the RTP stream.
     */
    class RedFmtp : public TextFmtp
    {
    public:
        RedFmtp() :
                TextFmtp(),
                m_nRedLevel(-1),
                m_nRedPayload(-1)
        {
        }

        RedFmtp(IN IMS_SINT32 nRedLevel, IN IMS_SINT32 nRedPayload) :
                TextFmtp(),
                m_nRedLevel(nRedLevel),
                m_nRedPayload(nRedPayload)
        {
        }

        RedFmtp(IN const RedFmtp& obj) :
                TextFmtp(obj),
                m_nRedLevel(obj.m_nRedLevel),
                m_nRedPayload(obj.m_nRedPayload)
        {
        }

        ~RedFmtp() override {}

        RedFmtp& operator=(IN const RedFmtp& obj)
        {
            if (this != &obj)
            {
                TextFmtp::operator=(obj);
                m_nRedLevel = obj.m_nRedLevel;
                m_nRedPayload = obj.m_nRedPayload;
            }
            return (*this);
        }

        bool operator==(IN const RedFmtp& obj) const
        {
            return (m_nRedLevel == obj.m_nRedLevel && m_nRedPayload == obj.m_nRedPayload);
        }

        bool operator!=(IN const RedFmtp& obj) const { return !(*this == obj); }

        inline void SetRedLevel(IN const IMS_SINT32 nRedLevel) { m_nRedLevel = nRedLevel; }
        inline IMS_SINT32 GetRedLevel() { return m_nRedLevel; }
        inline void SetRedPayload(IN const IMS_SINT32 nRedPayload) { m_nRedPayload = nRedPayload; }
        inline IMS_SINT32 GetRedPayload() { return m_nRedPayload; }

    private:
        /** Number of RED blocks, equals to the number of T.140 payload copies in a RED payload */
        IMS_SINT32 m_nRedLevel;
        /** Payload type number of the T.140 codec being protected by RED */
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
                BasePayload(),
                m_pFmtp(IMS_NULL)
        {
        }

        Payload(IN const Payload& obj) :
                BasePayload(obj),
                m_pFmtp(IMS_NULL)
        {
            if (obj.m_pFmtp != IMS_NULL)
            {
                if (obj.m_objRtpMap.GetPayloadType().EqualsIgnoreCase("red"))
                {
                    m_pFmtp = std::make_shared<RedFmtp>(
                            *std::static_pointer_cast<RedFmtp>(obj.m_pFmtp));
                }
                else if (obj.m_objRtpMap.GetPayloadType().EqualsIgnoreCase("t140"))
                {
                    m_pFmtp = std::make_shared<T140Fmtp>(
                            *std::static_pointer_cast<T140Fmtp>(obj.m_pFmtp));
                }
            }
        }

        virtual ~Payload() override {}

        std::shared_ptr<BasePayload> clone() const override
        {
            return std::make_shared<Payload>(*this);
        }

        Payload& operator=(IN const Payload& obj)
        {
            if (this != &obj)
            {
                BasePayload::operator=(obj);
                m_pFmtp.reset();

                if (obj.m_pFmtp != IMS_NULL)
                {
                    if (obj.m_objRtpMap.GetPayloadType().EqualsIgnoreCase("red"))
                    {
                        m_pFmtp = std::make_shared<RedFmtp>(
                                *std::static_pointer_cast<RedFmtp>(obj.m_pFmtp));
                    }
                    else if (obj.m_objRtpMap.GetPayloadType().EqualsIgnoreCase("t140"))
                    {
                        m_pFmtp = std::make_shared<T140Fmtp>(
                                *std::static_pointer_cast<T140Fmtp>(obj.m_pFmtp));
                    }
                }
            }

            return (*this);
        }

        bool operator==(IN const Payload& obj) const
        {
            if (!BasePayload::operator==(obj))
            {
                return false;
            }

            if (m_pFmtp == IMS_NULL || obj.m_pFmtp == IMS_NULL)
            {
                return m_pFmtp == obj.m_pFmtp;
            }

            if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("red"))
            {
                return *std::static_pointer_cast<const RedFmtp>(m_pFmtp) ==
                        *std::static_pointer_cast<const RedFmtp>(obj.m_pFmtp);
            }
            else if (m_objRtpMap.GetPayloadType().EqualsIgnoreCase("t140"))
            {
                return *std::static_pointer_cast<const T140Fmtp>(m_pFmtp) ==
                        *std::static_pointer_cast<const T140Fmtp>(obj.m_pFmtp);
            }

            return false;
        }

        bool operator!=(IN const Payload& obj) const { return !(*this == obj); }

        inline std::shared_ptr<TextFmtp> GetFmtp() { return m_pFmtp; }
        inline void SetFmtp(std::shared_ptr<TextFmtp> pFmtp) { m_pFmtp = pFmtp; }

    protected:
        std::shared_ptr<TextFmtp> m_pFmtp;
    };

public:
    TextProfile() :
            MediaBaseProfile(
                    IpAddress::IPv6NONE, 0, 0, "RTP/AVP", 0, 0, 0, 0, MEDIA_DIRECTION_INVALID),
            m_bKeepRedLevel(IMS_TRUE)
    {
    }

    virtual ~TextProfile() override {}

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

    inline void AddPayload(Payload* pPayload)
    {
        MediaBaseProfile::AddPayload(std::shared_ptr<Payload>(pPayload));
    }

    inline void AddPayload(std::shared_ptr<Payload> pPayload)
    {
        MediaBaseProfile::AddPayload(pPayload);
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
