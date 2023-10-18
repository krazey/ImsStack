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

#ifndef MOCK_I_PHONE_INFO_LOCATION_H_
#define MOCK_I_PHONE_INFO_LOCATION_H_

#include <gmock/gmock.h>

#include "AString.h"
#include "IPhoneInfoLocation.h"

class MockILocationProperties : public ILocationProperties
{
public:
    inline MockILocationProperties() {}
    inline virtual ~MockILocationProperties() {}

    MOCK_METHOD(const AString&, GetLatitude, (), (const, override));
    MOCK_METHOD(const AString&, GetLongitude, (), (const, override));
    MOCK_METHOD(const AString&, GetRadius, (), (const, override));
    MOCK_METHOD(const AString&, GetShape, (), (const, override));
    MOCK_METHOD(const AString&, GetConfidence, (), (const, override));
    MOCK_METHOD(const AString&, GetCurrentTime, (), (const, override));
    MOCK_METHOD(const AString&, GetMethod, (), (const, override));
    MOCK_METHOD(const AString&, GetCountry, (), (const, override));
    MOCK_METHOD(const AString&, GetState, (), (const, override));
    MOCK_METHOD(const AString&, GetCity, (), (const, override));
    MOCK_METHOD(const AString&, GetPostal, (), (const, override));
    MOCK_METHOD(const AString&, GetAltitude, (), (const, override));
    MOCK_METHOD(const AString&, GetVerticalAccuracy, (), (const, override));
};

class MockILocationInfo : public ILocationInfo
{
public:
    inline MockILocationInfo() {}
    inline virtual ~MockILocationInfo() {}

    MOCK_METHOD(
            IMS_BOOL, StartListeningForLocation, (IN IMS_UINT32 nUpdateIntervalInSec), (override));
    MOCK_METHOD(void, StopListeningForLocation, (), (override));
    MOCK_METHOD(ILocationProperties*, GetLocationProperties, (IN IMS_SINT32 nType), (override));
    MOCK_METHOD(IMS_BOOL, StartInstantLocationUpdate, (), (override));
    MOCK_METHOD(void, SetDefaultLocationProperties, (IN IMS_BOOL bFromUICC), (override));
    MOCK_METHOD(const AString&, GetLastKnownCountry, (), (const, override));
};

#endif
