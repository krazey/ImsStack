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
#ifndef INTERFACE_AOS_NCONFIGURATION_H_
#define INTERFACE_AOS_NCONFIGURATION_H_

#include "ImsTypeDef.h"

#define GET_N_CONFIG(SLOT) (AosProvider::GetInstance()->GetNConfiguration(SLOT))

class IAosNConfigurationListener;

/**
 * @brief This class provides an interface to access the configuration information related to Aos.
 *
 */
class IAosNConfiguration
{
public:
    virtual ~IAosNConfiguration(){};

    /**
     * @brief Get the slot id to be retrieved.
     *
     * @return IMS_SINT32 Return slot id to be retrieved
     */
    virtual IMS_SINT32 GetSlotId() const = 0;

    /**
     * @brief Set the listener for monitoring the configuration.
     *
     * @param piListener Indicate the configuration listener.
     */
    virtual void SetListener(IN IAosNConfigurationListener* piListener) = 0;

    /**
     * @brief Remove the listener for monitoring the configuration.
     *
     * @param piListener Indicate the configuration listener.
     */
    virtual void RemoveListener(IN IAosNConfigurationListener* piListener) = 0;

    /**
     * @brief Indicate whether SIP REGISTER message should be transmitted by TCP or not.
     *        The default value is false and if set to true, SIP REGISTER message will be
     *        transmitted by only TCP transport type.
     *
     * @return IMS_BOOL Return whether SIP REGISTER message should be transmitted by TCP or not.
     * @see {@code ims.use_tcp_transport_for_register_bool}
     */
    virtual IMS_BOOL IsTcpRequiredForReg() const = 0;

    /**
     * @brief Returns whether subscription is initiated after registration.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.registration_event_package_supported_bool}
     */
    virtual IMS_BOOL IsSubscription() const = 0;
    /**
     * @brief Returns whether un-subscription is initiated before de-registration.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.unsubscribe_registration_event_package_bool}
     */
    virtual IMS_BOOL IsUnSubscription() const = 0;

    /**
     * @brief Check if VoLTE is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code carrier_volte_available_bool}
     */
    virtual IMS_BOOL IsVoLteAvailable() const = 0;

    /**
     * @brief Check if VoLTE roaming is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code imsvoice.carrier_volte_roaming_available_bool}
     */
    virtual IMS_BOOL IsVoLteRoamingAvailable() const = 0;

    /**
     * @brief Check if Video calling is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code carrier_vt_available_bool}
     */
    virtual IMS_BOOL IsVtAvailable() const = 0;

    /**
     * @brief Check if WFC is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code carrier_wfc_ims_available_bool}
     */
    virtual IMS_BOOL IsWfcImsAvailable() const = 0;

    /**
     * @brief Check if single registration is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     * @see {@code ims.ims_single_registration_required_bool}
     */
    virtual IMS_BOOL IsImsSingleRegistrationRequired() const = 0;

    /**
     * @brief Check if RTT is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code rtt_supported_bool}
     */
    virtual IMS_BOOL IsRttSupported() const = 0;

    /**
     * @brief Check if RTT is supported while UE is on roaming network.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code rtt_supported_while_roaming_bool}
     */
    virtual IMS_BOOL IsRttSupportedWhileRoaming() const = 0;

    /**
     * @brief Check if limited admin sms mode is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code imssms.support_limited_admin_sms_mode_bool}
     */
    virtual IMS_BOOL IsSupportLimitedAdminSmsMode() const = 0;

    /**
     * @brief Check if network initiated USSD over IMS (USSI) is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code imsss.network_initiated_ussd_over_ims_supported_bool}
     */
    virtual IMS_BOOL IsNetworkInitiatedUssdOverImsSupported() const = 0;

    /**
     * @brief Check if TTY over VoLTE is supported.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code carrier_volte_tty_supported_bool}
     */
    virtual IMS_BOOL IsVolteTtySupported() const = 0;

    /**
     * @brief Check if VOPS is ignored for VoLTE enable.
     *
     * @return IMS_TRUE if ignored, IMS_FALSE if considered.
     * @see {@code imsvoice.ignore_vops_for_volte_enable_bool}
     */
    virtual IMS_BOOL IsVopsIgnoredForVolteEnabled() const = 0;

    /**
     * @brief Check if Sms Over Ims is available without voice capability.
     *
     * @return IMS_TRUE if available, IMS_FALSE if not available.
     * @see {@code imssms.sms_over_ims_available_without_voice_capa_bool}
     */
    virtual IMS_BOOL IsSmsOverImsAvailableWithoutVoiceCapability() const = 0;

    /**
     * @brief Indicates whether the UE supports the anonymous emergency call action.
     *
     * If set to true, the UE supports and will check if the network requires an
     * anonymous emergency call to be made using a specific action (e.g.,
     * <action>:anonymous-emergencycall) after receiving a 403 Forbidden response to the emergency
     * registration request. If set to false, the UE does not support or will not check for the
     * presence of the required action in the 403 Forbidden response.
     *
     * @return IMS_BOOL Returns {@code IMS_TRUE} if the anonymous emergency call action is
     * supported,
     * {@code IMS_FALSE} otherwise.
     * @see {@code imsemergency.support_anonymous_ecall_action_bool}
     */
    virtual IMS_BOOL IsAnonymousECallActionSupported() const = 0;

    /**
     * @brief Check if VoLTE service block by SSAC is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     * @see {@code imsvoice.required_volte_block_by_ssac_bool}
     */
    virtual IMS_BOOL IsRequiredVolteBlockBySsac() const = 0;

    /**
     * @brief Check if WFC service block by Airplane mode is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     * @see {@code imswfc.required_wfc_block_by_airplane_mode_bool}
     */
    virtual IMS_BOOL IsRequiredWfcBlockByAirplaneMode() const = 0;

    /**
     * @brief Returns whether to add geolocation pidf in initial registration over wifi.
     *
     *        It's applied if CarrierConfig::Ims::GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI is set
     *        in the CarrierConfig::Ims::KEY_GEOLOCATION_PIDF_IN_SIP_REGISTER_SUPPORT_INT_ARRAY.
     *
     * @return IMS_BOOL Return whether geolocation pidf is added to the initial registration or not
     * @see {@code imswfc.geolocation_pidf_in_wfc_init_reg_bool}
     */
    virtual IMS_BOOL IsGeolocationPidfInWfcInitReg() const = 0;

    /**
     * @brief Returns whether to add P-Access-Network-Info header in initial registration over wifi.
     *
     *        It's applied if CarrierConfig::Ims::
     *        KEY_ALLOW_SIP_P_ACCESS_NETWORK_INFO_HEADER_IN_INITIAL_REGISTER_BOOL is false.
     *        P-Access-Network-Info header : PANI header
     *
     * @return IMS_BOOL Return whether PANI header is added to the initial registration or not
     * @see {@code imswfc.pani_header_in_wfc_init_reg_bool}
     */
    virtual IMS_BOOL IsPaniHeaderInWfcInitReg() const = 0;

    /**
     * @brief Returns whether re-registration is sent on Wifi when the country is changed.
     *
     * @return IMS_BOOL Return whether to be sent re-registration or not
     * @see {@code imswfc.rereg_with_changed_country_on_wifi_bool}
     */
    virtual IMS_BOOL IsReregRetryWithChangedCountryOnWifi() const = 0;

    /**
     * @brief Returns whether IPSec is enabled for SIP messages in roaming networks.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.sip_over_ipsec_enabled_in_roaming_bool}
     */
    virtual IMS_BOOL IsSipOverIpsecInRoamingEnabled() const = 0;

    /**
     * @brief Check if country code availability check for wfc is required.
     *
     * @return IMS_TRUE if required, IMS_FALSE if not required.
     * @see {@code imswfc.use_wfc_country_code_availability_check_bool}
     */
    virtual IMS_BOOL UseWfcCountryCodeAvailabilityCheck() const = 0;

    /**
     * @brief Returns whether subscription also use the registration retry intervals.
     *
     *        This function relates to GetRegRetryIntervals() and
     *        GetRegRandomRetryIntervals()
     *        Subscription decides whether to follow the mentioned function by this return value.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.reg_retry_interval_used_for_sub_bool}
     */
    virtual IMS_BOOL IsRegRetryIntervalsUsedForSub() const = 0;

    /**
     * @brief Check if SmsOverIp is enabled.
     *
     * @return IMS_TRUE if enabled, IMS_FALSE if not enabled.
     * @see {@code KEY_SMS_OVER_IP_ENABLED} (@code ProvisioningManager)
     */
    virtual IMS_BOOL IsSmsOverIpEnabled() const = 0;

    /**
     * @brief Returns whether the ipsec is supported or not.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.sip_over_ipsec_enabled_bool}
     */
    virtual IMS_BOOL IsIpsecEnabled() const = 0;

    /**
     * @brief Returns whether to follow reg retry rule when emergency registration failed.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code imsemergency.use_reg_retry_rule_for_ereg_bool}
     */
    virtual IMS_BOOL IsRegRetryRuleForERegUsed() const = 0;

    /**
     * @brief Returns whether the security server port is used in reg contact of
     *        initial registration.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.use_security_server_port_in_reg_contact_of_init_reg_bool}
     */
    virtual IMS_BOOL IsSecurityServerPortInRegContactOfInitRegUsed() const = 0;

    /**
     * @brief Returns whether the security server port is used in initial registration.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.use_security_server_port_in_init_reg_bool}
     */
    virtual IMS_BOOL IsSecurityServerPortInInitRegUsed() const = 0;

    /**
     * @brief Returns whether the old security association is removed on establishing
     *        the security association
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.remove_old_sa_on_establishing_sa_bool}
     */
    virtual IMS_BOOL IsOldSaOnEstablishingSaRemoved() const = 0;

    /**
     * @brief Returns flag indicating whether +g.gsma.callcomposer feature tag is included
     *        in contact header of REGISTER messages when call composer is for B2C only.
     *
     * @return IMS_BOOL Return whether to be included or not
     * @see {@code ims.b2c_call_composer_feature_tag_in_reg_contact_bool}
     */
    virtual IMS_BOOL IsB2cCallComposerFeatureTagInRegContact() const = 0;

    /**
     * @brief Returns flag indicating whether PCSCFs that UE fails to register will be blocked.
     *
     * @return IMS_BOOL Return whether to be blocked or not
     * @see {@code ims.block_pcscf_on_reg_failure_bool}
     */
    virtual IMS_BOOL IsBlockPcscfOnRegFailure() const = 0;

