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

class AosHandleMtc : public AosHandle
{
public:
    AosHandleMtc(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType);
    virtual ~AosHandleMtc();

    // IAosHandle
    virtual IMS_BOOL App_Notify();

    // IAosCallTrackerListener
    virtual void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState);

    // IAosNetTrackerListener
    virtual void NetTracker_StatusChanged();

protected:
    virtual void InitializeHoldingBlocksPolicy();
    virtual void InitializeServiceBlock();
    virtual void InitializeServiceFeature();
    virtual void InitializeFeatureTags();

    virtual void CheckSuspended();
    virtual void SetSuspendedReason(IN IMS_UINT32 nReason);
    virtual void ResetSuspendedReason(IN IMS_UINT32 nReason);

    virtual void Init();
    virtual void CleanUp();

    virtual IMS_BOOL IsHandleBlocked() const;

    virtual void ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked);
    virtual void ProcessBlockChanged();
    virtual void ProcessCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities);
    virtual void ProcessNetworkChanged();
    virtual void ProcessVopsStateChanged(IN IMS_UINT32 nState, IN IMS_BOOL bUpdateState = IMS_TRUE);

    virtual void ReevaluateUnavailableFeature();

private:
    void UpdateGGsmaRcsTelephonyFeatureTag();
    IMS_UINT32 GetVoiceBlockReasonForIpcan();
    IMS_UINT32 GetVideoBlockReasonForIpcan();
    IMS_BOOL IsCsFeatureTagRequired();
    IMS_BOOL IsInvalidMobileNetwork() const;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

private:
    friend class AosHandleMtcTest;
};
#endif  // AOS_HANDLE_MTC_H_
