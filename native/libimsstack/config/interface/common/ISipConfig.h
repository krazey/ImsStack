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
#ifndef INTERFACE_SIP_CONFIG_H_
#define INTERFACE_SIP_CONFIG_H_

#include "AString.h"
#include "ISipConfigV.h"

class IConfigurable;

class ISipConfig
{
protected:
    virtual ~ISipConfig() = default;

public:
    /**
     * @brief Returns the configurable interface from the sip configuration.
     *
     * @return An IConfigurable instance.
     */
    virtual IConfigurable* GetConfigurable() const = 0;

    /**
     * @brief Returns the device's default SIP port number.
     *
     * @return A default SIP port number.
     */
    virtual IMS_SINT32 GetPort() const = 0;

    /**
     * @brief Returns the instance to the service-specific SIP config.
     *
     * @return An instance of the service-specific SIP config.
     */
    virtual const ISipConfigV* GetSipConfigV() const = 0;

    /**
     * @brief Returns the SIP feature capabilities.
     *
     * @return An SIP feature capabilities.\n
     *         #SIP_FEATURE_CAPS_IPSEC\n
     *         #SIP_FEATURE_CAPS_GRUU\n
     *         #SIP_FEATURE_CAPS_RPORT\n
     *         #SIP_FEATURE_CAPS_KEEP\n
     *         #SIP_FEATURE_CAPS_MULTIPLE_REG\n
     *         #SIP_FEATURE_CAPS_TRUST_DOMAIN\n
     *         #SIP_FEATURE_CAPS_UDP_FALLBACK\n
     *         #SIP_FEATURE_CAPS_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR\n
     *         #SIP_FEATURE_CAPS_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG\n
     *         #SIP_FEATURE_CAPS_SESSION_TIMER_UPDATE_REQUIRED_BY_REINVITE\n
     *         #SIP_FEATURE_CAPS_SIP_INSTANCE_PARAM_REQUIRED_IN_CONTACT_FOR_NON_REGISTER_REQUEST\n
     *         #SIP_FEATURE_CAPS_SUPPORT_SESSION_ID_HEADER\n
     *         #SIP_FEATURE_CAPS_LOCAL_TIMEZONE_PARAM_IN_PANI_HEADER\n
     *         #SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG\n
     *         #SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB\n
     *         #SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG\n
     *         #SIP_FEATURE_CAPS_SIP_INSTANCE_FOR_CALLER_PREFERENCE\n
     *         #SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG\n
     *         #SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE\n
     *         #SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER\n
     *         #SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER\n
     *         #SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER\n
     *         #SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT\n
     *         #SIP_FEATURE_CAPS_USER_AGENT\n
     *         #SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT\n
     *         #SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX\n
     *         #SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN\n
     */
    virtual IMS_UINT32 GetSipFeatureCaps() const = 0;

    /**
     * @brief Returns the service-specific version information of the device.
     *
     * Thif configuration is used for forming SIP header -  User-Agent / Server.
     *
     * @return A version information of the device or service.
     */
    virtual const AString& GetUaVersion() const = 0;