    /**
     * @brief Returns flag indicating whether to block IMS registration on CS call.
     *
     * @return IMS_BOOL Return whether to be blocked or not
     * @see {@code ims.block_reg_on_cs_call_bool}
     */
    virtual IMS_BOOL IsBlockRegOnCsCall() const = 0;

    /**
     * @brief Returns whether a call should be terminated due to expiration of registration.
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.call_end_and_pdn_reactivation_by_reg_terminated_bool}
     */
    virtual IMS_BOOL IsCallEndAndPdnReactivationByRegTerminated() const = 0;

    /**
     * @brief Returns whether unsecure tcp socket is destroyed on accomplishing the registration
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.destroy_unsecure_tcp_socket_on_accomplishing_reg_bool}
     */
    virtual IMS_BOOL IsUnsecureTcpSocketOnAccomplishingRegDestroyed() const = 0;

    /**
     * @brief Flag specifying if SMS over IMS support is available or not.
     *
     * @return IMS_BOOL Return whether to be available or not
     * @see {@code imssms.sms_over_ims_supported_bool}
     */
    virtual IMS_BOOL IsSmsOverImsSupported() const = 0;

    /**
     * @brief Flag specifying if IMS over NR is enabled or not.
     *
     * @return IMS_BOOL Return whether to be enabled or not
     * @see {@code carrier_nr_availabilities_int_array}
     */
    virtual IMS_BOOL IsImsOverNrEnabled() const = 0;

    /**
     * @brief Flag indicating whether the authorized IMPU from P-Associated-URI header in 200 OK
     *        for IMS registration is used in emergency call.
     *
     * @return IMS_BOOL Return whether to be supported or not
     * @see {@code imsemergency.ecall_based_on_p_associated_uri_of_normal_reg_bool}
     */
    virtual IMS_BOOL IsEmergencyCallBasedOnPauOfNormalRegistrationSupported() const = 0;

    /**
     * @brief Flag specifying if UE tries emergency registration on a random pcscf.
     *
     *        If this is set as TRUE, UE will choose P-CSCF randomly for emergency registration if
     *        UE receives multiple P-CSCF addresses from P-CSCF discovery for emergency.
     *
     * @return IMS_BOOL Return whether UE tries emergency registration on random pcscf.
     * @see {@code imsemergency.ereg_on_random_pcscf_bool}
     */
    virtual IMS_BOOL IsEmcRegOnRandomPcscf() const = 0;

    /**
     * @brief Flag specifying if emergency registration is transmitted only over TCP
     *        in roaming network.
     *
     *        If this is set as TRUE, it will be applied in all the SIP messages which are sent
     *        via this emergency IMS registration.
     *
     * @return IMS_BOOL Return whether to be set or not to use only TCP transport.
     * @see {@code imsemergency.ereg_set_tcp_only_in_roaming_bool}
     */
    virtual IMS_BOOL IsERegWithOnlyTcpInRoaming() const = 0;

    /**
     * @brief Flag specifying if the first public user identity to the list stored in the ISIM is
     *        used in emergency registration requests.
     *
     *        If this is set as TRUE, the first public user identity in the ISIM will be used for
     *        emergency IMS registration.
     *
     * @return IMS_BOOL Return whether to use the first public user identity in the ISIM for
     *         emergency registration.
     * @see {@code imsemergency.ereg_using_first_impu_in_isim_bool}
     */
    virtual IMS_BOOL IsERegUsingFirstImpuInIsim() const = 0;

    /**
     * @brief Flag specifying if an IMS emergency registration procedure is initiated when there is
     *        LTE emergency attach or NR emergency registration with valid SIM.
     *
     * @return IMS_BOOL Returns IMS_TRUE if an IMS emergency registration is required under
     *         the specified conditions, otherwise returns IMS_FALSE.
     * @see {@code imsemergency.support_ereg_when_eattach_with_valid_sim_bool}
     */
    virtual IMS_BOOL IsSupportERegWhenEAttachWithValidSim() const = 0;

    /**
     * @brief Flag specifying if emergency re-registration is required after handover.
     *
     *        If this is set as TRUE, emergency re-registration will be conducted
     *        when ipcan is changed
     *
     * @return IMS_BOOL Return whether emergency re-registration is required after handover.
     * @see {@code imsemergency.support_erereg_on_ipcan_change_bool}
     */
    virtual IMS_BOOL IsEmergencyReregSupportedOnIpcanChange() const = 0;

    /**
     * @brief Flag specifying if GIBA(GPRS-IMS-Bundled authentication) is supported for emergency
     *        registration in roaming.
     *
     * @return IMS_TRUE if supported, else IMS_FALSE
     * @see {@code imsemergency.support_giba_for_ereg_in_roaming_bool}
     */
    virtual IMS_BOOL IsGibaSupportedForERegInRoaming() const = 0;

    /**
     * @brief Flag specifying whether the re-registration is held when IPCAN is changed
     *        during IMS calls and performed immediately after they are released.
     *
     * @return IMS_BOOL Return whether to be held or not
     * @see {@code ims.hold_reg_with_ipcan_changed_during_ims_call_bool}
     */
    virtual IMS_BOOL IsRegWithIpcanChangedDuringImsCallHeld() const = 0;

    /**
     * @brief Flag specifying whether IMS de-registers if the network is changed to 3G
     *        while IMS registered.
     *
     * @return IMS_BOOL Return whether IMS de-registers or not.
     * @see {@code ims.ims_dereg_on_3g_network_bool}
     */
    virtual IMS_BOOL IsDeregOn3gNetwork() const = 0;

    /**
     * @brief Check if IMSI-based URI should be prioritized.
     *
     * @return IMS_TRUE if IMSI-based URI should be prioritized, IMS_FALSE otherwise.
     * @see {@code ims.imsi_based_uri_prioritized_bool}
     */
    virtual IMS_BOOL IsImsiBasedUriPrioritized() const = 0;

    /**
     * @brief Flag indicating whether ipsec setting is initialized with new pcscf.
     *
     * @return IMS_BOOL Return whether initialize ipsec wetting with new pcscf or not.
     * @see {@code ims.init_ipsec_setting_with_new_pcscf_bool}
     */
    virtual IMS_BOOL IsIpsecInitializedWithNewPcscf() const = 0;

    /**
     * @brief Flag indicating whether to send initial subscription when the subscription is
     *        terminated.
     *
     * @return IMS_BOOL Return whether to initialize subscription when the subscription is
     *         terminated.
     * @see {@code ims.init_sub_upon_sub_terminated_bool}
     */
    virtual IMS_BOOL IsInitSubUponSubTerminated() const = 0;

    /**
     * @brief Flag indicating whether to keep the emergency pdn when pcscf is unavailable by
     *        requesting fake registration with the next pcscf.
     *
     * @return IMS_BOOL Return whether to keep the emergency pdn or not in this case.
     * @see {@code imsemergency.keep_epdn_upon_pcscf_unavailable_bool}
     */
    virtual IMS_BOOL IsKeepEPdnUponPcscfUnavailable() const = 0;

    /**
     * @brief Flag indicating whether to keep on retrying emergency registration on WLAN.
     *
     * @return IMS_BOOL Return whether to keep on retrying emergency registration on WLAN or not.
     * @see {@code imsemergency.keep_ereg_retry_on_wlan_bool}
     */
    virtual IMS_BOOL IsKeepERegRetryOnWlanRequired() const = 0;

    /**
     * @brief Returns flag indicating whether to keep the count of IMS registrations retry
     *        even when PDN is reconnected.
     *
     * @return IMS_TRUE if need to keep a count of registrations retry, else IMS_FALSE
     * @see {@code ims.keep_reg_retry_cnt_upon_pdn_reconnect_bool}
     */
    virtual IMS_BOOL IsKeepRegRetryCntUponPdnReconnect() const = 0;

    /**
     * @brief Returns a flag indicating whether Aos will keep the registration retry timer running
     *        even if all enablers are detached.
     *
     * @return IMS_TRUE if need to keep the registration retry timer on all enablers detached,
     *         else IMS_FALSE
     * @see {@code ims.keep_reg_retry_timer_on_all_enablers_detached_bool}
     */
    virtual IMS_BOOL IsKeepRegRetryTimerOnAllEnablersDetached() const = 0;

    /**
     * @brief Indicates how to handle the emergency call as failure or to proceed it without
     *        completing the normal registration after the timer as CarrierConfig::IMSEMERGENCY::
     *        KEY_REG_TIMER_FOR_ECALL_MILLIS_INT setting value is expired.
     *
     * @return IMS_BOOL Returns whether to handle the emergency call as failure or not.
     * @see {@code imsemergency.reg_timer_for_ecall_timeout_as_failure_bool}
     */
    virtual IMS_BOOL IsRegTimerForECallTimeoutAsFailure() const = 0;

    /**
     * @brief Indicates whether RAT is checked or not when E-PDN is requested. If RAT is not
     *        IMS supported, the timer as CarrierConfig::IMSEMERGENCY::
     *        KEY_REG_TIMER_FOR_ECALL_MILLIS_INT setting value is not started and the emergency call
     *        is proceeded without that timer waiting for the normal registration.
     *
     * @return IMS_BOOL Returns whether to check RAT or not when E-PDN is requested.
     * @see {@code imsemergency.reg_timer_for_ecall_with_rat_check_enabled_bool}
     */
    virtual IMS_BOOL IsRegTimerForECallWithRatCheckEnabled() const = 0;

    /**
     * @brief Flag indicating whether to stop emergency registration timer on E-PDN connection.
     *
     * @return IMS_BOOL Return whether to stop on emergency registration timer on E-PDN connection
     *         or not.
     * @see {@code imsemergency.stop_ereg_timer_on_epdn_connected_bool}
     */
    virtual IMS_BOOL IsStopERegTimerOnEpdnConnected() const = 0;

    /**
     * @brief Returns whether UE will reconnect PDN when all pcscfs are unavailable.
     *
     * @return IMS_BOOL Return whether to reconnect PDN or not.
     * @see {@code ims.pdn_reconnect_on_all_pcscfs_unavailable_bool}
     */
    virtual IMS_BOOL IsPdnReconnectOnAllPcscfsUnavailable() const = 0;

