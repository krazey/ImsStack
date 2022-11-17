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
            bDestroyUnsecureTcpSocketOnAccomplishingReg(IMS_FALSE),
            bDisableT3482ForEmc(IMS_FALSE),
            bEmcCallBasedOnPAssociatedUriOfNormalReg(IMS_FALSE),
            bHoldRegWithIpcanChangedDuringImsCall(IMS_FALSE),
            bIgnoreVopsForVolteEnable(IMS_FALSE),
            bImsDeregOn3gNetwork(IMS_FALSE),
            bNoInitRegOnPcscfChange(IMS_FALSE),
            bRegContactValidation(IMS_FALSE),
            bRegRetryWithIpVerFallback(IMS_FALSE),
            bRemoveOldSaOnEstablishingSa(IMS_FALSE),
            bRequiredCdmalessFeatureTag(IMS_FALSE),
            bRequiredEmcRegInRoaming(IMS_FALSE),
            bRequiredInitRegAfterImsCallEndOnRegHeld(IMS_FALSE),
            bRequiredVolteBlockBySetting(IMS_FALSE),
            bRequiredVolteBlockByAirplaneMode(IMS_FALSE),
            bRequiredWfcBlockByAirplaneMode(IMS_FALSE),
            bReregWithChangedCountryOnWifi(IMS_FALSE),
            bSipOverIpsecEnabledInRoaming(IMS_TRUE),
            bSmsOverImsAvailableWithoutVoiceCapa(IMS_FALSE),
            bSupportContactUserInfo(IMS_TRUE),
            bSupportRegWithFeatureTagUnavailable(IMS_FALSE),
            bSupportVerstatForReg(IMS_FALSE),
            bUseRcsTelephonyFeatureTagAsAvailableVoiceCallType(IMS_FALSE),
            bUseSecurityServerPortInInitReg(IMS_FALSE),
            bUseSecurityServerPortInRegContactOfInitReg(IMS_FALSE),
            bUseWfcCountryCodeAvailabilityCheck(IMS_FALSE),
            bVideoOverWifiSupportedWithoutVoice(IMS_FALSE),
            nContactUserInfoPolicyForNonRegMessage(
                    CarrierConfig::Assets::CONTACT_USER_INFO_POLICY_DEFAULT),
            nEmcPreferredIpType(CarrierConfig::Assets::IP_VERSION_6),
            nGeolocationPidfFormingPolicy(
                    CarrierConfig::Assets::GEOLOCATION_POLICY_WITHOUT_POSITION),
            nImsPreferredIpType(CarrierConfig::Assets::IP_VERSION_6),
            nImsSignallingDscp(46),
            nRegActualWaitTimePolicy(CarrierConfig::Assets::AWT_POLICY_RFC_RULE),
            nRegOutOfServicePolicy(CarrierConfig::Assets::REG_OOS_POLICY_DEFAULT),
            nRegPcscfUpdatePolicy(CarrierConfig::Assets::REG_PCSCF_UPDATE_POLICY_DEFAULT),
            nRegRetry305Policy(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT),
            nRegRetry503Policy(CarrierConfig::Assets::SIP_503_CODE_POLICY_DEFAULT),
            nRegRetryCntResetPolicy(CarrierConfig::Assets::REG_RETRY_CNT_RESET_POLICY_REGISTRATION),
            nRegRetryDefaultPolicy(CarrierConfig::Assets::DEFAULT_RETRY_POLICY_SPEC),
            nRegRetryMinCnt(0),
            nRegRetryTimerFPolicy(CarrierConfig::Assets::TIMER_F_POLICY_NONE),
            nRegTimerForEmcCallMillis(0),
            nReregRetry305Policy(CarrierConfig::Assets::SIP_305_CODE_POLICY_DEFAULT),
            nReregRetryMaxCntToKeepReg(0),
            nSipMessageThresholdForTransportChange(200),
            objEmergencyPcscfRetryWaitTimeSec(IMSVector<IMS_SINT32>()),
            objRegErrCodeForPcscfDiscovery(IMSVector<IMS_SINT32>()),
            objRegPermanentErrMaxCnt(IMSVector<IMS_SINT32>()),
            objRegRetryErrCodeWithDiffPcscf(IMSVector<IMS_SINT32>()),
            objRegRetryErrCodeWithoutIpsec(IMSVector<IMS_SINT32>()),
            objReregErrCodeForCallEnd(IMSVector<IMS_SINT32>()),
            objReregErrCodeForImsPdnReactivation(IMSVector<IMS_SINT32>()),
            objReregErrCodeForInitRegWithAvailablePcscf(IMSVector<IMS_SINT32>()),
            objReregErrCodeForPdnReactivationAfterCallEnd(IMSVector<IMS_SINT32>()),
            objReregRetryErrCodeForInitReg(IMSVector<IMS_SINT32>()),
            objReregRetryErrCodeForInitRegWithSamePcscf(IMSVector<IMS_SINT32>()),
            objSubErrorCodeForInitRegWithNextPcscf(IMSVector<IMS_SINT32>()),
            objSubErrorCodeForStoppingByExpirationTime(IMSVector<IMS_SINT32>()),
            objSupportedRoamingRats(IMSVector<IMS_SINT32>()),
            objVowifiSubErrorCodeForInitReg(IMSVector<IMS_SINT32>()),
            objWfcSubErrByMissing911Address(IMSVector<IMS_SINT32>())
    {
    }

    AosAsset(IN const AosAsset&) = delete;
    AosAsset& operator=(IN const AosAsset&) = delete;

