#ifndef _AOS_ASSET_H_
#define _AOS_ASSET_H_

#include "IMSTypeDef.h"
#include "AString.h"
#include "IMSVector.h"
#include "CarrierConfig.h"

struct AosAsset
{
public:
    AosAsset()
        : bUseSecurityServerPortInRegContactOfInitialRegistration(IMS_FALSE)
        , bUseSecurityServerPortInInitialRegistration(IMS_FALSE)
        , bRemoveOldSaOnEstablishingSa(IMS_FALSE)
        , bUseGGsmaRcsTelephonyFeatureTagAsAvailableVoiceCallType(IMS_FALSE)
        , objPcscfDiscoveryMethodRoaming(IMSVector<IMS_SINT32>())
        , bHoldRegistrationWhenIpcanChangedWithImsActiveCall(IMS_FALSE)
        , bSupportVerstatForRegistration(IMS_FALSE)
        , bNoInitialRegistrationOnPcscfChange(IMS_FALSE)
        , bEmergencyCallBasedOnPAssociatedUriOfNormalRegistration(IMS_FALSE)
        , bSipOverIpsecEnabledInRoaming(IMS_TRUE)
        , bRegistrationContactValidation(IMS_FALSE)
        , nRegistrationOutOfServicePolicy(CarrierConfig::Ims::REGISTRATION_OOS_POLICY_DEFAULT)
        , objVowifiSubscriptionErrorCodeWithInitialRegistration(IMSVector<IMS_SINT32>())
        , objSubscriptionErrorCodeForStoppingByExpirationTime(IMSVector<IMS_SINT32>())
        , objSubscriptionErrorCodeForRegEventWithInitialRegistrationWithNextPcscf(
                IMSVector<IMS_SINT32>())
        , bIgnoreVopsForVolteEnable(IMS_FALSE)
        , bSmsOverImsAvailableInNoVops(IMS_FALSE)
        , bImsDeregisterOn3gNetworks(IMS_FALSE)
        , bVideoOverWifiSupportedWithoutVoice(IMS_FALSE)
        , bRequiredEmergencyRegistrationInRoaming(IMS_FALSE)
        , objClearPermanentPdnFailure(IMSVector<IMS_SINT32>())
        , bUseWfcCountryCodeAvailabilityCheck(IMS_FALSE)
        , bRequiredVolteBlockBySetting(IMS_FALSE)
        , bRequiredVolteBlockByAirplaneMode(IMS_FALSE)
        , bRequiredWfcBlockByAirplaneMode(IMS_FALSE)
        , objSupportedRoamingRats(IMSVector<IMS_SINT32>())
        , nEmergencyPreferredIpType(CarrierConfig::Ims::IP_VERSION_6)
        , nSipMessageThresholdForTransportChange(200)
        , bCdmalessFeatureTagRequired(IMS_FALSE)
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
    IMS_BOOL bSmsOverImsAvailableInNoVops;
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
};
#endif // _AOS_ASSET_H_