    /**
     * @brief Flag specifying if video call is supported over wifi when voice call is unavailable.
     *
     * @return IMS_BOOL Return whether video call is supported over wifi
     *         when voice call is unavailable.
     * @see {@code imswfc.video_over_wifi_supported_without_voice_bool}
     */
    virtual IMS_BOOL IsVideoOverWifiSupportedWithoutVoice() const = 0;

    /**
     * @brief Flag specifying if Geolocation-PIDF is supported.
     *
     * @param nGeolocationPidfType The Geolocation-PIDF type to be evaluated.\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_WIFI\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_EMERGENCY_ON_WIFI\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_NON_EMERGENCY_ON_CELLULAR\n
     *        #CarrierConfig#Ims#GEOLOCATION_PIDF_FOR_EMERGENCY_ON_CELLULAR
     *
     * @return IMS_TRUE if the specified Geolocation-PIDF type is supported, otherwise IMS_FALSE.
     * @see {@code ims.geolocation_pidf_in_sip_register_support_int_array}
     */
    virtual IMS_BOOL IsGeolocationPidfSupported(IN IMS_SINT32 nGeolocationPidfType) const = 0;

    /**
     * @brief Indicates whether to update ongoing reg retry timer when ims establishment timer is
     *        expired and the data is maintained.
     *
     *        When PLMN block is requested due to ims establishment timer expiry.
     *        If the value is true, the UE updates ongoing reg retry timer.
     *        If the value is false, the UE follows ongoing reg retry timer.
     *
     * @return IMS_BOOL Return whether to be applied or not.
     * @see {@code ims.update_ongoing_reg_retry_timer_on_ims_est_timer_expiry_bool}
     */
    virtual IMS_BOOL IsUpdateOngoingRegRetryTimerOnImsEstTimerExpiry() const = 0;

    /**
     * @brief Flag specifying if g.gsma.rcs.telephony feature tag is used
     *        to indicate available voice call type ("cs", "volte" or "cs,volte").
     *        Example) g.gsma.rcs.telephony = "cs,volte"
     *
     * @return IMS_BOOL Return whether the feature tag is used
     *                  to indicate available voice call type.
     * @see {@code ims.use_rcs_telephony_feature_tag_as_available_voice_call_type_bool}
     */
    virtual IMS_BOOL IsGGsmaRcsTelephonyFeatureTagUsedAsAvailableVoiceCallType() const = 0;

    /**
     * @brief Flag specifying if "+cdmaless" feature tag is required.
     *
     * @return IMS_BOOL Return whether the feature tag is required.
     * @see {@code ims.required_cdmaless_feature_tag_bool}
     */
    virtual IMS_BOOL IsCdmalessFeatureTagRequired() const = 0;

    /**
     * @brief Flag indicating whether the defined error codes are only applied with Retry-After
     *        header value.
     *
     *        This function relates to GetRegErrCodeWithRetryAfterTime() and
     *        GetReregErrCodeWithRetryAfterTime()
     *
     * @return IMS_BOOL Return whether the defined error codes is used.
     * @see {@code ims.reg_err_code_with_ra_time_only_defined_bool}
     */
    virtual IMS_BOOL IsRegErrCodeWithRetryAfterTimeOnlyDefined() const = 0;

    /**
     * @brief Returns whether the registration is handled as a failure when the UE receives
     *        an error response against re-registration in roaming
     *
     * @return IMS_BOOL Return whether to be handled as a failure or not.
     * @see {@code ims.extra_reg_err_code_as_failure_in_roaming_for_update_bool}
     */
    virtual IMS_BOOL IsExtraReregErrInRoamingAsFailureHandled() const = 0;

    /**
     * @brief Flag indicating whether the retry counter should be shared between REGISTER and
     *        SUBSCRIBE for reg event package.
     *
     * @return IMS_BOOL Return whether to be supported retry count shared for registration and
     *         reg event package
     * @see {@code ims.extra_reg_err_retry_cnt_shared_for_reg_and_sub_bool}
     */
    virtual IMS_BOOL IsExtraRegErrRetryCntSharedForRegAndSubRequired() const = 0;

    /**
     * @brief Returns Flag specifying whether UE should enter Emergency CallBack Mode (ECBM)
     *        after emergency call is ended.
     *
     * @return IMS_BOOL Return whether to be supported or not
     * @see {@code imsemergency.emergency_callback_mode_supported_bool}
     */
    virtual IMS_BOOL IsEmergencyCallbackModeSupported() const = 0;

    /**
     * @brief Returns Flag indicating whether or not sending emergency SMS messages over IMS
     *        is supported when in LTE/limited LTE (Emergency only) service mode.
     *
     * @return IMS_BOOL Return whether to be supported or not
     * @see {@code support_emergency_sms_over_ims_bool}
     */
    virtual IMS_BOOL IsEmergencySmsOverImsSupported() const = 0;

    /**
     * @brief Returns flag indicating whether the Contact URI validation is checked
     *        when the 200 OK response for SIP REGISTER is received.
     *        If the Contact URI is not matched, the 200 OK response is disregarded.
     *
     * @return IMS_BOOL Return whether to be checked or not
     * @see {@code ims.reg_contact_validation_bool}
     */
    virtual IMS_BOOL IsContactUriValidationChecked() const = 0;

    /**
     * @brief Returns whether IMS registration is retried based on IP version fallback
     *        from IPv6 to IPv4
     *
     * @return IMS_BOOL Return whether to be applied or not
     * @see {@code ims.reg_retry_with_ip_ver_fallback_bool}
     */
    virtual IMS_BOOL IsRegRetryWithIpVerFallback() const = 0;

    /**
     * @brief Flag specifying if emergency PDN would be released when the network is unavailable.
     *
     * @return IMS_TRUE if emergency PDN needs to be released, else IMS_FALSE
     * @see {@code imsemergency.release_epdn_of_unavailable_network_bool}
     */
    virtual IMS_BOOL IsReleaseEPdnOfUnavailableNetwork() const = 0;

    /**
     * @brief Flag specifying if emergency PDN would be released upon emergency call is terminated
     *        in fake mode.
     *
     * @return IMS_TRUE if released, else IMS_FALSE
     * @see {@code imsemergency.release_epdn_upon_ecall_end_in_fake_mode_bool}
     */
    virtual IMS_BOOL IsReleaseEPdnUponECallEndInFakeMode() const = 0;

    /**
     * @brief Flag indicating the initial registration is tried on not right after IMS call is ended
     *        while registration is held because re-registration is failed during active call.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code ims.required_init_reg_after_ims_call_end_on_reg_held_bool}
     */
    virtual IMS_BOOL IsRegRequiredAfterImsCallEndOnRegHeld() const = 0;

    /**
     * @brief Flag indicating the initial registration is tried on not right after IMS emergency
     *        call is ended while registration is held because re-registration is failed during
     *        active emergency call.
     *
     * @return IMS_TRUE if supported, IMS_FALSE if not supported.
     * @see {@code ims.required_init_reg_after_ims_ecall_end_on_reg_held_bool}
     */
    virtual IMS_BOOL IsRegRequiredAfterImsECallEndOnRegHeld() const = 0;

    /**
     * @brief Flag specifying if verstat is supported for registration or not
     *
     * @return IMS_BOOL Return whether to be supported or not
     * @see {@code ims.support_verstat_for_reg_bool}
     */
    virtual IMS_BOOL IsVerstatForRegistrationSupported() const = 0;

    /**
     * @brief Flag specifying if verstat is supported only when +g.3gpp.verstat feature tag is
     *        present in the 200 OK response to REGISTER message.
     *
     * @return IMS_BOOL Return TRUE if network feature is required for verstat, else IMS_FALSE
     * @see {@code ims.support_verstat_based_on_network_for_reg_bool}
     */
    virtual IMS_BOOL IsVerstatSupportedBasedOnNetworkForReg() const = 0;

    /**
     * @brief Flag specifying if service fallback is required when voice call is unavailable.
     *        This flag is only applicable for VoPS NOT SUPPORTED and SSAC barring cases.
     *
     * @return IMS_TRUE if required, else IMS_FALSE
     * @see {@code imsvoice.plmn_block_with_timeout_on_voice_call_unavailable_bool}
     */
    virtual IMS_BOOL IsPlmnBlockWithTimeoutOnVoiceCallUnavailable() const = 0;

    /**
     * @brief Flag specifying if WFC error message is supported.
     *
     * @return IMS_TRUE if supported, else IMS_FALSE
     * @see {@code imswfc.wfc_err_message_bundle}
     */
    virtual IMS_BOOL IsWfcErrorMessageSupported(IN IMS_SINT32 nError) const = 0;

    /**
     * @brief Flag specifying if video feature tag is supported for emergency registration.
     *
     * @return IMS_TRUE if required, else IMS_FALSE
     * @see {@code imsemergency.support_video_for_ereg_bool}
     */
    virtual IMS_BOOL IsVideoSupportedForEmergencyReg() const = 0;

    /**
     * @brief Returns flag indicating whether to allow use of contact information in RegInfo XML
     *        without checking for URI matching. If it is allowed, URI verification will be skipped
     *        during retrieving the "contact" element from RegInfo XML in the SIP Notify message.
     *
     * @return IMS_TRUE if use of contact element without URI check is allowed, else IMS_FALSE
     * @see {@code ims.use_reginfo_contact_without_uri_check_bool}
     */
    virtual IMS_BOOL IsUseRegInfoContactWithoutUriCheck() const = 0;

    /**
     * @brief Check if MTU from network is ignored.
     *
     * @return IMS_TRUE if ignored, IMS_FALSE if considered.
     * @see {@code ims.ignore_mtu_from_network_bool}
     */
    virtual IMS_BOOL IsIgnoreMtuFromNetwork() const = 0;

    /**
     * @brief Flag specifying if test mode is enabled.
     *
     * @param nType The test mode type to be enabled.\n
     *        #CarrierConfig#Ims#TEST_MODE_PERMANENT_FAILURE_WITHOUT_IMS_PDN_DEACTIVATION
     *
     * @return IMS_TRUE if enabled, else IMS_FALSE
     * @see {@code ims.test_mode_int_array}
     */
    virtual IMS_BOOL IsTestModeEnabled(IN IMS_SINT32 nType) const = 0;

