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
            bCdmalessFeatureTagRequired(IMS_FALSE),
            bDestroyUnsecureTcpSocketOnAccomplishingRegistration(IMS_FALSE),
            bDisableT3482ForEmergency(IMS_FALSE),
            bEmergencyCallBasedOnPAssociatedUriOfNormalRegistration(IMS_FALSE),
            bHoldRegistrationWhenIpcanChangedWithImsActiveCall(IMS_FALSE),
            bIgnoreVopsForVolteEnable(IMS_FALSE),
            bImsDeregisterOn3gNetworks(IMS_FALSE),
            bNoInitialRegistrationOnPcscfChange(IMS_FALSE),
            bRegistrationContactValidation(IMS_FALSE),
            bRemoveOldSaOnEstablishingSa(IMS_FALSE),
            bRequiredEmergencyRegistrationInRoaming(IMS_FALSE),
            bRequiredVolteBlockBySetting(IMS_FALSE),
            bRequiredVolteBlockByAirplaneMode(IMS_FALSE),
            bRequiredWfcBlockByAirplaneMode(IMS_FALSE),
            bRequireRegAfterImsCallEndOnRegHeld(IMS_FALSE),
            bSipOverIpsecEnabledInRoaming(IMS_TRUE),
            bSmsOverImsAvailableWithoutVoiceCapability(IMS_FALSE),
            bSupportContactUserInfo(IMS_TRUE),
            bSupportRegWithFeatureTagUnavailable(IMS_FALSE),
            bSupportVerstatForRegistration(IMS_FALSE),
            bUpdateRegistrationWithCountryChange(IMS_FALSE),
            bUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType(IMS_FALSE),
            bUseSecurityServerPortInInitialRegistration(IMS_FALSE),
            bUseSecurityServerPortInRegContactOfInitialRegistration(IMS_FALSE),
            bUseWfcCountryCodeAvailabilityCheck(IMS_FALSE),
            bVideoOverWifiSupportedWithoutVoice(IMS_FALSE),
            nContactUserInfoPolicyForNonRegisterMessage(
                    CarrierConfig::Assets::CONTACT_USER_INFO_POLICY_DEFAULT),
            nEmergencyPreferredIpType(CarrierConfig::Assets::IP_VERSION_6),
            nGeolocationPidfFormingPolicy(
                    CarrierConfig::Assets::GEOLOCATION_FORMING_POLICY_WITHOUT_POSITION),
            nImsPreferredIpType(CarrierConfig::Assets::IP_VERSION_6),
            nImsSignallingDscp(46),
            nRegistrationActualWaitTimePolicy(CarrierConfig::Assets::AWT_POLICY_RFC_RULE),
            nRegistrationOutOfServicePolicy(CarrierConfig::Assets::REGISTRATION_OOS_POLICY_DEFAULT),
            nRegistrationPcscfUpdatePolicy(0),
            nRegistrationRetryCountResetPolicy(0),
            nRegistrationTimerForEmergencyCallMillis(0),
            nSipMessageThresholdForTransportChange(200),
            objClearPermanentPdnFailure(IMSVector<IMS_SINT32>()),
            objEmergencyPcscfRetryWaitTimeSec(IMSVector<IMS_SINT32>()),
            objPcscfDiscoveryMethodRoaming(IMSVector<IMS_SINT32>()),
            objRegErrorCodesWithPcscfDiscovery(IMSVector<IMS_SINT32>()),
            objRegistrationPermanentErrorMaxCount(IMSVector<IMS_SINT32>()),
            objReregErrorCodesWithImsPdnReactivation(IMSVector<IMS_SINT32>()),
            objReregErrorCodesWithInitRegWithAvailablePcscf(IMSVector<IMS_SINT32>()),
            objSubscriptionErrorCodeForRegEventWithInitialRegistrationWithNextPcscf(
                    IMSVector<IMS_SINT32>()),
            objSubscriptionErrorCodeForStoppingByExpirationTime(IMSVector<IMS_SINT32>()),
            objSupportedRoamingRats(IMSVector<IMS_SINT32>()),
            objVowifiSubscriptionErrorCodeWithInitialRegistration(IMSVector<IMS_SINT32>()),
            objWfcRegEventErrorByMissing911Address(IMSVector<IMS_SINT32>())
    {
    }

    AosAsset(IN const AosAsset&) = delete;
    AosAsset& operator=(IN const AosAsset&) = delete;

