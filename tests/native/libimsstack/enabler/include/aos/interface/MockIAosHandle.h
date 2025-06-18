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

#ifndef MOCK_I_AOS_HANDLE_H_
#define MOCK_I_AOS_HANDLE_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "AString.h"

#include "interface/IAosHandle.h"

class IImsAosMonitor;
class AosFeatureTagList;

class MockIAosHandle : public IAosHandle
{
public:
    MOCK_METHOD(AString&, GetAppId, (), (override));
    MOCK_METHOD(AString&, GetServiceId, (), (override));
    MOCK_METHOD(IMS_UINT32, GetServiceType, (), (override));
    MOCK_METHOD(IImsAosMonitor*, GetMonitor, (), (override));
    MOCK_METHOD(IMS_SINT32, GetRequestType, (), (override));
    MOCK_METHOD(void, SetRequestType, (IN IMS_SINT32 nReqType), (override));
    MOCK_METHOD(IMS_BOOL, IsRegBinded, (), (override));
    MOCK_METHOD(void, SetRegBinded, (IN IMS_BOOL bBind), (override));
    MOCK_METHOD(IMS_BOOL, IsNetworkRegBinded, (), (override));
    MOCK_METHOD(void, SetNetworkRegBinded, (IN IMS_BOOL bNetworkBind), (override));
    MOCK_METHOD(IMS_BOOL, IsRegFeatureTagRequired, (), (override));
    MOCK_METHOD(IMS_BOOL, IsRegToNextPcscfRequested, (), (override));
    MOCK_METHOD(AosFeatureTagList&, GetFeatureTagList, (), (override));
    MOCK_METHOD(AosFeatureTagList&, GetBindedFeatureTagList, (), (override));
    MOCK_METHOD(void, ProcessFeatureTagChange, (), (override));
    MOCK_METHOD(void, Request, (IN IMS_UINT32 nType, IN IMS_UINT32 nState), (override));
    MOCK_METHOD(
            IMS_BOOL, App_StateChanged, (IN IMS_UINT32 nState, IN IMS_UINT32 nReason), (override));
    MOCK_METHOD(IMS_BOOL, App_Notify, (), (override));
    MOCK_METHOD(void, Handle_Notify, (IN IMS_UINT32 nType, IN IMS_BOOL bBlocked), (override));
    MOCK_METHOD(void, Init, (), (override));
    MOCK_METHOD(void, CleanUp, (), (override));
};

#endif  // MOCK_I_AOS_HANDLE_H_
