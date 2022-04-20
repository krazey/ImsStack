#ifndef _AOS_NCONFIGURATION_H_
#define _AOS_NCONFIGURATION_H_

#include "AString.h"

#include "ICarrierConfigListener.h"

#include "interface/IAosNConfiguration.h"
#include "provider/AosAsset.h"
#include "provider/AosAssetBundle.h"
#include "provider/AosCarrierConfig.h"
#include "provider/AosCarrierConfigBundle.h"

class ICarrierConfig;

/**
 * @brief This class is defined as the configuration information allows classes related Aos take.
 *
 *        This configuration is based on carrier and asset configuration
 */
class AosNConfiguration
    : public ICarrierConfigListener
    , public IAosNConfiguration
{
public:
    AosNConfiguration();
    virtual ~AosNConfiguration();

    virtual IMS_SINT32 GetSlotId() const;

    virtual IMS_BOOL IsSubscription() const;
    virtual IMS_BOOL IsUnSubscription() const;
    virtual IMS_BOOL IsVoLteAvailable() const;
    virtual IMS_BOOL IsVoLteRoamingAvailable() const;
    virtual IMS_BOOL IsVtAvailable() const;
    virtual IMS_BOOL IsDataEnableChangeIgnoredForVideoCalls() const;
    virtual IMS_BOOL IsWfcImsAvailable() const;
    virtual IMS_BOOL IsWfcRoamingEnabled() const;
    virtual IMS_BOOL IsImsSingleRegistrationRequired() const;
    virtual IMS_BOOL IsRttSupported() const;
    virtual IMS_BOOL IsSupportLimitedAdminSmsMode() const;
    virtual IMS_BOOL IsTtySupported() const;
    virtual IMS_BOOL IsVopsIgnoredForVolteEnabled() const;
    virtual IMS_BOOL IsRequiredEmergencyRegistrationInRoaming() const;
    virtual IMS_BOOL IsRequiredVolteBlockBySetting() const;
    virtual IMS_BOOL IsRequiredVolteBlockByAirplaneMode() const;
    virtual IMS_BOOL IsRequiredWfcBlockByAirplaneMode() const;
    virtual IMS_BOOL UseWfcCountryCodeAvailabilityCheck() const;
    virtual IMS_BOOL IsRegistrationRetryIntervalsUsedForSubscription() const;
    virtual IMS_BOOL IsSmsOverIpEnabled() const;
    virtual IMS_BOOL IsIpsecEnabled() const;
    virtual IMS_BOOL IsSecurityServerPortInRegContactOfInitialRegistrationUsed() const;
    virtual IMS_BOOL IsSecurityServerPortInInitialRegistrationUsed() const;
    virtual IMS_BOOL IsOldSaOnEstablishingSaRemoved() const;
    virtual IMS_BOOL IsEmergencyPdnWithEmergencyCallEndReleased() const;
    virtual IMS_BOOL IsSmsOverImsSupported() const;
    virtual IMS_BOOL IsImsOverNrEnabled() const;
    virtual IMS_BOOL IsVerstatForRegistrationSupported() const;
    virtual IMS_BOOL IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const;
    virtual IMS_BOOL IsRegistrationWhenIpcanChangedWithImsActiveCallHeld() const;
    virtual IMS_BOOL IsDeregisterOn3gNetworks() const;
    virtual IMS_BOOL IsVideoOverWifiSupportedWithoutVoice() const;
    virtual IMS_BOOL IsGeolocationPidfSupported(IN IMS_SINT32 nGeolocationPidfType) const;
    virtual IMS_BOOL IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType() const;
    virtual IMS_BOOL IsCdmalessFeatureTagRequired() const;

    virtual IMS_UINT32 GetRegistrationRetryBaseTime();
    virtual IMS_UINT32 GetRegistrationRetryMaxTime();
    virtual IMS_UINT32 GetIsimIndexForImpu();
    virtual IMS_SINT32 GetUssdMethod() const;
    virtual IMS_SINT32 GetPreferredIpType() const;
    virtual IMS_SINT32 GetEmergencyPreferredIpType() const;
    virtual IMS_SINT32 GetPcscfPort() const;
    virtual IMS_SINT32 GetSipPreferredTransport() const;
    virtual IMS_SINT32 GetIpv4MtuSize() const;
    virtual IMS_SINT32 GetIpv6MtuSize() const;
    virtual IMS_SINT32 GetPreferredEmergencyRegistration() const;
    virtual IMS_SINT32 GetEmergencyRegistrationTimerMillis() const;
    virtual IMS_SINT32 GetRegistrationRetryDefaultPolicy() const;
    virtual IMS_SINT32 GetPreferredImsDscp() const;
    virtual IMS_SINT32 GetImsSignallingDscp() const;
    virtual IMS_SINT32 GetRegistrationPreferredAccessTypeFeatureTag() const;
    virtual IMS_SINT32 GetRegistrationPrivateHeader() const;
    virtual IMS_SINT32 GetRegistrationActualWaitTimePolicy() const;
    virtual IMS_SINT32 GetSipMessageThresholdForTransportChange() const;
    virtual IMS_SINT32 GetRegistrationRetrySip305CodePolicy() const;
    virtual IMS_SINT32 GetReregistrationRetrySip305CodePolicy() const;
    virtual IMS_SINT32 GetRegistrationRetrySip503CodePolicy() const;
    virtual IMS_SINT32 GetSpecificRegistrationErrorFinalType() const;
    virtual IMS_SINT32 GetSpecificRegistrationErrorPolicy() const;

    virtual IMSVector<IMS_SINT32>& GetRegistrationRetryIntervals();
    virtual IMSVector<IMS_SINT32>& GetRegistrationRandomRetryIntervals();
    virtual IMSVector<IMS_SINT32>& GetIpsecAuthenticationAlgorithms();
    virtual IMSVector<IMS_SINT32>& GetIpsecEncryptionAlgorithms();

    virtual IMS_UINT32 GetNotifyEventForInitialRegistration() const;
    virtual IMS_SINT32 GetNotifyWaitTime() const;
    virtual IMS_UINT32 GetNotifyEventForInitialRegWithWaitTime() const;

    virtual IMSVector<IMS_SINT32>& GetSubErrorRegRequired();
    virtual IMS_SINT32 GetRetryCountSubErrorRegRequired() const;
    virtual IMSVector<IMS_SINT32>& GetSubErrorRegRequiredWithNextPcscf();
    virtual IMSVector<IMS_SINT32>& GetSubErrorSubTerminated();
    virtual IMS_SINT32 GetRetryCountSubErrorSubTerminated() const;
    virtual IMSVector<IMS_SINT32>& GetSubErrorStoppingResub();
    virtual IMSVector<IMS_SINT32>& GetVowifiSubErrorRegRequired();
    virtual IMS_UINT32 GetClearReasonForPermanentPdnFailure() const;
    virtual IMSVector<IMS_SINT32>& GetImsIdentityPriority();
    virtual IMSVector<IMS_SINT32>& GetPcscfDiscoveryMethod();
    virtual IMSVector<IMS_SINT32>& GetRoamingPcscfDiscoveryMethod();
    virtual IMSVector<IMS_SINT32>& GetUpdateRegistrationWithRatChange();
    virtual IMSVector<IMS_SINT32>& GetSupportedRats();
    virtual IMSVector<IMS_SINT32>& GetSupportedRoamingRats();
    virtual IMSVector<IMS_SINT32>& GetSmsOverImsSupportedRats();
    virtual IMSVector<IMS_SINT32>& GetSpecificRegErrNumMultipliedByPcscfNum();
    virtual IMSVector<IMS_SINT32>& GetSpecificRegistrationErrorCode();
    virtual IMSVector<IMS_SINT32>& GetReregRetryErrCodeWithInitialRegWithSamePcscf();

private:
    friend class AosBuildDirector;

    virtual void CarrierConfig_NotifyConfigChanged(IN IMS_SINT32 nSlotId);
    virtual void Init(IN IMS_SINT32 nSlotId = IMS_SLOT_0);

    void InitBundle(IN const ICarrierConfig* piCc);
    void InitConfig(IN const ICarrierConfig* piCc);
    void InitIpsecAlgorithm(IN const ICarrierConfig* piCc);

private:
    IMS_SINT32 m_nSlotId;

    AosAsset m_objAsset;
    AosCarrierConfig m_objCarrierConfig;

    AosMmtelRequiresProvisioningBundle m_objMmtelProvisioning;

    AosNotifyTerminatedForRegEventWithInitialRegistrationBundle m_objNotifyTerminated;

    AosRegistrationPermanentErrorCodeBundle m_objRegPermanentErrCode;

    AosRegistrationErrorCodeWithRetryAfterTimeBundle m_objRegErrCodeWithRetryAfterTime;
    AosRegistrationWithFeatureTagUnavailableBundle m_objRegWithFeatureTagUnavailable;
    AosRegistrationRetryBundle m_objRegRetry;
    AosRegistrationRetryIntervalBundle m_objRegRetryInterval;

    AosReregistrationErrorPolicyDuringCallBundle m_objReregErrPolicyCall;
    AosReregistrationRetryBundle m_objReregRetry;

    AosSpecificRegistrationErrorBundle m_objSpecificRegErr;

    AosSubscriptionErrorCodeForRegEventWithInitialRegistrationBundle
            m_objSubErrCodeWithInitReg;
    AosSubscriptionTerminatedErrorCodeForRegEventBundle m_objSubTerminatedErrCode;

    IMS_UINT32 m_nEventForInitRegOnTerminatedState;
    IMS_UINT32 m_nEventToFollowWtForInitRegOnTerminatedState;
    IMS_UINT32 m_nClearPermanentPdnFailure;

    AString m_strLogTag;
};
#endif // _AOS_NCONFIGURATION_H_
