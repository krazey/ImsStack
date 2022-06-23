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

#ifndef ASSET_ITEMS_H_
#define ASSET_ITEMS_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "ImsVector.h"

struct AssetItems
{
public:
    AssetItems() :
            bCheckConferenceEventPackageVersion(IMS_TRUE),
            bConferenceReferToUriSourcePaid(IMS_TRUE),
            nConferenceDropReferToUriSourceType(0),
            bEnableFakeQosCallFlowOnWifi(IMS_FALSE),
            nMediaTypeForOfferlessReinvite(0),
            bSupportVideoCallUpgradeRegardlessOfFeatureTags(IMS_FALSE),
            nOipTypeForUnavailable(1),
            bEnableOipHeaderPolicyFallBack(IMS_FALSE),
            nEmergencyRttGuardTimer(0),
            bRetryEmergencyCallOverEmergencyPdnWithNextPcscf(IMS_FALSE),
            nPreAlertingTimer(0),
            nPolicyForTcallTimerExpiryOfVolteCall(2),
            nPolicyForTcallTimerExpiryOfVolteEmergencyCall(2),
            nPolicyForTcallTimerExpiryOfVowifiCall(0),
            objCarrierSpecificSipHeaders(IMSVector<AString>()),
            bCheckAvchangeFeatureForCallConvertingCapability(IMS_FALSE),
            bSupportRegistrationRecoveryForFailureOfSessionRefresh(IMS_FALSE),
            objCallMaintainingOnRegistrationSuspendeds(IMSVector<IMS_SINT32>()),
            objRequiringEmergencyCallWhenVideoEmergencyCallFaileds(IMSVector<IMS_SINT32>()),
            bUseMcidSupplementaryService(IMS_FALSE),
            bUseMmcSupplementaryService(IMS_FALSE),
            bUseLtePreferredStatusForServiceCapability(IMS_FALSE),
            bAllowIncomingHoldRequestDuringConferenceCall(IMS_FALSE),
            bIgnore180After183Response(IMS_FALSE),
            bAddReplaceHeaderForConference(IMS_FALSE),
            objVilteToVolteRetryFailureResponseCodes(IMSVector<IMS_SINT32>()),
            bUseEmergencyNumberTranslationInRoamingStatus(IMS_FALSE),
            bIgnorePrackDeliveryFailure(IMS_FALSE),
            bSupportVideoCallOnlyInVopsOffStatus(IMS_FALSE),
            bBlockWifiEmergencyCallIfNotProvisioned(IMS_FALSE),
            objRegistrationDisconnectReasonToTerminateOngoingCalls(IMSVector<IMS_SINT32>()),
            nWifiEmergency18xTimer(0),
            bSupportCanidInfo(IMS_FALSE),
            bUseCarrierSpecificContactHeaderForOptionsResponse(IMS_FALSE),
            bUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration(IMS_FALSE),
            bEnableRegistrationRecoveryWhenCallRejectedByServerError(IMS_FALSE),
            bEnableRegistrationRecoveryWhenCallRetryUnavailable(IMS_FALSE),
            bRejectVowifiVoiceCallWhenVowifiSettingOff(IMS_FALSE),
            bCheckServerOutageReasonForVxlteCall(IMS_FALSE),
            bSetVideoTextFeatureExclusivelyInContactHeaderBySessionType(IMS_FALSE),
            nMaximumWaitTimerForGeolocationPidfInfo(0),
            bMaintainMultipleEarlySessionsByForking(IMS_TRUE),
            bStopRingbackTimerBy183WithSdpBody(IMS_FALSE),
            bEnableVoiceMailServiceByPaidHeader(IMS_FALSE),
            objInformationLevelOfGeolocationPidfs(IMSVector<IMS_SINT32>()),
            bInitializePemWhenNoHeader(IMS_FALSE),
            nPolicyForLocalRingbackToneWith180Response(0)
    {
    }
    ~AssetItems()
    {
        objCarrierSpecificSipHeaders.Clear();
        objCallMaintainingOnRegistrationSuspendeds.Clear();
        objRequiringEmergencyCallWhenVideoEmergencyCallFaileds.Clear();
        objVilteToVolteRetryFailureResponseCodes.Clear();
        objRegistrationDisconnectReasonToTerminateOngoingCalls.Clear();
        objInformationLevelOfGeolocationPidfs.Clear();
    }

