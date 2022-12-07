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
    virtual ~AosInfo();

private:
    // IImsAosInfo
    virtual AString GetAssociatedUri();
    virtual IMS_SINT32 GetConnectionType();
    virtual IMS_UINT32 GetImsFeatures();
    virtual IMS_UINT32 GetImsState();
    virtual IMS_SINT32 GetIpcanType();
    virtual AString GetLastPathHeaderValue();
    virtual AString GetLocalAddress();
    virtual IMS_UINT32 GetLocalPort();
    virtual IMS_UINT32 GetRegisteredNetworkType();
    virtual AString GetPathHeaderValue();
    virtual AString GetPcscfAddress();
    virtual IMS_UINT32 GetPcscfPort();
    virtual IMS_UINT32 GetRegistrationMode();
    virtual AString GetSupportedHeaderValue();
    virtual AString GetServiceRouteHeaderValue();
    virtual void NotifyEmergencyCallState(IN IMS_BOOL bIsInitialized);
    virtual void NotifyScbmState(IN IMS_UINT32 nState);
    virtual void NotifyPublishState(IN IMS_BOOL bIsStarted);
    virtual void NotifyEmergencySmsState(IN IMS_BOOL bIsInitialized);
    virtual void NotifyEpsfbCallState(IN IMS_UINT32 nState);

    IMS_BOOL IsForbiddenBlock();

private:
    IAosAppContext* m_piContext;

private:
    friend class AosInfoTest;
};
#endif  // AOS_INFO_H_
