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
#ifndef INTERFACE_SIP_RT_CONFIG_HELPER_H_
#define INTERFACE_SIP_RT_CONFIG_HELPER_H_

#include "SipRtConfig.h"

/**
 * @brief This class provides an interface to configure the runtime configuration for SIP handling.
 *
 * @see SipRtConfig
 */
class ISipRtConfigHelper
{
protected:
    virtual ~ISipRtConfigHelper() = default;

public:
    /**
     * @brief Disables SIP run-time feature.
     *
     * @param nFeature Feature to be disable\n
     *                 #SipRtConfig#FEATURE_NONE\n
     *                 #SipRtConfig#FEATURE_SIP_TX_PACKET_BLOCKED
     */
    virtual void DisableFeature(IN IMS_SINT32 nFeature) = 0;

    /**
     * @brief Enables SIP run-time feature.
     *
     * @param nFeature Feature to be enable\n
     *                 #SipRtConfig#FEATURE_NONE\n
     *                 #SipRtConfig#FEATURE_SIP_TX_PACKET_BLOCKED
     */
    virtual void EnableFeature(IN IMS_SINT32 nFeature) = 0;

    /**
     * @brief Gets SIP run-time features.
     *
     * @return Enabled features (Bit-mask of followings).\n
     *         #SipRtConfig#FEATURE_NONE\n
     *         #SipRtConfig#FEATURE_SIP_TX_PACKET_BLOCKED
     */
    virtual IMS_SINT32 GetFeatures() const = 0;

    /**
     * @brief Gets the run-time SIP header configuration.
     *
     * @param strName SIP header name
     * @return Pointer to SIP header configuration (SipRtConfig::Header*).
     */
    virtual const SipRtConfig::Header* GetHeader(IN const AString& strName) const = 0;

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
    virtual void RemoveConfig(IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam) = 0;

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
    virtual IMS_RESULT SetConfig(IN IMS_SINT32 nItem, IN SipRtConfig::Base* pParam) = 0;
};

#endif
