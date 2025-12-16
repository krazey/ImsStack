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

#ifndef MTC_AOS_EVENT_HANDLER_H
#define MTC_AOS_EVENT_HANDLER_H

#include "ImsList.h"
#include "ImsTypeDef.h"
#include "IuMtcService.h"
#include "helper/IMtcAosStateListener.h"

class IMessage;
class IMtcService;
class MtcConfigurationProxy;
class MtcEmergencyServiceManager;

class MtcAosEventHandler
{
public:
    explicit MtcAosEventHandler(
            IN IMtcService& objService, IN MtcConfigurationProxy& objConfiguration);
    virtual ~MtcAosEventHandler();
    MtcAosEventHandler(IN const MtcAosEventHandler&) = delete;
    MtcAosEventHandler& operator=(IN const MtcAosEventHandler&) = delete;

    virtual void AddListener(IN IMtcAosStateListener* piListener);
    virtual void RemoveListener(IN IMtcAosStateListener* piListener);

    virtual void OnConnected(IN IMS_UINT32 nFeatures);
    virtual void OnDisconnecting(IN IMS_UINT32 nReason);
    virtual void OnDisconnected(IN IMS_UINT32 nReason, IN IMS_SINT32 nDataFailureReason);
    virtual void OnSuspended(IN IMS_UINT32 nReason);
    virtual void OnResumed();

    virtual void OnServiceConnected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan);
    virtual void OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

private:
    void NotifyStateChanged(IN MtcAosState eState, IN IMS_UINT32 eAosReason,
            IN IMS_SINT32 nDataFailureReason = 0) const;
    IuMtcService::ServiceState ConvertAosFeatureToServiceState(IMS_UINT32 nFeatures) const;

    IMtcService& m_objService;
    MtcConfigurationProxy& m_objConfiguration;
    ImsList<IMtcAosStateListener*> m_objListeners;
};

#endif
