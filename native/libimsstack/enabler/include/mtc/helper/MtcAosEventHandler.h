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

#include "IMSTypeDef.h"

class IMessage;
class AString;
class IMtcCallController;
class MtcConfigurationProxy;
class IMtcService;
class JniMtcServiceThread;
class MtcEmergencyServiceManager;

class MtcAosEventHandler
{
public:
    explicit MtcAosEventHandler(
            IN IMtcService& objService, IN MtcConfigurationProxy& objConfiguration);
    ~MtcAosEventHandler();
    MtcAosEventHandler(IN const MtcAosEventHandler&) = delete;
    MtcAosEventHandler& operator=(IN const MtcAosEventHandler&) = delete;

    void OnConnected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan,
            IN JniMtcServiceThread* pServiceThread,
            IN MtcEmergencyServiceManager* pEmergencyServiceManager,
            IN IMtcCallController& objCallController);
    void OnDisconnecting(IN IMS_UINT32 nReason, IN IMtcCallController& objCallController);
    void OnDisconnected(IN IMS_UINT32 nReason, IN IMtcCallController& objCallController,
            IN JniMtcServiceThread* pServiceThread,
            IN MtcEmergencyServiceManager* pEmergencyServiceManager);
    void OnSuspended(IN IMS_UINT32 nReason, IN IMtcCallController& objCallController);
    void OnResumed();

    void OnServiceConnected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan);
    void OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

private:
    IMtcService& m_objService;
    MtcConfigurationProxy& m_objConfiguration;
    IMS_UINT32 m_nIpcan;
};

#endif
