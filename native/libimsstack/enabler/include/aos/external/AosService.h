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
#ifndef AOS_SERVICE_H_
#define AOS_SERVICE_H_

#include "IAosService.h"

class IAosRegistrationControlListener;
class IAosServiceSettingListener;
class IAosServicePhoneListener;
class IJniAosServiceThread;

class AosService : public IAosService
{
public:
    explicit AosService(IN IMS_SINT32 nSlotId);
    virtual ~AosService();

    IMS_BOOL AddListener(IN IAosRegistrationControlListener* piListener) override;
    IMS_BOOL RemoveListener(IN IAosRegistrationControlListener* piListener) override;

    IMS_BOOL AddListener(IN IAosServiceSettingListener* piListener) override;
    IMS_BOOL RemoveListener(IN IAosServiceSettingListener* piListener) override;

    IMS_BOOL AddListener(IN IAosServicePhoneListener* piListener) override;
    IMS_BOOL RemoveListener(IN IAosServicePhoneListener* piListener) override;

    /// Java -> Native
    void UpdateSipDelegateRegistration() override;
    void TriggerSipDelegateDeregistration() override;
    void TriggerFullNetworkRegistration(
            IN IMS_SINT32 nSipCode, IN const AString& sipReason) override;
    void NotifyCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities) override;
    void ControlRegistration(
            IN IMS_SINT32 nRequestType, IN IMS_SINT32 nPcscfOrder, IN IMS_SINT32 nCause) override;

    void NotifyAirplaneSetting(IN IMS_UINT32 nIsOn) override;
    void NotifyDataRoamingSetting(IN IMS_UINT32 nIsAllowed) override;
    void NotifyMobileDataSetting(IN IMS_UINT32 nIsOn) override;
    void NotifyRoamingPreferredVoiceNetwork(IN IMS_UINT32 nState) override;
    void NotifyServiceSetting(IN IMS_UINT32 nState, IN IMS_UINT32 nServiceBits) override;
    void NotifyTtySetting(IN IMS_UINT32 nIsOn) override;
    void NotifyVideoSetting(IN IMS_UINT32 nIsOn) override;
    void NotifyVolteSetting(IN IMS_UINT32 nIsOn) override;
    void NotifyWfcSetting(IN IMS_UINT32 nIsOn) override;

    void NotifyAosStart() override;
    void NotifyIpcanHandoverFailure(
            IN IMS_SINT32 nTargetNetwork, IN IMS_SINT32 nCauseCode) override;
    void NotifyIsimState(IN IMS_UINT32 nState) override;
    void NotifyLocationInfo(IN IMS_UINT32 nState) override;
    void NotifyMobileDataLimit(IN IMS_UINT32 nIsLimited) override;
    void NotifyNetworkVideoCapability(IN IMS_UINT32 nIsOn) override;
    void NotifyPhoneNumberState(IN IMS_UINT32 nIsRefresh, IN IMS_UINT32 nState) override;
    void NotifyPlmnChanged() override;
    void NotifyPowerOff() override;
    void NotifyPreciseCallState(IN IMS_SINT32 nState) override;
    void NotifyCarrierSignalPcoValueChanged(IN IMS_SINT32 nValue) override;

    // Native -> Java
    IMS_BOOL NotifyRegistered(IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const ImsList<AString>& objFeatureTags) override;

    IMS_BOOL NotifyRegistering(IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const ImsList<AString>& objFeatureTags) override;

    IMS_BOOL NotifyDeregistered(IN AosNetworkType eNetworkType, IN AosReasonCode eReason) override;
    IMS_BOOL NotifyTechnologyChangeFailed(
            IN AosNetworkType eNetworkType, IN IMS_SINT32 nCauseCode) override;
    IMS_BOOL NotifyAssociatedUriChanged(IN const ImsList<AString>& objUris) override;
    IMS_BOOL NotifyCapabilitiesUpdateFailed(IN AosCapability eCapabilities,
            IN AosNetworkType eNetworkType, IN AosReasonCode eReason) override;

    IMS_BOOL NotifyAosIsimState(IN AosIsimState eState) override;
    IMS_BOOL NotifyRegEventState(IN AosRegEvent eState) override;
    IMS_BOOL RequestPhoneNumberRetry(IN AosPhoneNumberRetryCommand eCommand) override;
    IMS_BOOL RequestWifiService(IN IMS_BOOL bIsOn) override;

    ImsMap<IMS_UINT32, IMS_UINT32>& GetCapabilities() override;
    IMS_UINT32 GetCapabilitiesForNetwork(AosNetworkType eNetworkType) override;
    IMS_BOOL IsSupportCapabilitiesForNetwork(
            AosNetworkType eNetworkType, AosCapability eCapability) override;

    inline void NotifyJniEnablerSet() override {}

    static AString PrintCapabilities(IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities);
    static const IMS_CHAR* NetworkTypeToString(IN IMS_SINT32 nType);
    static const AString CapabilitiesToString(IN IMS_UINT32 nCapabilities);

private:
    void Init();
    void CleanUp();
    void Attach();
    IJniAosServiceThread* GetJniThread();

private:
    IMS_SINT32 m_nSlotId;
    AString m_strTag;

    ImsList<IAosRegistrationControlListener*> m_objAosRegistrationControlListeners;
    ImsList<IAosServiceSettingListener*> m_objAosServiceSettingListeners;
    ImsList<IAosServicePhoneListener*> m_objAosServicePhoneListeners;

    // <AosNetworkType, AosCapability>
    ImsMap<IMS_UINT32, IMS_UINT32> m_objCapabilities;
};

#endif  // AOS_SERVICE_H_
