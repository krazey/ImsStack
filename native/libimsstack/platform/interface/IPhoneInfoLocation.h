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
#ifndef INTERFACE_PHONE_INFO_LOCATION_H_
#define INTERFACE_PHONE_INFO_LOCATION_H_

#include "AString.h"

class ILocationProperties
{
protected:
    virtual ~ILocationProperties() = default;

public:
    virtual const AString& GetLatitude() const = 0;
    virtual const AString& GetLongitude() const = 0;
    virtual const AString& GetRadius() const = 0;
    virtual const AString& GetShape() const = 0;
    virtual const AString& GetConfidence() const = 0;
    virtual const AString& GetCurrentTime() const = 0;
    virtual const AString& GetMethod() const = 0;
    virtual const AString& GetCountry() const = 0;
    virtual const AString& GetState() const = 0;
    virtual const AString& GetCity() const = 0;
    virtual const AString& GetPostal() const = 0;
    virtual const AString& GetAltitude() const = 0;
    virtual const AString& GetVerticalAccuracy() const = 0;
};

class ILocationInfo
{
protected:
    virtual ~ILocationInfo() = default;

public:
    virtual IMS_BOOL StartListeningForLocation(IN IMS_UINT32 nUpdateIntervalInSec) = 0;
    virtual void StopListeningForLocation() = 0;
    virtual ILocationProperties* GetLocationProperties(IN IMS_SINT32 nType = LOCATION_ALL) = 0;
    virtual IMS_BOOL StartInstantLocationUpdate() = 0;
    virtual void SetDefaultLocationProperties(IN IMS_BOOL bFromUICC = IMS_TRUE) = 0;
    virtual const AString& GetLastKnownCountry() const = 0;

public:
    /// Type for getting location properties
    enum
    {
        LOCATION_ALL = 0,
        LOCATION_POSITION_N_COUNTRY = 1,
        LOCATION_POSITION = 2
    };
};

#endif