    /**
     * @brief Indicates whether to keep using the existing pcscf to perform re-reg when handover
     *        between cellular and wifi during the call.
     *
     *        If the value is true, the UE uses the existing pcscf.
     *        If the value is false, the UE uses the new pcscf.
     *
     * @return IMS_BOOL Return whether to keep using the existing pcscf or not
     * @see {@code ims.keep_existing_pcscf_on_pcscf_change_during_the_call_bool}
     */
    virtual IMS_BOOL ShouldKeepExistingPcscfOnPcscfChangeDuringTheCall() const = 0;

    /**
     * @brief Get the SIP timer T1
     *
     *        This value is defined as per 3GPP TS 24.229 Table 7.7.1
     *        It defines in millisecond.
     *
     * @return IMS_SINT32 Return the SIP timer T1
     * @see {@code ims.sip_timer_t1_millis_int}
     */
    virtual IMS_SINT32 GetSipTimerT1() = 0;

    /**
     * @brief Get the registration retry base-time
     *
     *        This value defines as per RFC 5626 section 4.5
     *        It defines in millisecond.
     *
     * @return IMS_UINT32 Return retry base-time value
     * @see {@code ims.registration_retry_base_timer_millis_int}
     */
    virtual IMS_UINT32 GetRegistrationRetryBaseTime() = 0;

    /**
     * @brief Get the registration retry max-time
     *
     *         This value defines as per RFC 5626 section 4.5
     *         It defines in millisecond.
     *
     * @return IMS_UINT32 Return retry max-time value
     * @see {@code ims.registration_retry_max_timer_millis_int}
     */
    virtual IMS_UINT32 GetRegistrationRetryMaxTime() = 0;

    /**
     * @brief Get the ISIM index for IMPU.
     *
     * @return IMS_UINT32 Returns ISIM index for IMPU.
     * @see {@code ims.isim_index_for_impu_int}
     */
    virtual IMS_UINT32 GetIsimIndexForImpu() = 0;

    /**
     * @brief Returns IPCAN that emergency PDN shall be released after the emergency call is ended.
     *        Possible values are,
     *        CarrierConfig::Ims::IPCAN_NONE
     *        CarrierConfig::Ims::IPCAN_CELLULAR
     *        CarrierConfig::Ims::IPCAN_WLAN
     *        CarrierConfig::Ims::IPCAN_ALL
     *
     * @return IMS_SINT32 Return IPCAN
     * @see {@code imsemergency.ipcan_release_emergency_pdn_upon_emergency_call_end_int}
     */
    virtual IMS_SINT32 GetIpcanReleaseEmergencyPdnUponEmergencyCallEnd() const = 0;

    /**
     * @brief Indicate retry count to consider as a failure when receiving 401 response to the
     *        REGISTER message repeatedly and AKA result is successful.
     *
     * @return IMS_SINT32 retry count to consider as a auth failure
     * @see {@code ims.auth_failure_retry_max_cnt_int}
     */
    virtual IMS_SINT32 GetAuthFailureRetryMaxCnt() const = 0;

    /**
     * @brief Indicate which policy is applied to set user info for none register message.
     *        It will be applied in all the outgoing SIP request and response
     *        except for register request.
     *
     *        Possible values are,
     *        CarrierConfig::Ims::CONTACT_USER_INFO_POLICY_DEFAULT
     *        CarrierConfig::Ims::CONTACT_USER_INFO_POLICY_NONE
     *        CarrierConfig::Ims::CONTACT_USER_INFO_POLICY_AUTHORIZED_IMPU
     *
     * @return IMS_SINT32 Return the policy of setting the user info
     * @see {@code ims.contact_user_info_policy_for_non_reg_message_int}
     */
    virtual IMS_SINT32 GetUserInfoPolicyForNonRegisterMessage() const = 0;

    /**
     * @brief Indicate which policy is applied for creating geolocation pidf.
     *
     *        Possible values are,
     *        CarrierConfig::Ims::GEOLOCATION_POLICY_WITHOUT_POSITION
     *        CarrierConfig::Ims::GEOLOCATION_POLICY_WITH_POSITION
     *        CarrierConfig::Ims::GEOLOCATION_POLICY_WITH_POSITION_AND_COUNTRY
     *        CarrierConfig::Ims::GEOLOCATION_POLICY_WITHOUT_CIVIC
     *
     * @return IMS_SINT32 Return the policy of setting the user info
     * @see {@code ims.geolocation_pidf_forming_policy_int}
     */
    virtual IMS_SINT32 GetGeolocationPidfFormingPolicy() const = 0;

    /**
     * @brief Get the IMS establishment time for LTE.
     *
     * @return IMS_SINT32 Returns IMS establishment time for LTE.
     * @see {@code ims.ims_establishment_time_for_lte_sec_int}
     */
    virtual IMS_SINT32 GetImsEstablishmentTimeForLte() const = 0;

    /**
     * @brief Get the IMS establishment time for NR.
     *
     * @return IMS_SINT32 Returns IMS establishment time for NR.
     * @see {@code ims.ims_establishment_time_for_nr_sec_int}
     */
    virtual IMS_SINT32 GetImsEstablishmentTimeForNr() const = 0;

    /**
     * @brief Indicate whether DSCP(Differentiated Services (Diffserv) Codepoint)
     *        for SIP packets is set or not.
     *
     *        Specify the preferred ipcan for SIP Packets.
     *        Possible values are,
     *        CarrierConfig::Ims::PREFERRED_DSCP_NONE
     *        CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR
     *        CarrierConfig::Ims::PREFERRED_DSCP_WIFI
     *        CarrierConfig::Ims::PREFERRED_DSCP_CELLULAR_WIFI
     *
     * @return IMS_SINT32 Return the preferred IP-CAN type for SIP Packets
     * @see {@code ims.preferred_ims_dscp_int}
     */
    virtual IMS_SINT32 GetPreferredImsDscp() const = 0;

    /**
     * @brief Indicate whether the g.3gpp.accesstype feature tag indicating the access type
     *        in contact header of SIP REGISTER is included based on 3GPP TS 24.292.
     *
     *        Specify the preferred access feature tag of SIP REGISTER.
     *        Possible values are,
     *        CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_DISABLED
     *        CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED
     *        CarrierConfig::Ims::PREFERRED_ACCESSTYPE_FEATURE_TAG_ENABLED_WITHOUT_NUMERICAL_VALUE
     *
     * @return IMS_SINT32 Return the preferred IP-CAN type for SIP Packets
     * @see {@code ims.registration_preferred_accesstype_feature_tag_int}
     */
    virtual IMS_SINT32 GetRegistrationPreferredAccessTypeFeatureTag() const = 0;

    /**
     * @brief Get USSD preference method.
     *
     * @return IMS_SINT32 Returns USSD preference value.
     *
     *      CarrierConfig::USSD_OVER_CS_PREFERRED
     *      CarrierConfig::USSD_OVER_IMS_PREFERRED
     *      CarrierConfig::USSD_OVER_CS_ONLY
     *      CarrierConfig::USSD_OVER_IMS_ONLY
     *
     * @see {@code carrier_ussd_method_int}
     */
    virtual IMS_SINT32 GetUssdMethod() const = 0;

    /**
     * @brief Get the preferred IP address type
     *
     *        If both IPv4 and IPv6 addresses are assigned by the network to the UE,
     *        the UE use preferred IP address type of each operator's requirement.
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Ims::IP_VERSION_4
     * @see {@code ims.ims_preferred_iptype_int}
     */
    virtual IMS_SINT32 GetPreferredIpType() const = 0;

    /**
     * @brief Get the preferred IP address type for emergency
     *
     *        If both IPv4 and IPv6 addresses are assigned by the network to the UE,
     *        the UE use preferred IP address type of each operator's requirement.
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Ims::IP_VERSION_4
     * @see {@code imsemergency.epdn_preferred_iptype_int}
     */
    virtual IMS_SINT32 GetEmergencyPreferredIpType() const = 0;

    /**
     * @brief Get the retry attempt count about pcscfs discovered during emergency PDN/PDU setup.
     *
     *       Specify the number of emergency registration retry attempt to P-CSCFs. UE will try
     *       emergency registration with specified number of P-CSCFs when
     *       CarrierConfig::Ims::KEY_EREG_RETRY_TIMER_MILLIS_INT timer has expired. If the
     *       number is zero, UE will try registration on every P-CSCFs once. If the number of
     *       P-CSCF is less than a given number and UE's default retry policy is a
     *       CarrierConfig::Ims::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF, UE will try
     *       registration from the first P-CSCF again after attempting on all P-CSCFs.
     *       If UE doesn't support emerg-reg-retry defined in 3GPP 24.229, which is configured by
     *       CarrierConfig::Ims::KEY_EREG_RETRY_TIMER_MILLIS_INT, this configuration is
     *       discarded.
     *       As an exception, if this value is set to -1, the emergency registration retry will not
     *       be performed and will be treated as a failure. The subsequent action will be determined
     *       by the CarrierConfig::ImsEmergency::KEY_PREFERRED_EMERGENCY_REGISTRATION_INT
     *       setting value.
     *
     * @return IMS_SINT32 Return the retry attempt count
     * @see {@code imsemergency.ereg_retry_max_cnt_int}
     */
    virtual IMS_SINT32 GetEmcRegRetryMaxCnt() const = 0;

    /**
     * @brief Get the maximum time waiting for emergency registration
     *
     *        Specify the maximum time from sending a SIP REGISTER for an emergency registration
     *        until UE receives any final response from this P-CSCF. It will be stopped when the
     *        CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT timer has
     *        been stopped or expired. Upon this timer expiry, the UE considers that the emergency
     *        registration attempt for this P-CSCF has failed. The UE may retry registration on
     *        a different P-CSCF if available and restart the
     *        CarrierConfig::ImsEmergency::KEY_EREG_RETRY_TIMER_MILLIS_INT timer. If the UE has no
     *        more available P-CSCFs, the UE shall stop the
     *        CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT timer by
     *        considering the emergency registration has failed. If the value is zero, it considers
     *        that the UE doesn't support the emerg-reg-retry timer defined in 3GPP 24.229.
     *
     * @return IMS_SINT32 Return the milli-second time
     * @see {@code imsemergency.ereg_retry_timer_millis_int}
     */
    virtual IMS_SINT32 GetEmcRegRetryTimerMillis() const = 0;

    /**
     * @brief Get the IMS Server default port as per operator
     *
     * @return IMS_SINT32 Return default P-CSCF port number
     * @see {@code ims.sip_server_port_number_int}
     */
    virtual IMS_SINT32 GetPcscfPort() const = 0;

