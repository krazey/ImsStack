#ifndef _SIP_PROFILE_H_
#define _SIP_PROFILE_H_

#include "AStringArray.h"
#include "RCObject.h"
#include "ISipConfig.h"
#include "SipTimerValues.h"

/**
 * @brief This class defines SIP profile for the run-time configuration of SIP engine.
 */
class SIPProfile
    : public RCObject
{
public:
    /// Indicates that the value is not provisioned for configuration item of integer type
    enum { NOT_PROVISIONED = (-10) };
    /// Default threshold length to switch from UDP to TCP
    enum { TCP_CRITERION_LEN = 1300 };

public:
    inline SIPProfile()
        : nDefaultPort(NOT_PROVISIONED)
        , nTcpCriterionLength(NOT_PROVISIONED)
        , nSIPFeatures(NOT_PROVISIONED)
        , nDeviceId(NOT_PROVISIONED)
        , strDeviceId(AString::ConstNull())
        , nTV_T1(NOT_PROVISIONED)
        , nTV_T2(NOT_PROVISIONED)
        , strTagPrefix(AString::ConstNull())
        , strUAString(AString::ConstNull())
        , nRegExpires(NOT_PROVISIONED)
        , objRegAllowMethods(AStringArray::ConstNull())
        , strRegUAString(AString::ConstNull())
        , nRegSubscription(NOT_PROVISIONED)
        , nRegSubExpires(NOT_PROVISIONED)
        , nConfigValue(0)
    {}
    inline SIPProfile(IN CONST SIPProfile &objRHS)
        : RCObject(objRHS)
        , nDefaultPort(objRHS.nDefaultPort)
        , nTcpCriterionLength(objRHS.nTcpCriterionLength)
        , nSIPFeatures(objRHS.nSIPFeatures)
        , nDeviceId(objRHS.nDeviceId)
        , strDeviceId(objRHS.strDeviceId)
        , objTVs(objRHS.objTVs)
        , nTV_T1(objRHS.nTV_T1)
        , nTV_T2(objRHS.nTV_T2)
        , strTagPrefix(objRHS.strTagPrefix)
        , strUAString(objRHS.strUAString)
        , nRegExpires(objRHS.nRegExpires)
        , objRegAllowMethods(objRHS.objRegAllowMethods)
        , strRegUAString(objRHS.strRegUAString)
        , nRegSubscription(objRHS.nRegSubscription)
        , nRegSubExpires(objRHS.nRegSubExpires)
        , nConfigValue(objRHS.nConfigValue)
    {}
    inline virtual ~SIPProfile()
    {}

private:
    SIPProfile& operator=(IN CONST SIPProfile &objRHS);

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
    inline IMS_SINT32 GetDeviceId() const
    { return nDeviceId; }
    /**
     * @brief Gets a pre-defined device id (+sip.instance).
     *
     * @return The pre-defined device id.
     */
    inline const AString& GetPredefinedDeviceId() const
    { return strDeviceId; }
    /**
     * @brief Gets a SIP default port number.
     *
     * @return The port number.
     */
    inline IMS_SINT32 GetPort() const
    { return nDefaultPort; }
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
     *         #ISipConfig#SIP_FEATURE_CAPS_INVALID_MAC_ADDRESS_REQUIRED_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_LOCAL_TIME_ZONE_REQUIRED_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB\n
     *         #ISipConfig#SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_SIP_INSTANCE_FOR_CALLER_PREFERENCE\n
     *         #ISipConfig#SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE\n
     *         #ISipConfig#SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_COUNTRY_INFO_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_USER_AGENT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX\n
     *         #ISipConfig#SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN
     */
    inline IMS_SINT32 GetSipFeatureCaps() const
    { return nSIPFeatures; }
    /**
     * @brief Gets a default timer value for SIP timer T1.
     *
     * @return The timer value.
     */
    inline IMS_SINT32 GetTimerValueT1() const
    { return nTV_T1; }
    /**
     * @brief Gets a default timer value for SIP timer T2.
     *
     * @return The timer value.
     */
    inline IMS_SINT32 GetTimerValueT2() const
    { return nTV_T2; }
    /**
     * @brief Gets a tag prefix string.
     *
     * @return The tag prefix string.
     */
    inline const AString& GetTagPrefix() const
    { return strTagPrefix; }
    /**
     * @brief Gets a criteria length to convert a transport protocol from UDP to TCP.
     *
     * @return The TCP criteria length.
     */
    inline IMS_SINT32 GetTcpCriterionLength() const
    { return nTcpCriterionLength; }
    /**
     * @brief Gets a SIP timer values.
     *
     * @return The SIP timer values.
     */
    inline const SIPTimerValues& GetTimerValues() const
    { return objTVs; }
    /**
     * @brief Gets an UA string for User-Agent/Server header.
     *
     * @return The UA string.
     */
    inline const AString& GetUaString() const
    { return strUAString; }

    /**
     * @brief Gets the allowed methods for IMS registration.
     *
     * @return The allowed SIP methods.
     */
    inline const AStringArray& GetRegAllowMethods() const
    { return objRegAllowMethods; }
    /**
     * @brief Gets the expires value for IMS registration.
     *
     * @return The expires value for IMS registration.
     */
    inline IMS_SINT32 GetRegExpires() const
    { return nRegExpires; }
    /**
     * @brief Gets the expires value for "reg" event package subscription.
     *
     * @return The expires value for "reg" event package subscription.
     */
    inline IMS_SINT32 GetRegSubExpires() const
    { return nRegSubExpires; }
    /**
     * @brief Gets the reg-subscription configuration value.
     *
     * @return 1 if reg-subscription is supported. Otherwise, returns 0.
     */
    inline IMS_SINT32 GetRegSubscription() const
    { return nRegSubscription; }
    /**
     * @brief Gets an UA string for IMS registration.
     *
     * @return The UA string for IMS registration.
     */
    inline const AString& GetRegUaString() const
    { return strRegUAString; }

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
    inline void SetDeviceId(IN IMS_SINT32 nDeviceId)
    { this->nDeviceId = nDeviceId; }
    /**
     * @brief Sets the pre-defined device id (+sip.instance).
     *
     * @param strDeviceId The pre-defined device id
     */
    inline void SetPredefinedDeviceId(IN CONST AString &strDeviceId)
    { this->strDeviceId = strDeviceId; }
    /**
     * @brief Sets the SIP default port number.
     *
     * @param nPort The default port number
     */
    inline void SetPort(IN IMS_SINT32 nPort)
    { this->nDefaultPort = nPort; }
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
     *         #ISipConfig#SIP_FEATURE_CAPS_INVALID_MAC_ADDRESS_REQUIRED_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_LOCAL_TIME_ZONE_REQUIRED_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB\n
     *         #ISipConfig#SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_SIP_INSTANCE_FOR_CALLER_PREFERENCE\n
     *         #ISipConfig#SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG\n
     *         #ISipConfig#SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE\n
     *         #ISipConfig#SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_COUNTRY_INFO_IN_PANI_HEADER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER\n
     *         #ISipConfig#SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_USER_AGENT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT\n
     *         #ISipConfig#SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX\n
     *         #ISipConfig#SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN
     */
    inline void SetSIPFeatures(IN IMS_SINT32 nFeatures)
    { this->nSIPFeatures = nFeatures; }
    /**
     * @brief Sets the default SIP timer T1.
     *
     * @param nTV The timer value
     */
    inline void SetTimerValueT1(IN IMS_SINT32 nTV)
    { this->nTV_T1 = nTV; }
    /**
     * @brief Sets the default SIP timer T2.
     *
     * @param nTV The timer value
     */
    inline void SetTimerValueT2(IN IMS_SINT32 nTV)
    { this->nTV_T2 = nTV; }
    /**
     * @brief Sets the tag prefix string.
     *
     * @param strTagPrefix The tag prefix string
     */
    inline void SetTagPrefix(IN CONST AString &strTagPrefix)
    { this->strTagPrefix = strTagPrefix; }
    /**
     * @brief Sets a criteria length to convert a transport protocol from UDP to TCP.
     *
     * @param nTCPCriterionLength The criteria length
     */
    inline void SetTcpCriterionLength(IN IMS_SINT32 nTcpCriterionLength)
    { this->nTcpCriterionLength = nTcpCriterionLength; }
    /**
     * @brief Sets a SIP timer values.
     *
     * @param objTVs The SIP timer values
     */
    inline void SetTimerValues(IN CONST SIPTimerValues &objTVs)
    { this->objTVs = objTVs; }
    /**
     * @brief Sets an UA string for User-Agent/Server header.
     *
     * @param strUAString The UA string
     */
    inline void SetUAString(IN CONST AString &strUAString)
    { this->strUAString = strUAString; }

    /**
     * @brief Sets the allowed methods for IMS registration.
     *
     * @param objAllowMethods The allowed SIP methods
     */
    inline void SetRegAllowMethods(IN CONST AStringArray &objAllowMethods)
    { this->objRegAllowMethods = objAllowMethods; }
    /**
     * @brief Sets the expires value for IMS registration.
     *
     * @param nExpires The expires value for IMS registration
     */
    inline void SetRegExpires(IN IMS_SINT32 nExpires)
    { this->nRegExpires = nExpires; }
    /**
     * @brief Sets the reg-subscription configuration value.
     *
     * @param nRegSub 1 if reg-subscription is supported, 0 if not supported
     */
    inline void SetRegSubscription(IN IMS_SINT32 nRegSub)
    { this->nRegSubscription = nRegSub; }
    /**
     * @brief Sets the expires value for "reg" event package subscription.
     *
     * @param nExpires The expires value for "reg" event package subscription
     */
    inline void SetRegSubExpires(IN IMS_SINT32 nExpires)
    { this->nRegSubExpires = nExpires; }
    /**
     * @brief Sets an UA string for IMS registration.
     *
     * @param strUAString The UA string for IMS registration
     */
    inline void SetRegUAString(IN CONST AString &strUAString)
    { this->strRegUAString = strUAString; }

    /**
     * @brief Checks if SIP features are provisioned or not.
     *
     * @return If it's provisioned, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsSIPFeatureProvisioned() const
    { return nSIPFeatures != NOT_PROVISIONED; }
    /**
     * @brief Checks if the authentication algorithm is required or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsAuthenticationAlgorithmRequired() const
    { return (nSIPFeatures &
            ISipConfig::SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER) != 0; }
    /**
     * @brief Checks if the Cellular-Network-Info header is required or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsCellularNetworkInfoHeaderRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER) != 0; }
    /**
     * @brief Checks if the Contact header for all the 1xx responses should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsContactInAll1xxRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX) != 0; }
    /**
     * @brief Checks if the country information in P-Access-Network-Info header
     *        should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsCountryInfoRequiredInPANIHeader() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_COUNTRY_INFO_IN_PANI_HEADER) != 0; }
    /**
     * @brief Checks if the display name of SIP address should contain double quotation or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsDisplayNameDQUOTRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT) != 0; }
    /**
     * @brief Checks if the Expires header in REGISTER request should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsExpiresHeaderInRegRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG) != 0; }
    /**
     * @brief Checks if IPSec(IP Security) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsIpSecConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_IPSEC) != 0; }
    /**
     * @brief Checks if GRUU (Globally Routable User-agent URI) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsGRUUConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_GRUU) != 0; }
    /**
     * @brief Checks if "keep" parameter in Via header is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsKeepAliveConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_KEEP) != 0; }
    /**
     * @brief Checks if the multiple registration ("reg-id" parameter) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsMultipleRegConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_MULTIPLE_REG) != 0; }
    /**
     * @brief Checks if the multiple registration ("reg-id" parameter) is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsNoAcceptContactHeaderInBYE() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE) != 0; }
    /**
     * @brief Checks if P-Access-Network-Info header in the initial registration
     *        should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsPANInfoInInitialRegRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG) != 0; }
    /**
     * @brief Checks if P-Preferred-Identity header in "reg" subscription
     *        should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsPPreferredIdInRegSubRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB) != 0; }
    /**
     * @brief Checks if Route header in the registration should be added or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsRouteHeaderInRegRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG) != 0; }
    /**
     * @brief Checks if "rport" parameter is configured or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsRportConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_RPORT) != 0; }
    /**
     * @brief Checks if the transport layer's error notification of SIP transaction
     *        is required or not.
     *
     * @return If it's required, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsTransportErrorReportOnTxnRequired() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN) != 0; }
    /**
     * @brief Checks if the device is in the trust domain or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsTrustDomainConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_TRUST_DOMAIN) != 0; }
    /**
     * @brief Checks if the UDP fallback is configured or not
     *        when TCP connection can't be established.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsUdpFallbackConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_UDP_FALLBACK) != 0; }
    /**
     * @brief Checks if the UA string in SIP message should be added or not.
     *
     * @return If it's configured, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsUserAgentConfigured() const
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_USER_AGENT) != 0; }
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
    { return (nSIPFeatures & ISipConfig::SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT) != 0; }

    /**
     * @brief Checks if the SDP negotiation is required for non-RPR message.
     *
     * If it's true and non-RPR message contains SDP, then SIP engine handles
     * the basic SDP offer/answer procedure.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsSdpNegotiationRequiredForNonRpr() const
    { return HasFeature(ISipConfig::SIP_FEATURE_CAPS_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR); }

    /**
     * @brief Checks if the validation of Request-URI of the mid-dialog request is required.
     *
     * If it's true, then the validation of Request-URI of the mid-dialog request will be done.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsRequestUriValidationRequiredInMidDialog() const
    { return HasFeature(ISipConfig::
            SIP_FEATURE_CAPS_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG); }

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
    { return HasFeature(ISipConfig::SIP_FEATURE_CAPS_SESSION_TIMER_UPDATE_REQUIRED_BY_REINVITE); }

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
    { return HasFeature(ISipConfig::
            SIP_FEATURE_CAPS_SIP_INSTANCE_PARAM_REQUIRED_IN_CONTACT_FOR_NON_REGISTER_REQUEST); }

    /**
     * @brief Checks if the Session-Id header is supported or not.
     *
     * @return IMS_TRUE if it's supported, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsSessionIdHeaderSupported() const
    { return HasFeature(ISipConfig::SIP_FEATURE_CAPS_SUPPORT_SESSION_ID_HEADER); }

    /**
     * @brief Checks if the invalid MAC address is required in PANI header on Wi-Fi.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsInvalidMacAddressRequiredInPaniHeader() const
    { return HasFeature(ISipConfig::
            SIP_FEATURE_CAPS_INVALID_MAC_ADDRESS_REQUIRED_IN_PANI_HEADER); }

    /**
     * @brief Checks if the "local-time-zone" parameter is required in PANI header.
     *
     * @return IMS_TRUE if it's required, IMS_FALSE otherwise.
     */
    inline IMS_BOOL IsLocalTimeZoneRequiredInPaniHeader() const
    { return HasFeature(ISipConfig::SIP_FEATURE_CAPS_LOCAL_TIME_ZONE_REQUIRED_IN_PANI_HEADER); }

    // Runtime-Configuration {
    /**
     * @brief Gets the additional runtime configuration.
     *
     * @return The runtime configuration.\n
     *         Bitmasking of the following items:\n
     *         #CONFIG_IGNORE_RR_ON_IMPLICIT_ROUTE_REQUIRED
     */
    inline IMS_SINT32 GetConfiguration() const
    { return nConfigValue; }
    /**
     * @brief Checks if the additional configuration is set or not.
     *
     * @param nValue The configuration item to be checked
     * @return If the given configuration item is set, returns IMS_TRUE.
     *         Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsConfigurationSet(IN IMS_SINT32 nValue) const
    { return (nConfigValue & nValue) != 0; }
    /**
     * @brief Sets the additional runtime configuration.
     *
     * @param nValue The additional runtime configuration\n
     *               Bitmasking of the following items:\n
     *               #CONFIG_IGNORE_RR_ON_IMPLICIT_ROUTE_REQUIRED
     */
    inline void SetConfiguration(IN IMS_SINT32 nValue)
    { nConfigValue = nValue; }
    // }

private:
    inline IMS_BOOL HasFeature(IN IMS_SINT32 nSipFeature) const
    { return (nSIPFeatures & nSipFeature) != 0; }

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
    IMS_SINT32 nDefaultPort;
    // TCP criterion length on run-time
    // A default threshold value when MTU size is unknown; The default MTU size will be 1500.
    // 200 bytes is a buffer for collecting the Record-Route.
    IMS_SINT32 nTcpCriterionLength;
    // Refer to ISipConfig::FLAG_XXX
    IMS_SINT32 nSIPFeatures;
    // ISipConfig::DEVICE_ID_XXX
    IMS_SINT32 nDeviceId;
    // For pre-defined device identifier
    AString strDeviceId;
    // SIP transaction timer value: T1, T2
    SIPTimerValues objTVs;
    IMS_SINT32 nTV_T1;
    IMS_SINT32 nTV_T2;
    // Application specific tag prefix
    AString strTagPrefix;
    // UA string : sw_version + service_version
    AString strUAString;

    // Expires for Registration
    IMS_SINT32 nRegExpires;
    // SIP methods which sets in Allow header in Registration
    AStringArray objRegAllowMethods;
    // UA string for Registration
    AString strRegUAString;
    // Flag to indicate if Reg-Subscription is required or not
    IMS_SINT32 nRegSubscription;
    // Expires for Reg-Subscription
    IMS_SINT32 nRegSubExpires;

    // Runtime configuration
    IMS_SINT32 nConfigValue;
};

#endif // _SIP_PROFILE_H_
