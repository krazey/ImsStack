#ifndef _INTERFACE_SIP_RT_CONFIG_HELPER_H_
#define _INTERFACE_SIP_RT_CONFIG_HELPER_H_

#include "SipRtConfig.h"

/**
 * @brief This class provides an interface to configure the runtime configuration for SIP handling.
 *
 * @see SIPRTConfig
 */
class ISIPRTConfigHelper
{
public:
    /**
     * @brief Disables SIP run-time feature.
     *
     * @param nFeature Feature to be disable\n
     *                 #SIPRTConfig#FEATURE_NONE\n
     *                 #SIPRTConfig#FEATURE_SIP_TX_PACKET_BLOCKED
     */
    virtual void DisableFeature(IN IMS_SINT32 nFeature) = 0;

    /**
     * @brief Enables SIP run-time feature.
     *
     * @param nFeature Feature to be enable\n
     *                 #SIPRTConfig#FEATURE_NONE\n
     *                 #SIPRTConfig#FEATURE_SIP_TX_PACKET_BLOCKED
     */
    virtual void EnableFeature(IN IMS_SINT32 nFeature) = 0;

    /**
     * @brief Gets SIP run-time features.
     *
     * @return Enabled features (Bit-mask of followings).\n
     *         #SIPRTConfig#FEATURE_NONE\n
     *         #SIPRTConfig#FEATURE_SIP_TX_PACKET_BLOCKED
     */
    virtual IMS_SINT32 GetFeatures() const = 0;

    /**
     * @brief Gets the run-time SIP header configuration.
     *
     * @param strName SIP header name
     * @return Pointer to SIP header configuration (SIPRTConfig::Header*).
     */
    virtual const SIPRTConfig::Header* GetHeader(IN CONST AString &strName) const = 0;

    /**
     * @brief Removes the SIP run-time (or real-time) configuration.
     *
     * If pParam is null, it will remove all the configuration values for that item.
     *
     * @param nItem Configuration item to be removed\n
     *              #CONFIG_I_LOG_MASK\n
     *              #CONFIG_I_REUSEADDR\n
     *              #CONFIG_I_LINGER\n
     *              #CONFIG_I_SHUTDOWN\n
     *              #CONFIG_I_KEEPALIVE\n
     *              #CONFIG_I_TCP_KEEP_COUNT\n
     *              #CONFIG_I_TCP_KEEP_IDLE\n
     *              #CONFIG_I_TCP_KEEP_INTERVAL\n
     *              #CONFIG_I_IP_QOS\n
     *              #CONFIG_I_SIP_HEADER\n
     *              #CONFIG_I_IPSEC_SA\n
     *              #CONFIG_I_TCP_PORT_RANGE\n
     *              #CONFIG_I_REG_CONTACT_ADDRESS
     * @param pParam Related parameter of the specified configuration item
     */
    virtual void RemoveConfig(IN IMS_SINT32 nItem, IN SIPRTConfig::Base *pParam) = 0;

    /**
     * @brief Sets the SIP run-time (or real-time) configuration.
     *
     * @param nItem Configuration item to be set\n
     *              #CONFIG_I_LOG_MASK\n
     *              #CONFIG_I_REUSEADDR\n
     *              #CONFIG_I_LINGER\n
     *              #CONFIG_I_SHUTDOWN\n
     *              #CONFIG_I_KEEPALIVE\n
     *              #CONFIG_I_TCP_KEEP_COUNT\n
     *              #CONFIG_I_TCP_KEEP_IDLE\n
     *              #CONFIG_I_TCP_KEEP_INTERVAL\n
     *              #CONFIG_I_IP_QOS\n
     *              #CONFIG_I_SIP_HEADER\n
     *              #CONFIG_I_IPSEC_SA\n
     *              #CONFIG_I_TCP_PORT_RANGE\n
     *              #CONFIG_I_REG_CONTACT_ADDRESS
     * @param pParam Related parameter of the specified configuration item
     * @return If it succeeds, returns IMS_SUCCESS. Otherwise, returns IMS_FAILURE.
     */
    virtual IMS_RESULT SetConfig(IN IMS_SINT32 nItem, IN SIPRTConfig::Base *pParam) = 0;
};

#endif // _INTERFACE_SIP_RT_CONFIG_HELPER_H_
