/*
 * Copyright (C) 2023 The Android Open Source Project
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

#ifndef MOCK_AOS_CONNECTOR_H_
#define MOCK_AOS_CONNECTOR_H_

#include <gmock/gmock.h>

#include "ImsTypeDef.h"
#include "IIpcan.h"
#include "ITimer.h"
#include "interface/IAosPcscf.h"
#include "interface/IAosConnectionListener.h"
#include "connection/AosConnector.h"

class IAosAppContext;
class IAosConnection;
class IAosConnectorListener;
class AosUtil;

class MockAosConnector : public AosConnector
{
public:
    MockAosConnector() :
            AosConnector()
    {
    }
    ~MockAosConnector() override {}
    MOCK_METHOD(IMS_BOOL, Start, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
    MOCK_METHOD(void, Stop, (IN IMS_SINT32 nDelayTimeSec), (override));
    MOCK_METHOD(void, SetListener, (IN IAosConnectorListener * piListener), (override));
    MOCK_METHOD(IMS_BOOL, IsReady, (), (const, override));
    MOCK_METHOD(void, ResetReadyRecovery, (), (override));
    MOCK_METHOD(IMS_BOOL, IsCrossSimConnected, (), (const, override));
    MOCK_METHOD(IMS_BOOL, ProcessPendingPcscfChange, (), (override));
    MOCK_METHOD(void, CleanAll, (), (override));
    MOCK_METHOD(IMS_BOOL, ConfigurePcscf, (), (override));
    MOCK_METHOD(void, ProcessIpv6TimerExpired, (), (override));
    MOCK_METHOD(void, ProcessStopDelayTimerExpired, (), (override));
    MOCK_METHOD(void, ProcessReadyRecoveryTimerExpired, (), (override));
    MOCK_METHOD(void, StartTimer, (IN IMS_UINT32 nType, IN IMS_UINT32 nDuration), (override));
    MOCK_METHOD(void, StopTimer, (IN IMS_UINT32 nType), (override));
    MOCK_METHOD(void, ClearTimers, (), (override));
    MOCK_METHOD(void, AosConnection_StateChanged, (IN IMS_UINT32 nDataState), (override));
    MOCK_METHOD(void, AosConnection_IpChanged, (), (override));
    MOCK_METHOD(void, AosConnection_IpcanCatChanged, (), (override));
    MOCK_METHOD(void, AosConnection_PcscfChanged, (), (override));
    MOCK_METHOD(void, AosConnection_ConnectionFailed, (), (override));
    MOCK_METHOD(void, Pcscf_NotifyResult, (IN IMS_BOOL bResult), (override));
    MOCK_METHOD(void, Timer_TimerExpired, (IN ITimer * piTimer), (override));
    MOCK_METHOD(void, CleanUp, (), (override));
};

#endif  // MOCK_AOS_CONNECTOR_H_