    /**
     * @brief Get the preferred transport protocol for SIP messages.
     *        Possible values are,
     *        #PREFERRED_TRANSPORT_UDP,
     *        #PREFERRED_TRANSPORT_TCP,
     *        #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP,
     *        #PREFERRED_TRANSPORT_TLS
     *
     * @return IMS_SINT32
     * @see CarrierConfig::Ims::PREFERRED_TRANSPORT_UDP
     * @see {@code ims.sip_preferred_transport_int}
     */
    virtual IMS_SINT32 GetSipPreferredTransport() const = 0;

    /**
     * @brief Get the maximum IPV4 MTU size of SIP message on Cellular.
     *
     *        If GetSipPreferredTransport() is #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP and
     *        IPv4 SIP message size is more than this value, then SIP transport will be TCP,
     *        else the SIP transport is UDP.
     *
     * @return IMS_SINT32 Return IPV4 MTU size of SIP message
     * @see {@code ims.ipv4_sip_mtu_size_cellular_int}
     */
    virtual IMS_SINT32 GetIpv4MtuSize() const = 0;

    /**
     * @brief Get the maximum IPV6 MTU size of SIP message on Cellular.
     *
     *        If GetSipPreferredTransport() is #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP and
     *        IPv6 SIP message size is more than this value, then SIP transport will be TCP,
     *        else the SIP transport is UDP.
     *
     * @return IMS_SINT32 Return IPV6 MTU size of SIP message
     * @see {@code ims.ipv6_sip_mtu_size_cellular_int}
     */
    virtual IMS_SINT32 GetIpv6MtuSize() const = 0;

    /**
     * @brief Returns the wait time in millisecond before releasing an emergency PDN.
     *
     *        Emergency PDN can be released after a specific time when the emergency call ends.
     *        This returns the delay time from the end of the call to the start of the emergency
     *        PDN release.
     *        If this is set to zero, do not request to release emergency PDN after a specific time.
     *
     *        If it is configured to release emergency PDN just after the emergency call ends
     *        by following configurations, they will take precedence over setting of this.
     *        #KEY_IPCAN_RELEASE_EMERGENCY_PDN_UPON_EMERGENCY_CALL_END_INT
     *        #KEY_RELEASE_EPDN_UPON_ECALL_END_IN_FAKE_MODE_BOOL
     *
     * @return IMS_SINT32 Return the wait time in millisecond before releasing an emergency PDN.
     * @see {@code imsemergency.wait_time_millis_for_release_epdn_after_ecall_end_int}
     */
    virtual IMS_SINT32 GetWaitTimeMillisForReleaseEPdnAfterECallEnd() const = 0;

    /**
     * @brief Indicate whether emergency call is tried without emergency registration
     *
     *        Specify the preferred policy for emergency registration.
     *        Possible values are,
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
     * @return IMS_SINT32 Return the preferred policy for emergency registration
     * @see {@code imsemergency.preferred_emergency_registration_int}
     */
    virtual IMS_SINT32 GetPreferredEmergencyRegistration() const = 0;

    /**
     * @brief Get the maximum time waiting for emergency service
     *
     *        Specify the maximum time from deciding that an emergency service is to
     *        be established until completion of the emergency registration procedure.
     *        Upon timer expiry, the UE considers the emergency REGISTER request or
     *        the emergency call attempt as failed, and stop the
     *        CarrierConfig::ImsEmergency::KEY_EMERGENCY_REGISTRATION_TIMER_MILLIS_INT timer,
     *        if running.
     *
     * @return IMS_SINT32 Return the milli-second time
     * @see {@code imsemergency.emergency_registration_timer_millis_int}
     */
    virtual IMS_SINT32 GetEmergencyRegistrationTimerMillis() const = 0;

    /**
     * @brief Indicate the DSCP for IMS signalling
     *
     *        If GetPreferredImsDscp() is not PREFERRED_DSCP_NONE, it will be set
     *        according to IP connectivity access network.
     *        DSCP integer must be between 8 and 56.
     *
     * @return IMS_SINT32 Return the DSCP value for IMS signalling
     * @see {@code ims.ims_signalling_dscp_int}
     */
    virtual IMS_SINT32 GetImsSignallingDscp() const = 0;

    /**
     * @brief Indicate whether private header like P-Cellular-Network-Info or
     *        P-Last-Access-Network-Info is supported during the registration over WiFi
     *
     *        Possible values are,
     *        CarrierConfig::ImsWfc::REGISTRATION_P_NOT_SUPPORTED
     *        CarrierConfig::ImsWfc::REGISTRATION_P_CELLULAR_NETWORK_INFO
     *        CarrierConfig::ImsWfc::REGISTRATION_P_LAST_ACCESS_NETWORK_INFO
     *
     * @return IMS_SINT32 Return the preferred private hader value
     * @see {@code imswfc.registration_private_header_int}
     */
    virtual IMS_SINT32 GetRegistrationPrivateHeader() const = 0;

    /**
     * @brief Indicate the delay time before reconnecting IMS PDN when WFC setup is failed
     *        for all P-CSCFs while UE is on CS roaming network
     *
     * @return IMS_SINT32 Return the delay time (Second)
     * @see {@code
     *      ims.pdn_reconnect_delay_on_wfc_setup_fail_all_pcscfs_with_cs_roam_sec_int}
     */
    virtual IMS_SINT32 GetPdnReconnectDelayOnWfcSetupFailAllPcscfsWithCsRoam() const = 0;

    /**
     * @brief Indicate the actual wait time policy
     *
     *        Possible values are,
     *        CarrierConfig::Ims::AWT_POLICY_RFC_RULE
     *            Follow RFC 5626 section 4.5.
     *        CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EVERY_PCSCF
     *            Indicate whether it shall calculate a wait time based on RFC 5626 4.5
     *            if the registration to every known PCSCF is attempted and handled
     *            as temporary failure without Retry-After header.
     *        CarrierConfig::Ims::AWT_POLICY_FAILURE_TO_EACH_PCSCF
     *            Indicate whether it shall calculate a wait time based on RFC 5626 4.5
     *            if the registration to each known PCSCF is attempted and handled
     *            as temporary failure without Retry-After header.
     *        CarrierConfig::Ims::AWT_POLICY_SPECIFIED_INTERVAL
     *            Follow the wait time defined in KEY_REG_RETRY_INTERVAL_SEC_INT_ARRAY.
     *        CarrierConfig::Ims::AWT_POLICY_ONLY_RETRY_AFTER
     *            If retry-after header is present in the response, register with the same P-CSCF.
     *            The retry count is defined in KEY_REG_RETRY_CNT_PER_PCSCF_WITH_RA_TIME_INT.
     *            If retry-after is not present, register with the next P-CSCF.
     *
     * @return IMS_SINT32 Return the actual wait time policy
     * @see {@code ims.reg_actual_wait_time_policy_int}
     */
    virtual IMS_SINT32 GetRegActualWaitTimePolicy() const = 0;

    /**
     * @brief Indicate the default wait time prior to proceed to the next PCSCF.
     *        If this value is greater than 0, this will overlay the actual wait time.
     *
     * @return IMS_SINT32 Return the default wait time
     * @see {@code ims.reg_default_wait_time_int}
     */
    virtual IMS_SINT32 GetRegDefaultWaitTime() const = 0;

    /**
     * @brief Get the out of service policy object
     *
     *        CarrierConfig::Ims::REG_OOS_POLICY_DEFAULT
     *            Indicate that reregistration is not tried during OOS and
     *            reregistratioin is attempted after network service state is changed to in service
     *            and registration is not expired.
     *        CarrierConfig::Ims::REG_OOS_POLICY_DESTROY
     *            Indicate that registration is terminated if registration is refreshed during OOS.
     *            When network service state is changed to in service,
     *            the initial registration is tried.
     *
     * @return IMS_SINT32 Return the out of service policy
     * @see {@code ims.reg_out_of_service_policy_int}
     */
    virtual IMS_SINT32 GetRegOutOfServicePolicy() const = 0;

    /**
     * @brief Check what registration rule is required in making an emergency call in roaming.
     *
     *       Specify the preferred policy for emergency registration in roaming.
     *       Return value is to follow used configuration in GetPreferredEmergencyRegistration().
     *       (Additionally, PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED config is added
     *        for this function.)
     *        Possible values are,
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_SKIP
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_NORMAL
     *        CarrierConfig::ImsEmergency::PREFERRED_EMERGENCY_REGISTRATION_FALLBACK
     * @note  If 'PREFERRED_EMERGENCY_REGISTRATION_NOT_DEFINED' is returned,
     *        the UE in roaming follows the same rule as the home.
     *
     * @return IMS_SINT32 Return the emergency registration policy in roaming
     * @see {@code imsemergency.roaming_preferred_ereg_int}
     */
    virtual IMS_SINT32 GetRoamingPreferredEmcReg() const = 0;

    /**
     * @brief Get the SIP message threshold size caused by the transport change
     *
     *        If GetSipPreferredTransport() is #PREFERRED_TRANSPORT_DYNAMIC_UDP_TCP and
     *        SIP message size is more than MTU size - this value, then SIP transport will be TCP,
     *        else the SIP transport is UDP.
     *
     * @return IMS_SINT32 Return the threshold size of SIP message
     * @see {@code ims.sip_message_threshold_for_transport_change_int}
     */
    virtual IMS_SINT32 GetSipMessageThresholdForTransportChange() const = 0;

    /**
     * @brief Indicate the SIP 503 response policy for subscription (reg event package)
     *
     *        Possible values are,
     *        CarrierConfig::Ims::SIP_503_CODE_POLICY_DEFAULT
     *         - Follow default retry operation. (Retry SUBSCRIBE message after retry-after or AWT)
     *        CarrierConfig::Ims::SIP_503_CODE_POLICY_3GPP
     *         - Follow 3GPP 24.229.
     *
     * @return IMS_SINT32 Return the SIP 503 response policy
     * @see {@code ims.sub_retry_503_policy_int}
     */
    virtual IMS_SINT32 GetSubRetrySip503CodePolicy() const = 0;

