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
#ifndef INTERFACE_SUBSCRIBER_CONFIG_H_
#define INTERFACE_SUBSCRIBER_CONFIG_H_

#include "ImsVector.h"

#include "CarrierConfig.h"
#include "IAsyncConfig.h"
#include "IImsSubscriberInfo.h"

class IConfigurable;
class ISubscriberConfigListener;
class ServerAddress;

class ISubscriberConfig : public IAsyncConfig
{
protected:
    virtual ~ISubscriberConfig() = default;

public:
    /**
     * @brief Returns the primary P-CSCF address when the P-CSCF discovery method is
     *        #PCSCF_DISCOVERY_METHOD_CONFIG.
     *
     * @return A P-CSCF address.
     */
    virtual ServerAddress* GetPcscfAddress() const = 0;

    /**
     * @brief Returns all the P-CSCF addresses when the P-CSCF discovery method is
     *        #PCSCF_DISCOVERY_METHOD_CONFIG.
     *
     * @return A list of P-CSCF address.
     */
    virtual const ImsVector<ServerAddress*>& GetPcscfAddresses() const = 0;

    /**
     * @brief Returns the primary P-CSCF discovery method.
     *
     * @return A P-CSCF discovery method. Possible values are as follows:\n
     *         #PCSCF_DISCOVERY_METHOD_PCO\n
     *         #PCSCF_DISCOVERY_METHOD_CONFIG
     */
    virtual IMS_SINT32 GetPcscfDiscoveryMethod() const = 0;

    /**
     * @brief Returns the P-CSCF discovery methods.
     *
     * The first one is the primry discovery method, and others will be used for the fallback case.
     *
     * @return A P-CSCF discovery methods. Possible values are as follows:\n
     *         #PCSCF_DISCOVERY_METHOD_PCO\n
     *         #PCSCF_DISCOVERY_METHOD_CONFIG
     */
    virtual const ImsVector<IMS_SINT32>& GetPcscfDiscoveryMethods() const = 0;

    /**
     * @brief Returns the count of IMS subscriber information.
     *
     * @return A number of IMS subscriber info.
     */
    virtual IMS_SINT32 GetSubscriberCount() const = 0;

    /**
     * @brief Returns the instance of IMS subscriber info. at the specified index.
     *
     * @param nIndex The position to be retrieved.
     * @return An IImsSubscriberInfo instance.
     */
    virtual IImsSubscriberInfo* GetSubscriberInfo(IN IMS_SINT32 nIndex = 0) const = 0;

    /**
     * @brief Checks if the IMS AKA is supported or not.
     *
     * It will be determined by the algorithm of the credential information.
     *
     * @return IMS_TRUE if IMS AKA supports. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsAkaSupported() const = 0;

    /**
     * @brief Checks if the debug mode is enabled or not.
     *
     * @return IMS_TRUE if debug mode is enabled. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsDebugOn() const = 0;

    /**
     * @brief Checks if the IMS service is allowed or not.
     *
     * @return IMS_TRUE if IMS service is allowed. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsServiceAllowed() const = 0;

    /**
     * @brief Checks if the IMS supports the credential information from ISIM application.
     *
     * @return IMS_TRUE if IMS supports ISIM. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsIsimSupported() const = 0;

    /**
     * @brief Checks if the configuration is successfully loaded or not.
     *
     * The listener will be invoked after provisioning is completed.
     *
     * @return IMS_TRUE if the configuration is successfully provisioned. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsProvisioningDone() const = 0;

    /**
     * @brief Checks if the IMS supports the credential information from USIM application.
     *
     * @return IMS_TRUE if IMS supports USIM. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsUsimSupported() const = 0;

    /**
     * @brief Checks if the testmode is enabled or not.
     *
     * @return IMS_TRUE if the test mode is enabled. Otherwise, IMS_FALSE.
     */
    virtual IMS_BOOL IsTestMode() const = 0;

    /**
     * @brief Returns the subscription attributes.
     *
     * @return The subscription attributes.
     */
    virtual IMS_SINT32 GetSubscriptionAttributes() const = 0;

    /**
     * @brief Returns the configurable interface from the subscriber configuration.
     *
     * @return An IConfigurable instance.
     */
    virtual IConfigurable* GetConfigurable() const = 0;

    /**
     * @brief Removes the listener to be notified the status of IMS subscriber information.
     *
     * @param piListener The listener to monitor the subscription change.
     */
    virtual void RemoveListener(IN ISubscriberConfigListener* piListener) = 0;

    /**
     * @brief Sets the listener to be notified the status of IMS subscriber information.
     *
     * @param piListener The listener to monitor the subscription change.
     * @param nEvents The events of interest to listen.
     */
    virtual void SetListener(IN ISubscriberConfigListener* piListener,
            IN IMS_SINT32 nEvents = LISTEN_EVENT_DEFAULT) = 0;

    /**
     * @brief Enables ISIM attribute. The IMS-AKA also performs using the ISIM instead of USIM.
     *
     * NOTE: If ISIM attribute is being enabled, the IMS identities will be loaded from
     *       the ISIM application. Otherwise, there is no action.
     */
    virtual void EnableIsim() = 0;

