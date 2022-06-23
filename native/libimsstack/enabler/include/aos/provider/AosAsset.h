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
#ifndef AOS_ASSET_H_
#define AOS_ASSET_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "ImsVector.h"
#include "CarrierConfig.h"

struct AosAsset
{
public:
    AosAsset() :
            bUseSecurityServerPortInRegContactOfInitialRegistration(IMS_FALSE),
            bUseSecurityServerPortInInitialRegistration(IMS_FALSE),
            bRemoveOldSaOnEstablishingSa(IMS_FALSE),
            bUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType(IMS_FALSE),
            objPcscfDiscoveryMethodRoaming(IMSVector<IMS_SINT32>()),
            bHoldRegistrationWhenIpcanChangedWithImsActiveCall(IMS_FALSE),
            bSupportVerstatForRegistration(IMS_FALSE),
            bNoInitialRegistrationOnPcscfChange(IMS_FALSE),
            bEmergencyCallBasedOnPAssociatedUriOfNormalRegistration(IMS_FALSE),
            bSipOverIpsecEnabledInRoaming(IMS_TRUE),
            bRegistrationContactValidation(IMS_FALSE),
            nRegistrationOutOfServicePolicy(CarrierConfig::Assets::REGISTRATION_OOS_POLICY_DEFAULT),
            objVowifiSubscriptionErrorCodeWithInitialRegistration(IMSVector<IMS_SINT32>()),
            objSubscriptionErrorCodeForStoppingByExpirationTime(IMSVector<IMS_SINT32>()),
            objSubscriptionErrorCodeForRegEventWithInitialRegistrationWithNextPcscf(
                    IMSVector<IMS_SINT32>()),
            bIgnoreVopsForVolteEnable(IMS_FALSE),
            bSmsOverImsAvailableWithoutVoiceCapability(IMS_FALSE),
            bImsDeregisterOn3gNetworks(IMS_FALSE),
            bVideoOverWifiSupportedWithoutVoice(IMS_FALSE),
            bRequiredEmergencyRegistrationInRoaming(IMS_FALSE),
            objClearPermanentPdnFailure(IMSVector<IMS_SINT32>()),
            bUseWfcCountryCodeAvailabilityCheck(IMS_FALSE),
            bRequiredVolteBlockBySetting(IMS_FALSE),
            bRequiredVolteBlockByAirplaneMode(IMS_FALSE),
            bRequiredWfcBlockByAirplaneMode(IMS_FALSE),
            objSupportedRoamingRats(IMSVector<IMS_SINT32>()),
            nEmergencyPreferredIpType(CarrierConfig::Assets::IP_VERSION_6),
            nSipMessageThresholdForTransportChange(200),
            bCdmalessFeatureTagRequired(IMS_FALSE),
            objEmergencyPcscfRetryWaitTimeSec(IMSVector<IMS_SINT32>()),
            nRegistrationRetryCountResetPolicy(0),
            objRegistrationPermanentErrorMaxCount(IMSVector<IMS_SINT32>()),
            nImsPreferredIpType(CarrierConfig::Assets::IP_VERSION_6),
            nImsSignallingDscp(46),
            nRegistrationActualWaitTimePolicy(CarrierConfig::Assets::AWT_POLICY_RFC_RULE),
            bDisableT3482ForEmergency(IMS_FALSE),
            nRegistrationTimerForEmergencyCallMillis(0),
            bUpdateRegistrationWithCountryChange(IMS_FALSE)
    {
    }

    AosAsset(IN const AosAsset&) = delete;
    AosAsset& operator=(IN const AosAsset&) = delete;

public:
    IMS_BOOL bUseSecurityServerPortInRegContactOfInitialRegistration;
    IMS_BOOL bUseSecurityServerPortInInitialRegistration;
    IMS_BOOL bRemoveOldSaOnEstablishingSa;
    IMS_BOOL bUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType;
    IMSVector<IMS_SINT32> objPcscfDiscoveryMethodRoaming;
    IMS_BOOL bHoldRegistrationWhenIpcanChangedWithImsActiveCall;
    IMS_BOOL bSupportVerstatForRegistration;
    IMS_BOOL bNoInitialRegistrationOnPcscfChange;
    IMS_BOOL bEmergencyCallBasedOnPAssociatedUriOfNormalRegistration;
    IMS_BOOL bSipOverIpsecEnabledInRoaming;
    IMS_BOOL bRegistrationContactValidation;
    IMS_SINT32 nRegistrationOutOfServicePolicy;
    IMSVector<IMS_SINT32> objVowifiSubscriptionErrorCodeWithInitialRegistration;
    IMSVector<IMS_SINT32> objSubscriptionErrorCodeForStoppingByExpirationTime;
    IMSVector<IMS_SINT32> objSubscriptionErrorCodeForRegEventWithInitialRegistrationWithNextPcscf;
    IMS_BOOL bIgnoreVopsForVolteEnable;
    IMS_BOOL bSmsOverImsAvailableWithoutVoiceCapability;
    IMS_BOOL bImsDeregisterOn3gNetworks;
    IMS_BOOL bVideoOverWifiSupportedWithoutVoice;
    IMS_BOOL bRequiredEmergencyRegistrationInRoaming;
    IMSVector<IMS_SINT32> objClearPermanentPdnFailure;
    IMS_BOOL bUseWfcCountryCodeAvailabilityCheck;
    IMS_BOOL bRequiredVolteBlockBySetting;
    IMS_BOOL bRequiredVolteBlockByAirplaneMode;
    IMS_BOOL bRequiredWfcBlockByAirplaneMode;
    IMSVector<IMS_SINT32> objSupportedRoamingRats;
    IMS_SINT32 nEmergencyPreferredIpType;
    IMS_SINT32 nSipMessageThresholdForTransportChange;
    IMS_BOOL bCdmalessFeatureTagRequired;
    IMSVector<IMS_SINT32> objEmergencyPcscfRetryWaitTimeSec;
    IMS_SINT32 nRegistrationRetryCountResetPolicy;
    IMSVector<IMS_SINT32> objRegistrationPermanentErrorMaxCount;
    IMS_SINT32 nImsPreferredIpType;
    IMS_SINT32 nImsSignallingDscp;
    IMS_SINT32 nRegistrationActualWaitTimePolicy;
    IMS_BOOL bDisableT3482ForEmergency;
    IMS_SINT32 nRegistrationTimerForEmergencyCallMillis;
    IMS_BOOL bUpdateRegistrationWithCountryChange;
};
#endif  // AOS_ASSET_H_