    /**
     * @brief Checks if the GRUU attribute is configured or not.
     *
     * @return If the device configures GRUU attribute, returns IMS_TRUE. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsGruuConfigured() const = 0;

public:
    /// Flags for SIP feature (header or parameter fields)
    enum
    {
        SIP_FEATURE_CAPS_NONE = 0,

        /// Standard-based requirements
        SIP_FEATURE_CAPS_IPSEC = 1,
        SIP_FEATURE_CAPS_GRUU = 1 << 1,
        SIP_FEATURE_CAPS_RPORT = 1 << 2,
        SIP_FEATURE_CAPS_KEEP = 1 << 3,
        SIP_FEATURE_CAPS_MULTIPLE_REG = 1 << 4,
        SIP_FEATURE_CAPS_TRUST_DOMAIN = 1 << 5,
        SIP_FEATURE_CAPS_UDP_FALLBACK = 1 << 6,

        /// For SIP engine operations
        SIP_FEATURE_CAPS_SDP_NEGOTIATION_REQUIRED_FOR_NON_RPR = 1 << 8,
        SIP_FEATURE_CAPS_REQUEST_URI_VALIDATION_REQUIRED_IN_MID_DIALOG = 1 << 9,
        SIP_FEATURE_CAPS_SESSION_TIMER_UPDATE_REQUIRED_BY_REINVITE = 1 << 10,
        SIP_FEATURE_CAPS_SIP_INSTANCE_PARAM_REQUIRED_IN_CONTACT_FOR_NON_REGISTER_REQUEST = 1 << 11,
        SIP_FEATURE_CAPS_SUPPORT_SESSION_ID_HEADER = 1 << 12,
        SIP_FEATURE_CAPS_LOCAL_TIMEZONE_PARAM_IN_PANI_HEADER = 1 << 13,

        /// SIP header control
        /// 3GPP :: not required
        SIP_FEATURE_CAPS_PANI_HEADER_IN_INITIAL_REG = 1 << 16,
        /// 3GPP :: not required
        SIP_FEATURE_CAPS_PPI_HEADER_IN_REG_SUB = 1 << 17,
        /// 3GPP :: optional (as default, expires header parameter is used)
        SIP_FEATURE_CAPS_EXPIRES_HEADER_IN_REG = 1 << 18,
        /// For service routing evaluation ("+sip.instance" parameter)
        SIP_FEATURE_CAPS_SIP_INSTANCE_FOR_CALLER_PREFERENCE = 1 << 19,
        /// 3GPP :: not required
        SIP_FEATURE_CAPS_ROUTE_HEADER_IN_REG = 1 << 20,
        SIP_FEATURE_CAPS_NO_ACCEPT_CONTACT_HEADER_IN_BYE = 1 << 21,
        /// 3GPP :: Cellular-Network-Info (device is connected to ePDG via WLAN)
        SIP_FEATURE_CAPS_CELLULAR_NETWORK_INFO_HEADER = 1 << 22,
        /// Operator specific (device is connected to ePDG via WLAN)
        SIP_FEATURE_CAPS_COUNTRY_PARAM_IN_PANI_HEADER = 1 << 23,

        /// Operator specific requirements
        SIP_FEATURE_CAPS_AUTHENTICATION_ALGORITHM_PARAMETER = 1 << 24,
        SIP_FEATURE_CAPS_UA_SET_BY_CONTEXT = 1 << 25,
        SIP_FEATURE_CAPS_USER_AGENT = 1 << 26,
        SIP_FEATURE_CAPS_DISPLAY_NAME_DQUOT = 1 << 27,
        SIP_FEATURE_CAPS_CONTACT_IN_ALL_1XX = 1 << 28,
        SIP_FEATURE_CAPS_TRANSPORT_ERROR_REPORT_ON_TXN = 1 << 29
    };

    /// For GRUU, type of device id (+sip.instance, URN)
    enum
    {
        DEVICE_ID_NONE = (-1),  /// if device id is not used
        DEVICE_ID_GSMA_IMEI = 0,
        DEVICE_ID_UUID_IMEI_MD5,
        DEVICE_ID_UUID_IMEI_SHA1,
        DEVICE_ID_UUID_IMEI_NAMED_V3,
        DEVICE_ID_UUID_IMEI_NAMED_V5,
        DEVICE_ID_UUID_IMEI_V4,
        DEVICE_ID_PREDEFINED,
        DEVICE_ID_MAX
    };

    /// The policy that specifies whether to hide MAC address.
    /// SHOW_MAC_IN_PANI : show MAC address in P-Access-Network-Info header is allowed.
    /// HIDE_MAC_IN_PANI : showing MAC address in P-Access-Network-Info header is not allowed.
    /// HIDE_MAC_IN_PANI_EXCEPT_N11_AND_ECALL : showing MAC address in P-Access-Network-Info header
    ///                                         is not allowed except for N11 and emergency call.
    enum
    {
        SHOW_MAC_IN_PANI = 0,
        HIDE_MAC_IN_PANI = 1,
        HIDE_MAC_IN_PANI_EXCEPT_N11_AND_ECALL = 2
    };
};

#endif