    /**
     * @brief Indicate the USAT IMS registration event download policy.
     *
     *        Specify the support policy of USAT IMS registration event download
     *        TS 131.111 7.5.21 describes about the USAT IMS registration event download
     *        and the conditions to be notified to UICC when an IMS registration event occurs.
     *        This asset indicates whether USAT IMS registration event download is supported,
     *        and if so, whether to check the precondition before notifying.
     *        Possible values are,
     *        CarrierConfig::Ims::USAT_REG_EVENT_NOT_DOWNLOAD
     *        CarrierConfig::Ims::USAT_REG_EVENT_UNCONDITIONAL_DOWNLOAD
     *        CarrierConfig::Ims::USAT_REG_EVENT_CONDITIONAL_DOWNLOAD
     *
     * @return IMS_SINT32 Returns registration event download policy for USAT.
     * @see {@code ims.usat_reg_event_download_policy_int}
     */
    virtual IMS_SINT32 GetUsatRegEventDownloadPolicy() const = 0;

    /**
     * @brief Get the VoLTE Hysteresis time.
     *
     * @return IMS_SINT32 Returns VoLTE Hysteresis time.
     * @see {@code imsvoice.volte_hys_time_sec_int}
     */
    virtual IMS_SINT32 GetVolteHysTime() const = 0;

    /**
     * @brief  Indicate retry count to block wifi registration when receiving error response to the
     *         reg event package consecutively
     *
     *         The retry count is increased if delivering the wfc error message below.
     *         CarrierConfig::ImsWfc::KEY_WFC_ERR_SUB_403_STRING
     *         CarrierConfig::ImsWfc::KEY_WFC_ERR_NOTIFY_TERMINATED_STRING
     *         The retry count is reset if the error response is not received consecutively.
     *
     * @note   This asset is used if `CarrierConfig::ImsWfc::KEY_WFC_ERR_MESSAGE_BUNDLE` is set.
     * @return IMS_SINT32 Return the number of retry
     * @see {code imswfc.sub_consecutive_retry_cnt_for_reg_forbidden_in_wifi_int}
     */
    virtual IMS_SINT32 GetSubConsecutiveRetryCntForRegForbiddenInWifi() const = 0;

    /**
     * @brief Indicate the SIP 305 response policy for registration
     *
     *        Possible values are,
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT
     *            Follow KEY_DEFAULT_RETRY_POLICY_INT operation if 305 code is not configured
     *            from other configurations of this bundle.
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP
     *            Follow 3GPP 24.229.
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF
     *            Follow 3GPP 24.229 starting from the top of the existing PCSCF list.
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_USE_CONTACT_VALUE
     *            Flag indicating whether a new IMS registration is tried
     *            using the contact header field value as specified in RFC3261
     *            after a 305 response for registration is received.
     *
     * @return IMS_SINT32 Return the SIP 305 response policy
     * @see {@code ims.reg_retry_305_policy_int}
     */
    virtual IMS_SINT32 GetRegRetrySip305CodePolicy() const = 0;

    /**
     * @brief Indicate the delay time to release pdn after temporary plmn block.
     *
     *        If the value is 0, release ims pdn immediately after temporary plmn block.
     *        If the value is greater than 0, release ims pdn in the value(delay time) after
     *        temporary plmn block.
     *
     * @return Return the delay time (Seconds)
     * @see {@code ims.release_pdn_delay_sec_after_temp_plmn_block_int}
     */
    virtual IMS_SINT32 GetReleasePdnDelaySecAfterTempPlmnBlock() const = 0;

    /**
     * @brief Indicate the SIP 305 response policy for reregistration
     *
     *        Possible values are,
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_DEFAULT
     *            Follow KEY_DEFAULT_RETRY_POLICY_INT operation if 305 code is not configured
     *            from other configurations of this bundle.
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP
     *            Follow 3GPP 24.229.
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_3GPP_USING_TOP_PCSCF
     *            Follow 3GPP 24.229 starting from the top of the existing PCSCF list.
     *        CarrierConfig::Ims::SIP_305_CODE_POLICY_USE_CONTACT_VALUE
     *            Flag indicating whether a new IMS registration is tried
     *            using the contact header field value as specified in RFC3261
     *            after a 305 response for registration is received.
     *
     * @return IMS_SINT32 Return the SIP 305 response policy
     * @see {@code ims.rereg_retry_305_policy_int}
     */
    virtual IMS_SINT32 GetReregRetrySip305CodePolicy() const = 0;

    /**
     * @brief Indicate the SIP 503 response policy for registration
     *
     *        Possible values are,
     *        CarrierConfig::Ims::SIP_503_CODE_POLICY_DEFAULT
     *            Follow KEY_DEFAULT_RETRY_POLICY_INT operation if 305 code is not configured
     *            from other configurations of this bundle.
     *        CarrierConfig::Ims::SIP_503_CODE_POLICY_3GPP
     *            Follow 3GPP 24.229.
     *
     * @return IMS_SINT32 Return the SIP 503 response policy
     * @see {@code ims.reg_retry_503_policy_int}
     */
    virtual IMS_SINT32 GetRegRetrySip503CodePolicy() const = 0;

    /**
     * @brief Get the number of registration retry when there is only one PCSCF.
     *
     *        Indicate the number of retry for the PCSCF when there is only one PCSCF.
     *        If the value is 0, it is not applicable.
     *
     * @return IMS_SINT32 Return the number of retry
     * @see {@code ims.reg_retry_cnt_on_single_pcscf_int}
     */
    virtual IMS_SINT32 GetRegRetryCountOnSinglePcscf() const = 0;

    /**
     * @brief Get the number of registration retry for each PCSCF
     *
     *        Indicate the number of retry for each PCSCF.
     *        If the value is 0, it follows default scheme.
     *
     * @return IMS_SINT32 Return the number of retry
     * @see {@code ims.reg_retry_cnt_per_pcscf_int}
     */
    virtual IMS_SINT32 GetRegRetryCountPerPcscf() const = 0;

    /**
     * @brief Get retry count to try to register with that P-CSCF if retry-after header is present
     *        in the response.
     *
     *        If retry-after header is present in the response, register with the same P-CSCF.
     *        It's applied if CarrierConfig::IMS::KEY_REG_ACTUAL_WAIT_TIME_POLICY_INT is
     *        CarrierConfig::Ims::AWT_POLICY_ONLY_RETRY_AFTER.
     *
     * @return IMS_SINT32 Return the number of retry
     * @see {@code ims.reg_retry_cnt_per_pcscf_with_ra_time_int}
     */
    virtual IMS_SINT32 GetRegRetryCountPerPcscfWithRaTime() const = 0;

    /**
     * @brief Indicate the policy for clearing the registration retry count
     *
     *        Possible values are,
     *        CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_REGISTRATION
     *        CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_SUBSCRIPTION
     *        CarrierConfig::Ims::REG_RETRY_CNT_RESET_POLICY_NOTIFY
     *
     * @return IMS_SINT32 Return the policy of clearing the retry count for registration
     * @see {@code ims.reg_retry_cnt_reset_policy_int}
     */
    virtual IMS_SINT32 GetRegRetryCountResetPolicy() const = 0;

    /**
     * @brief Get the number of registration retry to maintain ipsec setting on authentication
     *        failure.
     *
     * @return IMS_SINT32 Return the number of retry
     * @see {@code ims.reg_retry_cnt_with_ipsec_on_auth_failure_int}
     */
    virtual IMS_SINT32 GetRegRetryCountWithIpsecOnAuthFailure() const = 0;

    /**
     * @brief Get the default policy for the registration retry with pcscf selection.
     *
     *        Specify the default retry policy about how to use the PCSCF address selection.
     *        Possible values are,
     *        CarrierConfig::Ims::DEFAULT_RETRY_POLICY_SPEC
     *        CarrierConfig::Ims::DEFAULT_RETRY_POLICY_CIRCULAR_NEXT_PCSCF
     *        CarrierConfig::Ims::DEFAULT_RETRY_POLICY_NEXT_PCSCF
     *
     * @return IMS_SINT32 Return the default policy for the registration retry
     * @see {@code ims.reg_retry_default_policy_int}
     */
    virtual IMS_SINT32 GetRegRetryDefaultPolicy() const = 0;

    /**
     * @brief Get the Reg Retry Timer F Policy object
     *
     *        This is related to managing pcscf. In case the UE uses this config.
     *        A value of KEY_REG_ERR_CODE_FOR_PCSCF_DISCOVERY_INT_ARRAY and
     *        KEY_REREG_ERR_CODE_FOR_INIT_REG_WITH_AVAILABLE_PCSCF_INT_ARRAY must be set to 0
     *        as well. (0: TimerF)
     *
     * @return IMS_SINT32 Return retry policy
     * @see {@code ims.reg_retry_timer_f_policy_int}
     */
    virtual IMS_SINT32 GetRegRetryTimerFPolicy() const = 0;

    /**
     * @brief Get a waiting max timer to proceed IMS registration for an emergency call.
     *
     *        After the timer expires, the emergency call is handled as failure.
     *        Indicate the number as millisecond.
     *
     * @return IMS_SINT32 Return a waiting reg timer for an emergency call
     * @see {@code imsemergency.reg_timer_for_ecall_millis_int}
     */
    virtual IMS_SINT32 GetRegTimerForEmcCall() const = 0;

    /**
     * @brief Indicate the extra error type for registration
     *
     *        Possible values are,
     *        CarrierConfig::Ims::ERROR_TYPE_NOT_SPECIFIED
     *        CarrierConfig::Ims::ERROR_TYPE_REPEATED
     *            Indicate that it results in blocking PLMN with the specific protocol timer.
     *            like T3402.
     *        CarrierConfig::Ims::ERROR_TYPE_CRITICAL
     *            Indicate that it results in blocking PLMN.
     *        CarrierConfig::Ims::ERROR_TYPE_ROAMING
     *            Indicate that it results in blocking PLMN basd on the attached network type.
     *        CarrierConfig::Ims::ERROR_TYPE_REPEATED_WITH_ONLY_ATTACHED_NETWORK
     *            Indicate that it results in blocking PLMN with the specific protocol timer
     *            like T3402.
     *        CarrierConfig::Ims::ERROR_TYPE_RAT_BLOCK
     *            Indicate that it results in blocking access based on the RAT.
     *
     * @return IMS_SINT32 Return the extra error type
     * @see {@code ims.extra_reg_err_final_type_int}
     */
    virtual IMS_SINT32 GetExtraRegErrFinalType() const = 0;

