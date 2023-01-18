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
#include "ServiceMemory.h"

#include "private/CapProperty.h"

class CapPropertyPrivate
{
public:
    inline CapPropertyPrivate() :
            m_nSectorId(CapProperty::SECTOR_INVALID),
            m_nMessageType(CapProperty::MESSAGE_TYPE_INVALID)
    {
    }
    inline ~CapPropertyPrivate() {}

    CapPropertyPrivate(IN const CapPropertyPrivate&) = delete;
    CapPropertyPrivate& operator=(IN const CapPropertyPrivate&) = delete;

private:
    friend class CapProperty;

    IMS_SINT32 m_nSectorId;
    IMS_SINT32 m_nMessageType;
    AStringArray m_objSdpFields;
};

PUBLIC GLOBAL const IMS_CHAR* CapProperty::SECTOR_STRING[CapProperty::SECTOR_MAX] = {
        "",
        "Session",
        "Framed",
        "StreamAudio",
        "StreamVideo",
};

PUBLIC GLOBAL const IMS_CHAR* CapProperty::MESSAGE_TYPE_STRING[CapProperty::MESSAGE_TYPE_MAX] = {
        "",
        "Req",
        "Resp",
        "Req_Resp",
};

PUBLIC
CapProperty::CapProperty() :
        ImsProperty(ImsProperty::PKEY_CAP),
        m_pPropertyPrivate(new CapPropertyPrivate())
{
}

PUBLIC
CapProperty::CapProperty(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType) :
        ImsProperty(ImsProperty::PKEY_CAP),
        m_pPropertyPrivate(new CapPropertyPrivate())
{
    m_pPropertyPrivate->m_nSectorId = nSectorId;
    m_pPropertyPrivate->m_nMessageType = nMessageType;
}

PUBLIC
CapProperty::CapProperty(IN const CapProperty& other) :
        ImsProperty(other),
        m_pPropertyPrivate(new CapPropertyPrivate())
{
    m_pPropertyPrivate->m_nSectorId = other.m_pPropertyPrivate->m_nSectorId;
    m_pPropertyPrivate->m_nMessageType = other.m_pPropertyPrivate->m_nMessageType;
    m_pPropertyPrivate->m_objSdpFields = other.m_pPropertyPrivate->m_objSdpFields;
}

PUBLIC VIRTUAL CapProperty::~CapProperty()
{
    if (m_pPropertyPrivate != IMS_NULL)
    {
        delete m_pPropertyPrivate;
    }
}

PUBLIC
CapProperty& CapProperty::operator=(IN const CapProperty& other)
{
    if (this != &other)
    {
        m_pPropertyPrivate->m_nSectorId = other.m_pPropertyPrivate->m_nSectorId;
        m_pPropertyPrivate->m_nMessageType = other.m_pPropertyPrivate->m_nMessageType;
        m_pPropertyPrivate->m_objSdpFields = other.m_pPropertyPrivate->m_objSdpFields;
    }

    return (*this);
}

PUBLIC VIRTUAL IMS_BOOL CapProperty::Equals(IN const AString& strValue) const
{
    AString strCapKey =
            CreateCapKey(m_pPropertyPrivate->m_nSectorId, m_pPropertyPrivate->m_nMessageType);

    return strCapKey.Equals(strValue);
}

PUBLIC
void CapProperty::AddValue(IN const AString& strValue)
{
    m_pPropertyPrivate->m_objSdpFields.AddElement(strValue);
}

PUBLIC
const AStringArray& CapProperty::GetValues() const
{
    return m_pPropertyPrivate->m_objSdpFields;
}

PUBLIC
void CapProperty::SetKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType)
{
    m_pPropertyPrivate->m_nSectorId = nSectorId;
    m_pPropertyPrivate->m_nMessageType = nMessageType;
}

PUBLIC GLOBAL AString CapProperty::CreateCapKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType)
{
    if ((nSectorId == SECTOR_INVALID) || (nMessageType == MESSAGE_TYPE_INVALID) ||
            (nMessageType == MESSAGE_TYPE_REQUEST_RESPONSE))
    {
        return AString();
    }

    AString strCapKey;
    IMS_SINT32 nCapKey = IMS_SINT32(nSectorId + (nMessageType << 4));

    return strCapKey.SetNumber(nCapKey, 16);
}

PUBLIC GLOBAL AString CapProperty::MessageTypeToString(IN IMS_SINT32 nMessageType)
{
    return IsValidMessageType(nMessageType) ? AString(MESSAGE_TYPE_STRING[nMessageType])
                                            : AString::ConstNull();
}

PUBLIC GLOBAL AString CapProperty::SectorIdToString(IN IMS_SINT32 nSectorId)
{
    return IsValidSectorId(nSectorId) ? AString(SECTOR_STRING[nSectorId]) : AString::ConstNull();
}

PUBLIC GLOBAL IMS_SINT32 CapProperty::StringToMessageType(IN const AString& strMessageType)
{
    for (IMS_SINT32 i = (MESSAGE_TYPE_INVALID + 1); i < MESSAGE_TYPE_MAX; ++i)
    {
        if (strMessageType.Equals(MESSAGE_TYPE_STRING[i]))
        {
            return i;
        }
    }

    return MESSAGE_TYPE_INVALID;
}

PUBLIC GLOBAL IMS_SINT32 CapProperty::StringToSectorId(IN const AString& strSectorId)
{
    for (IMS_SINT32 i = (SECTOR_INVALID + 1); i < SECTOR_MAX; ++i)
    {
        if (strSectorId.Equals(SECTOR_STRING[i]))
        {
            return i;
        }
    }

    return SECTOR_INVALID;
}

PRIVATE GLOBAL IMS_BOOL CapProperty::IsValidMessageType(IN IMS_SINT32 nMessageType)
{
    return (MESSAGE_TYPE_INVALID < nMessageType) && (nMessageType < MESSAGE_TYPE_MAX);
}

PRIVATE GLOBAL IMS_BOOL CapProperty::IsValidSectorId(IN IMS_SINT32 nSectorId)
{
    return (SECTOR_INVALID < nSectorId) && (nSectorId < SECTOR_MAX);
}
