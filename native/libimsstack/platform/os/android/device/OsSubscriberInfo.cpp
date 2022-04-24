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
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "device/OsSubscriberInfo.h"
#include "system-intf/System.h"

__IMS_TRACE_TAG_ADAPT__;

PUBLIC
OsSubscriberInfo::OsSubscriberInfo(IN IMS_SINT32 nSlotId)
    : ImsSlot(nSlotId)
{
}

PUBLIC VIRTUAL
OsSubscriberInfo::~OsSubscriberInfo()
{
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetPhoneNumber(OUT AString& strPhoneNumber) const
{
    System::GetInstance()->GetPhoneNumber(strPhoneNumber, GetSlotId());

    IMS_TRACE_D("SubsInfo :: phoneNumber=%s",
            IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? strPhoneNumber.GetStr() : "xxx", 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetMcc(OUT AString& strMcc) const
{
    System::GetInstance()->GetMcc(strMcc, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetMnc(OUT AString& strMnc) const
{
    System::GetInstance()->GetMnc(strMnc, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetOperator(OUT AString& strOperator) const
{
    System::GetInstance()->GetOperator(strOperator, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetCountry(OUT AString& strCountry) const
{
    System::GetInstance()->GetCountry(strCountry, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetNetworkCountry(OUT AString& strCountry) const
{
    System::GetInstance()->GetNetworkCountry(strCountry, GetSlotId());

    IMS_TRACE_D("SubsInfo :: networkCountry=%s", strCountry.GetStr(), 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetSubscriberId(OUT AString& strImsi) const
{
    System::GetInstance()->GetSubscriberId(strImsi, GetSlotId());

    IMS_TRACE_D("SubsInfo :: subscriberId(imsi)=%s",
            IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? strImsi.GetStr() : "xxx", 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetEmergencyNumberListFromSim(OUT AString& strEnlFromSim) const
{
    System::GetInstance()->GetEmergencyNumberListFromSim(strEnlFromSim, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_SINT32 OsSubscriberInfo::GetEmergencyPriorityFromModem()
{
    return System::GetInstance()->GetEmergencyPriorityFromModem(GetSlotId());
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::IsUiccGbaSupported()
{
    return System::GetInstance()->IsUiccGbaSupported(GetSlotId());
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::GetPreference(IN const AString& strFileName,
        IN const AString& strKey, OUT AString& strValue,
        IN PREFERENCE_VALUE_ENTYPE eValueType /*= PREFERENCE_VALUE_STRING*/)
{
    System::GetInstance()->GetPreference(strFileName,
            strKey, static_cast<IMS_UINT32>(eValueType), GetSlotId(), strValue);

    IMS_TRACE_D("SubsInfo :: preference(get)=%s", strValue.GetStr(), 0, 0);

    return IMS_TRUE;
}

PUBLIC VIRTUAL
IMS_BOOL OsSubscriberInfo::SetPreference(IN const AString& strFileName,
        IN const AString& strKey, IN const AString& strValue,
        IN PREFERENCE_VALUE_ENTYPE eValueType /*= PREFERENCE_VALUE_STRING*/)
{
    System::GetInstance()->SetPreference(strFileName,
            strKey, static_cast<IMS_UINT32>(eValueType), strValue, GetSlotId());

    return IMS_TRUE;
}
