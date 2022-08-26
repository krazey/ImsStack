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

#include "ImsTypeDef.h"
#include "ImsAosReason.h"

class IMessage;
class AString;
class IMtcCallController;
class MtcConfigurationProxy;
class IMtcService;
class IJniMtcServiceThread;
class MtcEmergencyServiceManager;

class MtcAosEventHandler
{
public:
    explicit MtcAosEventHandler(
            IN IMtcService& objService, IN MtcConfigurationProxy& objConfiguration);
    virtual ~MtcAosEventHandler();
    MtcAosEventHandler(IN const MtcAosEventHandler&) = delete;
    MtcAosEventHandler& operator=(IN const MtcAosEventHandler&) = delete;

    virtual void OnConnected(IN IMS_UINT32 nFeatures, IN IMS_UINT32 nIpcan,
            IN IJniMtcServiceThread* pServiceThread,
            IN MtcEmergencyServiceManager* pEmergencyServiceManager,
            IN IMtcCallController& objCallController);
    virtual void OnDisconnecting(IN IMS_UINT32 nReason, IN IMtcCallController& objCallController);
    virtual void OnDisconnected(IN IMS_UINT32 nReason, IN IMtcCallController& objCallController,
            IN IJniMtcServiceThread* pServiceThread,
            IN MtcEmergencyServiceManager* pEmergencyServiceManager);
    virtual void OnSuspended(IN IMS_UINT32 nReason, IN IMtcCallController& objCallController);
    virtual void OnResumed();

    virtual void OnServiceConnected(IN IMS_UINT32 nServices, IN IMS_UINT32 nIpcan);
    virtual void OnEventNotify(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    inline virtual void SetOnSrvcc(IN IMS_BOOL bOnSrvcc) { m_bOnSrvcc = bOnSrvcc; }

private:
    IMS_SINT32 GetCallReasonByAosReason(IN IMS_UINT32 nAosReason) const;

    IMtcService& m_objService;
    MtcConfigurationProxy& m_objConfiguration;
    IMS_UINT32 m_nIpcan;
    IMS_BOOL m_bOnSrvcc;
};

#endif
