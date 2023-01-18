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
#ifndef CAP_PROPERTY_H_
#define CAP_PROPERTY_H_

#include "private/ImsProperty.h"

class CapPropertyPrivate;

class CapProperty : public ImsProperty
{
public:
    CapProperty();
    CapProperty(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType);
    CapProperty(IN const CapProperty& other);
    virtual ~CapProperty();

public:
    CapProperty& operator=(IN const CapProperty& other);

public:
    // ImsProperty class
    IMS_BOOL Equals(IN const AString& strValue) const override;

    void AddValue(IN const AString& strValue);
    const AStringArray& GetValues() const;
    void SetKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType);

    static AString CreateCapKey(IN IMS_SINT32 nSectorId, IN IMS_SINT32 nMessageType);
    static AString MessageTypeToString(IN IMS_SINT32 nMessageType);
    static AString SectorIdToString(IN IMS_SINT32 nSectorId);
    static IMS_SINT32 StringToMessageType(IN const AString& strMessageType);
    static IMS_SINT32 StringToSectorId(IN const AString& strSectorId);

private:
    static IMS_BOOL IsValidMessageType(IN IMS_SINT32 nMessageType);
    static IMS_BOOL IsValidSectorId(IN IMS_SINT32 nSectorId);

public:
    /// Sector Id
    enum
    {
        SECTOR_INVALID = 0,
        SECTOR_SESSION,
        SECTOR_FRAMED,
        SECTOR_STREAM_AUDIO,
        SECTOR_STREAM_VIDEO,
        SECTOR_MAX
    };

    /// Message type
    enum
    {
        MESSAGE_TYPE_INVALID = 0,
        MESSAGE_TYPE_REQUEST,
        MESSAGE_TYPE_RESPONSE,
        MESSAGE_TYPE_REQUEST_RESPONSE,
        MESSAGE_TYPE_MAX
    };

    static const IMS_CHAR* SECTOR_STRING[SECTOR_MAX];
    static const IMS_CHAR* MESSAGE_TYPE_STRING[MESSAGE_TYPE_MAX];

private:
    CapPropertyPrivate* m_pPropertyPrivate;
};

#endif
