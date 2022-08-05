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
#ifndef DEVICE_CONFIG_H_
#define DEVICE_CONFIG_H_

#include "AString.h"

struct __DeviceConfig
{
    // Number of logical modems currently configured to be activated
    IMS_SINT32 nActiveModemCount;

    // 0: disabled, 1: enabled
    IMS_SINT32 nImsEmergencyEnabled;
    IMS_SINT32 nVoLteEnabled;
    IMS_SINT32 nVtEnabled;
    IMS_SINT32 nWfcEnabled;

    __DeviceConfig() :
            nActiveModemCount(1),
            // VoLte always enables because Ims emergency call should be supported.
            nImsEmergencyEnabled(1),
            nVoLteEnabled(0),
            nVtEnabled(0),
            nWfcEnabled(0)
    {
    }

    __DeviceConfig(IN IMS_SINT32 nActiveModemCount_, IN IMS_SINT32 nImsEmergencyEnabled_,
            IN IMS_SINT32 nVoLteEnabled_, IN IMS_SINT32 nVtEnabled_, IN IMS_SINT32 nWfcEnabled_) :
            nActiveModemCount(nActiveModemCount_),
            nImsEmergencyEnabled(nImsEmergencyEnabled_),
            nVoLteEnabled(nVoLteEnabled_),
            nVtEnabled(nVtEnabled_),
            nWfcEnabled(nWfcEnabled_)
    {
    }
};

class DeviceConfig
{
public:
    DeviceConfig() = delete;

public:
    inline static IMS_SINT32 GetActiveModemCount() { return s_objConfig.nActiveModemCount; }
    inline static IMS_BOOL IsImsEmergencyEnabled() { return s_objConfig.nImsEmergencyEnabled == 1; }
    inline static IMS_BOOL IsVoLteEnabled()
    {
        return (s_objConfig.nVoLteEnabled == 1) || IsImsEmergencyEnabled();
    }
    inline static IMS_BOOL IsVtEnabled() { return s_objConfig.nVtEnabled == 1; }
    inline static IMS_BOOL IsWfcEnabled() { return s_objConfig.nWfcEnabled == 1; }

    static AString ToString();

private:
    static void SetConfig(IN const __DeviceConfig& objConfig);

private:
    friend class NativeCommandsHandler;

    static __DeviceConfig s_objConfig;
};

#endif