    AssetItems(IN const AssetItems&) = delete;             // not planed
    AssetItems& operator=(IN const AssetItems&) = delete;  // not planed

public:
    IMS_BOOL bCheckConferenceEventPackageVersion;
    IMS_BOOL bConferenceReferToUriSourcePaid;
    IMS_SINT32 nConferenceDropReferToUriSourceType;
    IMS_BOOL bEnableFakeQosCallFlowOnWifi;
    IMS_SINT32 nMediaTypeForOfferlessReinvite;
    IMS_BOOL bSupportVideoCallUpgradeRegardlessOfFeatureTags;
    IMS_SINT32 nOipTypeForUnavailable;
    IMS_BOOL bEnableOipHeaderPolicyFallBack;
    IMS_SINT32 nEmergencyRttGuardTimer;
    IMS_BOOL bRetryEmergencyCallOverEmergencyPdnWithNextPcscf;
    IMS_SINT32 nPreAlertingTimer;
    IMS_SINT32 nPolicyForTcallTimerExpiryOfVolteCall;
    IMS_SINT32 nPolicyForTcallTimerExpiryOfVolteEmergencyCall;
    IMS_SINT32 nPolicyForTcallTimerExpiryOfVowifiCall;
    IMSVector<AString> objCarrierSpecificSipHeaders;
    IMS_BOOL bCheckAvchangeFeatureForCallConvertingCapability;
    IMS_BOOL bSupportRegistrationRecoveryForFailureOfSessionRefresh;
    IMSVector<IMS_SINT32> objCallMaintainingOnRegistrationSuspendeds;              // name?
    IMSVector<IMS_SINT32> objRequiringEmergencyCallWhenVideoEmergencyCallFaileds;  // name?
    IMS_BOOL bUseMcidSupplementaryService;
    IMS_BOOL bUseMmcSupplementaryService;
    IMS_BOOL bUseLtePreferredStatusForServiceCapability;
    IMS_BOOL bAllowIncomingHoldRequestDuringConferenceCall;
    IMS_BOOL bIgnore180After183Response;
    IMS_BOOL bAddReplaceHeaderForConference;
    IMSVector<IMS_SINT32> objVilteToVolteRetryFailureResponseCodes;
    IMS_BOOL bUseEmergencyNumberTranslationInRoamingStatus;
    IMS_BOOL bIgnorePrackDeliveryFailure;
    IMS_BOOL bSupportVideoCallOnlyInVopsOffStatus;
    IMS_BOOL bBlockWifiEmergencyCallIfNotProvisioned;
    IMSVector<IMS_SINT32> objRegistrationDisconnectReasonToTerminateOngoingCalls;
    IMS_SINT32 nWifiEmergency18xTimer;
    IMS_BOOL bSupportCanidInfo;
    IMS_BOOL bUseCarrierSpecificContactHeaderForOptionsResponse;
    IMS_BOOL bUseCarrierSpecificRejectPhraseForIncomingCallDuringNoRegistration;
    IMS_BOOL bEnableRegistrationRecoveryWhenCallRejectedByServerError;
    IMS_BOOL bEnableRegistrationRecoveryWhenCallRetryUnavailable;
    IMS_BOOL bRejectVowifiVoiceCallWhenVowifiSettingOff;
    IMS_BOOL bCheckServerOutageReasonForVxlteCall;
    IMS_BOOL bSetVideoTextFeatureExclusivelyInContactHeaderBySessionType;
    IMS_SINT32 nMaximumWaitTimerForGeolocationPidfInfo;
    IMS_BOOL bMaintainMultipleEarlySessionsByForking;
    IMS_BOOL bStopRingbackTimerBy183WithSdpBody;
    IMS_BOOL bEnableVoiceMailServiceByPaidHeader;
    IMSVector<IMS_SINT32> objInformationLevelOfGeolocationPidfs;
    IMS_BOOL bInitializePemWhenNoHeader;
    IMS_SINT32 nPolicyForLocalRingbackToneWith180Response;
};

#endif
