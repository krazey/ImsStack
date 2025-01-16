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

#include "IImsRadio.h"
#include "ITimer.h"

#include "handle/AosHandle.h"

#include "interface/IAosServicePhoneListener.h"

class AosHandleMtc :
        public AosHandle,
        public AosServicePhoneListener,
        public IImsRadioSsacListener,
        public ITimerListener
{
public:
    AosHandleMtc(IN IAosAppContext* piAppContext, IN const AString& strAppId,
            IN const AString& strServiceId, IN const IMS_SINT32 nServiceType);
    ~AosHandleMtc() override;

    // IImsAos
    IMS_UINT32 GetFeatures() override;

    // IAosHandle
    IMS_BOOL App_Notify() override;

    // IAosCallTrackerListener
    void CallTracker_StateChanged(IN IMS_UINT32 nType, IN CallState eState) override;

    // IAosNetTrackerListener
    void NetTracker_StatusChanged() override;

protected:
    void InitializeHoldingBlocksPolicy() override;
    void InitializeServiceBlock() override;
    void InitializeServiceFeature() override;
    void InitializeFeatureTags() override;

    void CheckSuspended() override;
    void SetSuspendedReason(IN IMS_UINT32 nReason) override;
    void ResetSuspendedReason(IN IMS_UINT32 nReason) override;

    void Init() override;
    void CleanUp() override;

    void AddListeners() override;
    void RemoveListeners() override;

    IMS_BOOL IsHandleBlocked() const override;
    IMS_BOOL IsFeatureBlocked(IN IMS_UINT32 nFeature) const override;

    void ProcessFeatureBlock(IN IMS_UINT32 nFeature, IN IMS_BOOL bBlocked) override;
    void ProcessBlockChanged() override;
    void ProcessCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& objNewCapabilities) override;
    void ProcessDataConnectionChanged() override;
    void ProcessNetworkChanged() override;
    void ProcessVopsStateChanged(
            IN IMS_UINT32 nState, IN IMS_BOOL bUpdateState = IMS_TRUE) override;

    void ReevaluateCapabilities(IN IMS_BOOL bNetworkChanged) override;
    void ReevaluateUnavailableFeature() override;

    // IAosHandle
    void Request(IN IMS_UINT32 nType, IN IMS_UINT32 nState = 0) override;

    void UpdateGGsmaRcsTelephonyFeatureTag();

    IMS_UINT32 GetVoiceBlockReasonForIpcan();
    IMS_UINT32 GetVideoBlockReasonForIpcan();

    IMS_BOOL IsCsFeatureTagRequired() const;
    IMS_BOOL IsInvalidMobileNetwork() const;
    IMS_BOOL IsPlmnBlockCondition() const;

    IMS_BOOL ProcessHoldingVopsState(IN IMS_UINT32 nState);
    IMS_BOOL ProcessHoldingSsacState(IN IMS_SINT32 nBarringFactorForVoice);

    void ProcessVolteHysTimerExpired();

    // Timer
    IMS_BOOL StartVolteHysTimer(IN IMS_UINT32 nDuration);
    void StopVolteHysTimer();
    IMS_BOOL IsVolteHysTimerRunning() const;

    // IAosNConfigurationListener
    void NConfiguration_NotifyConfigChanged() override;

    // IImsRadioSsacListener
    void ImsRadio_OnSsacChanged(IN const SsacInfo& objSsacInfo) override;

    // IAosServicePhoneListener
    void ServicePhone_PlmnChanged() override;

    // ITimerListener
    void Timer_TimerExpired(IN ITimer* piTimer) override;

protected:
    IImsRadio* m_piImsRadio;
    ITimer* m_piVolteHysTimer;
    IMS_BOOL m_bSsacBarred;
    IMS_BOOL m_bSsacHeld;
    IMS_BOOL m_bB2cCallComposerCapable;
};
#endif  // AOS_HANDLE_MTC_H_