    /**
     * @brief Indicate the extra error policy for registration
     *
     *        Possible values are,
     *        CarrierConfig::Ims::ERROR_POLICY_NOT_SPECIFIED
     *        CarrierConfig::Ims::ERROR_POLICY_PCSCF_FAILED
     *        CarrierConfig::Ims::ERROR_POLICY_SUBSCRIBER_FAILED
     *        CarrierConfig::Ims::ERROR_POLICY_PDN_REACTIVATED
     *
     * @return IMS_SINT32 Return the extra error policy
     * @see {@code ims.extra_reg_err_policy_int}
     */
    virtual IMS_SINT32 GetExtraRegErrPolicy() const = 0;

    /**
     * @brief Indicate max of retry count the extra error for registration
     *
     *      CarrierConfig::Ims::ERROR_TYPE_REPEATED
     *          Indicate the maximum retry count with the same PCSCF.
     *          If the retry count reaches the maximum count, initial registration is tried
     *          with other PCSCF.
     *      CarrierConfig::Ims::ERROR_TYPE_CRITICAL
     *          Indicate the number of error response that is included in KEY_ERROR_CODE_INT_ARRAY.
     *          If this number reaches, it is handled as a critical error.
     *
     * @return IMS_SINT32 Return max of the extra error
     * @see {@code ims.extra_reg_err_max_cnt_int}
     */
    virtual IMS_SINT32 GetExtraRegErrMaxCount() const = 0;

    /**
     * @brief Indicate which the PCSCF address  to use when the PCSCF address are changed
     *        with address and order.
     *
     *        Possible values are,
     *        CarrierConfig::Ims::REG_PCSCF_UPDATE_POLICY_DEFAULT
     *            Indicate that registration is tried with new PCSCF address
     *            only when the current PCSCF address that is used for registration
     *            is not contained in the new PCSCF list.
     *        CarrierConfig::Ims::REG_PCSCF_UPDATE_POLICY_ALL_THE_TIME
     *            Indicate that registration or reregistration is always tried
     *            when the PCSCF addresses are changed regardless of the existence
     *            of the current PCSCF address in the new PCSCF list.
     *
     * @return IMS_SINT32 Return the policy for which the pcscf address to use
     * @see {@code ims.reg_pcscf_update_policy_int}
     */
    virtual IMS_SINT32 GetRegistrationPcscfUpdatePolicy() const = 0;

    /**
     * @brief Get the retry attempt count about pcscfs being discovered in combined network
     *
     * @return IMS_SINT32 Return the retry attempt count
     * @see {@code ims.extra_reg_err_pcscfs_repeated_cnt_for_lte_combined_attached_int}
     */
    virtual IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForLteCombinedAttached() const = 0;

    /**
     * @brief Get the retry attempt count about pcscfs being discovered in only attached network
     *
     * @return IMS_SINT32 Return the retry attempt count
     * @see {@code ims.extra_reg_err_pcscfs_repeated_cnt_for_eps_5gs_only_attached_int}
     */
    virtual IMS_SINT32 GetExtraRegErrPcscfsRepeatedCntForEps5gsOnlyAttached() const = 0;

    /**
     * @brief Get the registration retry intervals for using when registration is failed
     *        as general codes.
     *
     *        These values represent wait times for next registration for retry.
     *        It defines in second.
     *
     * @return ImsVector<IMS_SINT32>& Return registration retry intervals.
     * @see {@code ims.reg_retry_interval_sec_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegRetryIntervals() = 0;

    /**
     * @brief Get the registration retry random intervals following the registration retry intervals
     *
     *        These values represent plus random value upper bound of the registration retry
     *        intervals by GetRegRetryIntervals().
     *        So the size of the return value has to be the same as the return value of
     *        GetRegRetryIntervals(). It doesn't work if the size is different.
     *        It defines in second.
     *
     * @return ImsVector<IMS_SINT32>& Return random value for registration retry intervals
     * @see {@code ims.reg_retry_interval_random_upper_value_sec_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegRandomRetryIntervals() = 0;

    /**
     * @brief Get the ipsec authentication algorithms which will be used.
     *
     *        These algorithms consist of the IpSecType class
     *
     * @return ImsVector<IMS_SINT32>& Return what authentication algorithms will be used
     * @see {@code ims.ipsec_authentication_algorithms_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetIpsecAuthenticationAlgorithms() = 0;

    /**
     * @brief Get the ipsec encryption algorithms which will be used.
     *
     *        These algorithms consist of the IpSecType class
     *
     * @return ImsVector<IMS_SINT32>& Return what encryption algorithms will be used
     * @see {@code ims.ipsec_encryption_algorithms_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetIpsecEncryptionAlgorithms() = 0;

    /**
     * @brief Get what NOTIFY event that is condition to perform initial registration.
     *
     *        This function is called when it receives a NOTIFY message with a terminated state.
     *        And decide whether to perform initial registration depending on the event.
     *        These events consist of bit masking.
     *        It defines as like NOTIFY_TERMINATED_EXPIRED = 0x01
     *        This function relates to GetNotifyEventForInitialRegWithWaitTime() if there are values
     *        in GetNotifyEventForInitialRegWithWaitTime(), use the values.
     *
     * @return IMS_UINT32 Return value whether to perform initial registration
     * @see {@code ims.notify_terminated_for_init_reg_used_event_int_array}
     */
    virtual IMS_UINT32 GetNotifyEventForInitialRegistration() const = 0;
    /**
     * @brief Get the set wait time to perform initial registration by NOTIFY message.
     *
     *        If there is 0, perform initial registration immediately.
     *        This function relates to GetNotifyEventForInitialRegWithWaitTime() and when it's an
     *        event taken from GetNotifyEventForInitialRegWithWaitTime() this return value is used
     *
     * @return IMS_SINT32 Return set wait time value
     * @see {@code ims.notify_terminated_for_init_reg_with_wait_time_int}
     */
    virtual IMS_SINT32 GetNotifyWaitTime() const = 0;
    /**
     * @brief Get what NOTIFY event that is condition to perform initial registration
     *        after set wait time..
     *
     *        This function is called when it receives a NOTIFY message with a terminated state.
     *        And decide whether to perform initial registration after the set wait time depending
     *        on the event. These events consist of bit masking.
     *        It defines as like NOTIFY_TERMINATED_EXPIRED = 0x01
     *        This function relates to GetNotifyWaitTime() and use wait time taken from
     *        GetNotifyWaitTime() at these return value
     *
     * @return IMS_UINT32 Return value whether to perform initial registration with wait time
     * @see {@code ims.notify_terminated_for_init_reg_used_event_with_wait_time_int_array}
     */
    virtual IMS_UINT32 GetNotifyEventForInitialRegWithWaitTime() const = 0;

    /**
     * @brief Returns max count to retry with fixed wait time.
     *        When the fail count does not reach this value yet, it waits during fixed time before
     *        retrying PCSCF recovery.
     *
     * @return IMS_SINT32 Return max count to retry with fixed wait time.
     * @see {@code ims.pcscf_recovery_max_cnt_int}
     */
    virtual IMS_SINT32 GetPcscfRecoveryMaxRetryCnt() const = 0;

    /**
     * @brief Returns wait time in seconds before retrying PCSCF recovery.
     *        This is used as waiting time for PCSCF recovery during fail count is under max count.
     *        If valid PCSCF acquisition fails during this time, IMS PDN reestablishment will be
     *        requested for PCSCF recovery.
     *
     * @return IMS_SINT32 Return wait time before retrying PCSCF recovery.
     * @see {@code ims.pcscf_recovery_wait_time_sec_int}
     */
    virtual IMS_SINT32 GetPcscfRecoveryWaitTime() const = 0;

    /**
     * @brief Returns base time in seconds that is used to calculate upper-bound wait time
     *        described in RFC5626. When the fail count reaches max count, waiting time for
     *        PCSCF recovery is determined as a random value between the upper-bound wait time
     *        and half of it.
     *
     * @return IMS_SINT32 Return base time value of upper-bound wait time.
     * @see {@code ims.pcscf_recovery_base_time_sec_int}
     */
    virtual IMS_SINT32 GetPcscfRecoveryBaseTime() const = 0;

