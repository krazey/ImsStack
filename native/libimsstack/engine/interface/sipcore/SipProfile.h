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
#ifndef SIP_PROFILE_H_
#define SIP_PROFILE_H_

#include "AStringArray.h"
#include "ISipConfig.h"
#include "RcObject.h"
#include "SipTimerValues.h"

/**
 * @brief This class defines SIP profile for the run-time configuration of SIP engine.
 */
class SipProfile : public RcObject
{
public:
    /// Indicates that the value is not provisioned for configuration item of integer type
    enum
    {
        NOT_PROVISIONED = (-10)
    };
    /// Default threshold length to switch from UDP to TCP
    enum
    {
        TCP_CRITERION_LEN = 1300
    };

public:
    inline SipProfile() :
            m_nDefaultPort(NOT_PROVISIONED),
            m_nTcpCriterionLength(NOT_PROVISIONED),
            m_nSipFeatures(NOT_PROVISIONED),
            m_nDeviceId(NOT_PROVISIONED),
            m_strDeviceId(AString::ConstNull()),
            m_nTimerValueT1(NOT_PROVISIONED),
            m_nTimerValueT2(NOT_PROVISIONED),
            m_strTagPrefix(AString::ConstNull()),
            m_strUaString(AString::ConstNull()),
            m_nHideMacInPaniHeader(NOT_PROVISIONED),
            m_nRegExpires(NOT_PROVISIONED),
            m_objRegAllowMethods(AStringArray::ConstNull()),
            m_strRegUaString(AString::ConstNull()),
            m_nRegSubscription(NOT_PROVISIONED),
            m_nRegSubExpires(NOT_PROVISIONED),
            m_nConfigValue(0)
    {
    }
    inline SipProfile(IN const SipProfile& other) :
            RcObject(other),
            m_nDefaultPort(other.m_nDefaultPort),
            m_nTcpCriterionLength(other.m_nTcpCriterionLength),
            m_nSipFeatures(other.m_nSipFeatures),
            m_nDeviceId(other.m_nDeviceId),
            m_strDeviceId(other.m_strDeviceId),
            m_objTimerValues(other.m_objTimerValues),
            m_nTimerValueT1(other.m_nTimerValueT1),
            m_nTimerValueT2(other.m_nTimerValueT2),
            m_strTagPrefix(other.m_strTagPrefix),
            m_strUaString(other.m_strUaString),
            m_nHideMacInPaniHeader(other.m_nHideMacInPaniHeader),
            m_nRegExpires(other.m_nRegExpires),
            m_objRegAllowMethods(other.m_objRegAllowMethods),
            m_strRegUaString(other.m_strRegUaString),
            m_nRegSubscription(other.m_nRegSubscription),
            m_nRegSubExpires(other.m_nRegSubExpires),
            m_nConfigValue(other.m_nConfigValue)
    {
    }
    inline virtual ~SipProfile() {}

public:
    SipProfile& operator=(IN const SipProfile&) = delete;

public:
    /**
     * @brief Gets a type of the device id (+sip.instance).
     *
     * @return The type of device id(+sip.instance).\n
     *         #ISipConfig#DEVICE_ID_GSMA_IMEI\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_MD5\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_SHA1\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_NAMED_V3\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_NAMED_V5\n
     *         #ISipConfig#DEVICE_ID_UUID_IMEI_V4\n
     *         #ISipConfig#DEVICE_ID_PREDEFINED
     */
    inline IMS_SINT32 GetDeviceId() const { return m_nDeviceId; }

    /**
     * @brief Gets a pre-defined device id (+sip.instance).
     *
     * @return The pre-defined device id.
     */
    inline const AString& GetPredefinedDeviceId() const { return m_strDeviceId; }

    /**
     * @brief Gets a SIP default port number.
     *
     * @return The port number.
     */
    inline IMS_SINT32 GetPort() const { return m_nDefaultPort; }

