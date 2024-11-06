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
#ifndef AOS_INFO_H_
#define AOS_INFO_H_

#include "IImsAosInfo.h"
#include "AString.h"

class IAosAppContext;

class AosInfo : public IImsAosInfo
{
public:
    explicit AosInfo(IN IAosAppContext* piContext);
    ~AosInfo() override;

private:
    // IImsAosInfo
    AString GetAssociatedUri() override;
    IMS_SINT32 GetConnectionType() override;
    IMS_UINT32 GetImsFeatures() override;
    IMS_UINT32 GetImsState() override;
    IMS_SINT32 GetIpcanType() override;
    AString GetLastPathHeaderValue() override;
    AString GetLocalAddress() override;
    IMS_UINT32 GetLocalPort() override;
    IMS_UINT32 GetRegisteredNetworkType() override;
    AString GetPathHeaderValue() override;
    AString GetPcscfAddress() override;
    IMS_UINT32 GetPcscfPort() override;
    IMS_UINT32 GetRegistrationMode() override;
    AString GetSupportedHeaderValue() override;
    AString GetServiceRouteHeaderValue() override;
    void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized) override;
    void NotifyPublishState(IN IMS_BOOL bIsStarted) override;
    void NotifyEmergencySmsState(IN IMS_BOOL bIsInitialized) override;
    void NotifyEpsfbCallState(IN IMS_UINT32 nState) override;

    IMS_BOOL IsForbiddenBlock();

private:
    IAosAppContext* m_piContext;

private:
    friend class AosInfoTest;
};
#endif  // AOS_INFO_H_
