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

#ifndef MOCK_I_MTC_AOS_CONNECTOR_H_
#define MOCK_I_MTC_AOS_CONNECTOR_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "helper/IMtcAosConnector.h"
#include <gmock/gmock.h>

class MockIMtcAosConnector : public IMtcAosConnector
{
public:
    MOCK_METHOD(IMS_UINT32, GetFeatures, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetSuspendedReason, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsFeatureConnected, (IN IMS_UINT32 nFeature), (const, override));
    MOCK_METHOD(IMS_BOOL, IsImsConnected, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsImsSuspended, (), (const, override));
    MOCK_METHOD(void, SetReady, (IN IMS_BOOL bReady, IN IMS_UINT32 nService), (const, override));
    MOCK_METHOD(void, UpdateFeature, (IN IMS_UINT32 nFeatures), (const, override));
    MOCK_METHOD(IMS_BOOL, Control, (IN IMS_UINT32 nType), (const, override));
    MOCK_METHOD(AString, GetAssociatedUri, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetConnectionType, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetImsState, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetIpcanType, (), (const, override));
    MOCK_METHOD(AString, GetLastPathHeaderValue, (), (const, override));
    MOCK_METHOD(AString, GetLocalAddress, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetLocalPort, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetRegisteredNetworkType, (), (const, override));
    MOCK_METHOD(AString, GetPathHeaderValue, (), (const, override));
    MOCK_METHOD(AString, GetPcscfAddress, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetPcscfPort, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetRegistrationMode, (), (const, override));
    MOCK_METHOD(AString, GetSupportedHeaderValue, (), (const, override));
    MOCK_METHOD(AString, GetServiceRouteHeaderValue, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCrossSimConnected, (), (const, override));
    MOCK_METHOD(void, NotifyEmergencyCallState, (IN IMS_BOOL bIsInitialized), (const, override));
    MOCK_METHOD(void, NotifyEpsfbCallState, (IN IMS_UINT32 nState), (const, override));
    MOCK_METHOD(void, RegisterWithNextPcscf, (IN IMS_UINT32 nUnavailableTimeForCurrentPcscf),
            (const, override));
};

#endif
