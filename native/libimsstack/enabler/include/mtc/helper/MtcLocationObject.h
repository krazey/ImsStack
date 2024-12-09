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

#ifndef MTC_LOCATION_OBJECT_H_
#define MTC_LOCATION_OBJECT_H_

#include "AString.h"
#include "ByteArray.h"
#include "ImsTypeDef.h"
#include "call/IMtcCall.h"

class IMessage;
class IMtcCallContext;
class ISubscriberConfig;
class MtcLocationProperties;

class MtcLocationObject final
{
public:
    explicit MtcLocationObject(IN IMtcCallContext& objContext);
    ~MtcLocationObject();

public:
    static IMS_BOOL IsGeolocationInfoRequired(IN IMtcCallContext& objContext);

    /**
     * Gets the location from the given message.
     *
     * @param objMessage Message contains PIDF-LO body.
     * @return The parsed location. Must be deleted by the user after using.
     *         Null if the message doesn't contain valid PIDF-LO body.
     */
    static MtcLocationProperties* GetLocationFromMessage(IN const IMessage& objMessage);

    void SetLocationToMessage(IN_OUT IMessage& objMessage, IN IMS_BOOL bGeolocationRouting,
            IN const ByteArray& objContent);
    inline void SetLocationToMessage(
            IN_OUT IMessage& objMessage, IN IMS_BOOL bGeolocationRouting = IMS_FALSE)
    {
        SetLocationToMessage(objMessage, bGeolocationRouting, CreateLocationBody());
    }

    ByteArray CreateLocationBody() const;
    ByteArray CreateCallComposerLocationBody(
            IN const AString& strLatitude, IN const AString& strLongitude) const;

private:
    AString CreateCid(IN const ISubscriberConfig& objSubscriberConfig) const;
    AString CreatePersonId() const;

    IMS_SINT32 GetInformationLevel() const;
    static AString GetLocationBodyFrom(IN const IMessage& objMessage);
    static AString GetGeolocationHeader(IN const AString& strCid);
    static AString GetContentLengthHeader(IN const ByteArray& objContent);
    static AString GetContentIdHeader(IN const AString& strCid);
    static AString GetContentDispositionHeader();
    static AString GetEntityUri(IN const ISubscriberConfig& objSubscriberConfig);

    static IMS_BOOL IsGeolocationBlocked(IN IMtcCallContext& objContext);

    IMtcCallContext& m_objContext;
};

class MtcLocationProperties
{
public:
    MtcLocationProperties() {}
    virtual ~MtcLocationProperties() {}

    inline const AString& GetLatitude() const { return m_strLatitude; }
    inline const AString& GetLongitude() const { return m_strLongitude; }
    inline const AString& GetRadius() const { return m_strRadius; }

    inline void SetLatitude(IN const AString& strLatitude) { m_strLatitude = strLatitude; }
    inline void SetLongitude(IN const AString& strLongitude) { m_strLongitude = strLongitude; }
    inline void SetRadius(IN const AString& strRadius) { m_strRadius = strRadius; }

private:
    AString m_strLatitude;
    AString m_strLongitude;
    AString m_strRadius;
};

#endif
