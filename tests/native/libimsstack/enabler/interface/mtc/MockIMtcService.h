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

#ifndef MOCK_I_MTC_SERVICE_H_
#define MOCK_I_MTC_SERVICE_H_

#include <gmock/gmock.h>
#include "IMtcService.h"
#include "ImsTypeDef.h"

class AString;
class ICoreService;
class IJniMtcServiceThread;
class IMtcAosConnector;
class IMtcAosStateListener;
class ISrvccStateListener;
enum class ServiceStatus;
enum class ServiceType;
enum class SrvccState;

class MockIMtcService : public IMtcService
{
public:
    virtual ~MockIMtcService() {}

    MOCK_METHOD(ServiceType, GetServiceType, (), (const, override));
    MOCK_METHOD(void, AddAosStateListener, (IN IMtcAosStateListener*), (override));
    MOCK_METHOD(void, RemoveAosStateListener, (IN IMtcAosStateListener*), (override));
    MOCK_METHOD(void, AddSrvccStateListener, (IN ISrvccStateListener* piListener), (override));
    MOCK_METHOD(void, RemoveSrvccStateListener, (IN ISrvccStateListener* piListener), (override));
    MOCK_METHOD(IMS_BOOL, IsActive, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergency, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsNr, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEpsCombinedAttach, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRoaming, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWlanIpCanType, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCsfbAvailable, (), (const, override));
    MOCK_METHOD(ServiceStatus, GetOldStatus, (), (const, override));
    MOCK_METHOD(ServiceStatus, GetStatus, (), (const, override));
    MOCK_METHOD(ICoreService*, GetICoreService, (), (const, override));
    MOCK_METHOD(IMtcAosConnector*, GetAosConnector, (), (const, override));
    MOCK_METHOD(IJniMtcServiceThread*, GetJniServiceThread, (), (const, override));
    MOCK_METHOD(SrvccState, GetSrvccState, (), (const, override));
    MOCK_METHOD(void, UpdateSrvccState, (IN SrvccState eState), (override));
    MOCK_METHOD(void, SetTerminalBasedCallWaiting, (IN IMS_BOOL bEnabled), (override));
    MOCK_METHOD(void, OpenEmergencyService, (IN ServiceType eServiceType), (override));
    MOCK_METHOD(void, StopEmergencyService, (), (override));
    MOCK_METHOD(
            void, ProcessTestCommand, (IN IMS_SINT32, IN IMS_SINT32, IN IMS_SINT32), (override));
    MOCK_METHOD(TbcwStatus, GetTbcwStatus, (), (const, override));

    // IEnablerService
    MOCK_METHOD(void, NotifyJniEnablerSet, (), (override));
};

#endif