    /**
     * @brief Gets the SIP features.
     *
     * @return The SIP features.\n
     *         Bitmasking of the following items:\n
     *         #ISipConfig#SIP_FEATURE_CAPS_IPSEC\n
     *         #ISipConfig#SIP_FEATURE_CAPS_GRUU\n
     *         #ISipConfig#SIP_FEATURE_CAPS_RPORT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_KEEP\n
     *         #ISipConfig#SIP_FEATURE_CAPS_MULTIPLE_REG\n
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
    inline IMS_SINT32 GetSipFeatureCaps() const { return m_nSipFeatures; }

    /**
     * @brief Gets a default timer value for SIP timer T1.
     *
     * @return The timer value.
     */
    inline IMS_SINT32 GetTimerValueT1() const { return m_nTimerValueT1; }

    /**
     * @brief Gets a default timer value for SIP timer T2.
     *
     * @return The timer value.
     */
    inline IMS_SINT32 GetTimerValueT2() const { return m_nTimerValueT2; }

    /**
     * @brief Gets a tag prefix string.
     *
     * @return The tag prefix string.
     */
    inline const AString& GetTagPrefix() const { return m_strTagPrefix; }

    /**
     * @brief Gets a criteria length to convert a transport protocol from UDP to TCP.
     *
     * @return The TCP criteria length.
     */
    inline IMS_SINT32 GetTcpCriterionLength() const { return m_nTcpCriterionLength; }

    /**
     * @brief Gets a SIP timer values.
     *
     * @return The SIP timer values.
     */
    inline const SipTimerValues& GetTimerValues() const { return m_objTimerValues; }

    /**
     * @brief Gets an UA string for User-Agent/Server header.
     *
     * @return The UA string.
     */
    inline const AString& GetUaString() const { return m_strUaString; }

    /**
     * @brief Gets the allowed methods for IMS registration.
     *
     * @return The allowed SIP methods.
     */
    inline const AStringArray& GetRegAllowMethods() const { return m_objRegAllowMethods; }

    /**
     * @brief Gets the policy whether MAC address should be hidden in PANI header on Wi-Fi.
     *
     * @return The policy that specifies whether hiding MAC address in PANI header is allowed or not
     *         Possible values are:
     *         #ISipConfig#SHOW_MAC_IN_PANI\n
     *         #ISipConfig#HIDE_MAC_IN_PANI\n
     *         #ISipConfig#HIDE_MAC_IN_PANI_EXCEPT_N11_AND_ECALL
     */
    inline IMS_SINT32 GetHideMacInPaniHeaderPolicy() const { return m_nHideMacInPaniHeader; }

    /**
     * @brief Gets the expires value for IMS registration.
     *
     * @return The expires value for IMS registration.
     */
    inline IMS_SINT32 GetRegExpires() const { return m_nRegExpires; }

    /**
     * @brief Gets the expires value for "reg" event package subscription.
     *
     * @return The expires value for "reg" event package subscription.
     */
    inline IMS_SINT32 GetRegSubExpires() const { return m_nRegSubExpires; }

    /**
     * @brief Gets the reg-subscription configuration value.
     *
     * @return 1 if reg-subscription is supported. Otherwise, returns 0.
     */
    inline IMS_SINT32 GetRegSubscription() const { return m_nRegSubscription; }

    /**
     * @brief Gets an UA string for IMS registration.
     *
     * @return The UA string for IMS registration.
     */
    inline const AString& GetRegUaString() const { return m_strRegUaString; }

    /**
     * @brief Sets the type of device id (+sip.instance).
     *
     * @param nDeviceId The type of device id\n
     *                  #ISipConfig#DEVICE_ID_GSMA_IMEI\n
     *                  #ISipConfig#DEVICE_ID_UUID_IMEI_MD5\n
     *                  #ISipConfig#DEVICE_ID_UUID_IMEI_SHA1\n
     *                  #ISipConfig#DEVICE_ID_UUID_IMEI_NAMED_V3\n
     *                  #ISipConfig#DEVICE_ID_UUID_IMEI_NAMED_V5\n
     *                  #ISipConfig#DEVICE_ID_UUID_IMEI_V4\n
     *                  #ISipConfig#DEVICE_ID_PREDEFINED
     */
    inline void SetDeviceId(IN IMS_SINT32 nDeviceId) { m_nDeviceId = nDeviceId; }

