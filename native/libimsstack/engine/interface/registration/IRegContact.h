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
#ifndef INTERFACE_REG_CONTACT_H_
#define INTERFACE_REG_CONTACT_H_

#include "IpAddress.h"

#include "CarrierConfig.h"

#include "SipAddress.h"

class IRegContactListener;

/**
 * @brief This class provides an interface to access/control the Contact information
 *        of IMS registration.
 */
class IRegContact
{
protected:
    virtual ~IRegContact() = default;

public:
    /**
     * @brief Adds the header parameter for this contact.
     *
     * @param strName the header parameter name
     * @param strValue the header parameter value
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddHeaderParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) = 0;

    /**
     * @brief Adds the uri parameter for this contact.
     *
     * @param strName the URI parameter name
     * @param strValue the URI parameter value
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddUriParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) = 0;

    /**
     * @brief Returns the contact address (SIP URI format).
     *
     * It excludes the header parameters, but includes the uri parameter.
     *
     * @return Contact address (SIP URI)
     */
    virtual const SipAddress& GetContactAddress() const = 0;

    /**
     * @brief Returns the expiration value for this contact.
     *
     * @return Expires value.
     */
    virtual IMS_UINT32 GetExpires() const = 0;

    /**
     * @brief Returns the IP address for this contact.
     *
     * @return IP address for this contact.
     */
    virtual const IpAddress& GetIpAddress() const = 0;

    /**
     * @brief Returns the port number for this contact.
     *
     * @return Port number for this contact.
     */
    virtual IMS_SINT32 GetPort() const = 0;

    /**
     * @brief Returns the header parameters of this contact.
     *
     * @return List of header parameter of this contact.
     */
    virtual const ImsList<SipParameter*>& GetHeaderParameters() const = 0;

    /**
     * @brief Returns the instance("+sip.instance") header parameter.
     *
     * @return Pointer to SIP header parameter for "+sip.instance".
     */
    virtual const SipParameter* GetInstanceParameter() const = 0;

    /**
     * @brief Returns the registration identifier("reg-id") header parameter.
     *
     * @return Pointer to SIP header parameter for "reg-id".
     */
    virtual const SipParameter* GetRegIdParameter() const = 0;

    /**
     * @brief Returns the public GRUU.
     *
     * @return Pointer to public GRUU.
     */
    virtual const SipAddress* GetPublicGruu() const = 0;

    /**
     * @brief Returns the valid (the latest) temporary GRUU.
     *
     * @return Pointer to temporary GRUU.
     */
    virtual const SipAddress* GetTemporaryGruu() const = 0;

    /**
     * @brief Returns the valid temporary GRUUs.
     *
     * @return List of temporary GRUU.
     */
    virtual const ImsList<SipAddress*>& GetTemporaryGruus() const = 0;

    /**
     * @brief Checks if the contact is an active or not.
     *
     * If the return value is TRUE, the binding (AOR - Contact) is successfully added
     * to the IMS network.
     *
     * @return If the contact is active binding, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsActiveBinding() const = 0;

    /**
     * @brief Checks if the service capability is registered or not.
     *
     * @return If there is no service registered, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsEmpty() const = 0;

    /**
     * @brief Removes the all header parameters from this contact.
     *
     * It just removes the parameters which are set by the application.
     */
    virtual void RemoveAllHeaderParameters() = 0;