    /**
     * @brief Updates IMS identities that are created based on USIM information.
     *
     * @param strHomeDomainName The home domain name.
     * @param strPrivateUserId The private user identity.
     * @param strPublicUserId The public user identity.
     * @param bIsimEnabled The flag specifying whether ISIM should be enabled for IMS AKA.
     */
    virtual void UpdateSubscriberInfo(IN const AString& strHomeDomainName,
            IN const AString& strPrivateUserId, IN const AString& strPublicUserId,
            IN IMS_BOOL bIsimEnabled = IMS_FALSE) = 0;

    /**
     * @brief Updates IMS identities that are created based on USIM information.
     *
     * @param strHomeDomainName The home domain name.
     * @param strPrivateUserId The private user identity.
     * @param objPublicUserIds The list of public user identity.
     * @param bIsimEnabled The flag specifying whether ISIM should be enabled for IMS AKA.
     */
    virtual void UpdateSubscriberInfo(IN const AString& strHomeDomainName,
            IN const AString& strPrivateUserId, IN const AStringArray& objPublicUserIds,
            IN IMS_BOOL bIsimEnabled = IMS_FALSE) = 0;

    //// APIs for the values of a default IImsSubscriberInfo

    /**
     * @brief Returns the credential of a default IMS subscriber.
     *
     * @return The credential information of a default IMS subscriber.
     */
    virtual const Credential& GetCredential() const = 0;

    /**
     * @brief Returns the home domain name of a default IMS subscriber.
     *
     * @return The home domain name of a default IMS subscriber.
     */
    virtual const AString& GetHomeDomainName() const = 0;

    /**
     * @brief Returns the reference index of a primary public user identity.
     *
     * @return A reference index to identify a primary public user identity.
     */
    virtual IMS_SINT32 GetIndexOfPrimaryPublicUserId() const = 0;

    /**
     * @brief Returns the domain name to be used in the phone-context parameter.
     *
     * @return The domain name for phone-context parameter.
     */
    virtual const AString& GetPhoneContext() const = 0;

    /**
     * @brief Returns the private user identity of a default IMS subscriber.
     *
     * @return A private user identity.
     */
    virtual const AString& GetPrivateUserId() const = 0;

    /**
     * @brief Returns the public user identity of a default IMS subscriber at the specified index.
     *
     * @param nImpuType The IMPU type to be retrieved.
     *                  #IImsSubscriberInfo#IMPU_REF_INDEX\n
     *                  #IImsSubscriberInfo#IMPU_SIP\n
     *                  #IImsSubscriberInfo#IMPU_TEL
     * @return A public user identity.
     */
    virtual const AString& GetPublicUserId(
            IN IMS_SINT32 nImpuType = IImsSubscriberInfo::IMPU_REF_INDEX) const = 0;

    /**
     * @brief Returns all the public user identities of a default IMS subscriber.
     *
     * @return List of public user identity.
     */
    virtual const AStringArray& GetPublicUserIds() const = 0;

public:
    /// Error codes when the provisioning failed
    enum
    {
        ERROR_UNSPECIFIED = 0,
        ERROR_NO_ISIM_APPLICATION
    };

    /// Subscription attributes
    enum
    {
        SUBSCRIPTION_ATTRIBUTE_NONE = 0,

        /// Ims service: on / off
        SUBSCRIPTION_ATTRIBUTE_IMS = 1 << 0,
        /// ISIM
        SUBSCRIPTION_ATTRIBUTE_ISIM = 1 << 1,
        /// USIM (for authentication)
        SUBSCRIPTION_ATTRIBUTE_USIM = 1 << 2,

        /// Testmode: for local test environment
        SUBSCRIPTION_ATTRIBUTE_TESTMODE = 1 << 29,
        /// Debug: for debugging purpose
        SUBSCRIPTION_ATTRIBUTE_DEBUG = 1 << 30,

        SUBSCRIPTION_ATTRIBUTE_ALL = 0x7FFFFFFF
    };

    /// P-CSCF discovery methods
    enum
    {
        PCSCF_DISCOVERY_METHOD_PCO = CarrierConfig::Ims::PCSCF_DISCOVERY_METHOD_PCO,
        PCSCF_DISCOVERY_METHOD_CONFIG = CarrierConfig::Ims::PCSCF_DISCOVERY_METHOD_CONFIG
    };

    /// Listening events for ISubscriberConfigListener
    enum
    {
        /// Calls the listener when anything related to ISIM provisioning changes.
        LISTEN_EVENT_ISIM_PROVISIONING = 1 << 0,
        /// Calls the listener when anything related to manual provisioning changes.
        LISTEN_EVENT_MANUAL_PROVISIONING = 1 << 1,
        /// Calls the listener when any information changes.
        LISTEN_EVENT_DEFAULT = LISTEN_EVENT_ISIM_PROVISIONING | LISTEN_EVENT_MANUAL_PROVISIONING
    };
};

#endif
