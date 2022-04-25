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
#ifndef AOS_HANDLE_MTC_H_
#define AOS_HANDLE_MTC_H_

#include "handle/AosHandle.h"

class AosHandleMtc
    : public AosHandle
{
public:
    AosHandleMtc
    (
        IN IAosAppContext* piAppContext,
        IN const AString& strAppId,
        IN const AString& strServiceId,
        IN const IMS_SINT32 nServiceType
    );
    virtual ~AosHandleMtc();

    // IAosHandle
    virtual void App_Notify();

    // IAosCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN IMS_UINT32 nState);

    // IAosNetTrackerListener
    virtual void NetTracker_StatusChanged();

protected:
    virtual void InitializeServiceBlock();
    virtual void InitializeServiceFeature();
    virtual void InitializeFeatureTags();

    virtual void UpdateFeatureTags();

    virtual void ProcessImsSuspended(IN IMS_UINT32 nReason = 0);
    virtual void ProcessImsResumed(IN IMS_UINT32 nReason = 0);

    virtual void CheckSuspended();
    virtual void SetSuspendedReason(IN IMS_UINT32 nReason);
    virtual void ResetSuspendedReason(IN IMS_UINT32 nReason);

    virtual void Init();
    virtual void CleanUp();

    virtual IMS_BOOL IsHandleBlocked() const;
    virtual IMS_BOOL IsBlockForMobile(IN IMS_UINT32 nBlock) const;
    virtual IMS_BOOL IsBlockForWifi(IN IMS_UINT32 nBlock) const;

    virtual void ProcessCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities);
    virtual void ProcessNetworkChanged();
    virtual void ProcessVopsStateChanged(IN IMS_UINT32 nState);

private:
    IMS_UINT32 m_nVops;
};
#endif // AOS_HANDLE_MTC_H_
