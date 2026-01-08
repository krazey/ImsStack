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
#ifndef INTERFACE_SERVICE_H_
#define INTERFACE_SERVICE_H_

#include "AStringArray.h"
#include "IpAddress.h"

#include "IConnection.h"
#include "SipAddress.h"
#include "SipMethod.h"

class IFeatureCaps;
class IRegInfo;
class IServiceFilterCriteria;
class SipProfile;

/**
 * @brief This class provides a base service interface for IMS services,
 *        and the service follows the Generic Connection Framework (GCF).
 *
 * @see IConnection, ICoreService
 */
class IService : public IConnection
{
protected:
    ~IService() override = default;

public:
    /**
     * @brief Returns the application id string that this Service was created with.
     *
     * @return An IMS application identifier.
     */
    virtual const AString& GetAppId() const = 0;

    /**
     * @brief Returns the scheme used for this Service.
     *
     * @return The connection scheme which is used in Connector#open().
     */
    virtual const AString& GetScheme() const = 0;

    /**
     * @brief Returns the display name and the authorized public user identity for the Service.
     *
     * If "userId=" parameter in the Connector::Open(...) is used to create the service,
     * it will be returned by this method.\n
     * Otherwise, according to the registration, the default public user identity of this
     * device or network-chosen public user identity (topmost in P-Associated-URI)
     * will be returned.
     *
     * @return Default public user identity, possibly with a display name.
     * @note IMS EXTENSION METHOD
     */
    virtual const SipAddress& GetAuthorizedUserId() const = 0;

    /**
     * @brief Returns the SIP contact address for this service.
     *
     * @return Contact address for this service.
     * @note IMS EXTENSION METHOD
     */
    virtual const SipAddress& GetContactAddress() const = 0;

    /**
     * @brief Returns the preferred contact address for all the outgoing SIP message.
     *
     * @return Preferred contact address for this service.
     * @note IMS EXTENSION METHOD
     */
    virtual const SipAddress* GetContactAddressForOutgoingMessage() const = 0;

    /**
     * @brief Returns the SIP Contact header for this service.
     *
     * NOTE: The application MUST release the resource for the returned ISipHeader instance.
     *
     * @param bPrivacy flag to indicate if the privacy is required or not
     * @param bRequest flag to indicate if the contact header is for request or response
     * @param nSipMethod SIP method to be set to obtain the method-specific Contact header\n
     *                   - INVITE, SUBSCRIBE, REFER, NOTIFY, OPTIONS, PUBLISH
     * @return Contact header for this service.
     * @note IMS EXTENSION METHOD
     */
    virtual ISipHeader* GetContactHeader(IN IMS_BOOL bPrivacy = IMS_FALSE,
            IN IMS_BOOL bRequest = IMS_TRUE,
            IN IMS_SINT32 nSipMethod = SipMethod::INVALID) const = 0;

    /**
     * @brief Returns the instance of feature capability (caller capability) for this service.
     *
     * @return Feature capability for this service.
     * @note IMS EXTENSION METHOD
     */
    virtual IFeatureCaps* GetFeatureCaps() const = 0;

    /**
     * @brief Returns the instance of service filter criteria for this service.
     *
     * @return Service filter criteria for this service.
     * @note IMS EXTENSION METHOD
     */
    virtual IServiceFilterCriteria* GetFilterCriteria() const = 0;

    /**
     * @brief Returns the Path header list.
     *
     * @return List of Path header.
     * @note IMS EXTENSION METHOD
     */
    virtual const AStringArray& GetPathHeaders() const = 0;

    /**
     * @brief Returns the reginfo of this service only if the 'reg' event package is supported.
     *
     * @return Pointer to IRegInfo or null.
     * @note IMS EXTENSION METHOD
     */
    virtual const IRegInfo* GetRegInfo() const = 0;

    /**
     * @brief Returns the device's IP address for this service.
     *
     * @return IP address for this service.
     * @note IMS EXTENSION METHOD
     */
    virtual const IpAddress& GetIpAddress() const = 0;

    /**
     * @brief Returns the SIP profile of this service.
     *
     * If the application doesn't set its own SIP profile, it returns the SIP profile
     * which is used in IMS registration if present.
     *
     * @return Pointer to SIPProfiile.
     * @note IMS EXTENSION METHOD, MULTI_REG_SIP_PROFILE
     */
    virtual SipProfile* GetSipProfile() const = 0;

