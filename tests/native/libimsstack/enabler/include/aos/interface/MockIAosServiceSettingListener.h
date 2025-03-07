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
#ifndef MOCK_I_AOS_SERVICE_SETTING_LISTENER_H_
#define MOCK_I_AOS_SERVICE_SETTING_LISTENER_H_

#include <gmock/gmock.h>

#include "interface/IAosServiceSettingListener.h"

class MockIAosServiceSettingListener : public IAosServiceSettingListener
{
public:
    MOCK_METHOD(void, ServiceSetting_AirplaneChanged, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD(void, ServiceSetting_DataRoamingChanged, (IN IMS_BOOL bIsAllowed), (override));
    MOCK_METHOD(void, ServiceSetting_MobileDataChanged, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD(void, ServiceSetting_RoamingPreferredVoiceNetworkChanged,
            (IN RoamingPreferredVoiceNetwork eState), (override));
    MOCK_METHOD(void, ServiceSetting_ServiceChanged,
            (IN ServiceSetting eState, IN IMS_UINT32 nServiceBits), (override));
    MOCK_METHOD(void, ServiceSetting_TtyChanged, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD(void, ServiceSetting_VideoChanged, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD(void, ServiceSetting_VolteChanged, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD(void, ServiceSetting_WfcChanged, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD(void, ServiceSetting_WifiChanged, (IN IMS_BOOL bIsOn), (override));
};

#endif  // MOCK_I_AOS_SERVICE_SETTING_LISTENER_H_