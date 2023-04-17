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
#ifndef OS_LOCATION_INFO_H_
#define OS_LOCATION_INFO_H_

#include "ImsSlot.h"
#include "IPhoneInfoLocation.h"

class LocationProperties;

class OsLocationInfo : public ImsSlot, public ILocationInfo
{
public:
    explicit OsLocationInfo(IN IMS_SINT32 nSlotId);
    virtual ~OsLocationInfo();

    OsLocationInfo(IN const OsLocationInfo&) = delete;
    OsLocationInfo& operator=(IN const OsLocationInfo&) = delete;

public:
    IMS_BOOL StartListeningForLocation(IN IMS_UINT32 nUpdateIntervalInSec) override;
    void StopListeningForLocation() override;
    ILocationProperties* GetLocationProperties(IN IMS_SINT32 nType = LOCATION_ALL) override;
    IMS_BOOL StartInstantLocationUpdate() override;
    void SetDefaultLocationProperties(IN IMS_BOOL bFromUicc = IMS_TRUE) override;
    const AString& GetLastKnownCountry() const override;

private:
    void SetLastKnownCountry(IN const AString& strCountry);
    AString GetCountryIso(IN IMS_BOOL bFromUicc = IMS_FALSE) const;

public:
    static const IMS_CHAR COUNTRY_ISO_UNKNOWN[];

private:
    IMS_BOOL m_bIsStarted;
    AString m_strLastKnownCountry;
    LocationProperties* m_pLocationProperties;
};

#endif
