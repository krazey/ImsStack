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
#include "DeviceConfig.h"

PRIVATE GLOBAL __DeviceConfig DeviceConfig::s_objConfig;

PUBLIC GLOBAL AString DeviceConfig::ToString()
{
    AString strDeviceConfig;

    strDeviceConfig.Sprintf("DeviceConfig=[ supportedSimCount=%d, activeSimCount=%d, "
                            "imsEmergencyEnabled=%d, voLteEnabled=%d, "
                            "vtEnabled=%d, wfcEnabled=%d ]",
            s_objConfig.nSupportedSimCount, s_objConfig.nActiveSimCount,
            s_objConfig.nImsEmergencyEnabled, s_objConfig.nVoLteEnabled, s_objConfig.nVtEnabled,
            s_objConfig.nWfcEnabled);

    return strDeviceConfig;
}

PRIVATE GLOBAL void DeviceConfig::SetConfig(IN const __DeviceConfig& objConfig)
{
    IMS_MEM_Memcpy(&s_objConfig, &objConfig, sizeof(__DeviceConfig));

    if (s_objConfig.nActiveSimCount == 0)
    {
        s_objConfig.nActiveSimCount = 1;
    }

    if (s_objConfig.nSupportedSimCount == 0)
    {
        s_objConfig.nSupportedSimCount = 1;
    }
}