    /**
     * @brief Removes the header parameter from this contact.
     *
     * If the strValue is NULL, the function removes the parameter.
     *
     * @param strName the header parameter name
     * @param strValue the header parameter value
     */
    virtual void RemoveHeaderParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) = 0;

    /**
     * @brief Removes the uri parameter from this contact.
     *
     * If the strValue is NULL, the function removes the parameter.
     *
     * @param strName the URI parameter name
     * @param strValue the URI parameter value
     */
    virtual void RemoveUriParameter(
            IN const AString& strName, IN const AString& strValue = AString::ConstNull()) = 0;

    /**
     * @brief Sets the display name for this contact.
     *
     * @param strDisplayName display name
     */
    virtual void SetDisplayName(IN const AString& strDisplayName) = 0;

    /**
     * @brief Sets the listener for this contact.
     *
     * @param piListener listener to be set
     */
    virtual void SetListener(IN IRegContactListener* piListener) = 0;

    /**
     * @brief Sets the policy for the caller capability. The default is based on the configuration.
     *
     * Generally, the caller capabilities are provisioned in IMSRegistry, and it refers
     * the ICSI, IARI and feature-tags.\n
     * If the application wants to use only its own provisioned capabilities, it MUST invoke
     * this method and pass the argument as "IMS_TRUE".
     *
     * @param bCapsByApp flag to indicate which caller capability will be choosed by this contact
     */
    virtual void SetPolicyForCallerCapability(IN IMS_BOOL bCapsByApp) = 0;

    /**
     * @brief Sets the port number for this contact.
     *
     * It can be invoked to change the listening channel when IPSec is used
     * & SA(Security Association) is established.
     *
     * @param nPort port number (Protected server port) to be set
     */
    virtual void SetPort(IN IMS_SINT32 nPort) = 0;

    /**
     * @brief Sets the rule for user-info part of Contact header for this contact.
     *
     * @param nUserInfoPart The user-info part to be applied\n
     *                      #USER_INFO_PART_UUID\n
     *                      #USER_INFO_PART_IMPU\n
     *                      #USER_INFO_PART_EMPTY
     */
    virtual void SetUserInfo(IN IMS_SINT32 nUserInfoPart) = 0;

    /**
     * @brief Adds the extra capability(feature) parameter for this contact.
     *
     * It allows strValue as the comma-separated value list with DQUOT.
     *
     * @param strName the feature parameter name
     * @param strValue the feature parameter value
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddExtraCapability(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Removes the extra capability(feature) parameter from this contact.
     *
     * If the strName & strValue is NULL, the function removes all the extra parameters.
     *
     * @param strName the feature parameter name
     * @param strValue the feature parameter value
     */
    virtual void RemoveExtraCapability(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Adds the specified service to the current registration contact.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @return If it succeeds, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL AddService(IN const AString& strAppId, IN const AString& strServiceId) = 0;

    /**
     * @brief Removes the specified service from the current registration contact.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     */
    virtual void RemoveService(IN const AString& strAppId, IN const AString& strServiceId) = 0;

    /**
     * @brief Checks if the specified service is registered or not.
     *
     * @param strAppId an IMS application identifier
     * @param strServiceId an IMS service identifier
     * @return If the service is registered via IMS network, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsServiceRegistered(
            IN const AString& strAppId, IN const AString& strServiceId) const = 0;

    /**
     * @brief Checks if the specified service is registered or not.
     *
     * @param strFtName the name of feature-tag
     * @param strFtValue the value of feature-tag
     * @return If the feature is registered via IMS network, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsFeatureRegistered(IN const AString& strFtName,
            IN const AString& strFtValue = AString::ConstNull()) const = 0;

    /**
     * @brief Recalculates the caller capabilities after adding or removing the services.
     */
    virtual void RecalculateCallerCapabilities() = 0;

public:
    /// Rule for user-info part of Contact header
    enum
    {
        /// Default, use time-based UUID
        USER_INFO_PART_UUID = CarrierConfig::Ims::REGISTRATION_CONTACT_USER_INFO_PART_UUID,
        /// Use user-info part from IMPU
        USER_INFO_PART_IMPU = CarrierConfig::Ims::REGISTRATION_CONTACT_USER_INFO_PART_IMPU,
        /// No user-info part
        USER_INFO_PART_EMPTY = CarrierConfig::Ims::REGISTRATION_CONTACT_USER_INFO_PART_EMPTY
    };
};

#endif
