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

#ifndef MOCK_I_AOS_SERVICE_H_
#define MOCK_I_AOS_SERVICE_H_

#include <gmock/gmock.h>

#include "AString.h"
#include "IAosService.h"
#include "ImsMap.h"
#include "interface/IAosEmergencyListener.h"
#include "interface/IAosServicePhoneListener.h"
#include "interface/IAosServiceSettingListener.h"

class MockIAosService : public IAosService
{
public:
    MOCK_METHOD(void, AddListener, (IN IAosEmergencyListener * piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosEmergencyListener * piListener), (override));
    MOCK_METHOD(void, AddListener, (IN IAosRegistrationControlListener * piListener), (override));
    MOCK_METHOD(
            void, RemoveListener, (IN IAosRegistrationControlListener * piListener), (override));
    MOCK_METHOD(void, AddListener, (IN IAosServiceSettingListener * piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosServiceSettingListener * piListener), (override));
    MOCK_METHOD(void, AddListener, (IN IAosServicePhoneListener * piListener), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosServicePhoneListener * piListener), (override));
    MOCK_METHOD(void, NotifyEmergencyCallbackModeChanged,
            (IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_ULONG nDuration), (override));
    MOCK_METHOD(void, UpdateSipDelegateRegistration, (), (override));
    MOCK_METHOD(void, TriggerSipDelegateDeregistration, (), (override));
    MOCK_METHOD(void, TriggerFullNetworkRegistration,
            (IN IMS_SINT32 nSipCode, IN const AString& sipReason), (override));
    MOCK_METHOD(void, NotifyCapabilitiesChanged,
            ((IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities)), (override));
    MOCK_METHOD(void, ControlRegistration,
            (IN IMS_SINT32 nRequestType, IN IMS_SINT32 nPcscfOrder, IN IMS_SINT32 nCause),
            (override));
    MOCK_METHOD(void, UpdateDataFailureReason, (IN IMS_SINT32 nReason), (override));
    MOCK_METHOD(void, NotifyAirplaneSetting, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyDataRoamingSetting, (IN IMS_UINT32 nIsAllowed), (override));
    MOCK_METHOD(void, NotifyMobileDataSetting, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyRoamingPreferredVoiceNetwork, (IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, NotifyServiceSetting, (IN IMS_UINT32 nState, IN IMS_UINT32 nServiceBits),
            (override));
    MOCK_METHOD(void, NotifyTtySetting, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyVideoSetting, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyVolteSetting, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyWfcSetting, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyWifiSetting, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyAosStart, (), (override));
    MOCK_METHOD(void, NotifyIpcanHandoverFailure,
            (IN IMS_SINT32 nTargetNetwork, IN IMS_SINT32 nCauseCode), (override));
    MOCK_METHOD(void, NotifyIsimState, (IN IMS_SINT32 nState), (override));
    MOCK_METHOD(void, NotifyLocationInfo, (IN IMS_UINT32 nState), (override));
    MOCK_METHOD(void, NotifyMobileDataLimit, (IN IMS_UINT32 nIsLimited), (override));
    MOCK_METHOD(void, NotifyNetworkVideoCapability, (IN IMS_UINT32 nIsOn), (override));
    MOCK_METHOD(void, NotifyPhoneNumberState, (IN IMS_UINT32 nIsRefresh, IN IMS_UINT32 nState),
            (override));
    MOCK_METHOD(void, NotifyPlmnChanged, (IN const AString& strPlmn), (override));
    MOCK_METHOD(void, NotifyVopsStateChanged, (IN IMS_UINT32 nState, IN const AString& strPlmn),
            (override));
    MOCK_METHOD(void, NotifyPowerOff, (), (override));
    MOCK_METHOD(void, NotifyPreciseCallState, (IN IMS_SINT32 nState), (override));
    MOCK_METHOD(void, NotifyCarrierSignalPcoValueChanged, (IN IMS_SINT32 nValue), (override));
    MOCK_METHOD(void, NotifyCrossSimStatus, (IN IMS_SINT32 nIsConnected), (override));
    MOCK_METHOD(void, NotifyNasSecurityAlgorithmChanged, (IN IMS_UINT32 nIsNullAlgo), (override));
    MOCK_METHOD(void, NotifyAllowedNetworkTypesChanged, (IN IMS_ULONG nNetworkTypesBitMask),
            (override));
    MOCK_METHOD(void, NotifyEmergencyRegistrationStateChanged, (IN IMS_UINT32 nIsEmergencyAttached),
            (override));
    MOCK_METHOD(void, NotifySimStateChanged, (IN IMS_SINT32 nSimState), (override));

    MOCK_METHOD(IMS_BOOL, NotifyRegistered,
            (IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
                    IN const ImsList<AString>& objFeatureTags),
            (override));
    MOCK_METHOD(IMS_BOOL, NotifyRegistering,
            (IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits,
                    IN const ImsList<AString>& objFeatureTags),
            (override));
    MOCK_METHOD(IMS_BOOL, NotifyDeregistered,
            (IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN AosReasonCode eReason,
                    IN IMS_SINT32 nDataFailureReason),
            (override));
    MOCK_METHOD(IMS_BOOL, NotifyDeregistering, (IN IMS_SINT32 nRegType), (override));
    MOCK_METHOD(IMS_BOOL, NotifyTechnologyChangeFailed,
            (IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN AosReasonCode eReason),
            (override));
    MOCK_METHOD(
            IMS_BOOL, NotifyAssociatedUriChanged, (IN const ImsList<AString>& objUris), (override));
    MOCK_METHOD(IMS_BOOL, NotifyCapabilitiesUpdateFailed,
            (IN AosCapability eCapabilities, IN AosNetworkType eNetworkType,
                    IN AosReasonCode eReason),
            (override));
    MOCK_METHOD(IMS_BOOL, NotifyAosIsimState, (IN AosIsimState eState), (override));
    MOCK_METHOD(IMS_BOOL, NotifyRegEventState,
            (IN IMS_UINT32 nStatusCode, IN const ImsList<AString>& objImpus), (override));
    MOCK_METHOD(IMS_BOOL, NotifyImsFeatureChanged,
            (IN IMS_SINT32 nRegType, IN AosNetworkType eNetworkType, IN IMS_UINT32 nFeatureTagBits),
            (override));
    MOCK_METHOD(IMS_BOOL, NotifyTrace, (IN AosRegistrationType eType, IN const AString& strLog),
            (override));
    MOCK_METHOD(IMS_BOOL, RequestPhoneNumberRetry, (IN AosPhoneNumberRetryCommand eCommand),
            (override));
    MOCK_METHOD(IMS_BOOL, RequestWifiService, (IN IMS_BOOL bIsOn), (override));
    MOCK_METHOD((ImsMap<IMS_UINT32, IMS_UINT32>&), GetCapabilities, (), (override));
    MOCK_METHOD(IMS_UINT32, GetCapabilitiesForNetwork, (AosNetworkType eNetworkType), (override));
    MOCK_METHOD(IMS_BOOL, IsSupportCapabilitiesForNetwork,
            (AosNetworkType eNetworkType, AosCapability eCapability), (override));
    MOCK_METHOD(IMS_BOOL, IsNasSecurityAlgorithmNull, (), (override));

    MOCK_METHOD(void, NotifyJniEnablerSet, (), (override));
};

#endif  // MOCK_I_AOS_SERVICE_H_
