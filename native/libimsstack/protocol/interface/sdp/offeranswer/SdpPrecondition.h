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
#ifndef SDP_PRECONDITION_H_
#define SDP_PRECONDITION_H_

#include "AString.h"

class SdpPrecondition
{
public:
    class DetailInfo
    {
    public:
        inline DetailInfo() :
                m_nStatus(STATUS_E2E),
                m_nDirection(DIRECTION_NONE),
                m_nStrength(STRENGTH_NOTUSED)
        {
        }
        inline DetailInfo(
                IN IMS_SINT32 nStatus, IN IMS_SINT32 nDirection, IN IMS_SINT32 nStrength) :
                m_nStatus(nStatus),
                m_nDirection(nDirection),
                m_nStrength(nStrength)
        {
        }
        inline DetailInfo(IN const DetailInfo& other) :
                m_nStatus(other.m_nStatus),
                m_nDirection(other.m_nDirection),
                m_nStrength(other.m_nStrength)
        {
        }
        ~DetailInfo() = default;

    public:
        DetailInfo& operator=(IN const DetailInfo& other)
        {
            if (this != &other)
            {
                m_nStatus = other.m_nStatus;
                m_nDirection = other.m_nDirection;
                m_nStrength = other.m_nStrength;
            }

            return (*this);
        }

    public:
        inline IMS_SINT32 GetDirection() const { return m_nDirection; }
        inline IMS_SINT32 GetStatus() const { return m_nStatus; }
        inline IMS_SINT32 GetStrength() const { return m_nStrength; }

        inline void SetDirection(IN IMS_SINT32 nDirection) { m_nDirection = nDirection; }
        inline void SetStatus(IN IMS_SINT32 nStatus) { m_nStatus = nStatus; }
        inline void SetStrength(IN IMS_SINT32 nStrength) { m_nStrength = nStrength; }

        AString ToString() const;

    private:
        IMS_SINT32 m_nStatus;
        IMS_SINT32 m_nDirection;
        IMS_SINT32 m_nStrength;
    };

public:
    explicit SdpPrecondition(IN IMS_SINT32 nType = TYPE_QOS, IN IMS_SINT32 nSubType = SUBTYPE_E2E);
    SdpPrecondition(IN const SdpPrecondition& other);
    virtual ~SdpPrecondition();

public:
    SdpPrecondition& operator=(IN const SdpPrecondition& other);

public:
    inline virtual IMS_BOOL AddStatus(IN IMS_SINT32 /*nStatus*/, IN IMS_SINT32 /*nDirection*/,
            IN IMS_SINT32 /*nStrength = STRENGTH_NOTUSED*/)
    {
        return IMS_FALSE;
    }
    inline virtual IMS_BOOL IsPreconditionPresent() const { return IMS_FALSE; }
    virtual AString ToSdp(IN IMS_SINT32 nAttribute) const;

    IMS_BOOL AddStatus(IN const AString& strStatusLine);
    inline IMS_SINT32 GetSubType() const { return m_nSubType; }
    inline IMS_SINT32 GetType() const { return m_nType; }

    static IMS_BOOL ExtractProperties(IN const AString& strStatusLine, OUT IMS_SINT32& nType,
            OUT IMS_SINT32& nSubType, OUT DetailInfo& objDetailInfo);

public:
    // Type of precondition
    enum
    {
        TYPE_INVALID = (-1),

        TYPE_QOS = 0,
        TYPE_OTHER,

        TYPE_MAX
    };

    // Type of precondition class
    enum
    {
        SUBTYPE_INVALID = (-1),

        SUBTYPE_E2E = 0,
        SUBTYPE_SEGMENTED,

        SUBTYPE_MAX
    };

    // Type of "status-type"
    enum
    {
        STATUS_INVALID = (-1),

        STATUS_E2E = 0,
        STATUS_LOCAL,
        STATUS_REMOTE,

        STATUS_MAX
    };

    // Type of "strength-tag"
    enum
    {
        STRENGTH_INVALID = (-1),

        STRENGTH_MANDATORY = 0,
        STRENGTH_OPTIONAL,
        STRENGTH_NONE,
        STRENGTH_FAILURE,
        STRENGTH_UNKNOWN,
        STRENGTH_NOTUSED,

        STRENGTH_MAX
    };

    // Type of "direction-tag"
    enum
    {
        DIRECTION_INVALID = (-1),

        DIRECTION_NONE = 0,
        DIRECTION_SEND,
        DIRECTION_RECV,
        DIRECTION_SENDRECV,

        DIRECTION_MAX
    };

    static const IMS_CHAR* STR_DIRECTION_TAG[DIRECTION_MAX];
    static const IMS_CHAR* STR_STATUS_TYPE[STATUS_MAX];
    static const IMS_CHAR* STR_STRENGTH_TAG[STRENGTH_MAX];
    static const IMS_CHAR* STR_TYPE[TYPE_MAX];

private:
    IMS_SINT32 m_nType;
    IMS_SINT32 m_nSubType;
};

#endif
