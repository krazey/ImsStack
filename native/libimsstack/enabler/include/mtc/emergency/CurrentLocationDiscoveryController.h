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

#ifndef CURRENT_LOCATION_DISCOVERY_CONTROLLER_H_
#define CURRENT_LOCATION_DISCOVERY_CONTROLLER_H_

#include "IRetryTaskHelperListener.h"
#include "ImsTypeDef.h"
#include "RetryCmd.h"
#include <memory>

class IMessage;
class IMtcCallContext;
class IPublication;
class ISipMessage;
class ISipServerConnection;
class RetryTaskHelper;

class CurrentLocationDiscoveryController : public RetryCmd
{
public:
    explicit CurrentLocationDiscoveryController(IN IMtcCallContext& objContext);
    virtual ~CurrentLocationDiscoveryController();
    CurrentLocationDiscoveryController(IN const CurrentLocationDiscoveryController&) = delete;
    CurrentLocationDiscoveryController& operator=(
            IN const CurrentLocationDiscoveryController&) = delete;

    static IMS_BOOL IsCurrentLocationDiscoveryInfoReceived(
            IN const ISipServerConnection& objSipServerConnection);
    static IMS_BOOL IsPeriodicLocationDiscoveryRequired(
            IN IMS_BOOL bEmergency, IN IMS_SINT32 nMethod);

    virtual void OnCurrentLocationDiscoveryInfoReceived(
            IN ISipServerConnection& objSipServerConnection);
    virtual void StartPeriodicLocationDiscovery();

    // RetryCmd class
    IMS_RESULT ExecuteCmd() override;

private:
    static IMS_BOOL HasRequestForCurrentLocation(IN const ISipMessage& objSipMessage);
    void SendResponseForInfo(IN ISipServerConnection& objSipServerConnection,
            IN IMS_UINT32 nResponseCode);
    void SendCurrentLocationPublish();
    void SetLocationInformation();
    void SendPublish();
    IMS_BOOL CreatePublication();
    void DestroyPublication();

private:
    IMtcCallContext& m_objContext;
    IPublication* m_piPublication;
    std::unique_ptr<RetryTaskHelper> m_pLocationTransmissionTask;
};

#endif
