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
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "device/OsSubscriberInfo.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsSubscriberInfo::OsSubscriberInfo(IN IMS_SINT32 nSlotId) :
        ImsSlot(nSlotId)
{
}

PUBLIC VIRTUAL OsSubscriberInfo::~OsSubscriberInfo() {}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::GetPhoneNumber(OUT AString& strPhoneNumber) const
{
    PlatformContext::GetInstance()->GetSystem()->GetPhoneNumber(strPhoneNumber, GetSlotId());

    IMS_TRACE_D("SubsInfo :: phoneNumber=%s",
            IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? strPhoneNumber.GetStr() : "xxx", 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::GetSimMcc(OUT AString& strMcc) const
{
    PlatformContext::GetInstance()->GetSystem()->GetSimMcc(strMcc, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::GetSimMnc(OUT AString& strMnc) const
{
    PlatformContext::GetInstance()->GetSystem()->GetSimMnc(strMnc, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::GetSimCountryIso(OUT AString& strCountry) const
{
    PlatformContext::GetInstance()->GetSystem()->GetSimCountryIso(strCountry, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::GetNetworkCountryIso(OUT AString& strCountry) const
{
    PlatformContext::GetInstance()->GetSystem()->GetNetworkCountryIso(strCountry, GetSlotId());

    IMS_TRACE_D("SubsInfo :: networkCountry=%s", strCountry.GetStr(), 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::GetSubscriberId(OUT AString& strImsi) const
{
    PlatformContext::GetInstance()->GetSystem()->GetSubscriberId(strImsi, GetSlotId());

    IMS_TRACE_D("SubsInfo :: subscriberId(imsi)=%s",
            IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? strImsi.GetStr() : "xxx", 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::GetPreference(
        IN const AString& strFileName, IN const AString& strKey, OUT AString& strValue)
{
    PlatformContext::GetInstance()->GetSystem()->GetPreference(
            strFileName, strKey, GetSlotId(), strValue);

    IMS_TRACE_D("SubsInfo :: preference(get)=%s", strValue.GetStr(), 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsSubscriberInfo::SetPreference(
        IN const AString& strFileName, IN const AString& strKey, IN const AString& strValue)
{
    PlatformContext::GetInstance()->GetSystem()->SetPreference(
            strFileName, strKey, strValue, GetSlotId());

    return IMS_TRUE;
}
