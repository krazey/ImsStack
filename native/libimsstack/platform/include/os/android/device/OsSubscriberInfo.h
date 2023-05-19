
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
#ifndef OS_SUBSCRIBER_INFO_H_
#define OS_SUBSCRIBER_INFO_H_

#include "ImsSlot.h"
#include "IPhoneInfoSubscriber.h"

class OsSubscriberInfo : public ImsSlot, public ISubscriberInfo
{
public:
    explicit OsSubscriberInfo(IN IMS_SINT32 nSlotId);
    virtual ~OsSubscriberInfo();

    OsSubscriberInfo(IN const OsSubscriberInfo&) = delete;
    OsSubscriberInfo& operator=(IN const OsSubscriberInfo&) = delete;

public:
    IMS_BOOL GetPhoneNumber(OUT AString& strPhoneNumber) const override;

    IMS_BOOL GetSimMcc(OUT AString& strMcc) const override;
    IMS_BOOL GetSimMnc(OUT AString& strMnc) const override;

    IMS_BOOL GetSimCountryIso(OUT AString& strCountry) const override;
    IMS_BOOL GetNetworkCountryIso(OUT AString& strCountry) const override;

    IMS_BOOL GetSubscriberId(OUT AString& strImsi) const override;

    IMS_BOOL GetPreference(IN const AString& strFileName, IN const AString& strKey,
            OUT AString& strValue) override;
    IMS_BOOL SetPreference(IN const AString& strFileName, IN const AString& strKey,
            IN const AString& strValue) override;
};

#endif
