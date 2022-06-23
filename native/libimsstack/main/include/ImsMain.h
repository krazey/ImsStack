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
#ifndef IMS_MAIN_H_
#define IMS_MAIN_H_

#include "DeviceConfig.h"
#include "SystemConfig.h"

class ImsMain
{
public:
    ImsMain() = delete;

public:
    // For system configuration or re-configuration
    static void SetConfiguration(
            IN IMS_SINT32 nEvent, IN IMS_SINT32 nCount, IN const __SystemConfig* pSysConfig);
    static void SetDeviceConfig(IN const __DeviceConfig& objConfig);

    static void Initialize();
    static void Uninitialize();
    static void Start();
    static void Stop();

private:
    static void InitializeConfigurationManager();
};

#endif
