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

#ifndef INTERFACE_MTC_AOS_CONNECTOR_H_
#define INTERFACE_MTC_AOS_CONNECTOR_H_

#include "AString.h"
#include "IMSTypeDef.h"

class IMtcAosConnector
{
public:
    virtual ~IMtcAosConnector() {}

    // IImsAos interface wrappers.
    virtual IMS_UINT32 GetFeatures() const = 0;
    virtual IMS_UINT32 GetSuspendedReason() const = 0;
    virtual IMS_BOOL IsFeatureConnected(IN IMS_UINT32 nFeature) const = 0;
    virtual IMS_BOOL IsImsConnected() const = 0;
    virtual IMS_BOOL IsImsSuspended() const = 0;
    virtual void SetReady(IN IMS_BOOL bReady, IN IMS_UINT32 nService) const = 0;
    virtual void UpdateFeature(IN IMS_UINT32 nFeatures) const = 0;
    virtual IMS_BOOL Control(IN IMS_UINT32 nType) const = 0;

    // IImsAosInfo interface wrappers.
    virtual AString GetAssociatedUri() const = 0;
    virtual IMS_UINT32 GetConnectionType() const = 0;
    virtual IMS_UINT32 GetImsState() const = 0;
    virtual IMS_UINT32 GetIpcanType() const = 0;
    virtual AString GetLastPathHeaderValue() const = 0;
    virtual AString GetLocalAddress() const = 0;
    virtual IMS_UINT32 GetLocalPort() const = 0;
    virtual IMS_UINT32 GetRegisteredNetworkType() const = 0;
    virtual AString GetPathHeaderValue() const = 0;
    virtual AString GetPcscfAddress() const = 0;
    virtual IMS_UINT32 GetPcscfPort() const = 0;
    virtual IMS_UINT32 GetRegistrationMode() const = 0;
    virtual AString GetSupportedHeaderValue() const = 0;
    virtual AString GetServiceRouteHeaderValue() const = 0;
    virtual void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized) = 0;
    virtual void NotifyEpsfbCallState(IN IMS_UINT32 nState) = 0;
};

#endif
