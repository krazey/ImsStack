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
#include "device/OsDeviceInfo.h"

__IMS_TRACE_TAG_IPL__;

PUBLIC
OsDeviceInfo::OsDeviceInfo() {}

PUBLIC VIRTUAL OsDeviceInfo::~OsDeviceInfo() {}

PUBLIC VIRTUAL IMS_BOOL OsDeviceInfo::GetDeviceId(IN IMS_SINT32 nSlotId, OUT AString& strId) const
{
    IMS_SINT32 nResult = PlatformContext::GetInstance()->GetSystem()->GetDeviceId(strId, nSlotId);

    if (strId.Equals("0"))
    {
        strId = AString::ConstNull();
    }

    IMS_TRACE_D("SubsInfo :: deviceId=%s",
            IMS_UTIL_SYS_PROP_IS_DEBUG_MODE() ? strId.GetStr() : "xxx", 0, 0);

    return (nResult == 1);
}

PUBLIC VIRTUAL IMS_BOOL OsDeviceInfo::GetDeviceSoftwareVersion(
        IN IMS_SINT32 nSlotId, OUT AString& strSv) const
{
    return PlatformContext::GetInstance()->GetSystem()->GetDeviceSoftwareVersion(strSv, nSlotId) ==
            1;
}

PUBLIC VIRTUAL IMS_BOOL OsDeviceInfo::GetDeviceName(OUT AString& strDeviceName) const
{
    return PlatformContext::GetInstance()->GetSystem()->GetDeviceName(strDeviceName) == 1;
}