    /**
     * @brief Sets the pre-defined device id (+sip.instance).
     *
     * @param strDeviceId The pre-defined device id
     */
    inline void SetPredefinedDeviceId(IN const AString& strDeviceId)
    {
        m_strDeviceId = strDeviceId;
    }

    /**
     * @brief Sets the SIP default port number.
     *
     * @param nPort The default port number
     */
    inline void SetPort(IN IMS_SINT32 nPort) { m_nDefaultPort = nPort; }

    /**
     * @brief Sets the SIP features.
     *
     * @param nFeatures The SIP features\n
     *         Bitmasking of the following items:\n
     *         #ISipConfig#SIP_FEATURE_CAPS_IPSEC\n
     *         #ISipConfig#SIP_FEATURE_CAPS_GRUU\n
     *         #ISipConfig#SIP_FEATURE_CAPS_RPORT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_KEEP\n
     *         #ISipConfig#SIP_FEATURE_CAPS_MULTIPLE_REG\n
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
    inline void SetSipFeatureCaps(IN IMS_SINT32 nFeatures) { m_nSipFeatures = nFeatures; }

    /**
     * @brief Sets the default SIP timer T1.
     *
     * @param nTimerValue The timer value
     */
    inline void SetTimerValueT1(IN IMS_SINT32 nTimerValue) { m_nTimerValueT1 = nTimerValue; }

    /**
     * @brief Sets the default SIP timer T2.
     *
     * @param nTimerValue The timer value
     */
    inline void SetTimerValueT2(IN IMS_SINT32 nTimerValue) { m_nTimerValueT2 = nTimerValue; }

    /**
     * @brief Sets the tag prefix string.
     *
     * @param strTagPrefix The tag prefix string
     */
    inline void SetTagPrefix(IN const AString& strTagPrefix) { m_strTagPrefix = strTagPrefix; }

    /**
     * @brief Sets a criteria length to convert a transport protocol from UDP to TCP.
     *
     * @param nTcpCriterionLength The criteria length
     */
    inline void SetTcpCriterionLength(IN IMS_SINT32 nTcpCriterionLength)
    {
        m_nTcpCriterionLength = nTcpCriterionLength;
    }

    /**
     * @brief Sets a SIP timer values.
     *
     * @param objTimerValues The SIP timer values
     */
    inline void SetTimerValues(IN const SipTimerValues& objTimerValues)
    {
        m_objTimerValues = objTimerValues;
    }

    /**
     * @brief Sets an UA string for User-Agent/Server header.
     *
     * @param strUaString The UA string
     */
    inline void SetUaString(IN const AString& strUaString) { m_strUaString = strUaString; }

    /**
     * @brief Sets the allowed methods for IMS registration.
     *
     * @param objAllowMethods The allowed SIP methods
     */
    inline void SetRegAllowMethods(IN const AStringArray& objAllowMethods)
    {
        m_objRegAllowMethods = objAllowMethods;
    }

    /**
     * @brief Sets the policy whether MAC address should be hidden in PANI header on Wi-Fi.
     *
     * @param nPolicy The policy that specifies whether hiding MAC address in PANI header
     *                is allowed or not.
     */
    inline void SetHideMacInPaniHeaderPolicy(IN IMS_SINT32 nPolicy)
    {
        m_nHideMacInPaniHeader = nPolicy;
    }

    /**
     * @brief Sets the expires value for IMS registration.
     *
     * @param nExpires The expires value for IMS registration
     */
    inline void SetRegExpires(IN IMS_SINT32 nExpires) { m_nRegExpires = nExpires; }

    /**
     * @brief Sets the reg-subscription configuration value.
     *
     * @param nRegSub 1 if reg-subscription is supported, 0 if not supported
     */
    inline void SetRegSubscription(IN IMS_SINT32 nRegSub) { m_nRegSubscription = nRegSub; }

    /**
     * @brief Sets the expires value for "reg" event package subscription.
     *
     * @param nExpires The expires value for "reg" event package subscription
     */
    inline void SetRegSubExpires(IN IMS_SINT32 nExpires) { m_nRegSubExpires = nExpires; }

    /**
     * @brief Sets an UA string for IMS registration.
     *
     * @param strUaString The UA string for IMS registration
     */
    inline void SetRegUaString(IN const AString& strUaString) { m_strRegUaString = strUaString; }

