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

#include "interface/IAosService.h"

class IAosRegistrationControlListener;
class IAosServiceSettingListener;
class IAosServicePhoneListener;
class JniAosService;
class JniAosServiceThread;

class AosService : public IAosService
{
public:
    AosService(IN IMS_SINT32 nSlotId);
    virtual ~AosService();

    void SetJniAosService(IN JniAosService* pJniAosService) override;

    void AddListener(IN IAosRegistrationControlListener* piListener) override;
    void RemoveListener(IN IAosRegistrationControlListener* piListener) override;

    void AddListener(IN IAosServiceSettingListener* piListener) override;
    void RemoveListener(IN IAosServiceSettingListener* piListener) override;

    void AddListener(IN IAosServicePhoneListener* piListener) override;
    void RemoveListener(IN IAosServicePhoneListener* piListener) override;

    /// Java -> Native
    void UpdateSipDelegateRegistration() override;
    void TriggerSipDelegateDeregistration() override;
    void TriggerFullNetworkRegistration(
            IN IMS_SINT32 nSipCode, IN const AString& sipReason) override;
    void NotifyCapabilitiesChanged(
            IN const IMSMap<IMS_UINT32, IMS_UINT32>& objCapabilities) override;
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

    // Native -> Java
    void NotifyRegistered(IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const IMSList<AString>& objFeatureTags) override;

    void NotifyRegistering(IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
            IN const IMSList<AString>& objFeatureTags) override;

    void NotifyDeregistered(IN AosReasonCode eReason) override;
    void NotifyTechnologyChangeFailed(
            IN AosNetworkType eNetworkType, IN IMS_SINT32 nCauseCode) override;
    void NotifyAssociatedUriChanged(IN const IMSList<AString>& objUris) override;
    void NotifyCapabilitiesUpdateFailed(IN AosCapability eCapabilities,
            IN AosNetworkType eNetworkType, IN AosReasonCode eReason) override;

    void NotifyAosIsimState(IN AosIsimState eState) override;
    void NotifyRegEventState(IN AosRegEvent eState) override;
    void RequestPhoneNumberRetry(IN AosPhoneNumberRetryCommand eCommand) override;
    void RequestWifiService(IN IMS_BOOL bIsOn) override;

    IMSMap<IMS_UINT32, IMS_UINT32>& GetCapabilities() override;
    IMS_UINT32 GetCapabilitiesForNetwork(AosNetworkType eNetworkType) override;
    IMS_BOOL IsSupportCapabilitiesForNetwork(
            AosNetworkType eNetworkType, AosCapability eCapability) override;

private:
    void Init();
    void CleanUp();
    IMS_BOOL Attach();
    static void PrintCapabilities(IN const IMSMap<IMS_UINT32, IMS_UINT32>& objCapabilities);
    static const IMS_CHAR* NetworkTypeToString(IN IMS_SINT32 nType);
    static const AString CapabilitiesToString(IN IMS_UINT32 nCapabilities);

private:
    IMS_SINT32 m_nSlotId;
    AString m_strTag;

    IMSList<IAosRegistrationControlListener*> m_objAosRegistrationControlListeners;
    IMSList<IAosServiceSettingListener*> m_objAosServiceSettingListeners;
    IMSList<IAosServicePhoneListener*> m_objAosServicePhoneListeners;

    // <AosNetworkType, AosCapability>
    IMSMap<IMS_UINT32, IMS_UINT32> m_objCapabilities;

    JniAosService* m_pJniAosService;
};

#endif  // AOS_SERVICE_H_
