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

#include "ImsTypeDef.h"
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
    MOCK_METHOD(IMS_BOOL, IsWfcImsAvailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsImsSingleRegistrationRequired, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRttSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSupportLimitedAdminSmsMode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsTtySupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVopsIgnoredForVolteEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSmsOverImsAvailableWithoutVoiceCapability, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredVolteBlockBySsac, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRequiredWfcBlockByAirplaneMode, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsReregRetryWithChangedCountryOnWifi, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSipOverIpsecInRoamingEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, UseWfcCountryCodeAvailabilityCheck, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegRetryIntervalsUsedForSub, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSmsOverIpEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIpsecEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSecurityServerPortInRegContactOfInitRegUsed, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSecurityServerPortInInitRegUsed, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsOldSaOnEstablishingSaRemoved, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsCallEndAndPdnReactivationByRegTerminated, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUnsecureTcpSocketOnAccomplishingRegDestroyed, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyPdnWithEmergencyCallEndReleased, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsSmsOverImsSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsImsOverNrEnabled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallBasedOnPauOfNormalRegistrationSupported, (),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmcRegOnRandomPcscf, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegWithIpcanChangedDuringImsCallHeld, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsDeregOn3gNetwork, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsIpsecInitializedWithNewPcscf, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsNoInitRegOnPcscfChange, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVideoOverWifiSupportedWithoutVoice, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsGeolocationPidfSupported, (IN IMS_SINT32 nGeolocationPidfType),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType, (),
            (const, override));
    MOCK_METHOD(IMS_BOOL, IsCdmalessFeatureTagRequired, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegErrCodeWithRetryAfterTimeOnlyDefined, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsExtraReregErrInRoamingAsFailureHandled, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsExtraRegErrRetryCntSharedForRegAndSubRequired, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencyCallbackModeSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsEmergencySmsOverImsSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsContactUriValidationChecked, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegRetryWithIpVerFallback, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsUserInfoInContactSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegRequiredAfterImsCallEndOnRegHeld, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsRegWithFeatureTagUnavailableSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsVerstatForRegistrationSupported, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsPlmnBlockWithTimeoutOnVoiceCallUnavailable, (), (const, override));
    MOCK_METHOD(IMS_BOOL, IsWfcErrorMessageSupported, (IN IMS_SINT32 nError), (const, override));

    MOCK_METHOD(IMS_UINT32, GetRegistrationRetryBaseTime, (), (override));
    MOCK_METHOD(IMS_UINT32, GetRegistrationRetryMaxTime, (), (override));
    MOCK_METHOD(IMS_UINT32, GetIsimIndexForImpu, (), (override));
    MOCK_METHOD(IMS_SINT32, GetImsEstablishmentTime, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPreferredImsDscp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationPreferredAccessTypeFeatureTag, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetUssdMethod, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPreferredIpType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyPreferredIpType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmcRegRetryMaxCnt, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmcRegRetryTimerMillis, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPcscfPort, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSipPreferredTransport, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetIpv4MtuSize, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetIpv6MtuSize, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetPreferredEmergencyRegistration, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetEmergencyRegistrationTimerMillis, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetImsSignallingDscp, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationPrivateHeader, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegActualWaitTimePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegOutOfServicePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRoamingPreferredEmcReg, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetSipMessageThresholdForTransportChange, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetUsatRegEventDownloadPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetVolteHysTime, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetrySip305CodePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetReregRetrySip305CodePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetrySip503CodePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetryCountOnSinglePcscf, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetryCountPerPcscf, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetryCountResetPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetryCountWithIpsecOnAuthFailure, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetryDefaultPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegRetryTimerFPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegTimerForEmcCall, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetExtraRegErrFinalType, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetExtraRegErrPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetExtraRegErrMaxCount, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetExtraRegErrMinCount, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetRegistrationPcscfUpdatePolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetUserInfoPolicyForNonRegisterMessage, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetGeolocationPidfFormingPolicy, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached, (),
            (const, override));
    MOCK_METHOD(IMS_SINT32, GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached, (),
            (const, override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetRegRetryIntervals, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetRegRandomRetryIntervals, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetIpsecAuthenticationAlgorithms, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetIpsecEncryptionAlgorithms, (), (override));
    MOCK_METHOD(IMS_UINT32, GetNotifyEventForInitialRegistration, (), (const, override));
    MOCK_METHOD(IMS_SINT32, GetNotifyWaitTime, (), (const, override));
    MOCK_METHOD(IMS_UINT32, GetNotifyEventForInitialRegWithWaitTime, (), (const, override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetSubErrorRegRequired, (), (override));
    MOCK_METHOD(IMS_SINT32, GetRetryCountSubErrorRegRequired, (), (const, override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetSubErrorRegRequiredWithNextPcscf, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetSubErrorSubTerminated, (), (override));
    MOCK_METHOD(IMS_SINT32, GetRetryCountSubErrorSubTerminated, (), (const, override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetSubErrorStoppingResub, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetVowifiSubErrorRegRequired, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetImsIdentityPriority, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetPcscfDiscoveryMethod, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetUpdateRegistrationWithRatChange, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetSupportedRats, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetSupportedRoamingRats, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetSmsOverImsSupportedRats, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetExtraRegErrCode, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetExtraReregErrCode, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetExtraRegErrWaitTime, (), (override));
    MOCK_METHOD(
            ImsVector<IMS_SINT32>&, GetReregRetryErrCodeForInitRegWithSamePcscf, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetRegPermanentErrCode, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetRegPermanentErrMaxCount, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetRegErrCodeWithoutIpsec, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetRegErrCodeWithRetryAfterTime, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetReregErrCodeWithRetryAfterTime, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetEmergencyPcscfRetryWaitTime, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetRegErrCodeForPcscfDiscovery, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetReregErrCodeForCallEnd, (), (override));
    MOCK_METHOD(
            ImsVector<IMS_SINT32>&, GetReregErrCodeForInitRegWithAvailablePcscf, (), (override));
    MOCK_METHOD(ImsVector<IMS_SINT32>&, GetReregErrCodeForImsPdnReactivation, (), (override));
    MOCK_METHOD(void, Init, (IN IMS_SINT32 nSlotId), (override));
};

#endif  // MOCK_I_AOS_N_CONFIGURATION_H_