    /**
     * @brief Checks if SIP features are provisioned or not.
     *
     * @return If it's provisioned, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsSipFeatureProvisioned() const { return m_nSipFeatures != NOT_PROVISIONED; }

    /**
     * @brief Checks if the authentication algorithm is required or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsAuthenticationAlgorithmRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER);
    }

    /**
     * @brief Checks if the Cellular-Network-Info header is required or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsCellularNetworkInfoHeaderRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER);
    }

    /**
     * @brief Checks if the Contact header for all the 1xx responses should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsContactInAll1xxRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX);
    }

    /**
     * @brief Checks if the country parameter in P-Access-Network-Info header
     *        should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsCountryParameterSupportedInPaniHeader() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER);
    }

    /**
     * @brief Checks if the display name of SIP address should contain double quotation or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsDisplayNameDquotRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT);
    }

    /**
     * @brief Checks if the Expires header in REGISTER request should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsExpiresHeaderInRegRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG);
    }

    /**
     * @brief Checks if IPSec(IP Security) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsIpSecConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_IPSEC);
    }

    /**
     * @brief Checks if GRUU (Globally Routable User-agent URI) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsGruuConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_GRUU);
    }

    /**
     * @brief Checks if "keep" parameter in Via header is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsKeepAliveConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_KEEP);
    }

    /**
     * @brief Checks if the multiple registration ("reg-id" parameter) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsMultipleRegConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_MULTIPLE_REG);
    }

    /**
     * @brief Checks if the multiple registration ("reg-id" parameter) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsNoAcceptContactHeaderInBye() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE);
    }

    /**
     * @brief Checks if P-Access-Network-Info header in the initial registration
     *        should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsPanInfoInInitialRegRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG);
    }

    /**
     * @brief Checks if P-Preferred-Identity header in "reg" subscription
     *        should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsPPreferredIdInRegSubRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB);
    }

    /**
     * @brief Checks if Route header in the registration should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsRouteHeaderInRegRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG);
    }

    /**
     * @brief Checks if "rport" parameter is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsRportConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_RPORT);
    }

    /**
     * @brief Checks if the transport layer's error notification of SIP transaction
     *        is required or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsTransportErrorReportOnTxnRequired() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN);
    }

    /**
     * @brief Checks if the device is in the trust domain or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsTrustDomainConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_TRUST_DOMAIN);
    }

    /**
     * @brief Checks if the UDP fallback is configured or not
     *        when TCP connection can't be established.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsUdpFallbackConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_UDP_FALLBACK);
    }

    /**
     * @brief Checks if the UA string in SIP message should be added or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsUserAgentConfigured() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_USER_AGENT);
    }

    /**
     * @brief Checks if the SIP header for UA string should be determined
     *        by the SIP signalling context or not.
     *
     * If it's true,
     *    - SIP request should contain User-Agent header.
     *    - SIP response should contain Server header.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsUserAgentSetByContext() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT);
    }

    /**
     * @brief Checks if the SDP negotiation is required for non-RPR message.
     *
     * If it's true and non-RPR message contains SDP, then SIP engine handles
     * the basic SDP offer/answer procedure.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsSdpNegotiationRequiredForNonRpr() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR);
    }

    /**
     * @brief Checks if the validation of Request-URI of the mid-dialog request is required.
     *
     * If it's true, then the validation of Request-URI of the mid-dialog request will be done.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsRequestUriValidationRequiredInMidDialog() const
    {
        return HasFeature(
                ISipConfig::SIP_FEATURE_CAPS_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG);
    }

    /**
     * @brief Checks if the session timer update is required when the session update is
     *        happening by the re-INVITE transaction such as call hold/resume,
     *        call switch from voice to video or vice versa.
     *
     * If it's true, then the session timer update will be done by these re-INVITE transactions.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsSessionTimerUpdateRequiredByReInvite() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_SESSION_TIMER_UPDATE_REQUIRED_BY_REINVITE);
    }

    /**
     * @brief Checks if the "+sip.instance" parameter should be included in the Contact header
     *        of non-REGISTER request.
     *
     * If it's true, then "+sip.instance" parameter will be added in the Contact header of
     * non-REGISTER request.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsSipInstanceParamRequiredInContactForNonRegisterRequest() const
    {
        return HasFeature(ISipConfig::
                        SIP_FEATURE_CAPS_SIP_INSTANCE_PARAM_REQUIRED_IN_CONTACT_FOR_NON_REGISTER_REQUEST);
    }

    /**
     * @brief Checks if the Session-Id header is supported or not.
     *
     * @return IMS_TRUE if it's supported, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsSessionIdHeaderSupported() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_SUPPORT_SESSION_ID_HEADER);
    }

    /**
     * @brief Checks if the "local-time-zone" parameter is supported in PANI header.
     *
     * @return IMS_TRUE if it's supported, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsLocalTimezoneParameterSupportedInPaniHeader() const
    {
        return HasFeature(ISipConfig::SIP_FEATURE_CAPS_LOCAL_TIMEZONE_PARAM_IN_PANI_HEADER);
    }

    // Runtime-Configuration {
    /**
     * @brief Gets the additional runtime configuration.
     *
     * @return The runtime configuration.\n
     *         Bitmasking of the following items:\n
     *         #CONFIG_IGNORE_RR_ON_IMPLICIT_ROUTE_REQUIRED
     */
    inline IMS_SINT32 GetConfiguration() const { return m_nConfigValue; }