    /**
     * @brief Returns max time in seconds that is used to calculate upper-bound wait time
     *        described in RFC5626. When the fail count reaches max count, waiting time for
     *        PCSCF recovery is determined as a random value between the upper-bound wait time
     *        and half of it.
     *
     * @return IMS_SINT32 Return max time value of upper-bound wait time.
     * @see {@code ims.pcscf_recovery_max_time_sec_int}
     */
    virtual IMS_SINT32 GetPcscfRecoveryMaxTime() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to perform
     *        initial registration.
     *        Negative value elements are used for exclusion purposes.
     *
     *        This function relates to GetRetryCountSubErrorRegRequired()
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     * @see {@code ims.sub_err_code_for_init_reg_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorRegRequired() = 0;

    /**
     * @brief Get the number of error responses against SUBSCRIBE message that is condition to
     *        perform initial registration.
     *
     *        This function relates to GetSubErrorRegRequired()
     *
     * @return IMS_SINT32 Return retry count for initial registration
     * @see {@code ims.sub_err_code_for_init_reg_with_retry_max_cnt_int}
     */
    virtual IMS_SINT32 GetRetryCountSubErrorRegRequired() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to perform
     *        initial registration with next pcscf.
     *        Negative value elements are used for exclusion purposes.
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     * @see {@code ims.sub_err_code_for_init_reg_with_next_pcscf_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorRegRequiredWithNextPcscf() = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to terminate
     *        its subscription.
     *        Negative value elements are used for exclusion purposes.
     *
     *        This function relates to GetRetryCountSubErrorSubTerminated()
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     * @see {@code ims.sub_err_code_for_terminated_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorSubTerminated() = 0;

    /**
     * @brief Get the number of error responses against SUBSCRIBE message that is condition to
     *        terminate its subscription.
     *
     *        This function relates to GetSubErrorSubTerminated()
     *
     * @return IMS_SINT32 Return retry count for terminating subscription
     * @see {@code ims.sub_err_code_for_terminated_with_retry_max_cnt_int}
     */
    virtual IMS_SINT32 GetRetryCountSubErrorSubTerminated() const = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg that is condition to stop
     *        resending SUBSCRIBE msg.
     *
     *        When there are error responses, it stops retrying the current subscription and keeps
     *        its subscription until its expiration time
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     * @see {@code ims.sub_err_code_for_stopping_by_expiration_time_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetSubErrorStoppingResub() = 0;

    /**
     * @brief Get error response information against SUBSCRIBE msg in Wifi that is condition to
     *        perform initial registration.
     *        Negative value elements are used for exclusion purposes.
     *
     * @return ImsVector<IMS_SINT32>& return array list for error response information
     * @see {@code imswfc.vowifi_sub_err_code_for_init_reg_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetVowifiSubErrorRegRequired() = 0;

    /**
     * @brief Get a priority of ISIM and USIM provisioning to obtain IMS Identity.
     *
     * @return vector list
     * @see {0} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM
     *      {1} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_USIM
     *      {2} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_ISIM_IMSI
     *      {3} CarrierConfig::Ims::IMS_IDENTITY_PRIORITY_CONF
     * @see {@code ims.ims_identity_priority_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetImsIdentityPriority() = 0;

    /**
     * @brief Get a P-CSCF address discovery methods and its preference order.
     *        For example, {0(PCO), 1(CONF)} means it supports two P-CSCF discovery methods.
     *        It uses P-CSCF address in PCO field if present.
     *        otherwise, it uses the pre-configured P-CSCF address.
     *
     * @return vector list
     * @see {@code ims.pcscf_discovery_method_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetPcscfDiscoveryMethod() = 0;

    /**
     * @brief Indicate the list of RATs where registration is updated with RAT change.
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        ...
     *
     * @return vector rat list
     * @see {@code ims.update_registration_with_rat_change_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetUpdateRegistrationWithRatChange() = 0;

    /**
     * @brief List of different RAT technologies on which IMS is supported
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN
     *
     * @return vector rat list
     * @see {@code ims.supported_rats_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetSupportedRats() = 0;

    /**
     * @brief List of different RAT technologies on which IMS is supported in roaming network
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN
     *
     * @return vector rat list
     * @see {@code ims.supported_roaming_rats_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetSupportedRoamingRats() = 0;

    /**
     * @brief List of different RAT technologies on which SMS over IMS is supported.
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_UTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_GERAN
     *
     * @return vector rat list
     * @see {@code imssms.sms_over_ims_supported_rats_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetSmsOverImsSupportedRats() = 0;

    /**
     * @brief Returns the list of RAT technologies on which emergency call using IMS is supported.
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_IWLAN
     *
     * @return vector rat list on which emergency call using IMS is supported
     * @see {@code imsemergency.emergency_over_ims_supported_rats_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetEmergencyOverImsSupportedRats() = 0;

    /**
     * @brief Indicate the error codes for registration
     *        - ERROR_POLICY_PCSCF
     *        Indicate the error codes that result in initial registration
     *        with other PCSCFs available.
     *        - ERROR_POLICY_SUBSCRIBER_FAILED
     *        Indicate the list of error codes that result in initial registration
     *        with another PCSCF after wait-time.
     *        If initial registration fails with all PCSCFs and another IMPU is available,
     *        registration is retried with changed IMPU.
     *        After it fails with both IMPUs based on MSISDN and IMSI,
     *        registration is handled as critcal error with PLMN block
     *        (wait time - KEY_WAIT_TIME_SEC_INT)
     *        - ERROR_POLICY_PDN_REACTIVATED
     *        Indicate the error causes that result in new PCSCF discovery
     *        when the calculated retry number reaches.
     *        - Possible wildcard values except for error codes are,
     *        CarrierConfig::Ims::REG_ERROR_CODE_TIMER_F, etc
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code list
     * @see {@code ims.extra_reg_err_code_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetExtraRegErrCode() = 0;

    /**
     * @brief Indicate the error codes for reregistration
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code list
     * @see {@code ims.extra_reg_err_code_for_update_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetExtraReregErrCode() = 0;

    /**
     * @brief Indicate the list of wait-time seconds when registration is retried.
     *
     * @return vector wait time list
     * @see {@code ims.extra_reg_err_wait_time_sec_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetExtraRegErrWaitTime() = 0;

    /**
     * @brief Returns the list of policies that are needed to keep the IMS registration with mmtel
     *        feature tag.
     *
     *        Possible values are,
     *        CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_VOPS
     *        CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_SSAC
     *        CarrierConfig::Ims::UNAVAILABLE_FEATURE_POLICY_3G
     *
     * @return vector policy list that needs to keep the IMS registration with mmtel feature tag.
     * @see {@code ims.keep_reg_with_mmtel_feature_tag_policy_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetKeepRegWithMmtelFeatureTagPolicy() = 0;

    /**
     * @brief Indicate the error codes to attempt the initial registration with same PCSCF
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code list
     * @see {@code ims.rereg_retry_err_code_for_init_reg_with_same_pcscf_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetReregRetryErrCodeForInitRegWithSamePcscf() = 0;

    /**
     * @brief Indicate the list of error responses. It shall not attempt any more IMS registrations
     *        until power cycled, switched in and out of airplane mode.
     *
     * @return vector permanent error code list
     * @see {@code ims.registration_permanent_error_code_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegPermanentErrCode() = 0;

    /**
     * @brief Indicate the number of error code considered the final result.
     *
     *        This function relates to GetRegPermanentErrCode()
     *        The UE check if how many registration will be sent through this function  when the UE
     *        receives error message that is in the GetRegPermanentErrCode()
     *
     *        This value is indicated by two numbers if the value is different in the home and
     *        roaming network. This value is indicated by a single number if the value is the same
     *        in the home and roaming network.
     *
     * @return vector max count list
     * @see {@code ims.reg_permanent_err_max_cnt_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegPermanentErrMaxCount() = 0;

    /**
     * @brief Indicate the list of error codes to attempt initial registration without ipsec.
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code list
     * @see {@code ims.reg_retry_err_code_without_ipsec_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegErrCodeWithoutIpsec() = 0;

    /**
     * @brief Indicate the list of RAT(Radio Access Technology) network types for which PLMN block
     *        should be applied when IMS registration fails on all P-CSCFs.
     *
     *        This configuration specifies whether to block the PLMN in certain network types
     *        if registration fails across all P-CSCFs.
     *        Possible values are,
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_NGRAN (6)
     *        CarrierConfig::Ims::ACCESS_NETWORK_TYPE_EUTRAN (3)
     *
     * @return vector of integer RAT network types
     * @see {@code ims.reg_temp_plmn_block_rats_on_all_pcscfs_fail_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegTempPlmnBlockRatsOnAllPcscfsFail() = 0;

    /**
     * @brief Indicate the list of the error response with time value containing Retry-After header
     *        for registration retry.
     *        Negative value elements are used for exclusion purposes.
     *
     *        This function relates to IsRegErrCodeWithRetryAfterTimeOnlyDefined()
     *
     * @return vector error code list
     * @see {@code ims.reg_err_code_with_ra_time_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegErrCodeWithRetryAfterTime() = 0;

    /**
     * @brief Indicate the list of the error response with time value containing Retry-After header
     *        for reregistration retry.
     *        Negative value elements are used for exclusion purposes.
     *
     *        This function relates to IsRegErrCodeWithRetryAfterTimeOnlyDefined()
     *
     * @return vector error code list
     * @see {@code ims.reg_err_code_with_ra_time_for_update_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeWithRetryAfterTime() = 0;

    /**
     * @brief Indicate the error codes of the registration followed by PCSCF discovery
     *        when PCSCF is unavailable.
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code
     * @see {@code ims.reg_err_code_for_pcscf_discovery_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetRegErrCodeForPcscfDiscovery() = 0;

    /**
     * @brief Indicate the list of error codes that result in terminating the IMS call
     *        when reregistration fails.
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code
     * @see {@code ims.rereg_err_code_for_call_end_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeForCallEnd() = 0;

    /**
     * @brief Indicate the error codes of the reregistration followed by initial registration
     *        with available PCSCF. If no available PCSCF, IMS PDN is re-activated.
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code
     * @see {@code ims.rereg_err_code_for_init_reg_with_available_pcscf_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeForInitRegWithAvailablePcscf() = 0;

    /**
     * @brief Indicate the error codes of the reregistration followed by IMS PDN reactivation
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code
     * @see {@code ims.rereg_err_code_for_ims_pdn_reactivation_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetReregErrCodeForImsPdnReactivation() = 0;

    /**
     * @brief List of features that unavailable in limited registration.
     *        Possible values are,
     *        CarrierConfig::Ims::REG_FEATURE_MMTEL
     *        CarrierConfig::Ims::REG_FEATURE_VIDEO
     *        CarrierConfig::Ims::REG_FEATURE_TEXT
     *        CarrierConfig::Ims::REG_FEATURE_SMS
     *
     * @return vector features list
     * @see {@code ims.unavailable_features_in_limited_reg_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetUnavailableFeaturesInLimitedReg() = 0;

    /**
     * @brief Indicate the error codes of emergency registration which does not supported the common
     *        policy. The UE follows default retry logic when receiving the code on this list.
     *        Negative value elements are used for exclusion purposes.
     *
     * @return vector error code
     * @see {@code imsemergency.ereg_err_code_not_supported_common_policy_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetERegErrCodeNotSupportedCommonPolicy() = 0;

    /**
     * @brief Indicate the network attach causes that requires the cross stack redial.
     *
     * @return vector error causes
     * @see {@code imsemergency.network_attach_reject_causes_for_cross_stack_redial_int_array}
     */
    virtual ImsVector<IMS_SINT32>& GetNetworkAttachRejectCausesForCrossStackRedial() = 0;

    /**
     * @brief Indicate the PLMNs that require UE to release an emergency PDN/PDU after an emergency
     *        call ends.
     *
     * @return vector PLMNs
     * @see {@code imsemergency.plmns_release_epdn_upon_ecall_end_in_fake_mode_string_array}
     */
    virtual ImsVector<AString>& GetPlmnsReleaseEPdnUponECallEndInFakeMode() = 0;

    enum
    {
        NOTIFY_TERMINATED_EXPIRED = 0x01,
        NOTIFY_TERMINATED_DEACTIVATED = 0x02,
        NOTIFY_TERMINATED_PROBATION = 0x04,
        NOTIFY_TERMINATED_UNREGISTERED = 0x08,
        NOTIFY_TERMINATED_REJECTED = 0x10
    };

private:
    friend class AosBuildDirector;
    virtual void Init(IN IMS_SINT32 nSlotId = IMS_SLOT_0) = 0;
};
#endif  // INTERFACE_AOS_NCONFIGURATION_H_
