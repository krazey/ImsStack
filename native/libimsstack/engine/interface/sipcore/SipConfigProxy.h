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
#ifndef SIP_CONFIG_PROXY_H_
#define SIP_CONFIG_PROXY_H_

#include "SipProfile.h"

/**
 * @brief This class providers a helper interface to check SIP configuration
 *        (static & runtime configuration).
 */
class SipConfigProxy
{
public:
    SipConfigProxy() = delete;

public:
    // "common"
    /**
     * @brief Gets a type of the device id (+sip.instance).
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The type of device id(+sip.instance).\n
     *         #ISipConfig#DEVICE_ID_GSMA_IMEI\n
     *         #ISipConfig#DEVICE_ID_GSMA_IMEISV\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_MD5\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_SHA1\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_NAMED_V3\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_NAMED_V5\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_V4\n
     *         #ISipConfig#DEVICE_ID_PREDEFINED
     */
    static IMS_SINT32 GetDeviceId(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets a pre-defined device id (+sip.instance).
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The pre-defined device id.
     */
    static const AString& GetPredefinedDeviceId(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets a SIP default port number.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The port number.
     */
    static IMS_SINT32 GetPort(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets the SIP features.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The SIP features.\n
     *         Bitmasking of the following items:\n
     *         #ISipConfig#SIP_FEATURE_CAPS_IPSEC\n
     *         #ISipConfig#SIP_FEATURE_CAPS_GRUU\n
     *         #ISipConfig#SIP_FEATURE_CAPS_RPORT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_KEEP\n
     *         #ISipConfig#SIP_FEATURE_CAPS_TRUST_DOMAIN\n
     *         #ISipConfig#SIP_FEATURE_CAPS_UDP_FALLBACK\n
     *         #ISipConfig#SIP_FEATURE_CAPS_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR\n
     *         #ISipConfig#SIP_FEATURE_CAPS_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_SESSION_TIMER_UPDATE_REQUIRED_BY_REINVITE\n
     *         #ISipConfig#SIP_FEATURE_CAPS_SIP_INSTANCE_PARAM_REQUIRED_IN_CONTACT_FOR_NON_REGISTER_REQUEST\n
     *         #ISipConfig#SIP_FEATURE_CAPS_SUPPORT_SESSION_ID_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_LOCAL_TIMEZONE_PARAM_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB\n
     *         #ISipConfig#SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_SIP_INSTANCE_FOR_CALLER_PREFERENCE\n
     *         #ISipConfig#SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE\n
     *         #ISipConfig#SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_USER_AGENT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX\n
     *         #ISipConfig#SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN
     */
    static IMS_UINT32 GetSipFeatureCaps(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets a tag prefix string.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The tag prefix string.
     */
    static const AString& GetTagPrefix(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets a criteria length to convert a transport protocol from UDP to TCP.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The TCP criteria length.
     */
    static IMS_SINT32 GetTcpCriterionLength(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets a preferred transport protocol (udp/tcp/tls/...) type.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The transport protocol type.\n
     *         #SipConfig#TRANSPORT_TYPE_UDP\n
     *         #SipConfig#TRANSPORT_TYPE_TCP\n
     *         #SipConfig#TRANSPORT_TYPE_DYNAMIC_UDP_TCP\n
     *         #SipConfig#TRANSPORT_TYPE_TLS
     */
    static IMS_SINT32 GetTransportType(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets an UA string for User-Agent/Server header.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The UA string.
     */
    static AString GetUaString(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    // "reg"
    /**
     * @brief Gets the allowed methods for IMS registration.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The allowed SIP methods.
     */
    static const AStringArray& GetRegAllowMethods(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets the rule of user-info part of Contact header for IMS registration.
     *
     * @param nSlotId The current slot id
     * @return The rule of user-info part of Contact header.
     */
    static IMS_SINT32 GetRegContactUserInfoPart(IN IMS_SINT32 nSlotId);
    /**
     * @brief Gets the expires value for IMS registration.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The expires value for IMS registration.
     */
    static IMS_SINT32 GetRegExpires(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets the expires value for "reg" event package subscription.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The expires value for "reg" event package subscription.
     */
    static IMS_SINT32 GetRegSubExpires(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Gets an UA string for IMS registration.
     *
     * If an UA string for IMS registration doesn't exist, the one of SipConfig will be used.
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The UA string for IMS registration.
     */
    static AString GetRegUaString(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the expires value of registration is configured or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsRegExpiresConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the expires value of "reg" subscription is configured or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsRegSubExpiresConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the "reg" subscription is enabled or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's enabled, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsRegSubscriptionConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    // "service-specific"
    /**
     * @brief Gets a service specific SIP configuration.
     *
     * @param nSlotId The current slot id
     * @return Pointer to ISipConfigV.
     */
    static const ISipConfigV* GetSipConfigV(IN IMS_SINT32 nSlotId);

    // "sip-features"
    /**
     * @brief Checks if the authentication algorithm is required or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsAuthenticationAlgorithmRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the Cellular-Network-Info header is required or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsCellularNetworkInfoHeaderRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the SIP header's compact form is configured or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsCompactFormConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the Contact header for all the 1xx responses should be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsContactInAll1xxRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the country parameter in P-Access-Network-Info header
     *        should be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsCountryParameterSupportedInPaniHeader(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the display name of SIP address should contain double quotation or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsDisplayNameDquotRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the Expires header in REGISTER request should be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsExpiresHeaderInRegRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if IPSec(IP Security) is configured or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsIpSecConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if GRUU (Globally Routable User-agent URI) is configured or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsGruuConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if "keep" parameter in Via header is configured or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsKeepAliveConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the multiple registration feature is configured or not.
     *
     * @param nSlotId The current slot id
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsMultipleRegConfigured(IN IMS_SINT32 nSlotId);
    /**
     * @brief Checks if the "reg-id" parameter in REGISTER request is configured or not.
     *
     * @param nSlotId The current slot id
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsRegIdParameterConfigured(IN IMS_SINT32 nSlotId);
    /**
     * @brief Checks if the Accept-Contact header in BYE request should not be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If the Accept-Contact header is not required, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsNoAcceptContactHeaderInBye(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if P-Access-Network-Info header in the initial registration
     *        should be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsPanInfoInInitialRegRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if P-Preferred-Identity header in "reg" subscription
     *        should be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsPPreferredIdInRegSubRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if Route header in the registration should be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsRouteHeaderInRegRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if "rport" parameter is configured or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsRportConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the transport layer's error notification of SIP transaction
     *        is required or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsTransportErrorReportOnTxnRequired(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the device is in the trust domain or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsTrustDomainConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the UDP fallback is configured or not
     *        when TCP connection can't be established.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsUdpFallbackConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the UA string in SIP message should be added or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsUserAgentConfigured(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);
    /**
     * @brief Checks if the SIP header for UA string should be determined
     *        by the SIP signalling context or not.
     *
     * If it's true,
     *    - SIP request should contain User-Agent header.
     *    - SIP response should contain Server header.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    static IMS_BOOL IsUserAgentSetByContext(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Checks if the SDP negotiation is required for non-RPR message.
     *
     * If it's true and non-RPR message contains SDP, then SIP engine handles
     * the basic SDP offer/answer procedure.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsSdpNegotiationRequiredForNonRpr(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Checks if the validation of Request-URI of the mid-dialog request is required.
     *
     * If it's true, then the validation of Request-URI of the mid-dialog request will be done.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsRequestUriValidationRequiredInMidDialog(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Checks if the session timer update is required when the session update is
     *        happening by the re-INVITE transaction such as call hold/resume,
     *        call switch from voice to video or vice versa.
     *
     * If it's true, then the session timer update will be done by these re-INVITE transactions.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsSessionTimerUpdateRequiredByReInvite(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Checks if the "+sip.instance" parameter should be included in the Contact header
     *        of non-REGISTER request.
     *
     * If it's true, then "+sip.instance" parameter will be added in the Contact header of
     * non-REGISTER request.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsSipInstanceParamRequiredInContactForNonRegisterRequest(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Checks if the Session-Id header is supported or not.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return IMS_TRUE if it's supported, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsSessionIdHeaderSupported(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Gets the display rule of MAC address in PANI header when using WLAN.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return The display rule of MAC address.\n
     *         #SipConfig#SHOW_MAC_IN_PANI\n
     *         #SipConfig#HIDE_MAC_IN_PANI\n
     *         #SipConfig#HIDE_MAC_IN_PANI_EXCEPT_N11_AND_ECALL
     */
    static IMS_SINT32 GetHideMacInPaniHeaderPolicy(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    /**
     * @brief Checks if Checks if the "local-time-zone" parameter is supported in PANI header.
     *
     * SipProfile is preferred than a default SipConfig.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @return IMS_TRUE if it's supported, IMS_FALSE otherwise.
     */
    static IMS_BOOL IsLocalTimezoneParameterSupportedInPaniHeader(
            IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile = IMS_NULL);

    // "sip-timers"
    /**
     * @brief Gets the timer value to send 100 Trying response
     *        when UA core doesn't send any response for an incoming request.
     *
     * @param nSlotId The current slot id
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValue100Trying(IN IMS_SINT32 nSlotId);
    /**
     * @brief Gets a default timer value for SIP timer T1.
     *
     * @param nSlotId The current slot id
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueT1(IN IMS_SINT32 nSlotId);
    /**
     * @brief Gets a default timer value for SIP timer T2.
     *
     * @param nSlotId The current slot id
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueT2(IN IMS_SINT32 nSlotId);
    /**
     * @brief Gets a timer value for SIP timer T1.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueT1(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV = IMS_NULL);
    /**
     * @brief Gets a timer value for SIP timer T2.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueT2(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV = IMS_NULL);
    /**
     * @brief Gets a timer value for SIP timer T4.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                       if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueT4(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer A.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                       if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueA(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer B.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                       if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueB(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer D.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                       if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueD(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer E.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                       if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueE(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer F.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                       if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueF(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer G.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                         if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueG(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer H.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                         if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueH(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer I.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                         if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueI(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer J.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                         if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueJ(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
    /**
     * @brief Gets a timer value for SIP timer K.
     *
     * SipProfile is preferred than ISipConfigV.
     *
     * @param nSlotId The current slot id
     * @param pProfile The dynamic SIP profile
     * @param piSipConfigV The SIP configuration (static configuration)
     * @param bDefaultRequired A default timer value is required
     *                         if all the timers are not available
     * @return The timer value.
     */
    static IMS_SINT32 GetTimerValueK(IN IMS_SINT32 nSlotId, IN const SipProfile* pProfile,
            IN const ISipConfigV* piSipConfigV, IN IMS_BOOL bDefaultRequired = IMS_TRUE);
};

#endif