    /**
     * @brief Checks if the additional configuration is set or not.
     *
     * @param nValue The configuration item to be checked
     * @return If the given configuration item is set, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsConfigurationSet(IN IMS_SINT32 nValue) const
    {
        return (m_nConfigValue & nValue) != 0;
    }

    /**
     * @brief Sets the additional runtime configuration.
     *
     * @param nValue The additional runtime configuration\n
     *               Bitmasking of the following items:\n
     *               #CONFIG_IGNORE_RR_ON_IMPLICIT_ROUTE_REQUIRED
     */
    inline void SetConfiguration(IN IMS_SINT32 nValue) { m_nConfigValue = nValue; }
    // }

private:
    inline IMS_BOOL HasFeature(IN IMS_SINT32 nSipFeature) const
    {
        return (m_nSipFeatures & nSipFeature) != 0;
    }

public:
    /// Runtime configuration to control J180 layer
    enum
    {
        CONFIG_NONE = 0x00000000,
        /// When Record-Route header should be ignored for implicit routing
        CONFIG_IGNORE_RR_ON_IMPLICIT_ROUTE_REQUIRED = 0x00000001,
    };

private:
    // SIP default port
    IMS_SINT32 m_nDefaultPort;
    // TCP criterion length on run-time
    // A default threshold value when MTU size is unknown; The default MTU size will be 1500.
    // 200 bytes is a buffer for collecting the Record-Route.
    IMS_SINT32 m_nTcpCriterionLength;
    // Refer to ISipConfig::FLAG_XXX
    IMS_SINT32 m_nSipFeatures;
    // ISipConfig::DEVICE_ID_XXX
    IMS_SINT32 m_nDeviceId;
    // For pre-defined device identifier
    AString m_strDeviceId;
    // SIP transaction timer value: T1, T2
    SipTimerValues m_objTimerValues;
    IMS_SINT32 m_nTimerValueT1;
    IMS_SINT32 m_nTimerValueT2;
    // Application specific tag prefix
    AString m_strTagPrefix;
    // UA string : sw_version + service_version
    AString m_strUaString;
    // Policy that specifies whether hiding MAC address in PANI header is allowed or not.
    IMS_SINT32 m_nHideMacInPaniHeader;
    // Expires for Registration
    IMS_SINT32 m_nRegExpires;
    // SIP methods which sets in Allow header in Registration
    AStringArray m_objRegAllowMethods;
    // UA string for Registration
    AString m_strRegUaString;
    // Flag to indicate if Reg-Subscription is required or not
    IMS_SINT32 m_nRegSubscription;
    // Expires for Reg-Subscription
    IMS_SINT32 m_nRegSubExpires;

    // Runtime configuration
    IMS_SINT32 m_nConfigValue;
};

#endif