    /**
     * @brief Returns network provisioned user identities. The first item is the network-chosen
     *        default user identity.
     *
     * The user identities can be used as an argument to creating services.\n
     * The user identities are formatted according to SIP or TEL URI, see [RFC3261] and [RFC3966].
     *
     * @return The network provisioned user identities, an empty string array
     *         will be returned if no user identities are available.
     * @note IMS EXTENSION METHOD
     */
    virtual const AStringArray& GetUserIdentities() const = 0;

    /**
     * @brief Returns network provisioned user identity with the specified URI scheme.
     *
     * The user identities can be used as an argument to creating services.
     * The user identities are formatted according to SIP or TEL URI, see [RFC3261] and [RFC3966].
     *
     * @param nScheme TEL/SIP URI (refer to @ref SIP class)
     * @return The network provisioned user identity, a null string
     *         will be returned if no user identities are available.
     * @note IMS EXTENSION METHOD
     */
    virtual const AString& GetUserIdentity(IN IMS_SINT32 nScheme) const = 0;

    /**
     * @brief Returns the instance("+sip.instance") header parameter.
     *
     * @return Pointer to SIP header parameter for "+sip.instance".
     * @note IMS EXTENSION METHOD
     */
    virtual const SipParameter* GetInstanceParameter() const = 0;

    /**
     * @brief Returns the public GRUU.
     *
     * @return Pointer to public GRUU.
     * @note IMS EXTENSION METHOD
     */
    virtual const SipAddress* GetPublicGruu() const = 0;

    /**
     * @brief Returns the valid (the latest) temporary GRUU.
     *
     * @return Pointer to temporary GRUU.
     * @note IMS EXTENSION METHOD
     */
    virtual const SipAddress* GetTemporaryGruu() const = 0;

    /**
     * @brief Returns the valid temporary GRUUs.
     *
     * @return List of temporary GRUU.
     * @note IMS EXTENSION METHOD
     */
    virtual const ImsList<SipAddress*>& GetTemporaryGruus() const = 0;

    /**
     * @brief Checks if the UA is located behind a NAT or not.
     *
     * @return If UA is located behind a NAT / FW, then returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     * @note IMS EXTENSION METHOD
     */
    virtual IMS_BOOL IsBehindNat() const = 0;

    /**
     * @brief Returns whether the service is connected to the IMS network.
     *
     * @return If the service is connected to the IMS network, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     * @note IMS EXTENSION METHOD
     */
    virtual IMS_BOOL IsImsConnected() const = 0;

    /**
     * @brief Checks if the UA is within the trust domain or not.
     *
     * @return If UA is located within the trust domain, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     * @note IMS EXTENSION METHOD
     */
    virtual IMS_BOOL IsWithinTrustDomain() const = 0;

    /**
     * @brief Adds the specific feature tags for this service.
     *
     * It updates the cached caller capabilities.
     *
     * @param objFeatureTags feature-tags to be added
     * @param bRegRequired flag to indicate that the REG. is required or not
     * @return If the caller capability is updated, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     * @note IMS EXTENSION METHOD
     */
    virtual IMS_BOOL AddFeatureTags(
            IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE) = 0;

    /**
     * @brief Removes the specified feature tags from this service.
     *
     * It updates the cached caller capabilities.
     *
     * @param objFeatureTags feature-tags to be removed
     * @param bRegRequired flag to indicate that the REG. is required or not
     * @return If the caller capability is updated, returns IMS_TRUE.\n
     *         Otherwise, returns IMS_FALSE.
     * @note IMS EXTENSION METHOD
     */
    virtual IMS_BOOL RemoveFeatureTags(
            IN const ImsList<AString>& objFeatureTags, IN IMS_BOOL bRegRequired = IMS_TRUE) = 0;

    /**
     * @brief Sets the SIP profile for specific configuration of SIP connections
     *        which will be created via this service.
     *
     * As a default, the service uses the SIP profile which is used in IMS registration.\n
     * If the application doesn't want to use this profile, it can set the specific
     * SIP profile for this application's specific configuration.
     *
     * @param pProfile SIP profile to be set
     * @note IMS EXTENSION METHOD, MULTI_REG_SIP_PROFILE
     */
    virtual void SetSipProfile(IN SipProfile* pProfile) = 0;
};

#endif
