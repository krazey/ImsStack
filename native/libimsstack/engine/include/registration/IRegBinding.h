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
#ifndef INTERFACE_REG_BINDING_H_
#define INTERFACE_REG_BINDING_H_

#include "AStringArray.h"
#include "IpAddress.h"

#include "SipAddress.h"

class IRegBindingListener;
class IRegInfo;
class SipProfile;

class IRegBinding
{
protected:
    virtual ~IRegBinding() = default;

public:
    /**
     * @brief Returns the network authorized public user identities.
     *
     * @return The list of public user identity.
     */
    virtual const AStringArray& GetAssociatedUris() const = 0;

    /**
     * @brief Returns the network authorized & registered public user identity.
     *
     * @return The registered public user identity.
     */
    virtual const SipAddress& GetAuthorizedAor() const = 0;

    /**
     * @brief Returns the preferred contact address with the highest 'q' value of this registration.
     *
     * @return The preferred contact address.
     */
    virtual const SipAddress& GetContactAddress() const = 0;

    /**
     * @brief Returns the preferred contact address for all the outgoing SIP message.
     *
     * @return The preferred contact address for outgoing SIP messages.
     */
    virtual const SipAddress* GetContactAddressForOutgoingMessage() const = 0;

    /**
     * @brief Returns the IP address of the preferred contact address.
     *
     * @return The IP address of preferred contact address.
     */
    virtual const IPAddress& GetIpAddress() const = 0;

    /**
     * @brief Returns the Path header list.
     *
     * @return The list of Path header.
     */
    virtual const AStringArray& GetPathHeaders() const = 0;

    /**
     * @brief Returns the port number for the flow control based on RFC5626.
     *
     * @return The port number for the flow control.
     * @note RFC5626_FLOW_CONTROL
     */
    virtual IMS_SINT32 GetPortFlowControl() const = 0;

    /**
     * @brief Returns the protected / unprotected client port number (UE).
     *
     * @return The UE's client port number.
     */
    virtual IMS_SINT32 GetPortUc() const = 0;

    /**
     * @brief Returns the protected / unprotected server port number (UE).
     *
     * @return The UE's server port number.
     */
    virtual IMS_SINT32 GetPortUs() const = 0;

    /**
     * @brief Returns the reginfo when the 'reg' event package subscription is supported.
     *
     * @return The IRegInfo instance.
     */
    virtual const IRegInfo* GetRegInfo() const = 0;

    /**
     * @brief Returns the list of Security-Client header if present.
     *
     * @return The list of Security-Client header.
     */
    virtual const AStringArray& GetSecurityClients() const = 0;

    /**
     * @brief Returns the list of Security-Verify header if present.
     *
     * @return The list of Security-Server header.
     */
    virtual const AStringArray& GetSecurityVerifys() const = 0;

    /**
     * @brief Returns the preloaded route set for the outgoing SIP request.
     *
     * @return The list of preloaded route for outgoing SIP requests.
     */
    virtual const AStringArray& GetServiceRoutes() const = 0;

    /**
     * @brief Returns the SIP profile of registration binding.
     *
     * @return The SipProfile instance.
     */
    virtual SipProfile* GetSipProfile() const = 0;

    /**
     * @brief Returns the state of registration binding.
     *
     * @return The state of RegBinding.\n
     *         #STATE_CREATED\n
     *         #STATE_INIT\n
     *         #STATE_INIT_PENDING\n
     *         #STATE_ACTIVE\n
     *         #STATE_ACTIVE_PENDING\n
     *         #STATE_ACTIVE_TERMINATING\n
     *         #STATE_TERMINATED
     */
    virtual IMS_SINT32 GetState() const = 0;

    /**
     * @brief Returns the subscription identifier which can identify the subscriber.
     *
     * @return The idnetifier of the subscriber.
     */
    virtual const AString& GetSubscriberId() const = 0;

    /**
     * @brief Returns the transport extensions.
     *
     * @return The transport extension (Sip#TRANSPORT_EXT_XXX in Sip.h).
     */
    virtual IMS_SINT32 GetTransportExt() const = 0;

    /**
     * @brief Returns the instance("+sip.instance") header parameter.
     *
     * @return The SipParameter instance for "+sip.instance" parameter.
     */
    virtual const SipParameter* GetInstanceParameter() const = 0;

    /**
     * @brief Returns the public GRUU.
     *
     * @return The public GRUU.
     */
    virtual const SipAddress* GetPublicGruu() const = 0;

    /**
     * @brief Returns the valid (the latest) temporary GRUU.
     *
     * @return The temporary GRUU.
     */
    virtual const SipAddress* GetTemporaryGruu() const = 0;

    /**
     * @brief Returns the valid temporary GRUUs.
     *
     * @return The list of temporary GRUU.
     */
    virtual const IMSList<SipAddress*>& GetTemporaryGruus() const = 0;

    /**
     * @brief Checks if the UA is located behind a NAT or not.
     *
     * @return true if UA is located behind a NAT / FW, false otherwise.
     */
    virtual IMS_BOOL IsBehindNat() const = 0;

    /**
     * @brief Checks if the UA is located within the trust domain.
     *
     * @return true if UA is located within the trust domain, false otherwise.
     */
    virtual IMS_BOOL IsWithinTrustDomain() const = 0;

    /**
     * @brief Notifies the registration when the caller capability is changed to refresh
     *        the IMS registration if the device is already registered to the IMS network.
     */
    virtual void NotifyCallerCapabilityChanged() = 0;

    /**
     * @brief Sets the listener for this registration binding.
     */
    virtual void SetListener(IN IRegBindingListener* piListener) = 0;

public:
    enum
    {
        STATE_CREATED = 0,
        STATE_INIT,
        STATE_INIT_PENDING,
        STATE_ACTIVE,
        STATE_ACTIVE_PENDING,
        STATE_ACTIVE_TERMINATING,
        STATE_TERMINATED
    };
};

#endif
