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
#ifndef SIP_FACTORY_PROXY_H_
#define SIP_FACTORY_PROXY_H_

#include "ImsTypeDef.h"

class ISipTokenGenerator;
class SipFactoryProxyPrivate;
class SipIpSecState;
class SipMessageTracker;
class SipPacketTracker;
class SipRoutingRejectNotifier;
class SipRtConfigHelper;
class SipTransportHelper;

class SipFactoryProxy
{
private:
    SipFactoryProxy();
    ~SipFactoryProxy();

public:
    SipFactoryProxy(IN const SipFactoryProxy&) = delete;
    SipFactoryProxy& operator=(IN const SipFactoryProxy&) = delete;

public:
    SipIpSecState* GetIpSecState(IN IMS_SINT32 nSlotId);
    SipMessageTracker* GetMessageTracker(IN IMS_SINT32 nSlotId);
    SipPacketTracker* GetPacketTracker(IN IMS_SINT32 nSlotId);
    SipRoutingRejectNotifier* GetRoutingRejectNotifier(IN IMS_SINT32 nSlotId);
    SipRtConfigHelper* GetRtConfigHelper(IN IMS_SINT32 nSlotId);
    SipTransportHelper* GetTransportHelper(IN IMS_SINT32 nSlotId);
    void SetTokenGenerator(IN IMS_SINT32 nSlotId, IN ISipTokenGenerator* piTokenGenerator);

    IMS_BOOL IsIpSecStateEnabled(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsMessageTrackerEnabled(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsPacketTrackerEnabled(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsRoutingRejectNotifierEnabled(IN IMS_SINT32 nSlotId) const;

public:
    static void CreateInstance();
    static void DestroyInstance();
    static SipFactoryProxy* GetInstance();

private:
    static SipFactoryProxy* s_pFactoryProxy;

    SipFactoryProxyPrivate* m_pPrivate;
};

#endif
