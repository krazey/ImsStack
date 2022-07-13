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

#ifndef MOCK_I_AOS_N_CONFIGURATION_H_
#define MOCK_I_AOS_N_CONFIGURATION_H_

#include <gmock/gmock.h>

#include "IMSTypeDef.h"
#include "interface/IAosNConfiguration.h"

class MockIAosNConfiguration : public IAosNConfiguration
{
public:
    MOCK_METHOD(IMS_SINT32, GetSlotId, (), (const, override));
    MOCK_METHOD(void, SetListener, (IN IAosNConfigurationListener*), (override));
    MOCK_METHOD(void, RemoveListener, (IN IAosNConfigurationListener*), (override));

    MOCK_METHOD(IMS_BOOL, IsSubscription, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUnSubscription, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVoLteAvailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVoLteRoamingAvailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVtAvailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsDataEnableChangeIgnoredForVideoCalls, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWfcImsAvailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWfcRoamingEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsImsSingleRegistrationRequired, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRttSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportLimitedAdminSmsMode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsTtySupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVopsIgnoredForVolteEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSmsOverImsAvailableWithoutVoiceCapability, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredEmergencyRegistrationInRoaming, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredVolteBlockBySetting, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredVolteBlockByAirplaneMode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredWfcBlockByAirplaneMode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, UseWfcCountryCodeAvailabilityCheck, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegistrationRetryIntervalsUsedForSubscription, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSmsOverIpEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIpsecEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSecurityServerPortInRegContactOfInitialRegistrationUsed, (),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsSecurityServerPortInInitialRegistrationUsed, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsOldSaOnEstablishingSaRemoved, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyPdnWithEmergencyCallEndReleased, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSmsOverImsSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsImsOverNrEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVerstatForRegistrationSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallBasedOnPauOfNormalRegistrationSupported, (),
            (const, override));
    MOCK_METHOD(
            IMS_BOOL, IsRegistrationWhenIpcanChangedWithImsActiveCallHeld, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsDeregisterOn3gNetworks, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoOverWifiSupportedWithoutVoice, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsGeolocationPidfSupported, (IN IMS_SINT32 nGeolocationPidfType),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType, (),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsCdmalessFeatureTagRequired, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegErrCodeWithRetryAfterTimeOnlyDefined, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSpecificRegErrRetryCountSharedForRegAndRegEventRequired, (),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegistrationEventForCatRequired, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallbackModeSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencySmsOverImsSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsContactUriValidationChecked, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUserInfoInContactSupported, (), (const, override));

    MOCK_METHOD(IMS_UINT32, GetRegistrationRetryBaseTime, (), (override));
    MOCK_METHOD(IMS_UINT32, GetRegistrationRetryMaxTime, (), (override));
    MOCK_METHOD(IMS_UINT32, GetIsimIndexForImpu, (), (override));
    MOCK_METHOD(IMS_SINT32, GetUssdMethod, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPreferredIpType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyPreferredIpType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPcscfPort, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSipPreferredTransport, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetIpv4MtuSize, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetIpv6MtuSize, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPreferredEmergencyRegistration, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyRegistrationTimerMillis, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationRetryDefaultPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPreferredImsDscp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetImsSignallingDscp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationPreferredAccessTypeFeatureTag, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationPrivateHeader, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationActualWaitTimePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSipMessageThresholdForTransportChange, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationRetrySip305CodePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetReregistrationRetrySip305CodePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationRetrySip503CodePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSpecificRegistrationErrorFinalType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSpecificRegistrationErrorPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSpecificRegistrationErrorMaxCount, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetryCountResetPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetReregRetryMaxCountKeptRegistration, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationPcscfUpdatePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetUserInfoPolicyForNonRegisterMessage, (), (const, override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRegistrationRetryIntervals, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRegistrationRandomRetryIntervals, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetIpsecAuthenticationAlgorithms, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetIpsecEncryptionAlgorithms, (), (override));
    MOCK_METHOD(IMS_UINT32, GetNotifyEventForInitialRegistration, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetNotifyWaitTime, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetNotifyEventForInitialRegWithWaitTime, (), (const, override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSubErrorRegRequired, (), (override));
    MOCK_METHOD(IMS_SINT32, GetRetryCountSubErrorRegRequired, (), (const, override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSubErrorRegRequiredWithNextPcscf, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSubErrorSubTerminated, (), (override));
    MOCK_METHOD(IMS_SINT32, GetRetryCountSubErrorSubTerminated, (), (const, override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSubErrorStoppingResub, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetVowifiSubErrorRegRequired, (), (override));
    MOCK_METHOD(IMS_UINT32, GetClearReasonForPermanentPdnFailure, (), (const, override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetImsIdentityPriority, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetPcscfDiscoveryMethod, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRoamingPcscfDiscoveryMethod, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetUpdateRegistrationWithRatChange, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSupportedRats, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSupportedRoamingRats, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSmsOverImsSupportedRats, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSpecificRegErrNumMultipliedByPcscfNum, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSpecificRegistrationErrorCode, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSpecificReregistrationErrorCode, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetSpecificRegErrWaitTime, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetReregRetryErrCodeWithInitialRegWithSamePcscf, (),
            (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRegPermanentErrCode, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRegPermanentErrMaxCount, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRegErrCodeWithRetryAfterTime, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetReregErrCodeWithRetryAfterTime, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRegWithFeatureTagUnavailable, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetRegWithFeatureTagUnavailablePolicy, (), (override));
    MOCK_METHOD(IMSVector<IMS_SINT32>&, GetEmergencyPcscfRetryWaitTime, (), (override));

    MOCK_METHOD(void, Init, (IN IMS_SINT32 nSlotId), (override));
};

#endif  // MOCK_I_AOS_N_CONFIGURATION_H_