public:
    IMS_BOOL bCdmalessFeatureTagRequired;
    IMS_BOOL bDestroyUnsecureTcpSocketOnAccomplishingRegistration;
    IMS_BOOL bDisableT3482ForEmergency;
    IMS_BOOL bEmergencyCallBasedOnPAssociatedUriOfNormalRegistration;
    IMS_BOOL bHoldRegistrationWhenIpcanChangedWithImsActiveCall;
    IMS_BOOL bIgnoreVopsForVolteEnable;
    IMS_BOOL bImsDeregisterOn3gNetworks;
    IMS_BOOL bNoInitialRegistrationOnPcscfChange;
    IMS_BOOL bRegistrationContactValidation;
    IMS_BOOL bRemoveOldSaOnEstablishingSa;
    IMS_BOOL bRequiredEmergencyRegistrationInRoaming;
    IMS_BOOL bRequiredVolteBlockBySetting;
    IMS_BOOL bRequiredVolteBlockByAirplaneMode;
    IMS_BOOL bRequiredWfcBlockByAirplaneMode;
    IMS_BOOL bRequireRegAfterImsCallEndOnRegHeld;
    IMS_BOOL bSipOverIpsecEnabledInRoaming;
    IMS_BOOL bSmsOverImsAvailableWithoutVoiceCapability;
    IMS_BOOL bSupportContactUserInfo;
    IMS_BOOL bSupportRegWithFeatureTagUnavailable;
    IMS_BOOL bSupportVerstatForRegistration;
    IMS_BOOL bUpdateRegistrationWithCountryChange;
    IMS_BOOL bUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType;
    IMS_BOOL bUseSecurityServerPortInInitialRegistration;
    IMS_BOOL bUseSecurityServerPortInRegContactOfInitialRegistration;
    IMS_BOOL bUseWfcCountryCodeAvailabilityCheck;
    IMS_BOOL bVideoOverWifiSupportedWithoutVoice;
    IMS_SINT32 nContactUserInfoPolicyForNonRegisterMessage;
    IMS_SINT32 nEmergencyPreferredIpType;
    IMS_SINT32 nGeolocationPidfFormingPolicy;
    IMS_SINT32 nImsPreferredIpType;
    IMS_SINT32 nImsSignallingDscp;
    IMS_SINT32 nRegistrationActualWaitTimePolicy;
    IMS_SINT32 nRegistrationOutOfServicePolicy;
    IMS_SINT32 nRegistrationPcscfUpdatePolicy;
    IMS_SINT32 nRegistrationRetryCountResetPolicy;
    IMS_SINT32 nRegistrationTimerForEmergencyCallMillis;
    IMS_SINT32 nSipMessageThresholdForTransportChange;
    IMSVector<IMS_SINT32> objClearPermanentPdnFailure;
    IMSVector<IMS_SINT32> objEmergencyPcscfRetryWaitTimeSec;
    IMSVector<IMS_SINT32> objPcscfDiscoveryMethodRoaming;
    IMSVector<IMS_SINT32> objRegErrorCodesWithPcscfDiscovery;
    IMSVector<IMS_SINT32> objRegistrationPermanentErrorMaxCount;
    IMSVector<IMS_SINT32> objReregErrorCodesWithImsPdnReactivation;
    IMSVector<IMS_SINT32> objReregErrorCodesWithInitRegWithAvailablePcscf;
    IMSVector<IMS_SINT32> objSubscriptionErrorCodeForRegEventWithInitialRegistrationWithNextPcscf;
    IMSVector<IMS_SINT32> objSubscriptionErrorCodeForStoppingByExpirationTime;
    IMSVector<IMS_SINT32> objSupportedRoamingRats;
    IMSVector<IMS_SINT32> objVowifiSubscriptionErrorCodeWithInitialRegistration;
    IMSVector<IMS_SINT32> objWfcRegEventErrorByMissing911Address;
};
#endif  // AOS_ASSET_H_