public:
    IMS_BOOL bDestroyUnsecureTcpSocketOnAccomplishingReg;
    IMS_BOOL bDisableT3482ForEmc;
    IMS_BOOL bEmcCallBasedOnPAssociatedUriOfNormalReg;
    IMS_BOOL bHoldRegWithIpcanChangedDuringImsCall;
    IMS_BOOL bIgnoreVopsForVolteEnable;
    IMS_BOOL bImsDeregOn3gNetwork;
    IMS_BOOL bNoInitRegOnPcscfChange;
    IMS_BOOL bRegContactValidation;
    IMS_BOOL bRegRetryWithIpVerFallback;
    IMS_BOOL bRemoveOldSaOnEstablishingSa;
    IMS_BOOL bRequiredCdmalessFeatureTag;
    IMS_BOOL bRequiredEmcRegInRoaming;
    IMS_BOOL bRequiredInitRegAfterImsCallEndOnRegHeld;
    IMS_BOOL bRequiredVolteBlockBySetting;
    IMS_BOOL bRequiredVolteBlockByAirplaneMode;
    IMS_BOOL bRequiredWfcBlockByAirplaneMode;
    IMS_BOOL bReregWithChangedCountryOnWifi;
    IMS_BOOL bSipOverIpsecEnabledInRoaming;
    IMS_BOOL bSmsOverImsAvailableWithoutVoiceCapa;
    IMS_BOOL bSupportContactUserInfo;
    IMS_BOOL bSupportRegWithFeatureTagUnavailable;
    IMS_BOOL bSupportVerstatForReg;
    IMS_BOOL bUseRcsTelephonyFeatureTagAsAvailableVoiceCallType;
    IMS_BOOL bUseSecurityServerPortInInitReg;
    IMS_BOOL bUseSecurityServerPortInRegContactOfInitReg;
    IMS_BOOL bUseWfcCountryCodeAvailabilityCheck;
    IMS_BOOL bVideoOverWifiSupportedWithoutVoice;
    IMS_SINT32 nContactUserInfoPolicyForNonRegMessage;
    IMS_SINT32 nEmcPreferredIpType;
    IMS_SINT32 nGeolocationPidfFormingPolicy;
    IMS_SINT32 nImsPreferredIpType;
    IMS_SINT32 nImsSignallingDscp;
    IMS_SINT32 nRegActualWaitTimePolicy;
    IMS_SINT32 nRegOutOfServicePolicy;
    IMS_SINT32 nRegPcscfUpdatePolicy;
    IMS_SINT32 nRegRetry305Policy;
    IMS_SINT32 nRegRetry503Policy;
    IMS_SINT32 nRegRetryCntResetPolicy;
    IMS_SINT32 nRegRetryDefaultPolicy;
    IMS_SINT32 nRegRetryMinCnt;
    IMS_SINT32 nRegRetryTimerFPolicy;
    IMS_SINT32 nRegTimerForEmcCallMillis;
    IMS_SINT32 nReregRetry305Policy;
    IMS_SINT32 nReregRetryMaxCntToKeepReg;
    IMS_SINT32 nSipMessageThresholdForTransportChange;
    IMSVector<IMS_SINT32> objEmergencyPcscfRetryWaitTimeSec;
    IMSVector<IMS_SINT32> objRegErrCodeForPcscfDiscovery;
    IMSVector<IMS_SINT32> objRegPermanentErrMaxCnt;
    IMSVector<IMS_SINT32> objRegRetryErrCodeWithDiffPcscf;
    IMSVector<IMS_SINT32> objRegRetryErrCodeWithoutIpsec;
    IMSVector<IMS_SINT32> objReregErrCodeForCallEnd;
    IMSVector<IMS_SINT32> objReregErrCodeForImsPdnReactivation;
    IMSVector<IMS_SINT32> objReregErrCodeForInitRegWithAvailablePcscf;
    IMSVector<IMS_SINT32> objReregErrCodeForPdnReactivationAfterCallEnd;
    IMSVector<IMS_SINT32> objReregRetryErrCodeForInitReg;
    IMSVector<IMS_SINT32> objReregRetryErrCodeForInitRegWithSamePcscf;
    IMSVector<IMS_SINT32> objSubErrorCodeForInitRegWithNextPcscf;
    IMSVector<IMS_SINT32> objSubErrorCodeForStoppingByExpirationTime;
    IMSVector<IMS_SINT32> objSupportedRoamingRats;
    IMSVector<IMS_SINT32> objVowifiSubErrorCodeForInitReg;
    IMSVector<IMS_SINT32> objWfcSubErrByMissing911Address;
};
#endif  // AOS_ASSET_H_
